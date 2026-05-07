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

## Legacy Examples Finalization

### Goal

Finish the last legacy migration pass:

- modernize the old DMA examples onto current `stm32f1xx-hal` APIs
- keep the software I2C examples on their original bitbang setup
- remove the `legacy_examples` Cargo feature if the bitbang path can be made compatible

### Commands Run

69. `cargo check --bins --features legacy_examples --message-format short`
70. `sed -n '1,220p' Cargo.toml`
71. `rg --files src/bin | sort`
72. `rg -n "required-features = \\[\\\"legacy_examples\\\"\\]|legacy_examples|adc-dma|bitbang" Cargo.toml README.md worklog.md src/bin`
73. `sed -n '1,260p' src/bin/adc-dma-test1.rs`
74. `sed -n '1,320p' src/bin/adc-dma-test2.rs`
75. `sed -n '1,340p' src/bin/adc-dma-test3.rs`
76. `sed -n '1,360p' src/bin/adc-dma-test4.rs`
77. `sed -n '1,340p' src/bin/adc-dma-test5.rs`
78. `sed -n '1,340p' src/bin/adc-dma-test6.rs`
79. `sed -n '1,240p' src/bin/i2c-bitbang-test1.rs`
80. `sed -n '1,260p' src/bin/i2c-bitbang-test2.rs`
81. `sed -n '1,320p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/adc-dma-rx.rs`
82. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/adc-dma-circ.rs`
83. `sed -n '1,360p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/bitbang-hal-0.3.3/src/i2c.rs`
84. `sed -n '1,180p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/i2c_scanner.rs`
85. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/lm75-1.0.0/Cargo.toml`
86. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/eeprom24x-0.7.2/Cargo.toml`
87. `sed -n '280,450p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/embedded-hal-1.0.0/src/i2c.rs`
88. `cargo check --bins --message-format short`
89. `cargo check --bins --features legacy_examples --message-format short`
90. `rg -n "legacy_examples|required-features = \\[\\\"legacy_examples\\\"\\]|bitbang_i2c_compat|embedded-hal-1" Cargo.toml README.md src worklog.md`
91. `tail -n 80 worklog.md`
92. `cargo check --bins --message-format short`

### Notes

- Rewrote `adc-dma-test1` through `adc-dma-test6` against the current HAL surface:
  - `RCC.freeze(...)` and `split(&mut rcc)` instead of the removed bus-specific `apb2` / `ahb` fields
  - `adc::Adc::new(...)` instead of the removed `Adc::adc1(...)`
  - current `AdcDma1<..., Continuous|Scan>` aliases instead of the old two-parameter `AdcDma` type alias
- Simplified the DMA examples to current, compilable patterns while keeping the original intent represented:
  - blocking single-channel DMA
  - repeated single-channel DMA
  - circular DMA
  - scan DMA
  - periodic scan DMA
  - alternating scan groups
- Kept the bitbang I2C examples on their original software-I2C pins instead of moving them to hardware I2C.
- The actual incompatibility was version skew in the driver stack:
  - `bitbang-hal` still implements `embedded-hal` 0.2 blocking I2C traits
  - `lm75` `1.0.0` and `eeprom24x` `0.7.2` now require `embedded-hal` 1.0 I2C
- Added a local compatibility shim in `src/bitbang_i2c_compat.rs` that adapts the existing `bitbang-hal::i2c::I2cBB` transport to `embedded-hal` 1.0 for the transaction shapes these drivers use (`read`, `write`, `write_read`).
- After the shim was in place, the bitbang examples built cleanly as normal bins, so the `legacy_examples` feature gate was removed entirely.

### Outcome

- `cargo check --bins`: passes
- `cargo check --bins --features legacy_examples`: passed immediately before the feature was removed
- No active `legacy_examples` references remain outside this historical worklog

## DMA Semantic Repair

### Goal

Correct the semantic regression introduced while modernizing `adc-dma-test3` through `adc-dma-test6`.

- restore IRQ-driven ownership handoff in `adc-dma-test3` and `adc-dma-test4`
- restore RTIC structure in `adc-dma-test5` and `adc-dma-test6`
- keep the current HAL / RTIC stack and append the correction to this worklog

### Commands Run

93. `git diff --staged -- src/bin/adc-dma-test3.rs src/bin/adc-dma-test4.rs src/bin/adc-dma-test5.rs src/bin/adc-dma-test6.rs`
94. `sed -n '1,260p' src/bin/adc-dma-test3.rs`
95. `sed -n '1,320p' src/bin/adc-dma-test4.rs`
96. `sed -n '1,320p' src/bin/adc-dma-test5.rs`
97. `sed -n '1,360p' src/bin/adc-dma-test6.rs`
98. `rg -n "listen\\(|Event::TransferComplete|DMA1_CHANNEL1|Transfer<|RxDma<" ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples`
99. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src/dma.rs`
100. `sed -n '260,520p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src/dma.rs`
101. `sed -n '1,260p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/usb_serial_rtic.rs`
102. `sed -n '1,240p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/timer-interrupt-rtic.rs`
103. `rg -n "SYST.*delay|SysDelay|delay\\(&rcc\\.clocks|Delay::new\\(|delay_ms\\(" ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/src`
104. `sed -n '1,220p' ~/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/stm32f1xx-hal-0.11.0/examples/delay.rs`
105. `cargo check --bins --message-format short`
106. `cargo check --bins --message-format short`
107. `cargo check --bins --message-format short`

