# ESP32-C6 1.54-inch LCD 出厂固件

[English](README.md)

| 文件 | 用途 |
| --- | --- |
| [`ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin`](ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin) | 有触摸版出厂固件。 |
| [`ESP32-C6-LCD-1.54_FactoryOnly_20260529.bin`](ESP32-C6-LCD-1.54_FactoryOnly_20260529.bin) | 无触摸版出厂固件。 |

使用 esptool 5 或更新版本。在本目录打开终端，将 `PORT` 替换为开发板的串口。下面以有触摸版为例。无触摸版请将写入命令中的文件名替换为表中对应的 BIN。

整片擦除 Flash 会清除原有固件、设置和用户数据，请提前备份。

```sh
python -m esptool --chip esp32c6 --port PORT erase-flash
python -m esptool --chip esp32c6 --port PORT write-flash 0x0 ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin
```

烧录完成后复位开发板，确认出厂演示正常启动。另见[有触摸版示例](ESP32-C6-Touch-LCD-1.54/README_ZH.md)和[无触摸版示例](ESP32-C6-LCD-1.54/README_ZH.md)。
