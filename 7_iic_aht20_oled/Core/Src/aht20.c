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

/* 私有函数原型 -----------------------------------------------*/
static HAL_StatusTypeDef AHT20_WriteCommand(AHT20_HandleTypeDef *aht20, uint8_t cmd);
static HAL_StatusTypeDef AHT20_WriteCommandWithParams(AHT20_HandleTypeDef *aht20, uint8_t cmd, uint8_t param1, uint8_t param2);

/* 导出函数 --------------------------------------------------------*/

/**
  * @brief  初始化AHT20传感器
  * @param  aht20: AHT20句柄结构体指针
  * @param  hi2c: I2C句柄指针
  * @retval HAL状态
  */
HAL_StatusTypeDef AHT20_Init(AHT20_HandleTypeDef *aht20, I2C_HandleTypeDef *hi2c)
{
    if (aht20 == NULL || hi2c == NULL) {
        return HAL_ERROR;
    }

    // 初始化句柄结构体
    aht20->hi2c = hi2c;
    aht20->deviceAddress = AHT20_DEFAULT_ADDRESS;
    aht20->measurementStarted = 0;

    // 上电后等待40ms再读取温湿度，数据手册第8页
    HAL_Delay(AHT20_POWER_ON_DELAY_MS);

    // 检查连接状态
    if (AHT20_IsConnected(aht20) == 0) {
        return HAL_ERROR;
    }

    // 读取状态用于调试
    uint8_t status = AHT20_GetStatus(aht20);

    // 检查校准位是否设置，如果没有则初始化传感器
    if ((status & AHT20_STATUS_CALIBRATED) == 0) {
        // 发送初始化命令 0xBE0800
        if (AHT20_Initialize(aht20) != HAL_OK) {
            return HAL_ERROR;
        }

        HAL_Delay(AHT20_CALIB_DELAY_MS); // 等待校准完成

        // 立即触发测量，发送命令 0xAC3300
        if (AHT20_TriggerMeasurement(aht20) != HAL_OK) {
            return HAL_ERROR;
        }

        HAL_Delay(AHT20_MEAS_DELAY_MS); // 等待测量完成

        uint8_t counter = 0;
        while (AHT20_IsBusy(aht20)) {
            HAL_Delay(AHT20_BUSY_RETRY_MS);
            if (counter++ > (AHT20_BUSY_TIMEOUT_MS / AHT20_BUSY_RETRY_MS)) {
                return HAL_ERROR; // 100ms后放弃
            }
        }

        // 读取数据以完成校准序列
        AHT20_ReadData(aht20);

        // 再等待一下并重新检查校准状态
        HAL_Delay(10);
        status = AHT20_GetStatus(aht20);
        if ((status & AHT20_STATUS_CALIBRATED) == 0) {
            return HAL_ERROR;
        }
    }

    // 将所有数据标记为新鲜（之前未读取过）
    aht20->sensorQueried.temperature = 0;
    aht20->sensorQueried.humidity = 0;

    return HAL_OK;
}

/**
  * @brief  检查AHT20是否连接到I2C总线
  * @param  aht20: AHT20句柄结构体指针
  * @retval 1表示已连接，0表示未连接
  */
uint8_t AHT20_IsConnected(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return 0;
    }

    // 尝试与设备通信
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(aht20->hi2c, aht20->deviceAddress, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        return 1;
    }

    // 如果IC未响应，给20ms额外时间用于上电启动
    // 数据手册第7页
    HAL_Delay(AHT20_RESET_DELAY_MS);

    status = HAL_I2C_IsDeviceReady(aht20->hi2c, aht20->deviceAddress, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        return 1;
    }

    return 0;
}

/**
  * @brief  检查是否有新数据可用
  * @param  aht20: AHT20句柄结构体指针
  * @retval 1表示有新数据，0表示未就绪
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
  * @brief  从AHT20获取状态字节
  * @param  aht20: AHT20句柄结构体指针
  * @retval 状态字节
  */
uint8_t AHT20_GetStatus(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return 0;
    }

    uint8_t status = 0;
    // 使用读地址：写地址0x70，读地址0x71
    // AHT20使用0x70写，0x71读
    HAL_StatusTypeDef result = HAL_I2C_Master_Receive(aht20->hi2c, aht20->deviceAddress + 1, &status, 1, HAL_MAX_DELAY);
    if (result != HAL_OK) {
        return 0;
    }
    return status;
}

/**
  * @brief  检查AHT20是否已校准
  * @param  aht20: AHT20句柄结构体指针
  * @retval 1表示已校准，0表示未校准
  */
uint8_t AHT20_IsCalibrated(AHT20_HandleTypeDef *aht20)
{
    return (AHT20_GetStatus(aht20) & (1 << 3));
}

/**
  * @brief  检查AHT20是否忙碌
  * @param  aht20: AHT20句柄结构体指针
  * @retval 1表示忙碌，0表示就绪
  */
