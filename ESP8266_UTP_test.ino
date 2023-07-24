#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "DFRobot_VEML7700.h"
#define PLAY_TIME 10
IPAddress staticIP(192, 168, 50, 125);  // esp8266开发板的固定IP地址
IPAddress gateway(192, 168, 50, 1);  // 网关地址，路由器的IP地址
IPAddress subnet(255, 255, 255, 0);  // 子网掩码
IPAddress dns(192, 168, 50, 1);  // dns服务器默认
IPAddress ComputerIP(192, 168, 50, 58);  // UE4工程所在的电脑IP

DFRobot_VEML7700 als; // initialize a light sensor
WiFiUDP Udp;

int transmit_count = 0;
int play_time = 0; //特效播放时间

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);
  als.begin();
  WiFi.mode(WIFI_STA);  // 以station模式连接WiFi
  WiFi.config(staticIP, gateway, subnet, dns);  // 配置静态IP地址
  WiFi.begin("The Office", "qwer1234!@#$"); // 连接WiFi
  while (WiFi.status() != WL_CONNECTED) { // 如果没有连接到WiFi就持续输出...
    Serial.print('.');
    delay(250); // 延迟100坤ms
  }
  Udp.begin(8888);  // UDP开始监听8888端口
}

void loop() {
  float lux;
  als.getALSLux(lux);
  Serial.print("Lux: ");//Read Lux and print
  Serial.print(lux);
  Serial.println(" lx");
  delay(500);
  // put your main code here, to run repeatedly:
  //Serial.println(WiFi.localIP());
  int packetSize = Udp.parsePacket();
  if (packetSize){ // 如果收到数据
    char packetBuffer[255];
    int n = Udp.read(packetBuffer, 255);  // 将信息读取到packetBuffer里
    packetBuffer[n] = 0;  // 清除缓存
    Serial.println("Received: " + (String)packetBuffer);  // 打印收到的数据
    // if((String)packetBuffer=="ESP8266Lighton"){Myfunction;} // 检查反馈给ESP8266的数据, 根据收到数据执行myfunction
  }
  if (lux > 50 && play_time < PLAY_TIME) { // 拔剑
    transmit_count = 0;
    Serial.println("Signal transmitted to the PC"); 
    Udp.beginPacket(ComputerIP, 3002);  // 向UE4工程所在的电脑的 3002端口 发数据
    Udp.write("ikusukaribata");
    Udp.endPacket();  // 发送数据结束
    play_time = play_time + 1;
    delay(1000);
  } else {
    Serial.println("Signal transmitted to the PC");
    Udp.beginPacket(ComputerIP, 3002);  // 向UE4工程所在的电脑的 3002端口 发数据
    Udp.write("finished");
    Udp.endPacket();  // 发送数据结束
    transmit_count = 0; //收剑
  }

  if (lux < 20 && transmit_count <= 5) { //收剑 防止快速抽插
    transmit_count = transmit_count + 1;
    play_time = 0;
    delay(1000);
    if (transmit_count >= 5){
      Serial.println("Signal transmitted to the PC");
      Udp.beginPacket(ComputerIP, 3002);  // 向UE4工程所在的电脑的 3002端口 发数据
      Udp.write("close_ikusukaribata");
      Udp.endPacket();  // 发送数据结束
      transmit_count = 10;
    }
  }
  

}
