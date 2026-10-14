# always the cross toolchain, the user programs link with x86_64-elf-gcc and a host cc
# built libc mixed in makes ld warn about .note.GNU-stack on every program
CC      := x86_64-elf-gcc
AR      := x86_64-elf-ar
AS      := x86_64-elf-as

SRC_DIR := src
INC_DIR := include
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
	-I$(INC_DIR) \
	-Wall -Wextra \
	-std=gnu11 \
	-O2 \

C_SRCS  := $(sort $(shell find $(SRC_DIR) -type f -name '*.c'))
S_SRCS  := $(sort $(shell find $(SRC_DIR) -type f -name '*.S'))

OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(BUILD)/%.o) \
	$(S_SRCS:$(SRC_DIR)/%.S=$(BUILD)/%.o)

START := $(BUILD)/start.o

.PHONY: all sysroot clean

all: sysroot

# the sysroot is rebuilt from scratch so headers removed from include/ don't linger
sysroot: $(LIBC_A)
	rm -rf $(SYSROOT)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(INC_OUT)
	cp $(LIBC_A) $(LIB_DIR)/
	cp $(START) $(LIB_DIR)/
	cp -r $(INC_DIR)/* $(INC_OUT)/

$(LIBC_A): $(OBJS)
	rm -f $@
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
