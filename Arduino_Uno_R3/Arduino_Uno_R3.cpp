#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// 定义传感器引脚
#define LIGHT_SENSOR_PIN A3       // 光敏传感器
#define SOIL_SENSOR_PIN A1        // 土壤湿度传感器
#define DHT_PIN 3                 // DHT11 温湿度传感器

#define DHT_TYPE DHT11            // DHT11 类型

// 初始化 DHT 传感器
DHT dht(DHT_PIN, DHT_TYPE);

// 初始化 BMP180 传感器
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);  // 10085 是传感器 ID

// 定义最大重试次数和延迟时间（用于传感器读取失败时重试）
const int MAX_RETRIES = 3;
const unsigned long RETRY_DELAY_MS = 100;

void setup() {
  Serial.begin(9600);  // 初始化串口通信
  dht.begin();         // 初始化 DHT11

  // 初始化 I2C 总线
  Wire.begin();

  // 初始化 BMP180 传感器
  if (!bmp.begin()) {
    Serial.println("错误：BMP180 初始化失败！");
  } else {
    Serial.println("BMP180 初始化成功！");
  }
}

void loop() {
  // 检查是否有来自串口的指令
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // 读取指令
    command.trim(); // 去掉多余的空格或换行符
    command.toUpperCase(); // 将指令转换为大写，避免大小写敏感问题

    // 清空串口缓冲区
    while (Serial.available() > 0) {
      Serial.read();
    }

    // 根据指令读取传感器数据并返回
    if (command == "LIGHT") {
      int lightValue = readLightSensor();
      Serial.println(lightValue); // 返回光敏传感器数据
    } else if (command == "SOIL") {
      int soilMoistureValue = readSoilSensor();
      Serial.println(soilMoistureValue); // 返回土壤湿度数据
    } else if (command == "TEMPERATURE") {
      float temperature = readTemperature();
      if (!isnan(temperature)) {
        Serial.println(temperature); // 返回温度数据
      } else {
        Serial.println("错误：无法读取温度！"); // 读取失败返回错误信息
      }
    } else if (command == "HUMIDITY") {
      float humidity = readHumidity();
      if (!isnan(humidity)) {
        Serial.println(humidity); // 返回湿度数据
      } else {
        Serial.println("错误：无法读取湿度！"); // 读取失败返回错误信息
      }
    } else if (command == "PRESSURE") {
      float pressure = readPressure();
      if (!isnan(pressure)) {
        Serial.println(pressure); // 返回气压数据
      } else {
        Serial.println("错误：无法读取气压！"); // 读取失败返回错误信息
      }
    } else if (command == "ALTITUDE") {
      float altitude = readAltitude();
      if (!isnan(altitude)) {
        Serial.println(altitude); // 返回海拔数据
      } else {
        Serial.println("错误：无法读取海拔！"); // 读取失败返回错误信息
      }
    } else if (command == "BMP_TEMP") {
      float bmpTemp = readBMPTemperature();
      if (!isnan(bmpTemp)) {
        Serial.println(bmpTemp); // 返回 BMP180 温度数据
      } else {
        Serial.println("错误：无法读取 BMP180 温度！"); // 读取失败返回错误信息
      }
    } else {
      Serial.println("错误：无效的命令！"); // 无效指令返回错误信息
    }
  }
}

// 读取光敏传感器数据
int readLightSensor() {
  return analogRead(LIGHT_SENSOR_PIN);
}

// 读取土壤湿度传感器数据
int readSoilSensor() {
  return analogRead(SOIL_SENSOR_PIN);
}

// 读取温度数据（带重试机制）
float readTemperature() {
  float temperature = NAN;
  for (int i = 0; i < MAX_RETRIES; i++) {
    temperature = dht.readTemperature();
    if (!isnan(temperature)) {
      break;
    }
    delay(RETRY_DELAY_MS); // 延迟后重试
  }
  return temperature;
}

// 读取湿度数据（带重试机制）
float readHumidity() {
  float humidity = NAN;
  for (int i = 0; i < MAX_RETRIES; i++) {
    humidity = dht.readHumidity();
    if (!isnan(humidity)) {
      break;
    }
    delay(RETRY_DELAY_MS); // 延迟后重试
  }
  return humidity;
}

// 读取 BMP180 气压数据
float readPressure() {
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    return event.pressure / 100.0F; // 转换为 hPa
  }
  return NAN;
}

// 读取 BMP180 海拔数据
float readAltitude() {
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    return bmp.pressureToAltitude(1013.25, event.pressure); // 基于标准海平面气压计算海拔
  }
  return NAN;
}

// 读取 BMP180 温度数据
float readBMPTemperature() {
  float temperature;
  bmp.getTemperature(&temperature);
  return temperature;
}
