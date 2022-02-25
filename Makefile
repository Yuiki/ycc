CFLAGS=-std=c11 -g -static

SRC=src
BUILD=build

SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/%.o)

$(BUILD)/9cc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

test: $(BUILD)/9cc
	./test.sh

clean:
	rm -f $(BUILD)/*

.PHONY: test clean
