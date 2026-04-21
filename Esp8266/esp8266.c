#include "esp8266.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gpio.h"

// 指令表字符串数组
const char AT_CmdList[][50] = {
  {"AT\r\n"},                                             // 测试AT
  {"AT+CWMODE=2\r\n"},                                    // 设置为AP模式
  {"AT+CWSAP=\"dashao\",\"12345678\",6,4\r\n"},           // 配置WiFi
  {"AT+CIPMUX=1\r\n"},                                    // 设置多连接
  {"AT+CIPSERVER=1,80\r\n"}                               // 开启服务
};

#define CMD_TOTAL_COUNT  (sizeof(AT_CmdList)/sizeof(AT_CmdList[0]))  // AT指令总数

// 接收数据固定帧头
const uint8_t frameHead[] = {0x0D, 0x0A, 0x2B, 0x49, 0x50, 0x44, 0x2C};
#define FRAME_HEAD_LEN  (sizeof(frameHead))               // 长度 = 7

// 发送数据指令表
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

// 指令函数
void Cmd_LED_RED_ON(void)  { LED_RED_ON(); }
void Cmd_LED_RED_OFF(void) { LED_RED_OFF(); }
void Cmd_LED_RED_TOGGLE(void) { LED_RED_TOGGLE(); }
void Cmd_LED_GREEN_TOGGLE(void) { LED_GREEN_TOGGLE(); }
void Cmd_LED_BLUE_TOGGLE(void) { LED_BLUE_TOGGLE(); }
void Cmd_LED_ALL_ON(void) { LED_RED_ON();LED_GREEN_ON();LED_BLUE_ON(); }
void Cmd_LED_ALL_OFF(void) { LED_RED_OFF();LED_GREEN_OFF();LED_BLUE_OFF(); }
void Cmd_LED_ALL_TOGGLE(void) { LED_RED_TOGGLE();LED_GREEN_TOGGLE();LED_BLUE_TOGGLE(); }

// 函数表，与指令表一一对应
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

static uint8_t rx_buf[ESP8266_RX_BUFF_MAXLENTH];
static UART_ChannelTypeDef esp8266_uart;
static ESP8266_HandleTypeDef esp8266_handle;

// static uint8_t esp8266_tx_buff[ESP8266_TX_BUFF_MAXLENTH];
// static uint8_t esp8266_tx_index = 0;

static void stateClear(void)
{
  esp8266_handle.curRetryCnt = 0;
  esp8266_handle.replyFlag = NONE;
  esp8266_handle.timeTick = HAL_GetTick();
}

static void AT_Transmit(const char cmd[])
{
  uint16_t len = strlen(cmd);
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)cmd, len, ESP8266_UARTX_TIMEOUT);
}

static void AT_SetReplyFlag(void)
{
  if(strstr((char*)esp8266_uart.buffer, "OK")){
    esp8266_handle.replyFlag = AT_OK;
    return;
  }

  if(strstr((char*)esp8266_uart.buffer, "ERROR")){
    esp8266_handle.replyFlag = AT_ERROR;
    return;
  }
}

