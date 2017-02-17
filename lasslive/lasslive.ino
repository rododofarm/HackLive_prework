#include "live.h";
static int pm1,pm25,pm10,bme280_p,bme280_h;
static float bme280_t;
Adafruit_BME280 bme;

void console_print(const void *argument){
  while(1){
    Serial.println(String("BME280:") + bme280_t +" C " + bme280_h + " %" + bme280_p + " pa, PM: " + pm1 + " ," + pm25 + " ," + pm10 );
    delay(1000);
  }
}

void read_bme(const void *argument){
  while(1){
    bme280_t=bme.readTemperature();
    bme280_p=bme.readPressure();
    bme280_h=bme.readHumidity();
    delay(2000);
  }
}

void read_g3(const void *argument){
  while(1){
    uint8_t buf[32];
    uint8_t c = 0;
    int idx = 0;
    memset(buf, 0, 32);
    while (true) {
      while (c != 0x42) {
        while (!Serial1.available());
        c = Serial1.read();
      }
      while (!Serial1.available());
      c = Serial1.read();
      if (c == 0x4d) {
        buf[idx++] = 0x42;
        buf[idx++] = 0x4d;
        break;
      }
    }
    while (idx != 32) {
      while(!Serial1.available());
      buf[idx++] = Serial1.read();
    }
    pm1 = ( buf[10] << 8 ) | buf[11];
    pm25 = ( buf[12] << 8 ) | buf[13];
    pm10 = ( buf[14] << 8 ) | buf[15];
    delay(3000);
  }
}

void setup() {
  //create many thread
  Serial1.begin(9600);
  Serial.begin(38400);
  bme.begin();
  os_thread_create(read_g3, NULL, OS_PRIORITY_NORMAL, 1024);
  os_thread_create(read_bme, NULL, OS_PRIORITY_NORMAL, 1024);
  os_thread_create(console_print, NULL, OS_PRIORITY_NORMAL, 1024);
}


























void loop() {}
// 你只能寫到 87行不能再多了 ....
//====No Code==== 



