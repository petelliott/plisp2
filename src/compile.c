#include <plisp/compile.h>
#include <lightning.h>
#include <Judy.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <plisp/gc.h>
#include <plisp/toplevel.h>
#include <plisp/builtin.h>
#include <plisp/object.h>
#include <plisp/saftey.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

static plisp_t lambda_sym;
static plisp_t define_sym;
static plisp_t if_sym;
static plisp_t quote_sym;
static plisp_t quasiquote_sym;
static plisp_t unquote_sym;
static plisp_t unquote_splicing_sym;
static plisp_t set_sym;

// associates jit context with functions
static Pvoid_t jit_info = NULL;

void plisp_init_compiler(char *argv0) {
    init_jit(argv0);
    lambda_sym = plisp_intern(plisp_make_symbol("lambda"));
    define_sym = plisp_intern(plisp_make_symbol("define"));
    if_sym = plisp_intern(plisp_make_symbol("if"));
    quote_sym = plisp_intern(plisp_make_symbol("quote"));
    quasiquote_sym = plisp_intern(plisp_make_symbol("quasiquote"));
    unquote_sym = plisp_intern(plisp_make_symbol("unquote"));
    unquote_splicing_sym = plisp_intern(plisp_make_symbol("unquote-splicing"));
    set_sym = plisp_intern(plisp_make_symbol("set!"));
}

void plisp_end_compiler(void) {
    finish_jit();
}

struct lambda_state {
    jit_state_t *jit;
    Pvoid_t arg_table;
    int stack_max;
    int stack_cur;
    int stack_nopop;
    struct lambda_state *parent;
    Pvoid_t closure_vars;
    size_t closure_idx;
    int closure_on_stack;
};
#define _jit (_state->jit)

static int push(struct lambda_state *_state, int reg) {
    if (_state->stack_max == _state->stack_cur) {
        _state->stack_cur = jit_allocai(sizeof(plisp_t));
        _state->stack_max = _state->stack_cur;
    } else {
        _state->stack_cur -= sizeof(plisp_t);
    }
    jit_stxi(_state->stack_cur, JIT_FP, reg);
    return _state->stack_cur;
}

static int push_perm(struct lambda_state *_state, int reg) {
    // like push, but prevents popping.
    int blk = push(_state, reg);
    _state->stack_nopop = blk;
    return blk;
}

static void pop(struct lambda_state *_state, int reg) {
    if (reg != -1) {
        jit_ldxi(reg, JIT_FP, _state->stack_cur);
    }
    _state->stack_cur += sizeof(plisp_t);
    assert(_state->stack_cur <= _state->stack_nopop);
}

#ifndef PLISP_UNSAFE
static void assert_is_closure(plisp_t clos) {
    if (!plisp_c_closurep(clos)) {
        fprintf(stderr, "error: attempt to call non-closure object\n");
    }
    assert(plisp_c_closurep(clos));
}

static void assert_nargs(size_t expected, size_t got) {
    if (expected != got) {
        fprintf(stderr, "error: expected %lu args, got %lu\n", expected, got);
    }
    assert(expected == got);
}

static void assert_gt_nargs(size_t expected, size_t got) {
    if (expected > got) {
        fprintf(stderr, "error: expected >=%lu args, got %lu\n", expected, got);
    }
    assert(expected <= got);
}

static void assert_bound(plisp_t obj, plisp_t sym) {
    if (obj == plisp_unbound) {
        fprintf(stderr, "error: attempt to reference unbound variable '");
        plisp_c_write(stderr, sym);
        fprintf(stderr, "'\n");
    }
    assert(obj != plisp_unbound);
}
#endif

static void plisp_compile_expr(struct lambda_state *_state, plisp_t expr);
static plisp_fn_t plisp_compile_lambda_context(
    plisp_t lambda,
    struct lambda_state *parent_state,
    Pvoid_t *closure_vars);

static void plisp_compile_call(struct lambda_state *_state, plisp_t expr) {
    int args[128];
    int nargs = 0;
    for (plisp_t arglist = plisp_cdr(expr);
         arglist != plisp_nil; arglist = plisp_cdr(arglist)) {

        plisp_compile_expr(_state, plisp_car(arglist));
        args[nargs++] = push(_state, JIT_R0);
    }

    plisp_compile_expr(_state, plisp_car(expr));

    #ifndef PLISP_UNSAFE
    push(_state, JIT_R0);

    // assert that we are calling a closure
    jit_prepare();
    jit_pushargr(JIT_R0);
    jit_finishi(assert_is_closure);

    pop(_state, JIT_R0);
    #endif

    // inline closure call (change whenever plisp_closure changes)
    jit_andi(JIT_R0, JIT_R0, ~LOTAGS);
    jit_ldxi(JIT_R1, JIT_R0, sizeof(plisp_fn_t));

    jit_prepare();
    jit_pushargr(JIT_R1); // push closure data
    jit_pushargi(nargs);
    for (int i = 0; i < nargs; ++i) {
        jit_ldxi(JIT_R1, JIT_FP, args[i]);
        jit_pushargr(JIT_R1);
    }
    for (int i = 0; i < nargs; ++i) {
        pop(_state, -1);
    }
    jit_ldr(JIT_R0, JIT_R0);
    jit_finishr(JIT_R0);
    jit_retval(JIT_R0);
}

