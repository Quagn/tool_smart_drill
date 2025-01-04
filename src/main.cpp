#include <Arduino.h>

//////////////////////////////////////////////////////////////////////////////////  DEFINE
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////// Comunicate
/////////////////////////////////////////////////////////////////////////////////
                                                                      // Comunicate Wired lib
#include <SPI.h>                                                  // SPI          
#include <OneWire.h>                                              // OneWire
#include <Wire.h>                                                 // I2C                  
                                                                      // Comunicate Wireless lib
#include <WiFi.h>                                                 // Wifi       
const char* ssid = "MinhPhone";                                   
const char* password = "12341234";
// MAC Addr: 94:b5:55:2b:5c:64                                    // ESP Now

/////////////////////////////////////////////////////////////////////////// Timer
/////////////////////////////////////////////////////////////////////////////////
uint32_t
loop_timer    = 0,                              // Loop timer
esc_timer     = 0,                              // Esc timer
bthold_timer  = 0,                              // Button hold timer
idle_timer    = 0,                              // Idle timer
sensor_timer  = 0,                              // Sensor timer
pulse_time    = 300,                            // Pulse drill time
window_timr   = 12500;                          // Windows time 80Hz

/////////////////////////////////////////////////////////////////////////// Input
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////// Input pin & address            
uint8_t                                                             
pin_imu_sda           = 21,                                       // IMU pin                 
pin_imu_scl           = 22,     
pin_button            = 36,                                       // Button (adc) pin
pin_temp              = 27,                                       // IntTemperature pin
pin_batt              = 35,                                       // Batt Vol Sensor pin      
pin_trigger_direction = 39,                                       // Remote pin
pin_trigger_power     = 34;  
#define MPU6050_ADDR  0x68
#define BMP180_ADDR   0x77
#define HMC5883_ADDR  0x1E
/////////////////////////////////////////////////////////// Input lib 
#include <DallasTemperature.h>                                        // 
OneWire oneWire(pin_temp);
DallasTemperature temp_sen_int(&oneWire);                         // IntTemperature lib      
#include <esp_now.h>                                                  // Remote trigger
typedef struct data_remote {                                      // Stuct remote data
  uint8_t power;
  bool direction_;
} data_remote;
data_remote remote;
/////////////////////////////////////////////////////////// Input variable
int32_t                                                               
accex, accey, accez,                                              // MPU6050
scale_accel           = 0; //     
caliaccex             = 0, 
caliaccey             = 0, 
caliaccez             = 0,
gyrox, gyroy, gyroz,   
scale_gyro            = 0; //                           
caligyrox             = 0, 
caligyroy             = 0, 
caligyroz             = 0,         
temp_ext, 
scale_temp_ext        = 0,   
cali_temp_ext         = 0,  
magnex, magney, magnez,                                           // HMC5883L
scale_magne           = 0,
calimagnex            = 0, 
calimagney            = 0, 
calimagnez            = 0, 
pressure,                                                         // BMP180
temp_baro,
calibaro              = 0,
calibaroac1, calibaroac2, calibaroac3,              // AC1, AC2, AC3    (signed)
calibaroac4, calibaroac5, calibaroac6,              // AC4, AC5, AC6  (unsigned)
calibarob1 , calibarob2 ,                           // B1, B2           (signed)
calibaromb , calibaromc , calibaromd;               // MB, MC, MD       (signed)

uint8_t 
temp_int              = 0;                                        // IntTemp
scale_temp_int        = 0; //
batt                  = 0,                                        // Batt percen
scale_batt            = 0; //
button                = 0,                                        // Button status
bthold                = 0,                                        // Button hold
remote_on             = 0;                                        // Remote status

/////////////////////////////////////////////////////////////////////////// Calculate
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////// State
float
Temp_int, Temp_ext, Temp_baro,                                    // Temp
Altitude, Heading,                                                // Altitude
AngleRoll, AnglePitch, AngleYaw,                                  // Angle
Tear_Roll, Tear_Pitch, Tear_Yaw;                                  // TearAngle

