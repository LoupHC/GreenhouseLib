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
  EEPROMTimer = 0;
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
  if (EEPROMTimer > 10000) {
    EEPROMTimer = 0;

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
  _routine = true;
  _closing = false;
  _opening = false;
  _closingCycle = false;
  _openingCycle = false;
  _safetyCycle = false;
  _incrementCounter = OFF_VAL;
  _stage = OFF_VAL;
  _localIndex = ROLLUP_INDEX + _index;
  _index += 7;
  EEPROMTimer = 0;
}

/*
Destructor
*/
Rollup::~Rollup(){}

void Rollup::initOutputs(byte mode, boolean relayType, byte rOpen, byte rClose){

  pinMode(rOpen, OUTPUT);
  pinMode(rClose, OUTPUT);

  if(relayType == ACT_HIGH){
  	digitalWrite(rOpen, LOW);
  	digitalWrite(rClose, LOW);
  }
  else if (relayType == ACT_LOW){
  	digitalWrite(rOpen, HIGH);
  	digitalWrite(rClose, HIGH);
  }
  _openingPin = rOpen;
  _closingPin = rClose;
  _mode = mode;
  _relayType = relayType;
}

void Rollup::initStage(byte stage, float mod, byte inc){
  _stages = stage;
  _stageMod[stage] = mod;
  _stageInc[stage] = inc;
}
/*
Open or close the rollups by increments when a certain temperature is reached
Adjust to the internal target temperature (Mode FIX_TEMP)
*/
void Rollup::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
     #ifdef DEBUG_ROLLUP_TEMP
       Serial.println("------------");
       Serial.print("Temperature :  ");
       Serial.println(temp);
       Serial.print("Opening temp : ");
       Serial.println(_tempParameter);
       Serial.print("Closing temp : ");
       Serial.println(_tempParameter-_hyst);
       Serial.println("-------------");
     #endif
      if ((temp < (_tempParameter - _hyst))||(_closingCycle == true)) {
        	closingSides();
      } else if ((temp > _tempParameter)||(_openingCycle == true)) {
        	openSides();
      }
    }
    safetyCycle();
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
      #ifdef DEBUG_ROLLUP_TEMP
        Serial.println("------------");
        Serial.print("Temperature :  ");
        Serial.println(temp);
        Serial.print("Opening temp : ");
        Serial.println(target+_tempParameter);
        Serial.print("Closing temp : ");
        Serial.println(target+_tempParameter-_hyst);
        Serial.println("-------------");
      #endif

      if ((temp < (activationTemp - _hyst))||(_closingCycle == true)) {
        	closingSides();
      } else if ((temp > activationTemp)||(_openingCycle == true)) {
        	openSides();
      }
    }
    safetyCycle();
  }
}

