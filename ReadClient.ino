// 数据模拟节点，使用时放在手机附近
// 数据模拟流程参考自：https://github.com/zanjie1999/dingBle
// 经过测试，2.0.6可以正常执行模拟操作，2.0.17无法运行，其他版本未做测试

#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>

static uint8_t legacy_adv_data[] = {
        0x02, 0x01, 0x06,
        0x03, 0x03, 0x3c, 0xfe,
        0x0C,0x09,0x52,0x54,0x4B,0x5F,0x42,0x54,0x5F,0x34,0x2E,0x31,0x00, 0X0
};

static uint8_t legacy_scan_rsp_data[] = {
        0x17,0xFF,0x00,0x01,0xB5,0x00,0x02,0x25,0xEC,0xD7,0x44,0x00,0x00,0x01,0xAA,0x91,0x77,0x67,0xAF,0x01,0x10,0x00,0x00,0x00, 0X0
};

const char* ssid = "123"; // 手机热点名称
const char* password = "66668888";  // 手机热点密码

// 目标MAC地址
uint8_t bleMac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; // 修改为你的打卡机蓝牙mac地址

String url = "https://example.com/send.php"; // 修改为你的服务器域名
String pwd = "your password";

String data = "00";

void getData(void);

void setup()
{
  Serial.begin(115200);
  while(!Serial){delay(10);}

  // esp32没有提供设置蓝牙mac地址的api 通过查看esp32的源代码
  // 此操作将根据蓝牙mac算出base mac
  if (UNIVERSAL_MAC_ADDR_NUM == FOUR_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 2;
  } else if (UNIVERSAL_MAC_ADDR_NUM == TWO_UNIVERSAL_MAC_ADDR) {
    bleMac[5] -= 1;
  }
  esp_base_mac_addr_set(bleMac);

  Serial.println();
  Serial.println("******************************************************");

  BLEDevice::init("ESP32");
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  BLEAdvertisementData oScanResponseData;
  pAdvertising->setScanResponseData(oScanResponseData);

  // 里面有个 m_customScanResponseData = true; 和 m_customScanResponseData = true; 所以只能先随便设置一下
  pAdvertising->setAdvertisementData(oScanResponseData);

  esp_err_t errRc = ::esp_ble_gap_config_adv_data_raw(legacy_adv_data, sizeof(legacy_adv_data));
  if (errRc != ESP_OK) {
    Serial.printf("esp_ble_gap_config_adv_data_raw: %d\n", errRc);
  }
  // 超过31
  errRc = ::esp_ble_gap_config_scan_rsp_data_raw(legacy_scan_rsp_data, sizeof(legacy_scan_rsp_data));
  if (errRc != ESP_OK) {
    Serial.printf("esp_ble_gap_config_scan_rsp_data_raw: %d\n", errRc);
  }

  pAdvertising->start();

  delay(100);

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
}

void loop() {
  getData();
  uint8_t bleRaw32[24];
  bleRaw32[0] = 0x17;
  bleRaw32[1] = 0xff;
  for (size_t i = 0; i < 22; i++) {
    String hexValue = data.substring(i * 2, (i * 2) + 2);
    bleRaw32[i + 2] = strtoul(hexValue.c_str(), nullptr, 16);
  }
  esp_err_t errRc = ::esp_ble_gap_config_scan_rsp_data_raw(bleRaw32, sizeof(bleRaw32));
  if (errRc != ESP_OK) {
    Serial.printf("esp_ble_gap_config_scan_rsp_data_raw: %d\n", errRc);
  }
  delay(1000);
}

void getData() {
  HTTPClient http;
  http.begin(url + "?password=" + pwd);
  
  int http_code = http.GET();
  if (http_code == 200) {
    String response = http.getString();
    if (response.length() == 49) {
      data = response.substring(5); // 去除 "data:" 前缀
    }
  }
  http.end();
}
