CFLAGS=-std=c11 -g -static

build/9cc: src/9cc.c
	cc $(CFLAGS) -o $@ $<

test: build/9cc
	./test.sh

clean:
	rm -f build/*

.PHONY: test clean
