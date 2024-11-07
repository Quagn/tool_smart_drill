#include <AsyncTCP.h>                                             // OTA lib
#include <ESPAsyncWebSrv.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);

#include "soc/soc.h"                                              // Sys lib
#include "soc/rtc_cntl_reg.h"

uint32_t loop_timer;                                              // Timer

void setup(void) {
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);                      // Turn off low input voltage
  
  WiFi.mode(WIFI_STA);                                            // Wifi connect
  WiFi.begin("MinhPhone", "12341234");

  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });
  */
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
}

void loop(void) {
  
  while (micros() - loop_timer < 4000);           // 250hz windows
  loop_timer = micros();
}
