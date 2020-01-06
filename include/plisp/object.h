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
    LT_CUSTOM = 3,
    LT_CLOS   = 4,
    LT_SYM    = 5,
    LT_VECTOR = 6,
    LT_STRING = 7,
};

// when LT_HITAGS is used, the higher 4 bits of the lowest byte is
// used for the following tags
enum plisp_hitag {
    HT_BOOL = 8, // second lowest byte is 1 for #t, 0 for #f
    HT_CHAR = 16, // highest 32-bits is a UTF-32 char
};

#define LOTAGS  0x0Flu
#define HITAGS  0xFFlu
#define LOSHIFT 4lu
#define HISHIFT 8lu

typedef uintptr_t plisp_t;

bool plisp_c_fixnump(plisp_t val);
plisp_t plisp_make_fixnum(int64_t val);
int64_t plisp_fixnum_value(plisp_t val);

bool plisp_c_boolp(plisp_t val);
plisp_t plisp_make_bool(bool val);
bool plisp_bool_value(plisp_t val);

bool plisp_c_charp(plisp_t val);
plisp_t plisp_make_char(uint32_t val);
char plisp_char_value(plisp_t val);

struct plisp_cons {
    plisp_t car;
    plisp_t cdr;
};

bool plisp_c_consp(plisp_t val);
plisp_t plisp_cons(plisp_t car, plisp_t cdr);
plisp_t plisp_car(plisp_t cons);
plisp_t plisp_cdr(plisp_t cons);

bool plisp_c_nullp(plisp_t val);
#define plisp_nil ((plisp_t) (0lu | LT_CONS))


typedef plisp_t (*plisp_fn_t)();

struct plisp_closure {
    void *data;
    plisp_fn_t fun;
};

bool plisp_c_closurep(plisp_t val);
plisp_t plisp_make_closure(plisp_t *data, plisp_fn_t fun);
struct plisp_closure_data *plisp_closure_data(plisp_t closure);
plisp_fn_t plisp_closure_fun(plisp_t closure);

enum plisp_vec_type {
    VEC_OBJ,
    VEC_INT,
    VEC_CHAR,
    VEC_FLOAT,
    VEC_BOOL,
};

#define VFLAG_IMMUTABLE (1 << 0)

struct plisp_vector {
    uint8_t  type;
    uint8_t  elem_width; // 2**elem_width = width in bytes
    uint16_t flags;
    uint32_t len;
    void     *vec;
};

bool plisp_c_symbolp(plisp_t val);
plisp_t plisp_make_symbol(const char *string);
plisp_t plisp_symbol_name(plisp_t val);

bool plisp_c_vectorp(plisp_t val);
plisp_t plisp_make_vector(enum plisp_vec_type type, uint8_t
                          elem_width, uint16_t flags, uint32_t len,
                          plisp_t initial_element, bool use_ie);
plisp_t plisp_vector_ref(plisp_t vec, size_t idx);
plisp_t plisp_vector_set(plisp_t vec, size_t idx, plisp_t value);
size_t plisp_vector_c_length(plisp_t vec);

bool plisp_c_stringp(plisp_t val);
plisp_t plisp_make_string(const char *string);
const char *plisp_string_value(plisp_t str);
size_t plisp_c_stringlen(plisp_t str);

struct plisp_custom {
    plisp_t typesym; // symbol that uniquely identifies the object
    void *data; // must be freeable
};

bool plisp_c_customp(plisp_t val);
plisp_t plisp_make_custom(plisp_t typesym, void *data);
plisp_t plisp_custom_typesym(plisp_t val);
void *plisp_custom_data(plisp_t val);

plisp_t plisp_make_consbox(plisp_t val);
plisp_t *plisp_get_consbox(plisp_t consbox);

#endif