static void plisp_compile_if(struct lambda_state *_state, plisp_t expr) {
    // get condition into R0
    plisp_compile_expr(_state, plisp_car(plisp_cdr(expr)));

    jit_node_t *cond = jit_beqi(JIT_R0, plisp_make_bool(false));
    plisp_compile_expr(_state, plisp_car(plisp_cdr(plisp_cdr(expr))));
    jit_node_t *rest = jit_jmpi();
    jit_patch(cond);
    plisp_compile_expr(_state,
                       plisp_car(plisp_cdr
                                 (plisp_cdr(plisp_cdr(expr)))));
    jit_patch(rest);
}


static ssize_t plisp_get_closure(struct lambda_state *_state, plisp_t sym) {
    if (_state->parent == NULL) {
        return -1;
    }

    int *pval;
    JLG(pval, _state->arg_table, sym);
    // tell everyone below us to add the value to their closures
    if (pval != NULL) {
        return 0;
    }

    size_t *clnum;
    JLG(clnum, _state->closure_vars, sym);
    if (clnum != NULL) {
        return *clnum;
    }

    ssize_t idx = plisp_get_closure(_state->parent, sym);
    if (idx == -1) {
        return -1;
    } else {
        JLI(clnum, _state->closure_vars, sym);
        *clnum = _state->closure_idx;
        _state->closure_idx++;
        return *clnum;
    }
}

static void plisp_compile_ref(struct lambda_state *_state, plisp_t sym) {
    int *pval;
    JLG(pval, _state->arg_table, sym);
    if (pval != NULL) {
        jit_ldxi(JIT_R0, JIT_FP, *pval);
        return;
    }

    ssize_t closure_idx = plisp_get_closure(_state, sym);
    if (closure_idx != -1) {
        jit_ldxi(JIT_R0, JIT_FP, _state->closure_on_stack);
        jit_ldxi(JIT_R0, JIT_R0, (closure_idx+1) * sizeof(plisp_t));
        return;
    }

    plisp_t *tl_slot = plisp_toplevel_ref(sym);
    jit_ldi(JIT_R0, tl_slot);

    #ifndef PLISP_UNSAFE
    // don't generate a runtime check if the variable is bound at
    // compile time
    if (*tl_slot == plisp_unbound) {
        push(_state, JIT_R0);

        // assert that the variable we reference is bound
        jit_prepare();
        jit_pushargr(JIT_R0);
        jit_pushargi(sym);
        jit_finishi(assert_bound);

        pop(_state, JIT_R0);
    }
    #endif
}

static void plisp_compile_gen_closure(struct lambda_state *_state,
                                      Pvoid_t closure) {
    size_t num_elems;
    JLC(num_elems, closure, 0, -1);

    // don't generate a malloc call if it isn't a closure
    if (num_elems == 0) {
        jit_movi(JIT_R1, (jit_word_t) NULL);
        return;
    }

    jit_prepare();
    jit_pushargi((num_elems+1) * sizeof(plisp_t));
    jit_finishi(malloc);
    jit_retval(JIT_R1); //R1 holds the closure data

    //store the length at the start
    jit_movi(JIT_R0, (jit_word_t) num_elems);
    jit_str(JIT_R1, JIT_R0);

    size_t *off;
    plisp_t idx = 0;
    JLF(off, closure, idx);
    while (off != NULL) {
        plisp_compile_ref(_state, idx);
        jit_stxi((*off+1) * sizeof(plisp_t), JIT_R1, JIT_R0);
        JLN(off, closure, idx);
    }
}

