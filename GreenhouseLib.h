// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.


//Debug lines
#define DEBUG_ROLLUP_TIMING
//#define DEBUG_ROLLUP_TEMP

#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"

#ifndef GreenhouseLib_h
#define GreenhouseLib_h




#define RAMPING 49
#define ROLLUP_INDEX 50
#define TIMEZONE_INDEX 0
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
#define MAN_TEMP 2

#define HEURE 2
#define MINUTE 1
#define SR 0
#define CLOCK 1
#define SS 2

#define CLOSE LOW
#define OPEN HIGH
#define ON HIGH
#define OFF LOW

#define ACT_HIGH 1
#define ACT_LOW 0

#define OFF_VAL 255
#define SAFETY_DELAY 1800000
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


/*
OBJECT : Override
Start and end of the override can be relative to sunrise or sunset (must be define in the main), or set manually
Parameters :
- type(SR, SS, CLOCK)
- modBegin/modEnd (only in SR/SS mode, between -60 min and +60 min relative to sunrise/sunset)
- hourBegin/minBegin/hourEnd/minEnd (only in CLOCK mode)
- action (indicate which function to run, the function is written outside the class)
*/
/*
class Override
{
	public:
	Override();
	~Override();
	void setParameters(byte type, short modBegin, short modEnd, byte action);
	void setParameters(byte type, byte hourBegin, byte minBegin, byte hourEnd, byte minEnd, byte action);
	void setTimeBegin(short modBegin);
	void setTimeBegin(byte hour, byte min);
	void setTimeEnd(short modEnd);
	void setTimeBegin(byte hour, byte min);
	void setAction(byte action);

	void loadEEPROMParameters();
	void setParametersInEEPROM(byte type, short modBegin, short modEnd, byte action);
	void setParametersInEEPROM(byte type, byte hourBegin, byte minBegin, byte hourEnd, byte minEnd, byte action);
	void setTimeBeginInEEPROM(short modBegin);
	void setTimeBeginInEEPROM(byte hour, byte min);
	void setTimeEndInEEPROM(short modEnd);
	void setTimeBeginInEEPROM(byte hour, byte min);
	void setActionInEEPROM(byte action);

   void EEPROMUpdate();

	unsigned short _type;
	short _modBegin;
	short _modEnd;
	unsigned short _hourBegin;
	unsigned short _minBegin;
	unsigned short _hourEnd;
	unsigned short _minEnd;
	float _targetTemp;

	static short sunRise[3];
	static short sunSet[3];


	private:
   	unsigned short _localIndex;
   	static unsigned short _index;
   	unsigned long _previousMillis;
   	static const unsigned long _interval;
};*/
/*
OBJECT : Timezone
Start time can be relative to sunrise or sunset (must be define in the main), or set manually
Parameters :
- type(SR, SS, CLOCK)
- mod (only in SR/SS mode, between -60 min and +60 min relative to sunrise/sunset)
- hour/min (only in CLOCK mode)
- targetTemp (temperature to reach during this timezone)
*/

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

	elapsedMillis EEPROMTimer;
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
    void initOutputs(byte mode, boolean relayType, byte rOpen, byte rClose);
		void initStage(byte stage, float mod, byte inc);

    //action functions
    void routine(float temp);												//Mode FIX_TEMP
    void routine(float target, float temp);					//Mode VAR_TEMP
		void manualRoutine(float target, float temp);		//Mode MAN_TEMP
    void startOpening();
    void stopOpening();
    void startClosing();
    void stopClosing();
		void openToInc(byte stage, byte targetIncrement);
		void closeToInc(byte stage, byte targetIncrement);
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
	  elapsedMillis EEPROMTimer;
		elapsedMillis safetyTimer;

    float _hyst;
    float _tempParameter;
    unsigned short _rotationUp;
    unsigned short _rotationDown;
    unsigned short _increments;
    unsigned short _pause;
    boolean _safety;
    unsigned short _incrementCounter;
    byte _mode;

  private:

    byte _openingPin;
    byte _closingPin;
    boolean _relayType;
    boolean _routine;
    boolean _opening;
    boolean _closing;
    boolean _openingCycle;
    boolean _closingCycle;
		boolean _safetyCycle;
		unsigned long _rotationCycle;
		unsigned long _pauseCycle;
		short _move;
		short _stage;
		byte _stages;
		byte _stageInc[5];
		float _stageMod[5];

    unsigned short _localIndex;
    static unsigned short _index;

    void openSides();
    void closingSides();
		void safetyCycle();
		void safetyOpen();
		void safetyClose();
		void printPause();
		void printEndPause();

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
      void initOutput(byte mode, byte pin);

      //action functions
      void routine(float temp);
      void routine(float target, float temp);
      void stop();
      void start();
      void desactivateRoutine();
      void activateRoutine();

      //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setTemp(float temp);
      void setSafety(boolean safety);

      void setParametersInEEPROM(byte temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(unsigned short temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);

		void EEPROMUpdate();

		byte _mode;
      byte _pin;
      float _hyst;
      float _tempParameter;
      boolean _safety;
		boolean _debug;

    private:
      boolean _routine;
      unsigned short _localIndex;
      static unsigned short _index;
		elapsedMillis EEPROMTimer;
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
      void initOutput(byte mode, byte pin);

      //action functions
      void routine(float temp);
      void routine(float target, float temp);
      void stop();
      void start();
      void desactivateRoutine();
      void activateRoutine();

          //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setTemp(float temp);
      void setSafety(boolean safety);

      void setParametersInEEPROM(byte temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(unsigned short temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);

		void EEPROMUpdate();

      byte _mode;
      byte _pin;
      float _hyst;
      float _tempParameter;
      boolean _safety;
		boolean _debug;

    private:
      boolean _routine;
      unsigned short _localIndex;
      static unsigned short _index;
		elapsedMillis EEPROMTimer;
};

byte negativeToByte(int data, byte mod);
int byteToNegative(int data, byte mod);
void convertDecimalToTime(int * heure, int * minut);


#endif
