/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    aht20.c
  * @brief   AHT20 Temperature and Humidity Sensor Driver Implementation
  *          Based on SparkFun Qwiic Humidity AHT20 Library
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "aht20.h"

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef AHT20_WriteCommand(AHT20_HandleTypeDef *aht20, uint8_t cmd);
static HAL_StatusTypeDef AHT20_WriteCommandWithParams(AHT20_HandleTypeDef *aht20, uint8_t cmd, uint8_t param1, uint8_t param2);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize AHT20 sensor
  * @param  aht20: Pointer to AHT20 handle structure
  * @param  hi2c: Pointer to I2C handle
  * @retval HAL status
  */
HAL_StatusTypeDef AHT20_Init(AHT20_HandleTypeDef *aht20, I2C_HandleTypeDef *hi2c)
{
    if (aht20 == NULL || hi2c == NULL) {
        return HAL_ERROR;
    }

    // Initialize handle structure
    aht20->hi2c = hi2c;
    aht20->deviceAddress = AHT20_DEFAULT_ADDRESS;
    aht20->measurementStarted = 0;

    // Wait 40 ms after power-on before reading temp or humidity. Datasheet pg 8
    HAL_Delay(AHT20_POWER_ON_DELAY_MS);

    // Check if connected
    if (AHT20_IsConnected(aht20) == 0) {
        return HAL_ERROR;
    }

    // Read status for debugging
    uint8_t status = AHT20_GetStatus(aht20);

    // Check if the calibrated bit is set. If not, init the sensor.
    if ((status & AHT20_STATUS_CALIBRATED) == 0) {
        // Send 0xBE0800
        if (AHT20_Initialize(aht20) != HAL_OK) {
            return HAL_ERROR;
        }

        HAL_Delay(AHT20_CALIB_DELAY_MS); // Wait for calibration to complete

        // Immediately trigger a measurement. Send 0xAC3300
        if (AHT20_TriggerMeasurement(aht20) != HAL_OK) {
            return HAL_ERROR;
        }

        HAL_Delay(AHT20_MEAS_DELAY_MS); // Wait for measurement to complete

        uint8_t counter = 0;
        while (AHT20_IsBusy(aht20)) {
            HAL_Delay(AHT20_BUSY_RETRY_MS);
            if (counter++ > (AHT20_BUSY_TIMEOUT_MS / AHT20_BUSY_RETRY_MS)) {
                return HAL_ERROR; // Give up after 100ms
            }
        }

        // Read data to complete calibration sequence
        AHT20_ReadData(aht20);

        // Wait a bit more and check calibration status again
        HAL_Delay(10);
        status = AHT20_GetStatus(aht20);
        if ((status & AHT20_STATUS_CALIBRATED) == 0) {
            return HAL_ERROR;
        }
    }

    // Mark all datums as fresh (not read before)
    aht20->sensorQueried.temperature = 0;
    aht20->sensorQueried.humidity = 0;

    return HAL_OK;
}

/**
  * @brief  Check if AHT20 is connected to I2C bus
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval 1 if connected, 0 if not connected
  */
uint8_t AHT20_IsConnected(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return 0;
    }

    // Try to communicate with device
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(aht20->hi2c, aht20->deviceAddress, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        return 1;
    }

    // If IC failed to respond, give it 20ms more for Power On Startup
    // Datasheet pg 7
    HAL_Delay(AHT20_RESET_DELAY_MS);

    status = HAL_I2C_IsDeviceReady(aht20->hi2c, aht20->deviceAddress, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        return 1;
    }

    return 0;
}

/**
  * @brief  Check if new data is available
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval 1 if new data available, 0 if not ready
  */
uint8_t AHT20_Available(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL) {
        return 0;
    }

    if (aht20->measurementStarted == 0) {
        AHT20_TriggerMeasurement(aht20);
        aht20->measurementStarted = 1;
        return 0;
    }

    if (AHT20_IsBusy(aht20) == 1) {
        return 0;
    }

    AHT20_ReadData(aht20);
    aht20->measurementStarted = 0;
    return 1;
}

/**
  * @brief  Get status byte from AHT20
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval Status byte
  */
uint8_t AHT20_GetStatus(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return 0;
    }

    uint8_t status = 0;
    // Try both read addresses: 0x38 (write) and 0x39 (read)
    // AHT20 uses 0x38 for write, 0x39 for read
    HAL_StatusTypeDef result = HAL_I2C_Master_Receive(aht20->hi2c, aht20->deviceAddress + 1, &status, 1, HAL_MAX_DELAY);
    if (result != HAL_OK) {
        return 0;
    }
    return status;
}

/**
  * @brief  Check if AHT20 is calibrated
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval 1 if calibrated, 0 if not calibrated
  */
uint8_t AHT20_IsCalibrated(AHT20_HandleTypeDef *aht20)
{
    return (AHT20_GetStatus(aht20) & (1 << 3));
}

/**
  * @brief  Check if AHT20 is busy
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval 1 if busy, 0 if ready
  */
uint8_t AHT20_IsBusy(AHT20_HandleTypeDef *aht20)
{
    return (AHT20_GetStatus(aht20) & (1 << 7));
}

/**
  * @brief  Initialize AHT20 sensor
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval HAL status
  */
HAL_StatusTypeDef AHT20_Initialize(AHT20_HandleTypeDef *aht20)
{
    return AHT20_WriteCommandWithParams(aht20, AHT20_REG_INITIALIZE, 0x80, 0x00);
}

