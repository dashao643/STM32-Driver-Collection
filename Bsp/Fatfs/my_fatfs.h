#ifndef __MY_FATFS_H__
#define __MY_FATFS_H__

// 调用接口
#include "ff.h"
// 底层驱动
#include "diskio.h"
// 数据定义
#include "integer.h"
#include <stdbool.h>

bool Fatfs_Mount(void);
void Fatfs_GetDiskInfo(void);
void Fatfs_ScanDir(const TCHAR* basePath);
void Fatfs_GetInfo(const TCHAR* filePath);
void Fatfs_ReadFile(const TCHAR* filePath);
void Fatfs_AppendWrite(const TCHAR* filePath, const TCHAR* content, UINT length);
void Fatfs_WriteTest(void);

// 物理驱动只支持单个: 默认 pdrv: 0
DRESULT Fatfs_ReadSector(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT Fatfs_WriteSector(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
DRESULT Fatfs_IO_Ctrl(BYTE pdrv, BYTE cmd, void *buff);
DWORD Fatfs_GetTimestamp(void);

#endif
