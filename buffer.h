#ifndef BUFFER_H
#define BUFFER_H
struct buffer_s {
    char    *front;
    char    *rear;
    int     size;
    int     capacity;
    char    data[];
};

typedef struct buffer_s *buffer;

buffer  bufferInit(void *area,int size);
int     bufferInsert(buffer f, char x);
int     bufferRemove(buffer f);

#define bufferCapacity(F) ((F)->capacity)
#define bufferEmpty(F) ((F)->size==0)
#define bufferFull(F) ((F)->size==bufferCapacity(F))

#endif