static void plisp_compile_set(struct lambda_state *_state, plisp_t expr) {
    plisp_t sym = plisp_car(plisp_cdr(expr));
    plisp_assert(plisp_c_symbolp(sym));

    plisp_t value = plisp_car(plisp_cdr(plisp_cdr(expr)));

    int *pval;
    JLG(pval, _state->arg_table, sym);
    if (pval != NULL) {
        plisp_compile_expr(_state, value);
        jit_stxi(*pval, JIT_FP, JIT_R0);
        jit_movi(JIT_R0, plisp_unspec);
        return;
    }

    // TODO: set enclosed variables

    plisp_t *tl_slot = plisp_toplevel_ref(sym);

    #ifndef PLISP_UNSAFE
    // don't generate a runtime check if the variable is bound at
    // compile time
    if (*tl_slot == plisp_unbound) {
        // assert that the variable we reference is bound
        jit_ldi(JIT_R0, tl_slot);
        jit_prepare();
        jit_pushargr(JIT_R0);
        jit_pushargi(sym);
        jit_finishi(assert_bound);
    }
    #endif

    plisp_compile_expr(_state, value);
    jit_sti(tl_slot, JIT_R0);
    jit_movi(JIT_R0, plisp_unspec);
}

static void plisp_compile_quasiquote(struct lambda_state *_state, plisp_t expr) {
    // TODO optimize when never unquoted

    if (plisp_c_consp(expr)) {
        if (plisp_car(expr) == unquote_sym) {
            plisp_compile_expr(_state, plisp_car(plisp_cdr(expr)));
        } else {
            plisp_compile_quasiquote(_state, plisp_car(expr));
            push(_state, JIT_R0);
            plisp_compile_quasiquote(_state, plisp_cdr(expr));
            pop(_state, JIT_R1);
            jit_prepare();
            jit_pushargr(JIT_R1);
            jit_pushargr(JIT_R0);
            jit_finishi(plisp_cons);
            jit_retval(JIT_R0);
        }
    } else {
        if (plisp_heap_allocated(expr)) {
            plisp_gc_permanent(expr);
        }
        jit_movi(JIT_R0, expr);
    }

}

static void plisp_compile_expr(struct lambda_state *_state, plisp_t expr) {
    if (plisp_c_consp(expr)) {
        if (plisp_car(expr) == lambda_sym) {
            Pvoid_t closure;
            plisp_fn_t fun = plisp_compile_lambda_context(expr, _state, &closure);

            // produces closure data in JIT_R1
            plisp_compile_gen_closure(_state, closure);

            size_t Rc_word;
            JLFA(Rc_word, closure);

            jit_prepare();
            jit_pushargr(JIT_R1);
            jit_pushargi((jit_word_t) fun);
            jit_finishi(plisp_make_closure);
            jit_retval(JIT_R0);
        } else if (plisp_car(expr) == if_sym) {
            plisp_compile_if(_state, expr);
        } else if (plisp_car(expr) == quote_sym) {
            plisp_t obj = plisp_car(plisp_cdr(expr));
            if (plisp_heap_allocated(obj)) {
                plisp_gc_permanent(obj);
            }
            jit_movi(JIT_R0, obj);
        } else if (plisp_car(expr) == quasiquote_sym) {
            plisp_compile_quasiquote(_state, plisp_car(plisp_cdr(expr)));
        } else if (plisp_car(expr) == set_sym) {
            plisp_compile_set(_state, expr);
        } else {
            plisp_compile_call(_state, expr);
        }
    } else if (plisp_c_symbolp(expr)) {
        plisp_compile_ref(_state, expr);
    } else {
        jit_movi(JIT_R0, expr);
    }
}

static void plisp_compile_local_define(struct lambda_state *_state,
                                       plisp_t form) {

    plisp_t sym;
    plisp_t valexpr;

    if (plisp_c_consp(plisp_car(plisp_cdr(form)))) {
        // function define
        sym = plisp_car(plisp_car(plisp_cdr(form)));
        plisp_assert(plisp_c_symbolp(sym));

        valexpr = plisp_cons(
                      lambda_sym,
                      plisp_cons(
                          plisp_cdr(plisp_car(plisp_cdr(form))),
                          plisp_cdr(plisp_cdr(form))));
    } else {
        // value define
        sym = plisp_car(plisp_cdr(form));
        plisp_assert(plisp_c_symbolp(sym));
        valexpr = plisp_car(plisp_cdr(plisp_cdr(form)));
    }


    int *pval;
    JLI(pval, _state->arg_table, sym);

    plisp_compile_expr(_state, valexpr);
    *pval = push_perm(_state, JIT_R0);

    jit_movi(JIT_R0, plisp_unspec);
}


static void plisp_compile_stmt(struct lambda_state *_state, plisp_t expr) {
    if (plisp_c_consp(expr) && plisp_car(expr) == define_sym) {
        plisp_compile_local_define(_state, expr);
    } else {
        plisp_compile_expr(_state, expr);
    }
}

