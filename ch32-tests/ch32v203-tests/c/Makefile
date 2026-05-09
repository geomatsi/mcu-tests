#
#
#

CROSS_COMPILE ?= riscv32-unknown-elf-
CH32V203EVT ?= /path/to/CH32V203EVT

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

ASFLAGS = -march=rv32imac_zicsr
CCFLAGS = -march=rv32imac_zicsr -mabi=ilp32 -std=gnu99 -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -Werror -fno-stack-protector
LDFLAGS = -nostdlib --gc-sections --fatal-warnings

# suppress warning about RWX SRAM:
LDFLAGS += --no-warn-rwx-segment

LIBGCC = $(shell $(CC) -march=rv32 -mabi=ilp32 -print-libgcc-file-name)

CCFLAGS += -Ievt/include \
	   -Icommon/include \
	   -I$(CH32V203EVT)/EXAM/SRC/Peripheral/inc \
	   -I$(CH32V203EVT)/EXAM/SRC/Core

VPATH += src \
	 startup \
	 evt/src \
	 common/src \
	 $(CH32V203EVT)/EXAM/SRC/Peripheral/src

all:
	@echo "Examples: test1 .. test4"


# test1: simple test: no-op loop

TEST1_SRCS := start1.c t1.c
TEST1_OBJS := $(TEST1_SRCS:.c=.o)
TEST1_LDS := ld/flash.ld

test1: $(TEST1_LDS) $(TEST1_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test2: blink: 'nop' based delay

TEST2_SRCS := start1.c t2.c ch32v20x_rcc.c ch32v20x_gpio.c
TEST2_OBJS := $(TEST2_SRCS:.c=.o)
TEST2_LDS := ld/flash.ld

test2: $(TEST2_LDS) $(TEST2_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test3: blink: systick based delay

TEST3_SRCS := start1.c t3.c ch32v20x_rcc.c ch32v20x_gpio.c system_ch32v20x.c
TEST3_OBJS := $(TEST3_SRCS:.c=.o)
TEST3_LDS := ld/flash.ld

test3: $(TEST3_LDS) $(TEST3_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test4: blink: 'systick' based delay and startup code in C

TEST4_SRCS := start2.c t3.c ch32v20x_rcc.c ch32v20x_gpio.c system_ch32v20x.c
TEST4_OBJS := $(TEST4_SRCS:.c=.o)
TEST4_LDS := ld/flash.ld

test4: $(TEST4_LDS) $(TEST4_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# common

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(OPTS) $(CCFLAGS) -c $< -o $@

tags:
	ctags -a -R . $(CH32V203EVT)

clean:
	rm -rf *.o
	rm -rf test*

.PHONY: all run clean tags
