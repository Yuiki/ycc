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
	./test.sh

clean:
	rm -f $(BUILD)/*

.PHONY: test clean
