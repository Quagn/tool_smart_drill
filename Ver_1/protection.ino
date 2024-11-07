void time_out_() 
{
  if (power == 0) 
  {
    if (micros() - idle_timer > 600e6)                    // Timer out 10 min              
    {
      protection += 1;
    }
  }
  else {
    idle_timer = micros();
  }
}
void anti_kick_back() 
{
  if (gyrox > 16000 || gyroy > 16000 || gyroz > 16000)    // Over acce
  {
    protection += 2;
  }
}

void over_temp() 
{
  if((Temp_int > 60) || (Temp_ext > 45))                  // Over temp
  {
    protection += 4;
  }
}

void low_batt() 
{
  if(batt < 5)                                            // Low batt
  {
    protection += 8;
  }
}

void protection_() 
{
  time_out_();
  anti_kick_back();
  over_temp();
  low_batt();
 
  if (protection > 0) 
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      led_on    = 20;
      buzzer_on = 50;                                    // Alarm
      vTaskDelay(400 / portTICK_PERIOD_MS);
      led_on    = 0;
      buzzer_on = 0; 
      vTaskDelay(80 / portTICK_PERIOD_MS);
    }
    protection = 0;
  }
}