static plisp_t va_to_list(size_t nargs, va_list args) {

    plisp_t lst = plisp_nil;
    for (size_t i = 0; i < nargs; ++i) {
        lst = plisp_cons(va_arg(args, plisp_t), lst);
    }

    return plisp_c_reverse(lst);
}

static plisp_fn_t plisp_compile_lambda_context(
    plisp_t lambda,
    struct lambda_state *parent_state,
    Pvoid_t *closure_vars) {

    // maps argument names to nodes
    struct lambda_state state = {
        .jit = jit_new_state(),
        .arg_table = NULL,
        .stack_max = 0,
        .stack_cur = 0,
        .stack_nopop = 0,
        .parent = parent_state,
        .closure_vars = NULL,
        .closure_idx = 0
    };

    struct lambda_state *_state = &state;

    assert(plisp_car(lambda) == lambda_sym);

    jit_prolog();

    jit_getarg(JIT_R0, jit_arg());
    _state->closure_on_stack = push_perm(_state, JIT_R0);

    jit_getarg(JIT_R0, jit_arg());
    int nargs = push_perm(_state, JIT_R0);

    int real_nargs = 0;

    plisp_t arglist;
    for (arglist = plisp_car(plisp_cdr(lambda));
         plisp_c_consp(arglist); arglist = plisp_cdr(arglist)) {

        int *pval;
        JLI(pval, _state->arg_table, plisp_car(arglist));

        jit_getarg(JIT_R0, jit_arg());
        *pval = push_perm(_state, JIT_R0);

        real_nargs++;
    }

    if (plisp_c_nullp(arglist)) {
        #ifndef PLISP_UNSAFE
        // assert that the right number of arguments were passed
        jit_ldxi(JIT_R0, JIT_FP, nargs);
        jit_prepare();
        jit_pushargi(real_nargs);
        jit_pushargr(JIT_R0);
        jit_finishi(assert_nargs);
        #endif
    } else {
        assert(plisp_c_symbolp(arglist));
        // pass the remaining arguments as a list

        jit_ellipsis();
        jit_va_start(JIT_R0);
        int va = push(_state, JIT_R0);

        #ifndef PLISP_UNSAFE
        // make sure we get the minimum number of arguments
        jit_ldxi(JIT_R0, JIT_FP, nargs);
        jit_prepare();
        jit_pushargi(real_nargs);
        jit_pushargr(JIT_R0);
        jit_finishi(assert_gt_nargs);
        #endif

        // get a list of the remaining arguments
        jit_ldxi(JIT_R1, JIT_FP, va);
        jit_ldxi(JIT_R0, JIT_FP, nargs);
        jit_addi(JIT_R0, JIT_R0, -real_nargs);
        jit_prepare();
        jit_pushargr(JIT_R0);
        jit_pushargr(JIT_R1);
        jit_finishi(va_to_list);

        pop(_state, JIT_R1);

        int *pval;
        JLI(pval, _state->arg_table, arglist);
        jit_retval(JIT_R0);
        *pval = push_perm(_state, JIT_R0);

        jit_va_end(JIT_R1);
    }

    for (plisp_t exprlist = plisp_cdr(plisp_cdr(lambda));
         exprlist != plisp_nil; exprlist = plisp_cdr(exprlist)) {

        plisp_compile_stmt(_state, plisp_car(exprlist));
    }
    jit_retr(JIT_R0);

    size_t Rc_word;
    JLFA(Rc_word, _state->arg_table);

    plisp_fn_t fun = jit_emit();
    jit_clear_state();



    if (closure_vars != NULL) {
        *closure_vars = _state->closure_vars;
    }

    jit_state_t **pval;
    JLG(pval, jit_info, (uintptr_t) fun);
    if (pval == NULL) {
        JLI(pval, jit_info, (uintptr_t) fun);
    }
    *pval = _state->jit;

    return fun;
}

plisp_fn_t plisp_compile_lambda(plisp_t lambda) {
    return plisp_compile_lambda_context(lambda, NULL, NULL);
}

#undef _jit

void plisp_free_fn(plisp_fn_t fn) {
    jit_state_t **pval;
    JLG(pval, jit_info, (uintptr_t) fn);
    jit_state_t *_jit = *pval;
    jit_destroy_state();
}

void plisp_disassemble_fn(plisp_fn_t fn) {
    jit_state_t **pval;
    JLG(pval, jit_info, (uintptr_t) fn);
    if (pval == NULL) {
        printf("builtins cannot be disassembled\n");
    } else {
        jit_state_t *_jit = *pval;

        jit_disassemble();
    }
}
