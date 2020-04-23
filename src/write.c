#include <plisp/write.h>
#include <ctype.h>

static void plisp_c_write_cons(FILE *f, plisp_t obj) {
    plisp_t car = plisp_car(obj);
    plisp_t cdr = plisp_cdr(obj);

    plisp_c_write(f, car);

    if (plisp_c_nullp(cdr)) {
        return;
    } else if (plisp_c_consp(cdr)) {
        fprintf(f, " ");
        plisp_c_write_cons(f, cdr);
    } else {
        fprintf(f, " . ");
        plisp_c_write(f, cdr);
    }
}

static void plisp_c_write_string(FILE *f, plisp_t obj) {
    fputc('"', f);
    const char *str = plisp_string_value(obj);
    for (size_t i = 0; i < plisp_c_stringlen(obj); ++i) {
        char ch = str[i];
        if (ch == '"') {
            fprintf(f, "\\\"");
        } else if (ch == '\\') {
            fprintf(f, "\\\\");
        } else if (ch == '\n') {
            fprintf(f, "\\n");
        } else if (ch == '\r') {
            fprintf(f, "\\r");
        } else if (ch == '\e') {
            fprintf(f, "\\e");
        } else if (ch == '\t') {
            fprintf(f, "\\t");
        } else if (isprint(ch)) {
            fputc(ch, f);
        } else {
            fprintf(f, "\\x%X", (unsigned char)ch);
        }
    }
    fputc('"', f);
}

static void plisp_c_write_vector(FILE *f, plisp_t obj) {
    fprintf(f, "#(");
    for (size_t i = 0; i < plisp_vector_c_length(obj); ++i) {
        if (i != 0) {
            fprintf(f, " ");
        }
        plisp_c_write(f, plisp_vector_ref(obj, i));
    }
    fprintf(f, ")");
}

void plisp_c_write(FILE *f, plisp_t obj) {
    if (plisp_c_consp(obj)) {
        fprintf(f, "(");
        plisp_c_write_cons(f, obj);
        fprintf(f, ")");
    } else if (plisp_c_fixnump(obj)) {
        fprintf(f, "%li", plisp_fixnum_value(obj));
    } else if (plisp_c_boolp(obj)) {
        if (plisp_bool_value(obj)) {
            fprintf(f, "#t");
        } else {
            fprintf(f, "#f");
        }
    } else if (plisp_c_stringp(obj)) {
        plisp_c_write_string(f, obj);
    } else if (plisp_c_symbolp(obj)) {
        fprintf(f, "%s", plisp_string_value(plisp_symbol_name(obj)));
    } else if (plisp_c_vectorp(obj)) {
        plisp_c_write_vector(f, obj);
    } else if (plisp_c_nullp(obj)) {
        fprintf(f, "()");

    } else if (plisp_c_closurep(obj)) {
        fprintf(f, "#<compiled closure>");
    } else if (obj == plisp_unspec) {
        fprintf(f, "#<unspecified>");
    } else if (obj == plisp_unbound) {
        fprintf(f, "#<unbound>");
    } else {
        fprintf(f, "#?");
    }
}
