void setup_sensor() 
{
  temp_sen_int.begin();                   // Setup IntTemp         
  temp_sen_int.setResolution(9);
  temp_sen_int.requestTemperatures();

  Wire.setClock(700000);                  // Setup I2C bus          
  Wire.begin(pin_imu_scl, pin_imu_sda);
  delay(10);

  // Setup MPU6050
  writeREG(MPU6050_ADDR, 0x6B, 0x00);     // Activate power
  writeREG(MPU6050_ADDR, 0x1B, 0x08);     // Set Gyro range (500dps)
  writeREG(MPU6050_ADDR, 0x1C, 0x10);     // Set Acce range (+/- 8g)
  writeREG(MPU6050_ADDR, 0x1A, 0x03);     // Digital Low Pass Filter

  // Setup BMP180
  writeREG(BMP180_ADDR, 0xF4, 0x2E);      // Set Baro oversampling to 8

  // Setup HMC5883L (Magnetometer)
  writeREG(MPU6050_ADDR, 0x37, 0x02);     // Set bypass mode (MPU6050)
  writeREG(MPU6050_ADDR, 0x6A, 0x00);  
  writeREG(HMC5883_ADDR, 0x00, 0x70);     // Set Mag 8-sample, 15Hz
  writeREG(HMC5883_ADDR, 0x01, 0x20);     // Set Mag 1.3 gauss
  writeREG(HMC5883_ADDR, 0x02, 0x00);     // Continuous mode

  delay(1000);                            // Stabilize
  run_buzzer(100);                        // Alarm for Gyro calibration
  cali_AcGy();                            // Calibration
  run_buzzer(100);                        // Alarm for Mag calibration
  cali_Mag();
  cali_Baro();
  run_buzzer(100);                        // Alarm for calibration done
}

//////////////////////////////// Temp
void read_Temp() 
{
  if (temp_sen_int.isConversionComplete()) {
    Temp_int = temp_sen_int.getTempCByIndex(0);   // Read internal temperature
    temp_sen_int.requestTemperatures(); 
  }
  readREG(MPU6050_ADDR, 0x41, &temp_ext, 2);      // Read external temperature
  Temp_ext = temp_ext / 340 + 16.53;
}

//////////////////////////////// Battery
void read_Batt() 
{
  uint8_t prevbatt = batt;
  batt = (((analogRead(pin_batt) - 3218) / 8.777) + prevbatt) / 2;   // Read battery voltage
}

//////////////////////////////// Accelerometer and Gyroscope
void read_AcGy() {                                // Read and calibrate gyro & accelerometer (MPU6050)
  read_AcGy_raw();
  accex -= 110;                                       // Compensate acce errors
  accez -= 190;

  gyrox -= caligyrox;                                 // Compensate gyro errors
  gyroy -= caligyroy;
  gyroz -= caligyroz;
}

void read_AcGy_raw() {                            // Read raw accelerometer and gyro data
  readREG(MPU6050_ADDR, 0x3B, &accex, 2);             // AcceX
  readREG(MPU6050_ADDR, 0x3D, &accey, 2);             // AcceY
  readREG(MPU6050_ADDR, 0x3F, &accez, 2);             // AcceZ
  readREG(MPU6050_ADDR, 0x43, &gyrox, 2);             // GyroX
  readREG(MPU6050_ADDR, 0x45, &gyroy, 2);             // GyroY
  readREG(MPU6050_ADDR, 0x47, &gyroz, 2);             // GyroZ
}

void cali_AcGy() {                                // Calibration for MPU6050
  tft.fillScreen(TFT_BLACK);           
  tft.setCursor(40, 80);
  tft.setTextSize(3);                    
  tft.print("Cali Gyro");
  
  for (uint16_t i = 0; i < 2000; i++) {    
    read_AcGy_raw();
    caligyrox += gyrox;
    caligyroy += gyroy;
    caligyroz += gyroz;
    delay(1);
  }
  caligyrox /= 2000;
  caligyroy /= 2000;
  caligyroz /= 2000;
}

