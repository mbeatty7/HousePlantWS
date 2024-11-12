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

AirQualitySensor sensor(A2);

const int OLED_RESET = -1;
const int moisturePin = A1;
const int Desert;
const int Moist;
const int Flood;
const int hexAddress = 0x76;

const char degree = 248;

int status;

float temp;



Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;

String getMoistureStatus(int moisture);

SYSTEM_MODE(AUTOMATIC);


SYSTEM_THREAD(ENABLED);


SerialLogHandler logHandler(LOG_LEVEL_INFO);


void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 10000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1,0);
  display.setCursor(0,0);
  display.display();

  status = bme.begin(hexAddress);
  if(status == false){
    Serial.printf("BME280 at address 0x%02x failed to start \n", hexAddress);
  }

  pinMode(moisturePin, INPUT);
}


void loop() {
 int moistureLevel = analogRead(moisturePin);
 String moistureStatus = getMoistureStatus(moistureLevel);
 String currentTime = Time.format(Time.now(), "%H:%M:%S");

 display.print("Time: ");
 display.printf(currentTime);
 display.print("Moisture: ");
 display.printf(String(moistureLevel));
 display.display();
 delay(100);

 //BME 
temp = bme.readTemperature();

  Serial.printf("temp = %0.1f \n", temp);
  display.printf("temp = %0.1f \n", temp);
  display.display();


// Air Quality Sensor
int quality = sensor.slope();

Serial.print("Sensor value: ");
Serial.printf(sensor.getValue());

if(quality == AirQualitySensor::FORCE_SIGNAL) {
  Serial.printf("I CAN'T BREATHE D: \n");
  display.printf("I CAN'T BREATHE D: \n");
  display.display();
}
if(quality == AirQualitySensor::HIGH_POLLUTION){
  Serial.printf("I could use some oxygen rn :/ \n");
  display.printf("I could use some oxygen rn :/ \n");
  display.display();
}
if(quality == AirQualitySensor::LOW_POLLUTION){
  Serial.printf("I could be breathing better :| \n");
  display.printf("I could be breating better :| \n");
  display.display();
}
if(quality == AirQualitySensor::FRESH_AIR){
  Serial.printf("*INHALE EXHALE* Ahhhhh \n");
  display.printf("*INHALE EXHALE* Ahhhhh \n");
  display.display();
}


}

String getMoistureStatus(int moisture) {
  if(moisture < Desert) {
    return "Desert";
  }
  if(moisture < Moist) {
    return "Moist";
  }
  else{
    return "Flood";
  }
}