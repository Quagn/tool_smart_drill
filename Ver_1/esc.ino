void setup_esc() 
{
  // Set pin modes for ESC direction and power
  pinMode(pin_esc_direction, OUTPUT);
  pinMode(pin_esc_power    , OUTPUT);
  
  GPIO.out1_w1ts.val = (1U << (pin_esc_power - 32));        // Set ESC power pin to off (set HIGH) at startup
}

void run_esc() 
{
  if (direction_ != prev_direction_){                       // Set ESC direction
    if (direction_) GPIO.out1_w1ts.val = (1U << (pin_esc_direction - 32));      // Set F
    else GPIO.out1_w1tc.val = (1U << (pin_esc_direction - 32));                 // Set R
    prev_direction_= direction_;
  }
  
  // Send ESC pulse (set power LOW)
  GPIO.out1_w1ts.val = (1U << (pin_esc_power - 32)); 

  if (drill_mode == 2 && (micros() - esc_timer >= 5e5))    // Pulse mode
  { 
    vTaskDelay(pulse_time / portTICK_PERIOD_MS);
    esc_timer = micros();
  }

  if (protection > 0) return;                               // If protection is active, skip further actions

  switch (power)                                            // Normal mode
  {
    case 0:                                                     // Power = 0 (stop)
      vTaskDelay(10 / portTICK_PERIOD_MS);
      primer_on = false;
      break;

    case 1:                                                     // Low power
      vTaskDelay(7 / portTICK_PERIOD_MS);
      GPIO.out1_w1tc.val = (1U << (pin_esc_power - 32));            // Set LOW
      vTaskDelay(3 / portTICK_PERIOD_MS);
      break;

    case 10:                                                    // High power
      GPIO.out1_w1tc.val = (1U << (pin_esc_power - 32));            // Set LOW
      vTaskDelay(10 / portTICK_PERIOD_MS);
      break;

    default:
      // If power < 4 and primer_on is not triggered, apply trigger
      if (power < 4 && !primer_on) 
      {
        GPIO.out1_w1tc.val = (1U << (pin_esc_power - 32));          // Set LOW
        vTaskDelay(35 / portTICK_PERIOD_MS);
        GPIO.out1_w1ts.val = (1U << (pin_esc_power - 32));          // Set HIGH
        primer_on = true;
      } else 
      {
        vTaskDelay((10 - power) / portTICK_PERIOD_MS);
        GPIO.out1_w1tc.val = (1U << (pin_esc_power - 32));          // Set LOW
        vTaskDelay(power / portTICK_PERIOD_MS);
      }
      break;
  }
}
