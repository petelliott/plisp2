#ifndef PLISP_OBJECT_H
#define PLISP_OBJECT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// we have 4 bits, each plisp object is aligned to 16 byte boundaries.
// this is at the lowest 4 bits of each word. a cons with a null
// pointer is the value nil
enum plisp_lotag {
    LT_FIXNUM = 0, // 60 bit signed integer
    LT_HITAGS = 1,
    LT_CONS   = 2,
    LT_OBJ    = 3,
    LT_FN     = 4,
    LT_CLOS   = 5,
    LT_SYM    = 6,
    LT_VECTOR = 7,
};

// when LT_HITAGS is used, the higher 4 bits of the lowest byte is
// used for the following tags
enum plisp_hitag {
    HT_BOOL = 8, // second lowest byte is 1 for #t, 0 for #f
    HT_CHAR = 16, // highest 32-bits is a UTF-32 char
};

typedef uintptr_t plisp_t;

bool plisp_c_fixnump(plisp_t val);
plisp_t plisp_make_fixnum(int64_t val);
int64_t plisp_fixnum_value(plisp_t val);

bool plisp_c_boolp(plisp_t val);
plisp_t plisp_make_bool(bool val);
bool plisp_bool_value(plisp_t val);

bool plisp_c_charp(plisp_t val);
plisp_t plisp_make_char(char val);
char plisp_char_value(plisp_t val);

bool plisp_c_consp(plisp_t val);
plisp_t plisp_cons(plisp_t car, plisp_t cdr);
plisp_t plisp_car(plisp_t cons);
plisp_t plisp_cdr(plisp_t cons);

bool plisp_c_nullp(plisp_t val);
plisp_t plisp_nil(void);

bool plisp_c_symbolp(plisp_t val);
plisp_t plisp_make_symbol(const char *string);
const char *plisp_symbol_value(plisp_t val);

#endif
