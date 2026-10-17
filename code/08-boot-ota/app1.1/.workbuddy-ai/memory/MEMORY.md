# 项目长期记忆 — OTA-Smart-Control-Terminal

## 仓库结构
教学项目，`code/` 下 8 章：01-modbus、02-modbus-freertos、03-mqtt-wifi、04-mqtt-4g、
05-mqtt-freertos、06-canopen、07-canopen-freertos、08-boot-ota。
第 8 章含三个工程：`boot/`、`app1.1/`、`app1.2/`。
仓库根有各章文档：Modbus.md / MQTT.md / freeRTOS.md / CANopen.md / OTA.md / README.md。

## 硬件与分区（第 8 章）
- MCU：STM32F103C8T6，Flash 64K（0x08000000–0x0800FFFF），页大小 1K。
- 分区：boot `0x08000000` + 8K；app1 `0x08002000` + 28K；app2 `0x08009000` + 28K（正好用满 64K）。
- OTA 启动标志：`0x0800FFFC`（= app2 末尾 4 字节）。0xFFFFFFFF=正常启动，0xAAAAAAAA=升级。
  注意 OTA.md 里写的是 0x0801FFFC，那是 128K 芯片的地址，**文档有误，以代码 0x0800FFFC 为准**。
- 串口：USART1 115200（printf 重定向，bootloader 横幅也走这里）；USART3 115200 接 ML307R 4G 模组。
- I2C1 = PB6/PB7（`hi2c1`），挂 OLED(0x78)、AHT20、INA226。注意：**三个器件共用一条 I2C 总线**。
- 其它：BEEP=PC13，RELAY=PB1，LED1=PB9，LED2=PB8。

## 工程约定
- HAL 时基：第 5/7/8 章用 **TIM1**（`stm32f1xx_hal_timebase_tim.c`），第 1/6 章用 SysTick。
- OLED 驱动（`oled.c` / `oled.h` / `codetab.h`）在各章间**逐字节复用**，
  第 8 章只是裁掉了 6x8 字体、汉字、BMP、OLED_Test。排 OLED 问题时不要怀疑驱动差异。
- app1.1 镜像 28640 字节 / 分区 28672 字节，**只剩 32 字节余量**，任何新增代码都可能链接溢出。

## 踩坑记录（重要）
- 该工程源文件在磁盘上是 **CRLF + Tab 缩进**（仓库内是 LF）。
  用 Edit 工具改这些文件会整文件重写、每行少一个 Tab，产生数百行噪声 diff。
  → 改这些文件优先用 Python 按字节精确替换，改完检查 `git diff --stat` 是否最小。
- 串口助手 UartAssist 默认 GBK 解码，读 ML307R 的 UTF-8 中文日志会乱码。
  还原：`garbled.encode('gbk').decode('utf-8')`。
