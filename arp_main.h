/* VVVVVV TODO: Move macros to wherever these _should_ be defined VVVVVV */

// Buttons SW1--SW4: Port E, Pins 4--7
// Button SW5: Port F, Pin 10
#define SW5_PORT gpioPortF
#define SW5_PIN 10

// LEDs D3--D6: Port A, Pins 0--3
#define D6_PORT gpioPortA
#define D6_PIN 3

void run_arpeggiator(void);
