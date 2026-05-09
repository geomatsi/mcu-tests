#
#
#

TARGET=freertos-demo

# Toolchain

CROSS_COMPILE ?= /home/matsi/devel/tools/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux-2013.05-24/bin/arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# Project paths

SRC_DIR = src
OBJ_DIR = build
INC_DIR = include

# Vendor paths

STM_DIR ?= /home/matsi/src/stm32/STM32F4-Discovery_FW_V1.1.0
FREERTOS_DIR ?= /home/matsi/code/FreeRTOSV7.3.0/FreeRTOS

# Toolchain flags

CFLAGS = -g -Wall -O2
CFLAGS += -mcpu=cortex-m4 -mthumb -mthumb-interwork
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp

LDFLAGS = -Tstm32f4-flash.ld
LIBGCC  = $(shell $(CC) -mthumb -mcpu=cortex-m4 -print-libgcc-file-name)

# Project basic functionality

CFLAGS += -I$(INC_DIR)

DEMO_SRCS += \
	src/main.c \
	src/startup_stm32f4xx.s \
	src/system_stm32f4xx.c \
	src/stdlib.c \
	src/stubs.c \
	src/misc.c \
	src/isr.c

# ST common BSP

CFLAGS += -DUSE_STDPERIPH_DRIVER

CFLAGS += \
	-I$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/inc \
	-I$(STM_DIR)/Utilities/STM32F4-Discovery \
	-I$(STM_DIR)/Libraries/CMSIS/ST/STM32F4xx/Include \
	-I$(STM_DIR)/Libraries/CMSIS/Include

DEMO_SRCS += \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c \
	$(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c

# FreeRTOS

CFLAGS += \
	-I$(FREERTOS_DIR)/Source/include \
	-I$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM4F

DEMO_SRCS += \
	$(FREERTOS_DIR)/Source/list.c \
	$(FREERTOS_DIR)/Source/queue.c \
	$(FREERTOS_DIR)/Source/tasks.c \
	$(FREERTOS_DIR)/Source/timers.c \
	$(FREERTOS_DIR)/Source/croutine.c \
	$(FREERTOS_DIR)/Source/portable/MemMang/heap_1.c \
	$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM4F/port.c

# ST USB driver

CFLAGS += \
	-I$(STM_DIR)/Libraries/STM32_USB_Device_Library/Core/inc \
	-I$(STM_DIR)/Libraries/STM32_USB_Device_Library/Class/hid/inc \
	-I$(STM_DIR)/Libraries/STM32_USB_OTG_Driver/inc/

DEMO_SRCS += \
	$(STM_DIR)/Libraries/STM32_USB_Device_Library/Core/src/usbd_req.c \
	$(STM_DIR)/Libraries/STM32_USB_Device_Library/Core/src/usbd_core.c \
	$(STM_DIR)/Libraries/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
	$(STM_DIR)/Libraries/STM32_USB_Device_Library/Class/hid/src/usbd_hid_core.c \
	$(STM_DIR)/Libraries/STM32_USB_OTG_Driver/src/usb_core.c \
	$(STM_DIR)/Libraries/STM32_USB_OTG_Driver/src/usb_dcd.c \
	$(STM_DIR)/Libraries/STM32_USB_OTG_Driver/src/usb_dcd_int.c

# Project USB functionality

CFLAGS += -Iusb_fs_hid

DEMO_SRCS += \
	usb_fs_hid/usb_bsp.c \
	usb_fs_hid/usbd_usr.c \
	usb_fs_hid/usbd_desc.c

#

DEMO_OBJS := $(DEMO_SRCS:.c=.o)
DEMO_OBJS := $(DEMO_OBJS:.s=.o)
DEMO_OBJS := $(addprefix $(OBJ_DIR)/,$(DEMO_OBJS))

#

all: $(OBJ_DIR)/$(TARGET).elf $(OBJ_DIR)/$(TARGET).hex $(OBJ_DIR)/$(TARGET).bin

$(OBJ_DIR)/$(TARGET).elf: $(DEMO_OBJS) $(LIBGCC) stm32f4-flash.ld
	$(LD) $(LDFLAGS) $(DEMO_OBJS) $(LIBGCC) -o $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJ_DIR)

.PHONY: clean
