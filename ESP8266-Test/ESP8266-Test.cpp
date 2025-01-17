const int sigPin = D5;  // 将SIG接口连接到ESP8266的D5引脚

// 去抖动函数
bool isPressed() {
  if (digitalRead(sigPin) == HIGH) {  // 检测到高电平
    delay(50);  // 延迟50毫秒去抖动
    if (digitalRead(sigPin) == HIGH) {  // 再次检测
      return true;  // 确认按压
    }
  }
  return false;  // 未按压
}

void setup() {
  Serial.begin(115200);  // 初始化串口通信
  pinMode(sigPin, INPUT);  // 设置SIG引脚为输入模式
  Serial.println("程序已启动，等待传感器信号...");
}

void loop() {
  if (isPressed()) {
    Serial.println("状态: 按压中");
  } else {
    Serial.println("状态: 未按压");
  }
  delay(100);  // 延迟100毫秒
}
