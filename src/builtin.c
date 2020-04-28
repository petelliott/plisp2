#include <plisp/builtin.h>
#include <plisp/object.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <plisp/toplevel.h>
#include <plisp/compile.h>
#include <plisp/saftey.h>
#include <plisp/gc.h>
#include <plisp/posix.h>
#include <stdarg.h>
#include <string.h>
#include <lightning.h>

static plisp_t filesym;

void plisp_init_builtin(void) {
    filesym = plisp_intern(plisp_make_symbol("%file"));
    // gcc passes variadic args just like regular args, so this is
    // okayish. the C standard forbids casting form f(...) to f().
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

    plisp_define_builtin("+", plisp_builtin_plus);
    plisp_define_builtin("-", plisp_builtin_minus);
    plisp_define_builtin("*", plisp_builtin_times);

    plisp_define_builtin("cons", plisp_builtin_cons);
    plisp_define_builtin("car", plisp_builtin_car);
    plisp_define_builtin("cdr", plisp_builtin_cdr);

    plisp_define_builtin("reverse", plisp_builtin_reverse);
    plisp_define_builtin("list", plisp_builtin_list);

    plisp_define_builtin("not", plisp_builtin_not);
    plisp_define_builtin("null?", plisp_builtin_nullp);
    plisp_define_builtin("eq?", plisp_builtin_eq);
    plisp_define_builtin("equal?", plisp_builtin_equal);
    plisp_define_builtin("<", plisp_builtin_lt);

    plisp_define_builtin("length", plisp_builtin_length);

    plisp_define_builtin("display", plisp_builtin_display);
    plisp_define_builtin("write", plisp_builtin_write);
    plisp_define_builtin("println", plisp_builtin_println);
    plisp_define_builtin("newline", plisp_builtin_newline);

    plisp_define_builtin("collect-garbage", plisp_builtin_collect_garbage);
    plisp_define_builtin("object-addr", plisp_builtin_object_addr);
    plisp_define_builtin("disassemble", plisp_builtin_disassemble);

    plisp_define_builtin("vector", plisp_builtin_vector);
    plisp_define_builtin("make-vector", plisp_builtin_make_vector);
    plisp_define_builtin("list->vector", plisp_builtin_list_to_vector);
    plisp_define_builtin("vector-ref", plisp_builtin_vector_ref);
    plisp_define_builtin("vector-set!", plisp_builtin_vector_set);
    plisp_define_builtin("vector-append", plisp_builtin_vector_append);
    plisp_define_builtin("string-append", plisp_builtin_string_append);
    plisp_define_builtin("vector-length", plisp_builtin_vector_length);
    plisp_define_builtin("string-length", plisp_builtin_string_length);


    plisp_define_builtin("read", plisp_builtin_read);
    plisp_define_builtin("load", plisp_builtin_load);

    plisp_define_builtin("eval", plisp_builtin_eval);
    plisp_define_builtin("apply", plisp_generate_apply());
    plisp_define_builtin("hashq", plisp_builtin_hashq);


    #pragma GCC diagnostic pop

    plisp_init_posix();
}

void plisp_define_builtin(const char *name, plisp_fn_t fun) {
    plisp_toplevel_define(
        plisp_intern(plisp_make_symbol(name)),
        plisp_make_closure(NULL, fun));
}

