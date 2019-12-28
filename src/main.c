#include <plisp/object.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <stdio.h>


int main(int argc, char *argv[]) {
    if (argc > 1) {
        /*
        FILE *file = fopen(argv[1], "r");

        while (1) {
            plisp_t obj = plisp_c_read(file);
        }
        */
    } else {
        while (1) {
            printf("> ");
            plisp_t obj = plisp_c_read(stdin);
            plisp_c_write(stdout, obj);
            putchar('\n');
        }
    }


    return 0;
}
