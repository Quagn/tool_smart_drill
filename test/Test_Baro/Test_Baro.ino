#include <Wire.h>

#define BMP180_ADDR   0x77

int32_t     
pressure, temp_ext,                                                                 // BMP180
calibaroac1, calibaroac2, calibaroac3,  
calibarob1, calibarob2, calibaromb, calibaromc, calibaromd,      
calibaroac4, calibaroac5, calibaroac6;                              

float
Temp_ext,                                               // Temp
Altitude;                                               // Altitude

void setup() {
  Serial.begin(115200);
  Wire.begin(22, 21);
  cali_Baro();
}

void loop() 
{
  read_Baro();

  delay(1000);  // Chờ 1 giây giữa các lần đo
}

void read_Baro() 
{
  int32_t x1, x2, x3, b3, b5, b6, p;
  uint32_t b4, b7;

  // Read raw data from BMP180
  read_Baro_raw();

  // Calculate true temperature (based on BMP180 datasheet)
  x1 = ((temp_ext - calibaroac6) * calibaroac5) >> 15;
  x2 = (calibaromc << 11) / (x1 + calibaromd);
  b5 = x1 + x2;
  Temp_ext = (b5 + 8) >> 4;

  // Calculate true pressure
  b6 = b5 - 4000;
  x1 = (calibarob2 * (b6 * b6 >> 12)) >> 11;
  x2 = (calibaroac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((calibaroac1 * 4 + x3) << 3) + 2) >> 2;
  x1 = (calibaroac3 * b6) >> 13;
  x2 = (calibarob1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (calibaroac4 * (uint32_t)(x3 + 32768)) >> 15;
  b7 = ((uint32_t)(pressure - b3) * (50000 >> 3));

  if (b7 < 0x80000000) {
    p = (b7 << 1) / b4;
  } else {
    p = (b7 / b4) << 1;
  }

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  pressure = p + ((x1 + x2 + 3791) >> 4);  // Use 'pressure' instead of 'P'

  // Calculate altitude using barometric formula
  Altitude = 44330 * (1.0 - pow((pressure / 100.0) / 1013.25, 0.1903));

  // Print results
  Serial.print("Temperature: ");
  Serial.print(Temp_ext / 10.0);  // BMP180 returns temperature in 0.1°C increments
  Serial.println(" °C");

  Serial.print("Pressure: ");
  Serial.print(pressure / 100.0);  // BMP180 returns pressure in Pa, converting to hPa
  Serial.println(" hPa");

  Serial.print("Altitude: ");
  Serial.print(Altitude);  // Altitude in meters
  Serial.println(" m");
}

void read_Baro_raw() 
{
  // Start temperature measurement
  writeREG(BMP180_ADDR, 0xF4, 0x2E);        // Write 0x2E into register 0xF4 to request temperature measurement
  vTaskDelay(5 / portTICK_PERIOD_MS);                         // Wait for measurement (4.5ms max)
  
  // Read raw temperature data (2 bytes)
  readREG(BMP180_ADDR, 0xF6, &temp_ext, 2);
  
  // Start pressure measurement (3 value affects delay)
  writeREG(BMP180_ADDR, 0xF4, 0x34 + (3 << 6));               // Start pressure measurement
  vTaskDelay((2 + (3 << 3)) / portTICK_PERIOD_MS);            // Wait for measurement (depends on 3)
  
  // Read raw pressure data (3 bytes, last byte is for 3 shift)
  readREG(BMP180_ADDR, 0xF6, &pressure, 3);
  pressure = pressure >> (8 - 3);
}

void cali_Baro() 
{                                                 // Calibrate BMP180
  // AC1 to AC6 are 16-bit signed or unsigned values, so they can be stored in int32_t and uint32_t
  readREG(BMP180_ADDR, 0xAA, &calibaroac1, 2);  // AC1
  readREG(BMP180_ADDR, 0xAC, &calibaroac2, 2);  // AC2
  readREG(BMP180_ADDR, 0xAE, &calibaroac3, 2);  // AC3
  readREG(BMP180_ADDR, 0xB0, &calibaroac4, 2);  // AC4 (unsigned)
  readREG(BMP180_ADDR, 0xB2, &calibaroac5, 2);  // AC5 (unsigned)
  readREG(BMP180_ADDR, 0xB4, &calibaroac6, 2);  // AC6 (unsigned)

  // B1, B2, MB, MC, MD are 16-bit signed values
  readREG(BMP180_ADDR, 0xB6, &calibarob1, 2);   // B1
  readREG(BMP180_ADDR, 0xB8, &calibarob2, 2);   // B2
  readREG(BMP180_ADDR, 0xBA, &calibaromb, 2);   // MB
  readREG(BMP180_ADDR, 0xBC, &calibaromc, 2);   // MC
  readREG(BMP180_ADDR, 0xBE, &calibaromd, 2);   // MD

  
  Serial.print(calibaroac1);
  Serial.print(" ");
  Serial.print(calibaroac2);
  Serial.print(" ");
  Serial.print(calibaroac3);
  Serial.print(" ");
  Serial.print(calibaroac4);
  Serial.print(" ");
  Serial.print(calibaroac5);
  Serial.print(" ");
  Serial.print(calibaroac6);
  Serial.print(" ");
  Serial.print(calibarob1);
  Serial.print(" ");
  Serial.print(calibarob2);
  Serial.print(" ");
  Serial.print(calibaromb);
  Serial.print(" ");
  Serial.print(calibaromc);
  Serial.print(" ");
  Serial.println(calibaromd);
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
