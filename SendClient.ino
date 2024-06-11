// 数据采集节点，放置在打卡机附近
// arduino esp32库蓝牙扫描函数存在结果不完整的问题，详情见：https://blog.csdn.net/verynicebest/article/details/103965703
// 经过测试，2.0.17版本库函数稳定性较好，60%的情况下可以正确扫描到Manufacturer Data数据

#include <WiFi.h>
#include <HTTPClient.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

const char* ssid = "WiFi SSID"; // 修改为你的WiFi SSID
const char* password = "WiFi password";  // 修改为你的WiFi password
int scanTime = 3; //In seconds
const char* targetMAC = "xx:xx:xx:xx:xx:xx"; // 修改为你的打卡机蓝牙mac地址（小写）
String url = "https://example.com/send.php"; // 修改为你的服务器域名
String pwd = "your password";


void processAdvertisement(const String advertisement);
void uploadData(const String data);

String data = "";
String lastData = "0x";
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  BLEScan* scan;// 用于停止扫描的 BLEScan 对象
  public : MyAdvertisedDeviceCallbacks(BLEScan* scan) : scan(scan) {}
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    if (advertisedDevice.getAddress().toString() == targetMAC) {
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      processAdvertisement(advertisedDevice.toString().c_str());
      scan->stop();
    }
  }
};

void setup()
{
  Serial.begin(115200);
  while(!Serial){delay(10);}

  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  BLEDevice::init("BLE_Scanner");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(pBLEScan));
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);  // less or equal setInterval value
}

void loop(){
  Serial.println("start Scan!");
  pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  if (data != "") {
    if (data != lastData) {
      lastData = data;
      uploadData(data);
      Serial.println("Refresh!!");
    }
  }
  delay(3000);
}

void processAdvertisement(const String advertisement) {
  // 检查是否包含制造商数据
  if (advertisement.indexOf("manufacturer data") != -1) {
    // 提取制造商数据的部分
    int start = advertisement.indexOf("manufacturer data: ") + 19;
    int end = advertisement.indexOf(", serviceUUID:");
    String manufacturerData = advertisement.substring(start, end);

    // 将制造商数据转换为字符串
    String hexString = manufacturerData;
    hexString.replace("0x", ""); // 移除前缀 "0x"

    // 打印转换后的字符串数据
    Serial.println("Manufacturer Data: " + hexString);
    data = hexString;
  }
}

void uploadData(const String data) {
  HTTPClient http;
  http.begin(url + "?password=" + pwd + "&data=" + data);
  
  int http_code = http.GET();
  Serial.printf("HTTP Code: %d; ", http_code);
  String response = http.getString();
  Serial.println(response);
  http.end();
}
