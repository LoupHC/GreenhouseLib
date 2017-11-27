// Created by Loup Hébert-Chartrand on 12/06/17.
// Copyright 2017 - Under creative commons license
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.

#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"
#include "GreenhouseLib_timing.h"


//****************************************************************
//*******************TIMEZONES FUNCTIONS**************************
//****************************************************************
Timezone::Timezone(){
  _localIndex = TIMEZONE_INDEX + _index;
  _index += 5;
  EEPROMTimer = 0;
}

Timezone::~Timezone(){}

void Timezone::setParameters(byte type, short mod, float heatingTemp, float coolingTemp){
	setType(type);
	setTime(mod);
	setHeatTemp(heatingTemp);
	setCoolTemp(coolingTemp);
	setMod(mod);
}
void Timezone::setParameters(byte type, byte hour, byte min, float heatingTemp, float coolingTemp){
	setType(type)	;
	setTime(hour,min);
	setHeatTemp(heatingTemp);
	setCoolTemp(coolingTemp);
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
void Timezone::setHeatTemp(float heatingTemp){
	_heatingTemp = heatingTemp;
}
void Timezone::setCoolTemp(float coolingTemp){
	_coolingTemp = coolingTemp;
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

  setHeatTemp((float)EEPROM.read(_localIndex+HEAT_INDEX));
  setCoolTemp((float)EEPROM.read(_localIndex+COOL_INDEX));
}

void Timezone::setParametersInEEPROM(byte type, short mod, float heatingTemp, float coolingTemp){
	setTimeInEEPROM(type, mod);
	setHeatTempInEEPROM(heatingTemp);
	setCoolTempInEEPROM(coolingTemp);
}

void Timezone::setParametersInEEPROM(byte type, byte hour, byte min, float heatingTemp, float coolingTemp){
	setTimeInEEPROM(type, hour, min);
	setHeatTempInEEPROM(heatingTemp);
	setCoolTempInEEPROM(coolingTemp);
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

void Timezone::setHeatTempInEEPROM(float heatingTemp){
		EEPROM.update(_localIndex+COOL_INDEX, heatingTemp);
}
void Timezone::setCoolTempInEEPROM(float coolingTemp){
		EEPROM.update(_localIndex+COOL_INDEX, coolingTemp);
}
#include "GreenhouseLib_actuators.h"


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
    if(_heatingTemp != EEPROM.read(_localIndex+ HEAT_INDEX)){
    		EEPROM.update(_localIndex+HEAT_INDEX, _heatingTemp);
    }
    if(_coolingTemp != EEPROM.read(_localIndex+ COOL_INDEX)){
    		EEPROM.update(_localIndex+COOL_INDEX, _coolingTemp);
    }
 }
}

unsigned short Timezone::hr(){
  return _hour;
}
unsigned short Timezone::mn(){
  return _min;
}
float Timezone::heatingTemp(){
  return _heatingTemp;
}
float Timezone::coolingTemp(){
  return _coolingTemp;
}

static unsigned short Timezone::_index = 0;

static short Timezone::sunRise[3] = {0};
static short Timezone::sunSet[3] = {0};


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