void Rollup::manualRoutine(float target, float temp){
  if (_mode == MAN_TEMP){
  	if(_routine == true){
      float targetTemp = target + _tempParameter;

      #ifdef DEBUG_ROLLUP_TEMP
        Serial.println("------------");
        Serial.print("Temperature :  ");
        Serial.println(temp);
        Serial.print("Stage 1 : Opening temp : ");
        Serial.println(targetTemp + _stageMod[1]);
        Serial.print("Stage 1 : Closing temp : ");
        Serial.println(targetTemp + _stageMod[1] -_hyst);
        Serial.print("Stage 2 : Opening temp : ");
        Serial.println(targetTemp + _stageMod[2]);
        Serial.print("Stage 2 : Closing temp : ");
        Serial.println(targetTemp + _stageMod[2] -_hyst);
        Serial.print("Stage 3 : Opening temp : ");
        Serial.println(targetTemp + _stageMod[3]);
        Serial.print("Stage 3 : Closing temp : ");
        Serial.println(targetTemp + _stageMod[3] -_hyst);
        Serial.print("Stage 4 : Opening temp : ");
        Serial.println(targetTemp + _stageMod[4]);
        Serial.print("Stage 4 : Closing temp : ");
        Serial.println(targetTemp + _stageMod[4] -_hyst);
        Serial.println("-------------");
      #endif


      if (((temp >= targetTemp + _stageMod[4])&&(_stage == OFF_VAL))||(_stage == 4)){
        if (_stages >= 4){
          openToInc(4, _stageInc[4]);
        }
      }
      else if (((temp >= targetTemp + _stageMod[3])&&(_stage == OFF_VAL))||(_stage == 3)){
        if (_stages >= 3){
          openToInc(3, _stageInc[3]);
        }
      }
      else if (((temp >= targetTemp + _stageMod[2])&&(_stage == OFF_VAL))||(_stage == 2)){
        if (_stages >= 2){
          openToInc(2, _stageInc[2]);
        }
      }
      else if (((temp >= targetTemp + _stageMod[1])&&(_stage == OFF_VAL))||(_stage == 1)){
        if (_stages >= 1){
          openToInc(1, _stageInc[1]);
        }
      }

      if (((temp < targetTemp + _stageMod[1] - _hyst)&&(_stage == OFF_VAL))||(_stage == -1)){
        if (_stages >= 1){
          closeToInc(-1, 0);
        }
      }
      else if (((temp < targetTemp + _stageMod[2] - _hyst)&&(_stage == OFF_VAL))||(_stage == -2)){
        if (_stages >= 2){
          closeToInc(-2, _stageInc[1]);
        }
      }
      else if (((temp < targetTemp + _stageMod[3] - _hyst)&&(_stage == OFF_VAL))||(_stage == -3)){
        if (_stages >= 3){
          closeToInc(-3, _stageInc[2]);
        }
      }
      else if (((temp < targetTemp + _stageMod[4] - _hyst)&&(_stage == OFF_VAL))||(_stage == -4)){
        if (_stages >= 4){
          closeToInc(-4, _stageInc[3]);
        }
      }
    }
    safetyCycle();
  }
}




void Rollup::openToInc(byte stage, byte targetIncrement){

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }
  if ((_opening == false)&&(_openingCycle == false)){
    _move = targetIncrement - _incrementCounter;

    if (_move > 0){
      rollupTimer = 0;
      _openingCycle = true;
      _stage = stage;
      startOpening();
    }
  }
  if (_move > 0){
    if(_openingCycle == true){

      #ifdef DEBUG_ROLLUP_TIMING
          Serial.println("-------------");
          Serial.println("OPENING CYCLE");
          Serial.print("Increment :     ");
          Serial.println(_incrementCounter);
          Serial.print("Target increment :     ");
          Serial.println(targetIncrement);
        	Serial.print("Timer :         ");
        	Serial.println(rollupTimer);
        	Serial.print("Rotation time : ");
        	Serial.println((unsigned long)_rotationUp*1000/_increments*_move);
        	Serial.print("Pause time :    ");
        	Serial.println((unsigned long)_rotationUp*1000/_increments*_move+_pause*1000);
            Serial.println("-------------");
      #endif

      if (rollupTimer >= ((unsigned long)_rotationUp*1000/_increments*_move)){
        if(_opening == true){
          _incrementCounter = _incrementCounter + _move;
          stopOpening();
          printPause();
        }
      }
      if (rollupTimer >= ((unsigned long)_rotationUp*1000/_increments*_move + (unsigned long)_pause*1000)){
        _openingCycle = false;
        _stage = OFF_VAL;
        printEndPause();
      }
    }
  }
}

