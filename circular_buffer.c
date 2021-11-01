#include "circular_buffer.h"

uint8_t circular_buffer_pop(circular_buffer_t *buf)
{

    if (circular_buffer_empty(buf))
        return -1;
    uint8_t item = buf->data[buf->head];
    buf->head++;
    if (buf->head >= BUFFER_SIZE)
        buf->head = 0;
    return item;
}

void circular_buffer_push(circular_buffer_t *buf, uint8_t item)
{
    buf->data[buf->tail] = item;
    buf->tail++;
    if (buf->tail >= BUFFER_SIZE)
        buf->tail = 0;
}

void circular_buffer_init(circular_buffer_t *buf)
{
    buf->head = 0;
    buf->tail = 0;
}

bool circular_buffer_empty(circular_buffer_t *buf)
{
    return buf->head == buf->tail;
}
