#include "AHT20.h"

#define AHT20_CMD_INIT        0xBE
#define AHT20_CMD_TRIGGER     0xAC  //测量开始触发指令
#define AHT20_CMD_SOFT_RESET  0xBA

#define AHT20_STATUS_BUSY     0x80  //AHT20状态字节芯片忙指示位
#define AHT20_STATUS_CALIB    0x08  //AHT20状态字节数据校准指示位

AHT20::AHT20(TwoWire &wirePort, uint8_t i2cAddr)
    // 注意：需要主函数进行I2C总线初始化，然后将句柄传入本函数
    : _wire(&wirePort), _addr(i2cAddr), _temperature(0), _humidity(0) {}

bool AHT20::begin() {
    // 注意：需要主函数进行I2C总线初始化，然后将句柄传入本函数

    // 软复位
    _wire->beginTransmission(_addr);
    _wire->write(AHT20_CMD_SOFT_RESET);
    _wire->endTransmission();
    delay(20);

    // 初始化命令 0xBE，参数 0x08 0x00
    _wire->beginTransmission(_addr);
    _wire->write(AHT20_CMD_INIT);
    _wire->write(0x08);
    _wire->write(0x00);
    if (_wire->endTransmission() != 0) {
        return false;
    }
    delay(10);

    uint8_t status;
    if (!readStatus(status)) {
        return false;
    }

    if (!(status & AHT20_STATUS_CALIB)) {//数据校准位无效，需要再次复位
        _wire->beginTransmission(_addr);
        _wire->write(AHT20_CMD_INIT);
        _wire->write(0x08);
        _wire->write(0x00);
        _wire->endTransmission();
        delay(10);
    }

    return true;
}

void AHT20::startMeasurement() {
    _wire->beginTransmission(_addr);
    _wire->write(AHT20_CMD_TRIGGER);
    _wire->write(0x33);
    _wire->write(0x00);
    _wire->endTransmission();
}

bool AHT20::readData() {
    startMeasurement();

    if (!waitForReady(200)) {
        return false;
    }

    _wire->requestFrom(_addr, (uint8_t)6);
    if (_wire->available() < 6) {
        return false;
    }

    uint8_t rawData[6];
    for (int i = 0; i < 6; i++) {
        rawData[i] = _wire->read();
    }

    uint32_t rawHumidity = ((uint32_t)rawData[1] << 12) |
                           ((uint32_t)rawData[2] << 4) |
                           ((uint32_t)rawData[3] >> 4);

    uint32_t rawTemperature = ((uint32_t)(rawData[3] & 0x0F) << 16) |
                              ((uint32_t)rawData[4] << 8) |
                              (uint32_t)rawData[5];

    _humidity = (rawHumidity * 100.0f) / 1048576.0f;
    _temperature = (rawTemperature * 200.0f) / 1048576.0f - 50.0f;

    return true;
}

float AHT20::getTemperature_C() { return _temperature; }
float AHT20::getTemperature_F() { return _temperature * 9.0f / 5.0f + 32.0f; }
float AHT20::getHumidity_RH()   { return _humidity; }

bool AHT20::writeCommand(uint8_t cmd, uint8_t param1, uint8_t param2) {
    _wire->beginTransmission(_addr);
    _wire->write(cmd);
    _wire->write(param1);
    _wire->write(param2);
    return (_wire->endTransmission() == 0);
}

bool AHT20::readStatus(uint8_t &status) {
    _wire->requestFrom(_addr, (uint8_t)1);
    if (_wire->available() < 1) {
        return false;
    }
    status = _wire->read();
    return true;
}

bool AHT20::waitForReady(uint32_t timeoutMs) {
    uint8_t status;
    uint32_t start = millis();

    while (millis() - start < timeoutMs) {
        if (!readStatus(status)) {
            return false;
        }
        if (!(status & AHT20_STATUS_BUSY)) {
            return true;
        }
        delay(10);
    }
    return false;
}