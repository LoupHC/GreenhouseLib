// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.

#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"
#include "GreenhouseLib.h"


//****************************************************************
//*******************TIMEZONES FUNCTIONS**************************
//****************************************************************
Timezone::Timezone(){
  _localIndex = TIMEZONE_INDEX + _index;
  _index += 5;
  _previousMillis = 0;
}

Timezone::~Timezone(){}

void Timezone::setParameters(byte type, short mod, float targetTemp){
	setType(type);
	setTime(mod);
	setTemp(targetTemp);
	setMod(mod);
}
void Timezone::setParameters(byte type, byte hour, byte min, float targetTemp){
	setType(type)	;
	setTime(hour,min);
	setTemp(targetTemp);
}
void Timezone::setTime( short mod){
	if (_type == SR){		
		short Time[3];
		Time[HEURE] = sunRise[HEURE];
		Time[MINUTE] = sunRise[MINUTE] + mod;
		convertDecimalToTime(&Time[HEURE], &Time[MINUTE]);
		_hour = (byte)Time[HEURE];
		_min = (byte)Time[MINUTE];
	}
	else if (_type == SS){		
		short Time[3];
		Time[HEURE] = sunSet[HEURE];
		Time[MINUTE] = sunSet[MINUTE] + mod;
		convertDecimalToTime(&Time[HEURE], &Time[MINUTE]);
		_hour = (byte)Time[HEURE];
		_min = (byte)Time[MINUTE];
	}
}
void Timezone::setTime(byte hour, byte min){
	if (_type == CLOCK){		
		_hour = hour;
		_min = min;
	}
}
void Timezone::setTemp(float targetTemp){
	_targetTemp = targetTemp;
}
void Timezone::setType(byte type){
	_type = type;
}
void Timezone::setMod(short mod){
	_mod = mod;
}

void Timezone::loadEEPROMParameters(){
  setType(EEPROM.read(_localIndex+TYPE_INDEX));
  setMod((short)EEPROM.read(_localIndex+MOD_INDEX));
  
  if ((_type == SR)||(_type == SS)){
	  setTime((short)EEPROM.read(_localIndex+MOD_INDEX));
  }
  else if(_type == CLOCK){
  	  setTime(EEPROM.read(_localIndex+HOUR_INDEX), EEPROM.read(_localIndex+MIN_INDEX));
  }
  
  setTemp((float)EEPROM.read(_localIndex+TARGET_INDEX));
}

void Timezone::setParametersInEEPROM(byte type, short mod, float targetTemp){	
	setTimeInEEPROM(type, mod);
	setTempInEEPROM(targetTemp);
}

void Timezone::setParametersInEEPROM(byte type, byte hour, byte min, float targetTemp){
	setTimeInEEPROM(type, hour, min);
	setTempInEEPROM(targetTemp);
}

void Timezone::setTimeInEEPROM(byte type, short mod){
	if ((type == SR)&&((mod >= -60) && (mod <= 60))){		
		short Time[3];
		Time[HEURE] = sunRise[HEURE];
		Time[MINUTE] = sunRise[MINUTE] + mod;
		convertDecimalToTime(&Time[HEURE], &Time[MINUTE]);
		EEPROM.update(_localIndex+HOUR_INDEX, (byte)Time[HEURE]);
		EEPROM.update(_localIndex+MIN_INDEX, (byte)Time[MINUTE]);
		EEPROM.update(_localIndex+TYPE_INDEX, type);
		EEPROM.update(_localIndex+MOD_INDEX, mod);
	}
	else if ((type == SS)&&((mod >= -60) && (mod <= 60))){		
		short Time[3];
		Time[HEURE] = sunSet[HEURE];
		Time[MINUTE] = sunSet[MINUTE] + mod;
		convertDecimalToTime(&Time[HEURE], &Time[MINUTE]);
		EEPROM.update(_localIndex+HOUR_INDEX, (byte)Time[HEURE]);
		EEPROM.update(_localIndex+MIN_INDEX, (byte)Time[MINUTE]);
		EEPROM.update(_localIndex+TYPE_INDEX, type);
		EEPROM.update(_localIndex+MOD_INDEX, mod);
	}
}

