#include "BMP280.h"

// 寄存器地址（数据手册 Section 5.3）
#define BMP280_REG_ID         0xD0
#define BMP280_REG_RESET      0xE0
#define BMP280_REG_STATUS     0xF3
#define BMP280_REG_CTRL_MEAS  0xF4
#define BMP280_REG_CONFIG     0xF5
#define BMP280_REG_PRESS_MSB  0xF7  // 0xF7~0xF9: 气压 20 位
#define BMP280_REG_TEMP_MSB   0xFA  // 0xFA~0xFC: 温度 20 位
#define BMP280_REG_CALIB_00   0x88  // 校准参数起始地址

// 命令
#define BMP280_RESET_CMD      0xB6
#define BMP280_CHIP_ID        0x58

BMP280::BMP280(TwoWire &wirePort, uint8_t i2cAddr)
    : _wire(&wirePort), _addr(i2cAddr), _t_fine(0),
      _temperature(0), _pressure(0) {}

bool BMP280::begin() {
    // 检查芯片 ID
    if (getChipID() != BMP280_CHIP_ID) {
        return false;
    }

    // 软复位（可选，确保已知状态）
    writeRegister(BMP280_REG_RESET, BMP280_RESET_CMD);
    delay(10);

    // 读取校准参数
    if (!readCalibration()) {
        return false;
    }

    // 配置：温度过采样 x2，气压过采样 x16，正常模式
    // ctrl_meas = (osrs_t << 5) | (osrs_p << 2) | mode
    // osrs_t=2 (x2), osrs_p=5 (x16), mode=3 (normal)
    uint8_t ctrl = (2 << 5) | (5 << 2) | 3;
    writeRegister(BMP280_REG_CTRL_MEAS, ctrl);

    // config：待机时间 500ms，IIR 滤波关闭
    // t_sb=4 (500ms), filter=0, spi3w_en=0
    uint8_t config = (4 << 5) | (0 << 2) | 0;
    writeRegister(BMP280_REG_CONFIG, config);

    delay(100);  // 等待首次测量完成
    return true;
}

bool BMP280::readData() {
    uint8_t rawData[6];

    // 连续读取 6 字节：气压 (0xF7~0xF9) + 温度 (0xFA~0xFC)
    if (!readRegisters(BMP280_REG_PRESS_MSB, rawData, 6)) {
        return false;
    }

    // 解析 20 位气压原始值
    int32_t adc_P = ((int32_t)rawData[0] << 12) |
                    ((int32_t)rawData[1] << 4) |
                    ((int32_t)rawData[2] >> 4);

    // 解析 20 位温度原始值
    int32_t adc_T = ((int32_t)rawData[3] << 12) |
                    ((int32_t)rawData[4] << 4) |
                    ((int32_t)rawData[5] >> 4);

    // 补偿计算（先温度后气压，气压补偿依赖 t_fine）
    _temperature = compensateTemperature(adc_T);
    _pressure = compensatePressure(adc_P);

    return true;
}

float BMP280::getTemperature_C() {
    return _temperature;
}

float BMP280::getPressure_hPa() {
    return _pressure;
}

float BMP280::getAltitude_m(float seaLevel_hPa) {
    // 国际标准大气公式：H = 44330 * (1 - (P/P0)^(1/5.255))
    // P0 单位为 hPa
    if (_pressure <= 0) return 0;
    return 44330.0f * (1.0f - pow(_pressure / seaLevel_hPa, 1.0f / 5.255f));
}

uint8_t BMP280::getChipID() {
    return readRegister(BMP280_REG_ID);
}

// ---------- 底层 I2C 操作 ----------

bool BMP280::writeRegister(uint8_t reg, uint8_t value) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->write(value);
    return (_wire->endTransmission() == 0);
}

uint8_t BMP280::readRegister(uint8_t reg) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->endTransmission();

    _wire->requestFrom(_addr, (uint8_t)1);
    if (_wire->available() < 1) return 0;
    return _wire->read();
}

bool BMP280::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (_wire->endTransmission() != 0) return false;

    _wire->requestFrom(_addr, length);
    if (_wire->available() < length) return false;

    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = _wire->read();
    }
    return true;
}

// ---------- 校准参数读取 ----------

bool BMP280::readCalibration() {
    uint8_t calib[24];
    if (!readRegisters(BMP280_REG_CALIB_00, calib, 24)) {
        return false;
    }

    // 数据手册 Table 17：校准参数为小端序
    // dig_T1 无符号，dig_T2/T3 有符号，dig_P1 无符号，dig_P2~P9 有符号
    dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);
    dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);

    return true;
}

// ---------- 补偿计算（数据手册 Section 3.11.3） ----------

float BMP280::compensateTemperature(int32_t adc_T) {
    int32_t var1, var2;

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) *
            ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
              ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;

    _t_fine = var1 + var2;

    // 温度输出精度：0.01°C（返回值即百分之一度）
    int32_t T = (_t_fine * 5 + 128) >> 8;
    return T / 100.0f;
}

float BMP280::compensatePressure(int32_t adc_P) {
    int64_t var1, var2, p;

    var1 = ((int64_t)_t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) +
           ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0;  // 避免除以零
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    // 气压输出精度：0.01 Pa，转换为 hPa 需除以 25600
    return (float)p / 25600.0f;
}