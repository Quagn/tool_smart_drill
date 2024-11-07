void calculator()
{
  calAngle();
  calAltitude();
  calHeading();
}

void calAngle() 
{
  // Scale accelerometer and gyroscope values
  float AcceX = accex / 409.59;
  float AcceY = accey / 409.59;
  float AcceZ = accez / 409.59;
  float GyroX = gyrox / 65.5;
  float GyroY = gyroy / 65.5;
  float GyroZ = gyroz / 65.5;
  
  float deltaTime = (micros() - sensor_timer) / 1e6;  // Calculate the time between measurements
  sensor_timer = micros();  // Update the sensor timer
  
  // Normalize accelerometer values (norm)
  float norm = sqrt(AcceX * AcceX + AcceY * AcceY + AcceZ * AcceZ);  // Compute the norm of accelerometer
  AcceX /= norm;  // Normalize AcceX
  AcceY /= norm;  // Normalize AcceY
  AcceZ /= norm;  // Normalize AcceZ

  // Adjust gyroscope values (gyroscope readings)
  float gx = GyroX * (M_PI / 180.0f);  // Convert GyroX from degrees/s to radians/s
  float gy = GyroY * (M_PI / 180.0f);  // Convert GyroY from degrees/s to radians/s
  float gz = GyroZ * (M_PI / 180.0f);  // Convert GyroZ from degrees/s to radians/s

  // Predict orientation change from gyroscope data
  float qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);  // Quaternion derivative for q0
  float qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);   // Quaternion derivative for q1
  float qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);   // Quaternion derivative for q2
  float qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);   // Quaternion derivative for q3

  // Calculate quaternion products
  float q1q3 = q1 * q3;  // Product of q1 and q3
  float q0q2 = q0 * q2;  // Product of q0 and q2

  // Compute error based on accelerometer feedback
  float _2q0 = 2.0f * q0;  // 2 * q0
  float _2q1 = 2.0f * q1;  // 2 * q1
  float _2q2 = 2.0f * q2;  // 2 * q2
  float _2q3 = 2.0f * q3;  // 2 * q3
  float _4q0 = 4.0f * q0;  // 4 * q0
  float _4q1 = 4.0f * q1;  // 4 * q1
  float _4q2 = 4.0f * q2;  // 4 * q2
  float _8q1 = 8.0f * q1;  // 8 * q1
  float _8q2 = 8.0f * q2;  // 8 * q2
  float q0q0 = q0 * q0;    // Square of q0
  float q1q1 = q1 * q1;    // Square of q1
  float q2q2 = q2 * q2;    // Square of q2
  float q3q3 = q3 * q3;    // Square of q3

  // Error equation (accelerometer feedback)
  float f1 = _2q2 * (2.0f * q1q3 - _2q0 * q2 - AcceX);  // First component of the error
  float f2 = _2q0 * q1 + _2q1 * q3 - AcceY;  // Second component of the error
  float f3 = 1.0f - _2q1 * q1 - 2.0f * q2q2 - AcceZ;  // Third component of the error
  
  float j11 = -_2q3;  // Jacobian component j11
  float j12 = _2q2;   // Jacobian component j12
  float j13 = -_2q0;  // Jacobian component j13
  float j14 = _2q1;   // Jacobian component j14

  // Gradient descent algorithm correction step
  float s1 = j14 * f2 - j11 * f1;  // Gradient for q0
  float s2 = j12 * f1 + j13 * f2;  // Gradient for q1
  float s3 = j11 * f2 - j14 * f3;  // Gradient for q2

  norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);  // Normalize the gradient
  s1 /= norm;  // Normalize s1
  s2 /= norm;  // Normalize s2
  s3 /= norm;  // Normalize s3

  // Adjust the quaternions
  qDot1 -= beta * s1;  // Update q0
  qDot2 -= beta * s2;  // Update q1
  qDot3 -= beta * s3;  // Update q2

  // Integrate to get the final quaternions
  q0 += qDot1 * deltaTime;  // Integrate q0 over time
  q1 += qDot2 * deltaTime;  // Integrate q1 over time
  q2 += qDot3 * deltaTime;  // Integrate q2 over time
  q3 += qDot4 * deltaTime;  // Integrate q3 over time

  // Normalize the quaternions again
  norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);  // Calculate quaternion norm
  q0 /= norm;  // Normalize q0
  q1 /= norm;  // Normalize q1
  q2 /= norm;  // Normalize q2
  q3 /= norm;  // Normalize q3

  // Calculate Roll, Pitch, and Yaw from quaternions
  float roll = atan2(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * (180 / M_PI);  // Roll calculation
  float pitch = asin(2.0f * (q0 * q2 - q3 * q1)) * (180 / M_PI);  // Pitch calculation
  float yaw = atan2(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * (180 / M_PI);  // Yaw calculation

  // Perform running average over 10 samples for Roll, Pitch, and Yaw
  totalRoll  -= rollSamples[sampleIndex];  // Subtract the old Roll value
  totalPitch -= pitchSamples[sampleIndex];  // Subtract the old Pitch value
  totalYaw   -= yawSamples[sampleIndex];  // Subtract the old Yaw value

  rollSamples[sampleIndex]  = roll;  // Store the new Roll value
  pitchSamples[sampleIndex] = pitch;  // Store the new Pitch value
  yawSamples[sampleIndex]   = yaw;  // Store the new Yaw value

  totalRoll  += roll;  // Add the new Roll value to the total
  totalPitch += pitch;  // Add the new Pitch value to the total
  totalYaw   += yaw;  // Add the new Yaw value to the total

  // Update index (wrap around using modulo)
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;  // Increment the sample index

  // Calculate the averaged angles
  AngleRoll  = totalRoll / NUM_SAMPLES;  // Compute the average Roll
  AnglePitch = totalPitch / NUM_SAMPLES;  // Compute the average Pitch
  AngleYaw   = totalYaw / NUM_SAMPLES;  // Compute the average Yaw
}

void calAltitude() 
{
  Altitude = (44330 * (1.0 - pow((pressure / 100.0) / 1013.25, 0.1903))) - 5040;
}

void calHeading() 
{
  Heading = atan2((float)magney, (float)magnex) * 180 / M_PI;
  if (Heading < 0) {
    Heading += 360;
  }
}
