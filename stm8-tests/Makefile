#
#
#

PLATFORM=stm8

CC      = sdcc
FLASH	= stm8flash

CFLAGS = -m${PLATFORM} --std-c99 --Werror --out-fmt-ihx
CFLAGS += -I include

OUT_DIR = out

#

all:
	@echo "Targets: blink flash"

#

BLINK_SRCS := src/blink.c

blink: ${BLINK_SRCS}
	mkdir -p ${OUT_DIR}/
	$(CC) $(CFLAGS) $^ -o ${OUT_DIR}/
	cp ${OUT_DIR}/blink.ihx ${OUT_DIR}/firmware.ihx

#

flash:
	${FLASH} -s flash -c stlinkv2 -p stm8s103f3 -w ${OUT_DIR}/firmware.ihx

#

clean:
	rm -rf ${OUT_DIR}
