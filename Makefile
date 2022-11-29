.PHONY: all test clean debug grid
all: grid

CFLAGS= -g3 -std=c99 -O2 -rdynamic -Wall -fPIC -shared -Wno-missing-braces

grid: grid.so
grid.so: luabinding.c grid.c node_freelist.c intlist.c
	gcc $(CFLAGS) -o $@ $^

test:
	lua test.lua

clean:
	rm -f *.so testc

debug: CFLAGS += -DDEBUG
debug: grid

testc:
	gcc -lm -fsanitize=address -ggdb intlist.c grid.c node_freelist.c testc.c -o testc

