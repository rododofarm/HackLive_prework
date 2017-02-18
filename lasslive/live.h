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
#include <GTimer.h>

char ssid[] = "Microwind_TWN";      // your network SSID (name)
char pass[] = "0919734011";     // your network password

char gps_lat[] = "24.7805647";  // device's gps latitude
char gps_lon[] = "120.9933177"; // device's gps longitude
char server[] = "gpssensor.ddns.net"; // the MQTT server of LASS
char clientId[16] = "";
char outTopic[32] = "";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

WiFiUDP Udp;
const char ntpServer[] = "123.204.45.116";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
const byte nptSendPacket[ NTP_PACKET_SIZE] = {
  0xE3, 0x00, 0x06, 0xEC, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x31, 0x4E, 0x31, 0x34,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
byte ntpRecvBuffer[ NTP_PACKET_SIZE ];

#define LEAP_YEAR(Y)     ( ((Y)>0) && !((Y)%4) && ( ((Y)%100) || !((Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
uint32_t epochSystem = 0; // timestamp of system boot up


// send an NTP request to the time server at the given address
void retrieveNtpTime() {
  bool hastime = 0;
  Serial.println("Send NTP packet");
  while(!hastime){
    Udp.beginPacket(ntpServer, 123); //NTP requests are to port 123
    Udp.write(nptSendPacket, NTP_PACKET_SIZE);
    Udp.endPacket();
    
    if(Udp.parsePacket()) {
      hastime = true;
      Serial.println("NTP packet received");
      Udp.read(ntpRecvBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      
      unsigned long highWord = word(ntpRecvBuffer[40], ntpRecvBuffer[41]);
      unsigned long lowWord = word(ntpRecvBuffer[42], ntpRecvBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
  
      epochSystem = epoch - millis() / 1000;
    }
  }
}

void getCurrentTime(unsigned long epoch, int *year, int *month, int *day, int *hour, int *minute, int *second) {
  int tempDay = 0;

  *hour = (epoch  % 86400L) / 3600;
  *minute = (epoch  % 3600) / 60;
  *second = epoch % 60;

  *year = 1970; // epoch starts from 1970
  *month = 0;
  *day = epoch / 86400;

  for (*year = 1970; ; (*year)++) {
    tempDay += (LEAP_YEAR(*year) ? 366 : 365);
    if (tempDay > *day) {
      tempDay -= (LEAP_YEAR(*year) ? 366 : 365);
      break;
    }
  }
  tempDay = *day - tempDay; // the days left in a year
  for ((*month) = 0; (*month) < 12; (*month)++) {
    if ((*month) == 1) {
      tempDay -= (LEAP_YEAR(*year) ? 29 : 28);
      if (tempDay < 0) {
        tempDay += (LEAP_YEAR(*year) ? 29 : 28);
        break;
      }
    } else {
      tempDay -= monthDays[(*month)];
      if (tempDay < 0) {
        tempDay += monthDays[(*month)];
        break;
      }
    }
  }
  *day = tempDay+1; // one for base 1, one for current day
  (*month)++;
}

void initializeWiFi() {

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    delay(5000);
    if(status == WL_CONNECTED){
      Udp.begin(2390);
      byte mac[6];
      WiFi.macAddress(mac);
      memset(clientId, 0, 16);
      sprintf(clientId, "FT1_LIVE%02X%02X", mac[4], mac[5]);
      sprintf(outTopic, "LASS/Test/PM25/live");
    }
  }

  while ((!client.connected()) && status == WL_CONNECTED) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
    }
    delay(100);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {

}

void initializeMQTT() {
  client.setServer(server, 1883);
  client.setCallback(callback);
}

#endif
