# Running C firmware on Longan Nano board

## Getting started

### Build

```bash
$ CROSS_COMPILE=/path/to/toolchain/bin/riscv32-unknown-elf- GD32VF103_BSP=/path/to/GD32VF103_Firmware_Library make
```

### Flash frmware using OpenOCD

Start OpenOCD using any suitable JTAG probe:
```bash
$ openocd -f ../support/openocd-ftdi232h.cfg
```
This repo includes OpenOCD configurations for J-Link and FT232H based probes.

Now connect to OpenOCD and flash firmware:
```bash
$ telnet localhost 4444
> flash_img firmware-noirq.bin
```
