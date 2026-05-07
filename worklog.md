# Worklog

## Goal

Refresh this STM32 Blue Pill Rust test project to a current embedded Rust baseline:

- move to the current Rust edition
- update crates
- update cargo/tool configs for newer Rust tooling
- leave a record of plans, steps, commands, and ideas

## Initial Notes

- Repository appears to be a collection of standalone binaries under `src/bin/`.
- Current manifest is on Rust 2018.
- The dependency stack is old enough that some upgrades will require source changes, not only version bumps.
- RTIC examples are likely to need the largest migration if updated to current crates.

## Plan

1. Inspect the current manifest, lockfile, cargo config, and representative binaries.
2. Run a baseline `cargo check` to identify the current failure surface with the existing toolchain.
3. Update manifest editions and dependency versions to a modern compatible set.
4. Update `.cargo` and related config files for current Rust and probe tooling conventions.
5. Fix source-level breakage introduced by dependency upgrades.
6. Regenerate the lockfile and verify with `cargo check`.
7. Record the final outcome and any remaining constraints.

## Commands Run

1. `pwd`
2. `rg --files`
3. `git status --short`
4. `ls -la`
5. `sed -n '1,220p' Cargo.toml`
6. `sed -n '1,220p' Cargo.lock`
7. `sed -n '1,220p' Makefile.toml`
8. `sed -n '1,220p' Embed.toml`
9. `find . -maxdepth 3 \( -name '.cargo' -o -name 'rust-toolchain*' -o -name 'build.rs' -o -name '*.toml' \) | sort`
10. `find .cargo -maxdepth 2 -type f | sort | xargs -r sed -n '1,220p'`
11. `sed -n '1,220p' README.md`
12. `sed -n '1,220p' src/bin/blink-rtt.rs`
13. `sed -n '1,260p' src/bin/shared-adc-rtic.rs`
14. `sed -n '1,220p' src/bin/blink-timer-rtfm.rs`
15. `cargo check`
16. `cargo check --message-format short`
17. `sed -n '1,260p' src/bin/pwm-test2.rs`
18. `rg -n "cortex-m-rtic|rtic::|embedded_hal::digital::v2|panic-rtt-target|rtt-target|shared_bus|stm32f1xx_hal::stm32|use stm32f1xx_hal as hal|Timer::tim|adc::Adc::adc1|into_push_pull_output|set_high\(|set_low\(|toggle\(" src/bin`
19. `find .github -maxdepth 3 -type f | sort | xargs -r sed -n '1,220p'`
20. `rg -n "compile_error!|cfg\(feature = \"tim|cargo embed|cargo flash|openocd|thumbv7m-none-eabi|runner =|CARGO_MAKE_CRATE_TARGET_TRIPLE|probe-rs|cargo-make|RTFM|RTIC" -S .`
21. `rustc --version && cargo --version`
22. `rg -n "static mut|G_[A-Z_]+" src/bin`
23. `sed -n '1,260p' src/bin/adc-dma-test2.rs`
24. `sed -n '1,220p' src/bin/blink-timer-irq-safe.rs`
25. `cargo check --bins`
26. `rg -n "Timer::new|CounterMs|CounterHz|counter_hz|delay::Delay|Delay::new|cortex_m::delay::Delay|Adc::new|with_dma|split\(&mut rcc\)|cfgr\(|afio|pac::Peripherals|hprintln!\(" ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src`
27. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/blinky.rs`
28. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/blinky_timer_irq.rs`
29. `sed -n '1,320p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/adc-dma-rx.rs`
30. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/timer-interrupt-rtic.rs`
31. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/adc_temperature.rs`
32. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/delay.rs`
33. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/pwm.rs`
34. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/exti_rtic.rs`
35. `cargo check --bins --message-format short`
36. `sed -n '1,220p' src/bin/i2c-bitbang-test2.rs`
37. `sed -n '1,220p' src/bin/rc522-test1.rs`
38. `sed -n '1,220p' src/bin/uart-test1.rs`
39. `sed -n '1,220p' src/bin/shared-adc-single-chan.rs`
40. `sed -n '1,220p' src/bin/ws2812-test1.rs`
41. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/spi.rs`
42. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/mfrc522.rs`
43. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/serial.rs`
44. `cargo check --bins --features legacy_examples --message-format short`
45. `cargo check --bins --message-format short`

## Progress Notes

- Baseline `cargo check` on Rust 1.92 mostly built, but failed on `pwm-test2` because the binary intentionally requires a PWM remap feature and used a manual `compile_error!`.
- The old `.cargo/config` deprecation warning confirmed that the cargo config file needs to be renamed to `.cargo/config.toml`.
- Current upgrade target is based on:
  - `stm32f1xx-hal` `0.11.0`
  - `cortex-m-rt` `0.7.5`
  - `cortex-m-rtic` `1.1.4`
  - `rtt-target` `0.6.2`
  - `panic-rtt-target` `0.2.0`

## Changes Applied

- Updated `Cargo.toml` to Rust 2024 and added `rust-version = "1.85"`.
- Bumped dependency versions to current releases or current compatible major lines.
- Replaced the git-pinned `shared-bus` dependency with crates.io `0.3.1`.
- Added a synthetic `pwm` feature and made the PWM remap features imply it.
- Gated `pwm-test2` with Cargo `required-features = ["pwm"]` so generic workspace checks no longer fail by design.
- Moved `.cargo/config` to `.cargo/config.toml`.
- Switched the default target runner to `probe-rs run --chip STM32F103C8`.
- Added `rust-toolchain.toml` to pin a stable embedded setup with the target predeclared.
- Updated GitHub Actions to `actions/checkout@v4` and `dtolnay/rust-toolchain@stable`.
- Migrated the default bin set onto `stm32f1xx-hal` `0.11` APIs so `cargo check --bins` succeeds again on stable Rust 1.92.
- Introduced `legacy_examples` and `legacy_rtic` gating for the oldest experiments that still need deeper per-example rewrites.

