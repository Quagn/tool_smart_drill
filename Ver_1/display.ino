void setup_lcd() 
{
  tft.init();                             // Setup display
  tft.fillScreen(TFT_BLACK);              // Clear screen
}

void run_lcd() 
{
  if (mode_run != prevMode) 
  {                                                                     // Reset screen between mode
    tft.fillScreen(TFT_BLACK);            // Clear screen only when mode changes
    prevMode = mode_run;
    tft.setTextSize(2);
  }
  
  if (batt != prevBatt) 
  {                                                                     // Batt display
    // Clear the old battery frame
    tft.fillRect(100, 220, 70, 20, TFT_BLACK); // Adjust the size of the cleared frame
    
    tft.drawRect(100, 220, 40, 20, TFT_WHITE); // Battery frame, 40 pixels long
    tft.fillRect(100 + 2, 220 + 2, (batt * 40) / 100, 16, TFT_GREEN); // Draw battery level
    prevBatt = batt;
  }

  if (perv_drill_mode != drill_mode)
  {                                                                     // Drill mode display
    tft.fillRect(10, 200, 220, 20, TFT_BLACK); // Clear old drill mode display
    tft.setCursor(90, 200);
    if (drill_mode == 2)
    {
      tft.print("PULSE");
    } else  tft.print("NORMAL");
  
    perv_drill_mode = drill_mode;
  }

  if(prevLedAlw != led_alw_on)                                          // Led alw mode display
  {
    tft.fillRect(10, 200, 220, 20, TFT_BLACK); // Clear old drill mode display
    tft.setCursor(60, 200);
    if (led_alw_on)
    {
      tft.print("LED ALW ON");
    } else  tft.print("LED ALW OFF");

    prevLedAlw = led_alw_on;
  }

  if(prevLedDri != led_dri_on)                                          // Led dri mode display
  {
    tft.fillRect(10, 200, 220, 20, TFT_BLACK); // Clear old drill mode display
    tft.setCursor(50, 200);
    if (led_dri_on)
    {
      tft.print("LED DRILL ON");
    } else  tft.print("LED DRILL OFF");

    prevLedDri = led_dri_on;
  }
  
//////////////////////////////////////////// Drill mode
  if (mode_run == 1) 
  {                                                                     
    float pitch = AnglePitch - Tear_Pitch;
    float yaw   = AngleYaw   - Tear_Yaw;
    
    int16_t x = (  yaw  * 4 + SCREEN_CENTER);
    int16_t y = (-pitch * 4 + SCREEN_CENTER);
    int16_t x_t = x - SCREEN_CENTER;
    int16_t y_t = y - SCREEN_CENTER;
    
    tft.setCursor(100, 20);
    tft.print("P: ");                               // Display pitch 
    tft.setCursor(100, 40);
    tft.print("Y: ");                               // Display yaw 

    if (x != prev_x || y != prev_y)                 // Only update if position changes
    {                 
      if (prev_x != -1 && prev_y != -1)                 
      {             
        tft.fillCircle(prev_x, prev_y, 18, TFT_BLACK);  // Clear old circle
      }

      tft.fillRect(120, 20, 80, 20, TFT_BLACK);     // Clear old pitch display
      tft.setCursor(120, 20);
      tft.print(pitch);                                 // Display new pitch
      tft.fillRect(120, 40, 80, 20, TFT_BLACK);     // Clear old yaw display
      tft.setCursor(120, 40);
      tft.print(yaw);                                   // Display new yaw

      tft.fillCircle(SCREEN_CENTER, SCREEN_CENTER, 5, TFT_GREEN);   // Draw new inner circle
      tft.drawCircle(x, y, 15, TFT_WHITE);                          // Draw new outer circles
      tft.drawCircle(x, y, 16, TFT_WHITE);
      tft.drawCircle(x, y, 17, TFT_WHITE);

      prev_x = x;                                   // Update previous coordinates
      prev_y = y;
    }

    if (((x_t > 20) || (x_t < -20)) || ((y_t > 20) || (y_t < -20)))
    {
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 64, TFT_RED);                        
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 65, TFT_RED);                        
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 66, TFT_RED);                        
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 67, TFT_RED);                        
    }
    else
    {
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 64, TFT_BLACK);                        
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 65, TFT_BLACK);                        
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 66, TFT_BLACK);                        
      tft.drawCircle(SCREEN_CENTER, SCREEN_CENTER, 67, TFT_BLACK);
    }                         
  } 
  
