#include <gpiointerrupt.h>
#include <em_gpio.h>
#include "leds.h"

void led_init()
{
    GPIO_PinModeSet(LED_PORT, LED3, gpioModeInput, 1);
}