uint8_t AHT20_IsBusy(AHT20_HandleTypeDef *aht20)
{
    return (AHT20_GetStatus(aht20) & (1 << 7));
}

/**
  * @brief  初始化AHT20传感器
  * @param  aht20: AHT20句柄结构体指针
  * @retval HAL状态
  */
HAL_StatusTypeDef AHT20_Initialize(AHT20_HandleTypeDef *aht20)
{
    return AHT20_WriteCommandWithParams(aht20, AHT20_REG_INITIALIZE, 0x80, 0x00);
}

/**
  * @brief  触发测量
  * @param  aht20: AHT20句柄结构体指针
  * @retval HAL状态
  */
HAL_StatusTypeDef AHT20_TriggerMeasurement(AHT20_HandleTypeDef *aht20)
{
    return AHT20_WriteCommandWithParams(aht20, AHT20_REG_MEASURE, 0x33, 0x00);
}

/**
  * @brief  读取并解析6字节数据为原始湿度和温度数据
  * @param  aht20: AHT20句柄结构体指针
  * @retval 无
  */
void AHT20_ReadData(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return;
    }

    // 清除之前的数据
    aht20->sensorData.temperature = 0;
    aht20->sensorData.humidity = 0;

    uint8_t buffer[6] = {0};
    // 使用读地址（0x71）接收数据
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

    // 去除大于20位的数据
    aht20->sensorData.temperature = aht20->sensorData.temperature & ~(0xFFF00000);

    // 标记数据为新鲜
    aht20->sensorQueried.temperature = 0;
    aht20->sensorQueried.humidity = 0;
}

/**
  * @brief  执行AHT20传感器软复位
  * @param  aht20: AHT20句柄结构体指针
  * @retval HAL状态
  */
HAL_StatusTypeDef AHT20_SoftReset(AHT20_HandleTypeDef *aht20)
{
    return AHT20_WriteCommand(aht20, AHT20_REG_RESET);
}

/**
  * @brief  获取温度测量值
  * @param  aht20: AHT20句柄结构体指针
  * @retval 摄氏温度值
  */
float AHT20_GetTemperature(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL) {
        return -999.0f; // 错误值
    }

    if (aht20->sensorQueried.temperature == 1) {
        // 如果数据已过期，触发新测量
        AHT20_TriggerMeasurement(aht20);

        HAL_Delay(AHT20_MEAS_DELAY_MS); // 等待测量完成

        uint8_t counter = 0;
        while (AHT20_IsBusy(aht20)) {
            HAL_Delay(AHT20_BUSY_RETRY_MS);
            if (counter++ > (AHT20_BUSY_TIMEOUT_MS / AHT20_BUSY_RETRY_MS)) {
                return -999.0f; // 100ms后放弃
            }
        }

        AHT20_ReadData(aht20);
    }

    // 根据数据手册第8页的公式
    float tempCelsius = ((float)aht20->sensorData.temperature / 1048576) * 200 - 50;

    // 标记数据为过期
    aht20->sensorQueried.temperature = 1;

    return tempCelsius;
}

/**
  * @brief  获取湿度测量值
  * @param  aht20: AHT20句柄结构体指针
  * @retval 相对湿度值 %RH
  */
float AHT20_GetHumidity(AHT20_HandleTypeDef *aht20)
{
    if (aht20 == NULL) {
        return -999.0f; // 错误值
    }

    if (aht20->sensorQueried.humidity == 1) {
        // 如果数据已过期，触发新测量
        AHT20_TriggerMeasurement(aht20);

        HAL_Delay(AHT20_MEAS_DELAY_MS); // 等待测量完成

        uint8_t counter = 0;
        while (AHT20_IsBusy(aht20)) {
            HAL_Delay(AHT20_BUSY_RETRY_MS);
            if (counter++ > (AHT20_BUSY_TIMEOUT_MS / AHT20_BUSY_RETRY_MS)) {
                return -999.0f; // 100ms后放弃
            }
        }

        AHT20_ReadData(aht20);
    }

    // 根据数据手册第8页的公式
    float relHumidity = ((float)aht20->sensorData.humidity / 1048576) * 100;

    // 标记数据为过期
    aht20->sensorQueried.humidity = 1;

    return relHumidity;
}

/* 私有函数 ---------------------------------------------------------*/

/**
  * @brief  向AHT20写入单个命令
  * @param  aht20: AHT20句柄结构体指针
  * @param  cmd: 命令字节
  * @retval HAL状态
  */
static HAL_StatusTypeDef AHT20_WriteCommand(AHT20_HandleTypeDef *aht20, uint8_t cmd)
{
    if (aht20 == NULL || aht20->hi2c == NULL) {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Transmit(aht20->hi2c, aht20->deviceAddress, &cmd, 1, HAL_MAX_DELAY);
}

/**
  * @brief  向AHT20写入带参数的命令
  * @param  aht20: AHT20句柄结构体指针
  * @param  cmd: 命令字节
  * @param  param1: 第一个参数字节
  * @param  param2: 第二个参数字节
  * @retval HAL状态
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