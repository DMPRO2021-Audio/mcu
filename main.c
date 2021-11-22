#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <em_timer.h>
#include <math.h>
#include <spidrv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "midi.h"
#include "synth.h"
#include "sound.h"
#include "channel.h"
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

volatile char current_note;

volatile bool event_flag = true;
volatile bool arpeggiator_note_off_flag, arpeggiator_note_on_flag;

int current_reverb_preset;

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
            F2FP(0)
        },
    },
};

Channel channels[16];
Channel *arp_channel = &channels[9];

SPIDRV_HandleData_t synth_spi;

SPIDRV_Init_t synth_spi_init = {
    .port = USART0,
    .portLocationTx = _USART_ROUTELOC0_TXLOC_LOC0,
    .portLocationRx = _USART_ROUTELOC0_RXLOC_LOC0,
    .portLocationClk = _USART_ROUTELOC0_CLKLOC_LOC0,
    .bitRate = 10000000,
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
    GPIO_PinOutClear(LED_PORT, LED0);
}

static void abort_synth_transfer(void) {
    ecode = SPIDRV_AbortTransfer(&synth_spi);
    if (ecode != ECODE_EMDRV_SPIDRV_OK && ecode != ECODE_EMDRV_SPIDRV_IDLE) {
        exit(1);
    }
}

static void start_synth_transfer(void) {
    GPIO_PinOutSet(LED_PORT, LED0);
    GPIO_PinOutClear(gpioPortE, 13);
    ecode = SPIDRV_MTransmit(&synth_spi, (void *)&synth, sizeof(synth), complete_synth_transfer);
    if (ecode != ECODE_EMDRV_SPIDRV_OK) exit(1);
}

static void handle_note_off(char status) {
    Channel *c = &channels[status & MIDI_CHANNEL_MASK];
    char key = uart_next_valid_byte();
    char velocity = uart_next_valid_byte();

    if (c == arp_channel) {
        remove_held_key(&arpeggiator, key);
    }
    channel_note_off(c, key, velocity / 127.0);
}

static void handle_note_on(char status) {
    Channel *c = &channels[status & MIDI_CHANNEL_MASK];
    char key = uart_next_valid_byte();
    char velocity = uart_next_valid_byte();

    if (c == arp_channel) {
        add_held_key(&arpeggiator, key);
    } else {
        channel_note_on(c, key, velocity / 127.0);
    }
}

static void handle_control_change(char status) {
    Channel *c = &channels[status & MIDI_CHANNEL_MASK];
    char ctrl = uart_next_valid_byte();
    char value = uart_next_valid_byte();

    switch (ctrl) {
    case MIDI_CC_MODULATION_WHEEL:
    case MIDI_CC_VOLUME:
        c->gain = value / 127.0;
        break;
    case MIDI_CC_SUSTAIN_KEY:
    case MIDI_CC_SUSTAIN_PEDAL:
        c->sustain = !value;
        break;
    case MIDI_CC_ALL_SOUND_OFF:
        channel_all_notes_off(c, true);
        return;
    case MIDI_CC_RESET_ALL_CONTROLLERS:
        c->gain = 0.5;
        c->pitch_bend = 0.0;
        c->sustain = false;
        break;
    case MIDI_CC_ALL_NOTES_OFF:
    case MIDI_CC_OMNI_MODE_OFF:
    case MIDI_CC_OMNI_MODE_ON:
    case MIDI_CC_MONO_MODE_ON:
    case MIDI_CC_MONO_MODE_OFF:
        channel_all_notes_off(c, false);
        return;
    }
    channel_update_wavegens(c);
}

static void handle_program_change(char status) {
    Channel *c = &channels[status & MIDI_CHANNEL_MASK];
    char program = uart_next_valid_byte();

    if (program >= num_programs) return;
    c->program = &programs[(int)program];
    channel_update_wavegens(c);
}

static void handle_pitch_bend_change(char status) {
    Channel *c = &channels[status & MIDI_CHANNEL_MASK];
    char lsb = uart_next_valid_byte();
    char msb = uart_next_valid_byte();

    c->pitch_bend = (((msb << 7) | lsb) - 0x2000) / (float)0x2000;
    channel_update_wavegens(c);
}

