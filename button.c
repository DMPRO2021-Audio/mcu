#include "button.h"

void button_init(void)
{
    GPIO_PinModeSet(BUTTON_PORT1, BUTTON_SW1, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT1, BUTTON_SW2, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT1, BUTTON_SW3, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT1, BUTTON_SW4, gpioModeInput, 0);
    GPIO_PinModeSet(BUTTON_PORT2, BUTTON_SW5, gpioModeInput, 0);

    GPIOINT_CallbackRegister(BUTTON_SW1, handle_button);
    GPIOINT_CallbackRegister(BUTTON_SW2, handle_button);
    GPIOINT_CallbackRegister(BUTTON_SW3, handle_button);
    GPIOINT_CallbackRegister(BUTTON_SW4, handle_button);
    GPIOINT_CallbackRegister(BUTTON_SW5, handle_button);

    GPIO_IntConfig(BUTTON_PORT1, BUTTON_SW1, false, true, true);
    GPIO_IntConfig(BUTTON_PORT1, BUTTON_SW2, false, true, true);
    GPIO_IntConfig(BUTTON_PORT1, BUTTON_SW3, false, true, true);
    GPIO_IntConfig(BUTTON_PORT1, BUTTON_SW4, false, true, true);
    GPIO_IntConfig(BUTTON_PORT2, BUTTON_SW5, false, true, true);
}
