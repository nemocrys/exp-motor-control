
/*

TODO


*/


// -------------------------------------------------------------------------------- Pins

const int pinLinDir = 9; 
const int pinLinSpeed2 = 8;

const int pinRotDir = 5;
const int pinRotSpeed = 11;

const int pinFanSpeed = 12;

const int pinBut1 = 2; //switch
const int pinBut2 = 3; //up
const int pinBut3 = 4; //down

const int pinLight = 7;

// Mega: SDA = D20, SCL = D21

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

#include <TimerFour.h>

#define DDRotDirCW  HIGH
#define DDRotDirCCW LOW

// internal INT (not float!!) variables for rotation
int speedHigh; // [%/10]
int speedMid; //  [%/10]
int speedLow; //  [%/10]
int timeHigh; //  [sec/10]
int timeLow; //   [sec/10]
int timeTrHigh; //[sec/10]
int timeTrLow; // [sec/10]

unsigned long tt1, tt2, tt3, tt4, tt5, tt6, tttrhigh, tttrlow;
unsigned long tt;
unsigned long motLastUpdate;
int motRotDuty, motRotDutyOld;      //PWM duty cycle 0-255
int motRotDir;
float motRotSpeed; //value including direction

// ---

// positive speed = up
#define DDLinDirUp HIGH
#define DDLinDirDown  LOW

// internal INT (not float!!) variable for pulling
int speedPull; // currently: step frequency in [Hz/10]

int motLinDir;
long motLinPeriod = 0, motLinPeriodOld = 0; //in milliseconds
float motLinSpeed; //value including direction

// internal INT (not float!!) variable for pulling
int speedFan; // [%]

int motFanDuty, motFanDutyOld;      //PWM duty cycle 0-255
float motFanSpeed;


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
  Serial.print("speedPull = ");  Serial.println(String(speedPull));
  Serial.print("speedFan = ");  Serial.println(String(speedFan));
  serialonce=false;
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
tt2 = tt1 + (long)timeHigh*100;
tt3 = tt2 + (long)timeTrHigh*100;
tt4 = tt3 + (long)timeTrLow*100;
tt5 = tt4 + (long)timeLow*100;
tt6 = tt5 + (long)timeTrLow*100;
tttrhigh = (long)timeTrHigh*100;
tttrlow = (long)timeTrLow*100;

motRotDuty = 0;
motRotDutyOld = 0;
motRotDir = 0;
motRotSpeed = 0;
motLastUpdate = 0;
tt = 0;

motLinDir = 0;
motLinPeriod = 0; 
motLinPeriodOld = 0;
motLinSpeed = 0;

motFanDuty = 0;
motFanDutyOld = 0;
motFanSpeed = 0;
}

// -------------------------------------------------------------------------------- Menu

const int numOfInputs = 3;
const int inputPins[numOfInputs] = {pinBut1,pinBut2,pinBut3};
int inputState[numOfInputs] = {0,0,0};
unsigned long inputTime[numOfInputs] = {0,0,0};
const int numOfScreens = 9;
int currentScreen;


String screens[numOfScreens][2] = 
{
{"01 V_H", "(+-100%)"},
{"02 V_M", "(+-100%)"},
{"03 V_L", "(+-100%)"},
{"04 T_H", "(0-1000s)"},
{"05 T_L", "(0-1000s)"},
{"06 T_TH", "(0-1000s)"},
{"07 T_TL", "(0-1000s)"},
{"08 V_P", "(+-1000Hz)"},
{"09 V_F", "(0-100%)"}
};


//value, increment, min, max, oldvalue
float parameters[numOfScreens][5]
{
{0,0.1,-100,100,0},
{0,0.1,-100,100,0},
{0,0.1,-100,100,0},
{0,0.1,0,1000,0},
{0,0.1,0,1000,0},
{0,0.1,0,1000,0},
{0,0.1,0,1000,0},
{0,0.1,-1000,1000,0},
{0,1,0,100,0}
};


void getparams() 
{
parameters[0][0] = 0.1*speedHigh;
parameters[1][0] = 0.1*speedMid;
parameters[2][0] = 0.1*speedLow;
parameters[3][0] = 0.1*timeHigh;
parameters[4][0] = 0.1*timeLow;
parameters[5][0] = 0.1*timeTrHigh;
parameters[6][0] = 0.1*timeTrLow;
parameters[7][0] = 0.1*speedPull;
parameters[8][0] = speedFan;
for (int i = 0; i < numOfScreens; i++) { parameters[i][4] = parameters[i][0];  } // store old values
}


