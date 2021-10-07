#include "buffer.h"

buffer bufferInit(void *b, int n) {
    buffer f = (buffer) b;

    f->front = f->rear = f->data;
    f->size = 0;
    f->capacity = n;
    return f;
}


int bufferInsert(buffer f, char x) {

    if( bufferFull(f) )
        return -1;
    *(f->rear++) = x;
    f->size++;
    if( (f->rear - f->data) > f->capacity )
        f->rear = f->data;
    return 0;
}

int bufferRemove(buffer f) {
    char ch;

    if( bufferEmpty(f) )
        return -1;
    ch = *(f->front++);
    f->size--;
    if( (f->front - f->data) > f->capacity )
        f->front = f->data;

    return ch;
}
