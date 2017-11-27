#include "Arduino.h"
#include "GreenhouseLib.h"

Greenhouse::Greenhouse(int timezone, float latitude, float longitude, byte timepoints, byte rollups, byte fans, byte heaters){
  _timepoints = timepoints;
  _rollups = rollups;
  _fans = fans;
  _heaters = heaters;
  _timezone = timezone;
  _latitude = latitude;
  _longitude = longitude;
  _ramping = 300000;
}

Greenhouse::~Greenhouse(){
}

void Greenhouse::setNow(byte rightNow[6]){
  for(byte x = 0; x < 6 ; x++){
      _rightNow[x] = rightNow[x];
  }
    myLord.DST(_rightNow);
}

void Greenhouse::fullRoutine(byte rightNow[6], float greenhouseTemperature){
  solarCalculations();
  selectActualProgram();
  startRamping();
  for (byte x = 0; x < _rollups; x++){

    rollup[x].manualRoutine(_coolingTemp, greenhouseTemperature);
  }

  for (byte x = 0; x < _fans; x++){

    fan[x].routine(_coolingTemp, greenhouseTemperature);
  }
  for (byte x = 0; x < _heaters; x++){

    heater[x].routine(_heatingTemp, greenhouseTemperature);
  }
}
void Greenhouse::EEPROMUpdate(){
  for (byte x = 0; x < _rollups; x++){

    rollup[x].EEPROMUpdate();
  }

  for (byte x = 0; x < _fans; x++){

    fan[x].EEPROMUpdate();
  }
  for (byte x = 0; x < _heaters; x++){

    heater[x].EEPROMUpdate();
  }
}

void Greenhouse::solarCalculations(){
  initTimeLord(_timezone, _latitude, _longitude);
  //Première lecture d'horloge pour définir le lever et coucher du soleil
  setSunrise();
  setSunset();
}

void Greenhouse::initTimeLord(int timezone, float latitude, float longitude){
  myLord.TimeZone(timezone * 60);
  myLord.Position(latitude, longitude);
  myLord.DstRules(3,2,11,1,60); // DST Rules for USA
}


void Greenhouse::setSunrise(){
  //Définit l'heure du lever et coucher du soleil
  for(byte x = 0; x < 6 ; x++){
      _sunTime[x] = _rightNow[x];
  }
  myLord.SunRise(_sunTime); ///On détermine l'heure du lever du soleil
  myLord.DST(_sunTime);//ajuster l'heure du lever en fonction du changement d'heure
  Timezone::sunRise[HEURE] = (short)_sunTime[HEURE];
  Timezone::sunRise[MINUTE] = (short)_sunTime[MINUTE];

  #ifdef DEBUG_SOLARCALC
    Serial.print("lever du soleil :");Serial.print(Timezone::sunRise[HEURE]);  Serial.print(":");  Serial.println(Timezone::sunRise[MINUTE]);
  #endif
}

void Greenhouse::setSunset(){
  // Sunset:
  for(byte x = 0; x < 6 ; x++){
      _sunTime[x] = _rightNow[x];
  }
  myLord.SunSet(_sunTime); // Computes Sun Set. Prints:
  myLord.DST(_sunTime);
  Timezone::sunSet[HEURE] = (short)_sunTime[HEURE];
  Timezone::sunSet[MINUTE] = (short)_sunTime[MINUTE];
  #ifdef DEBUG_SOLARCALC
    Serial.print("coucher du soleil :");  Serial.print(Timezone::sunSet[HEURE]);  Serial.print(":");  Serial.println(Timezone::sunSet[MINUTE]);
  #endif
}

void Greenhouse::startingParameters(){
  //Exécution du programme
  selectActualProgram();
  setTempCible();
  ramping = 0;
}

void Greenhouse::selectActualProgram(){
  //Sélectionne le programme en cour
    #ifdef DEBUG_PROGRAM
      Serial.println("----");
      Serial.print ("Heure actuelle ");Serial.print(" : ");Serial.print(_rightNow[HEURE] );Serial.print(" : ");Serial.println(_rightNow[MINUTE]);
    #endif
    for (byte y = 0; y < (_timepoints-1); y++){

    #ifdef DEBUG_PROGRAM
      Serial.print ("Programme "); Serial.print(y+1);Serial.print(" : ");Serial.print(P[y][HEURE]);Serial.print(" : ");Serial.println(P[y][MINUTE]);
    #endif
      if (((_rightNow[HEURE] == timepoint[y].hr())  && (_rightNow[MINUTE] >= timepoint[y].mn()))||((_rightNow[HEURE] > timepoint[y].hr()) && (_rightNow[HEURE] < timepoint[y+1].hr()))||((_rightNow[HEURE] == timepoint[y+1].hr())  && (_rightNow[MINUTE] <timepoint[y+1].mn()))){
          _timepoint = y+1;
        }
    }

    #ifdef DEBUG_PROGRAM
      Serial.print ("Programme ");Serial.print(_timepoints);Serial.print(" : ");Serial.print(P[_timepoints-1][HEURE]);Serial.print(" : ");Serial.println(P[_timepoints-1][MINUTE]);
    #endif

    if (((_rightNow[HEURE] == timepoint[_timepoints-1].hr())  && (_rightNow[MINUTE] >= timepoint[_timepoints-1].mn()))||(_rightNow[HEURE] > timepoint[_timepoints-1].hr())||(_rightNow[HEURE] < timepoint[0].hr())||((_rightNow[HEURE] == timepoint[0].hr())  && (_rightNow[MINUTE] < timepoint[0].mn()))){
      _timepoint = _timepoints;
    }
    #ifdef DEBUG_PROGRAM
      Serial.print ("Program is : ");
      Serial.println(_timepoint);
    #endif
}

void Greenhouse::setTempCible(){
  _coolingTemp = timepoint[_timepoint-1].coolingTemp();
  _heatingTemp = timepoint[_timepoint-1].heatingTemp();
}

void Greenhouse::startRamping(){
  //Définition des variables locales
  float newHeatingTemp;
  float newCoolingTemp;

  newHeatingTemp = timepoint[_timepoint-1].heatingTemp();
  newCoolingTemp = timepoint[_timepoint-1].coolingTemp();
  if (ramping > _ramping){
    if (newCoolingTemp > _coolingTemp){
      _coolingTemp += 0.5;
    }
    else if (newCoolingTemp < _coolingTemp){
      _coolingTemp -= 0.5;
    }
    if (newHeatingTemp > _heatingTemp){
      _heatingTemp += 0.5;
    }
    else if (newHeatingTemp < _heatingTemp){
      _heatingTemp -= 0.5;
    }
    ramping = 0;
  }
}
