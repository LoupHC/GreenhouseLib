#include "Arduino.h"
#include "TimeLord.h"
#include "elapsedMillis.h"
#include "GreenhouseLib_actuators.h"
#include "GreenhouseLib_timing.h"

#ifndef GreenhouseLib_h
#define GreenhouseLib_h

/*
A Greenhouse object inputs two parameters, a time array (sec, min, hour, date, month, year) and a temperature value and output multiple controle actions over
rollup motors, fans and furnace. 
To work nicely, parameters for each timepoints and outputs must be initialized in the setup part of an arduino sketch. 
It calculates sunrise and sunset from geographical positions, and adjusts target temperature according to timepoints. 
*/

class Greenhouse
{
  public:
    Greenhouse(int timezone, float latitude, float longitude, byte timezones, byte rollups, byte fans, byte heaters);
    ~Greenhouse();
    //when are we? (automatic hour-saving adjustment, input winter time array)
    void setNow(byte rightNow[6]);
    //define sunrise, sunset
    void solarCalculations();
    //set actual time and cooling/heating temperatures
    void startingParameters();

    //Objects instances
    TimeLord myLord;
    Timezone timezone[5];
    Rollup rollup[2];
    Fan fan[2];
    Heater heater[2];
  
    //Confirm timepoint, cooling/heating temperatures, routine for each outputs
    void fullRoutine(byte rightNow[6], float greenhouseTemperature);
    //Update parameters in EEPROM for each instance, every 10 seconds
    void EEPROMUpdate();
 
    byte _rightNow[6];      //actual time
    byte _sunTime[6];       //for sunrise, sunset calculation
    float _heatingTemp;     //reference temperature for heating devices
    float _coolingTemp;     //reference temperature for cooling devices
    byte _timezone;         //time zone of your location (Most of Quebec : -5)
    float _longitude;
    float _latitude;
    unsigned long _ramping; //delay between .5C transition in heating/cooling temperature
    byte _timepoint;        //actual timepoint

  private:
    byte _timepoints;        //# of timepoints
    byte _rollups;           //# of rollups
    byte _fans;              //# of fans
    byte _heaters;           //# of heaters
    void initTimeLord(int timezone, float latitude, float longitude);
    void setSunrise();
    void setSunset();
    void selectActualProgram();
    void setTempCible();
    void startRamping();
  
    elapsedMillis ramping;  //ramping timer


};




#endif
