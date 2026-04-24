#include "esp8266.h"
#include "my_uart.h"
#include "esp8266_app.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gpio.h"

/************************** 静态变量 *************************/
static uint8_t rxBuf[ESP8266_RX_BUFF_MAXLENTH];
static uint8_t txBuf[ESP8266_TX_BUFF_MAXLENTH];
static ESP8266_t esp8266 = {0};

/*************************** AT指令表 ***************************/
const char AT_CmdList[][50] = {
  {"AT\r\n"},                                             // 测试AT
  {"AT+CWMODE=2\r\n"},                                    // 设置为AP模式
  {"AT+CWSAP=\"dashao\",\"12345678\",6,4\r\n"},           // 配置WiFi
  {"AT+CIPMUX=1\r\n"},                                    // 设置多连接
  {"AT+CIPSERVER=1,80\r\n"}                               // 开启服务
};
#define CMD_TOTAL_COUNT  (sizeof(AT_CmdList)/sizeof(AT_CmdList[0]))  // AT指令总数

/********************** 接收数据固定帧头 **********************/
const uint8_t frameHead[] = {0x0D, 0x0A, 0x2B, 0x49, 0x50, 0x44, 0x2C};
#define FRAME_HEAD_LEN  (sizeof(frameHead))               // 长度 = 7

/************************** 操作指令表 *************************/
const char Data_CmdList[][20] = {
  {"LED_RED_ON"},
  {"LED_RED_OFF"},
  {"LED_RED_TOGGLE"},
  {"LED_GREEN_TOGGLE"},
  {"LED_BLUE_TOGGLE"},
  {"LED_ALL_ON"},
  {"LED_ALL_OFF"},
  {"LED_ALL_TOGGLE"},
};

/************************** 指令函数 *************************/
void Cmd_LED_RED_ON(void)  { LED_RED_ON(); }
void Cmd_LED_RED_OFF(void) { LED_RED_OFF(); }
void Cmd_LED_RED_TOGGLE(void) { LED_RED_TOGGLE(); }
void Cmd_LED_GREEN_TOGGLE(void) { LED_GREEN_TOGGLE(); }
void Cmd_LED_BLUE_TOGGLE(void) { LED_BLUE_TOGGLE(); }
void Cmd_LED_ALL_ON(void) { LED_RED_ON();LED_GREEN_ON();LED_BLUE_ON(); }
void Cmd_LED_ALL_OFF(void) { LED_RED_OFF();LED_GREEN_OFF();LED_BLUE_OFF(); }
void Cmd_LED_ALL_TOGGLE(void) { LED_RED_TOGGLE();LED_GREEN_TOGGLE();LED_BLUE_TOGGLE(); }

/********************* 函数表，与指令表对应 *******************/
const CmdFunc_t Data_FuncList[] = {
  Cmd_LED_RED_ON,
  Cmd_LED_RED_OFF,
  Cmd_LED_RED_TOGGLE,
  Cmd_LED_GREEN_TOGGLE,
  Cmd_LED_BLUE_TOGGLE,
  Cmd_LED_ALL_ON,
  Cmd_LED_ALL_OFF,
  Cmd_LED_ALL_TOGGLE,
};
#define DATA_TOTAL_COUNT  (sizeof(Data_CmdList)/sizeof(Data_CmdList[0]))  // 数据指令总数

static void stateClear(void);
static void AT_SetReplyFlag(void);
static void AT_Config(void);
static void cleanString(char *str);
static void frameReply(const uint8_t txID, const char* data);
static bool frameHeaderCheck(void);
static void frameExecute(void);
static void frameProcess(void);

static void stateClear(void)
{
  esp8266.curRetryCnt = 0;
  esp8266.replyFlag = NONE;
  esp8266.timeTick = HAL_GetTick();
}

