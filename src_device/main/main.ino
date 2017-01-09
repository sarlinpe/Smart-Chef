 /***************************************************************************************
 *
 * Copyright (c) 2016, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Smart Chef Project
 * File:        main.ino
 * Date:        2016-10-23
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Smart-Chef
 *
 * Description: Main file for the device's firmware.
 *
 ***************************************************************************************/
 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include<Wire.h>

#define DEBUG

/* Pinout */
#define BT_RX         11
#define BT_TX         12
#define ONE_WIRE_BUS  10     // Temperature probe pin
#define LED_R         A0
#define LED_O         A1
#define LED_G         A2

/* IMU handling */
#define ALPHA         0.7   // Filtering coefficient
#define MPU_ADDR      0x68  // I2C address of the MPU-6050
#define SHIFT_COEF    15
#define MOV_THRESHOLD 60

/* Master-Slave protocol */
#define REQ_READ      'r'
#define REQ_WRITE     'w'
#define ACK_2MST      0x10
#define ACK_2SLV      0x20
#define TIMEOUT       2000 // ms

/* Communication speeds */
#define USB_SPEED     9600
#define BT_SPEED      9600

#define POLL_PERIOD   500

SoftwareSerial BTserial(BT_RX, BT_TX);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp_probe(&oneWire);

int16_t acc[3], acc_avg[3] = {0};
uint16_t acc_norm;

char msg;

uint32_t last_read = 0;
int8_t last_temp = 0;
uint8_t last_co2 = 2.5;
uint32_t last_mov = 0; // 4H-6M-6S
char buffer_out[4] = {};

void setup()
{
  /* Bluetooth init. */
  BTserial.begin(9600);

  /* Temperature probe init. */
  temp_probe.begin();

  /* MPU init. */
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0); // Wake up MPU
  Wire.endTransmission(true);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_O, OUTPUT);
  pinMode(LED_G, OUTPUT);
  digitalWrite(LED_O, HIGH);

  #ifdef DEBUG
    Serial.begin(9600);
  #endif
}

void loop()
{
  if ( (millis() - last_read) > POLL_PERIOD)
  {
    #ifdef DEBUG
      Serial.println("Reading sensors...");
    #endif
    read_temp();
    read_co2();
    read_mov();
  }

  checkMaster();
  delay(500);
}


void checkMaster()
{
  if (BTserial.available())
  {
    msg = BTserial.read();

    #ifdef DEBUG
      Serial.print("Received new message: ");
      Serial.println(msg);
    #endif
    
    switch (msg)
    {
      case REQ_READ:
        #ifdef DEBUG
          Serial.println("Received: read request.");
        #endif
        buffer_out[0] = (char)last_temp;
        buffer_out[1] = (char)last_co2;
        uint16_t last_mov_dur;
        last_mov_dur = encode_to_time(last_mov);
        buffer_out[2] = (char)(((last_mov_dur >> 8) & 0xFF) | 0x80);
        buffer_out[3] = (char)(last_mov_dur & 0xFF);

        uint8_t i;
        for(i = 0; i < 4; i++) {
          BTserial.write(buffer_out[i]); 
        }
        break;
        
      case REQ_WRITE:
        #ifdef DEBUG
          Serial.println("Received: write request.");
        #endif
        msg = BTserial.read();
        digitalWrite(LED_R, msg & 1);
        digitalWrite(LED_O, (msg >> 1) & 1);
        digitalWrite(LED_G, (msg >> 2) & 1);
        break;
    }
  }
}

void read_temp()
{
  temp_probe.requestTemperatures();
  last_temp = temp_probe.getTempCByIndex(0);
  #ifdef DEBUG
    Serial.print("--> Temperature: ");
    Serial.println(last_temp);
  #endif
}

void read_co2()
{
  #ifdef DEBUG
    Serial.print("--> Last Co2: ");
    Serial.println(last_co2);
  #endif
}

void read_mov()
{
  uint8_t i;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true); // request a total of 6 registers
  acc[0] = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  acc[1] = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  acc[2] = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  acc_norm = 0;
  for (i = 0; i < 3; i++) {
    acc_avg[i] = ALPHA * acc_avg[i] + (1 - ALPHA) * acc[i];
    acc_norm = acc_norm + ((uint32_t)pow(acc[i] - acc_avg[i], 2) >> SHIFT_COEF);
  }

  #ifdef DEBUG
    Serial.print("--> Movement norm: ");
    Serial.println(acc_norm);
  #endif

  if (acc_norm > MOV_THRESHOLD) {
    last_mov = millis();
  }
  #ifdef DEBUG
    uint16_t last_mov_duration = encode_to_time(last_mov);
    Serial.print("--> Last movement: ");
    Serial.print((last_mov_duration >> 12) & 0xF); Serial.print(":");
    Serial.print((last_mov_duration >> 6) & 0x3F); Serial.print(":");
    Serial.println(last_mov_duration & 0x3F);
  #endif
}

uint16_t encode_to_time(uint32_t duration)
{
  uint16_t last_mov_duration;
  uint32_t current = millis() - last_mov;
  last_mov_duration = 0;
  last_mov_duration |= ((current / 1000) / 3600) << 12;       // set hours
  last_mov_duration |= (((current / 1000) % 3600) / 60) << 6; // set minutes
  last_mov_duration |= (current / 1000) % 60;                 // set seconds

  return last_mov_duration;
}

