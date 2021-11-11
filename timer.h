#include <stdint.h>

void setup_timers(uint16_t note_timer_top, uint16_t gate_timer_top);
void start_note_timer();
void stop_note_timer();
void start_gate_timer();
void stop_gate_timer();
void set_note_timer_top(uint16_t timer_top);
void set_gate_timer_top(uint16_t timer_top);