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
	-fpie \
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

LD       := x86_64-elf-ld
LIBC_SO  := libc.so
PIC      := $(BUILD)/pic
PIC_OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(PIC)/%.o) \
	$(filter-out $(PIC)/start.o,$(S_SRCS:$(SRC_DIR)/%.S=$(PIC)/%.o))
PIC_FLAGS := $(filter-out -fpie,$(CFLAGS)) -fPIC -DBLIBC_SHARED
CRT      := $(BUILD)/dstart.o $(BUILD)/crtdso.o

HEADERS := $(shell find $(INC_DIR) -type f)
SYSROOT_STAMP := $(SYSROOT)/.stamp

.PHONY: all sysroot clean

all: sysroot

sysroot: $(SYSROOT_STAMP)

# rebuilt from scratch so headers removed from include/ don't linger, but only when something
# changed, and built off to the side then swapped in. programs building at the same time all
# call this and copy out of it, an rm -rf under them was what broke parallel builds
$(SYSROOT_STAMP): $(LIBC_A) $(START) $(LIBC_SO) $(CRT) $(HEADERS)
	@rm -rf $(SYSROOT).tmp
	@mkdir -p $(SYSROOT).tmp/lib $(SYSROOT).tmp/include
	cp $(LIBC_A) $(START) $(LIBC_SO) $(CRT) $(SYSROOT).tmp/lib/
	cp -r $(INC_DIR)/* $(SYSROOT).tmp/include/
	@touch $(SYSROOT).tmp/.stamp
	@rm -rf $(SYSROOT) && mv $(SYSROOT).tmp $(SYSROOT)

$(LIBC_A): $(OBJS)
	rm -f $@
	$(AR) rcs $@ $^

# functions bind to themselves inside libc, data goes through the GOT so a programs copy
# of something like environ is the one everybody sees
$(LIBC_SO): $(PIC_OBJS)
	$(LD) -shared -soname libc.so -e _dlstart -Bsymbolic-functions --hash-style=both \
		--eh-frame-hdr -z now -z noexecstack -o $@ $(PIC_OBJS) $(shell $(CC) -print-libgcc-file-name)

$(PIC)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PIC_FLAGS) -MMD -MP -c $< -o $@

$(PIC)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(PIC_FLAGS) -MMD -MP -c $< -o $@

$(BUILD)/%.o: crt/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d) $(PIC_OBJS:.o=.d)

clean:
	rm -rf $(BUILD)
	rm -rf $(LIBC_A) $(LIBC_SO)
	rm -rf $(SYSROOT) $(SYSROOT).tmp
