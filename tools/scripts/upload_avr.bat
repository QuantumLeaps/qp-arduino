@echo off
:: ===========================================================================
:: Product: Batch script for uploading Arduino sketches
:: Last updated for: Arduino 1.6.x
:: Date of the Last Update: 2016-08-14
::
::                    Q u a n t u m     L e a P s
::                    ---------------------------
::                    innovating embedded systems
::
:: Copyright (C) Quantum Leaps, LLC. All rights reserved.
::
:: This program is open source software: you can redistribute it and/or
:: modify it under the terms of the GNU General Public License as published
:: by the Free Software Foundation, either version 3 of the License, or
:: (at your option) any later version.
::
:: This program is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
:: GNU General Public License for more details.
::
:: You should have received a copy of the GNU General Public License
:: along with this program. If not, see <http://www.gnu.org/licenses/>.
::
:: Contact information:
:: http://sourceforge.net/p/qpc/discussion/668726
:: ===========================================================================

setlocal

set PATH=%ARDUINO_HOME%

"%ARDUINO_SKETCHBOOK%\tools\utils\avrdude" -p %BOARD_MCU% -c arduino -P %BOARD_COM% -b %BOARD_BAUD% -D -v -C "%ARDUINO_HOME%\hardware\tools\avr\etc\avrdude.conf" -U flash:w:bin\%1.hex:i

endlocal