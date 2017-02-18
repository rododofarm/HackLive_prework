//volatile static int pm25, pm10, bme280_p, bme280_h;
static float bme280_t;
uint32_t sema;
#include "live.h";
Adafruit_BME280 bme;// I2C
#include <SoftwareSerial.h>
SoftwareSerial Serial1(0, 1); // RX, TX

#define pmsDataLen 32
uint8_t buf[pmsDataLen];
int idx = 0;
int pm10 = 0;
int pm25 = 0;
int pm100 = 0;
float bme280_p = 0;
int bme280_h = 0;

void read_g3() {
  uint8_t c = 0;
  idx = 0;
  memset(buf, 0, pmsDataLen);

  while (true) {
    while (c != 0x42) {
      while (!Serial1.available());
      c = Serial1.read();
    }
    while (!Serial1.available());
    c = Serial1.read();
    if (c == 0x4d) {
      // now we got a correct header)
      buf[idx++] = 0x42;
      buf[idx++] = 0x4d;
      break;
    }
  }

  while (idx != pmsDataLen) {
    while (!Serial1.available());
    buf[idx++] = Serial1.read();
  }

  pm10 = ( buf[10] << 8 ) | buf[11];
  pm25 = ( buf[12] << 8 ) | buf[13];
  pm100 = ( buf[14] << 8 ) | buf[15];
}

void read_bme280(){
  bme280_t = bme.readTemperature();
  bme280_p = bme.readPressure();
  bme280_h = bme.readHumidity();
}

void sendMQTT() {
    char payload[300];
    Serial.println("Sending MQTT");
    unsigned long epoch = epochSystem + millis() / 1000;
    int year, month, day, hour, minute, second;
    getCurrentTime(epoch, &year, &month, &day, &hour, &minute, &second);
  
    if (client.connected()) {
        sprintf(payload, "|ver_format=3|FAKE_GPS=1|app=PM25|ver_app=%s|device_id=%s|date=%4d-%02d-%02d|time=%02d:%02d:%02d|s_d0=%d|s_d1=%d|s_t0=%d|s_h0=%d|gps_lon=%s|gps_lat=%s",
          "live",
          clientId,
          year, month, day,
          hour, minute, second,
          pm25,pm10,(int)bme280_t,(int)bme280_h,
          gps_lon, gps_lat
        );
        Serial.println(payload);
        // Once connected, publish an announcement...
        client.publish(outTopic, payload);
      }
}

void setup() {
  Serial.begin(38400);
  Serial1.begin(9600); // PMS 3003 UART has baud rate 9600
  //Serial.println(F("BME280 test"));
  initializeMQTT();
  initializeWiFi();
  retrieveNtpTime();


  //===add this section, let bme280 start to work===
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  //=======
  
  Serial.println("OK start");
}

void loop() {
  read_g3();
  read_bme280();
  sendMQTT();
  //Serial.println(String("BME280:") + bme280_t + " C " + bme280_h + " % " + bme280_p / 100 + " pa, PM: " + pm25 + " ," + pm10 + " ," + pm100);
  delay(5000);
  client.loop();
}
// 你只能寫到 87行不能再多了 ....
//====No Code====



