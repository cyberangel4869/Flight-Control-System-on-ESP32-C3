#include <Wire.h>
#include "AHT20.h"
#include "BMP280.h"
#include "MPU6050.h"

// 若使用 ESP32，可指定引脚；普通 AVR 上 Wire.begin() 使用默认引脚
#define I2C_SDA 6
#define I2C_SCL 7

AHT20 aht20(Wire, 0x38);   // 把 Wire 句柄传给 AHT20
BMP280 bmp280(Wire,0x76);
MPU6050 mpu6050(Wire,0x68);

uint8_t ID;
void setup() {
    Serial.begin(115200);
    while (!Serial);

    // 主程序负责配置 I2C 总线（这里只调用一次）
    Wire.begin(I2C_SDA, I2C_SCL,100000);

    if (!aht20.begin()) {
        Serial.println("AHT20 初始化失败,请检查接线");
    }
    if(!bmp280.begin()){
        Serial.println("BMP280 初始化失败,请检查连线");
    }
    if(!mpu6050.begin()){
        Serial.println("MPU6050 初始化失败,请检查连线");
    }
}

void loop() {
    if (aht20.readData()) {
        Serial.print("温度: ");
        Serial.print(aht20.getTemperature_C());
        Serial.print(" C / ");
        Serial.print(aht20.getTemperature_F());
        Serial.print(" F，湿度: ");
        Serial.print(aht20.getHumidity_RH());
        Serial.println(" %RH");
    } else {
        Serial.println("AHT20 读取失败");
    }
    if (bmp280.readData()) {
        Serial.print("温度: ");
        Serial.print(bmp280.getTemperature_C());
        Serial.print(" C，气压: ");
        Serial.print(bmp280.getPressure_hPa());
        Serial.println(" hPa");
    } else {
        Serial.println("BMP280 读取失败");
    }
    ID=mpu6050.GetID();
    Serial.println("MPU6050 ID:");
    Serial.print(ID,HEX);
    delay(2000);
}
