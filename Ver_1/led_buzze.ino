void setup_led_buzzer() 
{
  // Set pin modes for buzzer and LED
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_led   , OUTPUT);
}

void run_buzzer(uint8_t bz_time) 
{
  if (bz_time != 0)         // Toggle buzzer for the specified time
  {
    for (uint8_t i = 0; i < bz_time; i++)
    {
      GPIO.out_w1ts = (1U << pin_buzzer);    // Set buzzer HIGH
      vTaskDelay(2 / portTICK_PERIOD_MS);
      GPIO.out_w1tc = (1U << pin_buzzer);    // Set buzzer LOW
      vTaskDelay(2 / portTICK_PERIOD_MS);
    }
  } else vTaskDelay(1 / portTICK_PERIOD_MS);
}

void run_led(uint8_t led_time) 
{
  if (led_alw_on) led_time = 33;
  
  if (led_time != 0)        // Toggle LED for the specified time
  {
    for (uint8_t i = 0; i < led_time; i++) {  
      GPIO.out_w1ts = (1U << pin_led);       // Set PWM LED HIGH
      vTaskDelay(3 / portTICK_PERIOD_MS);
      GPIO.out_w1tc = (1U << pin_led);       // Set PWM LED LOW
      vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    led_time = 0;
  } else vTaskDelay(1 / portTICK_PERIOD_MS);
}
