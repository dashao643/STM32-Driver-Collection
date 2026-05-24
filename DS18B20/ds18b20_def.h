#ifndef __DS18B20_DEF_H
#define __DS18B20_DEF_H

// ROM Commands
#define DS18B20_SEARCH_ROM                  0xF0    // 多从机使用
#define DS18B20_READ_ROM                    0x33    // 单从机使用
#define DS18B20_MATCH_ROM                   0x55    // 后跟从机ROM，对应从机响应
#define DS18B20_SKIP_ROM                    0xCC    // 后跟函数为广播发送，单从机可用于读取
#define DS18B20_ALARM_SEARCH                0xEC    // 检测从机报警标志

// Function Commands
#define DS18B20_CONVERT_T                   0x44    // 从机检测温度 
#define DS18B20_WRITE_SCRATCHPAD            0x4E    // 写缓存
#define DS18B20_READ_SCRATCHPAD             0xBE    // 读缓存
#define DS18B20_COPY_SCRATCHPAD             0x48    // 将缓存写入EE
#define DS18B20_RECALL_EE                   0xB8    // 从EE读到缓存
#define DS18B20_READ_POWER_SUPPLY           0xB4    // 读取从机电源模式

#endif
