//Uncomment to enable serial communication
//#define DEBUG

//Libraries
#include "Arduino.h"
#include <EEPROM.h>
#include "GreenhouseLib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <MENWIZ.h>

//Pinout
#define BUTTON_PAD      A0  //connect this pin to an analog four button pad
#define ONE_WIRE_BUS    A1 //connect this pin to the DS18B20 data line
#define MENU_PIN        2  //link this pin to ground with an on/off switch in between
#define OPENING_PIN     11 //connect this pin to the opening relay
#define CLOSING_PIN     10 //connect this pin to the closing relay

//Create rollup object
Rollup rollup1; //Pin 11 => opening relay; pin 10 => closing relay

//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Create lcd object using LiquidCrystal lib
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//Create menu object
menwiz menu;

//Debounce/delays/avoid repetitions in printing
unsigned long previousMillis = 0;
const long debugInterval = 1000;  //Send serial data every...
const long debouncingDelay = 80;  //button debouncing delay (range between 50 and 100 for faster or slower response)
boolean firstPrint = true;  //tells if the text on the LCD has been printed already
boolean control = true;     //tells if in mode control or menu

float targetTemp = 20;      //target temperature for the greenhouse

void setup() {


  Serial.begin(9600);//start serial communication


//declare inputs
  pinMode(BUTTON_PAD, INPUT_PULLUP);
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  pinMode(MENU_PIN, INPUT_PULLUP);

  sensors.begin(); //start communication with temp probe
  rollup1.initOutputs(FIX_TEMP,OPENING_PIN,CLOSING_PIN);

// IF IT IS YOUR FIRST UPLOAD  ----->

  //Uncomment the following line to load new parameters in EEPROM,
  //rollup1.setParametersinEEPROM(25, 1, 25, 25, 5, 5, true);

//Then put back the comment markers and upload again to allow new settings

  //PARAMETERS :
  //Activation temperature : 25C
  //hysteresis : 1C
  //Rotation time (Up): 25 sec
  //Rotation time (Down): 25 sec
  //Increments : 5
  //Pause between rotation : 5
  //Safety mode : ON

  rollup1.loadEEPROMParameters(); //read EEPROM values to define member variables
  createMenu(); //Create the menu structure
}

void loop() {
//MODE MENU
  if (digitalRead(MENU_PIN) == LOW) {
    menu.draw();  //execute the menu program
    control = false;
  }

//MODE CONTROLE
  else if (digitalRead(MENU_PIN) == HIGH) {
    lcdDisplay();
    rollup1.routine(greenhouseTemperature()); //execute the routine programme
    control = true;
  }

  printState(); //Print parameters of the rollup through serial line (if DEBUG is defined)
  rollup1.EEPROMUpdate(); //Update EEPPROM settings if necessary
}


void printState(){
  #ifdef DEBUG
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= debugInterval) {
    previousMillis = currentMillis;
    Serial.print(F("Temperature : "));
    Serial.println(greenhouseTemperature());
    Serial.print(F("Parametre de temperature : "));
    Serial.println(rollup1._tempParameter);
    Serial.print(F("Hysteresis : "));
    Serial.println(rollup1._hyst);
    Serial.print(F("Rotation(haut) : "));
    Serial.println(rollup1._rotationUp);
    Serial.print(F("Rotation(bas) : "));
    Serial.println(rollup1._rotationDown);
    Serial.print(F("Pause : "));
    Serial.println(rollup1._pause);
    Serial.print(F("Increments : "));
    Serial.println(rollup1._increments);
    Serial.print(F("Securite : "));
    Serial.println(rollup1._safety);
    Serial.println("");
  }
  #endif
}

