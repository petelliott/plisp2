#ifndef PLISP_READ_H
#define PLISP_READ_H

#include <plisp/object.h>
#include <stdio.h>

plisp_t plisp_intern(plisp_t sym);

plisp_t plisp_c_read(FILE *f);

#endif
