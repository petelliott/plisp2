#include <plisp/builtin.h>
#include <plisp/object.h>
#include <plisp/read.h>
#include <plisp/toplevel.h>
#include <assert.h>

void plisp_init_builtin(void) {
    plisp_define_builtin("+", plisp_plus);
    plisp_define_builtin("-", plisp_minus);
    plisp_define_builtin("cons", plisp_cons);
    plisp_define_builtin("car", plisp_car);
    plisp_define_builtin("cdr", plisp_cdr);
}

void plisp_define_builtin(const char *name, plisp_fn_t fun) {
    plisp_toplevel_define(
        plisp_intern(plisp_make_symbol(name)),
        plisp_make_closure(NULL, fun));
}

plisp_t plisp_plus(plisp_t a, plisp_t b) {
    assert(plisp_c_fixnump(a));
    assert(plisp_c_fixnump(b));

    return a + b;
}

plisp_t plisp_minus(plisp_t a, plisp_t b) {
    assert(plisp_c_fixnump(a));
    assert(plisp_c_fixnump(b));

    return a - b;
}
