# STM8 examples

## Meson build
Example workflow:

```bash
# configure meson build
$ mkdir build
$ meson build

# list ninja targets
$ ninja -C build -t targets
stm8flash_blink.ihx: phony
openocd_blink.ihx: phony
debug_blink.ihx: phony
...

# flash stm8 chip using OpenOCD and then attach to debug
$ ninja -C build openocd_blink.ihx
$ ninja -C build debug_blink.ihx
```
