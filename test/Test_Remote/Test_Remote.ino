#include <WiFi.h>

#include <esp_now.h>

#include <AsyncTCP.h>    
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebSrv.h>

typedef struct data_remote {
  uint8_t power;
  bool direction_;
} data_remote;

data_remote remote;
AsyncWebServer server(80);

uint8_t 
power = 0;
bool direction_, 
sel = false;

uint32_t 
loop_timer,
window_timer = 100000;

uint8_t                                         // Pin inout
pin_trigger_direction = 39,                       // Trigger pin      
pin_trigger_power     = 34;

void setup() 
{
  Serial.begin(115200);
  loop_timer = micros();                        // Start timer
  WiFi.mode(WIFI_STA);                          // Set to Station mode
  ota();

  setup_trigger();
}

void loop() 
{
  //AsyncElegantOTA.loop();                       // Handle OTA
  read_trigger();
  Serial.print("power: ");
  Serial.println(power);
  Serial.print("direction: ");
  Serial.println(direction_);
  delay(1000);
}
