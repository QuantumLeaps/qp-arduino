# dpp_bsp-rp2040

Dining Philosophers Problem (DPP) on the QV kernel of QP/C++ v7.3.4 for
RP2040 boards using the Earle Philhower arduino-pico core. Five `Philo` active
objects and one `Table` active object exercise publish/subscribe, direct
posting, dynamic events from an event pool, and time events. The model is
`dpp_bsp-rp2040.qm`; all sources except `README.md` and `qview/` are generated from it with
QM 6.1.1 (`qmc dpp_bsp-rp2040.qm`), so edit the model, not the code.

The first target is the qp-arduino RP2040 qualification fixture, a spare
`mouse-joystick-pcb` rev 0 board watched by a Kingst LA5032. **Keep the board's
36 V input unplugged.** The BSP applies the board's safe state first and never
enables the 36 V switch or the GPIO isolation bus.

## What to expect

| Signal | RP2040 | InstrumentPort-40 | LA5032 | Behaviour |
|--------|--------|-------------------|--------|-----------|
| LED / `FW_TRIG0` | GP25 | pin 26, `TRIG0` | ch 16 | high while the last reported philosopher is eating |
| tick probe / `FW_TRIG1` | GP21 | pin 28, `TRIG1` | ch 17 | high during each 100 Hz tick callback |
| QS trace | USB CDC | — | — | QSPY or QView at 115200 |

QS user records `PHILO_STAT` (philosopher number and state) and `PAUSED_STAT`
report the application state. QS commands from QSPY, QUTest or QView:
1 publishes `PAUSE_SIG`, 2 publishes `SERVE_SIG`, 3 publishes `TEST_SIG`,
4 provokes an assertion (LED on for two seconds, then a watchdog reset).

## Build, upload, observe

```sh
pixi run build-dpp-bsp-rp2040
pixi run upload-dpp-bsp-rp2040 /dev/ttyACM0
pixi run qspy -c /dev/ttyACM0 -b 115200
```

Start QSPY within five seconds of the upload to receive the dictionaries; the
BSP waits that long for a USB host before it starts QS
(`QP_ARDUINO_QS_HOST_WAIT_MS`). QView: run QSPY with `-u` and start
`qview/dpp.py` from QTools as in the Teensy example.

## Configuration

Same build flags as `blinky_bsp-rp2040`: `QP_ARDUINO_FIXTURE_MOUSE_JOYSTICK_PCB`,
`QS_ON`, `QP_ARDUINO_QS_SERIAL`, `QP_ARDUINO_QS_HOST_WAIT_MS`,
`QP_ARDUINO_TICK_PROBE_PIN`.
