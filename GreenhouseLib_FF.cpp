/*
  GreenhouseLib_FF.cpp

  You can find the latest version of this code at :
  https://github.com/LoupHC/GreenhouseLib

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "GreenhouseLib_FF.h"

//****************************************************
//******************FAN FUNCTIONS************************
//****************************************************************/

Fan::Fan(){
    _tempParameter.setLimits(-5, 10);
    _hyst.setLimits(0,5);
    _hyst.maximum(5);

    _debug = false;
    _routine = true;
    _localIndex = FAN_INDEX + _index;
    _index += 3;
    EEPROMTimer = 0;
}

static unsigned short Fan::_index = 0;

Fan::~Fan(){}
void Fan::initFan(){
    _tempParameter.setLimits(-5, 10);
    _hyst.setLimits(0,5);
    _hyst.maximum(5);

    _debug = false;
    _routine = true;
    _localIndex = FAN_INDEX + _index;
    _index += 3;
    EEPROMTimer = 0;
}

void Fan::initOutput(byte mode, byte pin){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    _pin = pin;
	 _mode = mode;
   if (_mode == FIX_TEMP){
     _tempParameter.minimum(0);
     _tempParameter.maximum(50);
   }
   else if(_mode = VAR_TEMP){
     _tempParameter.minimum(-5);
     _tempParameter.maximum(10);
   }
 }

//action functions
/*
Start or stop the fan when a certain temperature, defined within the class itself, is reached
(Mode FIX_TEMP)
*/
void Fan::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
      if (temp < (_tempParameter.value() - _hyst.value())) {
        	stop();
      } else if (temp > _tempParameter.value()) {
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
      float activationTemp = target + _tempParameter.value();
      if (temp < (activationTemp - _hyst.value())) {
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
  _hyst.setValue(hyst);
}

void Fan::setTemp(float temp){
  _tempParameter.setValue(temp);
}

void Fan::setSafety(boolean safety){
  _safety = safety;
}
/*
void Fan::setParametersInEEPROM(short temp, byte hyst, boolean safety){
  setHystInEEPROM(hyst);
  setTempInEEPROM(temp);
  setSafetyInEEPROM(safety);
}
/*
load last saved parameters

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
        if (_tempParameter.value() != (float)EEPROM.read(_localIndex+TEMP_INDEX)){
          setTempInEEPROM(_tempParameter.value());
        }
      break;
      case VAR_TEMP:
        if (_tempParameter.value() != (float)EEPROM.read(_localIndex+TEMP_INDEX)-10){
          unsigned short adjustTempParameter = _tempParameter.value()+10;
          setTempInEEPROM((byte) adjustTempParameter);
        }
      break;
    }

    if (_hyst.value() != (float)EEPROM.read(_localIndex+HYST_INDEX)){
      setHystInEEPROM(_hyst.value());
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
*/
byte Fan::mode(){
  return _mode;
}
byte Fan::pin(){
  return _pin;
}
float Fan::hyst(){
  return _hyst.value();
}
float Fan::tempParameter(){
  return _tempParameter.value();
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
    _tempParameter.setLimits(-10, 5);
    _hyst.setLimits(0,5);
    _hyst.maximum(5);

	  _debug = false;
    _routine = true;
    _localIndex = HEATER_INDEX + _index;
    _index += 3;
    EEPROMTimer = 0;
}

static unsigned short Heater::_index = 0;

Heater::~Heater(){}

void Heater::initHeater(){
    _tempParameter.setLimits(-10, 5);
    _hyst.setLimits(0,5);
    _hyst.maximum(5);

    _debug = false;
    _routine = true;
    _localIndex = HEATER_INDEX + _index;
    _index += 3;
    EEPROMTimer = 0;
}

void Heater::initOutput(byte mode, byte pin){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    _pin = pin;
    _mode = mode;
    if (_mode == FIX_TEMP){
      _tempParameter.minimum(0);
      _tempParameter.maximum(50);
    }
    else if(_mode = VAR_TEMP){
      _tempParameter.minimum(-10);
      _tempParameter.maximum(5);
    }
 }

//action functions
/*
Start or stop the heater when a certain temperature, defined within the class itself, is reached
(Mode FIX_TEMP)
*/
void Heater::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
      if (temp > (_tempParameter.value() + _hyst.value())) {
        	stop();
      } else if (temp < _tempParameter.value()) {
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
      float activationTemp = target + _tempParameter.value();
      if (temp > (activationTemp + _hyst.value())) {
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
  _hyst.setValue(hyst);
}

void Heater::setTemp(float temp){
  _tempParameter.setValue(temp);
}

void Heater::setSafety(boolean safety){
  _safety = safety;
}
/*
void Heater::setParametersInEEPROM(short temp, byte hyst, boolean safety){
  setHystInEEPROM(hyst);
  setTempInEEPROM(temp);
  setSafetyInEEPROM(safety);
}

//load last saved parameters

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
        if (_tempParameter.value() != (float)EEPROM.read(_localIndex+TEMP_INDEX)){
          setTempInEEPROM(_tempParameter.value());
        }
      break;
      case VAR_TEMP:
        if (_tempParameter.value() != (float)EEPROM.read(_localIndex+TEMP_INDEX)-10){
          unsigned short adjustTempParameter = _tempParameter.value()+10;
          setTempInEEPROM((byte) adjustTempParameter);
        }
      break;
    }

    if (_hyst.value() != (float)EEPROM.read(_localIndex+HYST_INDEX)){
      setHystInEEPROM(_hyst.value());
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
*/
