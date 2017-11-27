// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.

#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"
#include "GreenhouseLib_actuators.h"

//****************************************************************
//*******************ROLLUP FUNCTIONS*****************************
//****************************************************************

static unsigned short Rollup::_index = 0;

/*
Constructor : Define Opening and Closing pins on the arduino, to connect to the relay box
_routine (normal logic for the rollup applies if true)
_closing (closing pin is active if true)
_opening (same with opening pin)
_closingCycle (a closing cycle is going on, including pause time)
_closingCycle (same with closing)
_incrementCounter (what increment the rollup has reached)
incrementCounter is set to OFF_VAL so the program recognize the first opening/closing run.
_stage (what cooling stage the rollup has reached)
_localIndex (starting point for EEPROM savings)
_index (starting point for next object of the same class)
EEPROMTimer (checks every 10 seconds if EEPROM has to be updated)

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
/*
INIT OUTPUTS
mode:
- FIX_TEMP : target temperature is set within the class
- VAR_TEMP : target temperature is set outside the class
- MAN_TEMP : target temperature is set outisde the class and differ for each cooling stage
relayType:
- ACT_HIGH : relay is active when pin is high
- ACT_LOW : relay is active when pin is low
rOpen (pin connected to opening relay)
rClose (pin connected to closing relay)
*/

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
       Serial.println(F("------------"));
       Serial.print(F("Temperature :  "));
       Serial.println(temp);
       Serial.print(F("Opening temp : "));
       Serial.println(_tempParameter);
       Serial.print(F("Closing temp : "));
       Serial.println(_tempParameter-_hyst);
       Serial.println(F("-------------"));
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
        Serial.println(F("------------"));
        Serial.print(F("Temperature :  "));
        Serial.println(temp);
        Serial.print(F("Opening temp : "));
        Serial.println(target+_tempParameter);
        Serial.print(F("Closing temp : "));
        Serial.println(target+_tempParameter-_hyst);
        Serial.println(F("-------------"));
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
/*
Open or close the rollups to specific increment, using a maltiple cooling stages logic
Adjust to an external target temperature (Mode MAN_TEMP)
*/
void Rollup::manualRoutine(float target, float temp){
  if (_mode == MAN_TEMP){
  	if(_routine == true){
      float targetTemp = target + _tempParameter;

      #ifdef DEBUG_ROLLUP_TEMP
        Serial.println(F("------------"));
        Serial.print(F("Temperature :  "));
        Serial.println(temp);
        Serial.print(F("Stage 1 : Opening temp : "));
        Serial.println(targetTemp + _stageMod[1]);
        Serial.print(F("Stage 1 : Closing temp : "));
        Serial.println(targetTemp + _stageMod[1] -_hyst);
        Serial.print(F("Stage 2 : Opening temp : "));
        Serial.println(targetTemp + _stageMod[2]);
        Serial.print(F("Stage 2 : Closing temp : "));
        Serial.println(targetTemp + _stageMod[2] -_hyst);
        Serial.print(F("Stage 3 : Opening temp : "));
        Serial.println(targetTemp + _stageMod[3]);
        Serial.print(F("Stage 3 : Closing temp : "));
        Serial.println(targetTemp + _stageMod[3] -_hyst);
        Serial.print(F("Stage 4 : Opening temp : "));
        Serial.println(targetTemp + _stageMod[4]);
        Serial.print(F("Stage 4 : Closing temp : "));
        Serial.println(targetTemp + _stageMod[4] -_hyst);
        Serial.println(F("-------------"));
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


/*
Open to reach a specific increment
*/

void Rollup::openToInc(short stage, byte targetIncrement){

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
          Serial.println(F("-------------"));
          Serial.println(F("OPENING CYCLE");
          Serial.print(F("Increment :     "));
          Serial.println(_incrementCounter);
          Serial.print(F("Target increment :     "));
          Serial.println(targetIncrement);
        	Serial.print(F("Timer :         "));
        	Serial.println(rollupTimer);
        	Serial.print(F("Rotation time : "));
        	Serial.println((unsigned long)_rotationUp*1000/_increments*_move);
        	Serial.print(F("Pause time :    "));
        	Serial.println((unsigned long)_rotationUp*1000/_increments*_move+_pause*1000);
            Serial.println(F("-------------"));
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
/*
Close to reach a specific increment
*/
void Rollup::closeToInc(short stage, byte targetIncrement){

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }

  if ((_closing == false)&&(_closingCycle == false)){
      _move = (short)targetIncrement - (short)_incrementCounter;
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
        Serial.println(F("-------------"));
        Serial.println(F("CLOSING CYCLE"));
        Serial.print(F("Increment :     "));
        Serial.println(_incrementCounter);
        Serial.print(F("Target increment :     "));
        Serial.println(targetIncrement);
      	Serial.print(F("Timer :         "));
      	Serial.println(rollupTimer);
      	Serial.print(F("Rotation time : "));
      	Serial.println((unsigned long)_rotationDown*1000/_increments*(0-_move));
      	Serial.print(F("Pause time :    "));
      	Serial.println((unsigned long)_rotationDown*1000/_increments*(0-_move)+(unsigned long)_pause*1000);
          Serial.println(F("-------------"));
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

/*
Safety cycle :
safety closing or safety closing every X milliseconds (SAFETY_DELAY) to ensure rollup is effectively shut or fully open
*/

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
Activate or desactivate the routine function (can then be activated manually)
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
      Serial.println(F("-------------"));
      Serial.println(F("opening"));
      Serial.println(F("-------------"));
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
      Serial.println(F("-------------"));
      Serial.println(F("closing"));
      Serial.println(F("-------------"));
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
      Serial.println(F("-------------"));
      Serial.println(F("stop opening");
      Serial.println(F("-------------"));
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
      Serial.println(F("-------------"));
      Serial.println(F("stop closing"));
      Serial.println(F("-------------"));
    #endif
	}
}
/*
Open or close the rollups by one increment
*/
void Rollup::openSides(){
	#ifdef DEBUG_ROLLUP_TIMING
  if(_openingCycle == true){
    Serial.println(F("-------------"));
    Serial.println(F("OPENING CYCLE"));
  	Serial.print(F("Timer :         "));
  	Serial.println(rollupTimer);
  	Serial.print(F("Rotation time : "));
  	Serial.println(((unsigned long)_rotationUp*1000/_increments));
  	Serial.print(F("Pause time :    "));
  	Serial.println(((unsigned long)_rotationUp*1000/_increments+(unsigned long)_pause*1000));
    Serial.print(F("Increment :     "));
    Serial.println(_incrementCounter);
    Serial.println(F("-------------"));
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
    Serial.println(F("-------------"));
    Serial.println(F("ClOSING CYCLE"));
  	Serial.println(F("Timer :         "));
  	Serial.println(rollupTimer);
  	Serial.println(F("Rotation time : "));
  	Serial.println(((unsigned long)_rotationDown*1000/_increments));
  	Serial.println(F("Pause time : "));
  	Serial.println(((unsigned long)_rotationDown*1000/_increments+(unsigned long)_pause*1000));
    Serial.print(F("Increment :       "));
    Serial.println(_incrementCounter);
    Serial.println(F("-------------"));
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
    Serial.println(F("-------------"));
    Serial.println(F("start pause"));
    Serial.println(F("-------------"));
  #endif
}
void Rollup::printEndPause(){
  #ifdef DEBUG_ROLLUP_TIMING
    Serial.println(F("-------------"));
    Serial.println(F("end of the pause"));
    Serial.println(F("-------------"));
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

void Rollup::setParametersInEEPROM(short rTemp, byte rHyst, byte rotationUp, byte rotationDown, byte increments,byte pause, boolean safety){
  setTempInEEPROM(rTemp);
  setHystInEEPROM(rHyst);
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

void Rollup::setTempInEEPROM(short temp){
    switch (_mode){
      case FIX_TEMP:
          EEPROM.update(_localIndex+TEMP_INDEX, (byte)temp);
      break;
      case VAR_TEMP:
      case MAN_TEMP:
        //As tempParameter can be between -10 to 10, we add 10 to its value to convert it to "byte"
          byte adjustTemp = (byte)(temp+10);
          EEPROM.update(_localIndex+TEMP_INDEX, adjustTemp);
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
  	 #ifdef DEBUG_EEPROM
      Serial.println("ROLLUP TIMER : ");
      Serial.println(EEPROMTimer);
  	 #endif
     //Each 10 seconds
  if (EEPROMTimer > 10000) {
    //Reset timer
    EEPROMTimer = 0;
    //Check if variables match EEPROM saved values
    switch (_mode){
      case FIX_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex)){
          setTempInEEPROM(_tempParameter);
        }
      break;
      case VAR_TEMP:
      case MAN_TEMP:
        if (_tempParameter != (float)EEPROM.read(_localIndex)-10){
          byte adjustTempParameter = (byte)(_tempParameter + 10);
          setTempInEEPROM(adjustTempParameter);
        }
      break;
    }

    if (_hyst != (float)EEPROM.read(_localIndex+HYST_INDEX)){
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

    #ifdef DEBUG_EEPROM
    Serial.println(F("Rollup parameters saved in EEPROM"));
    //Print actual EEPROM values
    for(int x = _localIndex; x < _localIndex+PAUSE_INDEX; x++){
     Serial.print(F("Byte #")); Serial.print(x); Serial.print(": ");
     Serial.println(EEPROM.read(x));
    }
    #endif
  }
}

//Return private variables

float Rollup::hyst(){
  return _hyst;
}
float Rollup::tempParameter(){
  return _tempParameter;
}
unsigned short Rollup::rotationUp(){
  return _rotationUp;
}
unsigned short Rollup::rotationDown(){
  return _rotationDown;
}
unsigned short Rollup::increments(){
  return _increments;
}
unsigned short Rollup::pause(){
  return _pause;
}
boolean Rollup::safety(){
  return _safety;
}
unsigned short Rollup::incrementCounter(){
  return _incrementCounter;
}
byte Rollup::mode(){
  return _mode;
}
boolean Rollup::opening(){
  return _opening;
}
boolean Rollup::closing(){
  return _closing;
}
boolean Rollup::openingCycle(){
  return _openingCycle;
}
boolean Rollup::closingCycle(){
  return _closingCycle;
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
    #ifdef DEBUG_FAN
      Serial.println(F("Fan : off"));
    #endif
  }
}

void Fan::start(){
  if(digitalRead(_pin) == LOW){
    digitalWrite(_pin, HIGH);
    #ifdef DEBUG_FAN
    Serial.println(F("Fan : on"));
    #endif
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
  _tempParameter = temp;
}

void Fan::setSafety(boolean safety){
  _safety = safety;
}

void Fan::setParametersInEEPROM(short temp, byte hyst, boolean safety){
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

void Fan::setTempInEEPROM(short temp){
    switch (_mode){
      case FIX_TEMP:
          EEPROM.update(_localIndex+TEMP_INDEX, temp);
      break;
      case VAR_TEMP:
          byte adjustTemp = (byte)(temp+10);
          EEPROM.update(_localIndex+TEMP_INDEX, adjustTemp);
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
  	 #ifdef DEBUG_EEPROM
      Serial.println(F("FAN EEPROM TIMER : "));
      Serial.println(EEPROMTimer);
  	 #endif
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
    #ifdef DEBUG_EEPROM
    Serial.println(F("Fan parameters saved in EEPROM"));
    //Print actual EEPROM values
    for(int x = _localIndex; x < _localIndex+SAFETY_INDEX; x++){
     Serial.print(F("Byte #")); Serial.print(x); Serial.print(F(": "));
     Serial.println(EEPROM.read(x));
    }
    #endif
 }
}
byte Fan::mode(){
  return _mode;
}
byte Fan::pin(){
  return _pin;
}
float Fan::hyst(){
  return _hyst;
}
float Fan::tempParameter(){
  return _tempParameter;
}
boolean Fan::safety(){
  return _safety;
}
boolean Fan::debug(){
  return _debug;
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
    #ifdef DEBUG_HEATER
      Serial.println(F("Heater : off"));
    #endif
  }
}

void Heater::start(){
  if(digitalRead(_pin) == LOW){
    digitalWrite(_pin, HIGH);
    #ifdef DEBUG_HEATER
      Serial.println(F("Heater : on"));
    #endif
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
  _tempParameter = temp;
}

void Heater::setSafety(boolean safety){
  _safety = safety;
}

void Heater::setParametersInEEPROM(short temp, byte hyst, boolean safety){
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

void Heater::setTempInEEPROM(short temp){
    switch (_mode){
      case FIX_TEMP:
          EEPROM.update(_localIndex+TEMP_INDEX, temp);
      break;
      case VAR_TEMP:
          byte adjustTemp = (byte)(temp+10);
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
  #ifdef DEBUG_EEPROM
    Serial.println(F("HEATER EEPROM TIMER : "));
    Serial.println(EEPROMTimer);
  #endif
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
    #ifdef DEBUG_EEPROM
      Serial.println("Heater parameters saved in EEPROM");
      //Print actual EEPROM values
      for(int x = _localIndex; x < _localIndex+SAFETY_INDEX; x++){
        Serial.print(F("Byte #")); Serial.print(x); Serial.print(F(": "));
        Serial.println(EEPROM.read(x));
      }
    #endif
 }
}
