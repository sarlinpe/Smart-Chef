 /***************************************************************************************
 *
 * Copyright (c) 2016, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Smart Chef Project
 * File:        Test_DallasTemperature.ino
 * Date:        2016-10-23
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Smart-Chef
 *
 * Description: Test of the Dallas Temperature sensor using the OneWire library.
 *
 ***************************************************************************************/

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 7
 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
 
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature");

  // Start up the library
  sensors.begin();
}
 
 
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature for Device 1 is: ");
  Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"? 
    // You can have more than one IC on the same bus. 
    // 0 refers to the first IC on the wire

    delay(1000);
}
