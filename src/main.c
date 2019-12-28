#include <plisp/object.h>
#include <plisp/read.h>
#include <plisp/write.h>
#include <stdio.h>
#include <assert.h>


int main(int argc, char *argv[]) {
    plisp_init_reader();

    assert(plisp_intern(plisp_make_symbol("abcd"))
           == plisp_intern(plisp_make_symbol("abcd")));
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
            plisp_c_write(stdout, obj);
            putchar('\n');
        }
    }


    return 0;
}
