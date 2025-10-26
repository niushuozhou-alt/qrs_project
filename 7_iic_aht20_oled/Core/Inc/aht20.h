/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    aht20.h
  * @brief   AHT20 Temperature and Humidity Sensor Driver Header
  *          Based on SparkFun Qwiic Humidity AHT20 Library
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __AHT20_H__
#define __AHT20_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"

/* 导出定义 ------------------------------------------------------------*/
#define AHT20_DEFAULT_ADDRESS   0x70  // AHT20 I2C地址 (基于扫描结果更新)

/* AHT20命令 */
#define AHT20_REG_RESET         0xBA  // 软复位命令
#define AHT20_REG_INITIALIZE    0xBE  // 初始化命令
#define AHT20_REG_MEASURE       0xAC  // 触发测量命令

/* AHT20状态位 */
#define AHT20_STATUS_BUSY       0x80  // 忙状态位 (Bit[7])
#define AHT20_STATUS_CALIBRATED 0x08  // 校准使能位 (Bit[3])

/* AHT20时序常量 */
#define AHT20_POWER_ON_DELAY_MS 40    // 上电延时 >= 40ms
#define AHT20_RESET_DELAY_MS   20     // 软复位延时 <= 20ms
#define AHT20_CALIB_DELAY_MS   10     // 校准延时 10ms
#define AHT20_MEAS_DELAY_MS    75     // 测量延时 75ms (来自SparkFun)
#define AHT20_BUSY_RETRY_MS    1      // 传感器忙时重试延时
#define AHT20_BUSY_TIMEOUT_MS  100    // 忙等待超时时间

/* 导出类型 --------------------------------------------------------------*/
typedef struct {
    float temperature;      // 摄氏温度
    float humidity;         // 相对湿度 %RH
} AHT20_Data_t;

typedef struct {
    uint32_t humidity;      // 原始湿度数据 (20位)
    uint32_t temperature;   // 原始温度数据 (20位)
} AHT20_RawData_t;

typedef struct {
    uint8_t temperature : 1;  // 温度数据新鲜度标志
    uint8_t humidity : 1;     // 湿度数据新鲜度标志
} AHT20_DataStatus_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;     // I2C句柄指针
    uint8_t deviceAddress;       // 设备I2C地址
    uint8_t measurementStarted;  // 测量状态标志
    AHT20_RawData_t sensorData;  // 原始传感器数据
    AHT20_DataStatus_t sensorQueried; // 数据新鲜度状态
} AHT20_HandleTypeDef;

/* 导出函数原型 ------------------------------------------------*/
// 设备状态函数
HAL_StatusTypeDef AHT20_Init(AHT20_HandleTypeDef *aht20, I2C_HandleTypeDef *hi2c);
uint8_t AHT20_IsConnected(AHT20_HandleTypeDef *aht20);
uint8_t AHT20_Available(AHT20_HandleTypeDef *aht20);

// 测量辅助函数
uint8_t AHT20_GetStatus(AHT20_HandleTypeDef *aht20);
uint8_t AHT20_IsCalibrated(AHT20_HandleTypeDef *aht20);
uint8_t AHT20_IsBusy(AHT20_HandleTypeDef *aht20);
HAL_StatusTypeDef AHT20_Initialize(AHT20_HandleTypeDef *aht20);
HAL_StatusTypeDef AHT20_TriggerMeasurement(AHT20_HandleTypeDef *aht20);
void AHT20_ReadData(AHT20_HandleTypeDef *aht20);
HAL_StatusTypeDef AHT20_SoftReset(AHT20_HandleTypeDef *aht20);

// 执行测量
float AHT20_GetTemperature(AHT20_HandleTypeDef *aht20);
float AHT20_GetHumidity(AHT20_HandleTypeDef *aht20);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __AHT20_H__ */