// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.




#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"

#ifndef GreenhouseLib_timing_h
#define GreenhouseLib_timing_h


#define TIMEZONE_INDEX 0
#define ROLLUP_INDEX 50
#define FAN_INDEX 80
#define HEATER_INDEX 100

#define TYPE_INDEX 0
#define MOD_INDEX 1
#define HOUR_INDEX 2
#define MIN_INDEX 3
#define HEAT_INDEX 4
#define COOL_INDEX 5

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
		void setParameters(byte type, short mod, float heatingTemp, float coolingTemp);
		void setParameters(byte type, byte hour, byte min, float heatingTemp, float coolingTemp);
		void setTime(short mod);
		void setTime(byte hour, byte min);
		void setHeatTemp(float heatingTemp);
		void setCoolTemp(float coolingTemp);
		void setType(byte type);
		void setMod(short mod);

		void loadEEPROMParameters();
		void setParametersInEEPROM(byte type, short mod, float heatingTemp, float coolingTemp);
		void setParametersInEEPROM(byte type, byte hour, byte min, float heatingTemp, float coolingTemp);
		void setTimeInEEPROM(byte type, short mod);
		void setTimeInEEPROM(byte type, byte hour, byte min);
		void setHeatTempInEEPROM(float heatingTemp);
		void setCoolTempInEEPROM(float coolingTemp);
		void setTypeInEEPROM(byte type);
		void setModInEEPROM(short mod);


   	void EEPROMUpdate();

 		unsigned short hr();
 		unsigned short mn();
 		float heatingTemp();
 		float coolingTemp();

		static short sunRise[3];
		static short sunSet[3];


	private:

		unsigned short _type;
		short _mod;
		unsigned short _hour;
		unsigned short _min;
		float _heatingTemp;
		float _coolingTemp;
  	unsigned short _localIndex;
  	static unsigned short _index;
		elapsedMillis EEPROMTimer;
};


//Generic macros
byte negativeToByte(int data, byte mod);
int byteToNegative(int data, byte mod);
void convertDecimalToTime(int * heure, int * minut);

#endif
