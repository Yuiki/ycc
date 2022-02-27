CFLAGS=-std=c11 -g -static -Wall

SRC=src
BUILD=build

SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/%.o)

$(BUILD)/ycc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

test: $(BUILD)/ycc
	$(CC) -c test.c -o build/test.o
	./test.sh

clean:
	rm -f $(BUILD)/*

.PHONY: test clean
