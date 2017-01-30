This README provides a quick overview of QP-nano for Arduino.
Please refer to the companion webiste to this project at:

http://www.state-machine.com/arduino


Software Installation
=====================
The QP-nano adaptation for Arduino is distributed in a single ZIP
archive qpn-<ver>_arduino-1.6.x.zip, where <ver> stands for version of
the QP-nano framework.

You need to unzip the qpn-<ver>_arduino-1.6.x.zip archive into your
Arduino-Sketchbook folder. To find out where your Sketchbook folder is,
or to configure a different location for your Sketchbook folder, you
need to open the Arduino IDE and select File | Preferences menu. The
sketchbook location will be shown at the top of the Preferences dialog
box. Once you identify the Sketchbook folder, you simply unzip the whole
archive to your Sketchbook.

****
NOTE: Currently, the QP-nano for Arduino project is available only
on Windows. If you are interested in other platforms, such as Linux
or MacOS, please consider contribuing a tested port.
****


Building the Examples in the QM Modeling Tool
=============================================

****
NOTE: To start working with the freeware QM modeling tool, you need to
download the tool from http://sourceforge.net/projects/qpc/files/QM.
****

Each QP-nano example for Arduino (in the examples_avr folder) contains
a QM model, which is a file with the extension .qm., such as
<Sketchbook>\examples_avr\blinky\blinky.qm. These models and the QM
modeling tool take Arduino programming to the next level. Instead of
coding the state machines by hand, you draw them with the free QM
modeling tool, attach simple action code to states and transitions, and
you generate the complete Arduino sketch automatically-—literally by a
press of a button.


Licensing
=========
The QP-nano framework for Arduino is licensed under the modified GPLv3
license (see file GPLv3.txt) with the QP-Arduino Exception (see file
QP-Arduino_GPL_Exception.txt). Specifically, the Exception removes the
requirement to expose your propriatory source code as long as you deploy
it on an Arduino-Certified board.

The QM graphical modeling tool is freeware. The QM tool is provided
under the terms of a simple End-User License Agreement (EULA).


Documentation
=============
Please refer to the companion webiste to this project at:

http://www.state-machine.com/arduino

Specifically, the Application Note: "Event-Driven Arduino Programming
with QP and QM" is available from:

http://www.state-machine.com/doc/AN_Event-Driven_Arduino-1.6.x.pdf


Support Questions
=================
Please submit any questions or comments to the free QP support forum at:
http://sourceforge.net/p/qpc/discussion/668726
