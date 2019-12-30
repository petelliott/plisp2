#include <plisp/toplevel.h>
#include <plisp/gc.h>
#include <plisp/read.h>
#include <Judy.h>

static Pvoid_t toplevel_scope = NULL;

static plisp_t define_sym;

void plisp_init_toplevel(void) {
    define_sym = plisp_intern(plisp_make_symbol("define"));
}

void plisp_toplevel_define(plisp_t sym, plisp_t value) {
    plisp_t *pval;
    JLI(pval, toplevel_scope, sym);
    *pval = value;
    if (plisp_heap_allocated(value)) {
        plisp_gc_permanent(value);
    }
}

plisp_t *plisp_toplevel_ref(plisp_t sym) {
    plisp_t *pval;
    JLG(pval, toplevel_scope, sym);
    return pval;
}

static plisp_t do_define(plisp_t form) {
    //TODO function defines
    plisp_toplevel_define(plisp_car(plisp_cdr(form)),
                          plisp_toplevel_eval(plisp_car(plisp_cdr(plisp_cdr(form)))));
    return plisp_car(plisp_cdr(form));
}

plisp_t plisp_toplevel_eval(plisp_t form) {
    if (plisp_c_consp(form)) {
        if (plisp_car(form) == define_sym) {
            return do_define(form);
        } else {
            // TODO create and eval lambda
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
