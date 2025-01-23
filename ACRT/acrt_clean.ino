
/*

TODO


*/


// -------------------------------------------------------------------------------- Pins

const int pinRotDir = 5;
const int pinRotSpeed = 11;

const int pinBut1 = 2;
const int pinBut2 = 3;
const int pinBut3 = 4;

const int pinLight = 7;

//SCL is pin A5 and SDA is pin A4 on the Nano

const bool useserial = false; //enable serial output
bool serialonce; // write one recipe

// -------------------------------------------------------------------------------- LCD

#include <Wire.h> 
#include <LiquidCrystal_I2C.h> //adjust contrast on the LCD!!!
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // Set the LCD I2C address

unsigned long lcdLastUpdate;
unsigned long lcdLastLight;

// -------------------------------------------------------------------------------- Motor

#define DDRotDirCW  HIGH
#define DDRotDirCCW LOW

int speedHigh;
int speedMid;
int speedLow;
int timeHigh; //sec
int timeLow; //sec
int timeTrHigh; //sec/10!!!
int timeTrLow; //sec/10!!!


unsigned long tt1, tt2, tt3, tt4, tt5, tt6, tttrhigh, tttrlow;
unsigned long tt;
unsigned long motLastUpdate;
int motRotDuty, motRotDutyOld;      //PWM duty cycle 0-255
int motRotDir;
float motRotSpeed; //value including direction

// ---

void resetmotor(bool showlcd) 
{
if (useserial)
  {
  Serial.print("speedHigh = ");  Serial.println(String(speedHigh));
  Serial.print("speedMid = ");  Serial.println(String(speedMid));
  Serial.print("speedLow = ");  Serial.println(String(speedLow));
  Serial.print("timeHigh = ");  Serial.println(String(timeHigh));
  Serial.print("timeLow = ");  Serial.println(String(timeLow));
  Serial.print("timeTrHigh = ");  Serial.println(String(timeTrHigh));
  Serial.print("timeTrLow = ");  Serial.println(String(timeTrLow));
  serialonce=true;
  }

if (showlcd)
  {
  lcd.clear();
  lcd.setCursor(0,0);  lcd.print("Parameters      "); 
  lcd.setCursor(0,1);  lcd.print("changed!        ");
  }

// slow down over 2s
int dr = (int)ceil(((float)motRotDuty)/20.0);
while (motRotDuty>0)
  {
  motRotDuty -= dr;
  if (motRotDuty>=0) analogWrite(pinRotSpeed,motRotDuty);
  delay(100);
  }

// convert to millisec
tt1 = (long)timeTrHigh*100;
tt2 = tt1 + (long)timeHigh*1000;
tt3 = tt2 + (long)timeTrHigh*100;
tt4 = tt3 + (long)timeTrLow*100;
tt5 = tt4 + (long)timeLow*1000;
tt6 = tt5 + (long)timeTrLow*100;
tttrhigh = (long)timeTrHigh*100;
tttrlow = (long)timeTrLow*100;

motRotDuty = 0;
motRotDutyOld = 0;
motRotDir = 0;
motRotSpeed = 0;
motLastUpdate = 0;
tt = 0;
}

// -------------------------------------------------------------------------------- Menu

const int numOfInputs = 3;
const int inputPins[numOfInputs] = {pinBut1,pinBut2,pinBut3};
int inputState[numOfInputs] = {0,0,0};
unsigned long inputTime[numOfInputs] = {0,0,0};
const int numOfScreens = 7;
int currentScreen;


String screens[numOfScreens][2] = 
{
{"01 V_H", "(-100-100%)"},
{"02 V_M", "(-100-100%)"},
{"03 V_L", "(-100-100%)"},
{"04 T_H", "(0-10000s)"},
{"05 T_L", "(0-10000s)"},
{"06 T_TH", "(0-1000s)"},
{"07 T_TL", "(0-1000s)"}
};


