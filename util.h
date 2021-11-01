#include <stdint.h>

#include <cmsis_gcc.h>

#define lenof(a) (sizeof(a) / sizeof(*a))
#define endof(a) (a + lenof(a))
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

/* put a breakpoint on this when debugging ;) */
static inline void warn(void) {}

static inline uint32_t raise_priority(uint32_t priority) {
    uint32_t basepri = __get_BASEPRI();

    if (priority < basepri) warn();
    __set_BASEPRI(priority);
    return basepri;
}

static inline void restore_priority(uint32_t priority) {
    if (priority > __get_BASEPRI()) warn();
    __set_BASEPRI(priority);
}

#if __GNUC__

#define PACKED __attribute__((packed))

#define CRITICAL(prio) for ( \
    uint32_t __prev = __get_BASEPRI(), __go = (__set_BASEPRI(prio), 1); \
    __go; \
    __set_BASEPRI(__prev), __go = 0 \
)

#endif
