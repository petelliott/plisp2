#include <plisp/object.h>
#include <plisp/gc.h>
#include <plisp/saftey.h>
#include <string.h>
#include <stdlib.h>

bool plisp_c_fixnump(plisp_t val) {
    return (val & LOTAGS) == LT_FIXNUM;
}

plisp_t plisp_make_fixnum(int64_t val) {
    return (val << LOSHIFT) | LT_FIXNUM;
}

int64_t plisp_fixnum_value(plisp_t val) {
    plisp_assert(plisp_c_fixnump(val));
    return ((int64_t) val) >> LOSHIFT;
}

bool plisp_c_boolp(plisp_t val) {
    return (val & HITAGS) == (LT_HITAGS | HT_BOOL);
}

plisp_t plisp_make_bool(bool val) {
    return ((uintptr_t) val) << HISHIFT | LT_HITAGS | HT_BOOL;
}

bool plisp_bool_value(plisp_t val) {
    plisp_assert(plisp_c_boolp(val));
    return val >> HISHIFT;
}

bool plisp_c_charp(plisp_t val) {
    return (val & HITAGS) == (LT_HITAGS | HT_CHAR);
}

plisp_t plisp_make_char(uint32_t val) {
    return ((uintptr_t) val) << 32 | LT_HITAGS | HT_CHAR;
}

char plisp_char_value(plisp_t val) {
    plisp_assert(plisp_c_charp(val));
    return val >> 32;
}

bool plisp_c_consp(plisp_t val) {
    return ((val & LOTAGS) == LT_CONS) && (val != plisp_nil);
}

plisp_t plisp_cons(plisp_t car, plisp_t cdr) {
    plisp_t cell = plisp_alloc_obj(LT_CONS);
    struct plisp_cons *cellptr = (void *) (cell & ~LOTAGS);
    cellptr->car = car;
    cellptr->cdr = cdr;

    return cell;
}

plisp_t plisp_car(plisp_t cons) {
    plisp_assert(plisp_c_consp(cons));
    struct plisp_cons *cellptr = (void *) (cons & ~LOTAGS);
    return cellptr->car;
}

plisp_t plisp_cdr(plisp_t cons) {
    plisp_assert(plisp_c_consp(cons));
    struct plisp_cons *cellptr = (void *) (cons & ~LOTAGS);
    return cellptr->cdr;
}

bool plisp_c_nullp(plisp_t val) {
    return val == (0lu | LT_CONS);
}

bool plisp_c_closurep(plisp_t val) {
    return (val & LOTAGS) == LT_CLOS;
}

plisp_t plisp_make_closure(plisp_t *data, plisp_fn_t fun) {
    plisp_t closure = plisp_alloc_obj(LT_CLOS);
    struct plisp_closure *clptr = (void *) (closure & ~LOTAGS);
    clptr->data = data;
    clptr->fun = fun;

    return closure;
}

struct plisp_closure_data *plisp_closure_data(plisp_t closure) {
    plisp_assert(plisp_c_closurep(closure));
    struct plisp_closure *clptr = (void *) (closure & ~LOTAGS);
    return clptr->data;
}

plisp_fn_t plisp_closure_fun(plisp_t closure) {
    plisp_assert(plisp_c_closurep(closure));
    struct plisp_closure *clptr = (void *) (closure & ~LOTAGS);
    return clptr->fun;
}

bool plisp_c_symbolp(plisp_t val) {
    return (val & LOTAGS) == LT_SYM;
}

plisp_t plisp_make_symbol(const char *string) {
    plisp_t str = plisp_make_string(string);
    return (str & ~LOTAGS) | LT_SYM;
}

plisp_t plisp_symbol_name(plisp_t val) {
    plisp_assert(plisp_c_symbolp(val));
    return (val & ~LOTAGS) | LT_VECTOR;
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
    plisp_assert(plisp_c_vectorp(vec));
    struct plisp_vector *vecptr = (void *) (vec & ~LOTAGS);

    plisp_assert(idx < vecptr->len);

    if (vecptr->type == VEC_OBJ) {
        plisp_assert(vecptr->elem_width == sizeof(plisp_t));
        return *(plisp_t *)(vecptr->vec + vecptr->elem_width * idx);
    } else {
        //TODO
        assert(false);
    }

    return plisp_nil;
}

plisp_t plisp_vector_set(plisp_t vec, size_t idx, plisp_t value) {
    plisp_assert(plisp_c_vectorp(vec));
    struct plisp_vector *vecptr = (void *) (vec & ~LOTAGS);

    plisp_assert(idx < vecptr->len);

    if (vecptr->type == VEC_OBJ) {
        plisp_assert(vecptr->elem_width == sizeof(plisp_t));
        *(plisp_t *)(vecptr->vec + vecptr->elem_width * idx) = value;
    } else {
        //TODO
        assert(false);
    }

    return plisp_nil;
}

