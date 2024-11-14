/* 
 * Project HousePlantWS
 * Author: Mamie-Jo Beatty
 * Date: 11/11/24
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */



#include "Particle.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Adafruit_BME280.h"
#include "Grove_Air_Quality_Sensor.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

//***** feeds *****//

Adafruit_MQTT_Publish tempFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish humidFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humid");
Adafruit_MQTT_Publish aqFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/aq");
Adafruit_MQTT_Publish moistureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisture");
Adafruit_MQTT_Subscribe subFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/waterButton"); 

AirQualitySensor sensor(A2);    // AQ SENSOR AT A2

unsigned int lastTime;
const int OLED_RESET = -1;
const int moisturePin = A1;    // CAPACITATOR AT A1
const int hexAddress = 0x76;  // BME ADDRESS

bool buttonState;

int PUMP = D16;
int moistureLevel;
int moisture;

const char degree = 248;

int status;
int count;

float temp;       
float humid;
float pressure;
float subValue,pubValue;

void MQTT_connect();
bool MQTT_ping();

Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;

String getMoistureStatus(int moisture);
String dateTime, timeOnly;

SYSTEM_MODE(AUTOMATIC);


SYSTEM_THREAD(ENABLED);


SerialLogHandler logHandler(LOG_LEVEL_INFO);


void setup() {

  Time.zone(-7);
  Particle.syncTime();

  pinMode(PUMP, OUTPUT);

// TURN ON WIFI AND CONNECT TO THE INTERNET // 
    WiFi.on();
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");

  // Setup MQTT subscription
  mqtt.subscribe(&subFeed);

  Serial.begin(9600);
  waitFor(Serial.isConnected, 10000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1,0);
  display.setRotation(3);
  display.setCursor(0,0);
  display.display();

  status = bme.begin(hexAddress);
  if(status == false){
    Serial.printf("BME280 at address 0x%02x failed to start \n", hexAddress);
  }

  pinMode(moisturePin, INPUT);
  mqtt.subscribe(&subFeed);
}


void loop() {

  MQTT_connect();
  MQTT_ping();

   Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &subFeed) {
      buttonState = atof((char *)subFeed.lastread);
      Serial.printf("Button state: %i\n", buttonState);
    }
  }

if(moisture > 3000) {
  digitalWrite(PUMP, HIGH);
  display.printf("WATERING ... \n");
  delay(30000);
  digitalWrite(PUMP,LOW);
  display.printf("QUENCHED...\n");
  delay(30000);
}


if (buttonState == 1)
{
  digitalWrite(PUMP, HIGH);
}
if (buttonState == 0)
{
  digitalWrite(PUMP, LOW);
}

   // CAPACITATOR //
   
 moistureLevel = analogRead(moisturePin);
 Serial.printf(String(moistureLevel));
 String moistureStatus = getMoistureStatus(moistureLevel);
 String currentTime = Time.format(Time.now(), "%H:%M:%S");
 //display.setCursor(0,0);
 display.print("Time: ");
 display.printf(currentTime);
 display.print("Moisture: ");
 display.printf(String(moistureLevel));
 display.display();

 dateTime = Time.timeStr();
 timeOnly = dateTime.substring(11,19);
 if(millis()-lastTime>10000) {
  lastTime = millis();

  Serial.printf("Date and time is %s\n", dateTime.c_str());
  Serial.printf("Time is %s\n", timeOnly.c_str());
 }

 // BME //
temp = bme.readTemperature();
humid = bme.readHumidity();
pressure = bme.readPressure();

  Serial.printf("temp = %0.1f, humid = %0.1f, press = %0.1f \n", temp, humid, pressure);
  //display.setCursor(0,10);
  display.printf("temp = %0.1f, humid = %0.1f, pressure = %0.1f \n", temp, humid, pressure);
  display.display();
  delay(100);

  
  if((millis()-lastTime > 10000)) {
    if(mqtt.Update()) {
      tempFeed.publish(temp);
      Serial.printf("Publishing %0.1f \n", temp); 
      humidFeed.publish(humid);
      Serial.printf("Publishing %0.1f \n", humid);
      moistureFeed.publish(moistureLevel);
      Serial.printf("Publishing %i\n", moistureLevel);
      //aqFeed.publish(sensor);
      } 
    lastTime = millis();
  }

// Air Quality Sensor //
int quality = sensor.slope();

Serial.print("Sensor value: ");
Serial.printf("%i\n", sensor.getValue());

if(quality == AirQualitySensor::FORCE_SIGNAL) {
  Serial.printf("I CAN'T BREATHE D: \n");
  //display.setCursor(0, 40);
  display.printf("I CAN'T BREATHE D: \n");
  display.display();
}
if(quality == AirQualitySensor::HIGH_POLLUTION){
  Serial.printf("I could use some oxygen rn :/ \n");
  //display.setCursor(0, 40);
  display.printf("I could use some oxygen rn :/ \n");
  display.display();
}
if(quality == AirQualitySensor::LOW_POLLUTION){
  Serial.printf("I could be breathing better :| \n");
  //display.setCursor(0, 40);
  display.printf("I could be breating better :| \n");
  display.display();
}
if(quality == AirQualitySensor::FRESH_AIR){
  Serial.printf("*INHALE EXHALE* Ahhhhh \n");
  //display.setCursor(0, 40);
  display.printf("*INHALE EXHALE* Ahhhhh \n");
  display.display();
}
//delay(100);

}

// READ MOISTURE LEVELS TO DETERMINE WHEN THE PLANT NEEDS WATER, USE FUNCTION IN LOOP
String getMoistureStatus(int moisture) {
  if(moisture > 3000) {
    return "Thirsty";
  }
  if(moisture <= 2999) {
    return "Thank you";
  }
  else{
    return "TOO MUCH AHHHHHH";
  }
}

void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}