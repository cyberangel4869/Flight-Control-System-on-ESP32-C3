#ifndef AHT20_H
#define AHT20_H

#include <Arduino.h>
#include <Wire.h>

class AHT20 {
public:
    // 构造函数：传入已配置好的 I2C 句柄
    // 注意：不再在驱动内部调用 Wire.begin()
    AHT20(TwoWire &wirePort, uint8_t i2cAddr = 0x38);

    // 初始化传感器，返回 true 表示成功
    // 要求调用前总线已由主程序完成 begin()
    bool begin();

    // 触发一次温湿度测量（非阻塞）
    void startMeasurement();

    // 读取温湿度数据，返回 true 表示成功
    bool readData();

    // 获取上次读取的温度（摄氏度）
    float getTemperature_C();

    // 获取上次读取的温度（华氏度）
    float getTemperature_F();

    // 获取上次读取的湿度（%RH）
    float getHumidity_RH();

private:
    TwoWire *_wire;      // 保存 I2C 句柄
    uint8_t _addr;

    float _temperature;
    float _humidity;

    bool writeCommand(uint8_t cmd, uint8_t param1, uint8_t param2);
    bool readStatus(uint8_t &status);
    bool waitForReady(uint32_t timeoutMs = 200);
};

#endif