//set only parameters which where manually changed!
bool setparams() 
{
bool changed = false;
if (parameters[0][0] != parameters[0][4])  { speedHigh = 10*parameters[0][0];  changed=true; }
if (parameters[1][0] != parameters[1][4])  { speedMid = 10*parameters[1][0];  changed=true; }
if (parameters[2][0] != parameters[2][4])  { speedLow = 10*parameters[2][0];  changed=true; }
if (parameters[3][0] != parameters[3][4])  { timeHigh = 10*parameters[3][0];  changed=true; }
if (parameters[4][0] != parameters[4][4])  { timeLow = 10*parameters[4][0];  changed=true; }
if (parameters[5][0] != parameters[5][4])  { timeTrHigh = 10*parameters[5][0];  changed=true; }
if (parameters[6][0] != parameters[6][4])  { timeTrLow = 10*parameters[6][0];  changed=true; }
if (parameters[7][0] != parameters[7][4])  { speedPull = 10*parameters[7][0];  changed=true; }
if (parameters[8][0] != parameters[8][4])  { speedFan = parameters[8][0];  changed=true; }
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
const int speedPullAddr = 14;
const int speedFanAddr = 16;


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
int val; // Limits are in units for internal INT variables: %/10, sec/10
val = EEPROM_read(speedHighAddr);  if (val>=-1000 && val<=1000) speedHigh = val;
val = EEPROM_read(speedMidAddr);   if (val>=-1000 && val<=1000) speedMid = val;
val = EEPROM_read(speedLowAddr);   if (val>=-1000 && val<=1000) speedLow = val;
val = EEPROM_read(timeHighAddr);   if (val>=0 && val<=10000) timeHigh = val;
val = EEPROM_read(timeLowAddr);    if (val>=0 && val<=10000) timeLow = val;
val = EEPROM_read(timeTrHighAddr); if (val>=0 && val<=10000) timeTrHigh = val;  
val = EEPROM_read(timeTrLowAddr); if (val>=0 && val<=10000) timeTrLow = val;  
val = EEPROM_read(speedPullAddr); if (val>=-10000 && val<=10000) speedPull = val;  
val = EEPROM_read(speedFanAddr); if (val>=0 && val<=100) speedFan = val; 
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
EEPROM_write(speedPullAddr, speedPull);
EEPROM_write(speedFanAddr, speedFan);
}

// --------------------------------------------------------------------------------

