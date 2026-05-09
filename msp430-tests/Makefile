#
#
#
#

PLATFORM=msp430g2231

# toolchain

PREFIX=msp430

CC      = ${PREFIX}-gcc
LD      = ${PREFIX}-ld
ASM     = ${PREFIX}-as
OBJ	= ${PREFIX}-objdump

FLASH	= mspdebug

CFLAGS = -mmcu=${PLATFORM} -std=c99 -O0 -g -Wall -Werror
LFLAGS = -mmcu=${PLATFORM}

# paths

VPATH += src
OBJ_DIR = out

#

all:
	@echo "Targets: blink pir"

# blink

blink: $(OBJ_DIR)/blink.elf
	cp $(OBJ_DIR)/$@.elf $(OBJ_DIR)/fw.elf

BLINK_SRCS := blink.c
BLINK_OBJS := $(BLINK_SRCS:.c=.o)
BLINK_OBJS := $(addprefix $(OBJ_DIR)/,$(BLINK_OBJS))

$(OBJ_DIR)/blink.elf: $(BLINK_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $(BLINK_OBJS)

# pir

pir: $(OBJ_DIR)/pir.elf
	cp $(OBJ_DIR)/$@.elf $(OBJ_DIR)/fw.elf

PIR_SRCS := pir.c
PIR_OBJS := $(PIR_SRCS:.c=.o)
PIR_OBJS := $(addprefix $(OBJ_DIR)/,$(PIR_OBJS))

$(OBJ_DIR)/pir.elf: $(PIR_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $(PIR_OBJS)

#

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

#

flash:
	${FLASH} rf2500 "prog ${PWD}/${OBJ_DIR}/fw.elf"
	${FLASH} rf2500 "reset"

debug:
	${FLASH} rf2500

dump:
	${OBJ} -SD ${OBJ_DIR}/fw.elf

#

ctags:
	ctags -a -R *[ch]

clean:
	rm -rf ${OBJ_DIR}
