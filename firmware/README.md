# ESP32-C6 1.54-inch LCD Factory Firmware

[简体中文](README_ZH.md)

| File | Purpose |
| --- | --- |
| [`ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin`](ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin) | Factory firmware for the touch model. |
| [`ESP32-C6-LCD-1.54_FactoryOnly_20260529.bin`](ESP32-C6-LCD-1.54_FactoryOnly_20260529.bin) | Factory firmware for the non-touch model. |

Use esptool 5 or later. Open a terminal in this directory and replace `PORT` with the board's serial port. The command below uses the touch image. For the non-touch board, replace the filename with its BIN from the table.

Erasing the entire flash clears existing firmware, settings, and user data. Back up anything you need before erasing.

```sh
python -m esptool --chip esp32c6 --port PORT erase-flash
python -m esptool --chip esp32c6 --port PORT write-flash 0x0 ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin
```

Reset the board and check that the factory demo starts. See the prebuilt examples for the [touch model](ESP32-C6-Touch-LCD-1.54/README.md) and [non-touch model](ESP32-C6-LCD-1.54/README.md).
