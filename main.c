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
#include "buttons.h"

typedef void CommandHandler(char status);
typedef void ControlHandler(char ctrl, char value);

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

/* holds released wavegen indices */
Queue wavegen_queue = {
    .capacity = SYNTH_WAVEGEN_COUNT,
    .buf = {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, /* TODO */
};

char note_wavegens[128]; /* maps active MIDI notes to index of wavegen */
char wavegen_notes[SYNTH_WAVEGEN_COUNT];
double pitch_bend = 1.0;
char controls[128];

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

static void abort_synth_transfer(void) {
    ecode = SPIDRV_AbortTransfer(&synth_spi);
    if (ecode != ECODE_EMDRV_SPIDRV_OK && ecode != ECODE_EMDRV_SPIDRV_IDLE) {
        exit(1);
    }
}

static void start_synth_transfer(void) {
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
        { .rate = -10, .duration = 255, },
        { .rate = -7, .duration = 255, },
        { .rate = -5, .duration = 255, },
        { .rate = -4,  .duration = 255, },
        { .rate = -4,  .duration = 255, },
        { .rate = -3,   .duration = 255, },
    };

    next_wavegen = (next_wavegen + 1) % SYNTH_WAVEGEN_COUNT;
    char idx = next_wavegen;
    WaveGen *w;

    //if (!queue_get(&wavegen_queue, &idx, 1)) {
    //    return; /* no wavegens available */
    //}

    note_wavegens[note] = idx; /* TODO: what if note is already on? */
    wavegen_notes[idx] = note;

    w = &synth.wavegens[idx];
    w->freq = notes[note] * pitch_bend;
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
    w->cmds = WAVEGEN_CMD_ENABLE;
    if (!controls[MIDI_CC_SUSTAIN_KEY]) {
        wavegen_set_vol_envelope(w, envelope, lenof(envelope));
    }

    //if (!queue_put(&wavegen_queue, &idx, 1)) exit(1);
    GPIO_PinOutClear(gpioPortA, 0);
}

static void reset_wavegens(void) {
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

    for (WaveGen *w = synth.wavegens; w < endof(synth.wavegens); w++) {
        w->velocity = 0;
        w->cmds = WAVEGEN_CMD_RESET_ENVELOPE;
        wavegen_set_vol_envelope(w, envelope, lenof(envelope));
    }
}

static void handle_note_off(char status) {
    char key = uart_next_valid_byte();
    char velocity = uart_next_valid_byte();

    note_off(key, velocity);
}

static void handle_note_on(char status) {
    char key = uart_next_valid_byte();
    char velocity = uart_next_valid_byte();

    note_on(key, velocity);
}

static void handle_control_change(char status) {
    char ctrl = uart_next_valid_byte();
    char value = uart_next_valid_byte();

    switch (ctrl) {
    case MIDI_CC_ALL_SOUND_OFF:
        reset_wavegens();
        break;
    case MIDI_CC_RESET_ALL_CONTROLLERS:
        //reset_controllers();
        break;
    case MIDI_CC_ALL_NOTES_OFF:
    case MIDI_CC_OMNI_MODE_OFF:
    case MIDI_CC_OMNI_MODE_ON:
    case MIDI_CC_MONO_MODE_ON:
    case MIDI_CC_MONO_MODE_OFF:
        //all_notes_off();
        break;
    default:
        controls[ctrl] = value;
    }
}

static void handle_pitch_bend_change(char status) {
    char lsb = uart_next_valid_byte();
    char msb = uart_next_valid_byte();
    short value = (msb << 7) | lsb;

    pitch_bend = 1.0 + (.059463 * ((value - 16384) / 8192.0));

    for (int i = 0; i < SYNTH_WAVEGEN_COUNT; i++) {
        synth.wavegens[i].freq = notes[wavegen_notes[i]] * pitch_bend;
    }
}

int main(void) {
    static CommandHandler *const command_handlers[] = {
        [MIDI_NOTE_OFF]          = handle_note_off,
        [MIDI_NOTE_ON]           = handle_note_on,
        [MIDI_CONTROL_CHANGE]    = handle_control_change,
        [MIDI_PITCH_BEND_CHANGE] = handle_pitch_bend_change,
    };

    CHIP_Init();
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1);
    SPIDRV_Init(&synth_spi, &synth_spi_init);

    uart_init();
    button_init();
    __enable_irq();

    while (1) {
        uint8_t byte = uart_next_valid_byte();
        int idx;
        CommandHandler *handler;

        if (!(byte & MIDI_STATUS_MASK)) {
            warn();
            continue;
        }

        idx = (byte & MIDI_COMMAND_MASK) >> MIDI_COMMAND_SHIFT;
        if (idx > lenof(command_handlers)) continue;
        handler = command_handlers[idx];
        if (!handler) continue;

        abort_synth_transfer();
        handler(byte & ~MIDI_STATUS_MASK);
        start_synth_transfer();
    }
}

void _exit(int status) {
    (void) status;
    while (1) {}
}
