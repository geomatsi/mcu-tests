#
#
#

CROSS_COMPILE ?= riscv32-unknown-elf-
CH32V303EVT ?= /path/to/CH32V307EVT

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

ASFLAGS = -march=rv32imafc_zicsr -mabi=ilp32f
CCFLAGS = -march=rv32imafc_zicsr -mabi=ilp32f -std=gnu99 -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -Werror -fno-stack-protector
LDFLAGS = -nostdlib --gc-sections --fatal-warnings

# suppress warning about RWX SRAM:
LDFLAGS += --no-warn-rwx-segment

LIBGCC = $(shell $(CC) -march=rv32imafc -mabi=ilp32f -print-libgcc-file-name)

CCFLAGS += -Ievt/include \
	   -Icommon/include \
	   -I$(CH32V307EVT)/EXAM/SRC/Peripheral/inc \
	   -I$(CH32V307EVT)/EXAM/SRC/Core

VPATH += src \
	 startup \
	 evt/src \
	 common/src \
	 $(CH32V307EVT)/EXAM/SRC/Peripheral/src

all:
	@echo "Examples: test1 .. test6"


# test1: simple test: no-op loop

TEST1_SRCS := start1.c t1.c
TEST1_OBJS := $(TEST1_SRCS:.c=.o)
TEST1_LDS := ld/flash1.ld

test1: $(TEST1_LDS) $(TEST1_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test2: blink: 'nop' based delay

TEST2_SRCS := start1.c t2.c ch32v30x_rcc.c ch32v30x_gpio.c
TEST2_OBJS := $(TEST2_SRCS:.c=.o)
TEST2_LDS := ld/flash1.ld

test2: $(TEST2_LDS) $(TEST2_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test3: blink: systick based delay

TEST3_SRCS := start1.c t3.c ch32v30x_rcc.c ch32v30x_gpio.c system_ch32v30x.c
TEST3_OBJS := $(TEST3_SRCS:.c=.o)
TEST3_LDS := ld/flash1.ld

test3: $(TEST3_LDS) $(TEST3_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test4: blink: 'systick' based delay and startup code in C

TEST4_SRCS := start2.c t3.c ch32v30x_rcc.c ch32v30x_gpio.c system_ch32v30x.c
TEST4_OBJS := $(TEST4_SRCS:.c=.o)
TEST4_LDS := ld/flash1.ld

test4: $(TEST4_LDS) $(TEST4_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test5: detect stack overflows using pmp

TEST5_SRCS := start2.c t5.c ch32v30x_rcc.c ch32v30x_gpio.c
TEST5_OBJS := $(TEST5_SRCS:.c=.o)
TEST5_LDS := ld/flash2.ld

test5: $(TEST5_LDS) $(TEST5_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test6: blink: 'nop' based delay with a bit of an FPU entourage

TEST6_SRCS := start2.c t6.c ch32v30x_rcc.c ch32v30x_gpio.c
TEST6_OBJS := $(TEST6_SRCS:.c=.o)
TEST6_LDS := ld/flash1.ld

test6: $(TEST6_LDS) $(TEST6_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# common

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(OPTS) $(CCFLAGS) -c $< -o $@

tags:
	ctags -a -R . $(CH32V307EVT)

clean:
	rm -rf *.o
	rm -rf test*

.PHONY: all run clean tags
