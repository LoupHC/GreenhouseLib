#include "Arduino.h"
#include "TimeLord.h"
#include "elapsedMillis.h"
#include "GreenhouseLib_actuators.h"
#include "GreenhouseLib_timing.h"

#ifndef GreenhouseLib_h
#define GreenhouseLib_h


class Greenhouse
{
  public:
    Greenhouse(int timezone, float latitude, float longitude, byte timezones, byte rollups, byte fans, byte heaters);
    ~Greenhouse();
    void setNow(byte rightNow[6]);
    void solarCalculations();
    void startingParameters();

    TimeLord myLord;
    Timezone timezone[5];
    Rollup rollup[2];
    Fan fan[2];
    Heater heater[2];
    elapsedMillis ramping;

    void fullRoutine(byte rightNow[6], float greenhouseTemperature);
    void EEPROMUpdate();

    byte _rightNow[6];
    byte _sunTime[6];
    float _heatingTemp;
    float _coolingTemp;
    byte _timezone;
    float _longitude;
    float _latitude;
    unsigned long _ramping;
    byte _program;

  private:
    byte _timezones;
    byte _rollups;
    byte _fans;
    byte _heaters;
    void getDateAndTime();
    void initTimeLord(int timezone, float latitude, float longitude);
    void setSunrise();
    void setSunset();
    void selectActualProgram();
    void setTempCible();
    void startRamping();


};




#endif
