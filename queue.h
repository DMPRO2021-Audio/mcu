typedef volatile struct {
    uint32_t priority; /* max priority of any IRQ using this resource */
    size_t capacity;   /* sizeof(buf) */
    size_t length;     /* number of bytes in the queue */
    size_t head;       /* index of next byte to be removed */
    char buf[];
} Queue;

bool queue_get(Queue *self, void *buf, size_t len);
bool queue_put(Queue *self, const void *buf, size_t len);
