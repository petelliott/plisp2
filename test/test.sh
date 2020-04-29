#!/usr/bin/env bash

DIR=$1

code=0

for test in $(find $DIR -name '*.scm')
do
    out="$(echo $test | cut -f 1 -d '.').out"
    if PLISP_BOOT=scm/boot.scm ./plisp $test | diff - $out
    then
        echo -e "\e[32;1mPASS\e[0m" $test
    else
        code=1
        echo -e "\e[31;1mFAIL\e[0m" $test
    fi
done

exit $code
