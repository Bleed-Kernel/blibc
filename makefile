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

HEADERS := $(shell find $(INC_DIR) -type f)
SYSROOT_STAMP := $(SYSROOT)/.stamp

.PHONY: all sysroot clean

all: sysroot

sysroot: $(SYSROOT_STAMP)

# rebuilt from scratch so headers removed from include/ don't linger, but only when something
# changed, and built off to the side then swapped in. programs building at the same time all
# call this and copy out of it, an rm -rf under them was what broke parallel builds
$(SYSROOT_STAMP): $(LIBC_A) $(START) $(HEADERS)
	@rm -rf $(SYSROOT).tmp
	@mkdir -p $(SYSROOT).tmp/lib $(SYSROOT).tmp/include
	cp $(LIBC_A) $(START) $(SYSROOT).tmp/lib/
	cp -r $(INC_DIR)/* $(SYSROOT).tmp/include/
	@touch $(SYSROOT).tmp/.stamp
	@rm -rf $(SYSROOT) && mv $(SYSROOT).tmp $(SYSROOT)

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
	rm -rf $(SYSROOT) $(SYSROOT).tmp
