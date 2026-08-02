#include "stm32f4xx_hal.h"
#include "my_fatfs.h"
#include "integer.h"
#include "my_rtc.h"
// #include "general.h"

// CubeMX调用
#include "fatfs.h"
#include <stdint.h>
#include <string.h>

#define UART_DEBUG

#ifdef UART_DEBUG
#include <stdio.h>
#endif

// 选择物理存储介质和对应磁盘驱动编号(从0开始递增)
// #define W25Q64_DRIVE      0
#define SD_CARD_DRIVE     0

#ifdef W25Q64_DRIVE
#include "w25q64.h"
#endif

#define SECTOR_SIZE       512
#define WORK_BUFF_SIZE    (4 * SECTOR_SIZE)
// 挂载磁盘传入的工作区数组大小(更大的缓冲区能显著减少对存储介质的写入次数, 让格式化过程更快)

static BYTE workBuf[WORK_BUFF_SIZE];

static void printLog(const char* info)
{
#ifdef UART_DEBUG
  printf("%s\n", info);
#endif
}

static void printErrorCode(FRESULT code)
{
#ifdef UART_DEBUG
  if(code != FR_OK)
    printf("error code=%d\n", code);
#endif
}

static void printInfo(const FILINFO* fileInfo)
{
#ifdef UART_DEBUG
  printf("name: %s\n", fileInfo->fname);

  if(fileInfo->fattrib & AM_DIR) 
    printf("type: directory\n");
  else if(fileInfo->fattrib & AM_ARC)
    printf("type: file\n");
  else 
    printf("type: other (0x%02X)\n", fileInfo->fattrib);

  // 如果是文件夹, 大小固定为0B
  printf("size: %dB\n", (int)fileInfo->fsize);

  int year = (fileInfo->fdate >> 9) + 1980;
  int month = (fileInfo->fdate & 0x1FF) >> 5;
  int date = fileInfo->fdate & 0x1F;
  printf("date: %d-%d-%d\n", year, month, date);

  int hour = fileInfo->ftime >> 11;
  int minute = (fileInfo->ftime & 0x7FF) >> 5;
  int second = (fileInfo->ftime & 0x1F) * 2;
  printf("time: %02d:%02d:%02d\n", hour, minute, second);
#endif
}

/*---------------------------- APP ----------------------------*/
// 挂载一个驱动器, 第一次挂载失败则格式化
bool Fatfs_Mount(void)
{
  // F1: USERFatFS, F4: SDFatFS
  if (f_mount(&SDFatFS, "0:", 1) != FR_OK) {
    HAL_Delay(1000);
    // sdf=1: 开销小, 嵌入式设备. sfd=0: PC
    // F1, 簇大小设置为0: 自动设置
    // FRESULT res = f_mkfs("0:", 0, 0);
    // F4, 不超过32GB容量, 使用 FM_FAT32, 工作区数组, 
    FRESULT res = f_mkfs("0:", FM_FAT32, 0, workBuf,WORK_BUFF_SIZE);
    
    if (res != FR_OK) {
      printLog("fatfs format fail");
      printErrorCode(res);
      return false;
    }
    printLog("fatfs format success");
    return true;
  }

  printLog("disk mount success");

  return true;
}

// 获取磁盘0:信息
void Fatfs_GetDiskInfo(void)
{
  DWORD nclst;
  FATFS *fatfs;

  FRESULT res = f_getfree("0:", &nclst, &fatfs);
  printErrorCode(res);

#ifdef UART_DEBUG
  // 0=未挂载, 1=FAT12, 2=FAT16, 3=FAT32
  printf("文件系统类型: %d\n", fatfs->fs_type);
  // 1,2,4,8,16,32,64,128
  printf("每簇扇区数: %d\n", fatfs->csize);
  printf("FAT表数量: %d\n", fatfs->n_fats);
  printf("FAT表扇区数: %lu\n", (unsigned long)fatfs->fsize);
  printf("FAT条目数: %lu\n", (unsigned long)fatfs->n_fatent);
  printf("数据区起始扇区: %lu\n", (unsigned long)fatfs->database);
  printf("剩余簇数: %lu\n", (unsigned long)nclst);
  printf("剩余空间(MB): %d\n", (int)((unsigned long long)nclst * fatfs->csize * SECTOR_SIZE / 1000000));
#endif
}

// 获取指定目录下所有目录和文件(非递归), 根目录(0:)
void Fatfs_ScanDir(const TCHAR* basePath)
{
  DIR dir;
  FRESULT res = f_opendir(&dir, basePath);

  if(res != FR_OK) {
    printErrorCode(res);
    return;
  }

  FILINFO fileInfo;

  uint16_t index = 0;
  while(1) {
    res = f_readdir(&dir, &fileInfo);
    // 出错或文件名为空表示已读完
    if (res != FR_OK || fileInfo.fname[0] == 0)
      break;

    index++;
#ifdef UART_DEBUG
    printf("-------item%d info-------\n", index);
#endif
    printInfo(&fileInfo);
  }

  if(index == 0) {
#ifdef UART_DEBUG
    printf("directory is empty\n");
#endif
  }

  f_closedir(&dir);
}

