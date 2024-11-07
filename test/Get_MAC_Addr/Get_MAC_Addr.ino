///*
#include <WiFi.h>
#include <esp_wifi.h>

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup(){
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin();

  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
}
//*/
/*
 #include <ESP8266WiFi.h>

void readMacAddress() {
  String macAddress = WiFi.macAddress();  // Đọc địa chỉ MAC của ESP8266
  Serial.println(macAddress);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);  // Thiết lập chế độ WiFi là Station
  WiFi.begin();

  Serial.print("[DEFAULT] ESP8266 Board MAC Address: ");
  readMacAddress();  // Đọc và hiển thị địa chỉ MAC
}
*/
void loop(){
  readMacAddress();
  delay(1000);
} 
//*/

//uint8_t broadcastAddress[] = {0x94, 0xB5, 0x55, 0x2B, 0x5C, 0x64}; esp32 drill
//uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0x6B, 0x6A, 0x4C}; esp8266
//uint8_t broadcastAddress[] = {0x78, 0x21, 0x84, 0x79, 0x6f, 0xc4}; esp32 
