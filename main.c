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
#include "synth.h"
#include "usart.h"

Synth synth = {
    .master_volume = 1.0,
    .pan = { .balance = 0, },
    .reverb = {
        .tau = {3003, 3403, 3905, 4495, 241, 83},
        .gain = {
            F2FP(0.895),
            F2FP(0.883),
            F2FP(0.867),
            F2FP(0.853),
            F2FP(0.7),
            F2FP(0.7),
            F2FP(0.7)
        },
    },
};

/* holds released wavegen indices */
Queue wavegen_queue = {
    .capacity = SYNTH_WAVEGEN_COUNT,
    .buf = {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, /* TODO */
};

char note_wavegens[128]; /* maps active MIDI notes to index of wavegen */

SPIDRV_HandleData_t synth_spi;

SPIDRV_Init_t synth_spi_init = {
    .port = USART0,
    .portLocationTx = _USART_ROUTELOC0_TXLOC_LOC0,
    .portLocationRx = _USART_ROUTELOC0_RXLOC_LOC0,
    .portLocationClk = _USART_ROUTELOC0_CLKLOC_LOC0,
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
    GPIO_PinOutSet(PORTE, 13);
}

static void transfer_synth(void) {
    ecode = SPIDRV_AbortTransfer(&synth_spi);
    if (ecode != ECODE_EMDRV_SPIDRV_OK && ecode != ECODE_EMDRV_SPIDRV_IDLE) {
        exit(1);
    }
    GPIO_PinOutClear(PORTE, 13);
    ecode = SPIDRV_MTransmit(&synth_spi, (void *)&synth, sizeof(synth), complete_synth_transfer);
    if (ecode != ECODE_EMDRV_SPIDRV_OK) exit(1);
}

static void note_on(char note, char velocity) {
    /* TODO: use velocity to adjust envelope? */
    static EnvelopeStep envelope[] = {
        { .gain = 0,   .duration = 5, },
        { .gain = 255, .duration = 40, },
        { .gain = 212, .duration = 60, },
        { .gain = 176, .duration = 60, },
        { .gain = 126, .duration = 60, },
        { .gain = 64,  .duration = 60, },
        { .gain = 64,  .duration = 30, },
        { .gain = 64,   .duration = 0, },
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
        { .gain = 64,   .duration = 60, },
        { .gain = 32,   .duration = 60, },
        { .gain = 16,   .duration = 60, },
        { .gain =  0,   .duration = 60, },
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
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(PORTE, 13, gpioModePushPull, 1);
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
