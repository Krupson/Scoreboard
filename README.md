# Scoreboard
An open-sourced project of digital 3D-printed scoreboard based on ESP32C6 and some WS2812 LEDs.

## Features
- Configurable colors and brightness
- Current state memory (score does not reset on power loss)
- Debounced buttons (prevents accidental add/subtract score 2nd time in very short period)
- Remote control over WiFi (Work in progress)

## Controls
### Control score
Click Tx_ADD and Tx_SUB buttons once to control score for choosen team.

### Switch sides
Double click SWITCH_SIDES button to switch sides of team 1 and team 2.

### Reset score
Double click RESET_SCORE.

## Config mode
In order to enter config mode long press SWITCH_SIDES button. While in config mode there will be "0000" displayed on the board. In order to exit config mode (and save new settings) long press SWITCH_SIDES again.

### Change brightness
While in config mode you can press and hold T1_ADD / T1_SUB buttons to increase or decrease brightness of the display.

### Change team color
While in config mode you can press Tx_ADD / Tx_SUB to change color for choosen team. Red and green should be the least power-hungry :)

## Roadmap
This project was started as something intended for our internal use. Since I am designing 3D models for this project in Fusion360 and I am beginner with this software - quality of the project is much less than acceptable. Honestly I think that the whole 3D model part needs to be recreated from scratch with use of components, project variables for some dimensions etc. 

In near future I am planning to export all STLs I have right now and create Bambu Studio project with all the parts needed to be printed split into buildplates. Then I would like to focus on firmware part, create new PCB design with its own battery and charging module (currently we use old powerbank for providing power to the device).

### Things to be done
#### General
- Create docs with parts needed to build the scoreboard
- Cleanup repo

#### 3D models
- Recreate from scratch

#### Firmware
- Get rid of Arduino project, move everything to PlatformIO project
- Review RTOS code
- Finish remote control server
- Create remote control client firmware
- Add support for temperature and humidity sensors
- Add support for SD card logging (score + temp + hum over time)

#### PCB
- Add temperture and humidity sensor
- Add SD card slot
- Add support for battery