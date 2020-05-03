#include <plisp/continuation.h>
#include <plisp/builtin.h>
#include <plisp/saftey.h>
#include <setjmp.h>
#include <stdlib.h>


void plisp_init_continuation(void) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

    plisp_define_builtin("call-with-current-continuation", plisp_builtin_callcc);
    plisp_define_builtin("call/cc", plisp_builtin_callcc);

    #pragma GCC diagnostic pop
}


struct fake_clos {
    size_t length;
    jmp_buf env;
};

static plisp_t contret = plisp_unspec;

static plisp_t plisp_contfn(struct fake_clos *clos,
                            size_t nargs, plisp_t ret) {
    plisp_assert(nargs == 1);
    plisp_assert(clos != NULL);


    contret = ret;
    // TODO restore stack
    longjmp(clos->env, 1);

    return plisp_unspec;
}

plisp_t plisp_builtin_callcc(plisp_t *clos, size_t nargs, plisp_t proc) {
    plisp_assert(nargs == 1);
    plisp_assert(plisp_c_closurep(proc));

    struct fake_clos *fc = malloc(sizeof(struct fake_clos));
    fc->length = 0; // length is 0 so gc will not try to scan jmp_buf

    plisp_t cont = plisp_make_closure((void *) fc, (plisp_fn_t) plisp_contfn);

    if (setjmp(fc->env) == 0) {
        return plisp_closure_fun(proc)(
            plisp_closure_data(proc), 1, cont);
    } else {
        return contret;
    }
}
