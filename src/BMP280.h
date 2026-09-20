#ifndef BMP280_H
#define BMP280_H

#include <Arduino.h>
#include <Wire.h>

class BMP280 {
public:
    // 构造函数：传入已配置好的 I2C 句柄
    // 地址：0x76（SDO 接地）或 0x77（SDO 接 VDD）
    BMP280(TwoWire &wirePort, uint8_t i2cAddr = 0x76);

    // 初始化传感器，读取校准参数并配置测量模式
    // 返回 true 表示成功
    bool begin();

    // 读取温度和气压数据，返回 true 表示成功
    bool readData();

    // 获取上次读取的温度（摄氏度）
    float getTemperature_C();

    // 获取上次读取的气压（百帕 hPa）
    float getPressure_hPa();

    // 根据当前气压计算海拔高度（米）
    // seaLevel_hPa：当地海平面标准大气压，默认 1013.25 hPa
    float getAltitude_m(float seaLevel_hPa = 1013.25f);

    // 读取芯片 ID（BMP280 应返回 0x58）
    uint8_t getChipID();

private:
    TwoWire *_wire;
    uint8_t _addr;

    // 校准参数（数据手册 Section 3.11.2）
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    // 补偿计算中间值
    int32_t _t_fine;

    // 缓存的物理量
    float _temperature;
    float _pressure;

    // 底层 I2C 读写
    bool writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length);

    // 读取校准参数
    bool readCalibration();

    // 数据补偿计算
    float compensateTemperature(int32_t adc_T);
    float compensatePressure(int32_t adc_P);
};

#endif