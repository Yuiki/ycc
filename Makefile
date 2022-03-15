CFLAGS=-std=c11 -g -static -Wall

SRC=src
BUILD=build

SRCS=$(wildcard $(SRC)/*.c)

GEN1_OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen1/%.o)

GEN2_ASMS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen2/%.S)
GEN2_OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen2/%.o)

GEN3_ASMS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen3/%.S)
GEN3_OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen3/%.o)

gen1: $(BUILD)/gen1/ycc

gen2: $(BUILD)/gen2/ycc

gen3: $(BUILD)/gen3/ycc

$(BUILD)/gen1/ycc: $(GEN1_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/gen1/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/gen2/ycc: $(GEN2_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/gen2/%.o: $(BUILD)/gen2/%.S
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/gen2/%.S: $(SRC)/%.c
	$(BUILD)/gen1/ycc $^ > $@

$(BUILD)/gen3/ycc: $(GEN3_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/gen3/%.o: $(BUILD)/gen3/%.S
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/gen3/%.S: $(SRC)/%.c
	$(BUILD)/gen2/ycc $^ > $@

test: $(BUILD)/gen1/ycc
	./test.sh

host:
	make clean
	make gen1
	make gen2
	make gen3

clean:
	rm -rf $(BUILD)/*
	mkdir $(BUILD)/gen1 $(BUILD)/gen2 $(BUILD)/gen3

.PHONY: test host clean

.PRECIOUS: $(BUILD)/gen2/%.S $(BUILD)/gen3/%.S
