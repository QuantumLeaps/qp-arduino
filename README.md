This README provides a quick overview of QP for Arduino.
Please refer to the companion webiste to this project at:

https://www.state-machine.com/arduino

----
# Software Installation

The QP adaptation for Arduino is distributed in a single ZIP
archive `qp-<ver>_arduino-1.8.x.zip`, where `<ver>` stands for
the version of the QP frameworks.

You need to unzip the `qp-<ver>_arduino-1.8.x.zip` archive into your
Arduino `<Sketchbook>` folder. To find out where your `<Sketchbook>` folder
is, or to configure a different location for your `<Sketchbook>` folder,
you need to open the Arduino IDE and select `File|Preferences` menu.
The Sketchbook location will be shown at the top of the Preferences
dialog box. Once you identify the `<Sketchbook>` folder, you simply unzip
the whole `qp-<ver>_arduino-1.8.x.zip` archive to this folder.

----
# Provided Libraries and Tools

The archive `qp-<ver>_arduino-1.8.x.zip` contains two external libraries
for SAM-based Arduinos:

- `qpcpp_sam` -- QP/C++ framework for SAM-based Arduinos
- `qpn_avr`   -- QP-nano framework for AVR-based Arduinos
                 
NOTE: qpn_avr is now obsolete. It is provided for backgwards compatibility only!
 

The archive also contains the QM modeling tool for Windows

- `qm`        -- QM modeling tool for Windows 32/64-bit

After the installation, your Arduino `<Sketchbook>` folder should look
as follows:

    <Sketchbook>/             - your Arduino Sketchbook folder
     |
     +-libraries/             - libraries folder
     | +-qpcpp_sam/           - QP/C++ library for SAM-based Arduinos
     | | +-examples/          - QP/C++ library examples
     | | | +-blinky/          - Blinky example
     | | | | +-blinky.ino     - Blinky code (generated)
     | | | | +-blinky.qm      - Blinky model (for QM tool)
     | | +-src/               - QP/C++ library source code
     | | | +-qpcpp.h          - QP/C++ library header file
     | | | +-...              - QP/C++ library sources
     | | +-library.properties - QP/C++ library properties
     | |
     +-libraries/             - libraries folder
     | +-qpn_avr/             - QP-nano library for AVR-based Arduinos
     | | +-...                  (now obsolete, for backgwards compatibility only) 
     | |
     | +-qm/                  - QM modeling tool for Windows
     | | +-bin/               - QM binaries (executable and DLLs)
     | | | +-qm.exe           - QM executable for Windows
     | | +-Resources/         - QM resources
     | | | +-...
     |
     +-README.md              - this file
     +-QP-Arduino_GPL_Exception.txt - GPL exception for QP on Arduino


NOTE: The QP-Arduino archive contains QM for Windows only. But QM is also
available for Linux and MacOS hosts. If you wish to work on those operating
systems, you will need to install QM separately, as described at:
https://www.state-machine.com/qm/gs.html 


----
# Working with the Examples

Assumming that you have installed the QP library in your `<Sketchbook>`
folder, the examples provided in each QP library show up in the Arduino
IDE under the menu:

`File|Examples|Examples from Custom Libraries`

The intended way of working with the examples is to use the
[QM modeling tool](https://www.state-machine.com/products/qm/) to modify
the **models** (`.qm` files) and then to **generate code** from these
models. In this scenario, the Arduino IDE is used only to build and
upload the code to the Arduino board(s).

> **NOTE:** The preferences in the Arduino IDE should be set up to
**Use external editor** so that the IDE will update the code each time
it is re-generated by the QM modeling tool.


----
# Example Documentation

Each example model has documentation, which you can view in the QM tool.
The model documentation shows in the Property Editor when the model item
is active.


----
# Licensing

The QP/C++ real-time embedded framework (RTEF) for Arduino are licensed
under the modified GPLv3 license with the
[QP-Arduino Exception](https://www.state-machine.com/licensing/QP-Arduino_GPL_Exception.txt)
(see also the file QP-Arduino_GPL_Exception.txt in the root of the archive).
Specifically, the Exception removes the requirement to expose your
propriatory source code as long as you deploy it on an Arduino-Certified
board.

The [QM model-based design tool](https://www.state-machine.com/qm/) is freeware.
The QM tool is provided under the terms of a simple
[End-User License Agreement (EULA)](https://www.state-machine.com/qm/license.html).


----
# Companion Web Page

Please refer to the companion web page to this project at:

https://www.state-machine.com/arduino


----
# Support Questions

Please submit any questions or comments to the free QP support forum at:

https://sourceforge.net/p/qpc/discussion/668726
