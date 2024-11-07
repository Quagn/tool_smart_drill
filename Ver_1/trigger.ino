void setup_trigger() 
{
  // Set local trigger direction as input
  pinMode(pin_trigger_direction, INPUT);                                  

  // Initialize ESP-NOW and register callback for receiving data
  esp_now_init();                                                         
  esp_now_register_recv_cb(OnDataRecv);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) 
{
  // Copy incoming remote trigger data
  memcpy(&remote, incomingData, sizeof(remote));                          
  remote_on = 5;                // Reset remote trigger countdown
}

void read_Trigger() {
  if (remote_on > 0)            // Use remote trigger
  {          
    power = remote.power;
    direction_ = remote.direction_;
    remote_on--;                    // Decrement remote trigger countdown for smoothing
  } else                        // Use local trigger
  {                      
    power = analogRead(pin_trigger_power) / 409.5;  // Scale to appropriate range
    direction_ = (GPIO.in1.val & (1U << (pin_trigger_direction - 32))) != 0;  // Read direction from GPIO
  }
}
