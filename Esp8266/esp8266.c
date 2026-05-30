#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "esp8266.h"
#include "esp8266_app.h"
#include "at24c64_app.h"
#include "at24c64.h"
#include "modbus.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "led.h"

static uint8_t txBuf[ESP8266_TX_MAXLENTH];
static uint8_t rxBuf[ESP8266_RX_MAXLENTH];
static ESP8266_t esp8266 = {0};

/************************ 云端服务器 *************************/
const char SERVER_IP[] = {"\"192.168.31.155\""};
const char SERVER_PORT[] = {"6789"};
/********************** 接收数据固定帧头 **********************/
// 0D   0A  2B 49 50 44 2C  30  2C   33     3A
// 回车 换行 +  I  P  D  ,   id  ,  字节数    :
const uint8_t FRAME_HEAD[] = {0x0D, 0x0A, 0x2B, 0x49, 0x50, 0x44, 0x2C};
#define FRAME_HEAD_LEN  (sizeof(FRAME_HEAD))               // 长度 = 7


static bool transmitReceive(const char *tx, const char *rx, uint16_t timeout)
{
  uint8_t retryCnt = 0;
  
  while(retryCnt++ < ESP8266_RETRY_COUNT){
    ESP8266_AT_Transmit(tx);
    uint8_t state = ESP8266_AT_Receive(rx, timeout);
    if(state == HAL_OK) 
      return true;
    else if(state == HAL_BUSY){
      if(ESP8266_AT_Receive(rx, 1000) == HAL_OK)
        return true;
    }
    else if(state == HAL_ERROR)
      return false;
  }
  return false;
}

static bool AT_STA_Config(void)
{
  // 测试AT
  if(!transmitReceive("AT\r\n", "OK", 50)) return false;

  // if(!transmitReceive("AT+RST\r\n", "OK", 50)) return false;

  // 设置为STA模式
  if(!transmitReceive("AT+CWMODE=1\r\n", "OK", 50)) return false;

  // 开启多连接
  if(!transmitReceive("AT+CIPMUX=1\r\n", NULL, 10)) return false;
  // HAL_Delay(500);
  
  // 查询网络连接状态, 
  if(!transmitReceive("AT+CIPSTATUS\r\n", "OK", 50)) return false;
  // 若为未连接，连接WiFi
  if(!strstr((char*)esp8266.uart.rxBuf, "STATUS:2")) {
    // 从EE读出用户名和密码
    uint8_t ssidByte = 0, pwdByte = 0;
    AT24C64_Read_Page(WIFI_SSID_PAGE, &ssidByte, 1);
    AT24C64_Read_Page(WIFI_PASSWORD_PAGE, &pwdByte, 1);
    if(ssidByte == AT24C64_BLANK_BYTE || pwdByte == AT24C64_BLANK_BYTE){
      printf("WiFi not configured\n");
      return false;
    }
    char ssid[33] = {0};
    char pwd[33] = {0}; 
    AT24C64_App_ReadWiFiSSID(ssid, 33);
    AT24C64_App_ReadWiFiPassword(pwd, 33);
    char connectWiFi[100] = {0};
    sprintf(connectWiFi, "AT+CWJAP=%s,%s\r\n", ssid, pwd);
    if(!transmitReceive(connectWiFi, NULL, 1000)){
      printf("WiFi connected fail\n");
      return false;
    }
  }
  // 查询本机IP地址
  transmitReceive("AT+CIPSTA?\r\n", NULL, 10);

  // 开启本地TCP服务器
  // if(!transmitReceive("AT+CIPSERVER=1,80\r\n", "OK", 10)) return false;
  // HAL_Delay(500);

  // 与云端服务器建立TCP连接
  char tcpConnect[50] = {0};
  sprintf(tcpConnect, "AT+CIPSTART=0,\"TCP\",%s,%s\r\n", SERVER_IP, SERVER_PORT);

  if(!transmitReceive(tcpConnect, "OK", 2000)) return false;

  return true;
}

static bool AT_AP_Config(void)
{
  // 设置为AP模式
  if(!transmitReceive("AT+CWMODE=2\r\n", "OK", 50)) return false;
  // 配置WiFi
  if(!transmitReceive("AT+CWSAP=\"dashao\",\"12345678\",6,4\r\n", "OK", 50)) return false;
  // 设置多连接
  if(!transmitReceive("AT+CIPMUX=1\r\n", "OK", 50)) return false;
  // 开启服务 IP：192.168.4.1 Port:80
  if(!transmitReceive("AT+CIPSERVER=1,80\r\n", "OK", 50)) return false;  
  
  return true;
}

