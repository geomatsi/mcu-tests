#
#
#

CROSS_COMPILE ?= riscv64-unknown-elf-
RISCV_XLEN ?= 64

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

SPIKE ?= spike

BUILD ?= build

ifeq ($(RISCV_XLEN), 64)
	RISCV_ABI = lp$(RISCV_XLEN)
	RISCV_ELF = elf$(RISCV_XLEN)lriscv
	RISCV_ISA = rv$(RISCV_XLEN)imac_zicsr_zifencei
else
	RISCV_ABI = ilp$(RISCV_XLEN)
	RISCV_ELF = elf$(RISCV_XLEN)lriscv
	RISCV_ISA = rv$(RISCV_XLEN)imac_zicsr_zifencei
endif

ASFLAGS = -march=$(RISCV_ISA)

CCFLAGS = -march=$(RISCV_ISA) -mabi=$(RISCV_ABI) -mcmodel=medany
CCFLAGS += -ffunction-sections -fdata-sections -ffreestanding -fno-stack-protector
CCFLAGS += -Wall -Wextra -Werror -O2 -std=gnu99

LDFLAGS = -m $(RISCV_ELF)
LDFLAGS += -nostdlib --fatal-warnings --gc-sections
# FIXME: suppress warning about RWX SRAM:
LDFLAGS += --no-warn-rwx-segment

CCFLAGS += -Icommon/include
VPATH += src crt common/src

all:
	@echo "Examples: test1 test2 test3 test4"

#
# test1
#

TEST1_ASRCS := start1.S
TEST1_CSRCS := test1.c htif.c xprintf.c stdlib.c
TEST1_OBJS := $(patsubst %, $(BUILD)/%, $(TEST1_ASRCS:.S=.o))
TEST1_OBJS += $(patsubst %, $(BUILD)/%, $(TEST1_CSRCS:.c=.o))
TEST1_LDS := ld/spike.ld

test1: $(TEST1_LDS) $(TEST1_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	echo $(SPIKE) --isa=$(RISCV_ISA) $(BUILD)/$@.elf

#
# test2
#

TEST2_ASRCS := start1.S
TEST2_CSRCS := test2.c htif.c xprintf.c stdlib.c
TEST2_OBJS := $(patsubst %, $(BUILD)/%, $(TEST2_ASRCS:.S=.o))
TEST2_OBJS += $(patsubst %, $(BUILD)/%, $(TEST2_CSRCS:.c=.o))
TEST2_LDS := ld/spike.ld

test2: $(TEST2_LDS) $(TEST2_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	echo $(SPIKE) --isa=$(RISCV_ISA) $(BUILD)/$@.elf

#
# test3
#

TEST3_ASRCS := start1.S
TEST3_CSRCS := test3.c htif.c xprintf.c stdlib.c
TEST3_OBJS := $(patsubst %, $(BUILD)/%, $(TEST3_ASRCS:.S=.o))
TEST3_OBJS += $(patsubst %, $(BUILD)/%, $(TEST3_CSRCS:.c=.o))
TEST3_LDS := ld/spike.ld

test3: $(TEST3_LDS) $(TEST3_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	echo $(SPIKE) --real-time-clint --isa=$(RISCV_ISA) $(BUILD)/$@.elf

#
# test4
#

TEST4_ASRCS := start1.S
TEST4_CSRCS := test4.c htif.c xprintf.c stdlib.c opcodes.c
TEST4_OBJS := $(patsubst %, $(BUILD)/%, $(TEST4_ASRCS:.S=.o))
TEST4_OBJS += $(patsubst %, $(BUILD)/%, $(TEST4_CSRCS:.c=.o))
TEST4_LDS := ld/spike.ld

test4: $(TEST4_LDS) $(TEST4_OBJS)
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $(BUILD)/$@.elf
	echo $(SPIKE) --real-time-clint --isa=$(RISCV_ISA) $(BUILD)/$@.elf

# common

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.s | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(OPTS) $(CCFLAGS) -c $< -o $@

tags:
	ctags -a -R src crt common

clean:
	rm -rf $(BUILD)

.PHONY: all clean tags
