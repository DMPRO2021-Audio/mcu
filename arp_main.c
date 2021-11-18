#include <stdint.h>
#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <em_lcd.h>
#include <em_timer.h>
#include <gpiointerrupt.h>

#include "arp_main.h"
#include "synth.h"
#include "midi.h"
#include "arpeggiator.h"
#include "timer.h"

Arpeggiator arpeggiator;
uint16_t counter = 0;

uint8_t old_notes_per_beat = 1;
uint16_t old_BPM = 100;

char current_note;

void run_arpeggiator(void) {
    // GPIO setup
    GPIO_PinModeSet(D6_PORT, D6_PIN, gpioModePushPull, 0);  // TODO: Do this wherever the rest of the LEDs are set up

    // Initialise the arpeggiator itself
    arpeggiator = init_arpeggiator(100, 0, 1, 1, 0.5);

    // Set up interrupts/timers
    float beats_per_second = arpeggiator.BPM / 60.0;
    uint32_t note_timer_top = (uint32_t) (((CLOCK_FREQUENCY / CLOCK_PRESCALER) / TIMER_PRESCALER) / beats_per_second) / arpeggiator.notes_per_beat;
    uint32_t gate_timer_top = (uint32_t) note_timer_top * arpeggiator.gate_time;
    setup_timers(note_timer_top, gate_timer_top);

    start_note_timer();

    while(1) {
        uint8_t byte = uart_next_valid_byte();

        if (byte & MIDI_STATUS_MASK) {
            switch (byte & MIDI_COMMAND_MASK) {
            case MIDI_NOTE_ON:
                {
                    char note = uart_next_valid_byte();
                    char velocity = uart_next_valid_byte();  // Should not be used in arpeggiator

                    add_held_key(&arpeggiator, note);
                }
                break;
            case MIDI_NOTE_OFF:
                {
                    char note = uart_next_valid_byte();
                    char velocity = uart_next_valid_byte();  // Should not be used in arpeggiator

                    remove_held_key(&arpeggiator, note);
                }
                break;
            }
        }
    }
}

void set_timer_tops() {
    float beats_per_second = arpeggiator.BPM / 60.0;

    uint32_t note_timer_top = (uint32_t) (((CLOCK_FREQUENCY / CLOCK_PRESCALER) / TIMER_PRESCALER) / beats_per_second) / arpeggiator.notes_per_beat;
    uint32_t gate_timer_top = (uint32_t) note_timer_top * arpeggiator.gate_time;

    set_note_timer_top(note_timer_top);
    set_gate_timer_top(gate_timer_top);
}

// Start playing note (send single-note synth struct to FPGA, with start instruction)
// (If implemented, tick metronome)
void TIMER0_IRQHandler(void)
{
    // If notes_per_beat has changed
    if (arpeggiator.notes_per_beat != old_notes_per_beat || arpeggiator.BPM != old_BPM) {
        set_timer_tops();

        old_notes_per_beat = arpeggiator.notes_per_beat;
        old_BPM = arpeggiator.BPM;
    }

    // Handles the metronome (currently LED1 toggling)
    // Sort of a hack; this will often cause a jump in the metronome.
    if (arpeggiator.dynamic_NPB_switching) {
        if (arpeggiator.current_note_index == 0) {
            GPIO_PinOutToggle(D6_PORT, D6_PIN);
        }
    }
    else if (arpeggiator.notes_per_beat == 1 || counter % arpeggiator.notes_per_beat-1 == 0) {
        GPIO_PinOutToggle(D6_PORT, D6_PIN);
    }

    current_note = play_current_note(&arpeggiator);

    note_on(current_note, 128);
    transfer_synth();

    counter++;

    start_gate_timer();

    // Clear flag for TIMER0 OF interrupt
    TIMER_IntClear(TIMER0, TIMER_IF_OF);
}

// Stop playing note (send single-note synth struct to FPGA, with stop instruction)
void TIMER1_IRQHandler(void)
{
    char note;  // TODO: convert frequency to note OR store note
    note_off(current_note, 0);
    transfer_synth();

    stop_gate_timer();

    // Clear flag for TIMER1 OF interrupt
    TIMER_IntClear(TIMER1, TIMER_IF_OF);
}