void Timezone::setTimeInEEPROM(byte type, byte hour, byte min){
	if (type == CLOCK){
		EEPROM.update(_localIndex+HOUR_INDEX, hour);
		EEPROM.update(_localIndex+MIN_INDEX, min);
		EEPROM.update(_localIndex+TYPE_INDEX, type);
	}
}

void Timezone::setTempInEEPROM(float targetTemp){
		EEPROM.update(_localIndex+TARGET_INDEX, targetTemp);
}


void Timezone::EEPROMUpdate(){
  unsigned long currentMillis = millis();
  if (currentMillis - _previousMillis >= _interval) {
    _previousMillis = currentMillis;
    
    if(_type != EEPROM.read(_localIndex+ TYPE_INDEX)){
    		EEPROM.update(_localIndex+TYPE_INDEX, _type);
    }
    if(_mod != EEPROM.read(_localIndex+ MOD_INDEX)){
    		EEPROM.update(_localIndex+MOD_INDEX, _mod);
    }
    if(_hour != EEPROM.read(_localIndex+ HOUR_INDEX)){
    		EEPROM.update(_localIndex+HOUR_INDEX, _hour);
    }
    if(_min != EEPROM.read(_localIndex+ MIN_INDEX)){
    		EEPROM.update(_localIndex+MIN_INDEX, _type);
    }
    if(_targetTemp != EEPROM.read(_localIndex+ TARGET_INDEX)){
    		EEPROM.update(_localIndex+TARGET_INDEX, _targetTemp);
    }
 }
}

static unsigned short Timezone::_index = 0;

static short Timezone::sunRise[3] = {0};
static short Timezone::sunSet[3] = {0};

static const unsigned long Timezone::_interval = 10000;


unsigned long rampingE(){
  return (unsigned long)EEPROM.read(RAMPING)*60*1000;
}

//****************************************************************
//*******************TOOLBOX**************************************
//****************************************************************

byte negativeToByte(int data, byte mod){
  return data+mod;
}

int byteToNegative(int data, byte mod){
  return data-mod;
}
//Programme pour convertir l'addition de nombres décimales en format horaire
void convertDecimalToTime(int * heure, int * minut){
  //Serial.println(m);
  if ((*minut > 59) && (*minut < 120)){
    *heure += 1;
    *minut -= 60;
  }
  else if ((*minut < 0) && (*minut >= -60)){
    *heure -= 1;
    *minut +=60;
  }
}
//****************************************************************
//*******************ROLLUP FUNCTIONS*****************************
//****************************************************************

static unsigned short Rollup::_index = 0;

/*
Constructor : Define Opening and Closing pins on the arduino, to connect to the relay box, active by default
Increment counter is set to 100 so the program recognize the first opening/closing run.
*/
Rollup::Rollup(){
  _debug = false;
  _routine = true;
  _closingCycle = false;
  _openingCycle = false;
  _incrementCounter = 100;
  _localIndex = ROLLUP_INDEX + _index;
  _index += 7;
  _previousMillis = 0;
}

/*
Destructor
*/
Rollup::~Rollup(){}

