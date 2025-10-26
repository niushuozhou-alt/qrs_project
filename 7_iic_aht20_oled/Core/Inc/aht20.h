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

/* Exported defines ------------------------------------------------------------*/
#define AHT20_DEFAULT_ADDRESS   0x70  // AHT20 I2C address (updated based on scan)

/* AHT20 Commands */
#define AHT20_REG_RESET         0xBA  // Soft reset command
#define AHT20_REG_INITIALIZE    0xBE  // Initialize command
#define AHT20_REG_MEASURE       0xAC  // Trigger measurement command

/* AHT20 Status bits */
#define AHT20_STATUS_BUSY       0x80  // Busy bit (Bit[7])
#define AHT20_STATUS_CALIBRATED 0x08  // Calibration enable bit (Bit[3])

/* AHT20 Timing constants */
#define AHT20_POWER_ON_DELAY_MS 40    // Power-on delay >= 40ms
#define AHT20_RESET_DELAY_MS   20     // Soft reset delay <= 20ms
#define AHT20_CALIB_DELAY_MS   10     // Calibration delay 10ms
#define AHT20_MEAS_DELAY_MS    75     // Measurement delay 75ms (from SparkFun)
#define AHT20_BUSY_RETRY_MS    1      // Retry delay when sensor is busy
#define AHT20_BUSY_TIMEOUT_MS  100    // Timeout for busy wait

/* Exported types --------------------------------------------------------------*/
typedef struct {
    float temperature;      // Temperature in Celsius
    float humidity;         // Humidity in %RH
} AHT20_Data_t;

typedef struct {
    uint32_t humidity;      // Raw humidity data (20 bits)
    uint32_t temperature;   // Raw temperature data (20 bits)
} AHT20_RawData_t;

typedef struct {
    uint8_t temperature : 1;  // Temperature data freshness flag
    uint8_t humidity : 1;     // Humidity data freshness flag
} AHT20_DataStatus_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;     // I2C handle pointer
    uint8_t deviceAddress;       // Device I2C address
    uint8_t measurementStarted;  // Measurement state flag
    AHT20_RawData_t sensorData;  // Raw sensor data
    AHT20_DataStatus_t sensorQueried; // Data freshness status
} AHT20_HandleTypeDef;

/* Exported function prototypes ------------------------------------------------*/
// Device status functions
HAL_StatusTypeDef AHT20_Init(AHT20_HandleTypeDef *aht20, I2C_HandleTypeDef *hi2c);
uint8_t AHT20_IsConnected(AHT20_HandleTypeDef *aht20);
uint8_t AHT20_Available(AHT20_HandleTypeDef *aht20);

// Measurement helper functions
uint8_t AHT20_GetStatus(AHT20_HandleTypeDef *aht20);
uint8_t AHT20_IsCalibrated(AHT20_HandleTypeDef *aht20);
uint8_t AHT20_IsBusy(AHT20_HandleTypeDef *aht20);
HAL_StatusTypeDef AHT20_Initialize(AHT20_HandleTypeDef *aht20);
HAL_StatusTypeDef AHT20_TriggerMeasurement(AHT20_HandleTypeDef *aht20);
void AHT20_ReadData(AHT20_HandleTypeDef *aht20);
HAL_StatusTypeDef AHT20_SoftReset(AHT20_HandleTypeDef *aht20);

// Make measurements
float AHT20_GetTemperature(AHT20_HandleTypeDef *aht20);
float AHT20_GetHumidity(AHT20_HandleTypeDef *aht20);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __AHT20_H__ */