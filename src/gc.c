#include <plisp/gc.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define MAX_ALLOC_PAGE_SIZE 64

struct obj_allocs {
    size_t allocated[MAX_ALLOC_PAGE_SIZE/(sizeof(size_t)*8)];
    //size_t grey_set[MAX_ALLOC_PAGE_SIZE/(sizeof(size_t)*8)];
    size_t black_set[MAX_ALLOC_PAGE_SIZE/(sizeof(size_t)*8)];
    size_t freecdr[MAX_ALLOC_PAGE_SIZE/(sizeof(size_t)*8)];
    size_t num_objs;
    struct plisp_cons *objs;
    struct obj_allocs *next;
};

// pool for allocating cons sized objects
static struct obj_allocs *conspool = NULL;
static plisp_t perm_root = plisp_nil;
static plisp_t *stack_bottom;


// thanks jacob <3
void plisp_init_gc(void) {
    uintptr_t sb;
    FILE *statfp = fopen("/proc/self/stat", "r");
    assert(statfp != NULL);
    fscanf(statfp,
           "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
           "%*u %*u %*u %*u %*u %*u %*d %*d "
           "%*d %*d %*d %*d %*u %*u %*d "
           "%*u %*u %*u %lu", &sb);
    stack_bottom = (plisp_t *) sb;
    fclose(statfp);
}

static bool get_bit(size_t *array, size_t i) {
    return array[i/(sizeof(size_t)*8)]
        & (1lu << (i % (sizeof(size_t)*8)));
}

static void set_bit(size_t *array, size_t i, bool val) {
    if (val) {
        array[i/(sizeof(size_t)*8)] |= (1lu << (i % (sizeof(size_t)*8)));
    } else {
        array[i/(sizeof(size_t)*8)] &= ~(1lu << (i % (sizeof(size_t)*8)));
    }
}

// gets the index of the first free 0 bit
static size_t first_free(const size_t *array, size_t len) {
    for (size_t i = 0; i < len/(sizeof(size_t) * 8); ++i) {
        size_t block = array[i];
        if (block != 0xfffffffffffffffflu) {
            for (size_t j = 0; j < sizeof(size_t) * 8; ++j) {
                if (!(block & (1lu << j))) {
                    return i*sizeof(size_t)*8 + j;
                }
            }
        }
    }
    return len;
}

static void *allocate_or_null(struct obj_allocs *pool, bool freecdr) {
    if (pool == NULL) {
        return NULL;
    }

    size_t i = first_free(pool->allocated, pool->num_objs);
    if (i == pool->num_objs) {
        return allocate_or_null(pool->next, freecdr);
    }

    set_bit(pool->allocated, i, 1);
    set_bit(pool->freecdr, i, freecdr);
    return pool->objs + i;
}

static struct obj_allocs *make_obj_allocs(struct obj_allocs *next) {
    struct obj_allocs *allocs = malloc(sizeof(struct obj_allocs));

    memset(allocs->allocated, 0, sizeof(allocs->allocated));
    //memset(allocs->grey_set, 0, sizeof(allocs->grey_set));
    memset(allocs->black_set, 0, sizeof(allocs->black_set));
    memset(allocs->freecdr, 0, sizeof(allocs->freecdr));

    allocs->num_objs = MAX_ALLOC_PAGE_SIZE; // TODO: maybe set this dynamically
    allocs->objs = malloc(allocs->num_objs * sizeof(struct plisp_cons));
    allocs->next = next;

    return allocs;
}

static size_t get_pool_off(plisp_t obj, struct obj_allocs **pool) {
    if (*pool == NULL) {
        return 0;
    }
    struct plisp_cons *ptr = (struct plisp_cons *) (obj & ~LOTAGS);
    if (ptr >= (*pool)->objs && ptr < (*pool)->objs + (*pool)->num_objs) {
        return ptr - (*pool)->objs;
    } else {
        *pool = (*pool)->next;
        return get_pool_off(obj, pool);
    }
}

static void trace_object(plisp_t obj) {
    if (!plisp_heap_allocated(obj)) {
        return;
    }

    struct obj_allocs *pool = conspool;
    size_t off = get_pool_off(obj, &pool);
    if (pool == NULL) {
        return;
    }

    assert(get_bit(pool->allocated, off));

    if (get_bit(pool->black_set, off)) {
        return;
    }

    set_bit(pool->black_set, off, 1);

    if (plisp_c_consp(obj)) {
        trace_object(plisp_car(obj));
        trace_object(plisp_cdr(obj));
    } else if (plisp_c_customp(obj)) {
        trace_object(plisp_custom_typesym(obj));
    } else if (plisp_c_closurep(obj)) {
        //TODO: this is hard
    } else if (plisp_c_symbolp(obj)) {
        //nothing to do here
    } else if (plisp_c_vectorp(obj)) {
        //TODO: only scan if it's a vector of objects
    }
    // add numbers when needed
}

static void trace_stack(void) {
    plisp_t stack_top;
    for (plisp_t *n = &stack_top; n < stack_bottom; ++n) {
        trace_object(*n);
    }
}

#define PUSH_REGS                               \
    asm("pushq %rax");                          \
    asm("pushq %rbx");                          \
    asm("pushq %rcx");                          \
    asm("pushq %rdx");                          \
    asm("pushq %rsi");                          \
    asm("pushq %rdi");                          \
    asm("pushq %r8");                           \
    asm("pushq %r9");                           \
    asm("pushq %r10");                          \
    asm("pushq %r11");                          \
    asm("pushq %r12");                          \
    asm("pushq %r13");                          \
    asm("pushq %r14");                          \
    asm("pushq %r15");                          \


size_t plisp_collect_garbage(void) {
    for (struct obj_allocs *pool = conspool; pool != NULL; pool = pool->next) {
        memset(pool->black_set, 0, sizeof(pool->black_set));
    }

    trace_object(perm_root);

    // push registers, so they will be scanned with the stack
    //PUSH_REGS;
    // BROKEN

    trace_stack();

    size_t freed = 0;
    for (struct obj_allocs *pool = conspool; pool != NULL; pool = pool->next) {
        for (size_t i = 0; i < pool->num_objs; ++i) {
            if (!get_bit(pool->black_set, i) && get_bit(pool->allocated, i)) {
                ++freed;
                set_bit(pool->allocated, i, 0);
                if (get_bit(pool->freecdr, i)) {
                    free((void *) pool->objs[i].cdr);
                }
            }
        }
    }

    return freed;
}


plisp_t plisp_alloc_obj(uintptr_t tags, bool freecdr) {
    void *ptr = allocate_or_null(conspool, freecdr);
    if (ptr == NULL) {
        //fprintf(stderr, "collected %lu objects\n", plisp_collect_garbage());
        plisp_collect_garbage();
        ptr = allocate_or_null(conspool, freecdr);
        if (ptr == NULL) {
            conspool = make_obj_allocs(conspool);
            ptr = allocate_or_null(conspool, freecdr);
            assert(ptr != NULL);
        }
    }
    return ((plisp_t) ptr) | tags;
}

bool plisp_heap_allocated(plisp_t obj) {
    return !plisp_c_nullp(obj) &&
        (plisp_c_consp(obj)
         || plisp_c_symbolp(obj)
         || plisp_c_vectorp(obj)
         || plisp_c_stringp(obj)
         || plisp_c_customp(obj)
         || plisp_c_closurep(obj));
}


void plisp_gc_permanent(plisp_t obj) {
    assert(plisp_heap_allocated(obj));
    perm_root = plisp_cons(obj, perm_root);
}
