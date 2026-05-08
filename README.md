![Rust](https://github.com/geomatsi/rust-blue-pill-tests/workflows/Rust/badge.svg?branch=master)

# Probe-rs workflow

The project now uses `.cargo/config.toml` with a `probe-rs run` target runner for
`thumbv7m-none-eabi`, so the default happy path is:

```bash
$ cargo run --bin <binary name>
```

Flash-only with probe-rs tools:

```bash
$ cargo flash --release --chip STM32F103C8 --bin <binary name>
```

RTT / GDB session with `cargo-embed`:

```bash
$ cargo embed --bin <binary name> flash
$ cargo embed --bin <binary name>
```

# Toolchain

The repository includes `rust-toolchain.toml`, so `rustup` will pick the right
stable toolchain and `thumbv7m-none-eabi` target automatically.

# Legacy examples

The RTIC, DMA, and bitbang I2C examples are now part of the default checked bin
set. The default path is:

```bash
$ cargo check --bins
```

`pwm-test2` is feature-gated; select one PWM timer/remap feature such as
`tim2_remap_00`, `tim2_remap_01`, `tim2_remap_10`, `tim2_remap_11`,
`tim3_remap_00`, `tim3_remap_10`, or `tim4_remap_00`, then build or run with
that feature, for example:

```bash
$ cargo check --bin pwm-test2 --features tim3_remap_00
```

# Optional cargo-make / OpenOCD helpers

The existing `cargo make` tasks and `tools/openocd.cfg` helpers are still here
if you want the older OpenOCD-based flow:

```bash
$ cargo make debug
$ cargo make flash_release <binary name>
$ cargo make flash_debug <binary name>
```