#define NUM_SAMPLES 10  // Số lần trung bình cộng
// Mảng lưu giá trị 10 lần tính toán gần nhất
float rollSamples [NUM_SAMPLES] = {0};
float pitchSamples[NUM_SAMPLES] = {0};
float yawSamples  [NUM_SAMPLES] = {0};

// Tổng dùng để tính trung bình
float totalRoll = 0, totalPitch = 0, totalYaw = 0;

// Các biến quaternion để tính toán Madgwick
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;  // Quaternions đại diện cho hướng
float beta = 0.1f;  // Hệ số điều chỉnh Madgwick

int sampleIndex = 0;

/////////////////////////////////////////////////////////// Menu
uint8_t
drill_mode    = 1,                              // Drill mode
mode_run      = 2,                              // Mode runing
last_run      = 2,                              // Last mode

bool                                              // Display menu
led_dri_on    = false, prev_led_dri_on  = false,  // Led drill status
led_alw_on    = false, prev_led_alw_on  = false,  // Led always status
direction_    = true , prev_direction_  = true ,  // Direction
tear_on       = false,                            // Tear
primer_on     = false;                            // Esc primer   

int16_t 
prevMode        = -1,
prevBatt        = -1,
prevLedAlw      = -1,
prevLedDri      = -1,
prev_x          = -1, 
prev_y          = -1,                                     // Mode 1 (DRILL)
perv_drill_mode =  0,
prev_x1r        = -1, 
prev_y1r        = -1,                                     // Mode 2 (IDLE)
prev_x2r        = -1, 
prev_y2r        = -1,
prev_pitchX1r[5], 
prev_pitchY1r[5], 
prev_pitchX2r[5], 
prev_pitchY2r[5],
prevHeading     = -1,                                     // Mode 3 (MEASURE)
prevAltitude    = -1,  
prevTemp        = -1,
prevRemote      =  0;                                     // Mode 4 (REMOTE)

/////////////////////////////////////////////////////////////////////////// Safety
/////////////////////////////////////////////////////////////////////////////////
uint8_t 
protection    = 0;                                                    // Protection status   

/////////////////////////////////////////////////////////////////////////// Output
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////// Output pin
uint8_t                                                               // 
pin_esc_direction     = 33,                                       // Esc pin
pin_esc_power         = 32,
pin_buzzer            = 26,                                       // Buzzer pin
pin_led               = 25;                                       // Led pin
/////////////////////////////////////////////////////////// Output lib
#include <TFT_eSPI.h>                                                 // 
TFT_eSPI tft = TFT_eSPI();                                        // Lcd lib
#define SCREEN_RAIDUS 240
#define SCREEN_CENTER 120
/////////////////////////////////////////////////////////// Output variable
uint8_t                                                               // 
power                 = 0,                                  // Power val 
buzzer_on             = 0,                                  // Buzzer val
led_on                = 0;                                  // Led val

/////////////////////////////////////////////////////////////////////////// Checking
/////////////////////////////////////////////////////////////////////////////////
uint8_t
check_ota             = 0,

check_imu             = 0,
check_buttons         = 0,
check_remote          = 0,
check_int_temp        = 0,
check_batt_sensor     = 0,

check_esc             = 0,
check_lcd             = 0,
check_led             = 0,
check_buzz            = 0,

check_oke             = 0;

/////////////////////////////////////////////////////////////////////////// Logging
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////// Update
/////////////////////////////////////////////////////////////////////////////////
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>                                     // Wev lib
#include <AsyncElegantOTA.h>                                    // OTA lib
AsyncWebServer server(80);

//////////////////////////////////////////////////////////////////////////////////  SETUP
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  loop_timer = micros();                                              // Start timer
}

//////////////////////////////////////////////////////////////////////////////////  LOOP
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void loop() {

}