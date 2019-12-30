CC=gcc
CFLAGS=-Wall -O2 -Iinclude/
LIBS=-lJudy -llightning
OBJS=bin/object.o bin/gc.o bin/main.o bin/read.o bin/write.o \
	bin/compile.o bin/toplevel.o

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
