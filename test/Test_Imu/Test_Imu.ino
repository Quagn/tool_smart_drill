#include <Wire.h>
float Imu[6], CaliImu[3];             // Imu

uint32_t loop_timer;

void setup() {
  Wire.setClock(700000);            //Setup i2c bus
  Wire.begin();
  delay(250);

  Serial.begin(115200);
  writeREG(0x68, 0x6B, 0x00);             //Set the Powe register to activate
  writeREG(0x68, 0x1B, 0x08);             //Set the Acce register (500dps full scale)
  writeREG(0x68, 0x1C, 0x10);             //Set the Gyro register (+/- 8g full scale range)
  //writeREG(0x68, 0x1A, 0x03);             //Set Digital Low Pass Filter to ~43Hz

  loop_timer = micros();                                               //Reset the loop timer
}

void loop() {
  read_imu();
  Serial.print("Imu 1: ");
  Serial.print((int)Imu[3]);
  Serial.print("  Imu 2: ");
  Serial.print((int)Imu[4]);
  Serial.print("  Imu 3: ");
  Serial.println((int)Imu[5]);
  
  
  while(micros() - loop_timer < 10000);            //Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
  loop_timer = micros();   

}

void read_imu(){
  int32_t imu[7];
  readREG(0x68, 0x3B, imu, 7, 2);         // Đọc 3 giá trị từ địa chỉ 0x3B
  for (uint8_t j = 0; j < 3; j++){
    Imu[j] = (float)imu[j] / 819.18;        // accScale  = 417.53    65535 / 4g
  }
  readREG(0x68, 0x43, imu, 3, 2);         // Đọc 3 giá trị từ địa chỉ 0x43
  for (uint8_t j = 3; j < 6; j++){
    Imu[j] = (float)imu[j] / 131.07;        // gyroScale = 131.07    65535 / 500dps
  }
}

void writeREG(uint8_t deviceaddress, uint8_t eeaddress, uint8_t data){
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress);
  Wire.write(data);
  Wire.endTransmission();
}

void readREG(uint8_t deviceaddress, uint8_t eeaddress, int32_t* data, uint8_t number_of_request, uint8_t byte_per_request) {
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress);
  Wire.endTransmission(false);
  
  Wire.requestFrom(deviceaddress, uint8_t(number_of_request * byte_per_request));
  for (uint8_t j = 0; j < number_of_request; j++) {
    uint32_t value = 0;
    for (uint8_t k = 0; k < byte_per_request; k++) {
      value = (value << 8) | Wire.read();
    }

    if (value & (1 << ((byte_per_request * 8) - 1))) {        // Kiểm tra dấu. Nếu dấu - thì đổi toàn bộ bit phía trước,
      value |= 0xFFFFFFFF << (byte_per_request * 8);          // nếu + thì giữ nguyên
    }

    data[j] = value;
  }
}
