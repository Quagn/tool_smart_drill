void ota() 
{
  read_Trigger();
  while (power >= 9)
  {
    if (micros() - loop_timer >= 5e6)                               // If after boot 5s hold power run ota
    {
      WiFi.begin(ssid, password);                            // Wifi connect
    
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)     // Run OTA web
      { 
        request->send(200, "text/plain", "SmartDrill OTA.");
      });
    
      AsyncElegantOTA.begin(&server);                                 // Run OTA server
      server.begin();

      tft.fillScreen(TFT_BLACK); 
      tft.setTextSize(3);               
      tft.setCursor(100, 50);         
      tft.print("OTA");
      tft.setCursor(10,80);         
      tft.print("WF:");
      tft.setCursor(70, 80);         
      tft.print(ssid);
      delay(3000);
      tft.setCursor(30, 110);    
      tft.setTextSize(2);     
      tft.print(WiFi.localIP());

      run_buzzer(100);                        // Alarm for OTA
    
      while (true) delay(1);                                          // stay in OTA "delay(1) to bypass watchdog"
    }
    read_Trigger();
    delay(1);                                                       // delay bypass 
  }
}