static void AT_SetReplyFlag(void)
{
  if(strstr((char*)esp8266.uart.rxBuf, "OK")){
    esp8266.replyFlag = AT_OK;
    return;
  }

  if(strstr((char*)esp8266.uart.rxBuf, "ERROR")){
    esp8266.replyFlag = AT_ERROR;
    return;
  }
}

static void AT_Config(void)
{
  switch (esp8266.state){
  // 起始定时器延时
  case AT_INITIAL:{
    if((HAL_GetTick() - esp8266.timeTick) > INITIAL_DELAY_MS){
      esp8266.timeTick = HAL_GetTick();
      esp8266.state = AT_SEND;
    }
    break;
  }
  // 发送、接收状态循环
  case AT_SEND:{
    if(esp8266.curRetryCnt >= RETRY_COUNT){
      esp8266.state = AT_FAIL;
      return;
    }
    esp8266.curRetryCnt++;
    ESP8266_AT_Transmit(AT_CmdList[esp8266.cmdIndex]);
    esp8266.state = AT_WAIT_OK;
    esp8266.timeTick = HAL_GetTick();
    break;
  }
  case AT_WAIT_OK:{
    if((esp8266.replyFlag == AT_OK)) {
      esp8266.cmdIndex++;
      if(esp8266.cmdIndex == CMD_TOTAL_COUNT){
        esp8266.state = AT_SUCCESS;
      }
      else{
        esp8266.state = AT_SEND;
      }
      stateClear();
    }
    if((HAL_GetTick() - esp8266.timeTick) > WAIT_AT_REPLY_MS){
      esp8266.state = AT_SEND;
      esp8266.timeTick = HAL_GetTick();
    }
    break;
  }
  case AT_SUCCESS:{
    printf("config success\n");
    LED_GREEN_ON();
    esp8266.setConfigFlag = true;
    break;
  }
  case AT_FAIL:{
    LED_RED_ON();
    printf("config fail at %d cmd\n",esp8266.cmdIndex + 1);
    esp8266.setConfigFlag = true;
    break;
  }
  default:
    break;
  }
}

// 清除字符串里的不可见字符
static void cleanString(char *str)
{
  if(str == NULL) return;

  int i = 0, j = 0;
  while(str[i] != '\0'){
    // 只保留可见字符（ASCII 32 ~ 126）
    if(str[i] >= 32 && str[i] <= 126){
      str[j++] = str[i];
    }
    i++;
  }
  // 结尾补结束符
  str[j] = '\0'; 
}
// static void frameReply(const uint8_t txID, const char* data)
// {
//   uint8_t headLen = 0;
//   uint8_t dataLen = strlen(data);
//   uint8_t strBack[5] = {0};

// 	sprintf((char*)esp8266.uart.txBuf, "AT+CIPSEND=%u,%u\r\n", txID, dataLen);    // 转成字符
//   headLen = strlen((char*)esp8266.uart.txBuf);

//   if(headLen > esp8266.uart.txMaxSize)
//     return;

//   UART_Transmit(&esp8266.uart, headLen, BLOCK, 100);

//   // 阻塞接收
//   // HAL_UART_Receive(ESP8266_HANDLE, strBack, sizeof(strBack), 20);

//   // if(strstr((char*)strBack, "OK") || strstr((char*)strBack, ">")){
//   //   if(dataLen > esp8266.uart.txMaxSize)
//   //     dataLen = esp8266.uart.txMaxSize;

//   //   memcpy(esp8266.uart.txBuf, (const uint8_t*)data, dataLen);

//     HAL_Delay(1);
//     UART_Transmit(&esp8266.uart, dataLen, BLOCK, 100);
//   // }
//   // else{
//     LED_RED_TOGGLE();
//   // }
  
