#include <AsyncTCP.h>                                             // OTA lib
#include <ESPAsyncWebSrv.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);

#include "soc/soc.h"                                              // Sys lib
#include "soc/rtc_cntl_reg.h"

uint32_t loop_timer;                                              // Timer
int val = 0;                                                      // Giá trị analog

// HTML Web Page with AJAX to display the value
const char wpage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Value Display</title>
  <script>
    setInterval(function() {
      getValue();
    }, 500);                                  // update 500ms

    function getValue() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("val").innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "/getValue", true);
      xhr.send();
    }
  </script>
</head>
<body>
  <h1>ESP32 Analog Value</h1>
  <p>Value: <span id="val">0</span></p>
</body>
</html>
)rawliteral";

void setup(void) {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);                      // Turn off low input voltage
  
  WiFi.mode(WIFI_STA);                                            // Wifi connect
  WiFi.begin("MinhPhone", "12341234");

  // Route to load web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", wpage);                   // Serve the web page
  });

  // API route to get the value of val
  server.on("/getValue", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(val));                // Trả về giá trị `val`
  });

  AsyncElegantOTA.begin(&server);                                 // Start ElegantOTA
  server.begin();

  pinMode(39, INPUT);                                             // Trigger

  pinMode(32, OUTPUT);                                            // Esc
  pinMode(33, OUTPUT);                                            // Esc
  ledcSetup(32, 1000, 8);
}

void loop() {
  val = map(analogRead(34), 0, 4095, 0, 255);
  digitalWrite(32, 0);
  delay(60);
  digitalWrite(32, 1);
  delay(1000);
  /*
  while (micros() - loop_timer < 4000);                           // 250hz windows
  loop_timer = micros();
  */
}
