#include <stdint.h>
#include <em_device.h>
#include "buffer.h"
#include "clock_efm32gg_ext.h"

#define BUFFERSIZE 256
#define INTERRUPTLVL 6

const uint32_t BAUD = 115200;
const uint32_t OVS = 16;

static GPIO_P_TypeDef * const GPIOE = &(GPIO->P[4]); // RX/TX Port
static GPIO_P_TypeDef * const GPIOF = &(GPIO->P[5]); // Transceiver

unsigned rxBufferArea[(sizeof(struct buffer_s) + BUFFERSIZE + sizeof(unsigned)-1)/sizeof(unsigned)];

buffer rxBuffer = 0;

void UART_Init(void) {
	uint32_t bauddiv;

	CMU->HFPERCLKDIV |= CMU_HFPERCLKDIV_HFPERCLKEN;
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;

	GPIOE->MODEL &= ~_GPIO_P_MODEL_MODE0_MASK;
    	GPIOE->MODEL |= GPIO_P_MODEL_MODE0_PUSHPULL;
    	GPIOE->DOUT  |= (1U <<(0));

	GPIOE->MODEL &= ~_GPIO_P_MODEL_MODE1_MASK;
        GPIOE->MODEL |= GPIO_P_MODEL_MODE1_INPUT;
        GPIOE->DOUT  |= (1U <<(1));

	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_UART0;

	UART0->FRAME &= ~( _UART_FRAME_STOPBITS_MASK
                          |_UART_FRAME_PARITY_MASK
                          |_UART_FRAME_DATABITS_MASK );

        UART0->FRAME |=   UART_FRAME_STOPBITS_ONE
                        | UART_FRAME_PARITY_NONE
                        | UART_FRAME_DATABITS_EIGHT;


        UART0->CTRL  = _UART_CTRL_RESETVALUE|UART_CTRL_OVS_X16;

        bauddiv = (ClockGetPeripheralClockFrequency()*4)/(OVS*BAUD)-4;
        UART0->CLKDIV = bauddiv<<_UART_CLKDIV_DIV_SHIFT;

        GPIOF->MODEL &= ~_GPIO_P_MODEL_MODE7_MASK;
        GPIOF->MODEL |= GPIO_P_MODEL_MODE7_PUSHPULL;
	GPIOF->DOUT |= (1U <<(7));

	UART0->ROUTE = UART_ROUTE_LOCATION_LOC1 | UART_ROUTE_RXPEN | UART_ROUTE_TXPEN;

	rxBuffer = bufferInit(rxBufferArea, BUFFERSIZE);


	UART0->IFC = (uint32_t) -1;
        UART0->IEN |= UART_IEN_TXC|UART_IEN_RXDATAV;

        NVIC_SetPriority(UART0_RX_IRQn,INTERRUPTLVL);

        NVIC_ClearPendingIRQ(UART0_RX_IRQn);
        NVIC_EnableIRQ(UART0_RX_IRQn);

        UART0->CMD  = UART_CMD_TXDIS|UART_CMD_RXDIS;
        UART0->CMD  = UART_CMD_TXEN|UART_CMD_RXEN;
}


void UART0_RX_IRQHandler(void) {
	uint8_t ch;
	if(UART0->IF & (UART_IF_RXDATAV | UART_IF_RXFULL)) {
		ch = UART0->RXDATA;
		(void) bufferInsert(rxBuffer, ch);
	}
}

unsigned UART_GetChar(void) {
	int ch;

	if(bufferEmpty(rxBuffer))
		return 0;

	__disable_irq();
	ch = bufferRemove(rxBuffer);
	__enable_irq();
	return ch;
}

