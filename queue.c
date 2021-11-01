#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util.h"
#include "queue.h"

bool queue_get(Queue *self, void *buf, size_t len) {
    uint32_t prio = raise_priority(self->priority);
    size_t head = self->head;
    size_t len1 = self->capacity - head;

    if (len > self->length) return false;
    if (len1 > len) {
        len1 = len;
        self->head += len;
    } else {
        self->head = len - len1;
    }
    memcpy(buf, (void *)&self->buf[head], len1);
    memcpy(buf + len1, (void *)self->buf, len - len1);
    self->length -= len;

    restore_priority(prio);
    return true;
}

bool queue_put(Queue *self, const void *buf, size_t len) {
    uint32_t prio = raise_priority(self->priority);
    size_t tail = self->head + self->length;
    size_t len1 = self->capacity - tail;

    if (len > self->capacity - self->length) return false;
    if (len1 > len) len1 = len;
    memcpy((void *)&self->buf[tail], buf, len1);
    memcpy((void *)self->buf, buf + len1, len - len1);
    self->length += len;

    restore_priority(prio);
    return true;
}