//////////////////////////////////////////// Idle mode
  else if (mode_run == 2)
  {                                                                     
    if (((AngleRoll < 1) && (AngleRoll > -1)) && ((AnglePitch < 1) && (AnglePitch > -1)))
    {
      tft.fillRect(60, 20, 120, 20, TFT_GREEN);
    }
    else
    {
      tft.fillRect(60, 20, 120, 20, TFT_BLACK);
      tft.setCursor(90, 20);
      tft.print(-AnglePitch);
    }
      
    float radRoll = AngleRoll * PI / 180.0;          // Convert Roll angle to radians
    int16_t horizonOffset = (int16_t)(AnglePitch * 2); // Shift horizon based on Pitch
    
    int16_t x1 = -SCREEN_CENTER, y1 = horizonOffset;  // Start point of horizon before rotation
    int16_t x2 =  SCREEN_CENTER, y2 = horizonOffset;  // End point of horizon before rotation
    
    int16_t x1r = SCREEN_CENTER + (x1 * cos(radRoll) - y1 * sin(radRoll)); // Apply Roll rotation
    int16_t y1r = SCREEN_CENTER + (x1 * sin(radRoll) + y1 * cos(radRoll));
    int16_t x2r = SCREEN_CENTER + (x2 * cos(radRoll) - y2 * sin(radRoll));
    int16_t y2r = SCREEN_CENTER + (x2 * sin(radRoll) + y2 * cos(radRoll));
    
    if (prev_x1r != -1 && prev_y1r != -1 && prev_x2r != -1 && prev_y2r != -1) { // Clear old horizon
      for (int8_t i = -3; i <= 3; i++) {
        tft.drawLine(prev_x1r, prev_y1r + i, prev_x2r, prev_y2r + i, TFT_BLACK);
      }
    }

    for (int8_t i = -1; i <= 1; i++) {               // Draw thicker horizon
      tft.drawLine(x1r, y1r + i, x2r, y2r + i, TFT_WHITE);
    }

    prev_x1r = x1r;                                  // Save new horizon positions
    prev_y1r = y1r;
    prev_x2r = x2r;
    prev_y2r = y2r;

    for (int8_t i = -60, j = 0; i <= 60; i += 30, j++) 
    {                                               // Draw pitch lines rotated by Roll
      int pitchLineY = i;

      int16_t pitchX1 = -60, pitchY1 = pitchLineY;
      int16_t pitchX2 =  60, pitchY2 = pitchLineY;

      int16_t pitchX1r = SCREEN_CENTER + (pitchX1 * cos(radRoll) - pitchY1 * sin(radRoll));
      int16_t pitchY1r = SCREEN_CENTER + (pitchX1 * sin(radRoll) + pitchY1 * cos(radRoll));
      int16_t pitchX2r = SCREEN_CENTER + (pitchX2 * cos(radRoll) - pitchY2 * sin(radRoll));
      int16_t pitchY2r = SCREEN_CENTER + (pitchX2 * sin(radRoll) + pitchY2 * cos(radRoll));

      if (prev_pitchX1r[j] != -1 && prev_pitchY1r[j] != -1 && prev_pitchX2r[j] != -1 && prev_pitchY2r[j] != -1) 
      {
        for (int k = -2; k <= 2; k++) 
        {
          tft.drawLine(prev_pitchX1r[j], prev_pitchY1r[j] + k, prev_pitchX2r[j], prev_pitchY2r[j] + k, TFT_BLACK);
        }
        tft.fillRect(prev_pitchX2r[j] + 5, prev_pitchY2r[j] - 20, 50, 30, TFT_BLACK); // Clear old right number
        tft.fillRect(prev_pitchX1r[j] - 40, prev_pitchY1r[j] - 20, 50, 30, TFT_BLACK); // Clear old left number
      }

      for (int8_t k = -1; k <= 1; k++) 
      {
        tft.drawLine(pitchX1r, pitchY1r + k, pitchX2r, pitchY2r + k, TFT_WHITE);     // Draw thicker pitch lines
      }

      prev_pitchX1r[j] = pitchX1r;              // Save pitch line positions
      prev_pitchY1r[j] = pitchY1r;
      prev_pitchX2r[j] = pitchX2r;
      prev_pitchY2r[j] = pitchY2r;

      if (i != 0) 
      {                                         // Skip 0
        char pitchLabel[5];                           // Calculate pitch number positions after rotation
        sprintf(pitchLabel, "%d", ((-i)/2));
        tft.setTextColor(TFT_WHITE);

        int16_t pitchLabelXRight = pitchX2r + 5;      // Right number position
        int16_t pitchLabelYRight = pitchY2r - 10;
        int16_t pitchLabelXLeft = pitchX1r - 30;      // Left number position
        int16_t pitchLabelYLeft = pitchY1r - 10;

        tft.drawString(pitchLabel, pitchLabelXRight, pitchLabelYRight);  // Draw right number
        tft.drawString(pitchLabel, pitchLabelXLeft, pitchLabelYLeft);    // Draw left number
      }
    }
  } 
  
