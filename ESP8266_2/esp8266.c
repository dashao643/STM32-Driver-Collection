#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "at24c64_app.h"
#include "stm32f1xx_hal_def.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

const char SERVER_IP[] = {"\"192.168.31.155\""};
const char SERVER_PORT[] = {"6789"};

static uint8_t txBuf[ESP8266_TX_MAXLENTH];
static uint8_t rxBuf[ESP8266_RX_MAXLENTH];
// static uint8_t txBuf[ESP8266_TX_BUFF_MAXLENTH] = {0};
static ESP8266_t esp8266 = {0};

static bool transmitReceive(const char *tx, const char *rx, uint16_t timeout)
{
  uint8_t retryCnt = 0;
  
  while(retryCnt++ < ESP8266_RETRY_COUNT){
    ESP8266_AT_Transmit(tx);
    uint8_t state = ESP8266_AT_Receive(rx, timeout);
    if(state == HAL_OK) 
      return true;
    else if(state == HAL_BUSY){
      ESP8266_AT_Receive(rx, 1000);
    }
    else if(state == HAL_ERROR)
      return false;
  }
  return false;
}

bool AT_STA_Config(void)
{
  // 测试AT
  if(!transmitReceive("AT+RST\r\n", "OK", 10)) return false;

  // 设置为STA模式
  if(!transmitReceive("AT+CWMODE=1\r\n", "OK", 10)) return false;

  // 查询网络连接状态, 
  if(!transmitReceive("AT+CIPSTATUS\r\n", "OK", 10)) return false;
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
    if(!transmitReceive(connectWiFi, NULL, 50)){
      printf("WiFi connected fail\n");
      return false;
    }
  }
  // 查询本机IP地址
  transmitReceive("AT+CIPSTA?\r\n", NULL, 10);
  printf("%s\n",esp8266.uart.rxBuf);

  // 开启本机服务器多连接
  transmitReceive("AT+CIPMUX=1\r\n", "OK", 10);

  // 开启本地TCP服务器
  transmitReceive("AT+CIPSERVER=1,80\r\n", "OK", 10);

  // 与云端服务器建立TCP连接
  char tcpConnect[50] = {0};
  sprintf(tcpConnect, "AT+CIPSTART=0,\"TCP\",%s,%s\r\n", SERVER_IP, SERVER_PORT);
  transmitReceive(tcpConnect, "OK", 1000);

  return true;
}

void AT_AP_Config()
{

}

/**
 * @brief 发送AT指令，不限长度
 * 
 * @param cmd AT指令
 */
void ESP8266_AT_Transmit(const char *cmd)
{
  uint16_t len = strlen(cmd);

  // 发新指令前清空接收缓冲区，避免旧数据干扰
  UART_Clear(&esp8266.uart);

  HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)cmd, len, ESP8266_TX_TIMEOUT_MS);
}

// /**
//  * @brief 在超时时间内接收指定AT回复指令
//  * 
//  * @param res 指定回复指令，NULL代表数据无限制
//  * @param timeout 超时时间（单位ms）
//  * @return true 成功接收
//  * @return false 超时或错误
//  */
HAL_StatusTypeDef ESP8266_AT_Receive(const char *res, uint16_t timeout)
{
  uint16_t cnt = 0;
  while(cnt < timeout){
    if(esp8266.uart.frameEnd){
      esp8266.uart.frameEnd = false;

      // 给当前累计数据补0结尾
      uint16_t idx = esp8266.uart.rxWrIdx;
      if(idx >= esp8266.uart.rxMaxSize)
        idx = esp8266.uart.rxMaxSize - 1;
      esp8266.uart.rxBuf[idx] = 0;

      printf("cnt=%d\n", cnt);
      printf("rx=%s\n", esp8266.uart.rxBuf);

      // res为NULL表示不限制内容，收到即返回
      if(!res)
        return HAL_OK;

      if(strstr((char*)esp8266.uart.rxBuf, "busy") ||
         strstr((char*)esp8266.uart.rxBuf, "BUSY"))
        return HAL_BUSY;

      if(strstr((char*)esp8266.uart.rxBuf, "ERROR"))
        return HAL_ERROR;

      if(strstr((char*)esp8266.uart.rxBuf, res))
        return HAL_OK;

      // 还没匹配到目标，继续等下一帧数据
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
  UART_Clear(&esp8266.uart);
  __HAL_UART_ENABLE_IT(ESP8266_HANDLE, UART_IT_IDLE);

  // 如果STA模式进入失败，开启AP模式用于局域网通信和配网
  if(!AT_STA_Config()){
    AT_AP_Config();
  }
}

void ESP8266_Task(void)
{

}

My_UART_t* ESP8266_Get_UART(void)
{
  return &esp8266.uart;
}


