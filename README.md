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

The oldest RTIC / DMA / bitbang experiments are still in-tree, but they are now
opt-in so they do not block normal checks on the modernized dependency stack.
The default path is still:

```bash
$ cargo check --bins
```

For targeted migration work later, enable the legacy gates explicitly:

```bash
$ cargo check --bin adc-dma-test1 --features legacy_examples
$ cargo check --bin blink-timer-rtfm --features legacy_rtic
```

# Optional cargo-make / OpenOCD helpers

The existing `cargo make` tasks and `tools/openocd.cfg` helpers are still here
if you want the older OpenOCD-based flow:

```bash
$ cargo make debug
$ cargo make flash_release <binary name>
$ cargo make flash_debug <binary name>
```
