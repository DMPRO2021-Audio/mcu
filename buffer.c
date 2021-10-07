#include "buffer.h"
#include "buffer.h"

#define USE_ACCESSCONTROL


#ifdef USE_ACESSCONTROL
#define ENTER_CRITICAL_SECTION()    _disable_irq()
#define EXIT_CRITICAL_SECTION()     _enable_irq()
#else
#define ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()
#endif

buffer bufferInit(void *b, int n) {
    buffer f = (buffer) b;

    ENTER_CRITICAL_SECTION();
    f->front = f->rear = f->data;
    f->size = 0;
    f->capacity = n;
    EXIT_CRITICAL_SECTION();
    return f;
}

void bufferDeinit(buffer f) {

    ENTER_CRITICAL_SECTION();
    f->size = 0;
    f->front = f->rear = f->data;
    EXIT_CRITICAL_SECTION();

}

void bufferClear(buffer f) {

    ENTER_CRITICAL_SECTION();
    f->size = 0;
    f->front = f->rear = f->data;
    EXIT_CRITICAL_SECTION();

}

int bufferInsert(buffer f, char x) {

    if( bufferFull(f) )
        return -1;
    ENTER_CRITICAL_SECTION();
    *(f->rear++) = x;
    f->size++;
    if( (f->rear - f->data) > f->capacity )
        f->rear = f->data;
    EXIT_CRITICAL_SECTION();
    return 0;
}

int bufferRemove(buffer f) {
    char ch;

    if( bufferEmpty(f) )
        return -1;
    ENTER_CRITICAL_SECTION();
    ch = *(f->front++);
    f->size--;
    if( (f->front - f->data) > f->capacity )
        f->front = f->data;
    EXIT_CRITICAL_SECTION();

    return ch;
}
