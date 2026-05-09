#
#
#

# GNU Toolchain

GNU_ARM=/usr/local/angstrom/arm/bin

CC      = $(GNU_ARM)/arm-angstrom-linux-gnueabi-gcc
LD      = $(GNU_ARM)/arm-angstrom-linux-gnueabi-ld
ASM     = $(GNU_ARM)/arm-angstrom-linux-gnueabi-as
OBJCOPY = $(GNU_ARM)/arm-angstrom-linux-gnueabi-objcopy

# dependencies

MCU_BASE=../mcu_tests

# Options 

LDSCRIPT=lpc2378.ld
ARM_CPU=arm7tdmi
LIB=-L /usr/local/angstrom/arm/lib/gcc/arm-angstrom-linux-gnueabi/4.3.3 -lgcc 
INCLUDE=-I . -I ./system/include -I ./platform -I ${MCU_BASE}

AFLAGS = -mcpu=${ARM_CPU} 

LFLAGS = -T./${LDSCRIPT} -nostartfiles -Map=rtos.map

CFLAGS=	-g -mcpu=${ARM_CPU} ${INCLUDE} \
	-fomit-frame-pointer \
	-D PACK_STRUCT_END=__attribute\(\(packed\)\) \
	-D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) 
	#-D THUMB_INTERWORK

# sources 

ARM_SOURCE= \
		main.c \
		\
		./system/list.c \
		./system/queue.c \
		./system/tasks.c \
		\
		./platform/port.c \
		./platform/heap_2.c \
		./platform/portISR.c 
		
ARM_OBJS = ${ARM_SOURCE:.c=.o}

MCU_SOURCES = \
	      ${MCU_BASE}/emaclib.c \
	      ${MCU_BASE}/uartlib.c \
	      ${MCU_BASE}/viclib.c \
	      ${MCU_BASE}/stdlib.c \
	      ${MCU_BASE}/init.c

MCU_OBJS = ${MCU_SOURCES:.c=.o}

# make rules

all: FreeRTOS.bin

FreeRTOS.bin : FreeRTOS.elf
	${OBJCOPY} -O binary FreeRTOS.elf FreeRTOS.bin
	 
FreeRTOS.elf : boot.o ${ARM_OBJS} ${MCU_OBJS}
	${LD} ${LFLAGS} -o $@ $^ ${LIB} 

${ARM_OBJS} : %.o : %.c FreeRTOSConfig.h
	${CC} ${CFLAGS} -c $< -o $@

${MCU_OBJS} : %.o : %.c 
	${CC} ${CFLAGS} -c $< -o $@

boot.o: boot.s 
	${ASM} ${AFLAGS} -o $@ $^ 

ctags:
	#ctags -a ${ARM_SOURCE}
	ctags -a -R *

clean :
	rm ${ARM_OBJS}
	rm ${MCU_OBJS}
	rm FreeRTOS.elf
	rm FreeRTOS.bin
	rm rtos.map
	rm boot.o
	
	
	




