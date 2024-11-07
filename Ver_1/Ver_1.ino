uint8_t                                     // Pin inout
pin_trigger_direction = 39,                     // Trigger pin
pin_trigger_power     = 34,

pin_esc_direction     = 33,                     // Esc pin
pin_esc_power         = 32,

pin_button            = 36,                     // Add pin
pin_buzzer            = 26,
pin_led               = 25,
pin_temp              = 27,
pin_batt              = 35;

#include <WiFi.h>
const char* ssid = "MinhPhone";
const char* password = "12341234";

#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>                     // Wev lib

#include <AsyncElegantOTA.h>                    // OTA lib
AsyncWebServer server(80);
// MAC Addr: 94:b5:55:2b:5c:64

#include <esp_now.h>                            // Remote lib
typedef struct data_remote {
  uint8_t power;
  bool direction_;
} data_remote;
data_remote remote;

#include <SPI.h>                                // Comunicate lib
#include <OneWire.h>
#include <Wire.h>

#include <DallasTemperature.h>                  // Int temp lib
OneWire oneWire(pin_temp);
DallasTemperature temp_sen_int(&oneWire);

#include <TFT_eSPI.h>                           // Lcd lib
TFT_eSPI tft = TFT_eSPI();
#define SCREEN_RAIDUS 240
#define SCREEN_CENTER 120
int16_t 
prevMode = -1,
prevBatt = -1,
prevLedAlw = -1,
prevLedDri = -1,
prev_x = -1, prev_y = -1,               // Mode 1 (DRILL)
perv_drill_mode,
prev_x1r = -1, prev_y1r = -1,           // Mode 2 (IDLE)
prev_x2r = -1, prev_y2r = -1,
prev_pitchX1r[5], prev_pitchY1r[5], 
prev_pitchX2r[5], prev_pitchY2r[5],
prevHeading = -1,                       // Mode 3 (MEASURE)
prevAltitude = -1,  
prevTemp = -1,
prevRemote;                             // Mode 4 (REMOTE)

///////////////////////////// Valual /////////////////////////////
uint32_t
loop_timer    = 0,                              // Loop timer
esc_timer     = 0,                              // Esc timer
bthold_timer  = 0,                              // Button hold timer
idle_timer    = 0,                              // Idle timer
sensor_timer  = 0,                              // Sensor timer
pulse_time    = 300,                            // Pulse drill time
window_timr   = 12500;                          // Windows time 80Hz


uint8_t
batt        = 0,                                // Batt percen
power       = 0,                                // Power val    
buzzer_on   = 0,                                // Buzz val
led_on      = 0,                                // Led val
button      = 0,                                // Button status
bthold      = 0,                                // Button hold
remote_on   = 0,                                // Remote status
drill_mode  = 1,                                // Drill mode
mode_run    = 2,                                // Mode runing
last_run    = 2,                                // Last mode
protection  = 0;                                // Protection status   

bool
led_dri_on  = false, prev_led_dri_on  = false,  // Led drill status
led_alw_on  = false, prev_led_alw_on  = false,  // Led always status
direction_  = true , prev_direction_  = true ,  // Direction
tear_on     = false,                            // Tear
primer_on   = false;                            // Esc primer                   

  ///////////////////////////////// SENSOR    
#define MPU6050_ADDR  0x68
#define BMP180_ADDR   0x77
#define HMC5883_ADDR  0x1E

int32_t
accex, accey, accez , gyrox, gyroy, gyroz, temp_ext,                            // MPU6050
caligyrox, caligyroy, caligyroz,         
magnex, magney, magnez,                                                         // HMC5883L
calimagnex, calimagney, calimagnez, 
pressure, temp_baro, calibaro,                                                  // BMP180
calibaroac1, calibaroac2, calibaroac3,                      // AC1, AC2, AC3      (signed)
calibaroac4, calibaroac5, calibaroac6,                      // AC4, AC5, AC6    (unsigned)
calibarob1, calibarob2, calibaromb, calibaromc, calibaromd; // B1, B2, MB, MC, MD (signed)

  ///////////////////////////////// STATUS                                     
float
Temp_int, Temp_ext, Temp_baro,                                    // Temp
Altitude, Heading,                                                // Altitude
AngleRoll, AnglePitch, AngleYaw,                                  // Angle
Tear_Roll, Tear_Pitch, Tear_Yaw;                                  // TearAngle

  ///////////////////////////////// FILLTER    

#define NUM_SAMPLES 10  // Số lần trung bình cộng

// Mảng lưu giá trị 10 lần tính toán gần nhất
float rollSamples[NUM_SAMPLES] = {0};
float pitchSamples[NUM_SAMPLES] = {0};
float yawSamples[NUM_SAMPLES] = {0};

