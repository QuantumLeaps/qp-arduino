![QP framework](https://state-machine.com/img/qpn-arduino_banner.jpg)

This README provides a quick overview of QP-nano for Arduino.
Please refer to the companion webiste to this project at:

https://state-machine.com/arduino


---------------------------------------------------------------------------
## Software Installation on Windows
The QP-nano adaptation for Arduino is distributed in a single ZIP
archive qpn-<ver>_arduino-1.8.x.zip, where <ver> stands for version of
the QP-nano framework.

You need to unzip the qpn-<ver>_arduino-1.8.x.zip archive into your
Arduino-Sketchbook folder. To find out where your Sketchbook folder is,
or to configure a different location for your Sketchbook folder, you
need to open the Arduino IDE and select File | Preferences menu. The
sketchbook location will be shown at the top of the Preferences dialog
box. Once you identify the Sketchbook folder, you simply unzip the whole
archive to your Sketchbook.


## Software Installation on MacOS
The Mac funtionality has been tested, but other users will likely find
issues. Please submit issues at https://sourceforge.net/p/qpc/bugs/ and
submit questions and suggestions at
https://sourceforge.net/p/qpc/discussion/668726/


### Install Dependencies
1. Install the AVR-GCC tooltrain for Mac OS X. Following are the commands
to install using Homebrew (must have Homebrew installed):

  > ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  > brew tap osx-cross/avr
  > brew install avr-gcc
  > brew install avrdude --with-usb

2. Install Arduino for Mac OS X by placing the Arduino application in the
Applications folder. Open the application at least one time to overcome
security warnings.

3. Install Install QP-nano for Arduino into your Documents/Arduino folder
(documentation refers to this as the Arduino Sketchbook folder).

4. Install QM.


### Instructions for Configuring External Tools for a Model
1. Ensure the model is not open in QM.

2. Copy the Blinky model's settings
(libraries/qpn_avr/examples/blinky/.blinky.macos) from your Arduino
Sketchbook folder to the model's folder.

3. Rename the settings file to your model's name (i.e., .mymodel).

4. Open QM and edit the settings from External Tools.

Once you have a working .model file, copy it to other models and use it as
a starting point.


## Known Issues
1. The serialterm External Tool is working by opening a Terminal window.
An additional Terminal window also appears and can be safely closed. When
the Terminal window displaying the Arduino's output is closed, the port is
busy and can no longer be accessed. Work-around is to unplug and
re-connect the Arduino from the Mac's USB port.


---------------------------------------------------------------------------
# Building the Examples in the QM Modeling Tool

> **NOTE:** To start working with the freeware QM modeling tool, you need to
download the tool from https://sourceforge.net/projects/qpc/files/QM.

Each QP-nano example for Arduino (in the examples_avr folder) contains
a QM model, which is a file with the extension .qm., such as
<Sketchbook>\examples_avr\blinky\blinky.qm. These models and the QM
modeling tool take Arduino programming to the next level. Instead of
coding the state machines by hand, you draw them with the free QM
modeling tool, attach simple action code to states and transitions, and
you generate the complete Arduino sketch automatically-—literally by a
press of a button.


---------------------------------------------------------------------------
# Licensing
The QP-nano framework for Arduino is licensed under the modified GPLv3
license (see file GPLv3.txt) with the QP-Arduino Exception (see file
QP-Arduino_GPL_Exception.txt). Specifically, the Exception removes the
requirement to expose your propriatory source code as long as you deploy
it on an Arduino-Certified board.

The QM graphical modeling tool is freeware. The QM tool is provided
under the terms of a simple End-User License Agreement (EULA).


---------------------------------------------------------------------------
# Documentation
Please refer to the companion webiste to this project at:

https://state-machine.com/arduino

Specifically, the Application Note: "Event-Driven Arduino Programming
with QP and QM" is available from:

https://state-machine.com/doc/AN_Event-Driven_Arduino_QP-nano.pdf


---------------------------------------------------------------------------
# How to get help?
- [Free Support Forum](https://sourceforge.net/p/qpc/discussion/668726)
- [Bug Reports](https://sourceforge.net/p/qpc/bugs/)
- [Feature Requests](https://sourceforge.net/p/qpc/feature-requests/)
- [Quantum Leaps website](https://state-machine.com)

