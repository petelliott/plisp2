#ifndef PLISP_POSIX_H
#define PLISP_POSIX_H

#include <plisp/object.h>

void plisp_init_posix(void);

plisp_t plisp_realpath(plisp_t *clos, size_t nargs, plisp_t path);
plisp_t plisp_dirname(plisp_t *clos, size_t nargs, plisp_t path);
plisp_t plisp_basename(plisp_t *clos, size_t nargs, plisp_t path);

#endif
