#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

/// PINS
const int ENCSW = 2;         // Encoder Switch
const int ENCDT = 3;         // Encoder Data
const int ENCCLK = 4;        // Encoder Clock
const int BUZZERPIN = 5;     // Buzzer
const int VENTPIN  = 12;      // Magnetventil
const int REDPIN = 11;           // LED Rot
const int BLUEPIN = 9;           // LED Blau
const int GREENPIN = 10;          // LED Grün
const int FLOATPIN = 8;      // Anti-Schaum-Schwimmer normally open
const int FIREPIN =  6;       // Zündknopf für Orgeln

const int PRESSURESENS = A2;        /// Drucksensor
const int COMPRESSORRELY = 13;      ///  Kompresssor Relais
const int FLOWSENS = 7;            /// Durchflusssensor


// KONSTANTEN
const int DISPREFRESHTIME = 100;

/// ENCODER VARIABLEN
int encPosCount = 0;
int encCLKzuletzt = 0;
int encCLK = 0;
int encDT = 0;
int encSW = 0;
int encoderPosCount = 0;

bool pressureFlagSet = false;
bool pressureFlagFilled = false;
float pressure = 0;


long disIndex = 0;
long oldPos = 0;
long newPos;

volatile bool right;
volatile bool left;

unsigned long timeDispLast = 0;

// Variablen für Countdown
int countdown = 0;
bool countdownFlag = false;



long imp = 0;
long startTime = 0;


Encoder myEnc(ENCDT, ENCCLK);



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////BUZZER ERROR SIGNAL//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void buzzerErrorLight()
{
  int duration = 150;

  for (int i = 2; i >= 0; i--)
  {
    pause(duration);
    analogWrite(REDPIN, 255);
    digitalWrite(BUZZERPIN, HIGH);
    pause(duration);
    analogWrite(REDPIN, 0);
    digitalWrite(BUZZERPIN, LOW);
    pause(duration);
    analogWrite(REDPIN, 255);
    digitalWrite(BUZZERPIN, HIGH);
    pause(duration);
    analogWrite(REDPIN, 0);
    digitalWrite(BUZZERPIN, LOW);
    pause(duration);
  }

  return;


}

//////////////////////////////////////////////////////////////////////////
/////////////////////// FEHLER FÜR STATUS/////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void error(int pos)
{
  switch (pos)
  {
    case 1:

      printTwoStrings("Fehler! Bitte", "Druck Einstellen");
      buzzerErrorLight();
      pause(3000);
      break;

    case 2:

      printTwoStrings("Fehler! Bitte", "Druck Auftanken");
      buzzerErrorLight();
      pause(3000);
      break;

    case 3:

      printTwoStrings("Fehler! Bitte", "Bier einfuellen");
      buzzerErrorLight();
      pause(3000);
      break;

    case 4:

      printTwoStrings("Fehler! Bitte", "Countdown setzen");
      buzzerErrorLight();
      pause(3000);
      break;




    default:
      break;


  }
  return;
}


//////////////////////////////////////////////////////////////////////////
//////////// PRINT STRINGS IN DISPLAY///// ///////////////////////////////
/////////////////////////////////////////////////////////////////////////

