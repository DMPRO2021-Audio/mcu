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

#include <segmentlcd.h>
#include <em_dbg.h>

#define COUNTER_MAX 16
static uint8_t counter = 0;

static void handle_button(uint8_t pin) {
	switch (pin) {
	case 9:
		//BSP_LedToggle(1);
		counter = (counter + 1) % COUNTER_MAX;
		break;
	case 10:
		//BSP_LedToggle(0);
		counter = (counter - 1) % COUNTER_MAX;
		break;
	}
}

static SPIDRV_HandleData_t spi_handle_data;
static SPIDRV_Handle_t spi_handle = &spi_handle_data;

int main(void) {
	Ecode_t ret[2];
	static SPIDRV_Init_t spi_init = SPIDRV_MASTER_USART1;
	spi_init.bitOrder = spidrvBitOrderLsbFirst; // SPI peripheral implements LSB first

	SegmentLCD_Init(true);
	SegmentLCD_LowerNumber(5);


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


	ret[0] = SPIDRV_Init(spi_handle, &spi_init);
	SegmentLCD_UnsignedHex(ret[0]);
	while (1) {
		uint8_t buf = counter;

		ret[1] = SPIDRV_MTransmitB(spi_handle, &buf, 1);

		SegmentLCD_LowerHex(counter);
		while (buf == counter) __WFI();
	}
}

void _exit(int status) {
	(void) status;
	while (1) {}
}
