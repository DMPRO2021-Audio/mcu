#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <spidrv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "util.h"
#include "midi.h"
#include "queue.h"
#include "swo.h"
#include "synth.h"
#include "usart.h"

Synth synth = {
    .wavegens = {
        {.freq = F_SAMPLE / 261.63, .vol = 1.0}, /* C4 */
        {.freq = F_SAMPLE / 392.00, .vol = 1.0}, /* G4 */
    },
    .vol = 1.0
};

/* holds released wavegen indices */
Queue wavegen_queue = {
    .capacity = SYNTH_WAVEGEN_COUNT,
    .buf = {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, /* TODO */
};

char note_wavegens[128]; /* maps active MIDI notes to index of wavegen */

SPIDRV_HandleData_t synth_spi;

SPIDRV_Init_t synth_spi_init = {
    .port = USART1,
    .portLocationTx = USART_ROUTELOC0_TXLOC_LOC1,
    .portLocationRx = USART_ROUTELOC0_RXLOC_LOC1,
    .portLocationClk = USART_ROUTELOC0_CLKLOC_LOC1,
    .portLocationCs = USART_ROUTELOC0_CSLOC_LOC1,
    .bitRate = 1000000,
    .frameLength = 8,
    .type = spidrvMaster,
    .bitOrder = spidrvBitOrderLsbFirst,
    .clockMode = spidrvClockMode0,
    .csControl = spidrvCsControlApplication,
};

Ecode_t ecode = ECODE_OK; /* for debugging */

static void complete_synth_transfer(SPIDRV_Handle_t handle, Ecode_t status, int nbytes) {
    if (nbytes != sizeof(synth)) return;
    synth_clearcmds(&synth);
    //TODO: release chip select
}

static void transfer_synth(void) {
    ecode = SPIDRV_AbortTransfer(&synth_spi);
    if (ecode != ECODE_EMDRV_SPIDRV_OK && ecode != ECODE_EMDRV_SPIDRV_IDLE) {
        exit(1);
    }
    // TODO: assert chip select (but don't release it after abort!)
    ecode = SPIDRV_MTransmit(&synth_spi, (void *)&synth, sizeof(synth), complete_synth_transfer);
    if (ecode != ECODE_EMDRV_SPIDRV_OK) exit(1);
}

static void note_on(char note, char velocity) {
    /* TODO: use velocity to adjust envelope? */
    static EnvelopeStep envelope[] = {
        ENVELOPE_STEP(1.0, 0.2 * F_SAMPLE),
        ENVELOPE_STEP(-0.2, 0.2 * F_SAMPLE),
    };

    char idx;
    WaveGen *w;

    if (!queue_get(&wavegen_queue, &idx, 1)) {
        return; /* no wavegens available */
    }

    note_wavegens[note] = idx; /* TODO: what if note is already on? */
    w = &synth.wavegens[idx];
//    w->freq = notes[note];
    w->freq = 440;
    wavegen_set_vol_envelope(w, envelope, lenof(envelope));
}

static void note_off(char note, char velocity) {
    /* TODO: use velocity to adjust envelope? */
    static EnvelopeStep envelope[] = {
        ENVELOPE_STEP(-1.0, 0.2 * F_SAMPLE),
    };

    char idx;
    WaveGen *w;

    idx = note_wavegens[note]; /* TODO: what if note is already off? */
    w = &synth.wavegens[idx];
    wavegen_set_vol_envelope(w, envelope, lenof(envelope));

    if (!queue_put(&wavegen_queue, &idx, 1)) exit(1);
}

int main(void) {
    CHIP_Init();
    SWO_Setup();
    CMU_ClockEnable(cmuClock_GPIO, true);
    SPIDRV_Init(&synth_spi, &synth_spi_init);

    uart_init();
    __enable_irq();

    while (1) {
        uint8_t byte = uart_next_valid_byte();

        if (byte & MIDI_STATUS_MASK) {
            switch (byte & MIDI_COMMAND_MASK) {
            case MIDI_NOTE_ON:
                {
                    char note = uart_next_valid_byte();
                    char velocity = uart_next_valid_byte();

                    note_on(note, velocity);
                }
                break;
            case MIDI_NOTE_OFF:
                {
                    char note = uart_next_valid_byte();
                    char velocity = uart_next_valid_byte();

                    note_off(note, velocity);
                }
                break;
            }
            transfer_synth();
        }
    }
}

void _exit(int status) {
    (void) status;
    while (1) {}
}
