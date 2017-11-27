#include "Arduino.h"
#include "TimeLord.h"
#include "elapsedMillis.h"
#include "GreenhouseLib_actuators.h"
#include "GreenhouseLib_timing.h"

#ifndef GreenhouseLib_h
#define GreenhouseLib_h

//Don't change these values unless you know what you are doing:
#define MAX_TIMEPOINTS 5
#define MAX_ROLLUPS 2
#define MAX_FANS 2
#define MAX_HEATERS 2

class Greenhouse
{
  public:
    Greenhouse(int timezone, float latitude, float longitude, byte timezones, byte rollups, byte fans, byte heaters);
    ~Greenhouse();
    void setNow(byte rightNow[6]);
    void solarCalculations();
    void startingParameters();

    TimeLord myLord;
    Timpoint timepoint[MAX_TIMPOINTS];
    Rollup rollup[MAX_ROLLUPS];
    Fan fan[MAX_FANS];
    Heater heater[MAX_HEATERS];
  
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