//////////////////////////////////////////// Measure mode
  else if (mode_run == 3)
  {                                                                     
    tft.setCursor(80, 20);
    tft.print("Heading");  
    tft.setCursor(80, 80);
    tft.print("Altitude");  
    tft.setCursor(60, 140);
    tft.print("Temperature");  

    if (Heading != prevHeading) {                    // Update only if Heading changes
      tft.fillRect(90, 50, 100, 20, TFT_BLACK);      // Clear old Heading
      tft.setCursor(90, 50);
      tft.print(Heading);
      prevHeading = Heading;
    }

    if (Altitude != prevAltitude) {                  // Update only if Altitude changes
      tft.fillRect(90, 110, 100, 20, TFT_BLACK);     // Clear old Altitude
      tft.setCursor(90, 110);
      tft.print(Altitude);
      prevAltitude = Altitude;
    }

    if (Temp_ext != prevTemp) {                      // Update only if Temperature changes
      tft.fillRect(90, 170, 100, 20, TFT_BLACK);     // Clear old Temperature
      tft.setCursor(90, 170);
      tft.print(Temp_ext);
      prevTemp = Temp_ext;
    }
  }
  
//////////////////////////////////////////// Remote mode
  else if (mode_run == 4)
  {                                                                     
    tft.setCursor(45, 80);
    tft.setTextSize(3);                     // Increase text size
    tft.print("REMOTE ON");
    if (prevRemote != power)
    {
      tft.fillRect(110, 120, 100, 40, TFT_BLACK);
      tft.setCursor(110, 120);
      tft.print(power * 10);
      prevRemote = power;
    }
  }
  
//////////////////////////////////////////// Protection mode
  else if (mode_run == 5)
  {                                                                     
    if (protection & (1 << 0)) {
      tft.setCursor(80, 50);
      tft.print("TIME OUT");
    }

    if (protection & (1 << 1)) {
      tft.setCursor(80, 80);
      tft.print("KICK BACK");
    }

    if (protection & (1 << 2)) {
      tft.setCursor(80, 110);
      tft.print("OVER TEMP");
    }

    if (protection & (1 << 3)) {
      tft.setCursor(80, 140);
      tft.print("LOW BATT");
    }
  }
}
