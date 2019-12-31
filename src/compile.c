#include <plisp/compile.h>
#include <lightning.h>
#include <Judy.h>
#include <plisp/read.h>
#include <plisp/gc.h>
#include <assert.h>

static plisp_t lambda_sym;
static plisp_t plus_sym;

void plisp_init_compiler(char *argv0) {
    init_jit(argv0);
    lambda_sym = plisp_intern(plisp_make_symbol("lambda"));
    plus_sym = plisp_intern(plisp_make_symbol("+"));
}

void plisp_end_compiler(void) {
    finish_jit();
}

static void plisp_compile_expr(jit_state_t *_jit, plisp_t expr,
                               Pvoid_t *arg_table);

static void plisp_compile_plus(jit_state_t *_jit, plisp_t expr,
                               Pvoid_t *arg_table) {

    plisp_compile_expr(_jit, plisp_car(plisp_cdr(expr)), arg_table);

    for (plisp_t numlist = plisp_cdr(plisp_cdr(expr));
         numlist != plisp_nil; numlist = plisp_cdr(numlist)) {
        //TODO actually use stack properly/register allocation
        int save = jit_allocai(sizeof(plisp_t));
        jit_stxi(save, JIT_FP, JIT_R0);
        plisp_compile_expr(_jit, plisp_car(numlist), arg_table);
        jit_ldxi(JIT_R1, JIT_FP, save);
        jit_addr(JIT_R0, JIT_R0, JIT_R1);
    }
}

static void plisp_compile_call(jit_state_t *_jit, plisp_t expr,
                               Pvoid_t *arg_table) {
    jit_prepare();
    //TODO: functions with args
    plisp_compile_expr(_jit, plisp_car(expr), arg_table);
    // inline closure call (change whenever plisp_closure changes)
    jit_andi(JIT_R0, JIT_R0, ~LOTAGS);
    jit_ldxi(JIT_R0, JIT_R0, sizeof(struct plisp_closure_data *));
    jit_finishr(JIT_R0);
    jit_retval(JIT_R0);
}

static void plisp_compile_expr(jit_state_t *_jit, plisp_t expr,
                               Pvoid_t *arg_table) {
    if (plisp_c_consp(expr)) {
        if (plisp_car(expr) == plus_sym) {
            plisp_compile_plus(_jit, expr, arg_table);
        } else if (plisp_car(expr) == lambda_sym) {
            plisp_fn_t fun = plisp_compile_lambda(expr);
            jit_prepare();
            jit_pushargi((jit_word_t) NULL);
            jit_pushargi((jit_word_t) fun);
            jit_finishi(plisp_make_closure);
            jit_retval(JIT_R0);
        } else {
            plisp_compile_call(_jit, expr, arg_table);
            //assert(false); //TODO: function calls
        }
    } else if (plisp_c_symbolp(expr)) {
        jit_node_t **pval;
        JLG(pval, arg_table, expr);
        assert(pval != NULL);
        jit_getarg(JIT_R0, *pval);
    } else {
        jit_movi(JIT_R0, expr);
    }
}

plisp_fn_t plisp_compile_lambda(plisp_t lambda) {
    // maps argument names to nodes
    Pvoid_t arg_table = NULL;
    jit_state_t *_jit = jit_new_state();

    assert(plisp_car(lambda) == lambda_sym);

    jit_prolog();
    for (plisp_t arglist = plisp_car(plisp_cdr(lambda));
         arglist != plisp_nil; arglist = plisp_cdr(arglist)) {

        jit_node_t **pval;
        JLI(pval, arg_table, plisp_car(arglist));
        *pval = jit_arg();
    }

    for (plisp_t exprlist = plisp_cdr(plisp_cdr(lambda));
         exprlist != plisp_nil; exprlist = plisp_cdr(exprlist)) {

        plisp_compile_expr(_jit, plisp_car(exprlist), &arg_table);
    }
    //jit_movi(JIT_R0, plisp_nil);
    jit_retr(JIT_R0);

    size_t Rc_word;
    JLFA(Rc_word, arg_table);

    plisp_fn_t fun = jit_emit();
    jit_clear_state();

    return fun;
}
