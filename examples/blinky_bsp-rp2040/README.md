# blinky_bsp-rp2040

The simple [Blinky application](https://www.state-machine.com/qpcpp/tut_blinky.html)
on RP2040 boards with the Earle Philhower arduino-pico core and the QV kernel
of QP/C++ v7.3.4: one active object, driven by a periodic time event, toggles
the LED every 500 ms. The Raspberry Pi Pico is the reference board; any board
of that core works.

## Features Demonstrated

- the cooperative QV kernel on ARMv6-M (PRIMASK critical sections)
- a 100 Hz system clock tick from a hardware timer alarm the BSP owns
- QP/Spy software tracing over USB CDC with a bounded, non-blocking transport
- a transmit-only trace profile over UART0 or over a PIO UART on any free GPIO
- an optional analyzer probe pulsed by the tick callback

| Signal | Pin | Behaviour |
|--------|-----|-----------|
| LED | `LED_BUILTIN` (GP25 on a Pico), or `QP_ARDUINO_LED_PIN` | toggles every 500 ms |
| tick probe | `QP_ARDUINO_TICK_PROBE_PIN`, if defined | one pulse per 10 ms tick, as long as the tick callback |
| QS trace | USB CDC (`Serial`), UART0 (`Serial1`) or a PIO UART | 115200 baud, binary, read with QSPY |

An assertion shows as the LED held on for two seconds with the tick probe
silent, followed by a watchdog reset.

## Build Configurations

PlatformIO environments in `platformio.ini`:

| Environment | What it is |
|---|---|
| `pico` | QS on over USB CDC (default) |
| `pico_noqs` | QS compiled out; the idle loop sleeps. Baseline timing |
| `pico_uart` | QS on UART0 (GP0 TX, GP1 RX), no host wait |
| `pico_txonly` | transmit-only observation profile on UART0 |
| `pico_pio_trace` | the same profile on a PIO UART driving GP21; tick probe on GP20 |

Build flags read by the BSP:

- `QP_ARDUINO_LED_PIN` (default `LED_BUILTIN`): the GPIO the active object
  toggles. Set it for a board whose core defines no `LED_BUILTIN` or whose LED
  is somewhere else. On a Pico W, `LED_BUILTIN` is driven through the wireless
  chip, which pulls the CYW43 driver into the build (about 330 kB of flash) and
  has not been tested with this port; point the flag at a plain GPIO instead.
- `QP_ARDUINO_TICK_PROBE_PIN` (default: none): a GPIO pulsed by the tick
  callback, for a logic analyzer or oscilloscope. The pulse width is the
  callback's duration and the pulse period is the tick.
- `QP_ARDUINO_QS_OFF`: compile the trace out. Tracing is on by default, with
  QS output on `QP_ARDUINO_QS_SERIAL` (default `Serial`, the USB CDC port);
  `-DQP_ARDUINO_QS_SERIAL=Serial1` moves it to UART0 on GP0/GP1.
- `QP_ARDUINO_QS_HOST_WAIT_MS` (default 5000): how long the BSP waits for a USB
  CDC host before starting QS, so a QSPY started right after flashing receives
  the dictionaries; USB CDC discards output written before a host opens the
  port. 0 boots without waiting; a UART needs no wait.
- `QP_ARDUINO_QS_SERVICE_BYTES` (64) and `QP_ARDUINO_QS_SERVICE_US` (1000): the
  idle loop services the transport only when this many QS bytes are pending or
  this much time has passed. Every call into the USB CDC port runs the USB
  stack's task, which otherwise inflates tick and dispatch timings.
- `QP_ARDUINO_QS_TX_ONLY=1`: never read or parse QS-RX, so nothing on the trace
  link can reset, poke or command the target.
- `QP_ARDUINO_QS_DICT_PERIOD_MS` (0 = off): re-emit the BSP's and the
  application's dictionaries (`BSP::setDictionaryHook`) this often, so a
  capture that misses boot is still decodable by itself.
- `QP_ARDUINO_QS_STAT_PERIOD_MS` (0 = off): emit a `TRACE_STAT` record (QS
  buffer high-water mark, buffer-full count, buffer size, tick count) this
  often, so trace loss is reported from the target.
- `QP_ARDUINO_QS_PIO_TX_PIN`: send QS over a transmit-only PIO UART on this
  GPIO instead of a hardware port, for a board whose hardware UARTs are all
  spoken for. Mutually exclusive with `QP_ARDUINO_QS_SERIAL`.
- `QP_ARDUINO_QS_BAUD` (115200): baud rate of a UART or PIO UART transport.

## Code Organization

```
examples/blinky_bsp-rp2040/
  blinky_bsp-rp2040.ino   setup(): QF::init, BSP::init, start Blinky; loop(): QF::run()
  blinky.hpp              application interface (signals, AO pointer)
  bsp.hpp / bsp.cpp       LED and tick probe, tick timer, QS transport and
                          callbacks, Q_onError
  README.md               this file
```

All sources except this file are generated from `blinky_bsp-rp2040.qm` with
QM 6.1.1 (`qmc blinky_bsp-rp2040.qm`); edit the model, not the code.

## Building the Example

With Pixi (Linux, macOS, Windows):

```sh
pixi run build-blinky-bsp-rp2040
pixi run upload-blinky-bsp-rp2040 /dev/ttyACM0
pixi run qspy -c /dev/ttyACM0 -b 115200
```

Or with any PlatformIO installation:

```sh
python tools/pio_task.py upload --example examples/blinky_bsp-rp2040 --env pico --port /dev/ttyACM0
```

In the Arduino IDE, open the example from *File | Examples* and select a board
of the arduino-pico core; tracing is on by default, so nothing else is needed.

Upload uses `picotool` after the 1200 baud touch; a board not running firmware
that honours the touch must be put into BOOTSEL by hand. Start QSPY within
five seconds of the upload to receive the dictionaries.

## What To Expect

QSPY shows `Trg-RST`, the dictionaries, `St-Init` and then one `Tick<0>`
record every 10 ms with the time event's expiry and the LED's `on` and `off`
state changes every 500 ms. Under the `pico_txonly` and `pico_pio_trace`
profiles, a `TRACE_STAT` record arrives every second and the dictionaries
every ten; a capture decoded offline with `qspy -f` is fully named after one
dictionary period.

## The Tick Source

The BSP claims a hardware timer alarm of its own (`hardware_alarm_claim_unused`)
and re-arms it from its own interrupt before calling `QTimeEvt::tick()`,
catching up if a target was missed; the idle loop watches the tick and emits
a `TICK_LOST` record if it ever has to re-arm it. A repeating timer in the
core's shared alarm pool was tried first and stopped silently under trace-host
attach and detach cycling; see `docs/port-notes.md`.
