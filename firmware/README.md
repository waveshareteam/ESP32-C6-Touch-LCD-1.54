# ESP32-C6-Touch-LCD-1.54 Factory Firmware

[简体中文](README_ZH.md)

| File | Purpose |
| --- | --- |
| [`ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin`](ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin) | Restores the factory demo on the touch model. |

Use esptool 5 or later. Open a terminal in this directory and replace `PORT` with the board's serial port.

Erasing the entire flash clears existing firmware, settings, and user data. Back up anything you need before erasing.

```sh
python -m esptool --chip esp32c6 --port PORT erase-flash
python -m esptool --chip esp32c6 --port PORT write-flash 0x0 ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin
```

Reset the board and check that the factory demo starts. See the [prebuilt examples](ESP32-C6-Touch-LCD-1.54/README.md) for the touch model's demos.
