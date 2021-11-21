#include <gpiointerrupt.h>
#include <em_gpio.h>
#include "leds.h"

void led_init()
{
    GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED2, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED3, gpioModePushPull, 0);
}