//value, increment, min, max, oldvalue
float parameters[numOfScreens][5]
{
{0,1,-100,100,0},
{0,1,-100,100,0},
{0,1,-100,100,0},
{0,1,0,10000,0},
{0,1,0,10000,0},
{0,0.1,0,1000,0},
{0,0.1,0,1000,0}
};


void getparams() 
{
parameters[0][0] = speedHigh;
parameters[1][0] = speedMid;
parameters[2][0] = speedLow;
parameters[3][0] = timeHigh;
parameters[4][0] = timeLow;
parameters[5][0] = 0.1*timeTrHigh;
parameters[6][0] = 0.1*timeTrLow;
for (int i = 0; i < numOfScreens; i++) { parameters[i][4] = parameters[i][0];  } // store old values
}


//set only parameters which where manually changed!
bool setparams() 
{
bool changed = false;
if (parameters[0][0] != parameters[0][4])  { speedHigh = parameters[0][0];  changed=true; }
if (parameters[1][0] != parameters[1][4])  { speedMid = parameters[1][0];  changed=true; }
if (parameters[2][0] != parameters[2][4])  { speedLow = parameters[2][0];  changed=true; }
if (parameters[3][0] != parameters[3][4])  { timeHigh = parameters[3][0];  changed=true; }
if (parameters[4][0] != parameters[4][4])  { timeLow = parameters[4][0];  changed=true; }
if (parameters[5][0] != parameters[5][4])  { timeTrHigh = 10*parameters[5][0];  changed=true; }
if (parameters[6][0] != parameters[6][4])  { timeTrLow = 10*parameters[6][0];  changed=true; }
return changed;
}


void printScreen() {
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  char showstr[8];
  int showdec = 0;
  if (parameters[currentScreen][1] < 1) showdec = 1;
  if (parameters[currentScreen][1] < 0.1) showdec = 2;
  dtostrf(parameters[currentScreen][0],1,showdec,showstr); // show 0 or 1 or 2 decimals
  lcd.print(showstr);
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
}

// -------------------------------------------------------------------------------- Memory

#include <EEPROM.h>

const int speedHighAddr = 0;
const int speedMidAddr = 2;
const int speedLowAddr = 4;
const int timeHighAddr = 6;
const int timeLowAddr = 8;
const int timeTrHighAddr = 10;
const int timeTrLowAddr = 12;


void EEPROM_write(int address, int value)
{
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++) { EEPROM.update(address++, *p++); } //use update to avoid unneeded writes
}


int EEPROM_read(int address)
{
   int value = 0;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++) { *p++ = EEPROM.read(address++); }
   return value;
}


void loadparams()
{
int val;
val = EEPROM_read(speedHighAddr);  if (val>=-100 && val<=100) speedHigh = val;
val = EEPROM_read(speedMidAddr);   if (val>=-100 && val<=100) speedMid = val;
val = EEPROM_read(speedLowAddr);   if (val>=-100 && val<=100) speedLow = val;
val = EEPROM_read(timeHighAddr);   if (val>=0 && val<=10000) timeHigh = val;
val = EEPROM_read(timeLowAddr);    if (val>=0 && val<=10000) timeLow = val;
val = EEPROM_read(timeTrHighAddr); if (val>=0 && val<=1000) timeTrHigh = val;  
val = EEPROM_read(timeTrLowAddr); if (val>=0 && val<=1000) timeTrLow = val;  
}


void saveparams()
{
EEPROM_write(speedHighAddr, speedHigh);
EEPROM_write(speedMidAddr, speedMid);
EEPROM_write(speedLowAddr, speedLow);
EEPROM_write(timeHighAddr, timeHigh);
EEPROM_write(timeLowAddr, timeLow);
EEPROM_write(timeTrHighAddr, timeTrHigh);
EEPROM_write(timeTrLowAddr, timeTrLow);
}

// --------------------------------------------------------------------------------

