#ifndef __ESP8266_APP_H__
#define __ESP8266_APP_H__

#include <stdint.h>
#include <stdbool.h>

// 命令指令，字符串对应功能函数
typedef struct {
  const char *cmdStr;
  void (*pFunc)(void);
} CmdTable_t;

// 数据指令
typedef struct {
	const char* dataStr;
	bool (*pFunc)(const char*);
} DataTable_t;

typedef enum {
  CMD_OK = 0,
  CMD_ERROR,
  WiFi_CONFIG_OK,
  DATA_ERROR
} ESP8266_APP_RES_e;

ESP8266_APP_RES_e ESP8266_APP_LocalCmd(const char *cmdStr, uint16_t size);

#endif
