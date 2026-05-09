# Camera example

## Board

Board: esp32cam module

![alt text](../pics/esp32cam.jpg)

## Environment

Enable ESP-IDF paths:

```bash
$ . /path/to/esp/esp-idf/export.sh
```

Select target:

```bash
$ idf.py set-target esp32
```

If needed, tune ESP-IDF build configuration using menuconfig:

```bash
$ idf.py menuconfig
```

Build sample project:

```bash
$ idf.py build
```

Flash images to the board:

```bash
$ idf.py -p /dev/ttyUSB0 flash
```

Restart and monitor serial logs:

```bash
$ idf.py monitor
```
