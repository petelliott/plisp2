#include <plisp/object.h>
#include <plisp/gc.h>
#include <assert.h>
#include <string.h>

bool plisp_c_fixnump(plisp_t val) {
    return (val & LOTAGS) == LT_FIXNUM;
}

plisp_t plisp_make_fixnum(int64_t val) {
    return (val << LOSHIFT) & LT_FIXNUM;
}

int64_t plisp_fixnum_value(plisp_t val) {
    assert(plisp_c_fixnump(val));
    return val >> LOSHIFT;
}

bool plisp_c_boolp(plisp_t val) {
    return (val & HITAGS) == (LT_HITAGS | HT_BOOL);
}

plisp_t plisp_make_bool(bool val) {
    return ((uintptr_t) val) << HISHIFT | LT_HITAGS | HT_BOOL;
}

bool plisp_bool_value(plisp_t val) {
    assert(plisp_c_boolp(val));
    return val >> HISHIFT;
}

bool plisp_c_charp(plisp_t val) {
    return (val & HITAGS) == (LT_HITAGS | HT_CHAR);
}

plisp_t plisp_make_char(char val) {
    //TODO: maybe support 32-bit?
    return ((uintptr_t) val) << 32 | LT_HITAGS | HT_CHAR;
}

char plisp_char_value(plisp_t val) {
    assert(plisp_c_charp(val));
    return val >> 32;
}

bool plisp_c_consp(plisp_t val) {
    return (val & LOTAGS) == LT_CONS;
}

plisp_t plisp_cons(plisp_t car, plisp_t cdr) {
    plisp_t cell = plisp_alloc_obj(LT_CONS);
    struct plisp_cons *cellptr = (void *) (cell & ~LOTAGS);
    cellptr->car = car;
    cellptr->cdr = cdr;

    return cell;
}

plisp_t plisp_car(plisp_t cons) {
    assert(plisp_c_consp(cons));
    struct plisp_cons *cellptr = (void *) (cons & ~LOTAGS);
    return cellptr->car;
}

plisp_t plisp_cdr(plisp_t cons) {
    assert(plisp_c_consp(cons));
    struct plisp_cons *cellptr = (void *) (cons & ~LOTAGS);
    return cellptr->cdr;
}

bool plisp_c_nullp(plisp_t val) {
    return val == (0lu | LT_CONS);
}

plisp_t plisp_nil(void) {
    return 0lu | LT_CONS;
}

bool plisp_c_symbolp(plisp_t val) {
    return (val & LOTAGS) == LT_SYM;
}

plisp_t plisp_make_symbol(const char *string) {
    size_t len = strlen(string);
    plisp_t sym = plisp_alloc_atomic(len+1, LT_SYM);
    char *symptr = (void *) (sym & ~LOTAGS);
    strncpy(symptr, string, len+1);
    return sym;
}

const char *plisp_symbol_value(plisp_t val) {
    return (void *) (val & ~LOTAGS);
}
