// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.

#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"

#ifndef GreenhouseLib_h
#define GreenhouseLib_h

#define TIMEZONE_INDEX 0
#define RAMPING 49
#define ROLLUP_INDEX 50
#define FAN_INDEX 80
#define HEATER_INDEX 100

#define TYPE_INDEX 0
#define MOD_INDEX 1
#define HOUR_INDEX 2
#define MIN_INDEX 3
#define TARGET_INDEX 4

#define TEMP_INDEX 0
#define HYST_INDEX 1
#define SAFETY_INDEX 2
#define ROTATION_UP_INDEX 3
#define ROTATION_DOWN_INDEX 4
#define INCREMENTS_INDEX 5
#define PAUSE_INDEX 6

#define VAR_TEMP 0
#define FIX_TEMP 1

#define HEURE 2
#define MINUTE 1
#define SR 0
#define CLOCK 1
#define SS 2

#define CLOSE LOW
#define OPEN HIGH
#define ON HIGH
#define OFF LOW

/*
//A counting template...

template <typename T>
struct counter{
    counter(){count++;}
    virtual ~counter(){--count;}
    int count;
};

template <typename T> int counter<T>::count( 0 );
*/
//****************************************************************
//*******************TIMEZONES FUNCTIONS*****************************
//****************************************************************

void defineRamping(byte ramping);
unsigned long rampingE();


class Timezone
{
	public:
	
	Timezone();
    ~Timezone();
	void setParameters(byte type, short mod, float targetTemp);
	void setParameters(byte type, byte hour, byte min, float targetTemp);	
	void setTime(short mod);	
	void setTime(byte hour, byte min);
	void setTemp(float targetTemp);
	void setType(byte type);
	void setMod(short mod);

	void loadEEPROMParameters();
	void setParametersInEEPROM(byte type, short mod, float targetTemp);
	void setParametersInEEPROM(byte type, byte hour, byte min, float targetTemp);	
	void setTimeInEEPROM(byte type, short mod);	
	void setTimeInEEPROM(byte type, byte hour, byte min);
	void setTempInEEPROM(float targetTemp);
	void setTypeInEEPROM(byte type);
	void setModInEEPROM(short mod);
	
	
   void EEPROMUpdate();

	unsigned short _type;
	short _mod;
	unsigned short _hour;
	unsigned short _min; 
	float _targetTemp;
	static short sunRise[3];
	static short sunSet[3];
	
	
	private:
   unsigned short _localIndex;
   static unsigned short _index;
   unsigned long _previousMillis;
   static const unsigned long _interval;
};

/*
OBJECT : Rollup
Parameters :
- activation temperature
- hysteresis
- time of rotation for full opening (max. 250 sec.)
- time of rotation for a full closing (max. 250 sec.)
- number of increments (max. 99)
- pause time between increments (max. 250 sec.)
- safety mode (flag for further conditions)
*/
class Rollup
{
  public:
    //initialization functions
    Rollup();
    ~Rollup();
    void initOutputs(byte mode, byte rOpen, byte rClose);

    //action functions
    void routine(float temp);
    void routine(float target, float mod);

    void openCompletely();
    void closeCompletely();
    void desactivateRoutine();
    void activateRoutine();

    //programmation functions
    void setParameters(float temp, float hyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety);
    void setHyst(float hyst);
    void setTemp(float temp);
    void setRotationUp(unsigned short rotation);
    void setRotationDown(unsigned short rotation);
    void setIncrements(unsigned short increments);
    void setPause(unsigned short pause);
    void setSafety(boolean safety);
    
    void setIncrementCounter(unsigned short increment);

    void setParametersInEEPROM(unsigned short temp, byte hyst, byte rotationUp, byte rotationDown, byte increments, byte pause, boolean safety);
    void loadEEPROMParameters();
    void setTempInEEPROM(unsigned short temp);
    void setHystInEEPROM(byte rHyst);
    void setRotationUpInEEPROM(byte rotationUp);
    void setRotationDownInEEPROM(byte rotationDown);
    void setIncrementsInEEPROM(byte increments);
    void setPauseInEEPROM(byte pause);
    void setSafetyInEEPROM(boolean safety);

    void EEPROMUpdate();

	 elapsedMillis rollupTimer;
    float _hyst;
    float _tempParameter;
    unsigned short _rotationUp;
    unsigned short _rotationDown;
    unsigned short _increments;
    unsigned short _pause;
    boolean _safety;
    unsigned short _incrementCounter;
    boolean _debug;
    boolean _mode;

  private:
  
    byte _openingPin;
    byte _closingPin;
    boolean _routine;
    boolean _opening;
    boolean _closing;
    boolean _openingCycle;
    boolean _closingCycle;

    unsigned short _localIndex;
    static unsigned short _index;

    void openSides();
    void closingSides();
    void startOpening();
    void stopOpening();
    void startClosing();
    void stopClosing();

    unsigned long _previousMillis;
    static const unsigned long _interval;
        
   boolean _lastAction;

};
/*
OBJECT : Fan (cooling unit)

Parameters :
- activation temperature
- hysteresis
- safety mode
*/
class Fan
{
  public:

      //initialization functions
      Fan();
      ~Fan();
      void initOutput(byte pin);

      //action functions
      void routine(float temp);
      void stop();
      void start();
      void desactivate();
      void activate();

      //programmation functions
      void setParameters(float activationTemp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setTemp(float temp);
      void setSafety(boolean safety);

      void setParametersInEEPROM(byte temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(byte temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);
/*
            //display
      byte getPin();
      float getHyst();
      float getTemp();
      boolean getSafety();
*/
  //private:
      //variables privées
      byte _pin;
      float _hyst;
      float _tempParameter;
      boolean _safety;

    private:
      boolean _active;
      unsigned short _localIndex;
      static unsigned short _index;
};

/*
OBJECT : Heater (heating unit)

Parameters :
- hysteresis
- activation temperature
- safety mode
*/

class Heater
{
  public:
    //initialization functions
      Heater();
      ~Heater();
      void initOutput(byte pin);

      //action functions
      void routine(float temp);
      void stop();
      void start();
      void desactivate();
      void activate();

          //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setTemp(float temp);
      void setSafety(boolean safety);

      void setParametersInEEPROM(byte temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(byte temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);
/*
      //display
      byte getPin();
      float getHyst();
      float getTemp();
      boolean getSafety();
*/
  //private:
      //variables privées
      byte _pin;
      float _hyst;
      float _tempParameter;
      boolean _safety;
    private:
      boolean _active;
      unsigned short _localIndex;
      static unsigned short _index;
};

byte negativeToByte(int data, byte mod);
int byteToNegative(int data, byte mod);
void convertDecimalToTime(int * heure, int * minut);


#endif
