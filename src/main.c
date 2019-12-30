#include <plisp/object.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <plisp/compile.h>
#include <plisp/toplevel.h>
#include <stdio.h>
#include <assert.h>


int main(int argc, char *argv[]) {
    plisp_init_reader();
    plisp_init_compiler(argv[0]);
    plisp_init_toplevel();

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
            if (plisp_c_eofp(obj)) {
                putchar('\n');
                break;
            }
            plisp_c_write(stdout, plisp_toplevel_eval(obj));
            putchar('\n');
        }
    }

    plisp_end_compiler();

    return 0;
}
