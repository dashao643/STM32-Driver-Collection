# STM32 轻量级外设驱动合集(HAL库 + CubeMX)

## 目前已包含：
1. 定时器单按键,矩阵按键(三种模式切换)
2. dht11 ds18b20
3. tb6612 + 定时器电机案例
4. DMA,IDLE中断串口,RS485接收发送(待优化Ring Buffer)
5. Modbus上层协议(支持04,05,10功能码)
6. RTC时钟(F1系列)
7. 数码管驱动
8. ADC采集(无注入组)
9. 软件模拟I2C驱动AT24C64,ssd1306
10. SPI驱动w25q64
11. 串口IAP(无外部FLASH)
12. nrf24l01无线模块(单向遥控)
13. CAN通信
14. DS1302时钟
15. iwdg,wwdg(失败)
16. WiFi(ESP8266 AP+STA模式上云)
17. PID电机控制(待优化)
18. LCD屏幕驱动(8080)
19. 低功耗(简易实现)
20. st7735 TFT彩屏
21. max30102(数据不准)
22. 上位机向flash传输文件

## 未来：
1. 物联网平台MQTT
2. 红外通信,RFID
3. 以太网(W5500)
4. 蓝牙,4G,LoRa,NB,Zigbee
5. Ymodem协议OTA升级
6. SD卡,IC卡
8. DAC播放音频
9. PCF8574 ,MPU6050
10. IIS协议,USB协议
11. 摄像头+openmv
12. FSMC协议
13. RTOS操作系统
14. FatFS文件管理
15. LVGL屏幕库
17. LwIP协议
18. 低功耗(停止/待机模式定时唤醒 + 上报数据)