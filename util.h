#include <stddef.h>
#include <stdint.h>

#include <cmsis_gcc.h>

#define lenof(a) (sizeof(a) / sizeof(*a))
#define endof(a) (a + lenof(a))
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

typedef struct List List;
struct List {
    List *prev, *next;
};

static inline void list_move(List *item, List *next, List *prev) {
    if (item->next) item->next->prev = item->prev;
    if (item->prev) item->prev->next = item->next;
    if (next == item) next = item->next;
    if (prev == item) prev = item->prev;
    if (next && next->prev) next->prev->next = NULL;
    if (prev && prev->next) prev->next->prev = NULL;
    if (next) next->prev = item;
    if (prev) prev->next = item;
    item->next = next;
    item->prev = prev;
}

static inline void list_move_before(List *item, List *next) {
    list_move(item, next, next->prev);
}

static inline void list_move_after(List *item, List *prev) {
    list_move(item, prev->next, prev);
}

static inline void list_delete(List *item) {
    list_move(item, NULL, NULL);
}

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
