void ota() 
{
  read_trigger();
  while (power == 10) 
  {
    if (micros() - loop_timer > 5e6)                                  // If after boot 5s hold power run ota
    {                                    
      WiFi.begin("MinhPhone", "12341234");                            // Wifi connect
    
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
      { 
        request->send(200, "text/plain", "ESP8266 OTA.");
      });
    
      AsyncElegantOTA.begin(&server);
      server.begin();
      while (true) delay(1);                            // stay in OTA "delay(1) to bypass watchdog"
    }
    read_trigger();
  } 
}
