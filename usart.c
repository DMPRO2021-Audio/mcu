#include <em_device.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include <em_usart.h>
#include <stdint.h>

#include "circular_buffer.h"

#define BUFFERSIZE 256
#define INTERRUPTLVL 6

const uint32_t BAUD = 31250;
const uint32_t OVS = 16;

#define UART_EMPTY 0xFF

circular_buffer_t circular_buffer;

void uart_init(void) {
    uint32_t bauddiv;

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_UART0, true);
    GPIO_PinModeSet(gpioPortA, 4, gpioModeInput, 0);

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.oversampling = usartOVS16;
    USART_InitAsync(UART0, &init);

    UART0->CLKDIV = 9472;
    UART0->ROUTELOC0 = UART_ROUTELOC0_RXLOC_LOC2;
    UART0->ROUTEPEN = UART_ROUTEPEN_RXPEN;

    /* Setup the buffer that stores the incoming MIDI messages */
    circular_buffer_init(&circular_buffer);

    USART_IntEnable(UART0, UART_IEN_RXDATAV);
    NVIC_SetPriority(UART0_RX_IRQn, INTERRUPTLVL);
    NVIC_ClearPendingIRQ(UART0_RX_IRQn);
    NVIC_EnableIRQ(UART0_RX_IRQn);

}


void UART0_RX_IRQHandler(void) {
    uint8_t ch;
    if(UART0->IF & (UART_IF_RXDATAV | UART_IF_RXFULL)) {
        ch = UART0->RXDATA;
        circular_buffer_push(&circular_buffer, ch);
    }
}


uint8_t uart_next_byte(void) {
    uint8_t ch;

    if(circular_buffer_empty(&circular_buffer)) {
        return UART_EMPTY;
    }

    __disable_irq();
    ch = circular_buffer_pop(&circular_buffer);
    __enable_irq();
    return ch;
}

/* The function `uart_next_byte` will return UART_EMPTY if the circular buffer
 * is empty. To guarantee a valid byte, `uart_next_valid_byte` polls until the
 * circular buffer is non-empty*/
uint8_t uart_next_valid_byte(void)
{
    uint8_t byte = 0;
    do {
        byte = uart_next_byte();
    } while (byte == UART_EMPTY);
    return byte;
}
