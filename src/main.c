#include <plisp/object.h>
#include <stdio.h>


int main() {
    printf("%lx\n", plisp_make_fixnum(-255));
    printf("%lx\n", plisp_cons(plisp_nil, plisp_nil));
    printf("%lx\n", plisp_cons(plisp_nil, plisp_nil));
    printf("%lx\n", plisp_cons(plisp_nil, plisp_nil));
    printf("%lx\n", plisp_cons(plisp_nil, plisp_nil));
}