## Final Status

- `cargo check --bins`: passes
- `cargo check --bins --features legacy_examples`: still fails for old DMA / bitbang examples that need dedicated modernization
- `cargo check --bins --features legacy_rtic`: not expected to pass yet; the old RTIC / RTFM examples still need a full RTIC 1.x rewrite

## Remaining Ideas

- Rewrite `blink-timer-rtfm`, `shared-adc-rtic`, and `rc522-test2` to RTIC 1.x module-style apps.
- Port the old ADC DMA experiments to the current `stm32f1xx-hal` DMA type signatures and timer / RCC API.
- Revisit the bitbang I2C examples with either newer drivers that accept the current bitbang implementation or a locally compatible compatibility layer.

## Legacy RTIC Refresh

### Goal

Refresh the `legacy_rtic` examples:

- `blink-timer-rtfm`
- `shared-adc-rtic`
- `rc522-test2`

### Commands Run

46. `sed -n '1,260p' src/bin/blink-timer-rtfm.rs`
47. `sed -n '1,260p' src/bin/shared-adc-rtic.rs`
48. `sed -n '1,280p' src/bin/rc522-test2.rs`
49. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/timer-interrupt-rtic.rs`
50. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/exti_rtic.rs`
51. `sed -n '1,240p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/mfrc522.rs`
52. `rg -n "remap\(&mut afio\.mapr\)|Spi1Remap|spi\(" ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src/spi.rs`
53. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src/spi.rs`
54. `rg -n "new_cortexm|AdcProxy|acquire_adc" ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/shared-bus-0.3.1/src`
55. `sed -n '30,70p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/serial-interrupt-idle.rs`
56. `rg -n "enum Edge|RisingFalling|Rising|Falling" ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src/gpio.rs`
57. `cargo check --bins --features legacy_rtic --message-format short`
58. `sed -n '1,140p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/mfrc522-0.8.0/src/comm/eh02/spi.rs`
59. `sed -n '1,120p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/mfrc522-0.8.0/src/lib.rs`
60. `nl -ba src/bin/rc522-test2.rs | sed -n '1,180p'`
61. `cargo check --bins --features legacy_rtic --message-format short`

### Notes

- Rewrote all three old `const APP: () = { ... }` RTIC examples into current RTIC 1.x module-style apps.
- Replaced removed `rtic::cyccnt` scheduling in `shared-adc-rtic` with two hardware timer interrupt tasks. This keeps the example concurrent and periodic without adding new monotonic dependencies.
- Replaced old HAL timer and GPIO APIs with `counter_ms`, `clear_interrupt(Event::Update)`, current `split(&mut rcc)`, and current EXTI setup.
- Reworked the MFRC522 example to the current driver type-state API:
  - `mfrc522::comm::eh02::spi::SpiInterface`
  - explicit `Initialized` state
  - current SPI remap flow via `.remap(&mut afio.mapr).spi(...)`

### Outcome

- `cargo check --bins --features legacy_rtic`: passes
- `cargo check --bins`: still passes
- Remaining non-passing legacy area is now limited to `legacy_examples` DMA / bitbang experiments, not the RTIC set

## Ideas

- Prefer a compatible upgrade path that keeps the project useful as a scratchpad of examples instead of forcing a full architectural rewrite.
- Replace deprecated `cargo-flash` references with `probe-rs` tooling where appropriate.
- Consider pinning a current embedded target toolchain via `rust-toolchain.toml` if verification depends on nightly or specific components.

## RTIC Finalization

### Goal

Finish the RTIC migration:

- bump RTIC to the current crate line
- remove the `legacy_rtic` feature gate entirely
- keep the migration log appended in place

### Commands Run

62. `sed -n '1,240p' Cargo.toml`
63. `sed -n '1,260p' worklog.md`
64. `sed -n '1,220p' README.md`
65. `rg -n "rtic::app|cortex_m_rtic|use rtic|legacy_rtic|required-features" src/bin Cargo.toml README.md`
66. `rg -n "legacy_rtic|cortex-m-rtic|RTIC 1\\.x|oldest RTIC|opt-in" .`
67. `cargo check --bins --message-format short`
68. `rg -n "legacy_rtic" Cargo.toml README.md src .github .cargo`

### Notes

- Replaced the old `cortex-m-rtic` dependency line with `rtic = "2.2.0"` and removed the dedicated `legacy_rtic` Cargo feature from the manifest.
- RTIC 2.x requires an explicit backend selection; for `thumbv7m-none-eabi` on STM32F103 the correct choice is `features = ["thumbv7-backend"]`.
- Removed the `required-features = ["legacy_rtic"]` gates from the RTIC binaries so they are part of the normal `cargo check --bins` path.
- Updated the README to reflect that only the DMA / bitbang holdouts remain opt-in legacy examples.
- RTIC 2.x also rejects the older RTIC 1.x-style `#[init] -> (Shared, Local, Monotonics)` signature when no monotonic is declared. The three RTIC bins now return just `(Shared, Local)`.

### Outcome

- `cargo check --bins`: passes with the RTIC examples in the default bin set
- No active `legacy_rtic` references remain outside this historical worklog
