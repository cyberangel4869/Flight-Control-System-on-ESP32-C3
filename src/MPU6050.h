#include <Wire.h>
//MPU6050驱动函数

#define REG_WHO_AM_I 0x75
//低通滤波器带宽和采样率设置
#define REG_SMPRT_DIV 0x19 //采样率分频，一般设0
#define REG_CONFIG 0x1a //低三位设置数字DLPF_CFG配置低通滤波器
#define DLPF_BW_260_Fs_8k 0x00
#define DLPF_BW_184_Fs_1k 0x01
#define DLPF_BW_94_Fs_1k 0x02
#define DLPF_BW_44_Fs_1k 0x03
#define DLPF_BW_21_Fs_1k 0x04
#define DLPF_BW_10_Fs_1k 0x05
#define DLPF_BW_5_Fs_1k 0x06

//角速度量程设置
#define REG_GYRO_CONFIG 0x1b //配置角速度传感器量程，高位留作角速度传感器自检
#define GYRO_RANGE_250 0x00
#define GYRO_RANGE_500 0x08
#define GYRO_RANGE_1000 0x10
#define GYRO_RANGE_2000 0x18

//加速度量程设置
#define REG_ACCEL_CONFIG 0x1c //配置加速度传感器量程，高位留作加速度传感器自检
#define ACCL_RANGE_2G 0x00
#define ACCL_RANGE_4G 0x08
#define ACCL_RANGE_8G 0x10
#define ACCL_RANGE_16G 0x18

//中断引脚控制
#define REG_INT_CFG 0x37
//中断电平
#define INT_LEVEL_HIGH 0x00
#define INT_LEVEL_LOW 0x80
//中断输出模式
#define INT_PUSH_PULL 0x00
#define INT_OPEN_DRAIN 0x40
//中断输出锁存
#define INT_LATCH 0x20
#define INT_PULSE 0x00
//中断清除方式
#define INT_CLR_RD_ANY 0x10
#define INT_CLR_RD_STATE 0x00

//中断触发源设置
#define REG_INT_ENABLE 0x38
#define FIFO_OVFLOW_INT_EN 0x10
#define DATA_RDY_INT_EN 0x01

struct MPU6050datas
{
    int16_t ACCLx;
    int16_t ACCLy;
    int16_t ACCLz;
    int16_t TEMP;
    int16_t GYROx;
    int16_t GYROy;
    int16_t GYROz;
};


class MPU6050{
    public:
        MPU6050(TwoWire &Wire , uint8_t addr=0x68);
        bool begin();
        void SetDLPF(uint8_t DLPF);
        void SetGYROrange(uint8_t GYRO_RANGE);
        void SetACCLrange(uint8_t ACCL_RANGE);
        void SetINTmode(uint8_t INT_SET);
        void SetINTsourse(uint8_t INT_SOURCE);
        uint8_t GetID();
        MPU6050datas ReadRawDatas();
    private:
        uint8_t _address;
        TwoWire *_wire;
};


