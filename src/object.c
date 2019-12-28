#include <plisp/object.h>
#include <plisp/gc.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

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
    return ((val & LOTAGS) == LT_CONS) && (val != plisp_nil());
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
    plisp_t str = plisp_make_string(string);
    return (str & ~LOTAGS) | LT_SYM;
}

plisp_t plisp_symbol_name(plisp_t val) {
    assert(plisp_c_symbolp(val));
    return (val & ~LOTAGS) | LT_STRING;
}

bool plisp_c_vectorp(plisp_t val) {
    return (val & LOTAGS) == LT_VECTOR;
}

plisp_t plisp_make_vector(enum plisp_vec_type type, uint8_t
                          elem_width, uint16_t flags, uint32_t len,
                          plisp_t initial_element, bool use_ie) {

    plisp_t vector = plisp_alloc_obj(LT_VECTOR);
    struct plisp_vector *vecptr = (void *) (vector & ~LOTAGS);

    vecptr->type       = type;
    vecptr->elem_width = elem_width;
    vecptr->flags      = flags;
    vecptr->len        = len;
    vecptr->vec        = malloc(len * elem_width);

    if (use_ie) {
        //TODO: cases for types
    }

    return vector;
}

plisp_t plisp_vector_ref(plisp_t vec, size_t idx) {
    //TODO ref vectors
    return plisp_nil();
}

bool plisp_c_stringp(plisp_t val) {
    return (val & LOTAGS) == LT_STRING;
}

plisp_t plisp_make_string(const char *string) {
    size_t len = strlen(string);
    plisp_t str = plisp_make_vector(VEC_CHAR, 0, VFLAG_IMMUTABLE, len+1,
                                    plisp_nil(), false);
    str = (str & ~LOTAGS) | LT_STRING;
    struct plisp_vector *strptr = (void *) (str & ~LOTAGS);
    strncpy(strptr->vec, string, len+1);

    return str;
}

const char *plisp_string_value(plisp_t str) {
    assert(plisp_c_stringp(str));
    struct plisp_vector *strptr = (void *) (str & ~LOTAGS);
    return strptr->vec;
}
