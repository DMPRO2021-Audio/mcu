#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <em_timer.h>
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
#include "button.h"
#include "arpeggiator.h"
#include "timer.h"
#include "leds.h"

/* For arpeggiator */
Arpeggiator arpeggiator;
uint16_t counter = 0;

uint16_t old_BPM = START_BPM;
uint8_t old_notes_per_beat = START_NPB;
float old_gate_time = START_GATE_TIME;

char current_note;

bool arpeggiator_on = false;
/* For arpeggiator */

void start_arpeggiator() {
    start_note_timer();
}

void stop_arpeggiator() {
    stop_note_timer();
    stop_gate_timer();
}
/* For arpeggiator */

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
            F2FP(0.5)
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
int current_shape = WAVEGEN_SHAPE_SAWTOOTH;
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

    next_wavegen = (next_wavegen + 1) % (SYNTH_WAVEGEN_COUNT-1);
    char idx = next_wavegen;
    WaveGen *w;

    //if (!queue_get(&wavegen_queue, &idx, 1)) {
    //    return; /* no wavegens available */
    //}

    note_wavegens[(size_t) note] = idx; /* TODO: what if note is already on? */
    wavegen_notes[(size_t) idx] = note;

    w = &synth.wavegens[(size_t) idx];
    w->freq = notes[(size_t) note] * pitch_bend;
    w->velocity = 50ul * velocity;
    w->cmds = WAVEGEN_CMD_RESET_ENVELOPE | WAVEGEN_CMD_ENABLE;
    w->shape = current_shape;
    wavegen_set_vol_envelope(w, envelope, lenof(envelope));
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

    idx = note_wavegens[(size_t) note]; /* TODO: what if note is already off? */

    w = &synth.wavegens[(size_t) idx];
    w->cmds = WAVEGEN_CMD_ENABLE;
    if (!controls[MIDI_CC_SUSTAIN_KEY]) {
        wavegen_set_vol_envelope(w, envelope, lenof(envelope));
    }

    //if (!queue_put(&wavegen_queue, &idx, 1)) exit(1);
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
        controls[(size_t) ctrl] = value;
    }
}

static void handle_pitch_bend_change(char status) {
    char lsb = uart_next_valid_byte();
    char msb = uart_next_valid_byte();
    short value = (msb << 7) | lsb;

    pitch_bend = 1.0 + (.059463 * ((value - 16384) / 8192.0));

    for (int i = 0; i < SYNTH_WAVEGEN_COUNT; i++) {
        synth.wavegens[i].freq = notes[(size_t) wavegen_notes[i]] * pitch_bend;
    }
}

void handle_button(uint8_t pin) {
    switch(pin) {
    case BUTTON_SW1:
        /* Change shape */
        current_shape = (current_shape+1) % NUM_WAVEGEN_SHAPES;
        break;
    case BUTTON_SW2:
    case BUTTON_SW3:
    case BUTTON_SW4:
        break;
    case BUTTON_SW5:
        /* Toggle arpeggiator */
        if (!arpeggiator_on) {
            arpeggiator_on = true;
            GPIO_PinOutSet(LED_PORT, LED1);
            start_arpeggiator();
        }
        else {
            arpeggiator_on = false;
            GPIO_PinOutClear(LED_PORT, LED1);
            GPIO_PinOutClear(LED_PORT, LED3);
            stop_arpeggiator();
        }
        break;

    default:
        break;
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

    GPIOINT_Init();
    button_init();

    uart_init();
    __enable_irq();

    led_init();

    arpeggiator = setup_arpeggiator();

    // GPIO_PinOutToggle(LED_PORT, LED3);

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

        /* Hijacks loop if arpeggiator is on, and a key is pressed or released */
        if (arpeggiator_on && (idx == MIDI_NOTE_OFF || idx == MIDI_NOTE_ON)) {
            char note = uart_next_valid_byte();
            // char velocity = uart_next_valid_byte();  // Remains unused in arpeggiator
            (void) uart_next_valid_byte();
            if (idx == MIDI_NOTE_ON) {
                add_held_key(&arpeggiator, note);
                continue;
            }
            if (idx == MIDI_NOTE_OFF) {
                remove_held_key(&arpeggiator, note);
                continue;
            }
        }
        /* ------------------------------------------------------------------- */

        handler = command_handlers[(size_t) idx];
        if (!handler) continue;

        abort_synth_transfer();
        handler(byte & ~MIDI_STATUS_MASK);
        start_synth_transfer();
    }
}

// Start playing note (send single-note synth struct to FPGA, with start instruction)
// (If implemented, tick metronome)
void TIMER0_IRQHandler(void)
{
    // Clear flag for TIMER0 OF interrupt
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

    // If notes_per_beat, BPM or gate_time has changed
    if (arpeggiator.notes_per_beat != old_notes_per_beat || arpeggiator.BPM != old_BPM || arpeggiator.gate_time != old_gate_time) {
        set_timer_tops(arpeggiator);

        old_notes_per_beat = arpeggiator.notes_per_beat;
        old_BPM = arpeggiator.BPM;
        old_gate_time = arpeggiator.gate_time;
    }

    // Handles the metronome (currently LED3 toggling)
    // Sort of a hack; this will usually cause a jump in the metronome whenever dynamic_NPB_switching is toggled
    if (arpeggiator.dynamic_NPB_switching) {
        if (arpeggiator.current_note_index == 0) {
            GPIO_PinOutToggle(LED_PORT, LED3);
        }
    }
    else if (arpeggiator.notes_per_beat == 1 || counter % arpeggiator.notes_per_beat-1 == 0) {
        GPIO_PinOutToggle(LED_PORT, LED3);
    }

    current_note = play_current_note(&arpeggiator);

    if (arpeggiator.loop_length == 0) {
        return;
    }

    abort_synth_transfer();
    note_on(current_note, 128);
    start_synth_transfer();

    counter++;

    start_gate_timer();
}

// Stop playing note (send single-note synth struct to FPGA, with stop instruction)
void TIMER1_IRQHandler(void)
{
    // Clear flag for TIMER1 OF interrupt
    TIMER_IntClear(TIMER1, TIMER_IF_OF);

    abort_synth_transfer();
    // NB: Assumes current_note has not changed since last TIMER0 interrupt.
    // Will not hold if gate_time is e.g. greater than 1
    note_off(current_note, 0);
    start_synth_transfer();

    stop_gate_timer();
}

void _exit(int status) {
    (void) status;
    while (1) {}
}