// Tổng dùng để tính trung bình
float totalRoll = 0, totalPitch = 0, totalYaw = 0;

// Các biến quaternion để tính toán Madgwick
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;  // Quaternions đại diện cho hướng
float beta = 0.1f;  // Hệ số điều chỉnh Madgwick

int sampleIndex = 0;

void setup(void)
{
  Serial.begin(115200);
  loop_timer = micros();                                          // Start timer

  setup_lcd();                                                    // Setup LCD

  WiFi.mode(WIFI_STA);                                            // Turn on Wifi
  ota();                                                          // Ota

  setup_trigger();                                                // Setup trigger
  setup_lcd();                                                    // Setup LCD
  setup_esc();                                                    // Setup ESC
  setup_led_buzzer();                                             // Setup Add
  setup_sensor();                                                 // Setup sensor
  
  ///////////////////////// INPUT /////////////////////////                                     // Create INPUT read task
  xTaskCreatePinnedToCore(ReadTrigger, "ReadTrigger",  4096, NULL, 2, NULL, tskNO_AFFINITY);    
  xTaskCreatePinnedToCore(ReadButton , "ReadButton" ,  2048, NULL, 2, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ReadIMU    , "ReadIMU"    , 12288, NULL, 2, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ReadTemp   , "ReadTemp"   ,  2048, NULL, 1, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ReadBatt   , "ReadBatt"   ,  2048, NULL, 1, NULL, tskNO_AFFINITY);
  
  ///////////////////////// CALCU /////////////////////////                                     // Create CALCULATOR task
  xTaskCreatePinnedToCore(Mode       , "Mode"       ,  6144, NULL, 2, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(Protect    , "Protect"    ,  6144, NULL, 2, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(Calculator , "Calculator" ,  6144, NULL, 1, NULL, tskNO_AFFINITY);

  ///////////////////////// OUTPUT /////////////////////////                                    // Create OUPUT run task
  xTaskCreatePinnedToCore(RunEsc     , "RunEsc"     ,  2048, NULL, 2, NULL, 1);                
  xTaskCreatePinnedToCore(RunBuzz    , "RunBuzz"    ,  2048, NULL, 1, NULL, tskNO_AFFINITY);                
  xTaskCreatePinnedToCore(RunLed     , "RunLed"     ,  2048, NULL, 1, NULL, tskNO_AFFINITY); 
  xTaskCreatePinnedToCore(RunLcd     , "RunLcd"     , 16384, NULL, 1, NULL, tskNO_AFFINITY);     
    
}

  ///////////////////////// INPUT /////////////////////////
void ReadTrigger(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    read_Trigger();
    vTaskDelay(65 / portTICK_PERIOD_MS);
  }
}

void ReadButton(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    read_Button();
    vTaskDelay(165 / portTICK_PERIOD_MS);
  }
}

void ReadIMU(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    read_AcGy();
    read_Baro();
    read_Mag();
    vTaskDelay(165 / portTICK_PERIOD_MS);
  }
}

void ReadTemp(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    read_Temp();
    vTaskDelay(1650 / portTICK_PERIOD_MS);
  }
}

void ReadBatt(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    read_Batt();
    vTaskDelay(1650 / portTICK_PERIOD_MS);
  }
}
  
  ///////////////////////// CALCU /////////////////////////
void Mode(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    mode_menu();
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void Calculator(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    calculator();
    vTaskDelay(215 / portTICK_PERIOD_MS);
  }
}

void Protect(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    protection_();
    vTaskDelay(215 / portTICK_PERIOD_MS);
  }
}

  ///////////////////////// OUTPUT /////////////////////////
void RunEsc(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    run_esc();
  }
}

void RunBuzz(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    run_buzzer(buzzer_on);
  }
}

void RunLed(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    run_led(led_on);
  }
}

void RunLcd(void *pvParameters) {
  (void) pvParameters;
  for (;;){
    run_lcd();
    vTaskDelay(40 / portTICK_PERIOD_MS);
  }
}

void loop()
{

}

void writeREG(uint8_t deviceaddress, uint8_t eeaddress, uint8_t data)
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress);
  Wire.write(data);
  Wire.endTransmission();
}

void readREG(uint8_t deviceaddress, uint8_t eeaddress, int32_t* data, uint8_t byte_request) 
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress);
  Wire.endTransmission(false);
  
  Wire.requestFrom(deviceaddress, byte_request);
  
  uint32_t value = 0;
  for (uint8_t k = 0; k < byte_request; k++) 
  {
    value = (value << 8) | Wire.read();
  }

  if (value & (1 << ((byte_request * 8) - 1)))      // Check sign bit
  {   
    value |= 0xFFFFFFFF << (byte_request * 8);      // Extend sign bit if negative
  }

  *data = value;
}
