.PHONY: all test clean debug
all: grid

CFLAGS= -g3 -std=c99 -O2 -rdynamic -Wall -fPIC -shared -Wno-missing-braces

grid: grid.so
grid.so: luabinding.c grid.c node_freelist.c intlist.c
	gcc $(CFLAGS) -o $@ $^

all: test
test:
	lua test.lua

clean:
	rm -f *.so

debug: CFLAGS += -DDEBUG
debug: grid test