/**
  * @brief  Trigger measurement
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval HAL status
  */
HAL_StatusTypeDef AHT20_TriggerMeasurement(AHT20_HandleTypeDef *aht20)
{
    return AHT20_WriteCommandWithParams(aht20, AHT20_REG_MEASURE, 0x33, 0x00);
}

/**
  * @brief  Read and parse the 6 bytes of data into raw humidity and temperature
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval None
  */
void AHT20_ReadData(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return;
    }

    // Clear previous data
    aht20->sensorData.temperature = 0;
    aht20->sensorData.humidity = 0;

    uint8_t buffer[6] = {0};
    // Use read address (0x39) for receiving data
    HAL_StatusTypeDef result = HAL_I2C_Master_Receive(aht20->hi2c, aht20->deviceAddress + 1, buffer, 6, HAL_MAX_DELAY);

    if (result != HAL_OK) {
        return;
    }

    uint8_t state = buffer[0];

    uint32_t incoming = 0;
    incoming |= (uint32_t)buffer[1] << (8 * 2);
    incoming |= (uint32_t)buffer[2] << (8 * 1);
    uint8_t midByte = buffer[3];

    incoming |= midByte;
    aht20->sensorData.humidity = incoming >> 4;

    aht20->sensorData.temperature = (uint32_t)midByte << (8 * 2);
    aht20->sensorData.temperature |= (uint32_t)buffer[4] << (8 * 1);
    aht20->sensorData.temperature |= (uint32_t)buffer[5] << (8 * 0);

    // Need to get rid of data in bits > 20
    aht20->sensorData.temperature = aht20->sensorData.temperature & ~(0xFFF00000);

    // Mark data as fresh
    aht20->sensorQueried.temperature = 0;
    aht20->sensorQueried.humidity = 0;
}

/**
  * @brief  Perform soft reset of AHT20 sensor
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval HAL status
  */
HAL_StatusTypeDef AHT20_SoftReset(AHT20_HandleTypeDef *aht20)
{
    return AHT20_WriteCommand(aht20, AHT20_REG_RESET);
}

/**
  * @brief  Get temperature measurement
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval Temperature in degrees Celsius
  */
float AHT20_GetTemperature(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL) {
        return -999.0f; // Error value
    }

    if (aht20->sensorQueried.temperature == 1) {
        // We've got old data so trigger new measurement
        AHT20_TriggerMeasurement(aht20);

        HAL_Delay(AHT20_MEAS_DELAY_MS); // Wait for measurement to complete

        uint8_t counter = 0;
        while (AHT20_IsBusy(aht20)) {
            HAL_Delay(AHT20_BUSY_RETRY_MS);
            if (counter++ > (AHT20_BUSY_TIMEOUT_MS / AHT20_BUSY_RETRY_MS)) {
                return -999.0f; // Give up after 100ms
            }
        }

        AHT20_ReadData(aht20);
    }

    // From datasheet pg 8
    float tempCelsius = ((float)aht20->sensorData.temperature / 1048576) * 200 - 50;

    // Mark data as old
    aht20->sensorQueried.temperature = 1;

    return tempCelsius;
}

/**
  * @brief  Get humidity measurement
  * @param  aht20: Pointer to AHT20 handle structure
  * @retval Humidity in %RH
  */
float AHT20_GetHumidity(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL) {
        return -999.0f; // Error value
    }

    if (aht20->sensorQueried.humidity == 1) {
        // We've got old data so trigger new measurement
        AHT20_TriggerMeasurement(aht20);

        HAL_Delay(AHT20_MEAS_DELAY_MS); // Wait for measurement to complete

        uint8_t counter = 0;
        while (AHT20_IsBusy(aht20)) {
            HAL_Delay(AHT20_BUSY_RETRY_MS);
            if (counter++ > (AHT20_BUSY_TIMEOUT_MS / AHT20_BUSY_RETRY_MS)) {
                return -999.0f; // Give up after 100ms
            }
        }

        AHT20_ReadData(aht20);
    }

    // From datasheet pg 8
    float relHumidity = ((float)aht20->sensorData.humidity / 1048576) * 100;

    // Mark data as old
    aht20->sensorQueried.humidity = 1;

    return relHumidity;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Write single command to AHT20
  * @param  aht20: Pointer to AHT20 handle structure
  * @param  cmd: Command byte
  * @retval HAL status
  */
static HAL_StatusTypeDef AHT20_WriteCommand(AHT20_HandleTypeDef *aht20, uint8_t cmd)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Transmit(aht20->hi2c, aht20->deviceAddress, &cmd, 1, HAL_MAX_DELAY);
}

/**
  * @brief  Write command with parameters to AHT20
  * @param  aht20: Pointer to AHT20 handle structure
  * @param  cmd: Command byte
  * @param  param1: First parameter byte
  * @param  param2: Second parameter byte
  * @retval HAL status
  */
static HAL_StatusTypeDef AHT20_WriteCommandWithParams(AHT20_HandleTypeDef *aht20, uint8_t cmd, uint8_t param1, uint8_t param2)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t buffer[3] = {cmd, param1, param2};
    return HAL_I2C_Master_Transmit(aht20->hi2c, aht20->deviceAddress, buffer, 3, HAL_MAX_DELAY);
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */