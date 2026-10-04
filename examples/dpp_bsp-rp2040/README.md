# dpp_bsp-rp2040

The Dining Philosophers Problem (DPP) on RP2040 boards with the Earle
Philhower arduino-pico core and the QV kernel of QP/C++ v7.3.4. Five `Philo`
active objects and one `Table` active object exercise publish/subscribe,
direct posting, dynamic events from an event pool, and time events. The
Raspberry Pi Pico is the reference board; any board of that core works.

## What To Expect

| Signal | Pin | Behaviour |
|--------|-----|-----------|
| LED | `LED_BUILTIN` (GP25 on a Pico), or `QP_ARDUINO_LED_PIN` | high while the last reported philosopher is eating |
| tick probe | `QP_ARDUINO_TICK_PROBE_PIN`, if defined | high during each 100 Hz tick callback |
| QS trace | USB CDC | QSPY or QView at 115200 |

QS user records `PHILO_STAT` (philosopher number and state) and `PAUSED_STAT`
report the application state. QS commands from QSPY, QUTest or QView:
1 publishes `PAUSE_SIG`, 2 publishes `SERVE_SIG`, 3 publishes `TEST_SIG`,
4 provokes an assertion (LED on for two seconds, then a watchdog reset).

## Build, Upload, Observe

```sh
pixi run build-dpp-bsp-rp2040
pixi run upload-dpp-bsp-rp2040 /dev/ttyACM0
pixi run qspy -c /dev/ttyACM0 -b 115200
```

Start QSPY within five seconds of the upload to receive the dictionaries; the
BSP waits that long for a USB host before it starts QS
(`QP_ARDUINO_QS_HOST_WAIT_MS`). For QView, run QSPY with `-u` and start
`qview/dpp.py` from QTools.

## Configuration

The same build flags as `blinky_bsp-rp2040`: `QP_ARDUINO_LED_PIN`,
`QP_ARDUINO_TICK_PROBE_PIN`, `QP_ARDUINO_QS_OFF`, `QP_ARDUINO_QS_SERIAL`,
`QP_ARDUINO_QS_HOST_WAIT_MS`, `QP_ARDUINO_QS_SERVICE_BYTES`,
`QP_ARDUINO_QS_SERVICE_US`. The `pico` environment is the default; `pico_uart`
moves the trace to UART0.

## Code Organization

The model is `dpp_bsp-rp2040.qm`; all sources except this file and `qview/`
are generated from it with QM 6.1.1 (`qmc dpp_bsp-rp2040.qm`), so edit the
model, not the code. `bsp.cpp` holds the LED, the tick timer, the QS
transport and the QS command handler.

## The Tick Source

The BSP claims a hardware timer alarm of its own and re-arms it from its own
interrupt, catching up if a target was missed; the idle loop watches the tick
and emits a `TICK_LOST` record if it ever has to re-arm it. The trace
transport is serviced only when `QP_ARDUINO_QS_SERVICE_BYTES` are pending or
`QP_ARDUINO_QS_SERVICE_US` have passed. See `docs/port-notes.md` for why.