static void AT_Config(void)
{
  switch (esp8266_handle.state){
  // 起始定时器延时
  case AT_INITIAL:{
    if((HAL_GetTick() - esp8266_handle.timeTick) > INITIAL_DELAY_MS){
      esp8266_handle.timeTick = HAL_GetTick();
      esp8266_handle.state = AT_SEND;
    }
    break;
  }
  // 发送、接收状态循环
  case AT_SEND:{
    if(esp8266_handle.curRetryCnt >= RETRY_COUNT){
      esp8266_handle.state = AT_FAIL;
      return;
    }
    esp8266_handle.curRetryCnt++;
    AT_Transmit(AT_CmdList[esp8266_handle.cmdIndex]);
    esp8266_handle.state = AT_WAIT_OK;
    esp8266_handle.timeTick = HAL_GetTick();
    break;
  }
  case AT_WAIT_OK:{
    if((esp8266_handle.replyFlag == AT_OK) || (esp8266_handle.replyFlag == AT_ERROR)) {
      esp8266_handle.cmdIndex++;
      if(esp8266_handle.cmdIndex == CMD_TOTAL_COUNT){
        esp8266_handle.state = AT_SUCCESS;
      }
      else{
        esp8266_handle.state = AT_SEND;
      }
      stateClear();
    }
    if((HAL_GetTick() - esp8266_handle.timeTick) > WAIT_AT_REPLY_MS){
      esp8266_handle.state = AT_SEND;
      esp8266_handle.timeTick = HAL_GetTick();
    }
    break;
  }
  case AT_SUCCESS:{
    printf("config success\n");
    LED_GREEN_ON();
    esp8266_handle.setConfigFlag = true;
    break;
  }
  case AT_FAIL:{
    LED_RED_ON();
    printf("config fail at %d cmd\n",esp8266_handle.cmdIndex + 1);
    esp8266_handle.setConfigFlag = true;
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

static void frameReply(const uint8_t txID, const char* data)
{
  char tx_buf[30] = {0};
  if(strlen(data) > 10)
    return;
  uint8_t  len = strlen(data);
	sprintf(tx_buf,"AT+CIPSEND=%u,%u\r\n", txID, len);    // 转成字符
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)tx_buf, strlen(tx_buf), ESP8266_UARTX_TIMEOUT);
  // 等待返回 '>' 直接delay了
  HAL_Delay(1);
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)data, strlen(data), ESP8266_UARTX_TIMEOUT);
}

static bool frameHeaderCheck(void)
{
  // 最短帧长
  if(esp8266_uart.index < ESP8266_RX_BUFF_MINLENTH){
    return false;
  }
  // 校验帧头
  if(memcmp(frameHead, esp8266_uart.buffer, FRAME_HEAD_LEN) != 0){
    return false;
  }
  return true;
}

static void frameProcess(void)
{
  // 提取客户端ID用于回复
  uint8_t txID = esp8266_uart.buffer[7] - '0';

  char *colon = strchr((char*)esp8266_uart.buffer, ':');  // 找冒号
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

static void frameCheck(void)
{
  printf("%s\n",esp8266_uart.buffer);
  // AT指令已配置，校验数据帧头 + 执行指令
  if(esp8266_handle.setConfigFlag) {
    if(!frameHeaderCheck())
      return;
    frameProcess();
  }
  // 处理AT指令帧
  else{
    AT_SetReplyFlag();
  }
}

// .c文件里定义的数组，传入结构体，使用结构体里的数组
void ESP8266_Init(void)
{
  UART_Channel_Init(&esp8266_uart,rx_buf,ESP8266_RX_BUFF_MAXLENTH);
  __HAL_UART_ENABLE_IT(ESP8266_UARTX, UART_IT_IDLE);
  HAL_UART_Receive_IT(ESP8266_UARTX, esp8266_uart.buffer, 1);
  esp8266_handle.state = AT_INITIAL;
  esp8266_handle.cmdIndex = 0;
  esp8266_handle.replyFlag = NONE;
  esp8266_handle.curRetryCnt = 0;
  esp8266_handle.timeTick = HAL_GetTick();
  esp8266_handle.setConfigFlag = false;
}

void ESP8266_SingleByteProcess(void)
{
  UART_Channel_SingleByteProcess(&esp8266_uart, ESP8266_UARTX);
}

void ESP8266_SetFrameEndFlag(void)
{
  UART_Channel_SetFrameEndFlag(&esp8266_uart);
}

void ESP8266_Task(void)
{
  if(!esp8266_handle.setConfigFlag){
    AT_Config();
  }
  if(esp8266_uart.frameEnd){
    frameCheck();
    HAL_UART_AbortReceive_IT(ESP8266_UARTX);
    UART_Channel_Clear(&esp8266_uart);
    HAL_UART_Receive_IT(ESP8266_UARTX, esp8266_uart.buffer, 1);
  }
}

void ESP8266_AT_Transmit(const char cmd[])
{
  uint16_t len = strlen(cmd);
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)cmd, len, ESP8266_UARTX_TIMEOUT);
}