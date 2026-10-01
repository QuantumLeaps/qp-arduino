![QP Framework](https://www.state-machine.com/img/qp_banner.jpg)

# qp-arduino

[QP/C++](https://www.state-machine.com/qpcpp) real-time embedded framework for
Arduino boards based on ARM Cortex-M, packaged as one Arduino library with the
cooperative QV kernel and QP/Spy software tracing built in.

Repository maintainer: Peter Polidoro. Framework author: Quantum Leaps.
Companion website: <https://www.state-machine.com/arduino>.

# What's New?

| Item | Value |
|---|---|
| Package version | 7.3.4 |
| qpcpp source tag | `v7.3.4` (`10441f7a03fa956b9e4772a073a05a96487983e2`) |
| QM version | 6.1.1 |
| QTools version | 7.3.4 |
| Status | unreleased; RP2040 qualification open |

This is the qpcpp v7 migration of qp-arduino. It uses the strict v7 API
(`QP_API_VERSION 9999`), keeps the imported framework files byte-identical to
the upstream tag except for documented Arduino deltas, and targets RP2040 first
on a recorded qualification fixture. Port provenance, intentional deltas,
qualification gates and every measured result live in
[`UPGRADE_QPCPP_V7.md`](UPGRADE_QPCPP_V7.md); the
[observability development plan](docs/observability-plan.md) tracks the
related trace work. None of this establishes new target support or a release.

> **NOTE:** The qpcpp `v7.3.4` tag contains generated framework headers that
> still report `QP_VERSION_STR "7.3.3"` and `QP_VERSION 733U`. This package
> preserves those upstream files unchanged and identifies the imported source
> by the `v7.3.4` tag.

# Getting Started with qp-arduino

The library works with the [Arduino IDE](https://www.arduino.cc/en/software)
or with [PlatformIO](https://platformio.org/install/cli). The intended workflow
is to edit the models (`.qm` files) with the
[QM modeling tool](https://www.state-machine.com/products/qm/), generate code,
and use the Arduino IDE or PlatformIO only to build and upload.

## Provided library and examples

One library for ARM Cortex-M based Arduinos: `qp-arduino`, the QP/C++
framework with the QV kernel (`#include <qpcpp.hpp>`).

Examples under `examples/`:

| Example | Board | Notes |
|---|---|---|
| `blinky_bsp-teensy4`, `dpp_bsp-teensy4`, `port_validation-teensy4` | Teensy 4.1 | retained regression set; hardware checks deferred |
| `blinky_bsp-rp2040`, `dpp_bsp-rp2040`, `port_validation-rp2040` | RP2040 (Earle Philhower `arduino-pico` core) | built for the recorded qualification fixture, a spare `mouse-joystick-pcb` rev 0 board watched by a Kingst LA5032 |
| `blinky-sam`, `blinky2-sam`, `blinky_bsp-sam`, `dpp_bsp-sam` | Arduino Due (ATSAM3X8E) | best effort, not a support claim |

Each `*_bsp-*` example keeps board specifics in `bsp.cpp` so the active
objects stay portable; the `dpp_bsp-*` and Teensy examples are generated from
their `.qm` models.

`library.properties` says `architectures=rp2040`, the architecture string of
the arduino-pico core, because RP2040 is the only configuration with target
evidence. The Teensy 4.1 examples still build; on a Teensy the Arduino IDE
prints its "may be incompatible" warning, which is expected. `avr` will be
added back as a recorded delta when the deferred Teensy hardware gates pass,
and will then mean Teensy 4.x, since Teensyduino installs under Arduino's
`avr` namespace. Classic 8-bit AVR boards are not supported: the port
compiles only for ARM Cortex-M. The library version keeps the imported
QP/C++ `MAJOR.MINOR` and advances the patch digit with every qp-arduino
release; the exact qpcpp tag behind each release is recorded in
`UPGRADE_QPCPP_V7.md` ("Package Metadata Decision — 2026-10-01").

## Arduino IDE

Install the library with the Library Manager (search for `qp-arduino`) or put
this repository in your `<Sketchbook>/libraries/` directory as `qp-arduino`,
from the
[zip archive](https://github.com/QuantumLeaps/qp-arduino/archive/refs/heads/main.zip)
or with `git clone https://github.com/QuantumLeaps/qp-arduino.git`. The
examples then appear under *File | Examples | Examples from Custom Libraries*.

Set *Use external editor* in the IDE preferences so the IDE reloads code that
QM regenerates.

> **NOTE:** QS tracing (`QS_ON`) is enabled in all BSP-based examples because
> the Arduino port always compiles with `Q_SPY`. The trace output is binary;
> read it with QSPY from QTools, not a serial monitor.

## PlatformIO via Pixi (recommended)

[Pixi](https://pixi.sh) gives Linux, macOS and Windows the same PlatformIO
installation from `pixi.toml` and `pixi.lock`, plus pinned QM and QTools.
Every task is a thin wrapper around `pio` or a Python script in `tools/`, so
nothing requires Pixi; see the next section for plain PlatformIO.

```sh
pixi install                        # PlatformIO, Python, git, build tools
pixi run ports                      # list serial ports
pixi run build                      # default: examples/dpp_bsp-teensy4 on teensy41
pixi run upload                     # default port /dev/ttyACM0
pixi run build  examples/blinky_bsp-rp2040 rp2040_mouse_joystick
pixi run upload examples/blinky_bsp-rp2040 rp2040_mouse_joystick /dev/ttyACM0
pixi run monitor /dev/ttyACM0
```

Task arguments are positional: example directory, PlatformIO environment
(`teensy41`, `rp2040_mouse_joystick`, `due`), then the port for uploads.
Per-example tasks exist too:

```sh
# Teensy 4.1
pixi run build-blinky-bsp-teensy4          pixi run upload-blinky-bsp-teensy4 /dev/ttyACM0
pixi run build-dpp-bsp-teensy4             pixi run build-port-validation-teensy4
# RP2040 qualification fixture
pixi run build-blinky-bsp-rp2040           pixi run upload-blinky-bsp-rp2040 /dev/ttyACM0
pixi run build-dpp-bsp-rp2040              pixi run upload-dpp-bsp-rp2040 /dev/ttyACM0
pixi run build-port-validation-rp2040      pixi run upload-port-validation-rp2040 /dev/ttyACM0
pixi run validate-rp2040 /dev/ttyACM0      # host-driven QS-RX procedures
pixi run stress-rp2040 /dev/serial/by-id/usb-Orb_Weaver_qp-arduino-rp2040_XXXX-if00 3600   # hour-long reader-cycling stress
# Arduino Due (best effort)
pixi run build-blinky-sam   pixi run build-blinky2-sam   pixi run build-blinky-bsp-sam   pixi run build-dpp-bsp-sam
# checks
pixi run provenance    # imported files against qpcpp v7.3.4 in ../qpcpp (QPCPP_DIR/QPCPP_TAG)
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

## PlatformIO without Pixi

Any PlatformIO installation works, for example a Python virtual environment
with `pip install platformio`. With `pio` on your `PATH`:

```sh
python tools/pio_task.py build  --example examples/blinky_bsp-teensy4 --env teensy41
python tools/pio_task.py upload --example examples/blinky_bsp-rp2040 --env rp2040_mouse_joystick --port /dev/ttyACM0
python tools/pio_task.py monitor --port /dev/ttyACM0
```

Plain `pio run -e teensy41` also works if you set `PLATFORMIO_SRC_DIR` to the
example directory; `platformio.ini` deliberately sets no `src_dir`. To use the
library from your own PlatformIO project:

```ini
lib_deps =
    https://github.com/QuantumLeaps/qp-arduino.git
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
example from its model: `.tools/quantum-leaps/qm-6.1.1/bin/qmc.sh examples/dpp_bsp-rp2040/dpp_bsp-rp2040.qm`.

# QP/Spy Software Tracing

The library supports
[QP/Spy software tracing](https://www.state-machine.com/software-tracing), and
the BSP-based examples demonstrate it. The Arduino port is always compiled with
`Q_SPY` because an Arduino build cannot switch it per sketch, so every BSP must
provide the QS callbacks. `QS_ON` is enabled in the BSP-based examples: the
Teensy and SAM BSPs define it at the top of `bsp.cpp`, and the RP2040
environments pass `-DQS_ON`.

The output is binary; watch it with the
[QSPY host utility](https://www.state-machine.com/qtools/qspy.html). On the
RP2040 examples, which trace over USB CDC, start QSPY within five seconds of
the upload to receive the dictionaries; the BSPs wait that long for a host
before starting QS (`QP_ARDUINO_QS_HOST_WAIT_MS`) because USB CDC discards
output written before a host opens the port.

With QSPY running, the same link serves
[QView visualization and monitoring](https://www.state-machine.com/qtools/qview.html);
the `dpp_bsp-*` examples include QView scripts under `qview/`.

The `rp2040_mouse_joystick_uart` environment sends the trace out UART0
(GP0/GP1) instead, which the fixture exposes to the analyzer on channels 8 and
7, so a capture can carry the trace alongside the probe signals independently
of USB. `rp2040_mouse_joystick_txonly` is the transmit-only observation
profile on that UART: QS-RX is never read, the dictionaries are re-emitted
periodically so a capture that misses boot is still decodable, and a
`TRACE_STAT` record reports the QS buffer's fill and overwrite count.
`rp2040_mouse_joystick_noqs` compiles the trace out for baseline timing, and
`rp2040_mouse_joystick_fastflash` uses the quad-lane boot2.

The RP2040 BSPs drive the QP tick from a hardware timer alarm they claim
themselves, not from the core's shared alarm pool, and watch it from the idle
loop (`TICK_LOST` record). They touch the USB CDC port only when trace bytes
are pending or once per millisecond (`QP_ARDUINO_QS_SERVICE_BYTES`,
`QP_ARDUINO_QS_SERVICE_US`).

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

- [`UPGRADE_QPCPP_V7.md`](UPGRADE_QPCPP_V7.md): port provenance, intentional
  deltas, fixture and toolchain record, qualification gates and results.
- [`docs/observability-plan.md`](docs/observability-plan.md): the trace and
  observability work that uses this port.
- [`docs/evidence/`](docs/evidence): raw captures, trace logs and analysis for
  the recorded target runs.
- Each example directory has a `README.md`.
- Upstream: <https://QuantumLeaps.github.io/qp-arduino/>,
  [QP/C++ documentation](https://www.state-machine.com/qpcpp),
  [QTools](https://www.state-machine.com/qtools).

# How to Get Help?

Please submit questions to the free QP support forum at
<https://sourceforge.net/p/qpc/discussion/668726> or open an issue on this
repository.
