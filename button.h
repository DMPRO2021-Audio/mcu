#ifndef BUTTONH
#define BUTTONH

#include <gpiointerrupt.h>
#include <em_gpio.h>

#define BUTTON_PORT1 gpioPortE
#define BUTTON_PORT2 gpioPortF

// SW1-4 are on BUTTON_PORT1
#define BUTTON_SW1 4
#define BUTTON_SW2 5
#define BUTTON_SW3 6
#define BUTTON_SW4 7

// SW5 is on BUTTON_PORT1
#define BUTTON_SW5 10

void handle_button(uint8_t pin);
void button_init(void);

#endif
