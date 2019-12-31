#include <plisp/compile.h>
#include <lightning.h>
#include <Judy.h>
#include <plisp/read.h>
#include <plisp/gc.h>
#include <plisp/toplevel.h>
#include <assert.h>

static plisp_t lambda_sym;
static plisp_t if_sym;
static plisp_t quote_sym;

void plisp_init_compiler(char *argv0) {
    init_jit(argv0);
    lambda_sym = plisp_intern(plisp_make_symbol("lambda"));
    if_sym = plisp_intern(plisp_make_symbol("if"));
    quote_sym = plisp_intern(plisp_make_symbol("quote"));
}

void plisp_end_compiler(void) {
    finish_jit();
}


struct lambda_state {
    jit_state_t *jit;
    Pvoid_t arg_table;
    int stack_max;
    int stack_cur;
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

static void pop(struct lambda_state *_state, int reg) {
    if (reg != -1) {
        jit_ldxi(reg, JIT_FP, _state->stack_cur);
    }
    _state->stack_cur += sizeof(plisp_t);
}

static void assert_is_closure(plisp_t clos) {
    assert(plisp_c_closurep(clos));
}

static void plisp_compile_expr(struct lambda_state *_state, plisp_t expr);

static void plisp_compile_call(struct lambda_state *_state, plisp_t expr) {
    int args[128];
    int nargs = 0;
    for (plisp_t arglist = plisp_cdr(expr);
         arglist != plisp_nil; arglist = plisp_cdr(arglist)) {

        plisp_compile_expr(_state, plisp_car(arglist));
        args[nargs++] = push(_state, JIT_R0);
    }

    plisp_compile_expr(_state, plisp_car(expr));
    push(_state, JIT_R0);

    // assert that we are calling a closure
    jit_prepare();
    jit_pushargr(JIT_R0);
    jit_finishi(assert_is_closure);

    pop(_state, JIT_R0);

    jit_note(__FILE__, __LINE__);
    jit_prepare();
    jit_pushargi(nargs);
    for (int i = 0; i < nargs; ++i) {
        jit_ldxi(JIT_R1, JIT_FP, args[i]);
        jit_pushargr(JIT_R1);
    }
    for (int i = 0; i < nargs; ++i) {
        pop(_state, -1);
    }
    // inline closure call (change whenever plisp_closure changes)
    jit_andi(JIT_R0, JIT_R0, ~LOTAGS);
    jit_ldxi(JIT_R0, JIT_R0, sizeof(struct plisp_closure_data *));
    jit_finishr(JIT_R0);
    jit_retval(JIT_R0);
    jit_note(__FILE__, __LINE__);
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

static void plisp_compile_expr(struct lambda_state *_state, plisp_t expr) {
    if (plisp_c_consp(expr)) {
        if (plisp_car(expr) == lambda_sym) {
            plisp_fn_t fun = plisp_compile_lambda(expr);
            jit_prepare();
            jit_pushargi((jit_word_t) NULL);
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
        } else {
            plisp_compile_call(_state, expr);
        }
    } else if (plisp_c_symbolp(expr)) {
        int *pval;
        JLG(pval, _state->arg_table, expr);
        if (pval != NULL) {
            jit_ldxi(JIT_R0, JIT_FP, *pval);
            return;
        }

        plisp_t *tl_slot = plisp_toplevel_ref(expr);
        jit_ldi(JIT_R0, tl_slot);
    } else {
        jit_movi(JIT_R0, expr);
    }
}

plisp_fn_t plisp_compile_lambda(plisp_t lambda) {
    // maps argument names to nodes
    struct lambda_state state = {
        .jit = jit_new_state(),
        .arg_table = NULL,
        .stack_max = 0,
        .stack_cur = 0
    };

    struct lambda_state *_state = &state;

    assert(plisp_car(lambda) == lambda_sym);

    jit_prolog();

    //skip number of args for now
    jit_arg();
    for (plisp_t arglist = plisp_car(plisp_cdr(lambda));
         arglist != plisp_nil; arglist = plisp_cdr(arglist)) {

        int *pval;
        JLI(pval, _state->arg_table, plisp_car(arglist));

        jit_getarg(JIT_R0, jit_arg());
        *pval = push(_state, JIT_R0);
    }

    for (plisp_t exprlist = plisp_cdr(plisp_cdr(lambda));
         exprlist != plisp_nil; exprlist = plisp_cdr(exprlist)) {

        plisp_compile_expr(_state, plisp_car(exprlist));
    }
    jit_retr(JIT_R0);

    size_t Rc_word;
    JLFA(Rc_word, _state->arg_table);

    plisp_fn_t fun = jit_emit();
    jit_clear_state();

    //printf("lambda:\n");
    //jit_disassemble();

    return fun;
}