void Rollup::initOutputs(byte mode, byte rOpen, byte rClose){
  pinMode(rOpen, OUTPUT);
  digitalWrite(rOpen, LOW);
  pinMode(rClose, OUTPUT);
  digitalWrite(rClose, LOW);
  _openingPin = rOpen;
  _closingPin = rClose;
  _mode = mode;
}
/*
Open or close the rollups by increments when a certain temperature is reached
Adjust to the internal target temperature (Mode FIX_TEMP)
*/
void Rollup::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
      if ((temp < (_tempParameter - _hyst))||(_closingCycle == true)) {
        if (_openingCycle == false){
        	closingSides();
        }
      } else if ((temp > _tempParameter)||(_openingCycle == true)) {
        if (_closingCycle == false){
        	openSides();
        }
      }
    }
  }

  else{
    _debug = true;
  }
}
/*
Open or close the rollups by increments when a certain temperature is reached
Adjust to an external target temperature (Mode VAR_TEMP)
*/
void Rollup::routine(float target, float temp){
  if (_mode == VAR_TEMP){
  	 if(_routine == true){
      float activationTemp = target + _tempParameter;
      if ((temp < (activationTemp - _hyst))||(_closingCycle == true)) {
        if (_openingCycle == false){
        	closingSides();
        }
      } else if ((temp > activationTemp)||(_openingCycle == true)) {
        if (_closingCycle == false){
        	openSides();
        }
      }
    }
  }

  else{
    _debug = true;
  }
}

/*
Activate or desactivate the routine function
*/
void Rollup::desactivateRoutine(){
  _routine = false;
}
void Rollup::activateRoutine(){
  _routine = true;
}

/*
Activate or desactivate the opening or closing relay
*/

void Rollup::startOpening(){
	if(_closing == false){
		_opening = true;		
      digitalWrite(_openingPin, HIGH);
      #ifdef DEBUG
        Serial.println("opening");
      #endif
	}
}

void Rollup::startClosing(){
	if(_opening == false){
		_closing = true;		
      digitalWrite(_closingPin, HIGH);
      #ifdef DEBUG
        Serial.println("closing");
      #endif
	}
}

void Rollup::stopOpening(){
	_opening = false;
	digitalWrite(_openingPin, LOW);					

}

void Rollup::stopClosing(){
	_closing = false;
	digitalWrite(_openingPin, LOW);						

}
/*
Open or close the rollups by one increment
*/
void Rollup::openSides(){
  if (_incrementCounter == 100){
    _incrementCounter = 0;
    _lastAction = OPEN;
  }
  if(_lastAction == CLOSE){
	 _incrementCounter = 0;  
    _lastAction = OPEN;
  }
  
  if (_incrementCounter < _increments) {
    
    if ((_opening == false) && (_openingCycle == false)){		//Si le rollup était précédemment inactif et qu'aucune cycle d'ouverture n'est en cours
    	rollupTimer = 0;														//On initie le compteur
    	_openingCycle == true;												//Un cycle est en cours
    	startOpening();														//On active le relais d'ouverture
    	_incrementCounter += 1;												//L'incrément augmente de +1
    }
    
    if(rollupTimer >= (_rotationUp/_increments)){					//Si le compteur atteint le temps d'ouverture/le nombre d'incréments...
    	stopOpening();															//On désactive le relais
    }
    if(rollupTimer >= (_rotationUp/_increments + _pause)){		//Après le temps de pause...
    	_openingCycle == false;												//On peut recommencer le cycle
    }
    
  }
}

void Rollup::closingSides(){
  if (_incrementCounter == 100){
    _incrementCounter = _increments;
    _lastAction = CLOSE;
  }
  if(_lastAction == OPEN){
	 _incrementCounter = _increments;  
    _lastAction = CLOSE;
  }
  
  if (_incrementCounter < _increments) {
    
    if ((_closing == false) && (_closingCycle == false)){		//Si le rollup était précédemment inactif et qu'aucune cycle de fermeture n'est en cours
    	rollupTimer = 0;			//On initie le compteur
    	_closingCycle == true;			//Un cycle est en cours
    	startClosing();			//On active le relais d'ouverture
    	_incrementCounter += 1;	//L'incrément augmente de +1
    }
    
    if(rollupTimer >= (_rotationDown/_increments)){		//Si le compteur atteint le temps d'ouverture/le nombre d'incréments...
    	stopClosing();												//On désactive le relais
    }
    if(rollupTimer >= (_rotationDown/_increments + _pause)){	//Après le temps de pause...
    	_closingCycle == false;										//On peut recommencer le cycle
    }
    
  }
}
/*
Open or close the rollups to the maximum or minimum increment
*/
void Rollup::openCompletely(){
  if (_incrementCounter == 100){
    _incrementCounter = 0;
    _lastAction = OPEN;
  }
  #ifdef DEBUG_ROLLUP
  Serial.println("opening");
  #endif
  digitalWrite(_openingPin, HIGH);
  for(byte x = 0; x < _rotationUp; x--)
  {
    delay(1000);
  }
  _incrementCounter = _increments;
  digitalWrite(_openingPin, LOW);
}

