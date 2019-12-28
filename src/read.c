#include <plisp/read.h>
#include <ctype.h>
#include <stdlib.h>
#include <Judy.h>

static plisp_t make_interned_symbol(const char *text);

// Judy's API makes no sense
static Pvoid_t intern_table = NULL;

plisp_t plisp_eof = plisp_nil;

bool plisp_c_eofp(plisp_t obj) {
    return obj == plisp_eof;
}

void plisp_init_reader(void) {
    plisp_eof = plisp_make_custom(make_interned_symbol("eof"), NULL);
}

plisp_t plisp_intern(plisp_t sym) {
    const unsigned char *key = (const unsigned char *)
        plisp_string_value(plisp_symbol_name(sym));
    plisp_t *val;
    JSLG(val, intern_table, key);
    if (val != NULL) {
        return *val;
    }

    JSLI(val, intern_table, key);
    *val = sym;

    return sym;
}

static plisp_t plisp_read_list(FILE *f) {
    int ch;
    do {
        ch = fgetc(f);
    } while (isspace(ch));

    if (ch == EOF) {
        fprintf(stderr, "error while parsing list: unexpected EOF\n");
        return plisp_eof;
    }

    if (ch == ')') {
        return plisp_nil;
    } else if (ch == '.') {
        plisp_t cdr = plisp_c_read(f);
        do {
            ch = fgetc(f);
        } while (isspace(ch));

        if (ch != ')') {
            if (ch == EOF) {
                fprintf(stderr, "error while parsing cons cell: "
                        "expected ')', got EOF\n");
                return plisp_eof;
            } else {
                fprintf(stderr, "error while parsing cons cell: "
                        "expected ')', got '%c'\n", ch);
            }
            return plisp_nil;
        }

        return cdr;
    } else {
        ungetc(ch, f);
        plisp_t car = plisp_c_read(f);
        plisp_t cdr = plisp_read_list(f);

        return plisp_cons(car, cdr);
    }
}


static int hexdig(char dig) {
    if (dig >= 'A' && dig <= 'F') {
        return 0xa + (dig-'A');
    } else if (dig >= 'a' && dig <= 'f') {
        return 0xA + (dig-'a');
    } else if (dig >= '0' && dig <= '9') {
        return dig - '0';
    }
    return -1;
}

static char read_escape(FILE *f) {
    char ch = fgetc(f);
    if(ch == 'n') {
        return '\n';
    } else if (ch == 't') {
        return '\t';
    } else if (ch == 'r') {
        return '\r';
    } else if (ch == 'e') {
        return '\e';
    } else if (ch == '\\') {
        return '\\';
    } else if (isdigit(ch)) {
        return (ch-'0')*64 + (fgetc(f)-'0')*8 + (fgetc(f)-'0');
    } else if (isdigit(ch)) {
        return (ch-'0')*64 + (fgetc(f)-'0')*8 + (fgetc(f)-'0');
    } else if (ch=='x') {
        return hexdig(fgetc(f))*16 + hexdig(fgetc(f));
    } else {
        return ch;
    }
}

static const size_t STR_INIT_CAP = 16;

static plisp_t plisp_read_string(FILE *f) {
    char ch;
    char *str = malloc(STR_INIT_CAP);
    size_t len = 0;
    size_t cap = STR_INIT_CAP;

    while ((ch = fgetc(f)) != '"') {
        if (ch == EOF) {
            fprintf(stderr, "error while parsing string: unexpected EOF\n");
            return plisp_eof;
        }
        if (ch == '\\') {
            ch = read_escape(f);
        }
        str[len] = ch;
        len++;
        if (len+1 == cap) {
            cap = (cap * 5) / 4;
            str = realloc(str, cap);
        }
    }
    str[len] = 0;
    plisp_t newstr = plisp_make_string(str);
    free(str);
    return newstr;
    return newstr;
}

static bool symchar(int ch) {
    return !isspace(ch) && ch != EOF && ch != ')' &&
           ch != '(' && ch != '"';
}

static plisp_t make_interned_symbol(const char *text) {
    return plisp_intern(plisp_make_symbol(text));
}

static plisp_t plisp_read_symbol(FILE *f) {
    //TODO: unbounded symbols
    char text[1024];
    size_t off = 0;

    char ch;
    while (symchar(ch = fgetc(f))) {
        text[off] = ch;
        off++;
    }
    ungetc(ch, f);

    text[off] = '\0';
    return make_interned_symbol(text);
}

static plisp_t plisp_read_number(FILE *f) {
    //TODO: floats
    //TODO: negatives
    long num = 0;

    int ch;
    while (true) {
        ch = fgetc(f);
        if (isdigit(ch)) {
            num = num * 10 + (ch - '0');
        } else {
            ungetc(ch, f);
            break;
        }
    }

    return plisp_make_fixnum(num);
}

plisp_t plisp_read_call(FILE *f, const char *sym) {
    plisp_t body = plisp_c_read(f);
    return plisp_cons(
        make_interned_symbol(sym),
        plisp_cons(
            body,
            plisp_nil));
}

/*
TODO
plisp_t plisp_read_hash(FILE *f) {
    plisp_t *sym = plisp_read_symbol(f);

    if (plisp_c_eq(sym, fsym)) {
        return plisp_make_bool(false);
    } else if (plisp_c_eq(sym, tsym)) {
        return plisp_make_bool(true);
    }

    return NULL;
}
*/

plisp_t plisp_c_read(FILE *f) {
    int ch;
    do {
        ch = fgetc(f);
    } while (isspace(ch));

    if (ch == EOF) {
        return plisp_eof;
    }

    if (ch == '(') {
        return plisp_read_list(f);
    } else if (ch == '"') {
        return plisp_read_string(f);
    } else if (ch == '\'') {
        return plisp_read_call(f, "quote");
    } else if (ch == '`') {
        return plisp_read_call(f, "quasiquote");
    } else if (ch == ',') {
        return plisp_read_call(f, "unquote");
    } else if (ch == '#') {
        //return plisp_read_hash(f);
        return plisp_nil;
    } else if (isdigit(ch)) {
        ungetc(ch, f);
        return plisp_read_number(f);
    } else {
        ungetc(ch, f);
        return plisp_read_symbol(f);
    }

    return plisp_nil;
}
