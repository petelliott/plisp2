#include <plisp/continuation.h>
#include <plisp/builtin.h>
#include <plisp/saftey.h>
#include <plisp/gc.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>


void plisp_init_continuation(void) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

    plisp_define_builtin("call-with-current-continuation", plisp_builtin_callcc);
    plisp_define_builtin("call/cc", plisp_builtin_callcc);

    #pragma GCC diagnostic pop
}


struct fake_clos {
    size_t length;
    plisp_t stackvec;
    jmp_buf env;
};

static plisp_t save_stack(void) {
    plisp_t stack_top;
    char *stop = (char *) &stack_top;
    char *sbottom = (char *) stack_bottom;
    size_t length = (sbottom - stop);

    plisp_t vec = plisp_make_vector(VEC_CHAR, sizeof(char),
                                    0, length, 0, false);


    struct plisp_vector *vecptr = (void *)(vec & ~LOTAGS);
    memcpy(vecptr->vec, stop, length);

    return vec;
}


static plisp_t contret = plisp_unspec;

static plisp_t plisp_contfn(struct fake_clos *clos,
                            size_t nargs, plisp_t ret) {
    static jmp_buf *tmp_buf;
    static size_t length;
    static char *stop;
    static char *scopy;

    plisp_assert(nargs == 1 || nargs == 0);
    plisp_assert(clos != NULL);

    if (nargs == 1) {
        contret = ret;
    } else {
        contret = plisp_unspec;
    }
    tmp_buf = &(clos->env);

    struct plisp_vector *vecptr = (void *)(clos->stackvec & ~LOTAGS);
    stop = ((char *) stack_bottom) + length;
    length = vecptr->len;
    scopy = vecptr->vec;

    memcpy(stop, scopy, length);

    longjmp(*tmp_buf, 1);

    return plisp_unspec;
}

plisp_t plisp_builtin_callcc(plisp_t *clos, size_t nargs, plisp_t proc) {
    plisp_assert(nargs == 1);
    plisp_assert(plisp_c_closurep(proc));

    struct fake_clos *fc = malloc(sizeof(struct fake_clos));
    fc->length = 1; // length is 0 so gc will not try to scan jmp_buf
    fc->stackvec = save_stack();

    plisp_t cont = plisp_make_closure((void *) fc, (plisp_fn_t) plisp_contfn);

    if (setjmp(fc->env) == 0) {
        return plisp_closure_fun(proc)(
            plisp_closure_data(proc), 1, cont);
    } else {
        return contret;
    }
}
