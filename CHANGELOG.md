# Changelog

Versions follow the policy below: `MAJOR.MINOR` is the imported QP/C++
version's, and `PATCH` advances by one for every qp-arduino release on that
line, whether the change is a port fix or an upstream patch import. The exact
upstream tag behind each release is named here, never derived from the
version number. Release tags are `v` plus the library version.

## 7.3.4 — 2026-10-04

The QP/C++ v7 line of qp-arduino, built from upstream qpcpp tag `v7.3.4`
(`10441f7a03fa956b9e4772a073a05a96487983e2`). The previous indexed release,
6.9.6, was built from qpcpp 6.9.3.

### Framework

- Framework files imported flat from qpcpp `v7.3.4`: public and internal
  headers, QF/QEP, QS, QV, and the ARM Cortex-M QV GNU port. They are
  byte-identical to upstream except for the deltas listed in
  [`docs/provenance.md`](docs/provenance.md), which `pixi run provenance`
  verifies against the tag.
- Strict v7 API by default (`QP_API_VERSION 9999`): no v6 compatibility shims.
  [`docs/migration-6-to-7.md`](docs/migration-6-to-7.md) lists what a 6.9.x
  sketch or model has to change.
- Only the QV cooperative kernel is included. QK, QXK and the QUTest runtime
  are not part of the Arduino library build.
- `Q_SPY` is always compiled in, as before; every BSP provides the QS
  callbacks. Tracing is on by default in every example, because the Arduino
  IDE offers no build flags; the RP2040 BSPs compile it out when
  `QP_ARDUINO_QS_OFF` is defined.
- The upstream `v7.3.4` tag ships generated headers that still say
  `QP_VERSION_STR "7.3.3"` and `QP_VERSION 733U`; they are kept unchanged.

### Qualified configuration: RP2040

RP2040 with the Earle Philhower arduino-pico core 4.2.0 and GCC 14.2.0, QV
kernel, core 0 only. Measured on an RP2040 board at 133 MHz with a logic
analyzer on the tick and LED pins and QSPY on USB CDC:

- 100 Hz tick within ±1 % with no missed or doubled tick: 10 minutes of
  direct edge capture, a 2 h 15 min soak (720,001 ticks, mean 9,999.995 µs,
  min 9,980, max 10,020), and 394 trace-host attach/detach plus 78
  slow-reader cycles with the tick unaffected.
- Interrupt-to-active-object dispatch latency under the port-validation load:
  p99 121 µs, maximum 133 µs with QSPY attached over USB CDC; maximum 66 µs
  detached. The tick callback itself takes about 4 µs.
- Host-driven coverage over QS-RX, 8 of 8 procedures: queues, publish and
  subscribe, dynamic events, time events, dispatch, idle, QS TX and RX, and
  the assertion path through a watchdog reset.
- Two-hour assertion-free soak with the Dining Philosophers example.
- Bounded behaviour with the trace reader absent, slow or disconnected:
  records are dropped, never blocked on, and the application timing is
  unchanged.
- A transmit-only observation profile that accepts no input from the trace
  link, over UART0 and over a PIO UART on a marker pin, with periodic
  dictionaries and a `TRACE_STAT` record that reports trace loss from the
  target.

Sizes at release, built for the Raspberry Pi Pico (PlatformIO
figures; the Pico has 2 MiB of flash and 256 KiB of RAM): Blinky 77,584 B
flash / 12,024 B RAM; DPP 80,324 / 12,588; port validation 78,952 / 14,372.
An application-shaped load on the same port, a line protocol inside one
active object with a 1 kHz tick, ran at about 125 kB of flash and 35 kB of
RAM.

### Examples and board support

- New RP2040 examples: `blinky_bsp-rp2040`, `dpp_bsp-rp2040` (both generated
  from QM 6.1.1 models) and the handwritten `port_validation-rp2040` load
  generator with its host-side procedure and stress scripts. They target the
  Raspberry Pi Pico by default and adapt to any board of the core with
  `QP_ARDUINO_LED_PIN` and `QP_ARDUINO_TICK_PROBE_PIN`.
- The RP2040 BSPs claim a hardware timer alarm for the QP tick instead of a
  repeating timer in the core's shared alarm pool, which stopped silently
  under trace-host cycling; the idle loop watches the tick and reports a
  `TICK_LOST` record if it ever has to re-arm it.
- The QS transport is serviced from the idle loop only when bytes are pending
  or once per millisecond, takes at most the port's free space per pass, and
  takes QS blocks with interrupts disabled. Periodic dictionary re-emission no
  longer flushes synchronously. [`docs/port-notes.md`](docs/port-notes.md)
  records each of these with the defect that motivated it.
- Teensy 4.1 examples (`blinky_bsp-teensy4`, `dpp_bsp-teensy4`,
  `port_validation-teensy4`) are retained as a build regression set on the
  pinned Teensy platform 5.1.0; the SAM examples build for the Due as best
  effort. Tracing is enabled in every BSP example.

### Tooling

- A Pixi environment (`pixi.toml`, `pixi.lock`) gives Linux, macOS and
  Windows the same PlatformIO installation, with tasks for every example,
  the regression sets, the provenance check, and pinned QM 6.1.1 and QTools
  7.3.4 installers (`tools/quantum_leaps_tools.py`). Nothing requires Pixi.
- `tools/pio_task.py` selects an example through `PLATFORMIO_SRC_DIR` and
  gives each one its own build directory.
- `tools/check_upstream_provenance.py` compares the imported files and the
  license texts with the upstream tag.
- `src/qp-arduino/library.json` is a PlatformIO-only manifest for the
  `symlink://` dependency the repository's own builds use.

### Package metadata

- `architectures=rp2040`, the arduino-pico core's architecture string. The
  port compiles only for ARM Cortex-M, so the previous `*` and `avr` values
  promised builds that fail. The Arduino IDE warns on other boards; the
  Teensy and Due examples still compile.
- `LICENSE`, `QP-Arduino_GPL_Exception.txt` and `LICENSES/` are byte-identical
  to the upstream tag's texts.

### Known issues

- `QActive::getQueueMin()` is declared in upstream `qp.hpp` at `v7.3.4` but
  defined nowhere in the sources, so `QF::getQueueMin()` is a link error.
  Read an active object's queue minimum through its queue's `getNMin()`.
- This port's critical sections do not nest. A QF query that takes its own
  critical section (`QF::getPoolMin()`, for example) must not be called inside
  `QS_BEGIN`/`QS_END`; the port asserts.
- On the Due, GCC 7.2.1 warns about `-Wnoexcept-type` in the unchanged
  upstream `qs.hpp`.

### Not claimed

- On-target QUTest: the Arduino library always compiles `qv.cpp` and the QV
  port header, which a QUTest build would replace. The host-driven
  procedures cover the same behaviours against the real kernel, and the
  upstream host QUTest suites pass at the framework level (11 suites, 41
  tests).
- Teensy 4.1 and Arduino Due on hardware: builds only.
- Core 1 or multicore use, QK and QXK, QP 8.x, QP/C.

### Library Manager history

The tags `v6.9.7`, `v6.9.8` and `v6.9.9` exist, but `library.properties` at
each still read `6.9.6`, so the Arduino Library Manager indexed none of them.
For Library Manager users 7.3.4 follows 6.9.6 directly. Before tagging a
release, check that `library.properties` and `src/qp-arduino/library.json`
carry the version named by the tag.

## 6.9.6

qpcpp 6.9.3 repackaged for Arduino with the QV kernel: `architectures=*`,
examples for the Teensy 4 and the Arduino Due. The last release indexed by
the Arduino Library Manager before 7.3.4.
