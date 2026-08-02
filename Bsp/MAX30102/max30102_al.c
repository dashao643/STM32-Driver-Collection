#include "max30102_al.h"

#include <stddef.h>

#define MA4_SIZE        4
#define HAMMING_SIZE    5
#define SUM_HAMMING     1146

// 工作缓冲区(约 6KB SRAM)
static int32_t g_dx[MAX30102_ALGO_BUFFER_SIZE - MA4_SIZE];
static int32_t g_x[MAX30102_ALGO_BUFFER_SIZE];
static int32_t g_y[MAX30102_ALGO_BUFFER_SIZE];

// Hamming 窗系数 (已缩放 512 倍)
static const uint16_t hamming[5] = { 41, 276, 512, 276, 41 };

// SpO2 查找表, 公式: -45.060*ratio^2 + 30.354*ratio + 94.845
static const uint8_t spo2Table[184] = {
  95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98,
  99, 99, 99, 99, 99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 99, 99, 99, 99,
  99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 97, 97, 96, 96, 96, 96,
  95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 90, 90, 89, 89,
  89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 80, 80,
  79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67,
  66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52,
  51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34,
  33, 31, 30, 29, 28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12,
  11, 10, 9, 7, 6, 5, 3, 2, 1
};

static void findPeaks(int32_t *locs, int32_t *nPeaks, int32_t *x, int32_t size,
                      int32_t minHeight, int32_t minDistance, int32_t maxNum);
static void peaksAboveMinHeight(int32_t *locs, int32_t *nPeaks, int32_t *x,
                                int32_t size, int32_t minHeight);
static void removeClosePeaks(int32_t *locs, int32_t *nPeaks, int32_t *x,
                             int32_t minDistance);
static void sortAscend(int32_t *x, int32_t size);
static void sortIndicesDescend(const int32_t *x, int32_t *idx, int32_t size);

// --- 内部静态辅助函数 ---
static void findPeaks(int32_t *locs, int32_t *nPeaks, int32_t *x, int32_t size,
                      int32_t minHeight, int32_t minDistance, int32_t maxNum)
{
  peaksAboveMinHeight(locs, nPeaks, x, size, minHeight);
  removeClosePeaks(locs, nPeaks, x, minDistance);
  *nPeaks = (*nPeaks < maxNum) ? *nPeaks : maxNum;
}

static void peaksAboveMinHeight(int32_t *locs, int32_t *nPeaks, int32_t *x,
                                int32_t size, int32_t minHeight)
{
  int32_t i = 1, width;
  *nPeaks = 0;

  while (i < size - 1) {
    if (x[i] > minHeight && x[i] > x[i - 1]) {
      width = 1;
      while (i + width < size && x[i] == x[i + width]) {
        width++;
      }
      if (x[i] > x[i + width] && *nPeaks < 15) {
        locs[(*nPeaks)++] = i;
        i += width + 1;
      } else {
        i += width;
      }
    } else {
      i++;
    }
  }
}

static void removeClosePeaks(int32_t *locs, int32_t *nPeaks, int32_t *x,
                             int32_t minDistance)
{
  int32_t i, j, oldNPeaks, dist;

  sortIndicesDescend(x, locs, *nPeaks);

  for (i = -1; i < *nPeaks; i++) {
    oldNPeaks = *nPeaks;
    *nPeaks = i + 1;
    for (j = i + 1; j < oldNPeaks; j++) {
      dist = locs[j] - (i == -1 ? -1 : locs[i]);
      if (dist > minDistance || dist < -minDistance) {
        locs[(*nPeaks)++] = locs[j];
      }
    }
  }

  sortAscend(locs, *nPeaks);
}

static void sortAscend(int32_t *x, int32_t size)
{
  int32_t i, j, temp;
  for (i = 1; i < size; i++) {
    temp = x[i];
    for (j = i; j > 0 && temp < x[j - 1]; j--) {
      x[j] = x[j - 1];
    }
    x[j] = temp;
  }
}

static void sortIndicesDescend(const int32_t *x, int32_t *idx, int32_t size)
{
  int32_t i, j, temp;
  for (i = 1; i < size; i++) {
    temp = idx[i];
    for (j = i; j > 0 && x[temp] > x[idx[j - 1]]; j--) {
      idx[j] = idx[j - 1];
    }
    idx[j] = temp;
  }
}

