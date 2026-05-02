CC      ?= x86_64-elf-gcc
AR      ?= x86_64-elf-ar
AS      ?= x86_64-elf-gcc

SRC_DIR := src
INC_DIR := include/libc
BUILD   := build
SYSROOT := sysroot

LIB_DIR := $(SYSROOT)/lib
INC_OUT := $(SYSROOT)/include

LIBC_A  := blibc.a

CFLAGS  := \
	-ffreestanding \
	-fno-builtin \
	-fno-stack-protector \
	-fno-pic \
	-fno-pie \
	-nostdinc \
	-Iinclude \
	-Iinclude/libc \
	-Wall -Wextra \
	-std=gnu11 \
	-O2 \

C_SRCS  := $(sort $(shell find $(SRC_DIR) -type f -name '*.c'))
S_SRCS  := $(sort $(shell find $(SRC_DIR) -type f -name '*.S'))

OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(BUILD)/%.o) \
	$(S_SRCS:$(SRC_DIR)/%.S=$(BUILD)/%.o)

START_SRC := $(firstword $(filter %/start.S,$(S_SRCS)))
START     := $(START_SRC:$(SRC_DIR)/%.S=$(BUILD)/%.o)

.PHONY: all sysroot clean

all: sysroot

sysroot: $(LIBC_A)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(INC_OUT)
	cp $(LIBC_A) $(LIB_DIR)/
	cp $(START) $(LIB_DIR)/
	cp -r include/* $(INC_OUT)/

$(LIBC_A): $(OBJS)
	$(AR) rcs $@ $^

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
	rm -rf $(LIBC_A)
	rm -rf $(SYSROOT)