plisp_t plisp_builtin_plus(plisp_t *clos, size_t nargs, plisp_t a,
                           plisp_t b, ...) {
    plisp_assert(nargs >= 2);
    plisp_assert(plisp_c_fixnump(a));
    plisp_assert(plisp_c_fixnump(b));

    plisp_c_write(stdout, a);
    printf("\n");
    plisp_c_write(stdout, b);
    printf("\n");

    va_list vl;
    va_start(vl, b);

    plisp_t sum = a + b;

    for (size_t i = 2; i < nargs; ++i) {
        //printf("%lu\n", i);
        plisp_t arg = va_arg(vl, plisp_t);
        plisp_assert(plisp_c_fixnump(arg));
        plisp_c_write(stdout, arg);
        printf("\n");
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

plisp_t plisp_builtin_times(plisp_t *clos, size_t nargs, ...) {
    plisp_assert(nargs >= 2);

    va_list vl;
    va_start(vl, nargs);

    int64_t prod = 1;

    for (size_t i = 0; i < nargs; ++i) {
        plisp_t arg = va_arg(vl, plisp_t);
        plisp_assert(plisp_c_fixnump(arg));
        prod *= plisp_fixnum_value(arg);
    }

    va_end(vl);

    return plisp_make_fixnum(prod);
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
    return plisp_cdr(cell);
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

plisp_t plisp_builtin_equal(plisp_t *clos, size_t nargs, plisp_t a, plisp_t b) {
    plisp_assert(nargs == 2);
    return plisp_make_bool(plisp_c_equal(a,b));

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

plisp_t plisp_builtin_collect_garbage(plisp_t *clos, size_t nargs) {
    plisp_assert(nargs == 0);
    return plisp_make_fixnum(plisp_collect_garbage());
}

plisp_t plisp_builtin_object_addr(plisp_t *clos, size_t nargs, plisp_t obj) {
    plisp_assert(nargs == 1);
    if (plisp_heap_allocated(obj)) {
        fprintf(stderr, "%p\n", (void *) (obj & ~LOTAGS));
    } else {
        fprintf(stderr, "not heap allocated\n");
    }
    return plisp_unspec;
}

plisp_t plisp_builtin_vector(plisp_t *clos, size_t nargs, ...) {
    plisp_t vec = plisp_make_vector(VEC_OBJ, sizeof(plisp_t), 0,
                                    nargs, plisp_unspec, false);

    va_list vl;
    va_start(vl, nargs);

    for (size_t i = 0; i < nargs; ++i) {
        plisp_vector_set(vec, i, va_arg(vl, plisp_t));
    }

    return vec;
}

plisp_t plisp_builtin_make_vector(plisp_t *clos, size_t nargs,
                                  plisp_t len, plisp_t init) {
    plisp_assert(nargs == 1 || nargs == 2);
    plisp_assert(plisp_c_fixnump(len));

    if (nargs == 1) {
        init = plisp_unspec;
    }

    return plisp_make_vector(VEC_OBJ, sizeof(plisp_t), 0,
                             plisp_fixnum_value(len), init, true);
}

plisp_t plisp_list_to_vector(plisp_t lst) {
    plisp_assert(plisp_c_consp(lst) || plisp_c_nullp(lst));
    size_t len = plisp_c_length(lst);
    plisp_t vec = plisp_make_vector(VEC_OBJ, sizeof(plisp_t), 0,
                                    len, plisp_unspec, false);

    for (size_t i = 0; i < len; ++i) {
        plisp_vector_set(vec, i, plisp_car(lst));
        lst = plisp_cdr(lst);
    }


    return vec;
}

plisp_t plisp_builtin_list_to_vector(plisp_t *clos, size_t nargs, plisp_t lst) {
    plisp_assert(nargs == 1);
    return plisp_list_to_vector(lst);
}

plisp_t plisp_builtin_vector_ref(plisp_t *clos, size_t nargs,
                                 plisp_t vector, plisp_t idx) {
    plisp_assert(nargs == 2);
    return plisp_vector_ref(vector, plisp_fixnum_value(idx));
}

plisp_t plisp_builtin_vector_set(plisp_t *clos, size_t nargs,
                                 plisp_t vector, plisp_t idx, plisp_t val) {
    plisp_assert(nargs == 3);
    plisp_vector_set(vector, plisp_fixnum_value(idx), val);
    return plisp_unspec;
}

plisp_t plisp_builtin_vector_append(plisp_t *clos, size_t nargs, ...) {
    if (nargs == 0) {
        return plisp_make_vector(VEC_OBJ, sizeof(plisp_t),
                                 0, 0, plisp_unspec, false);
    }

    va_list vl;
    va_start(vl, nargs);

    struct plisp_vector *vecs[128]; //TODO allow unbounded vectors

    size_t newlen = 0;
    for (size_t i = 0; i < nargs; ++i) {
        plisp_t vec = va_arg(vl, plisp_t);
        plisp_assert(plisp_c_vectorp(vec));
        vecs[i] = (void *) (vec & ~LOTAGS);
        if (i != 0) {
            plisp_assert(vecs[i]->type == vecs[i-1]->type);
            plisp_assert(vecs[i]->elem_width == vecs[i-1]->elem_width);
        }
        newlen += vecs[i]->len;
    }

    plisp_t newvec = plisp_make_vector(vecs[0]->type, vecs[0]->elem_width,
                                       0, newlen, plisp_unspec, false);
    struct plisp_vector *newvecptr = (void *) (newvec & ~LOTAGS);


    size_t off = 0;
    for (size_t i = 0; i < nargs; ++i) {
        size_t vlen = vecs[i]->len * vecs[i]->elem_width;
        memcpy(newvecptr->vec + off, vecs[i]->vec, vlen);
        off += vlen;
    }

    return newvec;
}

plisp_t plisp_builtin_string_append(plisp_t *clos, size_t nargs, ...) {
    va_list vl;
    va_start(vl, nargs);

    struct plisp_vector *vecs[128]; //TODO allow unbounded vectors

    size_t newlen = 1;
    for (size_t i = 0; i < nargs; ++i) {
        plisp_t vec = va_arg(vl, plisp_t);
        plisp_assert(plisp_c_stringp(vec));
        vecs[i] = (void *) (vec & ~LOTAGS);
        if (i != 0) {
            plisp_assert(vecs[i]->elem_width == vecs[i-1]->elem_width);
        }
        newlen += vecs[i]->len-1;
    }

    //TODO utf-16 and utf-32
    plisp_t newvec = plisp_make_vector(VEC_CHAR, sizeof(char),
                                       VFLAG_IMMUTABLE, newlen,
                                       plisp_unspec, false);

    struct plisp_vector *newvecptr = (void *) (newvec & ~LOTAGS);


    size_t off = 0;
    for (size_t i = 0; i < nargs; ++i) {
        size_t vlen = vecs[i]->len * vecs[i]->elem_width - 1;
        memcpy(newvecptr->vec + off, vecs[i]->vec, vlen);
        off += vlen;
    }
    ((char *)newvecptr->vec)[off] = 0; // null terminate

    return newvec;
}

plisp_t plisp_builtin_vector_length(plisp_t *clos, size_t nargs, plisp_t vec) {
    plisp_assert(nargs == 1);
    return plisp_make_fixnum(plisp_vector_c_length(vec));
}

plisp_t plisp_builtin_string_length(plisp_t *clos, size_t nargs, plisp_t vec) {
    plisp_assert(nargs == 1);
    plisp_assert(plisp_c_stringp(vec));
    return plisp_make_fixnum(plisp_vector_c_length(vec)-1);
}

plisp_t plisp_builtin_read(plisp_t *clos, size_t nargs) {
    plisp_assert(nargs == 0);
    return plisp_c_read(stdin);
}

void plisp_c_load(const char *fname) {
    plisp_t oldfile = *plisp_toplevel_ref(filesym);
    plisp_toplevel_define(filesym, plisp_make_string(fname));

    FILE *file = fopen(fname, "r");
    plisp_assert(file != NULL);

    plisp_t obj;
    while (!plisp_c_eofp(obj = plisp_c_read(file))) {
        plisp_toplevel_eval(obj);
    }

    fclose(file);

    plisp_toplevel_define(filesym, oldfile);
}

plisp_t plisp_builtin_load(plisp_t *clos, size_t nargs, plisp_t fname) {
    plisp_assert(nargs == 1);
    plisp_c_load(plisp_string_value(fname));
    return plisp_unspec;
}

plisp_t plisp_builtin_eval(plisp_t *clos, size_t nargs, plisp_t expr) {
    plisp_assert(nargs == 1);
    return plisp_toplevel_eval(expr);
}

/* this function is written in assembly because its the only non-eval
way of passing a number of arguments that is only known at runtime
*/
plisp_fn_t plisp_generate_apply(void) {
    jit_state_t *_jit = jit_new_state();

    jit_prolog();

    jit_arg(); // ignore the closure
    jit_node_t *nargs = jit_arg();
    jit_node_t *fun = jit_arg();

    jit_getarg(JIT_V0, nargs);
    jit_getarg(JIT_R0, fun);
    jit_ellipsis();
    jit_va_start(JIT_V2);

    // used to find the start of the stack, and the function we are
    // going to call. double whammy!
    int fn = jit_allocai(sizeof(plisp_t));
    jit_stxi(fn, JIT_FP, JIT_R0);

    //TODO: assert nargs >= 1
    jit_subi(JIT_V1, JIT_V0, 2);

    jit_movi(JIT_R0, sizeof(plisp_t));
    jit_allocar(JIT_V(3), JIT_R0);

    jit_node_t *onlylist = jit_beqi(JIT_V0, 2);
    jit_node_t *startloop = jit_label();

    // main loop pushing args
    jit_va_arg(JIT_R1, JIT_V2);
    jit_movi(JIT_R0, sizeof(plisp_t));
    jit_allocar(JIT_R0, JIT_R0);
    jit_stxr(JIT_R0, JIT_FP, JIT_R1);

    // TODO DEBUGGING
    jit_prepare();
    jit_pushargi((jit_word_t)"off %li\n");
    jit_pushargr(JIT_R0);
    jit_finishi(printf);

    jit_subi(JIT_V0, JIT_V0, 1);
    jit_node_t *endloop = jit_bgti(JIT_V0, 2);
    jit_patch_at(endloop, startloop);
    jit_patch(onlylist);

    /* TODO TODO TODO TODO
    // now we must push the list arguments
    jit_va_arg(JIT_V2, JIT_V2);
    jit_node_t *nulllst = jit_beqi(JIT_V2, plisp_nil);
    jit_node_t *startloop2 = jit_label();

    // increase the number of arguments
    jit_addi(JIT_V1, JIT_V1, 1);

    //get the car
    jit_prepare();
    jit_pushargr(JIT_V2);
    jit_finishi(plisp_car);
    jit_retval(JIT_R1);

    //push the car
    jit_movi(JIT_R0, sizeof(plisp_t));
    jit_allocar(JIT_R0, JIT_R0);
    jit_stxr(JIT_R0, JIT_FP, JIT_R1);

    //get the cdr
    jit_prepare();
    jit_pushargr(JIT_V2);
    jit_finishi(plisp_cdr);
    jit_retval(JIT_V2);

    jit_node_t *endloop2 = jit_bnei(JIT_V2, plisp_nil);
    jit_patch_at(endloop2, startloop2);
    jit_patch(nulllst);
    */

    //TODO assert fn is a closure
    jit_ldxi(JIT_V2, JIT_FP, fn);
    jit_andi(JIT_V2, JIT_V2, ~LOTAGS);
    jit_ldxi(JIT_R0, JIT_V2, sizeof(plisp_fn_t));


    // TODO DEBUGGING
    jit_prepare();
    jit_pushargi((jit_word_t)"of %li\n");
    jit_pushargr(JIT_V(3));
    jit_finishi(printf);

    jit_prepare();
    jit_pushargr(JIT_R0);
    jit_pushargr(JIT_V1);
    //jit_pushargi(0);

    //* TODO
    jit_movr(JIT_R0, JIT_V(3));
    //printf("%i\n", fn);
    jit_node_t *noargs2 = jit_beqi(JIT_V1, 0);
    jit_node_t *startpushloop = jit_label();
    jit_subi(JIT_R0, JIT_R0, 16); //TODO figure out the size

    jit_ldxr(JIT_R1, JIT_FP, JIT_R0);

    jit_pushargr(JIT_R1);

    jit_subi(JIT_V1, JIT_V1, 1);
    jit_node_t *endpushloop = jit_bnei(JIT_V1, 0);
    jit_patch_at(endpushloop, startpushloop);
    jit_patch(noargs2);
    //*/

    jit_ldr(JIT_R0, JIT_V2);
    jit_finishr(JIT_R0);
    jit_retval(JIT_R0);
    jit_retr(JIT_R0);

    plisp_fn_t apply = jit_emit();
    jit_clear_state();

    jit_disassemble();

    return apply;
}


plisp_t plisp_builtin_disassemble(plisp_t *clos, size_t nargs, plisp_t expr) {
    plisp_assert(nargs == 1);
    plisp_disassemble_fn(plisp_closure_fun(expr));
    return plisp_unspec;
}

plisp_t plisp_builtin_hashq(plisp_t *clos, size_t nargs, plisp_t obj,
                            plisp_t bits) {
    plisp_assert(nargs == 2);
    return plisp_make_fixnum((((uintptr_t) obj) * 11400714819323198485llu)
                             >> (64 - plisp_fixnum_value(bits)));
}
