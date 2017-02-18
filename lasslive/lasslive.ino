volatile static int pm25,pm10,bme280_p,bme280_h;
static float bme280_t;
uint32_t sema;
#include "live.h";
Adafruit_BME280 bme;// I2C
#include <SoftwareSerial.h>
SoftwareSerial Serial1(0, 1); // RX, TX
bool hasbme;

void console_print(const void *argument){
  while(1){
    os_semaphore_wait(sema,0xFFFFFFFF);
    os_semaphore_release(sema);
    os_thread_yield();
    delay(1000);
  }
}

void read_bme(const void *argument){
  while(1){
    os_semaphore_wait(sema,0xFFFFFFFF);
    bme280_t=bme.readTemperature();
    bme280_p=bme.readPressure();
    bme280_h=bme.readHumidity();
    os_semaphore_release(sema);
    wdt_reset();
    os_thread_yield();
    delay(2000);
  }
}

void read_g3(const void *argument){
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
          Serial.println("[G3-ERROR-TIMEOUT]");
          //#TODO:make device fail alarm message here
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
      } else {
        Serial.println("#[exception] PM2.5 Sensor CHECKSUM ERROR!");
      }
      os_semaphore_release(sema);
      delay(3000);
   }
}

void setup() {
  //create many thread
  Serial.begin(38400);
  Serial1.begin(9600);
  wdt_enable(8000);
  //hasbme = bme.begin();
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  hasbme = 1;
  sema = os_semaphore_create(1);
  os_thread_create(read_g3, NULL, OS_PRIORITY_HIGH, 4096);
  //os_thread_create(read_bme, NULL, OS_PRIORITY_REALTIME, 1024);
  //os_thread_create(console_print, NULL, OS_PRIORITY_HIGH, 1024);
}



void loop() {
  os_semaphore_wait(sema, 0xFFFFFFFF);
  Serial.println(String("BME280:") + bme280_t +" C " + bme280_h + " % " + bme280_p/100 + " pa, PM: " + pm25 + " ," + pm10 );
  os_semaphore_release(sema);
  delay(1000);
 }
// 你只能寫到 87行不能再多了 ....
//====No Code==== 



