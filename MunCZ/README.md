# MunCZ
DC motor with a recipe and stepper motor with a fixed speed

## Description

This is an extended version of the Arduino code in [ACRT](https://github.com/nemocrys/exp-motor-control/blob/main/ACRT)

The DC Motor is controlled by changing the duty cycle of a 31 kHz PWM signal.

The Stepper motor is controlled by generating pulses at a given frequency.

## Wiring

- Arduino pin D9 = Stepper direction
- Arduino pin D8 = Stepper speed
- Arduino pin D5 = DC direction
- Arduino pin D11 = DC speed

- Arduino pin D2 = Button SWITCH
- Arduino pin D3 = Button UP
- Arduino pin D4 = Button DOWN
The other side of the buttons are connected to Arduino GND.

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
- SWITCH: consecutively switch between parameters (LCD screens). **To apply the changed value, click to the last screen, until "Parameters changed" is displayed**. The second last screen shows current value of time and speed. The last screen is empty. Press the button again to switch to the first screen.
- UP/DOWN: increase/decrease parameter value in predefined steps. A short press of 0.3 sec changes the value by one step. Holding the button over 3 s and 6 s increases the step by x10 and x100.

Parameter currently implemented:
- 01 V_H  (+-100%)   = Rotation velocity, high value
- 02 V_M  (+-100%)   = Rotation velocity, mid value
- 03 V_L  (+-100%)   = Rotation velocity, low value
- 04 T_H  (0-10000s) = Time interval, high value
- 05 T_L  (0-10000s) = Time interval, low value
- 06 T_TH (0-1000s)  = Transition time, high value
- 07 T_TL (0-1000s)  = Transition time, low value
- 08 V_P  (+-1000Hz) = Pull velocity (step motor)

The last parameter set is automatically stored in EEPROM memory. It is loaded at Arduino startup.

## Acknowledgements

[This project](https://nemocrys.github.io/) has received funding from the European Research Council (ERC) under the European Union's Horizon 2020 research and innovation programme (grant agreement No 851768).

<img src="https://github.com/nemocrys/test-cz-induction/blob/main/EU-ERC.png">
