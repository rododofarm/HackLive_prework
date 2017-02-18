volatile static int pm25,pm10,bme280_p,bme280_h;
static float bme280_t;
uint32_t sema;
#include "live.h";
Adafruit_BME280 bme;// I2C
#include <SoftwareSerial.h>
SoftwareSerial Serial1(0, 1); // RX, TX
bool hasbme;

void read_sensor(const void *argument){
  while(1){
      os_semaphore_wait(sema,0xFFFFFFFF);
      Serial.println("[READ SENSOR]");
      if(hasbme){
        bme280_t=bme.readTemperature();
        bme280_p=bme.readPressure();
        bme280_h=bme.readHumidity();
      }
      wdt_reset();
      unsigned long timeout = millis();
      int count=0;
      byte incomeByte[24];
      boolean startcount=false;
      byte data;
      while (1){
        if((millis() -timeout) > 1000) {    
          break;
        }
        if(Serial1.available()){
          data=Serial1.read();
        if(data==0x42 && !startcount){
          startcount = true;
          count++;
          incomeByte[0]=data;
        }else if(startcount){
          count++;
          incomeByte[count-1]=data;
          if(count>=24) {break;}
         }
        }
      }
      unsigned int calcsum = 0; // BM
      unsigned int exptsum;
      for(int i = 0; i < 22; i++) {
        calcsum += (unsigned int)incomeByte[i];
      }
      exptsum = ((unsigned int)incomeByte[22] << 8) + (unsigned int)incomeByte[23];
      if(calcsum == exptsum) {
        pm25 = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];
        //PM10
        pm10 = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];
      }
      os_semaphore_release(sema);
      delay(3000);
   }
}

void sendMQTT(const void *argument) {
  // Loop until we're reconnected
  while(1){
    delay(15000);
    os_semaphore_wait(sema, 0xFFFFFFFF);
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
        client.publish("LASS/Test/PM25", payload);
      }
    os_semaphore_release(sema);
  }
}
void setup() {
  Serial.begin(38400);
  Serial1.begin(9600);
  initializeMQTT();
  initializeWiFi();
  retrieveNtpTime();
  if (!bme.begin()) {
    hasbme = 0 ;
  }else{
    hasbme = 1;
  }
  wdt_enable(8000);
  sema = os_semaphore_create(1);
  os_thread_create(read_sensor, NULL, OS_PRIORITY_HIGH, 2048);
  os_thread_create(sendMQTT, NULL, OS_PRIORITY_REALTIME, 2048);
}

void loop() {
  delay(1000);
  client.loop();
}
// 你只能寫到 87行不能再多了 ....
//====No Code==== 