void setup() {

  // Serial

  if (useserial) 
    {
    Serial.begin(19200, SERIAL_8N1);
    serialonce = true;
    }
  
  // Motor

  TCCR2B = TCCR2B & B11111000 | B00000001; //Set PWM frequency for D3 & D11 from 980 Hz to 31372.55 Hz
  
  pinMode(pinRotDir, OUTPUT);    
  pinMode(pinRotSpeed, OUTPUT); // start in low-impedance state 
  pinMode(pinLight, OUTPUT);
  digitalWrite(pinLight, HIGH);

  speedHigh = 100;
  speedMid = 0;
  speedLow = -100;  
  timeHigh = 20; // 20 sec
  timeLow = 20; // 20 sec
  timeTrHigh = 5; // 0.5 sec
  timeTrLow = 5; // 0.5 sec
  loadparams(); //load from memory
  resetmotor(false);

  // LCD

  lcd.begin(16,2);
  lcd.setCursor(0,0);  lcd.print("Welcome...      "); 
  lcd.setCursor(0,1);  lcd.print("...to ACRT      ");

  digitalWrite(pinLight, HIGH);
  lcdLastUpdate = 0;
  lcdLastLight = 0;

  // Menu

  pinMode(pinBut1, INPUT_PULLUP);
  pinMode(pinBut2, INPUT_PULLUP);
  pinMode(pinBut3, INPUT_PULLUP);  

  currentScreen = numOfScreens;

}

// --------------------------------------------------------------------------------

