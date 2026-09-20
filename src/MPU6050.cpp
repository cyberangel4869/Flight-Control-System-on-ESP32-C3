// MPU6050.cpp (修改后的完整文件)
#include "MPU6050.h"
MPU6050::MPU6050(TwoWire &WirePort,uint8_t addr){
    _wire=&WirePort;
    _address=addr;
}

bool MPU6050::begin(){
    if(MPU6050::GetID()!=0x68){
        return false;
    }
    MPU6050::SetDLPF(DLPF_BW_44_Fs_1k);
    MPU6050::SetACCLrange(ACCL_RANGE_2G);
    MPU6050::SetGYROrange(GYRO_RANGE_250);
    return true;
}

uint8_t MPU6050::GetID() {
    // 读取芯片ID
    // return: ID号 
    _wire->beginTransmission(_address);
    _wire->write(REG_WHO_AM_I);
    _wire->endTransmission(0);
    _wire->requestFrom(_address, (uint8_t)1);
    return (_wire->read());
}

void MPU6050::SetDLPF(uint8_t DLPF) {
    // 向 CONFIG 寄存器 (0x1A) 写入 DLPF 配置值
    // Args:
    //  DLPF: 数字低通滤波器带宽配置
    //       只能传入 DLPF_XXX
    _wire->beginTransmission(_address);
    _wire->write(REG_CONFIG);
    _wire->write(DLPF & 0x07);  // 确保只写低3位，防止意外写入高位
    _wire->endTransmission();
}

void MPU6050::SetGYROrange(uint8_t GYRO_RANGE) {
    // 配置角速度传感器量程
    // Args:
    //  GYRO_RANGE: 参数应该使用 GYRO_RANGE_xxx
    _wire->beginTransmission(_address);
    _wire->write(REG_GYRO_CONFIG);
    _wire->write(GYRO_RANGE & 0x18);  // 只保留 bit4:3 (FS_SEL)，其他位清零
    _wire->endTransmission();
}

void MPU6050::SetACCLrange(uint8_t ACCL_RANGE) {
    // 配置加速度传感器量程
    // Args:
    //  ACCL_RANGE: 参数应该使用 ACCL_RANGE_xxG
    _wire->beginTransmission(_address);
    _wire->write(REG_ACCEL_CONFIG);
    _wire->write(ACCL_RANGE & 0x18);  // 只保留 bit4:3 (AFS_SEL)，其他位清零
    _wire->endTransmission();
}

void MPU6050::SetINTmode(uint8_t INT_SET) {
    // INT中断输出引脚配置
    // Args:
    //  INT_SET: INT_xxx 相或
    _wire->beginTransmission(_address);
    _wire->write(REG_INT_CFG);
    _wire->write(INT_SET & 0xf0);  // 低位保持为0
    _wire->endTransmission();
}

void MPU6050::SetINTsourse(uint8_t INT_SOURCE) {
    // 设置中断触发源
    // Args:
    //  INT_SOURCE: xx_INT_EN相或
    _wire->beginTransmission(_address);
    _wire->write(REG_INT_ENABLE);
    _wire->write(INT_SOURCE & 0x11);
    _wire->endTransmission();
}

// 数据读取函数实现
MPU6050datas MPU6050::ReadRawDatas() {
    MPU6050datas data = {0};  // 初始化为 0
    
    // 第1步：从 0x3B 开始读取 14 字节
    _wire->beginTransmission(_address);
    _wire->write(0x3B);          // 起始寄存器地址 (加速度计 X 轴高字节)
    if (_wire->endTransmission(false) != 0) {  // false = 不发送停止位，保持总线控制
        return data;             // I2C 通信失败，返回空数据
    }
    _wire->requestFrom(_address, (uint8_t)14);
    if (_wire->available() != 14) {
        return data;             // 数据长度不符，返回空数据
    }
    uint8_t buffer[14];
    for (int i = 0; i < 14; i++) {
        buffer[i] = _wire->read();
    }
    data.ACCLx = (int16_t)((buffer[0] << 8) | buffer[1]);
    data.ACCLy = (int16_t)((buffer[2] << 8) | buffer[3]);
    data.ACCLz = (int16_t)((buffer[4] << 8) | buffer[5]);
    data.TEMP  = (int16_t)((buffer[6] << 8) | buffer[7]);
    data.GYROx = (int16_t)((buffer[8] << 8) | buffer[9]);
    data.GYROy = (int16_t)((buffer[10] << 8) | buffer[11]);
    data.GYROz = (int16_t)((buffer[12] << 8) | buffer[13]);
    
    return data;
}