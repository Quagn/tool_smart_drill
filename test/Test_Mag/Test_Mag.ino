#include <Wire.h>

#define HMC5883L_ADDRESS 0x1E  // Địa chỉ I2C của HMC5883L

// Biến để lưu giá trị đọc từ cảm biến
int16_t mag_x, mag_y, mag_z;

void setup() {
  Serial.begin(115200);
  Wire.begin(22,21);
  //Bypass Mode
  Wire.beginTransmission(0x68);
  Wire.write(0x37);
  Wire.write(0x02);
  Wire.endTransmission();
  
  Wire.beginTransmission(0x68);
  Wire.write(0x6A);
  Wire.write(0x00);
  Wire.endTransmission();
  
  //Disable Sleep Mode
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  // Khởi tạo HMC5883L với cấu hình chính xác
  init_HMC5883L();
}

void loop() {
  // Đọc dữ liệu từ cảm biến
  read_Mag();
  
  // Hiển thị dữ liệu đọc được
  Serial.print("Mag X: "); Serial.print(mag_x);
  Serial.print(" Mag Y: "); Serial.print(mag_y);
  Serial.print(" Mag Z: "); Serial.println(mag_z);
  
  delay(1000);  // Delay 1 giây để xem dữ liệu dễ dàng
}

void init_HMC5883L() {
  // Cấu hình Register A:
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(0x00);  // Đăng ký cấu hình A
  Wire.write(0x70);  // 8 trung bình mẫu, 15 Hz (tần số lấy mẫu), chế độ đo bình thường
  Wire.endTransmission();

  // Cấu hình Register B (Độ lợi):
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(0x01);  // Đăng ký cấu hình B
  Wire.write(0x20);  // Độ lợi: ±1.3 gauss (độ nhạy 1090 LSb/gauss)
  Wire.endTransmission();

  // Cấu hình chế độ hoạt động:
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(0x02);  // Đăng ký chế độ
  Wire.write(0x00);  // Chế độ đo liên tục
  Wire.endTransmission();
}

void read_Mag() {
  Wire.beginTransmission(HMC5883L_ADDRESS);
  Wire.write(0x03);  // Đăng ký dữ liệu đầu ra X MSB
  Wire.endTransmission();
  
  Wire.requestFrom(HMC5883L_ADDRESS, 6);
  
  if (Wire.available() == 6) {
    mag_x = Wire.read() << 8 | Wire.read();
    mag_z = Wire.read() << 8 | Wire.read();
    mag_y = Wire.read() << 8 | Wire.read();
  }
}
