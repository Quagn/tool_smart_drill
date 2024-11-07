void setup_trigger() 
{
  pinMode(pin_trigger_direction, INPUT);                                  // Set local trigger direction input
                                                
  esp_now_init();                                                         // Set recv remote trigger
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&remote, incomingData, sizeof(remote));                          // Read remote trigger
  sel = true;
}

void read_trigger()                         
{
  if (sel == true)                                                        // Selecte trigger
  {                                                                       // Set val by remote trigger
    power = remote.power;
    direction_ = remote.direction_;

    sel = false;
  } else                                                                  // Set val by local trigger
  {                                                                   
    power = analogRead(pin_trigger_power) / 409.5;     
    direction_ = (GPIO.in1.val & (1 << (pin_trigger_direction - 32))) != 0;
  }
}