// }
static void frameReply(const uint8_t txID, const char* data)
{
  char tx_buf[30] = {0};
  uint8_t len = strlen(data);
  // uint8_t strBack[10] = {0};

  if(len > 10)
    return;

	sprintf(tx_buf,"AT+CIPSEND=%u,%u\r\n", txID, len);    // 转成字符
  HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)tx_buf, strlen(tx_buf), ESP8266_UARTX_TIMEOUT);
  // 等待返回 '>' 
  HAL_Delay(1);
  // HAL_UART_Receive(ESP8266_HANDLE,strBack,sizeof(strBack),20);
  // cleanString(strBack);
  // if(strstr((char*)strBack, "OK") || strstr((char*)strBack, ">")){
    // LED_GREEN_TOGGLE();
    HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)data, strlen(data), ESP8266_UARTX_TIMEOUT);
  // }
  // LED_RED_TOGGLE();
}

static bool frameHeaderCheck(void)
{
  // 最短帧长
  if(esp8266.uart.rxSize < ESP8266_RX_BUFF_MINLENTH){
    return false;
  }
  // 校验帧头
  if(memcmp(frameHead, esp8266.uart.rxBuf, FRAME_HEAD_LEN) != 0){
    return false;
  }
  return true;
}

static void frameExecute(void)
{
  // 提取客户端ID用于回复
  uint8_t txID = esp8266.uart.rxBuf[7] - '0';

  char *colon = strchr((char*)esp8266.uart.rxBuf, ':');  // 找冒号
  if (colon == NULL)
    return;

  char *data = colon + 1;

  cleanString(data);

  // 字符串匹配
  for(uint8_t i = 0; i < DATA_TOTAL_COUNT; i++){
      if(strcmp(Data_CmdList[i],data) == 0){
      Data_FuncList[i](); // 直接执行对应函数
      frameReply(txID,"ok");
      return;
    }
  }
  frameReply(txID,"no cmd");
}

static void frameProcess(void)
{
  printf("%s\n",esp8266.uart.rxBuf);
  // AT指令已配置，校验数据帧头 + 执行指令
  if(esp8266.setConfigFlag) {
    if(!frameHeaderCheck())
      return;
    frameExecute();
  }
  // 处理AT指令帧
  else{
    AT_SetReplyFlag();
  }
}

void ESP8266_Init(void)
{
  /******************* UART *******************/
  esp8266.uart.instance = ESP8266_INSTANCE;
  esp8266.uart.handle = ESP8266_HANDLE;
  esp8266.uart.rxBuf = rxBuf;
  esp8266.uart.rxMaxSize = ESP8266_RX_BUFF_MAXLENTH;
  esp8266.uart.txBuf = txBuf;
  esp8266.uart.txMaxSize = ESP8266_TX_BUFF_MAXLENTH;

  UART_Clear(&esp8266.uart);

  HAL_UART_Receive_DMA(ESP8266_HANDLE, esp8266.uart.rxBuf, esp8266.uart.rxMaxSize);
  __HAL_UART_ENABLE_IT(ESP8266_HANDLE, UART_IT_IDLE);

  /******************* esp8266 *******************/
  esp8266.state = AT_INITIAL;
  esp8266.cmdIndex = 0;
  esp8266.setConfigFlag = false;
}

void ESP8266_Task(void)
{
  if(!esp8266.setConfigFlag){
    AT_Config();
  }
  if(esp8266.uart.frameEnd){
    frameProcess();
    // HAL_UART_AbortReceive_IT(ESP8266_UARTX);
    UART_Clear(&esp8266.uart);
    // HAL_UART_Receive_IT(ESP8266_UARTX, esp8266_uart.buffer, 1);
  }
}

My_UART_t* ESP8266_Get_UART(void)
{
  return &esp8266.uart;
}

/**
 * @brief 阻塞式发送，不显大小
 * 
 * @param cmd 发送的字符串，需带\0
 */
void ESP8266_AT_Transmit(const char cmd[])
{
  uint16_t len = strlen(cmd);

  HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)cmd, len, ESP8266_UARTX_TIMEOUT);
}