size_t plisp_vector_c_length(plisp_t vec) {
    plisp_assert(plisp_c_vectorp(vec));
    struct plisp_vector *vecptr = (void *) (vec & ~LOTAGS);
    return vecptr->len;
}

bool plisp_c_stringp(plisp_t val) {
    struct plisp_vector *strptr = (void *) (val & ~LOTAGS);
    return plisp_c_vectorp(val) && strptr->type == VEC_CHAR;
}

plisp_t plisp_make_string(const char *string) {
    size_t len = strlen(string);
    plisp_t str = plisp_make_vector(VEC_CHAR, 0, VFLAG_IMMUTABLE, len+1,
                                    plisp_nil, false);
    str = (str & ~LOTAGS) | LT_VECTOR;
    struct plisp_vector *strptr = (void *) (str & ~LOTAGS);
    strncpy(strptr->vec, string, len+1);

    return str;
}

const char *plisp_string_value(plisp_t str) {
    plisp_assert(plisp_c_stringp(str));
    struct plisp_vector *strptr = (void *) (str & ~LOTAGS);
    return strptr->vec;
}

size_t plisp_c_stringlen(plisp_t str) {
    plisp_assert(plisp_c_stringp(str));
    struct plisp_vector *strptr = (void *) (str & ~LOTAGS);
    return strptr->len - 1;
}

bool plisp_c_customp(plisp_t val) {
    return (val & LOTAGS) == LT_CUSTOM;
}

plisp_t plisp_make_custom(plisp_t typesym, void *data) {
    plisp_t custom = plisp_alloc_obj(LT_CUSTOM);
    struct plisp_custom *customptr = (void *) (custom & ~LOTAGS);
    customptr->typesym = typesym;
    customptr->data = data;
    return custom;
}

plisp_t plisp_custom_typesym(plisp_t val) {
    struct plisp_custom *customptr = (void *) (val & ~LOTAGS);
    return customptr->typesym;
}

void *plisp_custom_data(plisp_t val) {
    struct plisp_custom *customptr = (void *) (val & ~LOTAGS);
    return customptr->data;
}

/*
to avoid using another type, boxed pointer are a cons cell
with the cdr nil. these are used internally for mutable closures.
*/

plisp_t plisp_make_consbox(plisp_t val) {
    return plisp_cons(val, plisp_nil);
}

plisp_t *plisp_get_consbox(plisp_t consbox) {
    struct plisp_cons *cellptr = (void *) (consbox & ~LOTAGS);
    return &(cellptr->car);
}

bool plisp_c_numberp(plisp_t obj) {
    return (obj & LOTAGS) == LT_NUMBER || plisp_c_fixnump(obj);
}

bool plisp_c_integerp(plisp_t obj) {
    struct plisp_number *objptr = (void *) (obj & ~LOTAGS);

    return plisp_c_numberp(obj) && (plisp_c_fixnump(obj) ||
                                    objptr->type == NUM_BIGINT);
}

bool plisp_c_rationalp(plisp_t obj) {
    struct plisp_number *objptr = (void *) (obj & ~LOTAGS);

    return plisp_c_integerp(obj) || (plisp_c_numberp(obj) &&
                                     objptr->type == NUM_RATIONAL);
}

bool plisp_c_realp(plisp_t obj) {
    struct plisp_number *objptr = (void *) (obj & ~LOTAGS);

    return plisp_c_rationalp(obj) || (plisp_c_numberp(obj) &&
                                      objptr->type == NUM_REAL);
}

bool plisp_c_complexp(plisp_t obj) {
    struct plisp_number *objptr = (void *) (obj & ~LOTAGS);

    return plisp_c_rationalp(obj) || (plisp_c_numberp(obj) &&
                                      objptr->type == NUM_COMPLEX);
}

plisp_t plisp_fixnum_to_bignum(plisp_t val);
plisp_t plisp_make_rational(plisp_t num, plisp_t denom);
plisp_t plisp_make_real(double real);

plisp_t plisp_add(plisp_t a, plisp_t b);
plisp_t plisp_sub(plisp_t a, plisp_t b);
plisp_t plisp_mul(plisp_t a, plisp_t b);
plisp_t plisp_div(plisp_t a, plisp_t b);
plisp_t plisp_mod(plisp_t a, plisp_t b);
plisp_t plisp_pow(plisp_t a, plisp_t b);
