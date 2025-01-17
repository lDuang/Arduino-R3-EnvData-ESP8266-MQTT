#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi 配置
const char* ssid = "ssid"; // WiFi名称
const char* password = "password";       // WiFi密码

// EMQX 公共服务器配置
const char* mqtt_server = "broker.emqx.io"; // EMQX 公共 Broker 地址
const int mqtt_port = 1883; // MQTT 端口
const char* mqtt_client_id = "Client_id"; // 客户端 ID
const char* mqtt_username = "user_id"; // 用户名
const char* mqtt_password = "user_password"; // 密码

// MQTT 主题
const char* topic_light = "home/sensor/light";
const char* topic_soil = "home/sensor/soil";
const char* topic_temperature = "home/sensor/temperature";
const char* topic_humidity = "home/sensor/humidity";
const char* topic_pressure = "home/sensor/pressure";
const char* topic_altitude = "home/sensor/altitude";
const char* topic_bmp_temp = "home/sensor/bmp_temp";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool lastMQTTConnectionState = false; // 记录上一次的连接状态

void setup() {
  Serial.begin(9600); // 初始化串口通信（与 R3 通信）

  // 连接到WiFi
  connectWiFi();

  // 配置 MQTT Broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback); // 设置回调函数（可选）

  // 连接到 MQTT Broker
  connectMQTT();
}

void loop() {
  maintainMQTTConnection();   // 保持 MQTT 连接
  handlePeriodicTasks();      // 定期发布传感器数据
}

void maintainMQTTConnection() {
  bool currentMQTTConnectionState = mqttClient.connected();

  // 只有当连接状态发生变化时才打印
  if (currentMQTTConnectionState != lastMQTTConnectionState) {
    Serial.println(String("MQTT Broker 连接状态: ") + (currentMQTTConnectionState ? "已连接" : "未连接"));
    lastMQTTConnectionState = currentMQTTConnectionState;
  }

  // 如果未连接，则尝试重新连接
  if (!currentMQTTConnectionState) {
    connectMQTT();
  }

  // 保持 MQTT 客户端的正常运行
  mqttClient.loop();
}

void handlePeriodicTasks() {
  // 定期发布传感器数据，每 5 秒一次
  static unsigned long lastPublishTime = 0;
  const unsigned long publishInterval = 5000;

  if (millis() - lastPublishTime >= publishInterval) {
    publishSensorData();
    lastPublishTime = millis();
  }
}

// 连接到 WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("正在连接 WiFi ...");
  }
  Serial.println("成功连接 WiFi ...");
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP()); // 打印ESP8266的IP地址
}

// 连接到 MQTT Broker
void connectMQTT() {
  Serial.println("正在连接 MQTT Broker...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("成功连接 MQTT Broker");
      break; // 连接成功后退出循环
    } else {
      Serial.print("连接失败, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" 5秒后重试...");
      delay(5000);
    }
  }
}

// 发布传感器数据
void publishSensorData() {
  const char* commands[] = {"LIGHT", "SOIL", "TEMPERATURE", "HUMIDITY", "PRESSURE", "ALTITUDE", "BMP_TEMP"};
  const char* unit[] = {"lux", "%", "°C", "%", "hPa", "m", "°C"};
  const char* topics[] = {topic_light, topic_soil, topic_temperature, topic_humidity, topic_pressure, topic_altitude, topic_bmp_temp};

  for (int i = 0; i < 7; i++) {
    sendCommand(commands[i]);
    String value = readSerialResponse();

    if (strcmp(commands[i], "SOIL") == 0) {
      value = convertToPercentage(value.toInt());
    }

    String json = createJson(value, unit[i]);
    mqttClient.publish(topics[i], json.c_str());
  }
}

// 土壤湿度转换
String convertToPercentage(int rawValue) {
  int dryValue = 990;
  int wetValue = 140;
  int percentage = (1 - (float)(rawValue - wetValue) / (dryValue - wetValue)) * 100;
  percentage = constrain(percentage, 0, 100);
  return String(percentage);
}

// 创建json
String createJson(String value, const char* unit) {
  String json = "{\n";
  json += "\"value\": \"" + value + "\",\n";
  json += "\"unit\": \"" + String(unit) + "\"\n";
  json += "}";
  return json;
}

// 发送指令并清空串口缓冲区
void sendCommand(const char* command) {
  while (Serial.available() > 0) {
    Serial.read(); // 清空串口缓冲区
  }
  Serial.println(command); // 发送指令
  delay(100); // 增加100ms延迟，确保Arduino有足够时间处理
}

// 读取Arduino返回的当前行数据
String readSerialResponse() {
  String response = Serial.readStringUntil('\n'); // 读取直到换行符
  response.trim(); // 去掉多余的空格或换行符
  return response;
}

// MQTT 回调函数（可选）
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
