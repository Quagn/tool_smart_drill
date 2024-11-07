#include <Arduino.h>

uint8_t pin_trigger_direction = 39,    // Trigger pin
        pin_trigger_power     = 34,
        pin_esc_direction     = 33,    // ESC pin
        pin_esc_power         = 32;

uint16_t power;
bool direction_, trigger;

void EscControl(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    direction_ = (GPIO.in1.val & (1 << (pin_trigger_direction - 32))) != 0;  // Read trigger state
    power = map(analogRead(pin_trigger_power), 0, 4095, 0, 10);              // Map analog value to power level

    (direction_ ? GPIO.out1_w1ts.val : GPIO.out1_w1tc.val) = (1 << (pin_esc_direction - 32));  // Set ESC direction

    if (power == 0) {
      GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));  // Power off ESC
      trigger = false;
    } else if (power < 3 && !trigger) {
      GPIO.out1_w1tc.val = (1 << (pin_esc_power - 32));  // Short ESC pulse
      vTaskDelay(30 / portTICK_PERIOD_MS);
      GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));
      trigger = true;
    } else if (power == 1) {
      GPIO.out1_w1tc.val = (1 << (pin_esc_power - 32));  // Low power pulse
      vTaskDelay(3 / portTICK_PERIOD_MS);
      GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));
      vTaskDelay(7 / portTICK_PERIOD_MS);
    } else {
      GPIO.out1_w1tc.val = (1 << (pin_esc_power - 32));  // Default pulse
      if (power < 10) {
        vTaskDelay(power / portTICK_PERIOD_MS);
        GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));
        vTaskDelay((10 - power) / portTICK_PERIOD_MS);
      }
    }
  }
}

void setup() {
  pinMode(pin_trigger_direction, INPUT);  // Initialize pins
  pinMode(pin_esc_direction, OUTPUT);
  pinMode(pin_esc_power, OUTPUT);

  xTaskCreatePinnedToCore(EscControl, "EscControl", 2048, NULL, 1, NULL, tskNO_AFFINITY);  // Create esc control task
}

void loop() { /* Empty: tasks handle the logic */ }
