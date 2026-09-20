#include <WiFi.h>

// 设置热点名称和密码
const char* ssid = "ESP32-C3-Test";
const char* password = "12345678"; // 密码至少需要8位

void setup() {
  Serial.begin(115200);
  delay(5000); // 等待USB串口连接

  Serial.println("\n--- ESP32-C3 WiFi 热点测试 ---");

  // 启动 AP 模式
  // 参数：SSID，密码。如果不需要密码，第二个参数传 NULL 或空字符串
  WiFi.softAP(ssid, password);

  // 等待 AP 启动
  delay(100);

  // 获取并打印热点信息
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("热点名称 (SSID): ");
  Serial.println(ssid);
  Serial.print("热点 IP 地址: ");
  Serial.println(myIP);

  Serial.println("热点已启动，等待设备连接...");
}

void loop() {
  // 每 3 秒打印一次当前连接数
  delay(3000);
  Serial.print("当前连接设备数: ");
  Serial.println(WiFi.softAPgetStationNum());
}