void Rollup::closeCompletely(){
  if (_incrementCounter == 100){
    _incrementCounter = _increments;
  }
  #ifdef DEBUG_ROLLUP
  Serial.println("closing");
  #endif
  digitalWrite(_closingPin, HIGH);
  for(byte x = 0; x < _rotationDown; x--)
  {
    delay(1000);
  }
  _incrementCounter = 0;
  digitalWrite(_closingPin, LOW);
}
/*
Program all parameters all at once...
*/
void Rollup::setParameters(float rTemp, float rHyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety){
  setTemp(rTemp);
  setHyst(rHyst);
  setRotationUp(rotationUp);
  setRotationDown(rotationDown);
  setIncrements(increments);
  setPause(pause);
  setSafety(safety);
}
/*
Or one by one...
*/
void Rollup::setTemp(float temp){
    switch (_mode){
      case FIX_TEMP:
        _tempParameter = temp;
      break;
      case VAR_TEMP:
        _tempParameter = temp-10;
      break;
    }
}

void Rollup::setHyst(float rHyst){
  _hyst = rHyst;
}
void Rollup::setRotationUp(unsigned short rotationUp){
  _rotationUp = rotationUp;
}

void Rollup::setRotationDown(unsigned short rotationDown){
  _rotationDown = rotationDown;
}

void Rollup::setIncrements(unsigned short increments){
  _increments = increments;
}

void Rollup::setPause(unsigned short pause){
  _pause = pause;
}

void Rollup::setSafety(boolean safety){
  _safety = safety;
}

void Rollup::setIncrementCounter(unsigned short increment){
  _incrementCounter = increment;
}
/*
upload new parameters in EEPROM memory
*/

void Rollup::setParametersInEEPROM(unsigned short rTemp, byte rHyst, byte rotationUp, byte rotationDown, byte increments,byte pause, boolean safety){
  setHystInEEPROM(rHyst);
  setTempInEEPROM(rTemp);
  setRotationUpInEEPROM(rotationUp);
  setRotationDownInEEPROM(rotationDown);
  setIncrementsInEEPROM(increments);
  setPauseInEEPROM(pause);
  setSafetyInEEPROM(safety);
}
/*
load last saved parameters
*/
void Rollup::loadEEPROMParameters(){
  setTemp((float)EEPROM.read(_localIndex+TEMP_INDEX));
  setHyst((float)EEPROM.read(_localIndex+HYST_INDEX));
  setRotationUp((unsigned short)EEPROM.read(_localIndex+ROTATION_UP_INDEX));
  setRotationDown((unsigned short)EEPROM.read(_localIndex+ROTATION_DOWN_INDEX));
  setIncrements((unsigned short)EEPROM.read(_localIndex+INCREMENTS_INDEX));
  setPause((unsigned short)EEPROM.read(_localIndex+PAUSE_INDEX));
  setSafety((boolean)EEPROM.read(_localIndex+SAFETY_INDEX));
}

void Rollup::setTempInEEPROM(unsigned short temp){
    switch (_mode){
      case FIX_TEMP:
          EEPROM.update(_localIndex+TEMP_INDEX, temp);
      break;
      case VAR_TEMP:
          unsigned short adjustTemp = temp+10;
          EEPROM.update(_localIndex+TEMP_INDEX, (byte)adjustTemp);
      break;
    }
}

