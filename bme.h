#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME280.h"

Adafruit_BME280 bme;
uint8_t bme_addres = 0x76;

struct bme_struct{
   float temperature;
   float humidity;
   float pressure;
} bme_data;

bool initBME(){
  if (!bme.begin(bme_addres)) 
  {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    return false;
  }
  return true;
}

bme_struct * get_bme_data(){
  bme_data.temperature = bme.readTemperature();
  bme_data.pressure = bme.readPressure() / 100.0;
  bme_data.humidity = bme.readHumidity();
  return &bme_data;
}

void print_data(bme_struct * bme280){
    Serial.print("Temperature = ");
    Serial.print(bme280->temperature);
    Serial.println(F(" *C"));
    
    Serial.print("Pressure = ");
    Serial.print(bme280->pressure);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme280->humidity);
    Serial.println(" %");
}