void loop() {

unsigned long now;
unsigned long nowdelta;

// ---------- Button menu

for (int i = 0; i < numOfInputs; i++) 
  {
  int reading = !digitalRead(inputPins[i]); //HIGH=0, LOW=1 !!!
  now = millis();

  //button pressed -> start counting time
  if (reading == 1 && inputState[i] == 0) 
    { 
    inputTime[i] = now;
    inputState[i] = 1;  
    //digitalWrite(LED_BUILTIN, HIGH);
    lcdLastLight = now;
    } 

  //button pressed for 300 ms, repeat if longer
  if (reading == 1 && inputState[i] >= 1 && now - inputTime[i] > 300 ) 
    {
    inputTime[i] = now;
    inputState[i] += 1;
    lcdLastLight = now;    

    if (i==0 && inputState[i]<=2) //switch screens, exclude from repeating!
      {
      if (currentScreen == numOfScreens ) lcd.clear();
      if (currentScreen == numOfScreens+1 ) { currentScreen = 0; } else currentScreen++;
        
      if (currentScreen == 0) { getparams();  printScreen(); }
      if (currentScreen > 0 && currentScreen < numOfScreens) { printScreen(); }
      if (currentScreen == numOfScreens )  
        { 
        if (setparams()==true) { resetmotor(true);  saveparams();  }
        }
      if (useserial) Serial.println("Button: Switch/ Menu: "+String(currentScreen)); 
      }

    int mult = 1;
    if (inputState[i]>=10) mult = 10; // speed up if pressed 3 s
    if (inputState[i]>=20) mult = 100; // speed up if pressed 6 s
          
    if (i==1 && currentScreen < numOfScreens) //up
      {
      if ( parameters[currentScreen][0] + parameters[currentScreen][1]*mult > parameters[currentScreen][3] ) 
        { parameters[currentScreen][0] = parameters[currentScreen][3]; }
      else { parameters[currentScreen][0] = parameters[currentScreen][0] + parameters[currentScreen][1]*mult; } 
      printScreen();
      if (useserial)  Serial.println("Button: Up/ Val: "+String(parameters[currentScreen][0]));                    
      }
      
    if (i==2 && currentScreen < numOfScreens) //down
      {
      if ( parameters[currentScreen][0] - parameters[currentScreen][1]*mult < parameters[currentScreen][2] ) 
        { parameters[currentScreen][0] = parameters[currentScreen][2]; }
      else { parameters[currentScreen][0] = parameters[currentScreen][0] - parameters[currentScreen][1]*mult; }      
      printScreen();
      if (useserial)  Serial.println("Button: Down/ Val: "+String(parameters[currentScreen][0]));                    
      }
    
    } //if

  //button released
  if (reading == 0 && inputState[i] >= 1)
    {
    inputState[i] = 0;
    //digitalWrite(LED_BUILTIN, LOW);
    }
    
  } //for

// ---------- Set motor speed for an ACRT recipe

now = millis();
nowdelta = now - motLastUpdate;
if (nowdelta < 0) { nowdelta=0;  motLastUpdate=0;  } //if time overflows

if (nowdelta >= 10) //each 10 ms
  {
  tt = tt + nowdelta; // avoid using absolute time, which may overflow
  if ( tt>=tt6 ) 
    {
    if (useserial && serialonce && motLastUpdate>0) //avoid directly after resetmotor()
      {
      serialonce = false;
      Serial.println("Finished log at tt = "+String(tt));
      }    
    tt=0;
    }

  // if some intervals are zero, corresponding steps are skipped
  if ( tt>=0   && tt<tt1 ) motRotSpeed = speedMid + (speedHigh-speedMid) * ((float)tt)/((float)tttrhigh);
  if ( tt>=tt1 && tt<tt2 ) motRotSpeed = speedHigh;
  if ( tt>=tt2 && tt<tt3 ) motRotSpeed = speedHigh + (speedMid-speedHigh) * ((float)(tt-tt2))/((float)tttrhigh);
  if ( tt>=tt3 && tt<tt4 ) motRotSpeed = speedMid + (speedLow-speedMid) * ((float)(tt-tt3))/((float)tttrlow);
  if ( tt>=tt4 && tt<tt5 ) motRotSpeed = speedLow;
  if ( tt>=tt5 && tt<tt6 ) motRotSpeed = speedLow + (speedMid-speedLow) * ((float)(tt-tt5))/((float)tttrlow);

  if (abs(motRotSpeed)>0) motRotDuty = (int)ceil(255 * 0.01 * abs(motRotSpeed)); else motRotDuty=0;  
  if (motRotDuty>255) motRotDuty=255;
  if (motRotSpeed>0) motRotDir=1; else motRotDir=0;

  if (motRotDir==1) digitalWrite(pinRotDir, DDRotDirCW); else digitalWrite(pinRotDir, DDRotDirCCW);
  if (motRotDuty!=motRotDutyOld && motRotDuty<=255 && motRotDuty>=0) 
    { 
    analogWrite(pinRotSpeed, motRotDuty);  
    motRotDutyOld=motRotDuty;
    }

  if (useserial && serialonce)
    {
    //50 chars & 19200 bps = 3 ms
    Serial.println(String(motLastUpdate)+" "+String(tt)+" "+String(motRotSpeed)+" "+String(motRotDuty)+" "+String(motRotDir));
    }

  motLastUpdate = now;
    
  } // if nowdelta

// ---------- LCD update

now = millis();
if (now - lcdLastLight < 5000) //backlight off after 5 sec
  {
  digitalWrite(pinLight, HIGH);
  }
else digitalWrite(pinLight, LOW);


nowdelta = now - lcdLastUpdate;
if (nowdelta>1000 || tt==0) //align with recipe interval
  {
  lcdLastUpdate = now;

  if (currentScreen==numOfScreens)
    { 
    char stra[6];  dtostrf(motRotSpeed,1,1,stra);
    lcd.setCursor(0,0);  lcd.print("Speed=          ");
    lcd.setCursor(6,0);  lcd.print(stra);
  
    char strb[6];  dtostrf(0.001*(double)tt,1,1,strb);
    lcd.setCursor(0,1);  lcd.print("Time=           ");
    lcd.setCursor(5,1);  lcd.print(strb);
    }    
  }

} //loop
