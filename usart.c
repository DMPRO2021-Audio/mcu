#include <stdint.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include "buffer.h"
#include "clock_efm32gg_ext.h"
#include <em_usart.h>

#define BUFFERSIZE 256
#define INTERRUPTLVL 6

const uint32_t BAUD = 115200;
const uint32_t OVS = 16;

unsigned rxBufferArea[(sizeof(struct buffer_s) + BUFFERSIZE + sizeof(unsigned)-1)/sizeof(unsigned)];
buffer rxBuffer = 0;

void UART_Init(void) {
	uint32_t bauddiv;

	USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART1, true);
	GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 1);
	USART_InitAsync(USART1, &init);

        bauddiv = (ClockGetPeripheralClockFrequency()*4)/(OVS*BAUD)-4;
        USART1->CLKDIV = bauddiv<<_USART_CLKDIV_DIV_SHIFT;
	USART1->ROUTE = USART_ROUTE_LOCATION_LOC1 | USART_ROUTE_RXPEN;
	rxBuffer = bufferInit(rxBufferArea, BUFFERSIZE);

	USART_IntEnable(USART1, USART_IEN_RXDATAV);

        NVIC_SetPriority(USART1_RX_IRQn, INTERRUPTLVL);

        NVIC_ClearPendingIRQ(USART1_RX_IRQn);
        NVIC_EnableIRQ(USART1_RX_IRQn);

}


void USART1_RX_IRQHandler(void) {
	uint8_t ch;
	if(USART1->IF & (USART_IF_RXDATAV | USART_IF_RXFULL)) {
		ch = USART1->RXDATA;
		(void) bufferInsert(rxBuffer, ch);
	}
}


unsigned int UART_GetChar(void) {
	int ch;

	if(bufferEmpty(rxBuffer)) {
		return 0;
	}

	__disable_irq();
	ch = bufferRemove(rxBuffer);
	__enable_irq();
	return ch;
}

