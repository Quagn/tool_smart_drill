void setup_trigger() {
  pinMode(pin_trigger_direction, INPUT);  // Set trigger direction input
}

void read_trigger() {
  direction_ = GPIO_INPUT_GET(GPIO_ID_PIN(pin_trigger_direction));  // Read trigger direction
  power = 10 - round(analogRead(pin_trigger_power) / 102.3);        // Read Esc power
}

void filter_power() {
  // Shift older readings and add the latest one
  for (int i = 4; i > 0; i--) {
    power_readings[i] = power_readings[i - 1];
  }
  power_readings[0] = power;

  // Calculate the filtered value (average of the last 5 readings)
  uint32_t sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += power_readings[i];
  }
  power_filtered = sum / 5;
}