/*-----------------------------------------------------------------------------------------*/

/**
 * @brief 计算原始ir和red数据,得出心率和血氧值
 * 
 * @param input 输入的原始数据
 *  @note input.length值必须为500
 * @param output 输出计算出的心率和血氧值
 */
void MAX30102_Algo_Process(const MAX30102_AlgoInput_t *input, MAX30102_AlgoOutput_t *output)
{
  int32_t irValleyLocs[15];
  int32_t exactIrValleyLocs[15];
  int32_t dxPeakLocs[15];

  uint32_t irMean = 0;
  int32_t i, k, s;
  int32_t nPeaks, nExactValleys;
  int32_t threshold;
  int32_t peakIntervalSum;
  int32_t yAc, xAc, yDcMax, xDcMax;
  int32_t yDcMaxIdx, xDcMaxIdx;
  int32_t ratios[5], ratioAvg;
  int32_t nume, denom, middleIdx;
  int32_t nRatioCount = 0;
  uint8_t onlyOnce;
  int32_t cMin;
  int32_t spo2Calc;

  // 初始化输出
  output->heartRate = -999;
  output->hrValid = 0;
  output->spo2 = -999;
  output->spo2Valid = 0;

  // 算法要求固定 500 点, 采样率 100Hz, 约 5 秒数据
  if (input == NULL || output == NULL ||
      input->irBuffer == NULL || input->redBuffer == NULL ||
      input->length != MAX30102_ALGO_BUFFER_SIZE) {
    return;
  }

  // 1. 去除 IR 直流分量
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE; k++) {
    irMean += input->irBuffer[k];
  }
  irMean /= MAX30102_ALGO_BUFFER_SIZE;

  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE; k++) {
    g_x[k] = (int32_t)input->irBuffer[k] - (int32_t)irMean;
  }

  // 2. 4 点滑动平均
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE - MA4_SIZE; k++) {
    denom = g_x[k] + g_x[k + 1] + g_x[k + 2] + g_x[k + 3];
    g_x[k] = denom / 4;
  }

  // 3. 对平滑后的 IR 求差分
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE - MA4_SIZE - 1; k++) {
    g_dx[k] = g_x[k + 1] - g_x[k];
  }

  // 4. 对差分结果做 2 点滑动平均
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE - MA4_SIZE - 2; k++) {
    g_dx[k] = (g_dx[k] + g_dx[k + 1]) / 2;
  }

  // 5. Hamming 窗滤波, 并翻转波形 (这样可以用峰值检测来找原始信号的谷值)
  for (i = 0; i < MAX30102_ALGO_BUFFER_SIZE - HAMMING_SIZE - MA4_SIZE - 2; i++) {
    s = 0;
    for (k = i; k < i + HAMMING_SIZE; k++) {
      s -= g_dx[k] * hamming[k - i];
    }
    g_dx[i] = s / SUM_HAMMING;
  }

  // 6. 计算阈值
  threshold = 0;
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE - HAMMING_SIZE; k++) {
    threshold += (g_dx[k] > 0) ? g_dx[k] : -g_dx[k];
  }
  threshold /= (MAX30102_ALGO_BUFFER_SIZE - HAMMING_SIZE);

  // 7. 找峰值 (对应原始 IR 信号的谷值)
  findPeaks(dxPeakLocs, &nPeaks, g_dx, MAX30102_ALGO_BUFFER_SIZE - HAMMING_SIZE,
            threshold, 8, 5);

  // 8. 计算心率 (采样率 100Hz, 间隔单位 10ms)
  if (nPeaks >= 2) {
    peakIntervalSum = 0;
    for (k = 1; k < nPeaks; k++) {
      peakIntervalSum += (dxPeakLocs[k] - dxPeakLocs[k - 1]);
    }
    peakIntervalSum /= (nPeaks - 1);
    output->heartRate = 6000 / peakIntervalSum;  // BPM
    output->hrValid = 1;
  } else {
    output->heartRate = -999;
    output->hrValid = 0;
  }

  // 将峰值位置映射回原始信号的大致谷值位置
  for (k = 0; k < nPeaks; k++) {
    irValleyLocs[k] = dxPeakLocs[k] + HAMMING_SIZE / 2;
  }

  // 9. 复制原始 IR 和 Red 数据用于 SpO2 计算
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE; k++) {
    g_x[k] = (int32_t)input->irBuffer[k];
    g_y[k] = (int32_t)input->redBuffer[k];
  }

  // 10. 在近似谷值位置附近寻找精确最小值
  nExactValleys = 0;
  for (k = 0; k < nPeaks; k++) {
    onlyOnce = 1;
    int32_t m = irValleyLocs[k];
    cMin = 16777216;  // 2^24
    if (m + 5 < MAX30102_ALGO_BUFFER_SIZE - HAMMING_SIZE && m - 5 > 0) {
      for (i = m - 5; i < m + 5; i++) {
        if (g_x[i] < cMin) {
          onlyOnce = 0;
          cMin = g_x[i];
          exactIrValleyLocs[k] = i;
        }
      }
      if (onlyOnce == 0) {
        nExactValleys++;
      }
    }
  }

  if (nExactValleys < 2) {
    output->spo2 = -999;
    output->spo2Valid = 0;
    return;
  }

  // 11. 对原始信号再做 4 点滑动平均
  for (k = 0; k < MAX30102_ALGO_BUFFER_SIZE - MA4_SIZE; k++) {
    g_x[k] = (g_x[k] + g_x[k + 1] + g_x[k + 2] + g_x[k + 3]) / 4;
    g_y[k] = (g_y[k] + g_y[k + 1] + g_y[k + 2] + g_y[k + 3]) / 4;
  }

  // 12. 计算 AC/DC 比值并查表得 SpO2
  for (k = 0; k < 5; k++) {
    ratios[k] = 0;
  }

  for (k = 0; k < nExactValleys; k++) {
    if (exactIrValleyLocs[k] > MAX30102_ALGO_BUFFER_SIZE) {
      output->spo2 = -999;
      output->spo2Valid = 0;
      return;
    }
  }

  for (k = 0; k < nExactValleys - 1; k++) {
    yDcMax = -16777216;
    xDcMax = -16777216;

    if (exactIrValleyLocs[k + 1] - exactIrValleyLocs[k] > 10) {
      for (i = exactIrValleyLocs[k]; i < exactIrValleyLocs[k + 1]; i++) {
        if (g_x[i] > xDcMax) {
          xDcMax = g_x[i];
          xDcMaxIdx = i;
        }
        if (g_y[i] > yDcMax) {
          yDcMax = g_y[i];
          yDcMaxIdx = i;
        }
      }

      // Red AC: 扣除线性直流分量后的幅值
      yAc = (g_y[exactIrValleyLocs[k + 1]] - g_y[exactIrValleyLocs[k]]) *
            (yDcMaxIdx - exactIrValleyLocs[k]);
      yAc = g_y[exactIrValleyLocs[k]] +
            yAc / (exactIrValleyLocs[k + 1] - exactIrValleyLocs[k]);
      yAc = g_y[yDcMaxIdx] - yAc;

      // IR AC
      xAc = (g_x[exactIrValleyLocs[k + 1]] - g_x[exactIrValleyLocs[k]]) *
            (xDcMaxIdx - exactIrValleyLocs[k]);
      xAc = g_x[exactIrValleyLocs[k]] +
            xAc / (exactIrValleyLocs[k + 1] - exactIrValleyLocs[k]);
      xAc = g_x[xDcMaxIdx] - xAc;

      nume = (yAc * xDcMax) >> 7;
      denom = (xAc * yDcMax) >> 7;

      if (denom > 0 && nRatioCount < 5 && nume != 0) {
        ratios[nRatioCount] = (nume * 20) / denom;
        nRatioCount++;
      }
    }
  }

  sortAscend(ratios, nRatioCount);
  middleIdx = nRatioCount / 2;

  if (middleIdx > 1) {
    ratioAvg = (ratios[middleIdx - 1] + ratios[middleIdx]) / 2;
  } else {
    ratioAvg = ratios[middleIdx];
  }

  if (ratioAvg > 2 && ratioAvg < 184) {
    spo2Calc = spo2Table[ratioAvg];
    output->spo2 = spo2Calc;
    output->spo2Valid = 1;
  } else {
    output->spo2 = -999;
    output->spo2Valid = 0;
  }
}