void setup() {

  // Serial

  if (useserial) 
    {
    Serial.begin(19200, SERIAL_8N1);
    serialonce = false;
    }
  
  // Motor

  //TCCR2B = TCCR2B & B11111000 | B00000001; //Set PWM frequency for D3 & D11 from 980 Hz to 31372.55 Hz
  // Set PWM freq. on Arduino Mega pins 11 and 12 (Timer 1) to 31000 Hz
  int myEraser = 7;  TCCR1B &= ~myEraser; 
  int myPrescaler = 1;  TCCR1B |= myPrescaler; // 1->31 kHz, 2->4 kHz, 3->490 Hz (default), 4->120 Hz, 5->30 Hz, 6-> <20 Hz


  Timer4.initialize(1000);
  
  pinMode(pinRotDir, OUTPUT);    
  pinMode(pinRotSpeed, OUTPUT); // start in low-impedance state 
  pinMode(pinLight, OUTPUT);
  digitalWrite(pinLight, HIGH);

  pinMode(pinLinDir, OUTPUT);
  pinMode(pinLinSpeed2, OUTPUT); 

  pinMode(pinFanSpeed, OUTPUT); // start in low-impedance state 

  speedHigh = 500;   // 50%
  speedMid = 0;
  speedLow = -500;   // 50%
  timeHigh = 100;    // 10 sec
  timeLow = 100;     // 10 sec
  timeTrHigh = 20;   // 2 sec
  timeTrLow = 20;    // 2 sec
  speedPull = 100;   // 10 Hz
  speedFan = 0;
  //saveparams(); //uncomment later!!!
  loadparams(); //load from memory
  resetmotor(false); //false = do not show on LCD

  // LCD

  lcd.begin(16,2);
  lcd.setCursor(0,0);  lcd.print("Welcome...      "); 
  lcd.setCursor(0,1);  lcd.print("...to ACRT/PULL ");

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

  if (reading == 1)
    {
    if (useserial) Serial.println("Button pressed: "+String(i)+ "Input states: "+String(inputState[0])+" "+String(inputState[1])+" "+String(inputState[2])); 
    }

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

// ---------- Set motor speed

now = millis();
nowdelta = now - motLastUpdate;
if (nowdelta < 0) { nowdelta=0;  motLastUpdate=0;  } //if time overflows

if (nowdelta >= 10) //each 10 ms
  {

  // Rotation
  
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

  motRotSpeed = 0.1 * motRotSpeed; // convert from [%/10] to [%] 
  if (abs(motRotSpeed)>0) motRotDuty = (int)ceil(255 * 0.01 * abs(motRotSpeed)); else motRotDuty=0;  
  if (motRotDuty>255) motRotDuty=255;
  if (motRotSpeed>0) motRotDir=1; else motRotDir=0;

  if (motRotDir==1) digitalWrite(pinRotDir, DDRotDirCW); else digitalWrite(pinRotDir, DDRotDirCCW);
  if (motRotDuty!=motRotDutyOld && motRotDuty<=255 && motRotDuty>=0) 
    { 
    analogWrite(pinRotSpeed, motRotDuty);  
    motRotDutyOld=motRotDuty;
    }


  // Pulling

  motLinSpeed = speedPull; // incl. direction!
  motLinSpeed = 0.1 * motLinSpeed; // convert from [Hz/10] to [Hz] 
    
  if (abs(motLinSpeed)>0) motLinPeriod = floor( 1000000 / abs(motLinSpeed) ); else motLinPeriod=0; // microseconds for timer!
  if (motLinSpeed>0) motLinDir=1; else motLinDir=0; 

  if (motLinDir==1) digitalWrite(pinLinDir, DDLinDirUp); else digitalWrite(pinLinDir, DDLinDirDown);

  if (motLinPeriod!=motLinPeriodOld && motLinPeriod<10000000 && motLinPeriod>=0) 
    {
    //if (motLinPeriodOld==0) Timer4.pwm(pinLinSpeed2, 512); 
    Timer4.disablePwm(pinLinSpeed2); //order of pwm/period commands is important here!!!
    if (motLinPeriod>0) 
      {
      Timer4.setPeriod(motLinPeriod);
      Timer4.pwm(pinLinSpeed2, 512); 
      }
    motLinPeriodOld=motLinPeriod; 
    } 

  // Fan

  motFanSpeed = speedFan; 

  if (abs(motFanSpeed)>0) motFanDuty = (int)ceil(255 * 0.01 * abs(motFanSpeed)); else motFanDuty=0;  
  if (motFanDuty>255) motFanDuty=255;

  if (motFanDuty!=motFanDutyOld && motFanDuty<=255 && motFanDuty>=0) 
    { 
    analogWrite(pinFanSpeed, motFanDuty);  
    motFanDutyOld=motFanDuty;
    }

    
  if (useserial && serialonce)
    {
    //50 chars & 19200 bps = 3 ms
    Serial.print(String(motLastUpdate)+" "+String(tt)+" "+String(motRotSpeed)+" "+String(motRotDuty)+" "+String(motRotDir));
    Serial.print(" "+String(motLinSpeed)+" "+String(motLinPeriod)+" "+String(motLinDir));
    Serial.println(" "+String(motFanSpeed)+" "+String(motFanDuty));
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
    char stra[5];  dtostrf(motRotSpeed,1,1,stra);
    char strc[5];  dtostrf(motLinSpeed,1,1,strc);
    lcd.setCursor(0,0);  lcd.print("R=      P=      ");
    lcd.setCursor(2,0);  lcd.print(stra);
    lcd.setCursor(10,0);  lcd.print(strc);
  
    char strb[6];  dtostrf(0.001*(double)tt,1,1,strb);
    lcd.setCursor(0,1);  lcd.print("Time=           ");
    lcd.setCursor(5,1);  lcd.print(strb);
    }    
  }

} //loop