void Rollup::closeToInc(byte stage, byte targetIncrement){

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }

  if ((_closing == false)&&(_closingCycle == false)){
      _move = targetIncrement - _incrementCounter;
      if (_move < 0){
        rollupTimer = 0;
        _closingCycle = true;
        _stage = stage;
        startClosing();
      }
  }
  if (_move < 0){
    if(_closingCycle == true){

      #ifdef DEBUG_ROLLUP_TIMING
        Serial.println("-------------");
        Serial.println("CLOSING CYCLE");
        Serial.print("Increment :     ");
        Serial.println(_incrementCounter);
        Serial.print("Target increment :     ");
        Serial.println(targetIncrement);
      	Serial.print("Timer :         ");
      	Serial.println(rollupTimer);
      	Serial.print("Rotation time : ");
      	Serial.println((unsigned long)_rotationDown*1000/_increments*(0-_move));
      	Serial.print("Pause time :    ");
      	Serial.println((unsigned long)_rotationDown*1000/_increments*(0-_move)+(unsigned long)_pause*1000);
          Serial.println("-------------");
      #endif

      if (rollupTimer >= ((unsigned long)_rotationDown*1000/_increments*(0-_move))){
        if(_closing == true){
          _incrementCounter = _incrementCounter + _move;
          stopClosing();
          printPause();
        }
      }
      if (rollupTimer >= ((unsigned long)_rotationDown*1000/_increments*(0-_move) + (unsigned long)_pause*1000)){
        _closingCycle = false;
        _stage = OFF_VAL;
        printEndPause();
      }
    }
  }
}

void Rollup::safetyCycle(){
  if(_safety == true){
    if(_incrementCounter == _increments){
      safetyOpen();
    }
    else if (_incrementCounter == 0){
      safetyClose();
    }
  }
}

void Rollup::safetyOpen(){
  if ((_safetyCycle == false)||(_stage != OFF_VAL)){
    _safetyCycle = true;
    safetyTimer = 0;
  }
  if(safetyTimer >= SAFETY_DELAY){
    if(_opening == false){
      desactivateRoutine();
      startOpening();
    }
  }
  if(safetyTimer >= SAFETY_DELAY+_rotationUp*1000){
    if(_opening == true){
      stopOpening();
      activateRoutine();
      _safetyCycle = false;
    }
  }
}

void Rollup::safetyClose(){
  if ((_safetyCycle == false)||(_stage != OFF_VAL)){
    _safetyCycle = true;
    safetyTimer = 0;
  }
  if(safetyTimer >= SAFETY_DELAY){
    if(_closing == false){
      desactivateRoutine();
      startClosing();
    }
  }
  if(safetyTimer >= SAFETY_DELAY+_rotationDown*1000){
    if(_closing == true){
      stopClosing();
      activateRoutine();
      _safetyCycle = false;
    }
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
    if(_relayType == ACT_HIGH){
    	digitalWrite(_openingPin, HIGH);
    }
    else if (_relayType == ACT_LOW){
    	digitalWrite(_openingPin, LOW);
    }

    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println("-------------");
      Serial.println("opening");
      Serial.println("-------------");
    #endif
	}
}

void Rollup::startClosing(){
	if(_opening == false){
		_closing = true;
    if(_relayType == ACT_HIGH){
    	digitalWrite(_closingPin, HIGH);
    }
    else if (_relayType == ACT_LOW){
    	digitalWrite(_closingPin, LOW);
    }
    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println("-------------");
      Serial.println("closing");
      Serial.println("-------------");
    #endif
	}
}

void Rollup::stopOpening(){
	if(_opening == true){
		_opening = false;
    if(_relayType == ACT_HIGH){
    	digitalWrite(_openingPin, LOW);
    }
    else if (_relayType == ACT_LOW){
    	digitalWrite(_openingPin, HIGH);
    }
    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println("-------------");
      Serial.println("stop opening");
      Serial.println("-------------");
    #endif
	}
}

void Rollup::stopClosing(){
	if(_closing == true){
		_closing = false;
    if(_relayType == ACT_HIGH){
    	digitalWrite(_closingPin, LOW);
    }
    else if (_relayType == ACT_LOW){
    	digitalWrite(_closingPin, HIGH);
    }
    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println("-------------");
      Serial.println("stop closing");
      Serial.println("-------------");
    #endif
	}
}
/*
Open or close the rollups by one increment
*/
void Rollup::openSides(){
	#ifdef DEBUG_ROLLUP_TIMING
  if(_openingCycle == true){
    Serial.println("-------------");
    Serial.println("OPENING CYCLE");
  	Serial.print("Timer :         ");
  	Serial.println(rollupTimer);
  	Serial.print("Rotation time : ");
  	Serial.println(((unsigned long)_rotationUp*1000/_increments));
  	Serial.print("Pause time :    ");
  	Serial.println(((unsigned long)_rotationUp*1000/_increments+(unsigned long)_pause*1000));
    Serial.print("Increment :     ");
    Serial.println(_incrementCounter);
      Serial.println("-------------");
  }
	#endif

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }

  if ((_incrementCounter < _increments)||(_openingCycle == true)) {
    if ((_opening == false) && (_openingCycle == false)){		//Si le rollup était précédemment inactif et qu'aucune cycle d'ouverture n'est en cours
    	rollupTimer = 0;														//On initie le compteur
    	_openingCycle = true;												//Un cycle est en cours
    	startOpening();														//On active le relais d'ouverture
    }
  }

  if(_openingCycle == true){
    if(rollupTimer >= (((unsigned long)_rotationUp*1000/_increments))){					//Si le compteur atteint le temps d'ouverture/le nombre d'incréments...
      if(_opening == true){
        _incrementCounter += 1;												//L'incrément augmente de +1
        stopOpening();															//On désactive le relais
        printPause();
      }
    }
    if(rollupTimer >= ((unsigned long)_rotationUp*1000/_increments + (unsigned long)_pause*1000)){		//Après le temps de pause...
    	_openingCycle = false;												//On peut recommencer le cycle
      printEndPause();
    }
  }
}


void Rollup::closingSides(){
	#ifdef DEBUG_ROLLUP_TIMING
  if(_closingCycle == true){
    Serial.println("-------------");
    Serial.println("ClOSING CYCLE");
  	Serial.println("Timer :         ");
  	Serial.println(rollupTimer);
  	Serial.println("Rotation time : ");
  	Serial.println(((unsigned long)_rotationDown*1000/_increments));
  	Serial.println("Pause time : ");
  	Serial.println(((unsigned long)_rotationDown*1000/_increments+(unsigned long)_pause*1000));
    Serial.print("Increment :       ");
    Serial.println(_incrementCounter);
    Serial.println("-------------");
  }
  #endif
  if (_incrementCounter == OFF_VAL){
    _incrementCounter = _increments;
  }

  if (_incrementCounter > 0) {
    if ((_closing == false) && (_closingCycle == false)){		//Si le rollup était précédemment inactif et qu'aucune cycle d'ouverture n'est en cours
    	rollupTimer = 0;														//On initie le compteur
    	_closingCycle = true;												//Un cycle est en cours
    	startClosing();														//On active le relais d'ouverture
    }
  }
  if(_closingCycle == true){
    if(rollupTimer >= ((unsigned long)_rotationDown*1000/_increments)){					//Si le compteur atteint le temps d'ouverture/le nombre d'incréments...
      if(_opening == true){
        _incrementCounter -= 1;												//L'incrément augmente de +1
        stopClosing();															//On désactive le relais
        printPause();
      }
    }
    if(rollupTimer >= ((unsigned long)_rotationDown*1000/_increments + (unsigned long)_pause*1000)){		//Après le temps de pause...
    	_closingCycle = false;												//On peut recommencer le cycle
      printEndPause();
    }
  }
}