//////////////////////////////// Barometer
void read_Baro() {                                // Read barometer (BMP180)
  int32_t x1, x2, x3, b3, b5, b6, p;
  uint32_t b4, b7;
  
  read_Baro_raw();

  x1 = ((temp_baro - calibaroac6) * calibaroac5) >> 15; // True temperature calculation
  x2 = (calibaromc << 11) / (x1 + calibaromd);
  b5 = x1 + x2;
  Temp_baro = (b5 + 8) >> 4;

  b6 = b5 - 4000;                                       // True pressure calculation
  x1 = (calibarob2 * (b6 * b6 >> 12)) >> 11;
  x2 = (calibaroac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((calibaroac1 * 4 + x3) << 3) + 2) >> 2;
  x1 = (calibaroac3 * b6) >> 13;
  x2 = (calibarob1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (calibaroac4 * (uint32_t)(x3 + 32768)) >> 15;
  b7 = ((uint32_t)(pressure - b3) * (50000 >> 3));

  p = (b7 < 0x80000000) ? (b7 << 1) / b4 : (b7 / b4) << 1;

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  pressure = p + ((x1 + x2 + 3791) >> 4); 
  Temp_baro = (Temp_baro / 10.0) - 8;
}

void read_Baro_raw() {                            // Read raw barometer data
  writeREG(BMP180_ADDR, 0xF4, 0x2E);                  // Start temperature measurement
  vTaskDelay(5 / portTICK_PERIOD_MS);                 // Wait for measurement
  readREG(BMP180_ADDR, 0xF6, &temp_baro, 2);
  
  writeREG(BMP180_ADDR, 0xF4, 0x34 + (3 << 6));       // Start pressure measurement
  vTaskDelay((2 + (3 << 3)) / portTICK_PERIOD_MS);    // Wait for pressure
  readREG(BMP180_ADDR, 0xF6, &pressure, 3);
  pressure = pressure >> (8 - 3);
}

void cali_Baro() {                                // Calibration for BMP180
  readREG(BMP180_ADDR, 0xAA, &calibaroac1, 2);      // AC1
  readREG(BMP180_ADDR, 0xAC, &calibaroac2, 2);      // AC2
  readREG(BMP180_ADDR, 0xAE, &calibaroac3, 2);      // AC3
  readREG(BMP180_ADDR, 0xB0, &calibaroac4, 2);      // AC4
  readREG(BMP180_ADDR, 0xB2, &calibaroac5, 2);      // AC5
  readREG(BMP180_ADDR, 0xB4, &calibaroac6, 2);      // AC6
  readREG(BMP180_ADDR, 0xB6, &calibarob1, 2);       // B1
  readREG(BMP180_ADDR, 0xB8, &calibarob2, 2);       // B2
  readREG(BMP180_ADDR, 0xBA, &calibaromb, 2);       // MB
  readREG(BMP180_ADDR, 0xBC, &calibaromc, 2);       // MC
  readREG(BMP180_ADDR, 0xBE, &calibaromd, 2);       // MD
}

//////////////////////////////// Magnetometer
void read_Mag() {                                 // Read magnetometer (HMC5883L)
  read_Mag_raw();
}

void read_Mag_raw() {                             // Read raw magnetometer data
  readREG(HMC5883_ADDR, 0x03, &magnex, 2);            // MagX
  readREG(HMC5883_ADDR, 0x05, &magney, 2);            // MagY
  readREG(HMC5883_ADDR, 0x07, &magnez, 2);            // MagZ
}

void cali_Mag() {                                 // Calibration for HMC5883L
  tft.fillScreen(TFT_BLACK);           
  tft.setCursor(40, 80);
  tft.setTextSize(3);                    
  tft.print("Cali Mage");
  
  int16_t min_x =  32767, min_y =  32767, min_z =  32767;
  int16_t max_x = -32768, max_y = -32768, max_z = -32768;

  for (uint16_t i = 0; i < 500; i++) 
  {
    read_Mag_raw();
    min_x = min(min_x, (int16_t)magnex);
    max_x = max(max_x, (int16_t)magnex);
    min_y = min(min_y, (int16_t)magney);
    max_y = max(max_y, (int16_t)magney);
    min_z = min(min_z, (int16_t)magnez);
    max_z = max(max_z, (int16_t)magnez);
    delay(10);
  }

  int16_t offset_x = (max_x + min_x) / 2;
  int16_t offset_y = (max_y + min_y) / 2;
  int16_t offset_z = (max_z + min_z) / 2;
}