### Notes

- The first DMA modernization pass preserved compilation but not the instructional meaning of the examples.
- `adc-dma-test3` and `adc-dma-test4` were restored to the original interrupt-driven pattern:
  - global transfer / dma / buffer state in `Mutex<RefCell<Option<...>>>`
  - `DMA1_CHANNEL1` owns transfer completion
  - the foreground loop only kicks off the next transfer after the IRQ hands resources back
- `adc-dma-test5` was restored as an RTIC example instead of a blocking loop:
  - `TIM2` starts the next DMA transfer
  - `DMA1_CHANNEL1` completes the transfer and rearms the timer
  - the old removed `rtic::cyccnt` monotonic was replaced with a hardware timer because RTIC 2.x no longer supports the original pattern directly
- `adc-dma-test6` was restored as an RTIC state-machine example:
  - `TIM2` starts whichever ADC scan group is active
  - `DMA1_CHANNEL1` waits the transfer, splits the DMA payload, swaps pin groups, and flips the state between group one and group two
- The resulting code is not byte-for-byte equivalent to the original pre-upgrade examples, but it does preserve the original concurrency model and demonstration intent while using current HAL / RTIC APIs.

### Outcome

- `cargo check --bins`: passes after restoring the IRQ / RTIC semantics

### Follow-up Note

- The `TIM2` dependency added in `adc-dma-test5` and `adc-dma-test6` is not a DMA requirement.
- The original versions of those examples already had paced retriggering via RTIC scheduling:
  - `init` scheduled the first `start_adc_dma(...)` at `Instant::now() + PERIOD.cycles()`
  - `DMA1_CHANNEL1` scheduled the next run the same way
- In other words, the old examples were already time-driven, but the timing source was RTIC's old `cyccnt` monotonic rather than a hardware timer peripheral.
- During the RTIC 2 migration, `TIM2` was used as the replacement pacing mechanism because the old `rtic::cyccnt` pattern is no longer available in the same form.
- This preserved the original "start the next DMA transfer later" behavior without introducing an additional monotonic crate.
- If a closer match to the old design is preferred later, the better replacement is a proper RTIC 2 monotonic rather than a general-purpose timer.

### Dispatcher Note

- The old RTIC / RTFM versions used `extern "C" { fn EXTI2(); }` because dispatcher interrupts were declared that way in the pre-2.x model.
- RTIC 2 changed that model: the changelog explicitly notes that dispatchers moved from the extern block to the `#[rtic::app(..., dispatchers = [...])]` argument.
- RTIC 2's macro code also checks dispatcher counts only against software-task priorities:
  - it computes `need` from `app.software_tasks`
  - and errors with `not enough interrupts to dispatch all software tasks`
- The current `adc-dma-test5` and `adc-dma-test6` do not define any software tasks at all.
- They use only hardware-bound tasks:
  - `#[task(binds = TIM2, ...)]`
  - `#[task(binds = DMA1_CHANNEL1, ...)]`
- They also do not use `spawn`, `schedule`, or an RTIC monotonic queue in their current form.
- Therefore no dispatcher interrupt is required, and dropping `EXTI2` is correct for the current RTIC 2 implementations.
- If these examples are later rewritten to use RTIC software tasks or a dispatcher-backed scheduling path again, the correct modern form would be `dispatchers = [EXTI2]` in the `#[rtic::app(...)]` attribute, not the old extern block.

### Embedded-HAL Split Note

- The project currently contains both `embedded-hal` 0.2 and `embedded-hal` 1.0 on purpose.
- `embedded-hal` 0.2 is still needed by:
  - `bitbang-hal`
  - `shared-bus`
  - the `mfrc522` `eh02` transport path used by the RC522 examples
  - local examples that still import 0.2 traits directly, such as `shared-adc-single-chan`
- `embedded-hal` 1.0 is needed by:
  - `lm75`
  - `eeprom24x`
  - `ws2812-spi`
  - parts of the current `stm32f1xx-hal` stack
- The main incompatibility that forced both versions to coexist during this refresh is the bitbang I2C path:
  - `bitbang-hal` exposes 0.2 blocking I2C traits
  - `lm75` and `eeprom24x` now require 1.0 I2C traits
- The local `src/bitbang_i2c_compat.rs` shim exists specifically to bridge that gap while keeping the original software-I2C examples intact.
- This means the project cannot simply "switch everything to embedded-hal 1.0" without additional migration work:
  - replacing or wrapping `bitbang-hal`
  - moving RC522 examples off the `eh02` path
  - updating remaining local examples that directly use 0.2 traits
- Even removing the direct `embedded-hal = "0.2.7"` dependency from the manifest would not necessarily remove 0.2 from the build graph as long as those crates remain.
