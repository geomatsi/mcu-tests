#
#
#

CROSS_COMPILE ?= riscv32-unknown-elf-
GD32VF103_BSP ?= /path/to/GD32VF103_Firmware_Library

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

OPTS = -DHXTAL_VALUE=8000000U

ASFLAGS = -march=rv32g
CCFLAGS = -march=rv32g -std=c11 -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -Werror -fno-stack-protector
LDFLAGS = -nostdlib --gc-sections --fatal-warnings

CCFLAGS += -Iinclude \
	 -I$(GD32VF103_BSP)/Firmware/GD32VF103_standard_peripheral/Include \
	 -I$(GD32VF103_BSP)/Firmware/GD32VF103_standard_peripheral \
	 -I$(GD32VF103_BSP)/Template

VPATH += src
VPATH += $(GD32VF103_BSP)/Firmware/GD32VF103_standard_peripheral/Source

# firmware-noirq files

FW_NOIRQ_SRCS := start-noirq.c main-noirq.c gd32vf103_usart.c gd32vf103_rcu.c gd32vf103_gpio.c
FW_NOIRQ_OBJS := $(FW_NOIRQ_SRCS:.c=.o)
FW_NOIRQ_LDS := ld/flash.ld

firmware-noirq: $(FW_NOIRQ_LDS) $(FW_NOIRQ_OBJS)
	$(LD) -T$< $(LDFLAGS) $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

# common

all: firmware-noirq

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(OPTS) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o firmware-*.*

.PHONY: all run clean
