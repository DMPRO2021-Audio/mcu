#define PACKED __attribute__((packed))

#define lenof(a) (sizeof(a) / sizeof(*a))
#define endof(a) (a + lenof(a))
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

/* put a breakpoint on this when debugging ;) */
static inline void warn(void) {}
