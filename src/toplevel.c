#include <plisp/toplevel.h>
#include <plisp/gc.h>
#include <plisp/read.h>
#include <plisp/compile.h>
#include <Judy.h>
#include <assert.h>

static Pvoid_t toplevel_scope = NULL;

static plisp_t define_sym;
static plisp_t lambda_sym;

void plisp_init_toplevel(void) {
    define_sym = plisp_intern(plisp_make_symbol("define"));
    lambda_sym = plisp_intern(plisp_make_symbol("lambda"));
}

void plisp_toplevel_define(plisp_t sym, plisp_t value) {
    *plisp_toplevel_ref(sym) = value;
}

plisp_t *plisp_toplevel_ref(plisp_t sym) {
    plisp_t *pval;
    JLG(pval, toplevel_scope, sym);
    if (pval == NULL) {
        plisp_t box = plisp_make_consbox(plisp_nil);
        plisp_gc_permanent(box);
        JLI(pval, toplevel_scope, sym);
        *pval = box;
    }
    return plisp_get_consbox(*pval);
}

static plisp_t do_define(plisp_t form) {
    if (plisp_c_consp(plisp_car(plisp_cdr(form)))) {
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
        // value define
        plisp_toplevel_define(
            plisp_car(plisp_cdr(form)),
            plisp_toplevel_eval(
                plisp_car(plisp_cdr(plisp_cdr(form)))));
    }

    return plisp_unspec;
}

plisp_t plisp_toplevel_eval(plisp_t form) {
    if (plisp_c_consp(form)) {
        if (plisp_car(form) == define_sym) {
            return do_define(form);
        } else {
            // compile an execute an argumentless lambda function
            plisp_t lamb = plisp_cons(
                               lambda_sym,
                               plisp_cons(plisp_nil,
                                          plisp_cons(form, plisp_nil)));

            plisp_t clos = plisp_make_closure(
                               NULL, plisp_compile_lambda(lamb));


            //TODO actually pass clousre info
            return plisp_closure_fun(clos)(NULL, 0);
        }
    } else if (plisp_c_symbolp(form)) {
        plisp_t *ref = plisp_toplevel_ref(form);
        if (ref == NULL) {
            fprintf(stderr, "attempt to reference undefined variale\n");
            return plisp_nil;
        }
        return *ref;
    }
    return form;
}
