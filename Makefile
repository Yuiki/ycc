CFLAGS=-std=c11 -g -static -Wall

SRC=src
BUILD=build

SRCS=$(wildcard $(SRC)/*.c)

GEN1_OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen1/%.o)

GEN2_ASMS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen2/%.S)
GEN2_OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen2/%.o)

GEN3_ASMS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen3/%.S)
GEN3_OBJS=$(SRCS:$(SRC)/%.c=$(BUILD)/gen3/%.o)

all: host

gen1: $(BUILD)/ycc_gen1

gen2: $(BUILD)/ycc_gen2

gen3: $(BUILD)/ycc_gen3

$(BUILD)/ycc_gen1: $(GEN1_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/gen1/%.o: $(SRC)/%.c
	mkdir -p $(BUILD)/gen1
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/ycc_gen2: $(GEN2_OBJS)
	mv $(BUILD)/gen2 $(BUILD)/tmp
	$(CC) $(CFLAGS) -o $@ $(GEN2_OBJS:$(BUILD)/gen2/%.o=$(BUILD)/tmp/%.o)
	mv $(BUILD)/tmp $(BUILD)/gen2

$(BUILD)/gen2/%.o: $(BUILD)/gen2/%.S
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/gen2/%.S: $(SRC)/%.c
	mkdir -p $(BUILD)/gen2
	$(BUILD)/ycc_gen1 $^ > $@

$(BUILD)/ycc_gen3: $(GEN3_OBJS)
	mv $(BUILD)/gen3 $(BUILD)/tmp
	$(CC) $(CFLAGS) -o $@ $(GEN3_OBJS:$(BUILD)/gen3/%.o=$(BUILD)/tmp/%.o)
	mv $(BUILD)/tmp $(BUILD)/gen3

$(BUILD)/gen3/%.o: $(BUILD)/gen3/%.S
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/gen3/%.S: $(SRC)/%.c
	mkdir -p $(BUILD)/gen3
	$(BUILD)/ycc_gen2 $^ > $@

test: $(BUILD)/ycc_gen1
	./test.sh

host:
	make clean
	make gen1
	make gen2
	make gen3
	md5sum $(BUILD)/ycc_gen2 $(BUILD)/ycc_gen3

clean:
	rm -rf $(BUILD)/*

.PHONY: init gen1 gen2 gen3 test host clean

.PRECIOUS: $(BUILD)/gen2/%.S $(BUILD)/gen3/%.S
