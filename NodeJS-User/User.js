const mqtt = require('mqtt');

// EMQX 公共服务器配置
const brokerUrl = 'mqtt://broker.emqx.io'; // EMQX 公共 Broker 地址
const options = {
  clientId: 'WXX_vm', // 客户端 ID
  username: 'duang', // 用户名
  password: 'duangkey', // 密码
};

// 订阅的主题
const topics = [
  'home/sensor/light',       // 光敏传感器
  'home/sensor/soil',        // 土壤湿度传感器
  'home/sensor/temperature', // DHT11 温度传感器
  'home/sensor/humidity',    // DHT11 湿度传感器
  'home/sensor/pressure',    // BMP180 气压传感器
  'home/sensor/altitude',    // BMP180 海拔传感器
  'home/sensor/bmp_temp',    // BMP180 温度传感器
];

// 连接到 MQTT Broker
const client = mqtt.connect(brokerUrl, options);

client.on('connect', () => {
  console.log('已连接到 MQTT Broker');

  // 订阅所有主题
  topics.forEach(topic => {
    client.subscribe(topic, (err) => {
      if (!err) {
        console.log(`已订阅主题: ${topic}`);
      } else {
        console.error(`订阅主题失败: ${topic}`, err);
      }
    });
  });
});

// 接收消息
client.on('message', (topic, message) => {
  const data = JSON.parse(message.toString()); // 解析 JSON 数据
  console.log(`收到主题 ${topic} 的消息:`);
  console.log(`值: ${data.value}`);
  console.log(`单位: ${data.unit}`);
  console.log('--------------------------');
});

// 处理错误
client.on('error', (err) => {
  console.error('MQTT 错误:', err);
});

// 断开连接
client.on('close', () => {
  console.log('已断开与 MQTT Broker 的连接');
});