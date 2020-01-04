#ifndef PLISP_SAFTEY_H
#define PLISP_SAFTEY_H

#include <assert.h>

#ifdef PLISP_UNSAFE
#define plisp_assert(exp)
#else
#define plisp_assert(exp) assert(exp)
#endif

#endif
