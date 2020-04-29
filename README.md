# plisp2

[![Build Status](https://travis-ci.com/Petelliott/plisp2.svg?branch=master)](https://travis-ci.com/Petelliott/plisp2)

a rewrite of the original
[plisp](https://github.com/petelliott/plisp), but with macros, tagged
pointers, just-in-time compilation, and a custom garbage collector.

## installation

install
[GNU Lighting](https://www.gnu.org/software/lightning/manual/lightning.html)
and [Judy](http://judy.sourceforge.net/).

build the project
```
$ make
```

currently, there is no easy way to install plisp2. you can run it like so:

```
$ PLISP_BOOT=scm/boot.scm rlwrap -n ./plisp
```

to run the tests:

```
$ test/test.sh test/
```

## about

I hope that someday plisp will be a full r5rs implementation

### features

- [x] just-in-time compiliation
- [x] tagged pointers
- [x] garbage collector
- [x] macros
- [ ] proper tail calls
- [x] vectors
- [ ] ffi
- [ ] module system
- [ ] continuations
- [x] apply/eval
- [x] immutable closures
- [x] mutable closures
- [ ] r5rs compliance
