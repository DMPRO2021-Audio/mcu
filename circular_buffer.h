#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 256

typedef struct {
    uint8_t data[BUFFER_SIZE];
    int head;
    int tail;
} circular_buffer_t;

void    circular_buffer_init(circular_buffer_t *buf);
uint8_t circular_buffer_pop(circular_buffer_t *buf);
void    circular_buffer_push(circular_buffer_t *buf, uint8_t data);
bool    circular_buffer_empty(circular_buffer_t *buf);
#endif
