# ESP32-C6-Touch-LCD-1.54 出厂固件

[English](README.md)

| 文件 | 用途 |
| --- | --- |
| [`ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin`](ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin) | 恢复触摸版的出厂演示。 |

使用 esptool 5 或更新版本。在本目录打开终端，将 `PORT` 替换为开发板的串口。

整片擦除 Flash 会清除原有固件、设置和用户数据，请提前备份。

```sh
python -m esptool --chip esp32c6 --port PORT erase-flash
python -m esptool --chip esp32c6 --port PORT write-flash 0x0 ESP32-C6-Touch-LCD-1.54_FactoryOnly_20260529.bin
```

烧录完成后复位开发板，确认出厂演示正常启动。触摸版其他演示请见[预编译示例](ESP32-C6-Touch-LCD-1.54/README_ZH.md)。
