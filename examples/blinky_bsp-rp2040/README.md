# blinky_bsp-rp2040

QP/C++ v7.3.4 Blinky on the QV kernel for RP2040 boards using the Earle
Philhower arduino-pico core. The first target is the qp-arduino RP2040
qualification fixture, a spare `mouse-joystick-pcb` rev 0 board watched by a
Kingst LA5032 through `la5032-instrument-port-adapter` v1.0. The fixture
record in `UPGRADE_QPCPP_V7.md` has the full identity, wiring and pinned
toolchain.

**Keep the board's 36 V input unplugged.** The example applies the board's
safe state first and never enables the 36 V switch or the GPIO isolation bus.
USB alone powers the MCU domain.

## What to expect

| Signal | RP2040 | InstrumentPort-40 | LA5032 | Behaviour |
|--------|--------|-------------------|--------|-----------|
| LED / `FW_TRIG0` | GP25 | pin 26, `TRIG0` | ch 16 | toggles every 500 ms (2 Hz) |
| tick probe / `FW_TRIG1` | GP21 | pin 28, `TRIG1` | ch 17 | short pulse every 10 ms (100 Hz tick ISR) |
| QS trace | USB CDC | — | — | QSPY at 115200 on the board's CDC port |

An assertion shows as the LED held on for two seconds with the tick probe
silent, followed by a watchdog reset.

## Build, upload, observe

With Pixi (Linux, macOS, Windows):

```sh
pixi run build   --example examples/blinky_bsp-rp2040 --env rp2040_mouse_joystick
pixi run upload  --example examples/blinky_bsp-rp2040 --env rp2040_mouse_joystick --port /dev/ttyACM0
```

Or with any PlatformIO installation:

```sh
python tools/pio_task.py upload --example examples/blinky_bsp-rp2040 --env rp2040_mouse_joystick --port /dev/ttyACM0
```

Upload uses `picotool` and the 1200 baud touch; a board that is not running
firmware that honours the touch must be put into BOOTSEL by hand.

QSPY on the host (qtools v7.3.4):

```sh
qspy -c /dev/ttyACM0 -b 115200
```

Analyzer capture of both markers at 1 MSa/s, triggered on the first tick:

```sh
sigrok-cli --driver kingst-la2016 --config samplerate=1m --samples 5m \
    --channels CH16,CH17 --triggers CH17=r -o blinky.sr
```

## Configuration

`build_flags` in `platformio.ini` select the variant:

- `-DQP_ARDUINO_FIXTURE_MOUSE_JOYSTICK_PCB=1`: the fixture pins and safe state
  above. Omit it for a generic RP2040 board: the LED is `LED_BUILTIN` and
  `-DQP_ARDUINO_TICK_PROBE_PIN=<gpio>` optionally adds the tick probe.
- `-DQS_ON`: QS output on `QP_ARDUINO_QS_SERIAL` (default `Serial`, the USB CDC
  port). `-DQP_ARDUINO_QS_SERIAL=Serial1` moves it to UART0 on GP0/GP1, which
  the fixture exposes on LA5032 channels 8 and 7 for an analyzer-side decode.
