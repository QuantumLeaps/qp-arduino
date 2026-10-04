![QP Framework](https://www.state-machine.com/img/qp_banner.jpg)

# qp-arduino

[QP/C++](https://www.state-machine.com/qpcpp) real-time embedded framework for
Arduino boards based on ARM Cortex-M, packaged as one Arduino library with the
cooperative QV kernel and QP/Spy software tracing built in. Framework author:
[Quantum Leaps](https://www.state-machine.com). Companion website:
<https://www.state-machine.com/arduino>.

# What's New?

| Item | Value |
|---|---|
| Package version | 7.3.4 |
| qpcpp source tag | `v7.3.4` (`10441f7a03fa956b9e4772a073a05a96487983e2`) |
| QM version | 6.1.1 |
| QTools version | 7.3.4 |
| Qualified configuration | RP2040 with the Earle Philhower arduino-pico core 4.2.0 |
| Status | 7.3.4 release candidate; tag not cut |

7.3.4 is the QP/C++ v7 line of qp-arduino. It uses the strict v7 API
(`QP_API_VERSION 9999`), keeps the imported framework files byte-identical to
the upstream tag except for a few documented Arduino deltas, and was qualified
on RP2040 with a logic analyzer and the QP/Spy tooling: 100 Hz tick within
±1 % with no missed or doubled tick over two hours, interrupt-to-dispatch
latency well under one tick, host-driven coverage of queues, publish/subscribe,
dynamic events, time events, idle and assertions, and bounded behaviour with
the trace reader absent, slow or disconnected. [`CHANGELOG.md`](CHANGELOG.md)
has the release notes; [`docs/port-notes.md`](docs/port-notes.md) explains how
the port works and what a board support package must do.

> **NOTE:** The qpcpp `v7.3.4` tag contains generated framework headers that
> still report `QP_VERSION_STR "7.3.3"` and `QP_VERSION 733U`. This package
> preserves those upstream files unchanged and identifies the imported source
> by the `v7.3.4` tag.

## Supported configurations

| Target | Core and toolchain | Status |
|---|---|---|
| RP2040 (Raspberry Pi Pico and other boards of the core) | arduino-pico 4.2.0 (Earle Philhower), GCC 14.2.0 | **qualified**: the configuration behind the measurements above |
| Teensy 4.1 | Teensyduino (PlatformIO `teensy@5.1.0`) | builds; the retained regression set, not measured on hardware in this release |
| Arduino Due (ATSAM3X8E) | Atmel SAM platform | builds, best effort |

The port compiles only for ARM Cortex-M (ARMv6-M with PRIMASK critical
sections, ARMv7-M with BASEPRI). Only the QV cooperative kernel is included,
and QP/Spy (`Q_SPY`) is always compiled in. `library.properties` says
`architectures=rp2040`, the arduino-pico core's architecture string, because
RP2040 is the only configuration with target evidence; the Arduino IDE shows
its "may be incompatible" warning on other boards, and the Teensy and Due
examples still build. Classic 8-bit AVR boards are not supported.

# Getting Started with qp-arduino

The library works with the [Arduino IDE](https://www.arduino.cc/en/software)
or with [PlatformIO](https://platformio.org/install/cli). The intended workflow
is to draw the active objects and state machines in the
[QM modeling tool](https://www.state-machine.com/products/qm/), generate code,
and use the IDE or PlatformIO only to build and upload. Handwritten QP/C++
works just as well; the `port_validation-rp2040` example is handwritten.

## The library and the examples

One library for ARM Cortex-M based Arduinos: `qp-arduino`, the QP/C++
framework with the QV kernel (`#include <qpcpp.hpp>`).

| Example | Board | Notes |
|---|---|---|
| `blinky_bsp-rp2040` | RP2040, Raspberry Pi Pico by default | one active object and a time event; the QS transport profiles |
| `dpp_bsp-rp2040` | RP2040 | Dining Philosophers: six active objects, publish/subscribe, event pool, QView scripts |
| `port_validation-rp2040` | RP2040 | host-driven load generator and probes used to qualify the port; a template for checking a new board or core version |
| `blinky_bsp-teensy4`, `dpp_bsp-teensy4`, `port_validation-teensy4` | Teensy 4.1 | retained regression set |
| `blinky-sam`, `blinky2-sam`, `blinky_bsp-sam`, `dpp_bsp-sam` | Arduino Due | best effort |

Each `*_bsp-*` example keeps board specifics in `bsp.cpp` so the active
objects stay portable; the `blinky_bsp-*` and `dpp_bsp-*` examples are
generated from their `.qm` models. The RP2040 examples assume only
`LED_BUILTIN` (GP25 on a Pico) and USB CDC; two build flags,
`QP_ARDUINO_LED_PIN` and `QP_ARDUINO_TICK_PROBE_PIN`, adapt them to any other
board of the core. Every example directory has a README.

## Arduino IDE

Install the library with the Library Manager (search for `qp-arduino`) or put
this repository in your `<Sketchbook>/libraries/` directory as `qp-arduino`,
from the
[zip archive](https://github.com/QuantumLeaps/qp-arduino/archive/refs/heads/main.zip)
or with `git clone https://github.com/QuantumLeaps/qp-arduino.git`. The
examples then appear under *File | Examples | Examples from Custom Libraries*.

Set *Use external editor* in the IDE preferences so the IDE reloads code that
QM regenerates.

> **NOTE:** The Arduino port always compiles with `Q_SPY`, so every BSP must
> provide the QS callbacks. The examples enable the trace with `QS_ON`; the
> output is binary and is read with QSPY from QTools, not a serial monitor.

## PlatformIO via Pixi (recommended)

[Pixi](https://pixi.sh) gives Linux, macOS and Windows the same PlatformIO
installation from `pixi.toml` and `pixi.lock`, plus pinned QM and QTools.
Every task is a thin wrapper around `pio` or a Python script in `tools/`, so
nothing requires Pixi; see the next section for plain PlatformIO.

```sh
pixi install                        # PlatformIO, Python, git, build tools
pixi run ports                      # list serial ports
pixi run build                      # default: examples/blinky_bsp-rp2040 for the Pico
pixi run upload                     # default port /dev/ttyACM0
pixi run build  examples/dpp_bsp-rp2040 pico
pixi run upload examples/dpp_bsp-rp2040 pico /dev/ttyACM0
pixi run monitor /dev/ttyACM0
```

Task arguments are positional: example directory, PlatformIO environment
(`pico`, `pico_uart`, `pico_txonly`, `pico_pio_trace`, `pico_noqs`,
`teensy41`, `due`), then the port for uploads. Per-example tasks exist too:

```sh
# RP2040 (Raspberry Pi Pico)
pixi run build-blinky-bsp-rp2040           pixi run upload-blinky-bsp-rp2040 /dev/ttyACM0
pixi run build-dpp-bsp-rp2040              pixi run upload-dpp-bsp-rp2040 /dev/ttyACM0
pixi run build-port-validation-rp2040      pixi run upload-port-validation-rp2040 /dev/ttyACM0
pixi run validate-rp2040 /dev/ttyACM0      # host-driven QS-RX procedures
pixi run stress-rp2040 /dev/serial/by-id/... 3600   # an hour of trace-host cycling
# Teensy 4.1
pixi run build-blinky-bsp-teensy4          pixi run upload-blinky-bsp-teensy4 /dev/ttyACM0
pixi run build-dpp-bsp-teensy4             pixi run build-port-validation-teensy4
# Arduino Due (best effort)
pixi run build-blinky-sam   pixi run build-blinky2-sam   pixi run build-blinky-bsp-sam   pixi run build-dpp-bsp-sam
# checks
pixi run provenance    # imported files against qpcpp v7.3.4 (QPCPP_DIR / QPCPP_TAG)
pixi run build-teensy  pixi run build-rp2040  pixi run build-sam
pixi run check         # provenance plus the Teensy and RP2040 sets
pixi task list
```

Ports look like `/dev/ttyACM0` on Linux, `/dev/cu.usbmodemXXXX` on macOS and
`COM3` on Windows. PlatformIO keeps its platforms and toolchains inside the
repository in `.pio-core` (set `PLATFORMIO_CORE_DIR` to share one) and each
example builds in its own directory under `.pio/build-examples`. Use one
PlatformIO installation per checkout; alternating installations invalidates
the build directories.

The RP2040 environments pin the community PlatformIO platform for the
arduino-pico core to the commit the port was qualified with (platform 1.14.0,
arduino-pico 4.2.0, GCC 14.2.0). Upload uses `picotool` after the 1200 baud
touch; a board not yet running firmware that honours the touch is put into
BOOTSEL by hand.

## PlatformIO without Pixi

Any PlatformIO installation works, for example a Python virtual environment
with `pip install platformio`. With `pio` on your `PATH`:

```sh
python tools/pio_task.py build  --example examples/blinky_bsp-rp2040 --env pico
python tools/pio_task.py upload --example examples/blinky_bsp-rp2040 --env pico --port /dev/ttyACM0
python tools/pio_task.py monitor --port /dev/ttyACM0
```

Plain `pio run -e pico` also works if you set `PLATFORMIO_SRC_DIR` to the
example directory; `platformio.ini` deliberately sets no `src_dir`. To use the
library from your own PlatformIO project, pin the release tag:

```ini
lib_deps =
    https://github.com/QuantumLeaps/qp-arduino.git#v7.3.4
```

## QM and QTools (QM 6.1.1 / QTools 7.3.4)

Only specific versions of QM and QTools match the qpcpp version used here:

- QM `6.1.1` (binaries are published for Linux and Windows only)
- QTools `7.3.4` (QSPY, QUTest, QView). This version has no binary release, so
  it is cloned from its git tag and `qspy` is built from source with `make` and
  a C compiler, which the Pixi environment provides. The `qspy` built from that
  tag reports version 7.3.3, as generated upstream.

The Pixi tasks install both into `./.tools/quantum-leaps/` (override with
`QL_TOOLS_DIR`):

```sh
pixi run ql-install                     # QM and QTools
pixi run qm                             # launch QM (installs first if needed)
pixi run qtools-install                 # QTools only
pixi run qspy -c /dev/ttyACM0 -b 115200 # arguments are passed through
pixi run qspy -- -h                     # qspy's own help
```

`qmc`, QM's console code generator, is in the same install and regenerates an
example from its model, for example
`.tools/quantum-leaps/qm-6.1.1/bin/qmc examples/dpp_bsp-rp2040/dpp_bsp-rp2040.qm`.

# QP/Spy Software Tracing

The library supports
[QP/Spy software tracing](https://www.state-machine.com/software-tracing), and
the BSP-based examples demonstrate it. The output is binary; watch it with the
[QSPY host utility](https://www.state-machine.com/qtools/qspy.html). On the
RP2040 examples, which trace over USB CDC by default, start QSPY within five
seconds of the upload to receive the dictionaries; the BSPs wait that long for
a host before starting QS (`QP_ARDUINO_QS_HOST_WAIT_MS`) because USB CDC
discards output written before a host opens the port.

With QSPY running, the same link serves
[QView visualization and monitoring](https://www.state-machine.com/qtools/qview.html);
the `dpp_bsp-*` examples include QView scripts under `qview/`.

Three further profiles exist for observing a running system rather than
debugging it on the bench. `pico_uart` sends the trace out UART0 (GP0/GP1) for
a USB-UART adapter or a logic analyzer's UART decoder. `pico_txonly` is the
transmit-only observation profile on that UART: QS-RX is never read, so
nothing on the trace link can reset, command or poke the target; the
dictionaries are re-emitted periodically so a capture that misses boot is
still decodable; and a `TRACE_STAT` record reports the QS buffer's fill and
overwrite count, so trace loss is reported from the target. `pico_pio_trace`
carries the same profile on a PIO UART driving any free GPIO, for a board
whose hardware UARTs are all spoken for. The RP2040 BSPs drive the QP tick
from a hardware timer alarm they claim themselves and touch the USB CDC port
only when trace bytes are pending or once per millisecond;
[`docs/port-notes.md`](docs/port-notes.md) explains why.

# QP Licensing

The QP/C++ framework for Arduino is licensed under the modified GPLv3 with the
[QP-Arduino Exception](https://www.state-machine.com/licenses/QP-Arduino_GPL_Exception.txt)
(see [`QP-Arduino_GPL_Exception.txt`](QP-Arduino_GPL_Exception.txt)), which
removes the requirement to expose your proprietary source code as long as you
deploy it on an Arduino-Certified board.

[`LICENSE`](LICENSE) is byte-identical to qpcpp `v7.3.4:LICENSES/GPL-3.0-or-later.txt`
and the exception text to `v7.3.4:LICENSES/QP-Arduino_GPL_Exception.txt`; the
[`LICENSES/`](LICENSES) directory mirrors the upstream license texts, including
the separate Quantum Leaps commercial license terms, which remain an
alternative obtained from Quantum Leaps. The imported framework source keeps
upstream's dual-license SPDX notices.

The [QM model-based design tool](https://www.state-machine.com/qm/) is freeware
under a simple
[End-User License Agreement](https://www.state-machine.com/qm/license.html).

# qp-arduino Documentation

- [`CHANGELOG.md`](CHANGELOG.md): what changed in each release, what is
  supported, what is known.
- [`docs/port-notes.md`](docs/port-notes.md): how the Arduino port works, what
  a board support package must provide, the RP2040 lessons, and the measured
  figures behind the support claim.
- [`docs/migration-6-to-7.md`](docs/migration-6-to-7.md): moving a sketch or
  QM model from the 6.9.x line to 7.3.4.
- [`docs/provenance.md`](docs/provenance.md): how the framework files relate
  to the upstream qpcpp tag, the documented Arduino deltas, and the check that
  verifies them.
- Each example directory has a `README.md`.
- Upstream: <https://QuantumLeaps.github.io/qp-arduino/>,
  [QP/C++ documentation](https://www.state-machine.com/qpcpp),
  [QTools](https://www.state-machine.com/qtools).

# How to Get Help?

Please submit questions to the free QP support forum at
<https://sourceforge.net/p/qpc/discussion/668726> or open an issue on this
repository.
