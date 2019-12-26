#include <plisp/object.h>
#include <assert.h>

const uintptr_t LOTAGS = 0x0F;
const uintptr_t HITAGS = 0xFF;
const uintptr_t LOSHIFT = 4;
const uintptr_t HISHIFT = 8;

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

bool plisp_c_consp(plisp_t val);
plisp_t plisp_cons(plisp_t car, plisp_t cdr);
plisp_t plisp_car(plisp_t cons);
plisp_t plisp_cdr(plisp_t cons);

bool plisp_c_nullp(plisp_t val) {
    return val == (0lu | LT_CONS);
}

plisp_t plisp_nil(void) {
    return 0lu | LT_CONS;
}

bool plisp_c_symbolp(plisp_t val);
plisp_t plisp_make_symbol(const char *string);
const char *plisp_symbol_value(plisp_t val);
