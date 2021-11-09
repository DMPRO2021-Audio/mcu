#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <gpiointerrupt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define LED_PORT gpioPortA
#define LED_PIN0 0
#define LED_PIN1 1
#define LED_PIN2 2
#define LED_PIN3 3

#define BUTTON_PORT gpioPortE
#define BUTTON_PIN0 4
#define BUTTON_PIN1 5
#define BUTTON_PIN2 6
#define BUTTON_PIN3 7

volatile int ms_ticks;
void SysTick_Handler(void)
{
    ms_ticks++;
}

int button_led_map[] = {
    [BUTTON_PIN0] = LED_PIN0,
    [BUTTON_PIN1] = LED_PIN1,
    [BUTTON_PIN2] = LED_PIN2,
    [BUTTON_PIN3] = LED_PIN3,
};

bool led_enabled[] = {
    [LED_PIN0] = false,
    [LED_PIN1] = false,
    [LED_PIN2] = false,
    [LED_PIN3] = false,
};

volatile int last_push_ticks = 0;

void handle_button(uint8_t pin)
{
    int this_push_ticks = ms_ticks;
    if (this_push_ticks - last_push_ticks < 50) {
        last_push_ticks = this_push_ticks;
        return;
    }
    int led = button_led_map[pin];
    if (led_enabled[led])
        GPIO_PinOutClear(LED_PORT, led);
    else
        GPIO_PinOutSet(LED_PORT, led);
    led_enabled[led] = !led_enabled[led];
    last_push_ticks = this_push_ticks;
}

int main(void) {
    CHIP_Init();
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

    GPIOINT_Init();

    GPIO_PinModeSet(LED_PORT, LED_PIN0, gpioModePushPull, 1);
    GPIO_PinModeSet(LED_PORT, LED_PIN1, gpioModePushPull, 1);
    GPIO_PinModeSet(LED_PORT, LED_PIN2, gpioModePushPull, 1);
    GPIO_PinModeSet(LED_PORT, LED_PIN3, gpioModePushPull, 1);

    GPIO_PinOutClear(LED_PORT, LED_PIN0);
    GPIO_PinOutClear(LED_PORT, LED_PIN1);
    GPIO_PinOutClear(LED_PORT, LED_PIN2);
    GPIO_PinOutClear(LED_PORT, LED_PIN3);

    GPIO_PinModeSet(BUTTON_PORT, BUTTON_PIN0, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT, BUTTON_PIN1, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT, BUTTON_PIN2, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT, BUTTON_PIN3, gpioModeInput, 0);
    GPIOINT_CallbackRegister(BUTTON_PIN0, handle_button);
    GPIOINT_CallbackRegister(BUTTON_PIN1, handle_button);
    GPIOINT_CallbackRegister(BUTTON_PIN2, handle_button);
    GPIOINT_CallbackRegister(BUTTON_PIN3, handle_button);
    GPIO_IntConfig(BUTTON_PORT, BUTTON_PIN0, false, true, true);
    GPIO_IntConfig(BUTTON_PORT, BUTTON_PIN1, false, true, true);
    GPIO_IntConfig(BUTTON_PORT, BUTTON_PIN2, false, true, true);
    GPIO_IntConfig(BUTTON_PORT, BUTTON_PIN3, false, true, true);

    while (1) {
        __WFI();
    }
}

void _exit(int status) {
    (void) status;
    while (1) {}
}
