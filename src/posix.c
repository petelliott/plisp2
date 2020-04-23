#include <plisp/posix.h>
#include <plisp/builtin.h>
#include <plisp/saftey.h>

#include <limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>


void plisp_init_posix(void) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

    plisp_define_builtin("realpath", plisp_realpath);
    plisp_define_builtin("dirname", plisp_dirname);
    plisp_define_builtin("basename", plisp_basename);

    #pragma GCC diagnostic pop
}

plisp_t plisp_realpath(plisp_t *clos, size_t nargs, plisp_t path) {
    plisp_assert(nargs == 1);

    char buf[PATH_MAX];
    char *res = realpath(plisp_string_value(path), buf);
    if (res == NULL) {
        return plisp_make_bool(false);
    } else {
        return plisp_make_string(res);
    }
}

plisp_t plisp_dirname(plisp_t *clos, size_t nargs, plisp_t path) {
    plisp_assert(nargs == 1);

    char *d = strdup(plisp_string_value(path));
    plisp_t ret = plisp_make_string(dirname(d));
    free(d);
    return ret;
}

plisp_t plisp_basename(plisp_t *clos, size_t nargs, plisp_t path) {
    plisp_assert(nargs == 1);

    char *d = strdup(plisp_string_value(path));
    plisp_t ret = plisp_make_string(basename(d));
    free(d);
    return ret;
}
