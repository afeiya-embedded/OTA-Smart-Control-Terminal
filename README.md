# OTA Smart Control Terminal

基于 STM32F103 系列的嵌入式通信与远程升级示例工程。项目以智能控制终端为场景，逐步实现 Modbus、MQTT、CANopen、FreeRTOS，以及基于 BootLoader 的双分区 OTA 升级。

## 功能概览

- 采集温湿度、电压电流等外设数据，并在 OLED 上显示。
- 通过 Modbus RTU 实现工业现场的请求/应答通信。
- 通过 ESP8266 Wi-Fi 或 4G 模块连接 MQTT Broker，使用 JSON 负载交换数据。
- 基于 CAN 总线实现 CANopen 节点通信。
- 在部分示例中使用 FreeRTOS 管理采集、通信与显示等并发任务。
- 使用 BootLoader、App1、App2 三个镜像完成本地双分区固件更新流程。

## 工程结构

| 目录 | 内容 | Keil 工程 |
| --- | --- | --- |
| `code/01-modbus` | 基础 Modbus RTU 通信示例 | `MDK-ARM/01-modbus.uvprojx` |
| `code/02-modbus-freertos` | FreeRTOS 下的 Modbus 示例 | `MDK-ARM/02-modbus-freertos.uvprojx` |
| `code/03-mqtt-wifi` | ESP8266 Wi-Fi MQTT 示例 | `MDK-ARM/03-mqtt-wifi.uvprojx` |
| `code/04-mqtt-4g` | 4G 网络 MQTT 示例 | `MDK-ARM/04-mqtt-4g.uvprojx` |
| `code/05-mqtt-freertos` | FreeRTOS 下的 MQTT 示例 | `MDK-ARM/05-mqtt-freertos.uvprojx` |
| `code/06-canopen` | 基础 CANopen 节点示例 | `MDK-ARM/06-canopen.uvprojx` |
| `code/07-canopen-freertos` | FreeRTOS 下的 CANopen 示例 | `MDK-ARM/07-canopen-freertos.uvprojx` |
| `code/08-boot-ota/boot` | BootLoader：更新检查、镜像搬运和 App 跳转 | `MDK-ARM/boot.uvprojx` |
| `code/08-boot-ota/app1.1` | 初始应用镜像 | `MDK-ARM/app1.1.uvprojx` |
| `code/08-boot-ota/app1.2` | 升级应用镜像 | `MDK-ARM/app1.2.uvprojx` |

## 文档索引

- [Modbus](Modbus.md)：Modbus RTU、FreeModbus 与自定义通信协议。
- [MQTT](MQTT.md)：发布/订阅模型、QoS、ESP8266 MQTT AT 指令与 JSON 负载。
- [FreeRTOS](freeRTOS.md)：任务、队列、信号量、互斥量、软件定时器和内存管理。
- [CANopen](CANopen.md)：CAN 总线、对象字典、PDO/SDO 与节点通信。
- [OTA](OTA.md)：BootLoader 工作方式、双分区升级流程及 Flash 分区示例。

## OTA 升级流程

OTA 工程采用 BootLoader + App1 + App2 的双应用分区设计：

1. BootLoader 启动后检查 App2 备份区是否存在待更新镜像。
2. 若存在，BootLoader 将 App2 镜像复制至 App1 运行区，并清除 App2 标记。
3. BootLoader 跳转至 App1；应用需要在启动时设置正确的中断向量表。
4. App1 获取升级镜像后写入 App2，并在下次重启时由 BootLoader 完成替换。

默认 Flash 分区定义见 `code/08-boot-ota/boot/Core/Inc/boot.h`：BootLoader 为 8 KB，App1 与 App2 各为 28 KB，应用起始地址分别为 `0x08002000` 与 `0x08009000`。修改芯片型号、链接地址或应用大小时，必须同步修改三个工程的链接配置、向量表偏移和分区宏；否则可能导致应用无法启动或覆盖其他镜像。

> OTA 示例展示的是固件分区和镜像切换机制。用于实际产品前，还应补充镜像长度与 CRC/签名校验、断电保护、版本管理、回滚策略和安全传输。

## 使用说明

- `freemodbus-v1.6` 为随仓库提供的 FreeModbus 协议栈源码与示例，`freemodbus-v1.6.zip` 是对应源码压缩包。
- 各工程均包含独立的 `Drivers`、`Core` 与 Keil 配置，避免跨工程混用生成文件。
- 仓库中的图片由配套协议文档引用，阅读文档时请保持其相对路径不变。

## 免责声明

本项目用于嵌入式通信与 OTA 技术学习。接入真实设备、生产网络或执行在线升级前，请根据实际硬件、Flash 容量、网络安全和失效保护要求完成验证。
