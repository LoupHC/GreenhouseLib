// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.




#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"
#include "Parameters.h"
#include "Defines.h"


#ifndef GreenhouseLib_timing_h
#define GreenhouseLib_timing_h



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
OBJECT : Timepoint
Start time can be relative to sunrise or sunset (must be define in the main), or set manually
Parameters :
- type(SR, SS, CLOCK)
- mod (only in SR/SS mode, between -60 min and +60 min relative to sunrise/sunset)
- hour/min (only in CLOCK mode)
- targetTemp (temperature to reach during this timezone)
*/

class Timepoint
{
	public:

		Timepoint();
    ~Timepoint();
		void setParameters(byte type, short hour, short min, float heatingTemp, float coolingTemp);
		void setTime(short mod);
		void setTime(short hour, short min);
		void setHeatTemp(float heatingTemp);
		void setCoolTemp(float coolingTemp);
		void setType(byte type);
/*
		void loadEEPROMParameters();
		void setParametersInEEPROM(byte type, short hour, short min, float heatingTemp, float coolingTemp);
		void setTimeInEEPROM(byte type, short hour, short min);
		void setHeatTempInEEPROM(float heatingTemp);
		void setCoolTempInEEPROM(float coolingTemp);
		void setTypeInEEPROM(byte type);
		void setModInEEPROM(short mod);

   	void EEPROMUpdate();
*/
		unsigned short hr();
 		unsigned short mn();
 		float heatingTemp();
 		float coolingTemp();

		static short sunRise[3];
		static short sunSet[3];


	private:

		byteParameter _type;
		shortParameter _hr;
		shortParameter _mn;
		floatParameter _heatingTemp;
		floatParameter _coolingTemp;

  	unsigned short _localIndex;
  	static unsigned short _index;
		elapsedMillis EEPROMTimer;
};



#endif
