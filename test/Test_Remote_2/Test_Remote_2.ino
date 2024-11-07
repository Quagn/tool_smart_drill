#include <esp_now.h>
#include <WiFi.h>

uint8_t                                         // Pin inout
pin_trigger_direction = 39,                       // Trigger pin      
pin_trigger_power     = 34,

pin_esc_direction     = 33,                       // Esc pin
pin_esc_power         = 32;

typedef struct data_remote {
  uint8_t power;
  bool direction_;
} data_remote;

data_remote remote;

uint8_t power;
bool direction_, trigger;

unsigned long lastRecvTime = 0;
const long fixedInterval = 100;  // 1 giây

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  unsigned long currentMillis = millis();
  
  // Chỉ xử lý gói tin nếu đã qua khoảng thời gian cố định (fixedInterval)
  if (currentMillis - lastRecvTime >= fixedInterval) {
    lastRecvTime = currentMillis;  // Cập nhật thời gian nhận cuối cùng
    
    memcpy(&remote, incomingData, sizeof(remote));
    //Serial.print("power: ");
    //Serial.println(remote.power);
    power = remote.power;
    //Serial.print("direction: ");
    //Serial.println(remote.direction_);
    direction_ = remote.direction_;
  }
}
 
void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  pinMode(pin_esc_direction, OUTPUT);                             // Set esc direction pin output
  pinMode(pin_esc_power, OUTPUT);                                 // Set esc power pin output

  xTaskCreatePinnedToCore(MotorControl,  "MotorControl",  2048, NULL, 1, NULL, 1);  // Create motor control task
}
 
void loop() 
{
  
}

void MotorControl(void *pvParameters) {
  (void) pvParameters;
  for (;;) 
  {
    //read_trigger();
    run_esc();
    //vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void read_trigger() 
{
  //direction_ = (GPIO.in1.val & (1 << (pin_trigger_direction - 32))) != 0; // Read trigger direction
  
  //power = analogRead(pin_trigger_power) / 409.5;     //Set Esc power
}

void run_esc() 
{          
  (direction_ ? GPIO.out1_w1ts.val : GPIO.out1_w1tc.val) 
    = ((uint32_t)1 << (pin_esc_direction - 32));                  // Set ESC direction

  if (power == 0) 
  {
    GPIO.out1_w1ts.val = ((uint32_t)1 << (pin_esc_power - 32));   // Power off ESC
    trigger = false;
  } else if (power < 3 && !trigger)
  {
    GPIO.out1_w1tc.val = ((uint32_t)1 << (pin_esc_power - 32));   // Short ESC pulse
    vTaskDelay(30 / portTICK_PERIOD_MS);
    GPIO.out1_w1ts.val = ((uint32_t)1 << (pin_esc_power - 32));
    trigger = true;
  } else if (power == 1) 
  {
    GPIO.out1_w1tc.val = ((uint32_t)1 << (pin_esc_power - 32));   // Low power pulse
    vTaskDelay(3 / portTICK_PERIOD_MS);
    GPIO.out1_w1ts.val = ((uint32_t)1 << (pin_esc_power - 32));
    vTaskDelay(7 / portTICK_PERIOD_MS);
  } else 
  {
    GPIO.out1_w1tc.val = ((uint32_t)1 << (pin_esc_power - 32));   // Default pulse
    if (power < 10) 
    {
      vTaskDelay(power / portTICK_PERIOD_MS);
      GPIO.out1_w1ts.val = ((uint32_t)1 << (pin_esc_power - 32));
      vTaskDelay((10 - power) / portTICK_PERIOD_MS);
    }
  }
}
