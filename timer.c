#include <stdint.h>
#include <stdbool.h>
#include <em_cmu.h>
#include <em_timer.h>
#include <em_core.h>

#include "timer.h"

// TODO: Automate using emlib
#define CLOCK_FREQUENCY 19000000
#define CLOCK_PRESCALER 8
#define TIMER_PRESCALER 1024

#define HFPER_CLOCK_PRESCALE cmuClkDiv_8
#define NOTE_TIMER_PRESCALE timerPrescale1024
#define GATE_TIMER_PRESCALE timerPrescale1024

void setup_timers(uint16_t note_timer_top, uint16_t gate_timer_top)
{
	// Prescale the HFPERCLK 
	CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_8);

	/* ----- Timer setup ----- */
	// Enable clock for timer0 and timer1
	CMU_ClockEnable(cmuClock_TIMER0, true);
	CMU_ClockEnable(cmuClock_TIMER1, true);

	// Define initial settings for timers, including syncing gate_timer with note_timer enable
	TIMER_Init_TypeDef note_timer_init = TIMER_INIT_DEFAULT;
	note_timer_init.enable = false;
	note_timer_init.prescale = NOTE_TIMER_PRESCALE;

	TIMER_Init_TypeDef gate_timer_init = TIMER_INIT_DEFAULT;
	gate_timer_init.enable = false;
	gate_timer_init.prescale = GATE_TIMER_PRESCALE;
	// gate_timer_init.sync = false;

	// Set timer tops
	TIMER_TopSet(TIMER0, note_timer_top);
	TIMER_TopSet(TIMER1, gate_timer_top);

	// Initialise timers with settings supplied above
	TIMER_Init(TIMER0, &note_timer_init);
	TIMER_Init(TIMER1, &gate_timer_init);

	// Enable TIMER0 and TIMER1 interrupt vectors in NVIC
	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_EnableIRQ(TIMER1_IRQn);

	// Enable overflow interrupt
	TIMER_IntEnable(TIMER0, TIMER_IF_OF);
	TIMER_IntEnable(TIMER1, TIMER_IF_OF);
}


void start_note_timer()
{
	// Start the timer
	TIMER_Enable(TIMER0, true);
	NVIC_SetPriority(TIMER0_IRQn, 1);
}

void stop_note_timer()
{
	// Stop the timer
	TIMER_Enable(TIMER0, false);
}


void start_gate_timer()
{
	// Start the timer
	TIMER_Enable(TIMER1, true);

	// TIMER_IntEnable(TIMER1, TIMER_IF_OF);
}

void stop_gate_timer()
{
	// Stop the timer
	TIMER_Enable(TIMER1, false);

	// TIMER_IntEnable(TIMER0, TIMER_IF_OF);
}


// Sets the top of the timer; should be executed whenever BPM is changed
void set_note_timer_top(uint16_t timer_top)
{
    TIMER_TopSet(TIMER0, timer_top);
}

void set_gate_timer_top(uint16_t timer_top) {
    TIMER_TopSet(TIMER1, timer_top);
}
