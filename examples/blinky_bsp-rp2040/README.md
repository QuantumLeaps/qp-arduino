# Blinky on the RP2040 qualification fixture

This example implements the simple
[Blinky application](https://www.state-machine.com/qpcpp/tut_blinky.html) on
RP2040 boards using the Earle Philhower arduino-pico core and the QV kernel of
QP/C++ v7.3.4. It toggles the "LED" every 500 ms (1 Hz blink) from one active
object driven by a periodic time event.

The first target is the qp-arduino RP2040 qualification fixture: a spare
`mouse-joystick-pcb` rev 0 board watched by a Kingst LA5032 through
`la5032-instrument-port-adapter` v1.0. The fixture record in
`UPGRADE_QPCPP_V7.md` has the identity, wiring and pinned toolchain.
**Keep the board's 36 V input unplugged.** The BSP applies the board's safe
state first and never enables the 36 V switch or the GPIO isolation bus; USB
alone powers the MCU domain.

## Features Demonstrated

- cooperative QV kernel on ARMv6-M (PRIMASK critical sections)
- system clock tick from a pico-sdk repeating timer, SysTick left to the core
- QP/Spy output over USB CDC with a bounded, non-blocking transport
- analyzer probes on the fixture's two free pins

| Signal | RP2040 | InstrumentPort-40 | LA5032 | Behaviour |
|--------|--------|-------------------|--------|-----------|
| LED / `FW_TRIG0` | GP25 | pin 26, `TRIG0` | ch 16 | toggles every 500 ms |
| tick probe / `FW_TRIG1` | GP21 | pin 28, `TRIG1` | ch 17 | short pulse every 10 ms (100 Hz tick callback) |
| QS trace | USB CDC | — | — | QSPY at 115200 |
| QS trace, `trig1` profile | GP21 (`FW_TRIG1`, PIO UART) | pin 28, `TRIG1` | ch 17 | 115200 8N1, transmit only; the tick probe then moves to GP0 (`PYC_UART_TX` pad, `DIG8`, ch 8) |

An assertion shows as the LED held on for two seconds with the tick probe
silent, followed by a watchdog reset.

## Build Configurations

PlatformIO environments in `platformio.ini`:

- `rp2040_mouse_joystick`: the fixture, QS on, universal boot2 (default)
- `rp2040_mouse_joystick_noqs`: QS compiled out, idle loop sleeps
- `rp2040_mouse_joystick_fastflash`: quad-lane boot2 matched to the fixture's flash
- `rp2040_mouse_joystick_uart`: QS on UART0 (GP0/GP1), no host wait
- `rp2040_mouse_joystick_txonly`: transmit-only observation profile on UART0
- `rp2040_mouse_joystick_trig1`: the same profile on a transmit-only PIO UART
  driving `FW_TRIG1` (GP21), the trace path a board with no free hardware UART
  has to its instrument port; tick probe on GP0

Build flags used by the BSP:

- `-DQP_ARDUINO_FIXTURE_MOUSE_JOYSTICK_PCB=1`: fixture pins and safe state.
  Omit it for a generic RP2040 board: the LED is `LED_BUILTIN` and
  `-DQP_ARDUINO_TICK_PROBE_PIN=<gpio>` optionally adds the tick probe.
- `-DQS_ON`: QS output on `QP_ARDUINO_QS_SERIAL` (default `Serial`, the USB CDC
  port). `-DQP_ARDUINO_QS_SERIAL=Serial1` moves it to UART0 on GP0/GP1, which
  the fixture exposes on LA5032 channels 8 and 7.
- `-DQP_ARDUINO_QS_HOST_WAIT_MS=<ms>` (default 5000): how long the BSP waits
  for a USB CDC host before starting QS, so a QSPY started right after flashing
  receives the dictionaries. Use 0 to boot without waiting.
- `-DQP_ARDUINO_QS_SERVICE_BYTES=<n>` (64) and `-DQP_ARDUINO_QS_SERVICE_US=<us>`
  (1000): the idle loop services the transport only when this many QS bytes
  are pending or this much time has passed.
- `-DQP_ARDUINO_QS_TX_ONLY=1`: never read or parse QS-RX (no reset, poke,
  event injection or command from the trace link).
- `-DQP_ARDUINO_QS_DICT_PERIOD_MS=<ms>` (0 = off): re-emit the BSP's and the
  application's dictionaries (`BSP::setDictionaryHook`) this often.
- `-DQP_ARDUINO_QS_STAT_PERIOD_MS=<ms>` (0 = off): emit `TRACE_STAT` (QS fill
  high-water mark, buffer-full count, buffer size, tick count) this often.

The BSP drives the tick from a hardware timer alarm it claims itself and
re-arms from its own interrupt; the idle loop re-arms it and emits `TICK_LOST`
if it ever stops (see the ledger for the defect this replaced).

## Code Organization

```
examples/blinky_bsp-rp2040/
  blinky_bsp-rp2040.ino   setup(): QF::init, BSP::init, start Blinky; loop(): QF::run()
  blinky.hpp              application interface (signals, AO pointer)
  bsp.hpp / bsp.cpp       fixture safe state, LED and tick probe, tick timer,
                          QS transport and callbacks, Q_onError
  README.md               this file
```

All sources are generated from `blinky_bsp-rp2040.qm` with QM 6.1.1 (`qmc blinky_bsp-rp2040.qm`); edit the model, not the code.

## Building the Example

With Pixi (Linux, macOS, Windows):

```sh
pixi run build-blinky-bsp-rp2040
pixi run upload-blinky-bsp-rp2040 /dev/ttyACM0
pixi run qspy -c /dev/ttyACM0 -b 115200
```

Or with any PlatformIO installation:

```sh
python tools/pio_task.py upload --example examples/blinky_bsp-rp2040 --env rp2040_mouse_joystick --port /dev/ttyACM0
```

Upload uses `picotool` after the 1200 baud touch; a board not running firmware
that honours the touch must be put into BOOTSEL by hand. Start QSPY within
five seconds of the upload to receive the dictionaries.

Analyzer capture of both markers at 1 MSa/s, triggered on the first tick, from
the `instrument-control` Guix environment:

```sh
sigrok-cli --driver kingst-la2016 --config samplerate=1m --samples 6000000 \
    --channels CH16,CH17 --triggers CH17=r -o blinky.sr
```

Measured results for this example are in
`qp-arduino-example/evidence/2026-09-30-rp2040-blinky/`.
