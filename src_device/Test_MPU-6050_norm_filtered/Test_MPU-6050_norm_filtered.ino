 /***************************************************************************************
 *
 * Copyright (c) 2016, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Smart Chef Project
 * File:        Test_MPU-6050_norm_filtered.ino
 * Date:        2016-10-23
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Smart-Chef
 *
 * Description: Test of the MPU-6050 IMU, displaying low-pass filtered values of the 
 *              acceleration vector's norm.
 *
 ***************************************************************************************/

#include<Wire.h>

#define ALPHA       0.7   // filtering coefficient
#define MPU_ADDR    0x68  // I2C address of the MPU-6050

int16_t acc[3], avg[3] = {0};
uint16_t norm;
uint8_t i;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
}
void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true); // request a total of 6 registers
  acc[0] = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  acc[1] = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  acc[2] = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  norm = 0;
  for(i = 0; i < 3; i++) {
    avg[i] = ALPHA*avg[i] + (1-ALPHA)*acc[i];
    norm = norm + ((uint32_t)pow(acc[i]-avg[i],2)>>15);
  }

  Serial.print("Filtered norm = "); Serial.println(norm);

  delay(333);
}
