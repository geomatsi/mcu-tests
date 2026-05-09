## Run RISCV examples using Spike

### Build and run RV32 binaries

```bash
$ make CROSS_COMPILE=/path/to/riscv32-unknown-elf- RISCV_XLEN=32 test1
$ spike --real-time-clint --isa=rv32imac_zicsr_zifencei build/test1.elf
```

### Build and run RV64 binaries

```bash
$ make CROSS_COMPILE=/path/to/riscv64-unknown-elf- RISCV_XLEN=64 test1
$ spike --real-time-clint --isa=rv64imac_zicsr_zifencei build/test1.elf
```

## Links

- Spike internals
  -- https://github.com/poweihuang17/Documentation_Spike
- Spike HTIF Q/A
  -- https://github.com/riscv-software-src/riscv-isa-sim/issues/364
