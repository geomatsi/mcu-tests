# probe-rs workflow

The project now uses `.cargo/config.toml` with a `probe-rs run` target runner for
`thumbv7em-none-eabihf`, so the default workflow is:

```bash
$ cargo run --bin <binary-name>
```

To flash without running:
```bash
$ cargo flash --release --chip STM32F401CEUx --bin <binary-name>
```

# cargo-make workflow

Flash a release image:
```bash
$ cargo make flash_release <binary-name>
```

Flash a debug image:
```bash
$ cargo make flash_debug <binary-name>
```

# Debug options

## Semihosting output using probe-rs

```bash
$ cargo run --bin <binary-name>
```

## OpenOCD debug

```bash
$ sudo openocd -f tools/openocd.cfg -c 'attach ()'
```