void Rollup::setHystInEEPROM(byte hyst){
EEPROM.update(_localIndex+HYST_INDEX, hyst);
}

void Rollup::setRotationUpInEEPROM(byte rotationUp){
EEPROM.update(_localIndex+ROTATION_UP_INDEX, rotationUp);
}

void Rollup::setRotationDownInEEPROM(byte rotationDown){
EEPROM.update(_localIndex+ROTATION_DOWN_INDEX, rotationDown);
}

void Rollup::setIncrementsInEEPROM(byte increments){
EEPROM.update(_localIndex+INCREMENTS_INDEX, increments);
}

void Rollup::setPauseInEEPROM(byte pause){
EEPROM.update(_localIndex+PAUSE_INDEX, pause);
}

void Rollup::setSafetyInEEPROM(boolean safety){
EEPROM.update(_localIndex+SAFETY_INDEX, safety);
}

static const unsigned long Rollup::_interval = 10000;

void Rollup::EEPROMUpdate(){
  unsigned long currentMillis = millis();
  if (currentMillis - _previousMillis >= _interval) {
    _previousMillis = currentMillis;

    switch (_mode){
      case FIX_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex)){
          setTempInEEPROM(_tempParameter);
        }
      break;
      case VAR_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex)-10){
          unsigned short adjustTempParameter = _tempParameter+10;
          setTempInEEPROM((byte) adjustTempParameter);
        }
      break;
    }


    if (_hyst != (float)EEPROM.read(_localIndex+1)){
      setHystInEEPROM(_hyst);
    }
    if (_rotationUp != (unsigned short)EEPROM.read(_localIndex+2)){
      setRotationUpInEEPROM(_rotationUp);
    }
    if (_rotationDown != (unsigned short)EEPROM.read(_localIndex+3)){
      setRotationDownInEEPROM(_rotationDown);
    }
    if (_increments != (unsigned short)EEPROM.read(_localIndex+4)){
      setIncrementsInEEPROM(_increments);
    }
    if (_pause != (unsigned short)EEPROM.read(_localIndex+5)){
      setPauseInEEPROM(_pause);
    }
    if (_safety != (boolean)EEPROM.read(_localIndex+6)){
      setSafetyInEEPROM(_safety);
    }
  }
}

//****************************************************
//******************FAN FUNCTIONS************************
//****************************************************************

Fan::Fan(){
    _active = true;
    _localIndex = FAN_INDEX + _index;
    _index += 3;
}

static unsigned short Fan::_index = 0;

Fan::~Fan(){}

void Fan::initOutput(byte pin){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    _pin = pin;
 }

//action functions
void Fan::routine(float temp){
  if (_active == true){
    if (temp < (_tempParameter - _hyst)) {
      if(digitalRead(_pin) == HIGH){
        digitalWrite(_pin, LOW);
        Serial.println("Fan : off");
      }
    }
    else if (temp > _tempParameter) {
      if(digitalRead(_pin) == LOW){
        digitalWrite(_pin, HIGH);
        Serial.println("Fan : on");
      }
    }
  }
}

void Fan::stop(){
  if(digitalRead(_pin) == HIGH){
    digitalWrite(_pin, LOW);
    Serial.println("Fan : off");
  }
}

void Fan::start(){
  if(digitalRead(_pin) == LOW){
    digitalWrite(_pin, HIGH);
    Serial.println("Fan : off");
  }
}
/*
Activate or desactivate the routine function
*/
void Fan::desactivate(){
  _active = false;
}
void Fan::activate(){
  _active = true;
}

//programmation functions

void Fan::setParameters(float activationTemp, float hyst, boolean safety){
  setHyst(hyst);
  setTemp(activationTemp);
  setSafety(safety);
}
/*
Or one by one...
*/
void Fan::setHyst(float hyst){
  _hyst = hyst;
}

