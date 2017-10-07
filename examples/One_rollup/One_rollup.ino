//Libraries
#include "Arduino.h"
#include "GreenhouseLib.h"
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

//Declaring rollup object
Rollup rollup1; //Pin 6 => opening relay; pin 7 => closing relay

void setup() {
  sensors.begin();  //start communication with temp probe
  rollup1.initOutputs(FIX_TEMP, OPENING_PIN, CLOSING_PIN);
  rollup1.setParameters(23, 1, 25, 25, 5, 5, true); //Set parameters as follow...

    //Activation temperature : 23C
    //hysteresis : 1C
    //Rotation time (Up): 25 sec
    //Rotation time (Down): 25 sec
    //Increments : 5
    //Pause between rotation : 5
    //Safety mode : ON

}

void loop() {

  sensors.requestTemperatures();  //ask for temperature
  greenhouseTemperature = sensors.getTempCByIndex(0); //store value
  rollup1.routine(greenhouseTemperature); //decide what to do

}
