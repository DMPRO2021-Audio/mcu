#include <stdint.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include "clock_efm32gg_ext.h"
#include <em_usart.h>

#include "circular_buffer.h"

#define BUFFERSIZE 256
#define INTERRUPTLVL 6

const uint32_t BAUD = 31250;
const uint32_t OVS = 16;

circular_buffer_t circular_buffer;

void uart_init(void) {
    uint32_t bauddiv;

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_USART1, true);
    GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.oversampling = usartOVS16;
    USART_InitAsync(USART1, &init);

    bauddiv = (ClockGetPeripheralClockFrequency()*4)/(OVS*BAUD)-4;
    USART1->CLKDIV = bauddiv<<_USART_CLKDIV_DIV_SHIFT;
    USART1->ROUTE = USART_ROUTE_LOCATION_LOC1 | USART_ROUTE_RXPEN;

    /* Setup the buffer that stores the incoming MIDI messages */
    circular_buffer_init(&circular_buffer);

    USART_IntEnable(USART1, USART_IEN_RXDATAV);

    NVIC_SetPriority(USART1_RX_IRQn, INTERRUPTLVL);
    NVIC_ClearPendingIRQ(USART1_RX_IRQn);
    NVIC_EnableIRQ(USART1_RX_IRQn);

}


void USART1_RX_IRQHandler(void) {
    uint8_t ch;
    if(USART1->IF & (USART_IF_RXDATAV | USART_IF_RXFULL)) {
        ch = USART1->RXDATA;
        circular_buffer_push(&circular_buffer, ch);
        // (void) bufferInsert(rxBuffer, ch);
    }
}


uint8_t uart_next_byte(void) {
    uint8_t ch;

    if(circular_buffer_empty(&circular_buffer)) {
        return 0;
    }

    __disable_irq();
    ch = circular_buffer_pop(&circular_buffer);
    __enable_irq();
    return ch;
}

uint8_t uart_next_nonzero_byte(void)
{
    uint8_t byte = 0;
    do {
        byte = uart_next_byte();
    } while (byte == 0);
    return byte;
}
