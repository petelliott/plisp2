#include <plisp/toplevel.h>
#include <plisp/gc.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <plisp/compile.h>
#include <plisp/saftey.h>
#include <Judy.h>
#include <assert.h>

static Pvoid_t toplevel_scope = NULL;

static plisp_t define_sym;
static plisp_t lambda_sym;
static plisp_t set_sym;

void plisp_init_toplevel(void) {
    define_sym = plisp_intern(plisp_make_symbol("define"));
    lambda_sym = plisp_intern(plisp_make_symbol("lambda"));
    set_sym = plisp_intern(plisp_make_symbol("set!"));
}

void plisp_toplevel_define(plisp_t sym, plisp_t value) {
    *plisp_toplevel_ref(sym) = value;
}

plisp_t *plisp_toplevel_ref(plisp_t sym) {
    plisp_assert(plisp_c_symbolp(sym));

    plisp_t *pval;
    JLG(pval, toplevel_scope, sym);
    if (pval == NULL) {
        plisp_t box = plisp_make_consbox(plisp_unbound);
        plisp_gc_permanent(box);
        JLI(pval, toplevel_scope, sym);
        *pval = box;
    }
    return plisp_get_consbox(*pval);
}

static plisp_t do_define(plisp_t form) {
    if (plisp_c_consp(plisp_car(plisp_cdr(form)))) {
        // predifine as bound to avoid generating the runtime
        // check for bound variables when recursing
        plisp_toplevel_define(
            plisp_car(plisp_car(plisp_cdr(form))),
            plisp_unspec);
        // function define
        plisp_toplevel_define(
            plisp_car(plisp_car(plisp_cdr(form))),
            plisp_toplevel_eval(
                plisp_cons(
                    lambda_sym,
                    plisp_cons(
                        plisp_cdr(plisp_car(plisp_cdr(form))),
                        plisp_cdr(plisp_cdr(form))))));
    } else {
        // predifine as bound to avoid generating the runtime
        // check for bound variables when recursing
        if (plisp_car(plisp_cdr(plisp_cdr(form)))
            != plisp_car(plisp_cdr(form))) {

            plisp_toplevel_define(
                plisp_car(plisp_cdr(form)),
                plisp_unspec);
        }
        // value define
        plisp_toplevel_define(
            plisp_car(plisp_cdr(form)),
            plisp_toplevel_eval(
                plisp_car(plisp_cdr(plisp_cdr(form)))));
    }

    return plisp_unspec;
}

static plisp_t do_set(plisp_t form) {
    plisp_t sym = plisp_car(plisp_cdr(form));
    plisp_t value = plisp_car(plisp_cdr(plisp_cdr(form)));

    plisp_assert(*plisp_toplevel_ref(sym) != plisp_unbound);
    *plisp_toplevel_ref(sym) = value;
    return plisp_unspec;
}

plisp_t plisp_toplevel_eval(plisp_t form) {
    if (plisp_c_consp(form)) {
        if (plisp_car(form) == define_sym) {
            return do_define(form);
        } else if (plisp_car(form) == set_sym) {
            return do_set(form);
        } else {
            // compile an execute an argumentless lambda function
            plisp_t lamb = plisp_cons(
                               lambda_sym,
                               plisp_cons(plisp_nil,
                                          plisp_cons(form, plisp_nil)));
            jit_state_t *_jit;
            plisp_fn_t fn = plisp_compile_lambda(lamb, &_jit);
            plisp_t result = fn(NULL, 0);
            jit_destroy_state();
            return result;
        }
    } else if (plisp_c_symbolp(form)) {
        plisp_t *ref = plisp_toplevel_ref(form);
        if (*ref == plisp_unbound) {
            fprintf(stderr, "error: attempt to reference unbound variable '");
            plisp_c_write(stderr, form);
            fprintf(stderr, "'\n");
            return plisp_unspec;
        }
        return *ref;
    }
    return form;
}
