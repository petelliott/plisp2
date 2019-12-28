#ifndef PLISP_READ_H
#define PLISP_READ_H

#include <plisp/object.h>
#include <stdio.h>

void plisp_init_reader(void);

plisp_t plisp_intern(plisp_t sym);

plisp_t plisp_read(FILE *f);

#endif
