//-------------- Test Complete 05-09-18----------------
#include <Arduino.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)      // Arduino UNO
    #include <AltSoftSerial.h>
    AltSoftSerial bc95serial;
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)   // Arduino MEGA2560
    #define bc95serial Serial1
#endif

#include "BC95Udp.h"
#include "MicrogearNB.h"
#include <avr/wdt.h>
#include <DHT.h>

#define APPID    "NBT1"
#define KEY      "AamSCkPSSsaIrNb"
#define SECRET   "Y09odqGd2hnupN9TCCm8BNAIm"
//#define KEY      "AamSCkPSSsaIrNb"
//#define SECRET   "Y09odqGd2hnupN9TCCm8BNAIm"

#define DHTPIN    7
#define DHTTYPE   DHT22
DHT dht(DHTPIN, DHTTYPE);

#define MAXBUFFSIZE 48
char payload[MAXBUFFSIZE];
long lastDHTRead = 0;
long count;
int i;
BC95UDP client;
Microgear mg(&client);

int hum_test=10;

void setup() {
    bc95serial.begin(9600);
    BC95.begin(bc95serial);
    BC95.reset();
    
    Serial.begin(9600);
    Serial.println(F("Microgear Arduino NB-IoT 24-09-19"));
    Serial.print(F("IMEI: "));
    Serial.println(BC95.getIMEI());
    Serial.print(F("IMSI: "));
    Serial.println(BC95.getIMSI());

    Serial.print(F("Attach Network..."));
    while (!BC95.attachNetwork()) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(F("\nNB-IOT attached!"));
    Serial.print(F("RSSI: "));
    Serial.println(BC95.getSignalStrength());
    Serial.print(F("IPAddress: "));
    Serial.println(BC95.getIPAddress());

    mg.init(APPID, KEY, SECRET);
    mg.begin(5555);
     count = 0;
     i = 5;
     wdt_enable(WDTO_4S); 
}

void loop() {
  
  // เซนเซอร์​ DHT อ่านถี่เกินไปไม่ได้ จะให้ค่า error
  // จึงต้องเช็คเวลาครั้งสุดท้ายที่อ่านค่าว่าทิ้งช่วงนานพอหรือยัง ในที่นี้ตั้งไว้ 1 วินาที
  if(millis() - lastDHTRead > 10000){
    wdt_reset();
    count++;
    float humid = random(20,100);//dht.readHumidity();     // อ่านค่าความชื้น
    float temp  = (float)hum_test;//random(30,40);//dht.readTemperature();  // อ่านค่าอุณหภูมิ
    hum_test = hum_test+1;
    if (hum_test >= 100)
    hum_test  = 0;
    lastDHTRead = millis();
    wdt_reset();
    Serial.print("Humid: "); Serial.print(humid); Serial.print(" %, ");
    Serial.print("Temp: "); Serial.print(temp); Serial.println(" C ");
    // ตรวจสอบค่า humid และ temp เป็นตัวเลขหรือไม่
    if (isnan(humid) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else{
      // เตรียมสตริงในรูปแบบ "humid,temp"
      float ssi =  BC95.getSignalStrength();
      //String datastring = (String)humid + "," + (String)temp;
      String datastring = (String)humid + "," + (String)temp + "," + (String)count + ","+(String)ssi;      
      Serial.print(F("Sending 1 --> "));
      wdt_reset();
      Serial.println(datastring);      
      datastring.toCharArray(payload, MAXBUFFSIZE-1); // wdt_reset();
      mg.publish("/nbiot/sensor", payload);
      Serial.println("end ");
     // Serial.println(datastring);    
    }      
        wdt_reset();
        delay(1000);
   //      wdt_reset();
      /*  String mapstring = "1402.4540,10036.8730";        
        Serial.println("Send Lat,Long -->");
        Serial.println(mapstring);
        mapstring.toCharArray(payload, MAXBUFFSIZE-1);
        wdt_reset();
        mg.publish("/nbiot/gps",payload);*/
      i = 0;

  }
   wdt_reset();
  mg.loop();
}
