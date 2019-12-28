CC=gcc
CFLAGS=-Wall -O2 -Iinclude/
LIBS=
OBJS=bin/object.o bin/gc.o bin/main.o bin/read.o

plisp: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

bin/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -c $^ -o $@

bin:
	dirname $(OBJS) | sort -u | xargs mkdir -p

.PHONY: debug clean

debug: CFLAGS=-Wall -g -Iinclude
debug: clean plisp

clean:
	-rm -r bin
	-rm  plisp
