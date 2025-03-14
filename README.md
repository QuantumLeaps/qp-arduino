## Brought to you by:
[![Quantum Leaps](https://www.state-machine.com/attachments/logo_ql_400.png)](https://www.state-machine.com)

---------------------------------------------------------------------
![QP-Arduino](https://www.state-machine.com/img/logo_qp-arduino.png)

This README provides a quick overview of QP for Arduino.

More detailed documentation for qpcpp, qm, and qtools can be found at:

https://quantumleaps.github.io/qp-arduino/

Please refer to the companion website to this project at:

https://www.state-machine.com/arduino

----
# Provided Library and Examples

This repository contains one external library for ARM Cortex-M based Arduinos:

- `qp-arduino` -- QP/C++ framework for Arduinos based on ARM Cortex-M

----
# Software Installation

The QP adaptation for Arduino may be used either with the [Arduino
IDE](https://www.arduino.cc/en/software) or with
[PlatformIO](https://platformio.org/install/cli).

## Download Library

The QP adaptation for Arduino can be downloaded in a single zip archive or
cloned to a local git repository.

### Zip File

https://github.com/QuantumLeaps/qp-arduino/archive/refs/heads/master.zip

### Git

```bash
git clone https://github.com/QuantumLeaps/qp-arduino.git
```

## Arduino IDE

You need to unzip the zip archive, or clone the git repository, into your
Arduino `<Sketchbook>` folder. To find out where your `<Sketchbook>` folder is,
or to configure a different location for your `<Sketchbook>` folder, you need to
open the Arduino IDE and select `File|Preferences` menu. The Sketchbook location
will be shown at the top of the Preferences dialog box. Once you identify the
`<Sketchbook>` folder, you simply unzip the whole zip archive, or clone the git
repository, to this folder, renaming the folder qp-arduino if necessary.

After the installation, your Arduino `<Sketchbook>` folder should look
as follows:

```
<Sketchbook>/
└── libraries
    └── qp-arduino
        ├── examples
        │   ├── blinky-sam
        │   ├── blinky2-sam
        │   ├── blinky_bsp-sam
        │   ├── blinky_bsp-teensy4
        │   ├── dpp_bsp-sam
        │   └── dpp_bsp-teensy4
        └── src
            └── qp-arduino
```

## PlatformIO

Unzip the zip archive or clone the git repository into any folder you prefer.

----
# Additional Tools

The QM modeling tool may be downloaded and installed from:

https://www.state-machine.com/products/qm

----
# Working with the Examples

The intended way of working with the examples is to use the
[QM modeling tool](https://www.state-machine.com/products/qm/) to modify
the **models** (`.qm` files) and then to **generate code** from these
models.

In this scenario, the Arduino IDE or PlatformIO are used only to build and
upload the code to the Arduino board(s).

## Arduino IDE

Assumming that you have installed the QP library in your `<Sketchbook>`
folder, the examples provided in each QP library show up in the Arduino
IDE under the menu:

`File|Examples|Examples from Custom Libraries`

> **NOTE:** The preferences in the Arduino IDE should be set up to
**Use external editor** so that the IDE will update the code each time
it is re-generated by the QM modeling tool.

The Arduino IDE serial monitor may be used to observe the output of the dpp
examples.

## PlatformIO

PlatformIO may be used either with its IDE or on the command line.

### Command Line

Navigate inside the qp-arduino directory where the platformio.ini file is contained.

Modify the platformio.ini file to compile the example that you prefer.

Compile and upload the example with a command like this:

```bash
pio run -e teensy41 -t upload
```

To use the serial monitor to observe the output of the dpp examples run:

```bash
pio device monitor
```

----
## Example Documentation

Each example model has documentation, which you can view in the QM tool.
The model documentation shows in the Property Editor when the model item
is active.

----
# QP/Spy Software Tracing Support

The qpcpp_arm-cm library now supports the
[QP/Spy software tracing](https://www.state-machine.com/software-tracing).
This feature is also demonstrated in the qpcpp examples (with the BSP).

To activate the software tracing, you would need to un-comment the `QS_ON`
macro at the top of bsp.cpp source file.

> **NOTE:** Because the QP/Spy output is binary, you cannot watch it
with a standard serial terminal. Instead, you need to use the
[QSPY host utility](https://www.state-machine.com/qtools/qspy.html).

----
## QView Demonstration
Once QP/Spy tracing is enabled, this communication with the target system
can be used to for
[QView visualization and monitoring](https://www.state-machine.com/qtools/qview.html)

The `dpp_bsp-sam` example demonstrates the [QView monitoring]. To see this
feature, you need to upload the dpp_bsp-sam code to the Arduino UNO board and
then you need to launch QView, from the sub-directory `qview` in the
`dpp_bsp-sam` example.


----
# Licensing

The QP/C++ real-time embedded framework (RTEF) for Arduino are licensed
under the modified GPLv3 license with the
[QP-Arduino Exception](https://www.state-machine.com/licenses/QP-Arduino_GPL_Exception.txt)
(see also the file [QP-Arduino_GPL_Exception.txt](./QP-Arduino_GPL_Exception.txt)
in the root of the archive).

Specifically, the Arduino GPLv3 Exception removes the requirement to expose your
proprietary source code as long as you deploy it on an Arduino-Certified
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
