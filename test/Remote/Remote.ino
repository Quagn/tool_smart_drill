#include <espnow.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);

uint8_t pin_trigger_direction = 5;
uint8_t pin_trigger_power     = A0;
uint8_t pin_led               = 2;

uint8_t remote_Addr[] = {0x94, 0xB5, 0x55, 0x2B, 0x5C, 0x64};

typedef struct data_remote {
  uint8_t power;
  bool direction_;
} data_remote;

data_remote remote;

uint8_t power = 0;
bool direction_ = true;

uint32_t 
loop_timer_send = 0,
loop_timer_read = 0,
loop_timer      = 0,
idle_timer      = 0;

uint32_t send_interval = 25000;   // 40Hz = 25000 microseconds
uint32_t read_interval = 5000;    // 200Hz = 5000 microseconds

uint8_t power_readings[5];  // Array to store the last 5 readings
uint8_t power_filtered = 0;

uint32_t blink_interval = 500000;  // LED blink interval in microseconds (e.g., 500ms)
uint32_t last_blink_time = 0;
bool ledState = LOW;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  loop_timer_send = micros();  // Initialize timers
  loop_timer_read = micros();
  loop_timer = micros();
  ota();

  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remote_Addr, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  pinMode(pin_led, OUTPUT); // Set the LED pin as output
  
  setup_trigger();
}

void loop() {
  uint32_t now = micros();

  // Read and filter power every 5000 microseconds (200Hz)
  if (now - loop_timer_read >= read_interval) {
    loop_timer_read = now;  // Update the read timer
    read_trigger();         // Assuming read_trigger() is implemented elsewhere
    filter_power();         // Assuming filter_power() is implemented elsewhere
  }

  // Send data every 25000 microseconds (40Hz)
  if (now - loop_timer_send >= send_interval) {
    loop_timer_send = now;  // Update the send timer
    remote.power = power_filtered;
    remote.direction_ = !direction_;
    esp_now_send(remote_Addr, (uint8_t*)&remote, sizeof(data_remote));
  }

  // Check if the system is idle (power == 0) and blink LED
  if (power == 0) {
    if (now - idle_timer > 90e6) {  // 90e6 microseconds = 90 seconds
      // Blink the LED every 'blink_interval'
      if (now - last_blink_time >= blink_interval) {
        ledState = !ledState;  // Toggle LED state
        if (ledState) {
          GPOS = (1 << pin_led);  // Set the LED pin low (LED OFF)
        } else {
          GPOC = (1 << pin_led);  // Set the LED pin high (LED ON)
        }
        last_blink_time = now;  // Reset the blink timer
      }
    } else {
      GPOC = (1 << pin_led);   // Ensure the LED is on if not idle long enough
      ledState = LOW;
    } 
  } else {
    idle_timer = now;  // Reset the idle timer when power is non-zero
    GPOC = (1 << pin_led);  // Turn on the LED if power is non-zero
    ledState = LOW;  // Reset LED state
  }
}