void printTwoStrings (const char* LINE1, const char* LINE2)
{
  if (millis() - timeDispLast > DISPREFRESHTIME)
  {
    timeDispLast = millis();
    lcd.noBlink();
    lcd.clear();
    lcd.setCursor (0, 0);
    lcd.print (LINE1);
    lcd.setCursor (0, 1);
    lcd.print (LINE2);
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////PAUSE///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void pause(unsigned long intervall)
{
  unsigned long intervallEnd = millis() + intervall;
  while (millis() < intervallEnd)
  {
    ;
  }
}

///////////////////////////////////////////////////////////////////////////
//////////////////////////KNOPF ENCODER////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

long encoderSwitch (long a, long target) {
  if (digitalRead(ENCSW) == LOW) {
    a = target;
    Serial.println(target);
    pause(500);
  }
  return a;
}

///////////////////////////////////////////////////////////////////////////
/////////////////////////////ENCODER DREHUNG///////////////////////////////
///////////////////////////////////////////////////////////////////////////


void encoderChangeDisplay(float target1, float target2) {
  encCLK = digitalRead(ENCCLK);
  if (encCLK != encCLKzuletzt && encCLK == LOW) {

    if (digitalRead(ENCDT) == HIGH) {
      disIndex = target2;
    }
    else {
      disIndex = target1;
    }
  }
  encCLKzuletzt = encCLK;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////// RIGHT/LEFT AUßERHALB DES MENÜS///////////////////////



void checkRight()
{
  if (newPos >= (oldPos + 4 ))                 /// NACH AAUFRUFEN MUSS RIGHT WIEDER AUF FALSE !!!!
  {

    oldPos = newPos;
    right = true;
    return;

  }
  return;
}

void checkLeft()
{

  if (newPos <= (oldPos - 4 ))                   ///// NACH AUFRUFEN MUSS LEFT WIEDER AUF FALSE !!!!
  {
    oldPos = newPos;
    left = true;
    return;
  }


  return;

}
//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// SCWIMMERSTATUS HOLEN//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool getFlowStatus()
{
  bool isTankFull;
  int flowStatus = digitalRead(FLOATPIN);

  if (flowStatus == LOW)
  {
    isTankFull = false;
    return isTankFull;
  }

  if (flowStatus == HIGH)
  {
    isTankFull = true;
    return isTankFull;
  }

}
//////////////////////////////////////////////////////////////////////////////////
////////////////// LED STEUERUNG//////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*

void ledCtrlIdle()
{


  int r, g, b;
  unsigned long int fadespeed = random(50, 150);

  while (1)
  {


    for (r = 0; r < 256; r++)
    { // BLAU -> ViOLETT
      analogWrite(REDPIN, r);
      pause(fadespeed);
    }

    for (b = 255; b > 0; b--)
    { // VIOLETT -> ROT
      analogWrite(BLUEPIN, b);
      pause(fadespeed);
    }
    for (g = 0; g < 256; g++)
    { // ROT -> GELB
      analogWrite(GREENPIN, g);
      pause(fadespeed);
    }

    for (r = 255; r > 0; r--)
    { // GELB -> GRÜN
      analogWrite(REDPIN, r);
      pause(fadespeed);
    }

    for (b = 0; b < 256; b++)
    { // GRÜN -> BLAUGRÜN
      analogWrite(BLUEPIN, b);
      pause(fadespeed);
    }

    for (g = 255; g > 0; g--)
    { // BLAUGRÜN -> BLAU
      analogWrite(GREENPIN, g);
      pause(fadespeed);


    }

  }
}
*/
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////ORGELN//////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void orgeln()
{
  bool tankFull = getFlowStatus();
  int i = countdown;
  int brightness = 250 / i;
  int hellcount = brightness;
  bool ctrl = true;
  bool pushed = false;



  printTwoStrings("Pruefe", "System...");

  pause(500);


  if (pressureFlagSet == false)
  {
    error(1);
    return;


  }
  if (pressureFlagFilled == false)
  {
    error(2);
    return;
  }

  if (tankFull == false)
  {
    error(3);
    return;
  }

  if (countdownFlag == false)
  {
    error(4);
    return;
  }

  showSetting();


  while (ctrl)
  {
    printTwoStrings("Bereit zum", "Orgeln !");
    analogWrite(GREENPIN, 255);
    pause(100);
    analogWrite(GREENPIN, 0);

    if (digitalRead(FIREPIN) == HIGH)
    {
      ctrl = false;
    }

  }

  pause(300);

  printTwoStrings("Wohl bekommts !", "Wohl bekommts !");


  for (i; i > 0; i--)
  {
    analogWrite(REDPIN, 255);
    digitalWrite(BUZZERPIN, HIGH);
    pause(500);

    if (i == 1)
    {
      pause(1000);
    }
    analogWrite(REDPIN, 0);

    digitalWrite(BUZZERPIN, LOW);
    pause(500);

  }

  analogWrite(REDPIN, 0);
  digitalWrite(BUZZERPIN, LOW);





  while (digitalRead(FLOATPIN) == HIGH)
  {
    analogWrite(GREENPIN, 255);;
    digitalWrite(VENTPIN, HIGH);

  }

  analogWrite(GREENPIN, 0);
  digitalWrite(VENTPIN, LOW);



  lcd.clear();

  return;
}

/////////////////////////////////////////////////////////////////////////////////
////////////////////// SET PRESSURE//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void setPressure()
{
  myEnc.write(0);
  Serial.println("newpos = "); 
  Serial.println(newPos);
  Serial.println("oldpos = ");
  Serial.println(oldPos);
  
  bool ctrl = true;


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Druck Einstellen");
  lcd.setCursor(0, 1);
  lcd.print("auf ");
  lcd.setCursor(11, 1);
  lcd.print("Bar");
  lcd.setCursor(5, 1);
  lcd.print(pressure, 1);


  while (ctrl)
  {

    newPos = myEnc.read();

  //  Serial.println(newPos); 

    checkLeft();
    checkRight();

    lcd.setCursor(5, 1);
    lcd.print(pressure);


    if (right && pressure < 5)
    {
      pressure += 0.5;

    }
    if (left && pressure > 0)
    {
      pressure -= 0.5;
    }



    if (digitalRead(ENCSW) == LOW)
    {

      pressureFlagSet = true;
      oldPos = newPos;
      ctrl = false;

    }

   
    left = false;
    right = false;

  }
  Serial.println("newpos = "); 
  Serial.println(newPos);
  Serial.println("oldpos = ");
  Serial.println(oldPos);

  printTwoStrings("Druck", "Gespeichert !");

  pause(2000);

  return;
}
/////////////////////////////////////////////////////////////////////////////////
///////////////// PRINT HIGHSCORE/////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



void printHighscore()
{

  float highScore;
  EEPROM.get(0, highScore);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bisher georgelt:");
  lcd.setCursor(0, 1);
  lcd.print(highScore);
  lcd.setCursor(11, 1);
  lcd.print("Liter");

  pause(2500);
  lcd.clear();

  return;



}


/////////////////////////////////////////////////////////////////////////////////
//////////////////////SET COUNTDOWN//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void setCountdown()
{
  myEnc.write(0);
  bool ctrl = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Countdown setzen");
  lcd.setCursor(0, 1);
  lcd.print("auf ");
  lcd.setCursor(8, 1);
  lcd.print("Sekunden");
  lcd.setCursor(5, 1);
  lcd.print(countdown);

  while (ctrl)
  {

    newPos = myEnc.read();

    checkLeft();
    checkRight();

    lcd.setCursor(5, 1);
    lcd.print(countdown);

    if (right && countdown < 7)
    {
      countdown += 1;

    }
    if (left && countdown > 1)
    {
      countdown -= 1;

    }


    if (digitalRead(ENCSW) == LOW)
    {
      countdownFlag = true;
      oldPos = newPos;
      ctrl = false;
    }
    left = false;
    right = false;
  }

  printTwoStrings("Countdown", "Gespeichert !");
  pause(2000);

  return;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////GET PRESSSURESENNSOR VALUE//////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

float getPressureVal()
{
  float pressureSensVal = 0;
  pressureSensVal = analogRead(PRESSURESENS);
 

  return pressureSensVal;


}

//////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// SHOW SETTINGS/////////////////////////////////////////////////

void showSetting()
{

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Druck: ");
  lcd.setCursor(8, 0);
  lcd.print(pressure, 1);
  lcd.setCursor(12, 0);
  lcd.print("Bar");
  lcd.setCursor(0, 1);
  lcd.print("Timer: ");
  lcd.setCursor(9, 1);
  lcd.print(countdown);
  lcd.setCursor(11, 1);
  lcd.print("Sek.");
  pause(2000);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////BUZZER READY SIGNAL/////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void buzzerReadyToneLight()
{
  int duration = 150;

  for (int i = 2; i >= 0; i--)
  {
    pause(duration);
    analogWrite(GREENPIN, 255);
    digitalWrite(BUZZERPIN, HIGH);
    pause(duration);
    analogWrite(GREENPIN, 0);
    digitalWrite(BUZZERPIN, LOW);
    pause(duration);
    analogWrite(GREENPIN, 255);
    digitalWrite(BUZZERPIN, HIGH);
    pause(duration);
    analogWrite(GREENPIN, 0);
    digitalWrite(BUZZERPIN, LOW);
    pause(duration);
  }

  return;
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////// FILL PRESSURE//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void fillPressure()
{
  bool ctrl = true; 

  if (pressureFlagSet == false)
  {
    error(1);
    return;
  }

  float sensVal = 0;
  float settedPressure = 130.0 + (pressure * 64.0);
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Betankung laeuft");
  lcd.setCursor(0, 1);
  lcd.print("auf ");
  lcd.setCursor(5, 1);
  lcd.print(pressure);
  lcd.setCursor(10, 1);
  lcd.print("Bar");

  // Serial.println(settedPressure);  // TEST von Drucksensor

  while (ctrl)
  {
    
    sensVal = getPressureVal();
  //  Serial.println(sensVal);   // TEST von Drucksensor
    analogWrite(BLUEPIN, 255);
    digitalWrite(COMPRESSORRELY, HIGH);

if(sensVal >= settedPressure)
{
  ctrl = false; 
}

  }


  analogWrite(BLUEPIN, 0);
  digitalWrite(COMPRESSORRELY, LOW);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Betankung");
  lcd.setCursor(0, 1);
  lcd.print("Abgeschlossen");



  buzzerReadyToneLight();




  pressureFlagFilled = true;


  pause(1000);

  return;



}

///////////////////////////////////////////////////////////////////////////////////
////////////////////////////// RIGHT FOR FLOW//////////////////////////////////////

bool rightForFlow()
{
  if (newPos >= (oldPos + 4 ))
  {

    oldPos = newPos;

    return true;

  }
  return false;

}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////CHECK FOR SCREENSAVER///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////GETFLOWVALUE///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void getFlowValue()
{

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bitte Bier");
  lcd.setCursor(0, 1);
  lcd.print("einfuellen");

  analogWrite(REDPIN, 255);


  while (digitalRead(ENCSW) != LOW)
  {
    startTime = micros();
    do {
      if (pulseIn(7, HIGH) > 0)imp ++;
    }
    while ( micros() < (startTime + 1e4));
  }


  float liter = imp / 370.0;

  lcd.clear();
  analogWrite(REDPIN, 0);
  pause(1000);

  analogWrite(GREENPIN, 255);

  lcd.setCursor(0, 0);
  lcd.print("Es wurden ");
  lcd.setCursor(11, 0);
  lcd.print(liter);
  lcd.setCursor(0, 1);
  lcd.print("Liter getankt");

  pause(3000);

  float highScore;
  EEPROM.get(0, highScore);

  liter += highScore;

  EEPROM.put(0, liter);

  analogWrite(GREENPIN, 0);
  lcd.clear();

  return;

}
//////////////////////////////////////////////////////////////////////////////////
////////////////////MENÜ//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*
  0 Highscore
  13 Bestätigen
  1 Orgeln
  11 Bestätigen
  111 Orgeln
  21 Abbrechen
  2 Einstellungen
  21 Countdown
  211 Bestätigen(SetCountSown)
  22 Druck
  221 Bestätigen(SetPressure)
  23 Zurück zum Menü
  3 Druckluft betanken
  4 Getränk einfüllen
  41 Bestätigen
  5 Zufallsorgeln
  51 Bestätigen
  511 Zufallsorgeln
  52 Zurück
*/
void selectDisplayStatus()
{

  
  if (disIndex == 0)
  {
    printTwoStrings("Highscore", "Anzeigen");
    encoderChangeDisplay(4, 1);
    disIndex = encoderSwitch(disIndex, 13);

  }

  else if ( disIndex == 13)
  {
    printHighscore();
    disIndex = 0;
  }

  else if (disIndex == 1)
  {
    printTwoStrings("ORGELN", "ORGELN");
    encoderChangeDisplay(0, 2);
    disIndex = encoderSwitch(disIndex, 11);
  }

  else if (disIndex == 11)
  {

    printTwoStrings("-->Bestaetigen", "   Abbrechen");
    encoderChangeDisplay(12, 12);
    disIndex = encoderSwitch(disIndex, 111);

  }

  else if (disIndex == 12)
  {


    printTwoStrings("   Bestaetigen", "-->Abbrechen");
    encoderChangeDisplay(11, 11);
    disIndex = encoderSwitch(disIndex, 0);
  }

  else if (disIndex == 111)
  {
    orgeln();
    disIndex = 0;
  }

  else if ( disIndex == 2)
  {
    printTwoStrings("Einstellungen", "");
    encoderChangeDisplay(1, 3);
    disIndex = encoderSwitch(disIndex, 21);
  }

  else if (disIndex == 21)
  {
    printTwoStrings("Countdown", "Aendern");
    encoderChangeDisplay(23, 22);
    disIndex = encoderSwitch(disIndex, 211);
  }

  else if ( disIndex == 211)
  {
    setCountdown();
    pause(500);
    disIndex = 0;
  }

  else if (disIndex == 22)
  {
    printTwoStrings("Druck", "Einstellen");
    encoderChangeDisplay(21, 23);
    disIndex = encoderSwitch(disIndex, 221);
  }

  else if (disIndex == 221)
  {
    setPressure();
    pause(500);
    disIndex = 0;
  }

  else if ( disIndex == 23)
  {
    printTwoStrings("Zurueck zum", "Menue");
    encoderChangeDisplay(22, 21);
    disIndex = encoderSwitch(disIndex, 0);
  }

  else if (disIndex == 3)
  {
    printTwoStrings("Druckluft", "Auftanken");
    encoderChangeDisplay(2, 4);
    disIndex = encoderSwitch(disIndex, 31);
  }
  else if (disIndex == 31)
  {
    fillPressure();
    pause(1500);
    disIndex = 0;
  }

  else if (disIndex == 4)
  {
    printTwoStrings("Getraenk", "einfuellen");
    encoderChangeDisplay(3, 5);
    disIndex = encoderSwitch(disIndex, 41);
  }

  else if (disIndex == 41)
  {
    getFlowValue();
    disIndex = 0;
  }

  else if (disIndex == 5)
  {
    printTwoStrings("Zufallsorgeln", " ");
    encoderChangeDisplay(4, 0);
    disIndex = encoderSwitch(disIndex, 51);
    
  }
  else if (disIndex == 51)
  {
    printTwoStrings("Zufallsorgeln" , "->Ein  Zurueck");
    encoderChangeDisplay(52, 51); 
    disIndex= encoderSwitch(disIndex, 511); 
  }


  else if (disIndex == 511)
  {
    /*
     * 
     * 
     * Funktion zum Zufallsorgeln 
     *    
     */
  }
  
  else if (disIndex == 51)
  {
    printTwoStrings("Zufallsorgeln" , "   Ein->Zurueck");
    encoderChangeDisplay(51, 52); 
    disIndex= encoderSwitch(disIndex, 0); 
  }
  
}


void setup()
{

  pinMode(BUZZERPIN, OUTPUT);
  pinMode(VENTPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(FLOATPIN, INPUT_PULLUP);
  pinMode(FIREPIN, OUTPUT);
  pinMode(COMPRESSORRELY, OUTPUT);
  pinMode(PRESSURESENS, INPUT);
  pinMode(FLOWSENS, INPUT);

  lcd.init();
  lcd.backlight();
  Serial.begin(9600);  

  randomSeed(analogRead(0));

  oldPos = myEnc.read(); 

}

void loop() {



  selectDisplayStatus();
  

}
