# ACRT
Periodic change of rotation direction with a DC motor

## Description

ACRT (Accelerated crucible rotation) is a technique initially used for the rotation of crucibles during crystal growth [1].

Motor rotation rate is increased and decreased in a periodic way, optionally changing also the direction.

>[1] H. J. Scheel, E. O. Schulz-Dubois, Flux growth of large crystals by accelerated crucible-rotation technique. Journal of Crystal Growth 8 (1971) 304-306.

## Schematics

- Max. motor current = 0.5 A
- Supply voltage = 9...24 V

![pic](photo/ACRT_schematic.jpg).

## Layout

![pic](photo/ACRT_inside.jpg).
![pic](photo/ACRT_layout.jpg).

## Motor box

![pic](photo/ACRT_test.jpg).

## Arduino code (acrt_clean.ino)

- Motor is controlled with a 31 kHz PWM signal
- Motor recipe is defined with 3 voltage levels (duty cycle in %, "-" changes direction) and 4 time intervals (seconds)
- Parameters for motor recipe can be adjusted with 3 buttons on a 2-line display
- Last parameters are stored in EEPROM memory
- Auto backlight-off for the display

## Acknowledgements

[This project](https://nemocrys.github.io/) has received funding from the European Research Council (ERC) under the European Union's Horizon 2020 research and innovation programme (grant agreement No 851768).

<img src="https://github.com/nemocrys/test-cz-induction/blob/main/EU-ERC.png">
