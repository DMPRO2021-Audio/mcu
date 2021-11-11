#include <stdint.h>
#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <em_lcd.h>
#include <em_timer.h>
#include <gpiointerrupt.h>

// STK3700-specific includes
#include <segmentlcd.h>

#include "arpeggiator.h"
#include "timer.h"
#include "notes.h"

// TEMPORARY: These are probably defined elsewhere
#define CLOCK_FREQUENCY 14000000
#define CLOCK_PRESCALER 8
#define TIMER_PRESCALER 1024


Arpeggiator test_arpeggiator;
uint16_t counter = 0;

uint8_t old_notes_per_beat = 1;

int main(int argc, char const *argv[])
{
    CHIP_Init();

    // GPIO setup
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 0);

    SegmentLCD_Init(false);
    // SegmentLCD_Write("Hello");

    // Create an arpeggiator for testing
    test_arpeggiator = init_arpeggiator(30, 0, 1, 1, 0.5);
    add_held_key(&test_arpeggiator, NOTE_C4);
    add_held_key(&test_arpeggiator, NOTE_E4);
    add_held_key(&test_arpeggiator, NOTE_G4);
    add_held_key(&test_arpeggiator, NOTE_B4);

    // Set up interrupts/timers
    uint32_t note_timer_top = 1709;  // 1709 gives ~1 s period using 14 MHz oscillator with div_8 and timer with div_1024
    uint32_t gate_timer_top = (uint32_t) note_timer_top * test_arpeggiator.gate_time;
    setup_timers(note_timer_top, gate_timer_top);

    start_note_timer();

    while(1) {
        __WFI();
    }

    return 0;
}

void set_timer_tops() {
    float beats_per_second = test_arpeggiator.BPM / 60.0;

    uint32_t note_timer_top = (uint32_t) (((CLOCK_FREQUENCY / CLOCK_PRESCALER) / TIMER_PRESCALER) / beats_per_second) / test_arpeggiator.notes_per_beat;
    uint32_t gate_timer_top = (uint32_t) note_timer_top * test_arpeggiator.gate_time;

    set_note_timer_top(note_timer_top);
    set_gate_timer_top(gate_timer_top);
}

void BTN0_IRQHandler(void)
{
    uint32_t notes_to_add[8] = {NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7};
}

void BTN1_IRQHandler(void)
{
    // Flip between 3 and 1 octaves

}

// Start playing note (send single-note synth struct to FPGA, with start instruction)
// (If implemented, tick metronome)
void TIMER0_IRQHandler(void)
{
    // If notes_per_beat has changed
    if (test_arpeggiator.notes_per_beat != old_notes_per_beat) {
        set_timer_tops();

        old_notes_per_beat = test_arpeggiator.notes_per_beat;
    }

    // Handles the metronome (currently LED1 toggling)
    // Sort of a hack; this will often cause a jump in the metronome.
    if (test_arpeggiator.dynamic_NPB_switching) {
        if (test_arpeggiator.current_note_index == 0) {
            GPIO_PinOutToggle(gpioPortE, 3);
        }
    }
    else if (test_arpeggiator.notes_per_beat == 1 || counter % test_arpeggiator.notes_per_beat-1 == 0) {
        GPIO_PinOutToggle(gpioPortE, 3);
    }

    // Clear flag for TIMER0 OF interrupt
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

    start_gate_timer();
    
    // Display the note index to be played
    SegmentLCD_Number(test_arpeggiator.current_note_index);

    uint32_t current_note = play_current_note(&test_arpeggiator);
    SegmentLCD_LowerNumber(current_note);

    // if (counter == 5) {
    //     add_held_key(&test_arpeggiator, NOTE_D4);
    // }
    // if (counter == 10) {
    //     remove_held_key(&test_arpeggiator, NOTE_E4);
    // }
    // if (counter == 7) {
    //     add_held_key(&test_arpeggiator, NOTE_F2);
    // }
    // if (counter == 15) {
    //     change_num_octaves(&test_arpeggiator, 3);
    // }
    // if (counter == 12) {
    //     change_playback_order(&test_arpeggiator, 2);
    // }
    if (counter == 6) {
        test_arpeggiator.dynamic_NPB_switching = true;
        test_arpeggiator.notes_per_beat = test_arpeggiator.loop_length;
    }
    // if (counter == 16) {
    //     remove_held_key(&test_arpeggiator, NOTE_F2);
    // }
    
    // SegmentLCD_Number(counter % test_arpeggiator.loop_length);
    // SegmentLCD_LowerNumber(test_arpeggiator.loop_length);
    // SegmentLCD_LowerNumber(test_arpeggiator.current_note_index);
    // SegmentLCD_LowerNumber(test_arpeggiator.arp_loop[0]);

    // Toggle LEDs 0 and 1
    GPIO_PinOutToggle(gpioPortE, 2);

    counter++;
}

// Stop playing note (send single-note synth struct to FPGA, with stop instruction)
void TIMER1_IRQHandler(void)
{
    // Clear flag for TIMER1 OF interrupt
    TIMER_IntClear(TIMER1, TIMER_IF_OF);

    GPIO_PinOutToggle(gpioPortE, 2);

    SegmentLCD_Write("");

    stop_gate_timer();
}
