#ifndef LIVETOOL
#define LIVETOOL

#define analogRead(x) 0
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "wiring_watchdog.h"
#include <WiFi.h>
#include <BlynkSimpleWifi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
SoftwareSerial Serial1(17, 5); // RX, TX
#endif
