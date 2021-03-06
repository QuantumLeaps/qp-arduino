/*.$file${.::bsp_uno.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: blinky_bsp.qm
* File:  ${.::bsp_uno.cpp}
*
* This code has been generated by QM 5.1.3 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*.$endhead${.::bsp_uno.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* This file has been prepared for Doxygen **************************/

/*! \file bsp_uno.ino *************************************************
*
* \brief Arduino Board Support Package for the Arduino-UNO
*
* Copyright (C) 2021 W.Nijs (ALF4all)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* \author   W.Nijs.
* \date     04/01/2021
* \version  1.0 04/01/2021, initial revision, W.Nijs
*
*********************************************************************/
/*****************************************************************//**
* Include section
*  Add all #includes here.
*********************************************************************/
#include "qpn.h"       // QP-nano framework
#include "blinky.hpp"  // Blinky interface
#include "bsp.hpp"     // BSP for this application
#include "Arduino.h"  // Main include file for the Arduino SDK

//#include <avr/interrupt.h>

/*****************************************************************//**
* Define section
*  Add all #defines here.
*********************************************************************/
#define DEBUG
#define BUTTON          2 // Can only be pin 2 or 3 on the Uno
#define ONBOARDLED     13

//unsigned long previousMillis = 0;


/*****************************************************************//**
* ISR BSP_buttonPressed()
*********************************************************************/
void ISR_buttonPressed() {
  QACTIVE_POST_ISR((QActive *)&AO_Blinky, Q_BUTTON_PRESSED_SIG, 0U);
};

//============================================================================
static QEvt l_blinkyQSto[10]; // Event queue storage for Blinky
//...

//============================================================================
// QF_active[] array defines all active object control blocks ----------------
QActiveCB const Q_ROM QF_active[] = {
    { (QActive *)0,           (QEvt *)0,        0U                  },
    { (QActive *)&AO_Blinky,  l_blinkyQSto,     Q_DIM(l_blinkyQSto) }
};

//............................................................................
void setup() {
    // initialize the QF-nano framework
    QF_init(Q_DIM(QF_active));
    BSP_initSetup();

    // initialize all AOs...
    Blinky_ctor();
}

//............................................................................
void loop() {
    QF_run(); // run the QF-nano framework
}

/*****************************************************************//**
* BSP_initSetup()
*********************************************************************/
void BSP_initSetup(void) {
  // initialize the hardware used in this sketch...
  pinMode(BUTTON, INPUT_PULLUP); // set the BUTTON pin to input
  attachInterrupt(digitalPinToInterrupt(2),ISR_buttonPressed,FALLING);
  pinMode(ONBOARDLED, OUTPUT);   // set the ONBOARDLED pin to output
#if defined(DEBUG)
  Serial.begin(115200);
  Serial.println("--- Blinky by ALF4all ---");
  Serial.println("-------------------------");
#endif
};

/*****************************************************************//**
* BSP_initTick()
*********************************************************************/
void BSP_initTick(void) {
  // set Timer2 in CTC mode, 1/1024 prescaler, start the timer ticking...
  TCCR2A = (1U << WGM21) | (0U << WGM20);
  TCCR2B = (1U << CS22 ) | (1U << CS21) | (1U << CS20); // 1/2^10
  ASSR  &= ~(1U << AS2);
  TIMSK2 = (1U << OCIE2A); // enable TIMER2 compare Interrupt
  TCNT2  = 0U;

  // set the output-compare register based on the desired tick frequency
  OCR2A  = (F_CPU / BSP_TICKS_PER_SEC / 1024U) - 1U;
};

/*****************************************************************//**
* BSP_onboardLedOff()
*********************************************************************/
void BSP_onboardLedOff(void) {
  digitalWrite(ONBOARDLED, LOW);
#if defined(DEBUG)
  Serial.println("LED off");
#endif
};

/*****************************************************************//**
* BSP_onboardLedOn()
*********************************************************************/
void BSP_onboardLedOn(void) {
  digitalWrite(ONBOARDLED, HIGH);
#if defined(DEBUG)
  Serial.println("LED on");
#endif
};

/*****************************************************************//**
* ISR TIMER2 COMPA
*********************************************************************/
ISR(TIMER2_COMPA_vect) {
  QF_tickXISR(0); // process time events for tick rate 0
};


//============================================================================
// QF callbacks...
void QF_onStartup(void) {
    BSP_initTick();
}
//............................................................................
void QV_onIdle(void) {   // called with interrupts DISABLED
    // Put the CPU and peripherals to the low-power mode. You might
    // need to customize the clock management for your application,
    // see the datasheet for your particular AVR MCU.
    SMCR = (0 << SM0) | (1 << SE); // idle mode, adjust to your project
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
}
//............................................................................
Q_NORETURN Q_onAssert(char const Q_ROM * const module, int location) {
    // implement the error-handling policy for your application!!!
    (void)module;
    (void)location;
    QF_INT_DISABLE(); // disable all interrupts
    QF_RESET();  // reset the CPU
    for (;;) {
    }
}


