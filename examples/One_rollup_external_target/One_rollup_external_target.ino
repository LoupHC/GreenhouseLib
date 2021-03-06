//Libraries
#include "Arduino.h"
#include "GreenhouseLib_rollups.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//Pinout
#define OPENING_PIN 6 //connect this pin to the opening relay
#define CLOSING_PIN 7 //connect this pin to the closing relay
#define ONE_WIRE_BUS A1 //connect this pin to the DS18B20 data line

//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float greenhouseTemperature;  //store temperature value
float targetTemperature = 20; //target temperature

//Declaring rollup object
Rollup rollup1;

//Declaring target temperature

void setup() {
  Serial.begin(9600);
  sensors.begin();  //start communication with temp probe
  rollup1.initRollup(0);
  rollup1.initOutputs(VAR_TEMP, ACT_HIGH, OPENING_PIN, CLOSING_PIN);
  rollup1.setParameters(-1, 1, 25, 25, 5, 5, true); //Set parameters as follow...

    //Temperature mod : -1C (adjust to external target temperature)
    //hysteresis : 1C (open at : targetTemp+mod ; close at : targetTemp+mod-hyst)
    //Rotation time (Up): 25 sec (for full opening) (0 to 255 seconds)
    //Rotation time (Down): 25 sec (for full closing) (0 to 255 seconds)
    //Increments : 5
    //Pause between rotation : 5 (0 to 255 seconds)
    //Safety mode : ON (safety opening cycle every 30 min even if considered fully open)


}

void loop() {

  sensors.requestTemperatures();  //ask for temperature
  greenhouseTemperature = sensors.getTempCByIndex(0); //store value
  rollup1.routine(targetTemperature, greenhouseTemperature); //decide what to do

}
