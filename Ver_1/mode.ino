void read_Button() 
{
  int button_value = analogRead(pin_button);

  if (button_value <= 100) 
  {
    button = 2;
    if (micros() - bthold_timer >= 4e5) bthold = 2;
    else  bthold = 0;
  } 
  
  else if (button_value >= 1800 && button_value <= 2100) 
  {
    button = 1;
    if (micros() - bthold_timer >= 4e5) bthold = 1; 
    else  bthold = 0;
  } 
  
  else                    // Reset
  {
    button = 0;
    bthold = 0;
    bthold_timer = micros();
  }
}

void mode_menu() 
{
  if (button == 1 && bthold != 1)
  {
    if (--mode_run <= 1)  mode_run = 3;     // Mode run cycling
  }
  
  if (button == 2 && bthold != 2)
  {
    if (--drill_mode < 1) drill_mode = 2;   // Drill Mode cycling
  }

  if (bthold == 1)
  {
    if (prev_led_dri_on)
    {
      led_dri_on = !led_dri_on;               // Turn ON/OFF LED DRILL
      prev_led_dri_on = false;
    }
  } else  prev_led_dri_on = true;

  if (bthold == 2)
  {
    if (prev_led_alw_on)
    {
      led_alw_on = !led_alw_on;               // Turn ON/OFF LED ALWAYS
      prev_led_alw_on = false;
    }
  } else  prev_led_alw_on = true;
  
  if (protection > 0) 
  {
    mode_run = 5;                           // Protection mode
  } 
  
  else if (remote_on > 0)
  {
    mode_run = 4;                           // Remote mode
  }
  
  else
  {
    if (mode_run == 1 || mode_run == 4 || mode_run == 5)
    {
      mode_run = 2;                         // Reset mode
    }
    
    if (power > 0)                          // Drill mode
    {
      if (!tear_on)                         // Tear for angle holder
      {
        Tear_Pitch = AnglePitch;
        Tear_Yaw   = AngleYaw;
        tear_on = true;
      }
      mode_run = 1;
      if (led_dri_on) led_on = 2;
    }
    else
    {
      tear_on = false;
      led_on = 0;
    }
  }
}