void handle_button(uint8_t pin) {
    switch(pin) {
    case BUTTON_SW1:
        if (arpeggiator_on) {
            set_gate_time(&arpeggiator, arpeggiator.gate_time - 0.1);
        }
        else {
            // Handle reverb preset browsing
        }
        break;
    case BUTTON_SW2:
        if (arpeggiator_on) {
            set_gate_time(&arpeggiator, arpeggiator.gate_time + 0.1);
        }
        else {
            if (arpeggiator.dynamic_NPB_switching) {
                arpeggiator.dynamic_NPB_switching = false;
                set_notes_per_beat(&arpeggiator, 4);
            }
            else {
                arpeggiator.dynamic_NPB_switching = true;
            }
        }
        break;
    case BUTTON_SW3:
        if (arpeggiator_on) {
            set_BPM(&arpeggiator, arpeggiator.BPM - 10);
        }
        else {
            // Browse playback order
            set_playback_order(&arpeggiator, (arpeggiator.playback_order + 1) % 4);
        }
        break;
    case BUTTON_SW4:
        if (arpeggiator_on) {
            set_BPM(&arpeggiator, arpeggiator.BPM + 10);
        }
        else {
            // Funky modulo because num_octaves ranges between 1-3, not 0-2
            set_num_octaves(&arpeggiator, (arpeggiator.num_octaves % 3)+1);
        }
        break;
    case BUTTON_SW5:
        /* Toggle arpeggiator */
        if (!arpeggiator_on) {
            arpeggiator_on = true;
            GPIO_PinOutSet(LED_PORT, LED1);
        }
        else {
            arpeggiator_on = false;
            GPIO_PinOutClear(LED_PORT, LED1);
            GPIO_PinOutClear(LED_PORT, LED3);
        }
        break;
    default:
        break;
    }
}

void update_arpeggiator(void) {
    if (arpeggiator_note_off_flag) {
        arpeggiator_note_off_flag = false;
        channel_note_off(arp_channel, current_note, 0);
    }
    if (arpeggiator_note_on_flag) {
        arpeggiator_note_on_flag = false;

        // If notes_per_beat, BPM or gate_time has changed
        if (arpeggiator.notes_per_beat != old_notes_per_beat || arpeggiator.BPM != old_BPM || arpeggiator.gate_time != old_gate_time) {
            set_timer_tops(arpeggiator);

            old_notes_per_beat = arpeggiator.notes_per_beat;
            old_BPM = arpeggiator.BPM;
            old_gate_time = arpeggiator.gate_time;
        }

        current_note = play_current_note(&arpeggiator);

        counter++;
        if (arpeggiator.loop_length == 0 || current_note == 0) {
            return;
        }
        channel_note_on(arp_channel, current_note, 1.0);
    }
}

void recv_midi_command(void) {
    static CommandHandler *const command_handlers[] = {
        [MIDI_NOTE_OFF]          = handle_note_off,
        [MIDI_NOTE_ON]           = handle_note_on,
        [MIDI_CONTROL_CHANGE]    = handle_control_change,
        [MIDI_PROGRAM_CHANGE]    = handle_program_change,
        [MIDI_PITCH_BEND_CHANGE] = handle_pitch_bend_change,
    };

    uint8_t byte = uart_next_byte();
    int idx;
    CommandHandler *handler;

    if (byte == 0xFF) return;
    if (!(byte & MIDI_STATUS_MASK)) {
        warn();
        return;
    }

    idx = (byte & MIDI_COMMAND_MASK) >> MIDI_COMMAND_SHIFT;
    if (idx >= lenof(command_handlers)) return;
    handler = command_handlers[(int)idx];
    if (!handler) return;
    handler(byte & ~MIDI_STATUS_MASK);
}

int main(void) {
    for (int i = 0; i < num_programs && i < lenof(channels); i++) {
        Channel *c = &channels[i];

        c->program = &programs[i];
        c->gain = 0.5;
    }

    CHIP_Init();
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 1);
    SPIDRV_Init(&synth_spi, &synth_spi_init);
    GPIOINT_Init();
    button_init();

    reset_channels();
    uart_init();
    __enable_irq();

    led_init();

    arpeggiator = setup_arpeggiator();
    start_arpeggiator();

    while (1) {
        while (!event_flag) __WFI();
        event_flag = false;
        abort_synth_transfer();

        update_arpeggiator();
        recv_midi_command();

        start_synth_transfer();
    }
}

// Start playing note (send single-note synth struct to FPGA, with start instruction)
// (If implemented, tick metronome)
void TIMER0_IRQHandler(void)
{
    
    // Clear flag for TIMER0 OF interrupt
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

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
    
    if (arpeggiator.loop_length == 0) return;
    event_flag = arpeggiator_note_on_flag = true;
    start_gate_timer();
}

// Stop playing note (send single-note synth struct to FPGA, with stop instruction)
void TIMER1_IRQHandler(void)
{
    // Clear flag for TIMER1 OF interrupt
    TIMER_IntClear(TIMER1, TIMER_IF_OF);
    event_flag = arpeggiator_note_off_flag = true;
    // NB: Assumes current_note has not changed since last TIMER0 interrupt.
    // Will not hold if gate_time is e.g. greater than 1

    stop_gate_timer();
}

void _exit(int status) {
    (void) status;
    while (1) {}
}
