# Running Rust firmware on Longan Nano board

## Getting started

### Build

```bash
$ cargo build --bin <binary name>
$ cargo build --example <example name>
```

### Flash using cargo utils

Flash image using cargo-flash:
```bash
$ cargo flash --connect-under-reset --chip GD32VF103CBT6 --bin <binary name>
```

Flash image using cargo-embed:
```bash
$  cargo embed --bin <binary name> flash
```

# Flash using OpenOCD and cargo-make tool

Build and flash release image:
```bash
$ cargo make flash_release <binary name>
```
Build and flash debug image:
```bash
$ cargo make flash_debug <binary name>
```

# Flash using dfu-util and cargo-make tool

Before flashing bring the board into DFU mode: press and hold
BOOT0 button, then press RESET button, then release buttons
and run flash command.

Build and flash release image:
```bash
$ cargo make dfu_release <binary name>
```
Build and flash debug image:
```bash
$ cargo make dfu_debug <binary name>
```



### Debug using OpenOCD

Start OpenOCD using any suitable JTAG probe:
```bash
$ openocd -f ../support/openocd-ftdi232h.cfg
```
This repo includes OpenOCD configurations for J-Link and FT232H based probes.

Now run any binary or example:
```bash
$ cargo run --bin <binary name>
```

### Notes

#### Cargo flash tools and J-Link probe
At the moment probe-rs [does not yet support](https://github.com/probe-rs/probe-rs/issues/665) multiple TAPs in a JTAG chain for JLink probes.
As a result, currently J-Link probe can not be used to flash Longan Nano board with cargo-flash and cargo-embed.

#### Cargo flash tools and FTDI probes
FTDI support is [optional](https://github.com/probe-rs/cargo-flash/blob/master/README.md#ftdi-support). It can be enabled with the ftdi feature:
```bash
$ cargo install cargo-flash --features ftdi
```