static void frameReply(char id, const char* data)
{
  // char tx_buf[30] = {0};
  // uint8_t len = strlen(data);
  // uint8_t strBack[10] = {0};

  // if(len > 10)
  //   return;

	// sprintf(tx_buf,"AT+CIPSEND=%u,%u\r\n", txID, len);    // 转成字符
  // HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)tx_buf, strlen(tx_buf), ESP8266_UARTX_TIMEOUT);
  // 等待返回 '>' 
  // HAL_Delay(1);
  // HAL_UART_Receive(ESP8266_HANDLE,strBack,sizeof(strBack),20);
  // cleanString(strBack);
  // if(strstr((char*)strBack, "OK") || strstr((char*)strBack, ">")){
    // LED_GREEN_TOGGLE();
    // HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)data, strlen(data), ESP8266_UARTX_TIMEOUT);
  // }
  // LED_RED_TOGGLE();
}

static bool frameHeaderCheck(void)
{
  // 最短帧长
  if(esp8266.uart.rxIdx < ESP8266_RX_MINLENTH){
    return false;
  }
  // 补充结尾符
  esp8266.uart.rxBuf[esp8266.uart.rxIdx] = 0;
  // 校验帧头
  if(memcmp(FRAME_HEAD, esp8266.uart.rxBuf, FRAME_HEAD_LEN) != 0){
    return false;
  }
  return true;
}

static void frameExecute(void)
{
  // 提取客户端id用于回复
  char id = esp8266.uart.rxBuf[7];

  // 找冒号,冒号后是数据
  char *colon = strchr((char*)esp8266.uart.rxBuf, ':');
  if (colon == NULL) {
    frameReply(id, "frame error");
    return;
  };
  char *data = colon + 1;
  // 总长度减去固定帧头长度
  uint16_t size = esp8266.uart.rxIdx - 11;

  if(ESP8266_APP_LocalCmd(data, size)){
    frameReply(id, "ok");
  }
  else{
    frameReply(id, "cmd error");
  }
}

static void frameProcess(void)
{
  if(!frameHeaderCheck()){
    return;
  }
    
  frameExecute();
}

/*-----------------------------------------------------------------*/

/**
 * @brief 发送AT指令，不限长度
 * 
 * @param cmd AT指令
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef ESP8266_AT_Transmit(const char *cmd)
{
  printf("%s\n",cmd);

  uint16_t len = strlen(cmd);

  // 发新指令前重置缓冲区
  UART_Clear_AT(&esp8266.uart);

  return HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)cmd, len, ESP8266_TX_TIMEOUT_MS);
}

/**
 * @brief 在超时时间内接收指定AT回复指令
 * 
 * @param res 指定回复指令，NULL代表数据无限制
 * @param timeout 超时时间（单位ms）
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef ESP8266_AT_Receive(const char *res, uint16_t timeout)
{
  uint16_t cnt = 0;

  while(cnt < timeout){
    if(esp8266.uart.frameEnd){
      esp8266.uart.frameEnd = false;

      // 缓冲区索引处补充\0结束符
      esp8266.uart.rxBuf[esp8266.uart.rxIdx] = 0;

      // printf("cnt=%d\n", cnt);
      Modbus_Transmit(esp8266.uart.rxBuf, esp8266.uart.rxIdx);

      if(!res) return HAL_OK;

      if(strstr((char*)esp8266.uart.rxBuf, "busy")) return HAL_BUSY;

      if(strstr((char*)esp8266.uart.rxBuf, "ERROR")) return HAL_ERROR;

      if(strstr((char*)esp8266.uart.rxBuf, res)) return HAL_OK;
    }
    HAL_Delay(1);
    cnt++;
  }
  return HAL_TIMEOUT;
}

void ESP8266_Init(void)
{
  /******************* UART *******************/
  esp8266.uart.instance = ESP8266_INSTANCE;
  esp8266.uart.handle = ESP8266_HANDLE;
  esp8266.uart.rxBuf = rxBuf;
  esp8266.uart.rxMaxSize = ESP8266_RX_MAXLENTH;
  esp8266.uart.txBuf = txBuf;
  esp8266.uart.txMaxSize = ESP8266_TX_MAXLENTH;
  UART_Clear_AT(&esp8266.uart);
  __HAL_UART_ENABLE_IT(ESP8266_HANDLE, UART_IT_IDLE);

  /******************* ESP8266 *******************/
  esp8266.isConfig = false;

  // 如果STA模式进入失败，开启AP模式用于局域网通信和配网
  if(AT_STA_Config()){
    LED_GREEN_TOGGLE();
    esp8266.isConfig = true;
    return;
  }
  if(AT_AP_Config()){
    LED_BLUE_TOGGLE();
    esp8266.isConfig = true;
    return;
  }
  LED_RED_TOGGLE();
}

void ESP8266_Task(void)
{
  if(!esp8266.isConfig) return;

  if(esp8266.uart.frameEnd){
    frameProcess();
    UART_Clear_AT(&esp8266.uart);
  }
}

My_UART_t* ESP8266_Get_UART(void)
{
  return &esp8266.uart;
}