void Rollup::printPause(){
  #ifdef DEBUG_ROLLUP_TIMING
    Serial.println("-------------");
    Serial.println("start pause");
    Serial.println("-------------");
  #endif
}
void Rollup::printEndPause(){
  #ifdef DEBUG_ROLLUP_TIMING
    Serial.println("-------------");
    Serial.println("end of the pause");
    Serial.println("-------------");
  #endif
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
  _tempParameter = temp;
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


void Rollup::EEPROMUpdate(){
	Serial.println("EEPROM TIMER : ");
	Serial.println(EEPROMTimer);
  	 #ifdef DEBUG_EEPROM
  	 Serial.prinln(EEPROMTimer);
  	 #endif
  if (EEPROMTimer > 10000) {
  	 #ifdef DEBUG_EEPROM
  	 Serial.prinln("Rollup parameters saved in EEPROM");
  	 #endif
    EEPROMTimer = 0;

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


    if (_hyst != (float)EEPROM.read(_localIndex+TEMP_INDEX)){
      setHystInEEPROM(_hyst);
    }
    if (_rotationUp != (unsigned short)EEPROM.read(_localIndex+ROTATION_UP_INDEX)){
      setRotationUpInEEPROM(_rotationUp);
    }
    if (_rotationDown != (unsigned short)EEPROM.read(_localIndex+ROTATION_DOWN_INDEX)){
      setRotationDownInEEPROM(_rotationDown);
    }
    if (_increments != (unsigned short)EEPROM.read(_localIndex+INCREMENTS_INDEX)){
      setIncrementsInEEPROM(_increments);
    }
    if (_pause != (unsigned short)EEPROM.read(_localIndex+PAUSE_INDEX)){
      setPauseInEEPROM(_pause);
    }
    if (_safety != (boolean)EEPROM.read(_localIndex+SAFETY_INDEX)){
      setSafetyInEEPROM(_safety);
    }
  }
}

//****************************************************
//******************FAN FUNCTIONS************************
//****************************************************************

Fan::Fan(){

  	 _debug = false;
  	 _routine = true;
    _localIndex = FAN_INDEX + _index;
    _index += 3;
  	 EEPROMTimer = 0;
}

static unsigned short Fan::_index = 0;

Fan::~Fan(){}

void Fan::initOutput(byte mode, byte pin){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    _pin = pin;
	 _mode = mode;
 }

//action functions
/*
Start or stop the fan when a certain temperature, defined within the class itself, is reached
(Mode FIX_TEMP)
*/
void Fan::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
      if (temp < (_tempParameter - _hyst)) {
        	stop();
      } else if (temp > _tempParameter) {
        	start();
      }
    }
  }

  else{
    _debug = true;
  }
}
/*
Start or stop the fan when a certain temperature is reached
Adjust to an external target temperature (Mode VAR_TEMP)
*/
void Fan::routine(float target, float temp){
  if (_mode == VAR_TEMP){
  	 if(_routine == true){
      float activationTemp = target + _tempParameter;
      if (temp < (activationTemp - _hyst)) {
        	stop();
      } else if (temp > activationTemp) {
        	start();
      }
    }
  }

  else{
    _debug = true;
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
    Serial.println("Fan : on");
  }
}
/*
Activate or desactivate the routine function
*/
void Fan::desactivateRoutine(){
  _routine = false;
}
void Fan::activateRoutine(){
  _routine = true;
}

//programmation functions

void Fan::setParameters(float temp, float hyst, boolean safety){
  setHyst(hyst);
  setTemp(temp);
  setSafety(safety);
}
/*
Or one by one...
*/
void Fan::setHyst(float hyst){
  _hyst = hyst;
}

