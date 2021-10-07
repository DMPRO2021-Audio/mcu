#ifndef BUFFER_H
#define BUFFER_H
/**
 *  @file   buffer.h
 */


/**
 *  @brief  Data structure to store info about fifo, including its data
 *
 * @note    Uses x[0] hack. This structure is a header
 * @note    First element is a pointer to force data alignement
 */

struct buffer_s {
    char    *front;             // pointer to first char in fifo
    char    *rear;              // pointer to last char in fifo
    int     size;               // number of char stored in fifo
    int     capacity;           // number of chars in data
    char    data[];             // flexible array
};

typedef struct buffer_s *buffer;

#define DECLARE_BUFFER_AREA(AREANAME,SIZE) unsigned AREANAME[(sizeof(struct buffer_s)+(SIZE)+sizeof(unsigned)-1)/sizeof(unsigned)]

buffer  bufferInit(void *area,int size);
void    bufferFeinit(buffer f);
int     bufferInsert(buffer f, char x);
int     bufferRemove(buffer f);
void    bufferClear(buffer f);

#define bufferCapacity(F) ((F)->capacity)
#define bufferSize(F) ((F)->size)
#define bufferEmpty(F) ((F)->size==0)
#define bufferFull(F) ((F)->size==bufferCapacity(F))

#endif