void Fan::setTemp(float temp){
  _tempParameter = temp;
}

void Fan::setSafety(boolean safety){
  _safety = safety;
}

void Fan::setParametersInEEPROM(byte temp, byte hyst, boolean safety){
  setHystInEEPROM(hyst);
  setTempInEEPROM(temp);
  setSafetyInEEPROM(safety);
}
/*
load last saved parameters
*/
void Fan::loadEEPROMParameters(){
  setTemp((float)EEPROM.read(_localIndex+TEMP_INDEX));
  setHyst((float)EEPROM.read(_localIndex+HYST_INDEX));
  setSafety((boolean)EEPROM.read(_localIndex+SAFETY_INDEX));
}

void Fan::setTempInEEPROM(byte temp){
    EEPROM.update(_localIndex+TEMP_INDEX, temp);
}

void Fan::setHystInEEPROM(byte hyst){
EEPROM.update(_localIndex+HYST_INDEX, hyst);
}
void Fan::setSafetyInEEPROM(boolean safety){
EEPROM.update(_localIndex+SAFETY_INDEX, safety);
}

//****************************************************************
//******************HEATER FUNCTIONS************************
//****************************************************************

Heater::Heater(){
    _active = true;
    _localIndex = HEATER_INDEX + _index;
    _index += 3;
}

static unsigned short Heater::_index = 0;

Heater::~Heater(){}


void Heater::initOutput(byte pin){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    _pin = pin;
 }
 
//action functions
void Heater::routine(float temp){
  if (_active == true){
    if (temp > (_tempParameter + _hyst)) {
      if(digitalRead(_pin) == HIGH){
        digitalWrite(_pin, LOW);
        Serial.println("Heater : off");
      }
    }
    else if (temp < _tempParameter) {
      if(digitalRead(_pin) == LOW){
        digitalWrite(_pin, HIGH);
        Serial.println("Heater : on");
      }
    }
  }
}

void Heater::stop(){
  if(digitalRead(_pin) == HIGH){
    digitalWrite(_pin, LOW);
    Serial.println("Heater : off");
  }
}

void Heater::start(){
  if(digitalRead(_pin) == LOW){
    digitalWrite(_pin, HIGH);
    Serial.println("Heater : off");
  }
}
/*
Activate or desactivate the routine function
*/
void Heater::desactivate(){
  _active = false;
}
void Heater::activate(){
  _active = true;
}

//programmation functions

void Heater::setParameters(float activationTemp, float hyst, boolean safety){
  setHyst(hyst);
  setTemp(activationTemp);
  setSafety(safety);
}

/*
Or one by one...
*/
void Heater::setHyst(float hyst){
  _hyst = hyst;
}

void Heater::setTemp(float temp){
  _tempParameter = temp;
}

void Heater::setSafety(boolean safety){
  _safety = safety;
}

void Heater::setParametersInEEPROM(byte temp, byte hyst, boolean safety){
  setHystInEEPROM(hyst);
  setTempInEEPROM(temp);
  setSafetyInEEPROM(safety);
}
/*
load last saved parameters
*/
void Heater::loadEEPROMParameters(){
  setTemp((float)EEPROM.read(_localIndex+TEMP_INDEX));
  setHyst((float)EEPROM.read(_localIndex+HYST_INDEX));
  setSafety((boolean)EEPROM.read(_localIndex+SAFETY_INDEX));
}

void Heater::setTempInEEPROM(byte temp){
    EEPROM.update(_localIndex+TEMP_INDEX, temp);
}
void Heater::setHystInEEPROM(byte hyst){
EEPROM.update(_localIndex+HYST_INDEX, hyst);
}
void Heater::setSafetyInEEPROM(boolean safety){
EEPROM.update(_localIndex+SAFETY_INDEX, safety);
}
