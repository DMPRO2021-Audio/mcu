#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <spidrv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "midi.h"
#include "queue.h"
#include "synth.h"
#include "usart.h"

Synth synth = {
    .master_volume = F2FP(1.0),
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

// Calculated using [math.ceil(256 * 440 * pow(2, (i-69)/12)) for i in range(128)]
const uint32_t notes[128] = 
{
    2094, 2218, 2350, 2490, 2638, 2794, 2960, 3136, 3323, 3520, 3730, 3952, 4187, 4435, 4699, 4979, 5275, 5588, 5920, 6272, 6645, 7040, 7459, 7903, 8373, 8870, 9398, 9957, 10549, 11176, 11840, 12544, 13290, 14080, 14918, 15805, 16745, 17740, 18795, 19913, 21097, 22351, 23680, 25088, 26580, 28160, 29835, 31609, 33489, 35480, 37590, 39825, 42193, 44702, 47360, 50176, 53160, 56320, 59669, 63218, 66977, 70959, 75179, 79649, 84385, 89403, 94719, 100351, 106319, 112640, 119338, 126435, 133953, 141918, 150357, 159298, 168770, 178805, 189438, 200702, 212637, 225280, 238676, 252869, 267905, 283836, 300713, 318595, 337539, 357610, 378875, 401404, 425273, 450560, 477352, 505737, 535810, 567671, 601426, 637189, 675078, 715220, 757749, 802807, 850545, 901120, 954704, 1011474, 1071619, 1135341, 1202851, 1274377, 1350155, 1430439, 1515498, 1605614, 1701089, 1802240, 1909407, 2022947, 2143237, 2270681, 2405702, 2548753, 2700309, 2860878, 3030995, 3211227
};

Synth_tmp synth_tmp;

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
    GPIO_PinOutSet(gpioPortE, 13);
}

static void transfer_synth(void) {
    ecode = SPIDRV_AbortTransfer(&synth_spi);
    if (ecode != ECODE_EMDRV_SPIDRV_OK && ecode != ECODE_EMDRV_SPIDRV_IDLE) {
        exit(1);
    }
    GPIO_PinOutClear(gpioPortE, 13);
    ecode = SPIDRV_MTransmit(&synth_spi, (void *)&synth, sizeof(synth), complete_synth_transfer);
    if (ecode != ECODE_EMDRV_SPIDRV_OK) exit(1);
}


char next_wavegen = 0;
static void note_on(char note, char velocity) {
    /* TODO: use velocity to adjust envelope? */
    static EnvelopeStep envelope[] = {
        { .rate = 127,   .duration = 255, },
        { .rate = -30, .duration = 100, },
        { .rate = -20, .duration = 255, },
        { .rate = -10, .duration = 255, },
        { .rate = -5, .duration = 255, },
        { .rate = -80,  .duration = 255, },
        { .rate = -80,  .duration = 255, },
        { .rate = -4,   .duration = 100, },
    };

    next_wavegen = (next_wavegen + 1) % SYNTH_WAVEGEN_COUNT;
    char idx = next_wavegen;
    WaveGen *w;

    //if (!queue_get(&wavegen_queue, &idx, 1)) {
    //    return; /* no wavegens available */
    //}

    note_wavegens[note] = idx; /* TODO: what if note is already on? */
    w = &synth.wavegens[idx];
    w->freq = notes[note];

    w->velocity = 50ul * velocity;
    w->cmds = WAVEGEN_CMD_RESET_ENVELOPE | WAVEGEN_CMD_ENABLE;
    w->shape = WAVEGEN_SHAPE_SQUARE;
    
    wavegen_set_vol_envelope(w, envelope, lenof(envelope));
    GPIO_PinOutSet(gpioPortA, 0);
}

static void note_off(char note, char velocity) {
    /* TODO: use velocity to adjust envelope? */
    static EnvelopeStep envelope[] = {
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
        { .rate = -30,   .duration = 255, },
    };

    char idx;
    WaveGen *w;

    idx = note_wavegens[note]; /* TODO: what if note is already off? */
    w = &synth.wavegens[idx];
    wavegen_set_vol_envelope(w, envelope, lenof(envelope));

    w->cmds = WAVEGEN_CMD_ENABLE;

    //if (!queue_put(&wavegen_queue, &idx, 1)) exit(1);
    GPIO_PinOutClear(gpioPortA, 0);
}

int main(void) {
    CHIP_Init();
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0);
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
