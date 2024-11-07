uint8_t                                         // Pin inout
pin_trigger_direction = 39,                       // Trigger pin      
pin_trigger_power     = 34,

pin_esc_direction     = 33,                       // Esc pin
pin_esc_power         = 32,

pin_button            = 36,                       // Add pin
pin_buzzer            = 26,
pin_led               = 25,
pin_temp              = 27,
pin_batt              = 35;

#include <SPI.h>                                // Comunicate lib
#include <OneWire.h>
#include <Wire.h>                            

#include <DallasTemperature.h>                  // Sensor temp
OneWire oneWire(pin_temp);
DallasTemperature temp_sen_int(&oneWire);

uint32_t 
loop_timer,                                     // Loop timer
idle_timer,                                     // Idle timer
window_timer = 20000;                           // Windows time 100Hz

uint16_t
power,                                          //
temp_int,
temp_ext; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  loop_timer = micros();                                          // Start timer
  
  temp_sen_int.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  temp_sen_int.requestTemperatures(); 
  temp_int = temp_sen_int.getTempCByIndex(0);
  Serial.println(micros() - loop_timer);
  while (micros() - loop_timer < window_timer);
  loop_timer = micros();
  //Serial.println(temp_int);
}
