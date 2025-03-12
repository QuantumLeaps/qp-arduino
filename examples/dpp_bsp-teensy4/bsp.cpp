//.$file${.::bsp.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp_bsp-teensy4.qm
// File:  ${.::bsp.cpp}
//
// This code has been generated by QM 5.1.3 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::bsp.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"   // QP-C++ framework
#include "dpp.hpp"     // DPP application
#include "bsp.hpp"     // Board Support Package

using namespace QP;

//............................................................................
// QS facilities

// un-comment if QS instrumentation needed
#define QS_ON

enum AppRecords { // application-specific QS trace records
    PHILO_STAT = QP::QS_USER,
};

static QP::QSpyId const l_TIMER_ID = { 0U }; // QSpy source ID

//----------------------------------------------------------------------------
// BSP functions

void BSP::init(void) {
    // initialize the hardware used in this sketch...
    // NOTE: interrupts are configured and started later in QF::onStartup()
    pinMode(LED_BUILTIN, OUTPUT);

    randomSeed(1234); // seed the Random Number Generator

#ifdef QS_ON
    QS_INIT(0);

    // output QS dictionaries
    QS_OBJ_DICTIONARY(&l_TIMER_ID);
    QS_USR_DICTIONARY(PHILO_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records

#else

    Serial.begin(115200); // set the highest standard baud rate of 115200 bps
    Serial.print("QP-C++: ");
    Serial.print(QP_VERSION_STR);
    Serial.println("");

#endif
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char_t const *stat) {
    digitalWrite(LED_BUILTIN, (stat[0] == 'e') ? HIGH : LOW);

#ifdef QS_ON
    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);  // Philo number
        QS_STR(stat); // Philo status
    QS_END()
#else
    Serial.print("Philosopher ");
    Serial.print(n, DEC);
    Serial.print(" ");
    Serial.println(stat);
#endif
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    char const *msg = paused ? "Paused ON" : "Paused OFF";
#ifndef QS_ON
    Serial.println(msg);
#endif
}
//............................................................................
void BSP::ledOff(void) {
    digitalWrite(LED_BUILTIN, LOW);
}
//............................................................................
void BSP::ledOn(void) {
    digitalWrite(LED_BUILTIN, HIGH);
}

//............................................................................
static uint32_t l_rnd; // random seed

void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time
    return (rnd >> 8);
}

//----------------------------------------------------------------------------
// QF callbacks...

//
// NOTE: The usual source of system clock tick in ARM Cortex-M (SysTick timer)
// is aready used by the Arduino library. Therefore, this code uses a different
// hardware Timer1 of the Teensy 4 board for providing the system clock tick.
//
// NOTE: You can re-define the macros to use a different ATSAM timer/channel.
//
#include <TimerOne.h>  // Teensy Timer1 interface

#define TIMER1_CLCK_HZ  1000000
#define TIMER_HANDLER   T1_Handler

// interrupts.................................................................
void TIMER_HANDLER(void) {
    QF::TICK_X(0, &l_TIMER_ID); // process time events for tick rate 0

#ifndef QS_ON
    if (Serial.available() > 0) {
        switch (Serial.read()) { // read the incoming byte
            case 'p':
            case 'P':
                static QEvt const pauseEvt = { PAUSE_SIG, 0U, 0U};
                QF::PUBLISH(&pauseEvt, &l_TIMER_ID);
                break;
            case 's':
            case 'S':
                static QEvt const serveEvt = { SERVE_SIG, 0U, 0U};
                QF::PUBLISH(&serveEvt, &l_TIMER_ID);
                break;
        }
    }
#endif
}
//............................................................................
void QF::onStartup(void) {
    // configure the timer-counter channel........
    Timer1.initialize(TIMER1_CLCK_HZ / BSP::TICKS_PER_SEC);
    Timer1.attachInterrupt(TIMER_HANDLER);
    // ...
}
//............................................................................
void QV::onIdle(void) { // called with interrupts DISABLED
#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode. You might
    // need to customize the clock management for your application,
    // see the datasheet for your particular MCU.
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
#else
    QF_INT_ENABLE(); // simply re-enable interrupts

#ifdef QS_ON

    // transmit QS outgoing data (QS-TX)
    uint16_t len = Serial.availableForWrite();
    if (len > 0U) { // any space available in the output buffer?
        uint8_t const *buf = QS::getBlock(&len);
        if (buf) {
            Serial.write(buf, len); // asynchronous and non-blocking
        }
    }

    // receive QS incoming data (QS-RX)
    len = Serial.available();
    if (len > 0U) {
        do {
            QP::QS::rxPut(Serial.read());
        } while (--len > 0U);
        QS::rxParse();
    }

#endif // QS_ON

#endif
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int location) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)location;

    QF_INT_DISABLE(); // disable all interrupts
    BSP::ledOn();  // trun the LED on
    for (;;) { // freeze in an endless loop for now...
    }
}

//----------------------------------------------------------------------------
// QS callbacks...
#ifdef QS_ON

//............................................................................
bool QP::QS::onStartup(void const * arg) {
    static uint8_t qsTxBuf[1024]; // buffer for QS transmit channel (QS-TX)
    static uint8_t qsRxBuf[128];  // buffer for QS receive channel (QS-RX)
    initBuf  (qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
    Serial.begin(115200); // run serial port at 115200 baud rate
    return true; // return success
}
//............................................................................
void QP::QS::onCommand(uint8_t cmdId, uint32_t param1,
                       uint32_t param2, uint32_t param3)
{
}

#endif // QS_ON

//............................................................................
void QP::QS::onCleanup(void) {
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime(void) {
    return millis();
}
//............................................................................
void QP::QS::onFlush(void) {
#ifdef QS_ON
    uint16_t len = 0xFFFFU; // big number to get as many bytes as available
    uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
    while (buf != nullptr) { // data available?
        Serial.write(buf, len); // might poll until all bytes fit
        len = 0xFFFFU; // big number to get as many bytes as available
        buf = QS::getBlock(&len); // try to get more data
    }
    Serial.flush(); // wait for the transmission of outgoing data to complete
#endif // QS_ON
}
//............................................................................
void QP::QS::onReset(void) {
    //??? TBD for Teensy
}
