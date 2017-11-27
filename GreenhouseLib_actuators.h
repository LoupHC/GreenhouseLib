// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.




#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"

#ifndef GreenhouseLib_actuators_h
#define GreenhouseLib_actuators_h


//Debug lines
//#define DEBUG_ROLLUP_TIMING
//#define DEBUG_ROLLUP_TEMP
//#define DEBUG_FAN
//#define DEBUG_HEATER
#define DEBUG_EEPROM


#define ROLLUP_INDEX 50
#define TIMEZONE_INDEX 0
#define FAN_INDEX 80
#define HEATER_INDEX 100

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
		void openToInc(short stage, byte targetIncrement);
		void closeToInc(short stage, byte targetIncrement);
		void desactivateRoutine();
    void activateRoutine();
		void setIncrementCounter(unsigned short increment);

    //Parameters functions
    void setParameters(float temp, float hyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety);
    void setHyst(float hyst);
    void setTemp(float temp);
    void setRotationUp(unsigned short rotation);
    void setRotationDown(unsigned short rotation);
    void setIncrements(unsigned short increments);
    void setPause(unsigned short pause);
    void setSafety(boolean safety);


		//EEPROM management functions
    void setParametersInEEPROM(short temp, byte hyst, byte rotationUp, byte rotationDown, byte increments, byte pause, boolean safety);
    void loadEEPROMParameters();
    void setTempInEEPROM(short temp);
    void setHystInEEPROM(byte rHyst);
    void setRotationUpInEEPROM(byte rotationUp);
    void setRotationDownInEEPROM(byte rotationDown);
    void setIncrementsInEEPROM(byte increments);
    void setPauseInEEPROM(byte pause);
    void setSafetyInEEPROM(boolean safety);

    void EEPROMUpdate();

		//return private variables
    float hyst();
    float tempParameter();
    unsigned short rotationUp();
    unsigned short rotationDown();
    unsigned short increments();
    unsigned short pause();
    boolean safety();
    unsigned short incrementCounter();
    byte mode();
    boolean opening();
    boolean closing();
    boolean openingCycle();
    boolean closingCycle();

  private:
		//Parameters
		float _hyst;
    float _tempParameter;
    unsigned short _rotationUp;
    unsigned short _rotationDown;
    unsigned short _increments;
    unsigned short _pause;
    boolean _safety;
    byte _openingPin;
    byte _closingPin;
    boolean _relayType;
		byte _stageInc[5];
		float _stageMod[5];

		//Logic variables
		unsigned short _incrementCounter;
    byte _mode;
    boolean _opening;
    boolean _closing;
    boolean _openingCycle;
    boolean _closingCycle;
    boolean _routine;
		boolean _safetyCycle;
		short _move;
		short _stage;
		byte _stages;

		//Timers
		elapsedMillis rollupTimer;
		elapsedMillis EEPROMTimer;
		elapsedMillis safetyTimer;

		//Indexes
    unsigned short _localIndex;
    static unsigned short _index;

		//private functions
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

			//EEPROM functions
      void setParametersInEEPROM(short temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(short temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);
			void EEPROMUpdate();

			//return private variables
			byte mode();
			byte pin();
			float hyst();
			float tempParameter();
			boolean safety();
			boolean debug();

    private:
			//Parameters
			byte _mode;
			byte _pin;
			float _hyst;
			float _tempParameter;
			boolean _safety;
			//Logic variables
			boolean _debug;
      boolean _routine;
			//Indexes
			unsigned short _localIndex;
      static unsigned short _index;
			//Timer
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
				//EEPROM functions
      void setParametersInEEPROM(short temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(short temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);
			void EEPROMUpdate();

			//return private variables
			byte mode();
			byte pin();
			float hyst();
			float tempParameter();
			boolean safety();
			boolean debug();

    private:
			//parameters
			byte _mode;
		  byte _pin;
		  float _hyst;
		  float _tempParameter;
		  boolean _safety;
			//logic
			boolean _debug;
      boolean _routine;
			//indexes
      unsigned short _localIndex;
      static unsigned short _index;
			//timer
			elapsedMillis EEPROMTimer;
};

//Generic macros
byte negativeToByte(int data, byte mod);
int byteToNegative(int data, byte mod);
void convertDecimalToTime(int * heure, int * minut);

#endif