void createMenu(){
  _menu *r,*s1,*s2;

  menu.begin(&lcd,16,2); //declare lcd object and screen size to menwiz lib

  r = menu.addMenu(MW_ROOT,NULL,F("Menu principal"));

  /***************************************************
  *******************MENU CAPTEURS********************
  ***************************************************/

    s1 = menu.addMenu(MW_VAR,r, F("Capteurs"));
      s1 ->addVar(MW_ACTION, displayTemp);
      s1 ->setBehaviour(MW_ACTION_CONFIRM, false);

  /***************************************************
  *******************MENU ROLLUPS*********************
  ***************************************************/

    s1 = menu.addMenu(MW_SUBMENU,r, F("Rollups"));
      s2 = menu.addMenu(MW_VAR, s1, F("Etat"));
        s2 ->addVar(MW_ACTION, displayRollup);
        s2 ->setBehaviour(MW_ACTION_CONFIRM, false);
      s2 = menu.addMenu(MW_VAR, s1, F("Def. temp mod"));
        s2 ->addVar(MW_AUTO_FLOAT,&rollup1._tempParameter,-10,10,1);
      s2 = menu.addMenu(MW_VAR, s1, F("Def. hyst"));
        s2 ->addVar(MW_AUTO_FLOAT,&rollup1._hyst,0,5,1);
      s2 = menu.addMenu(MW_VAR, s1, F("Def. rotation haut"));
        s2 ->addVar(MW_AUTO_INT,&rollup1._rotationUp,0,240,1);
      s2 = menu.addMenu(MW_VAR, s1, F("Def. rotation bas"));
        s2 ->addVar(MW_AUTO_INT,&rollup1._rotationDown,0,240,1);
      s2 = menu.addMenu(MW_VAR, s1, F("Def. increments"));
        s2 ->addVar(MW_AUTO_INT,&rollup1._increments,0,5,1);
      s2 = menu.addMenu(MW_VAR, s1, F("Def. securite"));
        s2 ->addVar(MW_BOOLEAN,&rollup1._safety);

    menu.addUsrNav(buttonState, 4);//Use 4 custom buttons
}


void displayTemp(){
  while(buttonState() != MW_BTE){
    if(firstPrint == true){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Capteurs");
      lcd.setCursor(0,1);
      lcd.print("Temp :");
      lcd.print(greenhouseTemperature());
      lcd.print("C");
      firstPrint = false;
    }
  }
  if(firstPrint == false){firstPrint = true;};
}

void displayRollup(){
  while(buttonState() != MW_BTE){
    if(firstPrint == true){
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print(F("Inc:")); lcd.print(rollup1._increments); lcd.print(F(""));; lcd.setCursor(8,0);lcd.print(F("|P:")); lcd.print(rollup1._pause); lcd.print(F("s"));
      lcd.setCursor(0, 1); lcd.print(F("R:")); lcd.print(rollup1._rotationUp); lcd.print(F("s")); lcd.setCursor(8,1);lcd.print(F("|T:")); lcd.print(rollup1._tempParameter);lcd.print(F("C"));

      firstPrint = false;
    }
  }
  if(firstPrint == false){firstPrint = true;};
}

int buttonState() {
  int x = readButtonState();
  delay(debouncingDelay);
  int y = readButtonState();
  if (x == y){return y;}
}

int readButtonState(){
  int x = analogRead(A0);
  if (x < 50)       {    return MW_BTC;}    //Button confirm
  else if (x < 250) {    return MW_BTU;}    //Button up
  else if (x < 450) {    return MW_BTD;}    //Button down
  else if (x < 650) {    return MW_BTE;}    //Button exit
  else              {    return MW_BTNULL;} //No button
}

float greenhouseTemperature(){
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  return temp;
}

void lcdDisplay() {
  if(control == false){
    lcd.clear();
  }
  lcdPrintTemp();
  lcdPrintTempCible();
  lcdPrintRollups();
  //lcdPrintTime();
  //lcdPrintOutputsStatus();
}


void lcdPrintRollups(){
    lcd.setCursor(0, 1); lcd.print(F("         "));
    lcd.setCursor(0, 1); lcd.print(F("Rollups: "));
    lcd.setCursor(8, 1);
    if(rollup1._incrementCounter == 100){lcd.print(F(""));}
    else{lcd.print(rollup1._incrementCounter);}
 }
void lcdPrintTemp(){
    float temperature = greenhouseTemperature();
    lcd.setCursor(0,0); lcd.print(F("         "));
    lcd.setCursor(0,0); lcd.print(temperature); lcd.print(F("C"));
}

void lcdPrintTempCible(){
      lcd.setCursor(8,0);lcd.print("|");lcd.print(targetTemp + rollup1._tempParameter); lcd.print(F("C "));
}

void lcdPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  lcd.print("0");
  lcd.print(digits);
}

void serialPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  Serial.print("0");
}