// 指定路径, 获取文件夹或文件信息
void Fatfs_GetInfo(const TCHAR* filePath)
{
  FILINFO fileInfo;

  FRESULT res = f_stat(filePath, &fileInfo);

  if(res != FR_OK) {
    printErrorCode(res);
    return;
  }
  printInfo(&fileInfo);
}

// 最多读取 256B 内容
void Fatfs_ReadFile(const TCHAR* filePath)
{
  FIL file;
  char content[256];

  FRESULT res = f_open(&file, filePath, FA_READ);

  if(res != FR_OK) {
    printErrorCode(res);
    return;
  }

  printLog("content:");

  while(!f_eof(&file)) {
    f_gets(content, 256, &file);
    printLog(content);
  }

  f_close(&file);
}

/*
FA_OPEN_EXISTING	0x00	    文件必须存在
FA_CREATE_NEW	    0x04	    文件不能存在
FA_CREATE_ALWAYS	0x08	    文件存在则覆盖, 不存在则创建(覆盖写入)
FA_OPEN_ALWAYS	  0x10	    文件存在则打开, 不存在则创建(追加写入)
*/

// 追加写入文件
void Fatfs_AppendWrite(const TCHAR* filePath, const TCHAR* content, UINT length)
{
  FIL file;
  UINT bw;

  FRESULT res = f_open(&file, filePath, FA_WRITE | FA_OPEN_ALWAYS);

  if(res != FR_OK) {
    printErrorCode(res);
    return;
  }

  // 将文件指针移动到文件末尾
  res = f_lseek(&file, f_size(&file));

  if(res != FR_OK) {
    printErrorCode(res);
    return;
  }
  res = f_write(&file, content, length, &bw);
  printErrorCode(res);
  if(bw != length)
    printLog("write fail");

  f_close(&file);
}

// 0:/src/main.c 写入hello world
void Fatfs_WriteTest(void)
{
  f_mkdir("0:/src");

  FIL file;
  f_open(&file, "0:/src/main.c", FA_WRITE | FA_CREATE_ALWAYS);
  char content[] = {
    "#include <stdio.h>\n"
    "\n"
    "int main()\n"
    "{\n"
    "    printf(\"hello world!\\n\");\n"
    "\n"
    "    return 0;\n"
    "}\n"
  };
  UINT resSize;
  f_write(&file, content, strlen(content), &resSize);
  f_close(&file);
}

/*---------------------------- BSP ----------------------------*/

DRESULT Fatfs_ReadSector(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
#ifdef W25Q64_DRIVE
  if(pdrv == W25Q64_DRIVE) {
    BYTE state = W25Q64_ReadSector(sector, buff, count * W25Q64_SECTOR_SIZE);

    if(state == HAL_OK) return RES_OK;
    else if(state == HAL_ERROR) return RES_ERROR;
    else if(state == HAL_BUSY) return RES_NOTRDY;
  }
#endif

  return RES_ERROR;
}

DRESULT Fatfs_WriteSector(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
#ifdef W25Q64_DRIVE
  if(pdrv == W25Q64_DRIVE) {
    for(uint8_t i = 0; i < count; i++)
      if (W25Q64_EraseSector(sector + i) != HAL_OK) return RES_ERROR;

    if (W25Q64_WriteSector(sector, buff, count * W25Q64_SECTOR_SIZE) == HAL_OK)
      return RES_OK;
  }
#endif

  return RES_ERROR;
}

DRESULT Fatfs_IO_Ctrl(BYTE pdrv, BYTE cmd, void *buff)
{
#ifdef W25Q64_DRIVE
  if(pdrv == W25Q64_DRIVE) {
    // 无缓存操作
    if (cmd == CTRL_SYNC);
    else if (cmd == GET_SECTOR_COUNT)
      *(DWORD*)buff = W25Q64_SECTOR_CNT;
    else if (cmd == GET_SECTOR_SIZE)
      *(DWORD*)buff = W25Q64_SECTOR_SIZE;
    // 获取擦除块大小(32KB, 64KB 块擦除: 8, 16)
    else if (cmd == GET_BLOCK_SIZE)
      *(DWORD*)buff = 8;
    else if (cmd == CTRL_TRIM)
      *(DWORD*)buff = W25Q64_SECTOR_CNT;
    else
      return RES_ERROR;
  }
  return RES_OK;
#endif

  return RES_ERROR;
}

DWORD Fatfs_GetTimestamp(void)
{
  RTC_DateTypeDef date;
  RTC_TimeTypeDef time;

  RTC_GetDateTime(&date, &time);

  // uint32_t: 
  // bit31:25: Year origin from the 1980
  // bit24:21: Month (1..12)
  // bit20:16: Day of the month (1..31)
  // bit15:11: Hour (0..23)
  // bit10:5: Minute (0..59)
  // bit4:0: Second / 2 (0..29, e.g. 25 for 50)

  return (DWORD)(date.Year + 2000 - 1980) << 25 |
         (DWORD)date.Month << 21 |
         (DWORD)date.Date << 16 |
         (DWORD)time.Hours << 11 |
         (DWORD)time.Minutes << 5 |
         (DWORD)time.Seconds >> 1;
}
