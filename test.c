#include <bsp.h>
#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <gpiointerrupt.h>
#include <spidrv.h>
#include <stdbool.h>
#include <stdint.h>

volatile uint8_t counter;

static void handle_button(uint8_t pin) {
	switch (pin) {
	case 9:
		BSP_LedToggle(1);
		counter++;
		break;
	case 10:
		BSP_LedToggle(0);
		counter--;
		break;
	}
}

int main(void) {
	static SPIDRV_Init_t spi_init = SPIDRV_MASTER_USART2;
	SPIDRV_Handle_t spi;

	CHIP_Init();
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIOINT_Init();
	GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
	GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);
	GPIOINT_CallbackRegister(9, handle_button);
	GPIOINT_CallbackRegister(10, handle_button);
	GPIO_IntConfig(gpioPortB, 9, false, true, true);
	GPIO_IntConfig(gpioPortB, 10, false, true, true);
	BSP_LedsInit();
	while (1) {
		uint8_t buf = counter;

		SPIDRV_Init(spi, &spi_init);
		SPIDRV_MTransmitB(spi, &buf, 1);
		__WFI();
	}
}

void _exit(int status) {
	(void) status;
	while (1) {}
}