void Fan::setTemp(float temp){
    switch (_mode){
      case FIX_TEMP:
        _tempParameter = temp;
      break;
      case VAR_TEMP:
        _tempParameter = temp-10;
      break;
    }
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

void Fan::setTempInEEPROM(unsigned short temp){
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

void Fan::setHystInEEPROM(byte hyst){
EEPROM.update(_localIndex+HYST_INDEX, hyst);
}
void Fan::setSafetyInEEPROM(boolean safety){
EEPROM.update(_localIndex+SAFETY_INDEX, safety);
}
void Fan::EEPROMUpdate(){
  if (EEPROMTimer > 10000) {
    EEPROMTimer = 0;

    switch (_mode){
      case FIX_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex+TEMP_INDEX)){
          setTempInEEPROM(_tempParameter);
        }
      break;
      case VAR_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex+TEMP_INDEX)-10){
          unsigned short adjustTempParameter = _tempParameter+10;
          setTempInEEPROM((byte) adjustTempParameter);
        }
      break;
    }

    if (_hyst != (float)EEPROM.read(_localIndex+HYST_INDEX)){
      setHystInEEPROM(_hyst);
    }
    if (_safety != (boolean)EEPROM.read(_localIndex+SAFETY_INDEX)){
      setSafetyInEEPROM(_safety);
    }
 }
}
//****************************************************************
//******************HEATER FUNCTIONS************************
//****************************************************************

Heater::Heater(){
	 _debug = false;
    _routine = true;
    _localIndex = HEATER_INDEX + _index;
    _index += 3;
    EEPROMTimer = 0;
}

static unsigned short Heater::_index = 0;

Heater::~Heater(){}


void Heater::initOutput(byte mode, byte pin){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    _pin = pin;
    _mode = mode;
 }

//action functions
/*
Start or stop the heater when a certain temperature, defined within the class itself, is reached
(Mode FIX_TEMP)
*/
void Heater::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
      if (temp > (_tempParameter + _hyst)) {
        	stop();
      } else if (temp < _tempParameter) {
        	start();
      }
    }
  }

  else{
    _debug = true;
  }
}
/*
Start or stop the heater when a certain temperature is reached
Adjust to an external target temperature (Mode VAR_TEMP)
*/
void Heater::routine(float target, float temp){
  if (_mode == VAR_TEMP){
  	 if(_routine == true){
      float activationTemp = target + _tempParameter;
      if (temp > (activationTemp + _hyst)) {
        	stop();
      } else if (temp < activationTemp) {
        	start();
      }
    }
  }

  else{
    _debug = true;
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
    Serial.println("Heater : on");
  }
}
/*
Activate or desactivate the routine function
*/
void Heater::desactivateRoutine(){
  _routine = false;
}
void Heater::activateRoutine(){
  _routine = true;
}

//programmation functions


void Heater::setParameters(float temp, float hyst, boolean safety){
  setHyst(hyst);
  setTemp(temp);
  setSafety(safety);
}
/*
Or one by one...
*/
void Heater::setHyst(float hyst){
  _hyst = hyst;
}

void Heater::setTemp(float temp){
    switch (_mode){
      case FIX_TEMP:
        _tempParameter = temp;
      break;
      case VAR_TEMP:
        _tempParameter = temp-10;
      break;
    }
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

void Heater::setTempInEEPROM(unsigned short temp){
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

void Heater::setHystInEEPROM(byte hyst){
EEPROM.update(_localIndex+HYST_INDEX, hyst);
}
void Heater::setSafetyInEEPROM(boolean safety){
EEPROM.update(_localIndex+SAFETY_INDEX, safety);
}
void Heater::EEPROMUpdate(){
  if (EEPROMTimer > 10000) {
    EEPROMTimer = 0;

    switch (_mode){
      case FIX_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex+TEMP_INDEX)){
          setTempInEEPROM(_tempParameter);
        }
      break;
      case VAR_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex+TEMP_INDEX)-10){
          unsigned short adjustTempParameter = _tempParameter+10;
          setTempInEEPROM((byte) adjustTempParameter);
        }
      break;
    }

    if (_hyst != (float)EEPROM.read(_localIndex+HYST_INDEX)){
      setHystInEEPROM(_hyst);
    }
    if (_safety != (boolean)EEPROM.read(_localIndex+SAFETY_INDEX)){
      setSafetyInEEPROM(_safety);
    }
 }
}
