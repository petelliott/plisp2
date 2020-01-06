#include <plisp/builtin.h>
#include <plisp/object.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <plisp/toplevel.h>
#include <plisp/saftey.h>
#include <stdarg.h>

void plisp_init_builtin(void) {
    // gcc passes variadic args just like regular args, so this is
    // okayish. the C standard forbids casting form f(...) to f().
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

    plisp_define_builtin("+", plisp_builtin_plus);
    plisp_define_builtin("-", plisp_builtin_minus);

    plisp_define_builtin("cons", plisp_builtin_cons);
    plisp_define_builtin("car", plisp_builtin_car);
    plisp_define_builtin("cdr", plisp_builtin_cdr);

    plisp_define_builtin("reverse", plisp_builtin_reverse);
    plisp_define_builtin("list", plisp_builtin_list);

    plisp_define_builtin("not", plisp_builtin_not);
    plisp_define_builtin("null?", plisp_builtin_nullp);
    plisp_define_builtin("eq?", plisp_builtin_eq);
    plisp_define_builtin("<", plisp_builtin_lt);

    plisp_define_builtin("length", plisp_builtin_length);

    plisp_define_builtin("display", plisp_builtin_display);
    plisp_define_builtin("write", plisp_builtin_write);
    plisp_define_builtin("println", plisp_builtin_println);
    plisp_define_builtin("newline", plisp_builtin_newline);

    #pragma GCC diagnostic pop
}

void plisp_define_builtin(const char *name, plisp_fn_t fun) {
    plisp_toplevel_define(
        plisp_intern(plisp_make_symbol(name)),
        plisp_make_closure(NULL, fun));
}

plisp_t plisp_builtin_plus(plisp_t *clos, size_t nargs, plisp_t a,
                           plisp_t b, ...) {
    plisp_assert(plisp_c_fixnump(a));
    plisp_assert(plisp_c_fixnump(b));
    plisp_assert(nargs >= 2);

    va_list vl;
    va_start(vl, b);

    plisp_t sum = a + b;

    for (size_t i = 2; i < nargs; ++i) {
        plisp_t arg = va_arg(vl, plisp_t);
        plisp_assert(plisp_c_fixnump(arg));
        sum += arg;
    }

    va_end(vl);

    return sum;
}

plisp_t plisp_builtin_minus(plisp_t *clos, size_t nargs, plisp_t a,
                            plisp_t b, ...) {
    plisp_assert(plisp_c_fixnump(a));
    plisp_assert(plisp_c_fixnump(b));
    plisp_assert(nargs >= 2);

    va_list vl;
    va_start(vl, b);

    plisp_t sum = a - b;

    for (size_t i = 2; i < nargs; ++i) {
        plisp_t arg = va_arg(vl, plisp_t);
        plisp_assert(plisp_c_fixnump(arg));
        sum -= arg;
    }

    va_end(vl);

    return sum;
}

plisp_t plisp_builtin_cons(plisp_t *clos, size_t nargs, plisp_t car,
                           plisp_t cdr) {
    plisp_assert(nargs == 2);
    return plisp_cons(car, cdr);
}

plisp_t plisp_builtin_car(plisp_t *clos, size_t nargs, plisp_t cell) {
    plisp_assert(nargs == 1);
    return plisp_car(cell);
}

plisp_t plisp_builtin_cdr(plisp_t *clos, size_t nargs, plisp_t cell) {
    plisp_assert(nargs == 1);
    return plisp_car(cell);
}

plisp_t plisp_c_reverse(plisp_t lst) {
    return plisp_builtin_reverse(NULL, 2, lst, plisp_nil);
}

plisp_t plisp_builtin_reverse(plisp_t *clos, size_t nargs,
                              plisp_t lst, plisp_t onto) {
    plisp_assert(nargs == 1 || nargs == 2);

    if (plisp_c_nullp(lst)) {
        return onto;
    }

    if (nargs == 1) {
        onto = plisp_nil;
    }

    return plisp_builtin_reverse(NULL, 2, plisp_cdr(lst),
                                 plisp_cons(plisp_car(lst), onto));
}

plisp_t plisp_builtin_list(plisp_t *clos, size_t nargs, ...) {
    va_list vl;
    va_start(vl, nargs);

    plisp_t lst = plisp_nil;
    for (size_t i = 0; i < nargs; ++i) {
        lst = plisp_cons(va_arg(vl, plisp_t), lst);
    }

    va_end(vl);

    return plisp_c_reverse(lst);
}

plisp_t plisp_builtin_not(plisp_t *clos, size_t nargs, plisp_t obj) {
    plisp_assert(nargs == 1);
    if (obj == plisp_make_bool(false)) {
        return plisp_make_bool(true);
    } else {
        return plisp_make_bool(false);
    }
}

plisp_t plisp_builtin_nullp(plisp_t *clos, size_t nargs, plisp_t obj) {
    plisp_assert(nargs == 1);
    return plisp_make_bool(plisp_c_nullp(obj));
}

plisp_t plisp_builtin_eq(plisp_t *clos, size_t nargs, plisp_t a, plisp_t b) {
    plisp_assert(nargs == 2);
    return plisp_make_bool(a == b);
}

plisp_t plisp_builtin_lt(plisp_t *clos, size_t nargs, plisp_t a, plisp_t b) {
    plisp_assert(nargs == 2);
    return plisp_make_bool(a < b);
}

size_t plisp_c_length(plisp_t lst) {
    plisp_assert(plisp_c_consp(lst) || plisp_c_nullp(lst));

    if (plisp_c_nullp(lst)) {
        return 0;
    }

    return 1 + plisp_c_length(plisp_cdr(lst));
}

plisp_t plisp_builtin_length(plisp_t *clos, size_t nargs, plisp_t lst) {
    plisp_assert(nargs == 1);
    return plisp_make_fixnum(plisp_c_length(lst));
}

plisp_t plisp_builtin_display(plisp_t *clos, size_t nargs, plisp_t obj) {
    plisp_assert(nargs == 1);
    plisp_c_write(stdout, obj);
    return plisp_unspec;
}

plisp_t plisp_builtin_write(plisp_t *clos, size_t nargs, plisp_t obj) {
    plisp_assert(nargs == 1);
    plisp_c_write(stdout, obj);
    return plisp_unspec;
}

plisp_t plisp_builtin_println(plisp_t *clos, size_t nargs, ...) {
    va_list vl;
    va_start(vl, nargs);

    for (size_t i = 0; i < nargs; ++i) {
        if (i != 0) {
            putchar(' ');
        }
        plisp_t arg = va_arg(vl, plisp_t);
        plisp_c_write(stdout, arg);
    }
    putchar('\n');

    va_end(vl);
    return plisp_unspec;
}

plisp_t plisp_builtin_newline(plisp_t *clos, size_t nargs) {
    plisp_assert(nargs == 0);
    putchar('\n');
    return plisp_unspec;
}
