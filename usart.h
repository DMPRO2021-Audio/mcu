#ifndef USART_H
#define USART_H
#include <stdint.h>

void uart_init(void);
uint8_t uart_next_byte(void);
uint8_t uart_next_nonzero_byte(void);
#endif
