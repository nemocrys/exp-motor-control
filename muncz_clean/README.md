# MunCZ
DC motor with a recipe and stepper motor with a fixed speed

## Description

This is an extended version of the Arduino code in [ACRT](https://github.com/nemocrys/exp-motor-control/blob/main/ACRT)

The DC Motor is controlled by changing the duty cycle of a 31 kHz PWM signal.

The Stepper motor is controlled by generating pulses at a given frequency.

Additional 31 kHz PWM signal can be used to control a fan.

## Wiring

Motors:

- Arduino pin D22 = Stepper direction
- Arduino pin D8 = Stepper speed
- Arduino pin D23 = DC motor direction
- Arduino pin D11 = DC motor speed
- Arduino pin D52 = Fan speed

Buttons:

- Arduino pin D2 = Button SWITCH
- Arduino pin D3 = Button UP
- Arduino pin D4 = Button DOWN

The other side of the buttons are connected to Arduino GND.

LCD: 

- Arduino pin D20 = LCD SDA
- Arduino pin D21 = LCD SCL
- Arduino pin D7 = LCD Backlight via a 100 Ohm resistor. Needed only for LCDs with dark letters on color light background (positive mode).

The LCD also requires GND and 5V connections for power supply.

## Installation

The following libraries should be added in Arduino IDE:

- NewliquidCrystal
- TimerFour-master

The ZIP files provided [here](https://github.com/nemocrys/exp-motor-control/blob/main/libs) can be also manually unpacked to the *libraries* folder of Arduino IDE.

## Menu

The LCD allows to adjust motor parameters using 3 buttons:

- SWITCH: consecutively switch between parameters (LCD screens). **To apply the changed value, click to the last screen, until "Parameters changed" is displayed**. The second last screen shows current values of time and speed. The last screen is empty. Press the button again to switch to the first screen.
- UP/DOWN: increase/decrease parameter value in predefined steps. A short press of 0.3 sec changes the value by one step. Holding the button for 3 s and 6 s increases the step by x10 and x100.

Parameters currently implemented:

- 01 V_H  (+-100%)   = Rotation velocity, high value
- 02 V_M  (+-100%)   = Rotation velocity, mid value
- 03 V_L  (+-100%)   = Rotation velocity, low value
- 04 T_H  (0-1000s) = Time interval for high velocity
- 05 T_L  (0-1000s) = Time interval for low velocity
- 06 T_TH (0-1000s)  = Transition time for high velocity
- 07 T_TL (0-1000s)  = Transition time for low velocity
- 08 V_P  (+-1000Hz) = Pull velocity (stepper motor)
- 09 V_F  (0-100%) = Fan speed

### Pull velocity setting for the MunCZ hardware setup

- Pull velocity positive = clockwise rotation = translation up (pulling); DirPin LOW
- Pull velocity negative = counter-clockwise rotation = translation down; DirPin HIGH

### Using sketch file to set the EEPROM parameters 

By default, the command saveparams() is active, which allows to overwrite the parameter set stored in the Arduino EEPROM memory with the values specified in the Arduino sketch file.
Conversely, if the command loadparams() is active, the last parameter set which is stored in EEPROM memory is loaded at Arduino startup and the values from the sketch are ignored.


## Acknowledgements

[This project](https://nemocrys.github.io/) has received funding from the European Research Council (ERC) under the European Union's Horizon 2020 research and innovation programme (grant agreement No 851768).

<img src="https://github.com/nemocrys/test-cz-induction/blob/main/EU-ERC.png">
