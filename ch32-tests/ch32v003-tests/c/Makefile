#
#
#

CROSS_COMPILE ?= riscv32-unknown-elf-
CH32V003EVT ?= /path/to/CH32V003EVT

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

ASFLAGS = -march=rv32e_zicsr
CCFLAGS = -march=rv32e_zicsr -mabi=ilp32e -std=gnu99 -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -Werror -fno-stack-protector
LDFLAGS = -nostdlib --gc-sections --fatal-warnings

LIBGCC = $(shell $(CC) -march=rv32e -mabi=ilp32e -print-libgcc-file-name)

CCFLAGS += -Ievt/include \
	   -Icommon/include \
	   -I$(CH32V003EVT)/EXAM/SRC/Peripheral/inc \
	   -I$(CH32V003EVT)/EXAM/SRC/Core

VPATH += src \
	 evt/src \
	 common/src \
	 $(CH32V003EVT)/EXAM/SRC/Peripheral/src

all:
	@echo "Examples: test1 .. test9"

# test1 files

TEST1_SRCS := start.c t1.c
TEST1_OBJS := $(TEST1_SRCS:.c=.o)
TEST1_LDS := ld/flash.ld

test1: $(TEST1_LDS) $(TEST1_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test2 files

TEST2_SRCS := start.c t2.c
TEST2_OBJS := $(TEST2_SRCS:.c=.o)
TEST2_LDS := ld/flash.ld

test2: $(TEST2_LDS) $(TEST2_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test3 files

TEST3_SRCS := start.c t3.c ch32v00x_rcc.c ch32v00x_gpio.c
TEST3_OBJS := $(TEST3_SRCS:.c=.o)
TEST3_LDS := ld/flash.ld

test3: $(TEST3_LDS) $(TEST3_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test4 files

TEST4_SRCS := start.c t4.c ch32v00x_rcc.c ch32v00x_gpio.c system_ch32v00x.c
TEST4_OBJS := $(TEST4_SRCS:.c=.o)
TEST4_LDS := ld/flash.ld

test4: $(TEST4_LDS) $(TEST4_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test5 files

TEST5_SRCS := start.c t5.c ch32v00x_rcc.c ch32v00x_gpio.c system_ch32v00x.c
TEST5_OBJS := $(TEST5_SRCS:.c=.o)
TEST5_LDS := ld/flash.ld

test5: $(TEST5_LDS) $(TEST5_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test6 files

TEST6_SRCS := start.c t6.c ch32v00x_rcc.c ch32v00x_gpio.c system_ch32v00x.c
TEST6_OBJS := $(TEST6_SRCS:.c=.o)
TEST6_LDS := ld/flash.ld
# suppress warning about RWX SRAM:
TEST6_LDFLAGS = --no-warn-rwx-segment

test6: $(TEST6_LDS) $(TEST6_OBJS)
	$(LD) -T$< $(LDFLAGS) $(TEST6_LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test7 files

TEST7_SRCS := start.c t7.c ch32v00x_rcc.c ch32v00x_usart.c ch32v00x_gpio.c system_ch32v00x.c
TEST7_OBJS := $(TEST7_SRCS:.c=.o)
TEST7_LDS := ld/flash.ld

test7: $(TEST7_LDS) $(TEST7_OBJS) $(LIBGCC)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test8 files

TEST8_SRCS := start.c t8.c ch32v00x_rcc.c ch32v00x_usart.c ch32v00x_gpio.c system_ch32v00x.c xprintf.c stdlib.c
TEST8_OBJS := $(TEST8_SRCS:.c=.o)
TEST8_LDS := ld/flash.ld

test8: $(TEST8_LDS) $(TEST8_OBJS) $(LIBGCC)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# test9 files

TEST9_SRCS := start.c t9.c ch32v00x_rcc.c ch32v00x_usart.c ch32v00x_gpio.c system_ch32v00x.c xprintf.c stdlib.c
TEST9_OBJS := $(TEST9_SRCS:.c=.o)
TEST9_LDS := ld/flash.ld

test9: $(TEST9_LDS) $(TEST9_OBJS) $(LIBGCC)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# common

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(OPTS) $(CCFLAGS) -c $< -o $@

tags:
	ctags -a -R . $(CH32V003EVT)

clean:
	rm -rf *.o
	rm -rf test*

.PHONY: all run clean tags
