README for MacOS

Overview

This describes the initially implemented support on Mac OS X. 

The Mac funtionality has been tested, but other users will likely find
issues. Please submit issues at https://sourceforge.net/p/qpc/bugs/ and
submit questions and suggestions at
https://sourceforge.net/p/qpc/discussion/668726/

Install Dependencies

1. Install the AVR-GCC tooltrain for Mac OS X. Following are the commands
to install using Homebrew (must have Homebrew installed):

  > ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  > brew tap osx-cross/avr
  > brew install avr-libc
  > brew install avrdude --with-usb

2. Install Arduino for Mac OS X by placing the Arduino application in the
Applications folder. Open the application at least one time to overcome
security warnings.

3. Install Install QP-nano for Arduino into your Documents/Arduino folder
(documentation refers to this as the Arduino Sketchbook folder).

4. Install QM.

Instructions for Configuring External Tools for a Model

1. Ensure the model is not open in QM.

2. Copy the Blinky model's settings
(libraries/qpn_avr/examples/blinky/.blinky.macos) from your Arduino
Sketchbook folder to the model's folder.

3. Rename the settings file to your model's name (i.e., .mymodel).

4. Open QM and edit the settings from External Tools.

Once you have a working .model file, copy it to other models and use it as
a starting point.

Known Issues

1.  The serialterm External Tool is working by opening a Terminal window.
An additional Terminal window also appears and can be safely closed. When
the Terminal window displaying the Arduino's output is closed, the port is
busy and can no longer be accessed. Work-around is to unplug and
re-connect the Arduino from the Mac's USB port.
