# STM32 轻量级外设驱动合集(HAL库)

## 已包含：
1. 定时器单按键,矩阵按键(三种模式切换)
2. dht11 ds18b20
3. tb6612 + 定时器电机案例
4. DMA,IDLE中断串口,RS485接收发送
5. Modbus上层协议(支持04,05,10和自定义功能码)
6. RTC时钟(F1系列)
7. 数码管驱动
8. ADC采集(无注入组)
9. 软件模拟I2C驱动AT24C64
10. ssd1306, 带gb2312中文字库显示
11. SPI驱动w25q64
12. IAP(串口直传/读取外部FLASH)
13. nrf24l01无线模块(单向遥控)
14. CAN通信
15. DS1302时钟
16. iwdg,wwdg(失败)
17. WiFi(ESP8266 AP+STA模式上云)
18. PID电机控制(待优化)
19. LCD屏幕驱动(8080)
20. 低功耗(简易实现)
21. st7735 TFT彩屏
22. max30102(数据不准)
23. 上位机向flash传输文件
24. ov7670摄像头(花屏)
25. SD卡  + FatFS文件管理
26. USB Host + FatFS
27. 红外(), 遥感按键()

## 未来：
1. 以太网外设 + lwip库实现http, mqtt访问
2. 触摸屏 + LVGL库
3. Ymodem协议OTA升级
4. IC卡, RFID
5. DAC, IIS播放音频
6. FSMC协议
7. 低功耗(停止/待机模式定时唤醒 + 上报数据)

------

###### Bsp: 不使用cubeMX, 包含hal库初始化外设函数

###### Bsp_Cube: 使用cubeMX, 不包含cubuMX生成部分

###### App: 应用层函数

###### Middleware: 中间层函数