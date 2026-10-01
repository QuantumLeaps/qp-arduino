# qp-arduino qpcpp v7.3.4 Upgrade Plan

## Summary

- Upgrade `qp-arduino` from qpcpp v6-derived files to qpcpp tag `v7.3.4` (`10441f7a03fa956b9e4772a073a05a96487983e2`).
- Use strict v7 API by default: `QP_API_VERSION` defaults to `9999`; do not add v6 public-header compatibility shims.
- Preserve upstream generated framework metadata exactly, including the known `QP_VERSION_STR "7.3.3"` value in qpcpp `v7.3.4`; document this mismatch instead of locally patching generated files.
- As decided 2026-09-30, RP2040 with the Earle Philhower Arduino core family is the primary qualification target. The fixture and the exact platform/core/toolchain versions were selected and recorded on 2026-09-30 (see "RP2040 Fixture And Toolchain Record"); the remaining RP2040 software and all hardware gates stay open.
- Preserve the completed Teensy software baseline and future build regressions. Teensy hardware gates are deferred and nonblocking for RP2040; STM32G0B1CEU6 is the later second target. SAM examples are best-effort, not a support claim.

## Observability Workstream

The [observability development plan](docs/observability-plan.md) tracks
`OBS-QP`: qualify QP `v7.3.4`/QV on a simple non-actuating RP2040 fixture, then
measure a representative trace pilot. The fixture is separate from the working
joystick and can use qualified bench connections without waiting for a new
Digital-50 board, debug connector or external probe product. This document
remains the sole port acceptance and provenance ledger. BSP, tick and trace
transport remain in examples/applications; QP adoption in the delivered
application remains optional. No build or hardware result is added by this
2026-09-30 planning revision.

## Port Provenance and Local Deltas

The v7 port was created from the local `../qpcpp` repository at tag
`v7.3.4` (`10441f7a03fa956b9e4772a073a05a96487983e2`). The Arduino package
keeps qpcpp's framework files flat under `src/qp-arduino` so Arduino builders
can compile the library without the upstream qpcpp directory layout.

Direct upstream imports are verified by `scripts/check_upstream_provenance.sh`.
They include:

- public/internal headers from qpcpp `include/`;
- QF/QEP sources from qpcpp `src/qf/`;
- QS sources from qpcpp `src/qs/`;
- QV source from qpcpp `src/qv/`;
- ARM Cortex-M QV GNU port files from qpcpp `ports/arm-cm/qv/gnu/`.

Intentional Arduino-local deltas are:

- `src/qpcpp.hpp`: keeps the existing Arduino public include name, rewrites
  includes to local `src/qp-arduino` paths, and defaults `QP_API_VERSION` to
  `9999` for strict v7 API use.
- `src/qp-arduino/qp_port.hpp`: includes `Arduino.h`, forces `Q_SPY`, keeps
  low-RAM Arduino defaults for `QF_MAX_ACTIVE`/`QF_MAX_TICK_RATE`, and keeps
  the QV/Cortex-M `QF_BASEPRI` setting.
- `src/qp-arduino/qs_port.hpp`: differs from upstream only by removing one
  trailing blank line at EOF so `git diff --check` passes.
- `src/qp-arduino/qv_port.cpp`: wraps the upstream `SCB_CPACR` and
  `FPU_FPCCR` register aliases in `#ifndef` guards so Arduino/Teensy core
  definitions are reused instead of producing macro redefinition warnings; it
  also removes one trailing blank line at EOF.
- `platformio.ini`: keeps `src/qp-arduino` as an explicit local dependency so
  PlatformIO links the QP library with `lib_ldf_mode = off`. Since 2026-09-30
  it is referenced as `symlink://src/qp-arduino`, so PlatformIO links the
  source tree instead of copying it into `.pio/libdeps` (a bare path is copied
  once and never refreshed). PlatformIO requires a manifest for a linked
  library, so `src/qp-arduino/library.json` was added: a PlatformIO-only
  manifest that mirrors `library.properties` and is not an upstream file. The
  same date added the pinned `rp2040_mouse_joystick` environment described in
  the fixture record below.
- `scripts/check_upstream_provenance.sh` is, since 2026-09-30, a thin wrapper
  around `tools/check_upstream_provenance.py`, the single implementation of the
  check, so it runs identically on Linux, macOS and Windows (`pixi run
  provenance`). Its checks, exit codes and `QPCPP_DIR`/`QPCPP_TAG` overrides
  are unchanged.
- `pixi.toml`, `pixi.lock` and `tools/pio_task.py` (added 2026-09-30) give
  every platform a reproducible PlatformIO environment and example-selection
  wrapper; the plain Python virtual environment documented in the README keeps
  working without Pixi.
- `scripts/build_teensy_examples.sh`: removes PlatformIO's generated
  `.pio/libdeps/teensy41/qp-arduino` cache before each verification build so
  tests compile the current source tree instead of stale copied sources.
- `library.properties`: uses `architectures=avr`, the narrowest Arduino Library
  Manager architecture selector that includes Teensy 4.1 because Teensyduino
  installs under Arduino's `avr` namespace. The field cannot express a single
  validated board and is not a classic-AVR or all-Teensy support claim.
  (Superseded 2026-10-01 by `architectures=rp2040`; see "Package Metadata
  Decision — 2026-10-01".)
- Examples and `.qm` snippets are migrated from v6 APIs to v7 APIs without
  claiming QM regeneration unless the model is actually regenerated later.

## Original v7 Migration Changes

- Replace the production framework payload under `src/qp-arduino` with the v7 QV/Cortex-M set: public/internal headers, QF/QEP/QS/QV sources, `qstamp.cpp`, `qs_64bit.cpp`, and ARM-CM QV GNU port files.
- Exclude QK, QXK, and QUTest runtime files from the normal Arduino library build so normal sketches do not compile unused kernels or test-only code.
- Replace the old split port headers with v7-style `qp_port.hpp`; remove obsolete v6 headers such as `qep.hpp`, `qf.hpp`, `qpset.hpp`, `qassert.h`, `qep_port.hpp`, `qf_port.hpp`, and `qv_port.hpp`.
- Keep `qpcpp.hpp` as the public Arduino include, but rewrite it for local include paths and strict v7 defaults.
- Preserve current Arduino port behavior where appropriate:
  - Include `Arduino.h` from the port layer.
  - Force `Q_SPY` in the port, matching current qp-arduino behavior.
  - Keep low-RAM defaults: `QF_MAX_ACTIVE` 8 on ARMv6-M, 16 otherwise; `QF_MAX_TICK_RATE` 2.
  - Keep `QF_BASEPRI` at upstream/current value `0x3F`.
- Update examples to real v7 API usage:
  - `Q_onAssert(...)` becomes `Q_onError(...)`.
  - Static events use `QEvt(SIG)` constructor form.
  - Timer ticks use `QTimeEvt::tick(...)`.
  - Pub/sub initialization uses `QActive::psInit(...)`.
  - Timer/ISR publishes use `QActive::publish_(event, sender, qsId)` with an explicit QS ID instead of deprecated `QF::PUBLISH`.
- Update docs and metadata:
  - `library.properties` version becomes `7.3.4`.
  - README names qpcpp source tag `v7.3.4`, qtools `v7.3.4`, and QM `6.1.1`.
  - README documents that the upstream v7.3.4 framework headers still report `QP_VERSION_STR "7.3.3"`.

## Original Verification Work — Teensy Software Baseline

- Add a provenance check script that compares imported upstream files against qpcpp `v7.3.4`, allowing only intentional local deltas in `qpcpp.hpp`, `qp_port.hpp`, and Arduino-specific docs/metadata.
- Add a Teensy build helper that builds at least:
  - `examples/blinky_bsp-teensy4`
  - `examples/dpp_bsp-teensy4`
  - the new port-validation example
- Use PlatformIO for Teensy builds; if `pio` is not on `PATH`, create/use the README-documented virtual environment.
- Build local qtools as needed:
  - `make -C ../qtools/qspy/posix`
  - use `../qtools/qutest/qutest.py` for QUTest runs.
- Run upstream qpcpp v7 QUTest examples on the host as a framework/toolchain baseline before blaming qp-arduino port code.
- Add a Teensy 4.1 port-validation example with QUTest/QSPY coverage for event queues, pub/sub, dynamic events, time events, active object dispatch, idle behavior, assertions, QS TX, and QS RX.
- Add optional `QP_ARDUINO_TEST_PROBES` timing probes using Teensy digital pins 2-6 for timer ISR, dispatch, idle, event receipt, and validation markers.

## Software Verification Results

On 2026-07-14, the software-only migration gates were run from the repository
root on branch `upgrade/qpcpp-v7`:

- `scripts/check_upstream_provenance.sh` passed against qpcpp tag `v7.3.4`
  (`10441f7a03fa956b9e4772a073a05a96487983e2`).
- PlatformIO was absent, so a repository-local `.venv` was created and
  PlatformIO Core 6.1.19 was installed. `scripts/build_teensy_examples.sh`
  passed with PlatformIO Teensy 5.2.0, Arduino Teensy framework 1.162.0,
  `toolchain-gccarmnoneeabi-teensy` 1.150201.0, and TimerOne commit `953f240`:
  - `examples/blinky_bsp-teensy4`: success; 16,256 bytes code, 4,040 bytes
    initialized data, and 5,152 bytes RAM1 variables.
  - `examples/dpp_bsp-teensy4`: success; 26,084 bytes code, 4,040 bytes
    initialized data, and 6,976 bytes RAM1 variables.
  - `examples/port_validation-teensy4`: success; 24,656 bytes code, 4,040 bytes
    initialized data, and 7,744 bytes RAM1 variables.
- These are cross-compilation results only. No firmware was uploaded and no
  Teensy target handshake, timing, or soak result is implied.
- The upstream host QUTest baseline was isolated under `/tmp` so the existing
  qpcpp and qtools source repositories were not changed. It used qpcpp
  `v7.3.4`, qpcpp-examples commit
  `a4becf08ed700750fce0bfea22a8968349330e55` pinned by that tag, qtools
  `v7.3.4` (`2303bdea30051fb915341c1574d45ece5f9f4b93`), QUTest 7.3.4,
  QSPY 7.3.3 as generated by that qtools tag, Python 3.13.5, and GCC 16.1.0.
  QSPY/QUTest connected over localhost UDP 7701 and TCP 6601. All 11 regular
  host suites passed: 41 tests in 19 groups. The separate QUTest self-test ran
  15 tests; 13 passed and tests 12 and 14 produced the two failures explicitly
  required by `test_assert.py` to verify failure reporting.

The package-level evidence was also reconciled:

- `architectures=*` was narrowed to `architectures=avr`. Teensyduino's Arduino
  package path is `hardware/teensy/avr`; Arduino's architecture field has no
  board-level value for Teensy 4.1. README and this plan therefore state the
  exact validation scope and the selector's limitation.
- `LICENSE` now exactly matches
  `qpcpp v7.3.4:LICENSES/GPL-3.0-or-later.txt`. The previous file differed only
  at four outdated `http` URLs.
- `QP-Arduino_GPL_Exception.txt` now exactly matches
  `qpcpp v7.3.4:LICENSES/QP-Arduino_GPL_Exception.txt`. The previous local file
  removed QP-nano references and changed two URLs and the final copyleft
  wording. Because the exception prohibits changing its text, the authoritative
  upstream tagged text is packaged verbatim even though this package contains
  QP/C++ only.
- qpcpp's imported source retains its upstream
  `GPL-3.0-or-later OR LicenseRef-QL-commercial` notices. This package includes
  the exact upstream GPL text and Arduino GPL exception; the separate Quantum
  Leaps commercial-license alternative remains governed by its own terms.

The final post-change software checks passed with these commands:

```sh
scripts/check_upstream_provenance.sh
scripts/build_teensy_examples.sh
git diff --check
```

## Current Qualification Decision — 2026-09-30

RP2040 is the primary qualification target, using the Earle Philhower Arduino
core family already used by `mouse-joystick-firm`. This is a development target,
not a support claim. Select and record the exact simple RP2040 fixture, core,
PlatformIO platform/toolchain versions, clock settings and pin map before
building the qualification baseline. Do not use the working joystick as the
initial port fixture or require a new Digital-50 board before port work starts.
**Done 2026-09-30**: see the fixture and toolchain record below. The fixture is
a spare rev 0 board, not either of the two boards driving the rig.

The original v7 framework import and Teensy software baseline above are
completed historical work. RP2040 introduces new **open software and hardware
gates** below. Teensy examples and software regression coverage remain; its
unperformed hardware gates are explicitly deferred and **do not block RP2040
qualification**. Neither target inherits validation from the other.

STM32G0B1CEU6 is the later second qualification target, with its own Arduino
core/BSP/build/target plan still to be selected. RP2040 and STM32G0B1CEU6 are
Cortex-M0+ devices without SWO/ITM trace. Plan QS output over a qualified UART
or other explicitly measured transport; SWD is programming/debug access, not
a replacement for QS output.

## RP2040 Fixture And Toolchain Record — 2026-09-30

The fixture was selected on 2026-09-30 from bench hardware the joystick project
had already validated. It is a spare unit, not one of the two boards driving
the rig, and it is non-actuating as long as its 36 V input stays unplugged; the
QP examples never enable the 36 V switch or the GPIO isolation bus. Board and
adapter facts below were read from the tagged design records
(`mouse-joystick-pcb` tag `v0.2`, `la5032-instrument-port-adapter` tag `v1.0`)
and from `mouse-joystick-firm/src/BoardPins.hpp`, without checking those
repositories out.

- **Board:** `mouse-joystick-pcb` rev 0, unit "Board 1", RP2040 flash unique
  ID `E4654C159F2C2534`. Rev 0 is PCB git tags `v0.1`/`v0.2`, whose RP2040
  pad-to-net maps are byte-identical. Its `R203` LATCH strap was removed on
  2026-09-02; that only affects the 36 V switch, which this fixture never
  enables. USB CDC enumerates as `2e8a:f00a`. A v0.3 board also exists; it adds
  only switched-domain DAC strands and no debug access, so it offers nothing
  extra for port work and is left with the rig.
- **Clock:** 12 MHz crystal `Y501` (±30 ppm), `clk_sys` 133 MHz, `clk_peri`,
  `clk_usb` and `clk_adc` 48 MHz, measured 2026-09-01 in that board's bring-up
  log. Crystal absolute error measured below the ±110 ppm noise floor of the
  host method used there.
- **Debug access:** none. No SWD header, no RUN connection, no LED, no test
  points. Programming is BOOTSEL (`SW501`, needs USB and pyControl unplugged
  for the button) or the 1200-baud touch through `picotool`; both verified on
  this board family in the joystick bring-up log.
- **Analyzer path:** InstrumentPort-40 `J1501` (Molex 5051104091, bottom
  contact) → 40-pin 0.5 mm FFC, confirmed not mirrored functionally on
  2026-09-01 → `la5032-instrument-port-adapter` v1.0 (only unit built) →
  Kingst LA5032 (`77a1:01a2` before firmware load). Host capture path validated
  the same day: `sigrok-cli` 0.7.2, libsigrok 0.6.0, driver `kingst-la2016`,
  from `instrument-control`'s Guix manifest, firmware in
  `~/.local/share/sigrok-firmware`.
- **Probe map** (RP2040 GPIO → board net → `J1501` pin → LA5032 channel):

  | GPIO | Net | `J1501` pin | LA5032 | Fixture role |
  |------|-----|-------------|--------|--------------|
  | GP25 | `FW_TRIG0` (also `LED_BUILTIN`) | 26 (`TRIG0`) | 16 | Blinky LED; validation marker |
  | GP21 | `FW_TRIG1` | 28 (`TRIG1`) | 17 | tick-ISR probe |
  | GP0 | `PYC_UART_TX` (UART0 TX) | 14 (`DIG8`) | 8 | optional QS TX over UART |
  | GP1 | `PYC_UART_RX` (UART0 RX) | 13 (`DIG7`) | 7 | optional QS RX over UART |

  All four bypass the GPIO isolation switch and are buffered by always-on
  `SN74LVC125A` packages that stay high-Z until GP24 `~INSTR_BUF_OE` is driven
  low. Only two RP2040 pins are free markers, so the Teensy probe roles (timer,
  dispatch, idle, event, marker on pins 2-6) cannot be mapped one-to-one; the
  RP2040 port-validation example must define how roles share GP25/GP21 or use
  the UART. The TMC SPI taps on channels 2-5 sit on the switched domain behind
  the bus switch and are not used. Odd `J1501` pins 1-31 not listed are `GND`.
- **Safe state** the BSP applies before anything else (from the joystick
  firmware's `SafeState`): GP9 `~BUS_SW_EN` high, GP19 `HV_EN` low, GP14
  `HV_DIAG_EN` low, GP2 `~TMC_CS` high, GP3 `~TMC_DRV_EN` high, markers low;
  then GP24 low to enable the instrument port. `Serial2` (GP8/GP9) and `Wire1`
  (GP26/GP27) must never be used on this board.
- **Core 1:** the Earle Philhower core launches core 1 only when `setup1` or
  `loop1` is defined; the QP examples define neither, so QP, QS and all drivers
  stay on core 0 as required.

Toolchain pinned to the set the joystick firmware qualified on this board:

- PlatformIO platform `https://github.com/janelia-arduino/platform-raspberrypi.git`
  at commit `5e4e06bb9c9971ad003a415bb32796da307b5676` (version 1.14.0, a fork
  of the maxgerhardt community platform). That platform manifest pins:
- `framework-arduinopico` 1.40200.0 = arduino-pico 4.2.0, commit
  `c4b6521849570c465e073255c983941dc6adf4fb` (bundles its pico-sdk);
- `toolchain-rp2040-earlephilhower` 5.140200.240929 = GCC 14.2.0;
- `tool-picotool-rp2040-earlephilhower` 5.140200.240929.
- Build configuration (`platformio.ini` env `rp2040_mouse_joystick`): board
  `generic`, `board_build.core = earlephilhower`, `cortex-m0plus`, `F_CPU`
  133 MHz, 16 MiB flash, no filesystem, universal
  `boot2_generic_03h_4_padded_checksum.S`, `-DQP_ARDUINO_FIXTURE_MOUSE_JOYSTICK_PCB=1`,
  `-DQS_ON`, strict v7 defaults from `qpcpp.hpp`.
- USB servicing while QV owns the run loop: this core services USB with
  `tud_task()` from a spare IRQ that a periodic alarm pends (`RP2040USB.cpp`),
  and `main()` calls `Serial.begin()` before `setup()`. `QF::run()` therefore
  never returning to `loop()` costs only the `serialEvent` hooks, which the
  examples do not use. This is from reading the core source; behaviour on the
  target remains a hardware gate.

## Active RP2040 Software Gates

- [x] Select a simple non-actuating fixture with accessible programming,
      trace and measurement pins. Record its identity, revision, actual wiring
      and clock setup. **Recorded 2026-09-30** in the fixture record above:
      `mouse-joystick-pcb` rev 0 unit `E4654C159F2C2534` with 36 V unplugged.
- [x] Adapt validation probe roles to that pin map; Teensy pins 2-6 are not an
      RP2040 allocation. **Done 2026-09-30**: GP21 is the fixed tick-ISR probe
      and GP25 takes the marker, dispatch, idle or event-receipt role selected
      at run time by QS command 5 in `examples/port_validation-rp2040`.
- [x] Pin and record the Earle Philhower core version/commit, PlatformIO
      platform, compiler, SDK dependencies and build configuration.
      **Recorded 2026-09-30** in the fixture record above and pinned in
      `platformio.ini` (`rp2040_mouse_joystick`); the Blinky build recorded
      below is the first build with that configuration.
- [ ] Audit the imported ARMv6-M QV path as the starting point. Cortex-M0+
      critical sections use PRIMASK/global interrupt masking, not BASEPRI.
      Verify architecture selection, assembly/linkage, ISR entry/exit and
      critical-section assumptions against this Arduino core. Existing
      ARMv6-M source is not evidence of an operating RP2040 port.
- [ ] Keep QP, QS and application drivers on core 0 initially. Do not launch
      application work on core 1, call QP APIs there, or share application
      drivers across cores. PRIMASK masks only the executing core; it is not
      a multicore lock. Document any core/runtime background activity and
      its implications rather than assuming the second core cannot interfere.
- [ ] Implement the fixture BSP/tick/trace callbacks with explicit timer and
      interrupt ownership. Verify Arduino timekeeping and USB/UART servicing
      while QV owns the run loop; bound trace drain/flush paths and idle work.
      Keep these details outside generated or handwritten application logic.
- [ ] Build RP2040 Blinky, DPP and port-validation examples using strict v7
      defaults. Supply concrete host-driven procedures for queues, pub/sub,
      dynamic events, time events, active-object dispatch, idle, assertions,
      QS TX and bench QS RX; do not count a planned test as implemented.
      **Met 2026-09-30**: Blinky, DPP and port-validation build from strict
      v7 defaults, and `tools/port_validation_rp2040.py` supplies the
      host-driven procedures, which pass on the fixture (see "RP2040
      Port-Validation Results").
- [ ] Run provenance checks after framework/port changes and retain the
      existing Teensy example build regressions. Re-run the recorded upstream
      host baseline when framework/test-tool changes warrant it; distinguish
      those host results from RP2040 target evidence.
- [x] Review Arduino package architecture metadata against the selected core
      and actual build evidence before publishing RP2040 support.
      **Decided 2026-10-01**: `architectures=rp2040`; `library.json`
      `platforms` narrowed to `raspberrypi`; version policy fixed. See
      "Package Metadata Decision — 2026-10-01". `avr` returns only with
      Teensy hardware evidence; `sam` and `*` are not used.

## RP2040 Software Build Results — 2026-09-30

First build with the pinned configuration, run from the repository root on
branch `upgrade/qpcpp-v7`. Cross-compilation only: no firmware was uploaded
and no target behaviour, timing or trace result is implied.

- `examples/blinky_bsp-rp2040`, env `rp2040_mouse_joystick`: **success**, no
  compiler warnings. PlatformIO reported 76,272 bytes of flash and 11,980
  bytes of RAM; `firmware.bin` is 88,304 bytes and `firmware.uf2` 176,640
  bytes. Resolved packages matched the record above exactly: platform
  `1.14.0+sha.5e4e06b`, `framework-arduinopico 1.40200.0+sha.c4b65218`,
  `toolchain-rp2040-earlephilhower 5.140200.240929` (GCC 14.2.0),
  `tool-picotool-rp2040-earlephilhower 5.140200.240929`. The ELF carries
  `Tag_CPU_arch v6S-M`, `QF_int_disable_` disassembles to `mrs r0, PRIMASK` /
  `cpsid i`, and `QF_qlog2` is linked, so the ARMv6-M/PRIMASK port path is the
  one compiled. Built identically through the repository `.venv` (PlatformIO
  Core 6.1.19) and through Pixi (`pixi run build-rp2040`, PlatformIO Core
  6.2.0 from conda-forge).
- The example's BSP applies the fixture safe state, enables the instrument
  port buffers, drives the 100 Hz tick from a pico-sdk repeating timer
  (start-to-start period, alarm IRQ on core 0), toggles GP25 at 2 Hz, pulses
  GP21 per tick and emits QS records over USB CDC with a bounded, non-blocking
  transmit path. `Q_onError` holds GP25 high for two seconds and resets
  through the watchdog because the board has no reset button or SWD.
- Retained Teensy regression (`scripts/build_teensy_examples.sh`, `.venv`
  PlatformIO 6.1.19) passed with the `symlink://` dependency and the new
  manifest: `blinky_bsp-teensy4` 16,160 bytes code, 4,040 data, 5,152 RAM1
  variables; `dpp_bsp-teensy4` 26,132 / 4,040 / 6,976; `port_validation-teensy4`
  24,704 / 4,040 / 7,744. The packages actually present in `.pio-core` were
  Teensy platform 5.1.0, `framework-arduinoteensy` 1.160.0 and
  `toolchain-gccarmnoneeabi-teensy` 1.110301.0 (GCC 11.3.1), with TimerOne at
  commit `953f240`. That differs from the platform 5.2.0 / framework 1.162.0 /
  toolchain 1.150201.0 set recorded on 2026-07-14, which explains the small
  code-size differences; the Teensy pins should be made explicit in
  `platformio.ini` before any Teensy claim is refreshed.
- `scripts/check_upstream_provenance.sh` (wrapper) and
  `tools/check_upstream_provenance.py` passed against qpcpp `v7.3.4`
  (`10441f7a03fa956b9e4772a073a05a96487983e2`), including through
  `pixi run provenance`. The Python port was checked against deliberate
  failures (wrong tag, modified import, edits beyond the documented
  `qv_port.cpp`/`qs_port.hpp` deltas, an obsolete v6 file, a missing port
  setting) before replacing the bash implementation. `../qpcpp` stayed at its
  `v6.9.3` checkout throughout.
- `git diff --check` passed.

Commands used for the post-change software checks:

```sh
pixi run provenance
pixi run build-rp2040
scripts/build_teensy_examples.sh
git diff --check
```

## Upstream Merge — 2026-09-30

The official `QuantumLeaps/qp-arduino` master (commit `98c2cd0`, 2026-02-18)
was merged into this branch. It carried five commits made after this branch's
base: a Pixi workflow, `tools/quantum_leaps_tools.py` (pinned QM/QTools
installer), `QS_ON` enabled in every example, a reworked `platformio.ini` with
no `src_dir` and a pinned TimerOne, README changes and the
`blinky_bsp-teeny4` typo fix. Conflicts in `.gitignore`, `README.org`,
`pixi.toml` and `platformio.ini` were resolved as follows:

- `platformio.ini` takes the upstream shape (no `src_dir`; examples selected
  through `PLATFORMIO_SRC_DIR`) with the `symlink://` dependency, the pinned
  `rp2040_mouse_joystick` environment and the upstream `due` environment.
  Teensy builds now use TimerOne `1.2.0` from the janelia-arduino fork, which
  resolves to the same commit `953f240` used before.
- `pixi.toml` keeps the upstream per-example task names
  (`build-blinky-bsp-teensy4`, `upload-...`, SAM tasks, `ql-install`, `qm`,
  `qtools-install`, `qspy`) and adds the generic `build/upload --example
  --env` form, `provenance`, the RP2040 tasks and the `build-teensy`,
  `build-rp2040`, `build-sam` and `check` sets. `pixi.lock` was regenerated for
  all four platforms.
- `tools/quantum_leaps_tools.py` is re-pinned for qpcpp v7.3.4: QM `6.1.1`
  (GitHub publishes Linux and Windows binaries only) and QTools `7.3.4`. QTools
  7.3.4 has no binary release on GitHub or SourceForge, so the tool clones the
  git tag and builds `qspy` with `make` and the C compiler from the Pixi
  environment (MinGW gcc on Windows, untested). Verified on Linux: the clone
  and build succeed and `qspy -h` reports `QSPY 7.3.3`, as generated by that
  tag. The QM 6.1.1 download succeeds.
- The upstream `#define QS_ON` now applies to the Teensy and SAM examples on
  this branch as well; `blinky_bsp-teensy4` therefore links the QS code and
  grows to 23,360 bytes of code (from 16,160).
- `examples/dpp_bsp-sam/dpp_bsp-sam.ino` and the matching `.qm` template lose
  a bare `Q_DEFINE_THIS_FILE` line: the Arduino sketch preprocessor duplicates
  such a line when generating prototypes, producing a redefinition error, and
  the sketch uses no assertions (the Teensy DPP sketch never had it).

Post-merge software checks, all from the repository `.venv` (PlatformIO
6.1.19) unless noted:

- `examples/blinky_bsp-rp2040`: success, 76,272 bytes flash, 11,980 bytes RAM
  (unchanged).
- Teensy 4.1: `blinky_bsp-teensy4` 23,360 code / 4,040 data / 6,400 RAM1
  variables; `dpp_bsp-teensy4` 26,132 / 4,040 / 6,976;
  `port_validation-teensy4` 24,704 / 4,040 / 7,744.
- Arduino Due (`due`, Atmel SAM platform 9.0.0, framework-arduino-sam 1.6.12,
  GCC 7.2.1): `blinky-sam` 28,480 bytes flash, `blinky2-sam` 28,760,
  `blinky_bsp-sam` 33,528, `dpp_bsp-sam` 36,828 after the fix above. GCC 7.2.1
  warns about `QS::force_cast` mangling (`-Wnoexcept-type`) in the unchanged
  upstream `qs.hpp`; SAM remains best effort.
- Provenance check passed; `git diff --check` passed; no conflict markers.

## RP2040 Target Results — 2026-09-30

First target run, performed automatically from this branch with the fixture
and analyzer path recorded above. All evidence of every run (READMEs,
analysis scripts, `result_*.json` outputs, logs, raw analyzer captures,
exported VCDs and trace dumps) lives in the sibling repository
`qp-arduino-example` under `evidence/<run directory>/`; until 2026-10-01 the
READMEs, scripts, results and short logs were kept here under
`docs/evidence/` with the large files mirrored there, and on 2026-10-01 the
whole tree moved out so this library holds no evidence files and stays small
to clone. Blinky: `qp-arduino-example/evidence/2026-09-30-rp2040-blinky/`. The
application under test is `examples/blinky_bsp-rp2040`, not the
port-validation firmware, so these numbers are a Blinky baseline under the
stated conditions and do not by themselves close the coverage gates.

Firmware under test (all built from the merge commit `55ee412` plus the
uncommitted example changes recorded in this section's commit):

- **A** `rp2040_mouse_joystick` with the QS host wait (uploaded twice: once
  without it, once with it), `-DQS_ON`, universal single-lane `boot2`.
- **B** the same environment after the QS write-path fix below (final state
  of the fixture at the end of the session).
- **C** `rp2040_mouse_joystick_noqs`: QS compiled out, idle loop sleeps.
- **D** `rp2040_mouse_joystick_fastflash`: A plus the quad-lane
  `boot2_w25q128jvxq` second-stage loader.

Procedure: `pixi run upload-blinky-bsp-rp2040 /dev/ttyACM0` (1200 baud
touch, `picotool`), `qspy -k -c /dev/ttyACM0 -b 115200 -o` (QSPY 7.3.3 built
from the QTools `v7.3.4` tag), and `sigrok-cli --driver kingst-la2016` from
`instrument-control`'s Guix manifest with LA5032 CH16 = GP25 (LED) and
CH17 = GP21 (tick-ISR probe), exported to VCD and analyzed by
`analysis/analyze_blinky_capture.py`.

Results:

- **QSPY/QS handshake over USB CDC works.** Records decode with names once
  QSPY attaches within the 5 s host wait: `Trg-RST QP-Ver=733`, object,
  function and signal dictionaries, `St-Init`, then five records per
  transition (`AO-Post`, `AO-GetL`, `Disp`, `St-Entry`, `Tran`). A first
  attach 25 s after boot showed why the wait was added: USB CDC discards
  everything written before a host opens the port, so the dictionaries were
  lost and objects printed as addresses (resolved against the ELF: `0x20001818`
  = `Blinky::instance`, `0x10003338/0x1000336c` = `Blinky::off/on`).
  QUTest was not exercised.
- **Trace continuity, 11 minutes:** 1,319 consecutive `AO-Post` intervals,
  mean 500,000.01 µs, min 499,975, max 500,029, standard deviation 8.5 µs; no
  assertion, error or drop records. The one `<COMMS>` checksum error and
  sequence discontinuity in that log mark the boundary between QSPY sessions
  (52.5 s with no host attached), not a stall during the run.
- **Tick period (direct, ISR probe edges):** 601 ticks over 6 s at 1 MSa/s,
  mean 10,000.03 µs, min 9,987, max 10,013 µs with QSPY attached; 10,000.01 µs
  with min 9,997 / max 10,003 µs with the host detached or QS compiled out.
  Zero missed or doubled ticks in every capture (50 or 600 ticks each).
- **Tick average and continuity over 10 minutes:** the 600 s capture ran at
  100 kSa/s, which under-samples the 1.7 µs tick pulse, so its tick column is
  invalid. The LED channel is valid: 1,199 consecutive intervals of exactly
  50 ticks, mean 500,000.94 µs (tick average 10,000.02 µs, +0.0002 %), min
  499,770, max 500,260 µs. A missed or doubled tick would shift an interval by
  10 ms; the largest deviation was 0.26 ms. A direct 10-minute edge capture
  needs ≥ 2 MSa/s (1.2 G samples; the device reports a 10 G-sample limit) and
  was not run.
- **ISR duration:** 1.7 µs per tick (20 MSa/s), except the tick that posts the
  time event: 35 µs with the host detached or QS compiled out, 65 to 92 µs
  with QSPY attached, and 12 µs on the following tick. The quad-lane loader
  (D) gives the same 36 µs, so XIP cache misses are not the cause. The
  post path's cost is above expectation; it is 0.35 % of the tick. (Resolved
  later the same day: the alarm-pool timer that carried the tick cost most
  of it; 20 µs with the BSP-owned hardware alarm. See "Tick Source Defect
  Found And Fixed".)
- **Tick-to-LED latency** (ISR rising edge to the LED write in the active
  object's entry action, i.e. ISR plus QV dispatch): host detached 95 to 123
  µs, QS compiled out 93 to 150 µs, quad-lane loader 90 to 123 µs; with QSPY
  attached 137 to 314 µs (6 s) and 110 to 430 µs over the 600 s run (p99 360
  µs). The maximum is 4.3 % of one tick. The trace transport's share is the
  idle loop servicing USB CDC writes (`tud_task()` runs inside `Serial.write`),
  which delays the QV loop's next pass.
- **Tick jitter under trace load:** ±13 µs with QSPY attached versus ±3 µs
  without; both far inside the ±1 % criterion.
- **Application unaffected by the reader:** LED intervals stayed at 500 ms
  ±0.05 % with the host attached, detached and never attached; QS records were
  dropped, not blocked, when no host was reading.
- **Fixes made during the run:** the QS host wait
  (`QP_ARDUINO_QS_HOST_WAIT_MS`, default 5 s); QS callbacks are now defined
  unconditionally because the port always compiles `Q_SPY` (the no-QS variant
  otherwise fails to link); the bounded QS write path now honours the byte
  count `Serial.write()` returns instead of assuming the whole block went
  out. Two new environments, `rp2040_mouse_joystick_noqs` and
  `rp2040_mouse_joystick_fastflash`, exist for these comparisons.

What this does and does not establish: the imported ARMv6-M QV port runs on
the fixture with the pinned core, the tick and dispatch paths behave, and QS
over USB CDC is usable on the bench. Framework coverage (queues, pub/sub,
dynamic events, assertions, QS RX), QUTest, the two-hour soak, the slow-reader
case and the transmit-only profile remain open below.

## Repository Alignment And Further Examples — 2026-09-30

Changes made after the first Blinky target run, all on this branch:

- `README.org` became `README.md`, following the section order of the official
  `qpcpp` README (What's New, Getting Started, QP/Spy, Licensing,
  Documentation, Help). Example READMEs follow the `qpcpp-examples` layout
  (Features Demonstrated, Build Configurations, Code Organization, Building).
- `LICENSES/` mirrors the upstream license texts (`GPL-3.0-or-later.txt`,
  `QP-Arduino_GPL_Exception.txt`, `LicenseRef-QL-commercial.txt`,
  `License.txt`) from qpcpp `v7.3.4`; the root `LICENSE` and exception file
  stay for Arduino tooling. The provenance check now verifies all six files
  against the tag.
- `platformio.ini` pins the Teensy platform to `teensy@5.1.0`, the version the
  retained regression builds use (framework-arduinoteensy 1.160.0, GCC 11.3.1).
- `examples/dpp_bsp-rp2040` is generated from `dpp_bsp-rp2040.qm` and
  `examples/blinky_bsp-rp2040` from `blinky_bsp-rp2040.qm` with QM 6.1.1's
  console generator (`qmc`), which the QTools installer now makes
  executable; both models derive from the Teensy models with RP2040 BSPs. The model is the Teensy DPP model with an RP2040 fixture BSP;
  regenerating the Teensy DPP model with `qmc` reproduces the checked-in code
  except for QM 6 header comments, so the migrated models match their sources.
- `examples/port_validation-rp2040` ports the Teensy validator to the fixture
  with the two-marker scheme: GP21 is the tick probe; GP25's role (marker,
  dispatch, idle, event receipt) is selected by QS command 5.
  `tools/port_validation_rp2040.py` drives it through QSPY's UDP front-end
  (QS-RX info, counters, publish, dynamic events, roles, counter reset,
  assertion with reboot and re-attach); `pixi run validate-rp2040`.
- **On-target QUTest is deferred.** A QUTest build replaces `QF::run()`,
  `QActive::start()` and friends with the versions in `qutest.cpp` and uses the
  separate `ports/arm-cm/qutest` port header, but the Arduino library always
  compiles `qv.cpp` and the QV port header, so the two would collide. Running
  the upstream `qutest/` suites on the RP2040 target therefore needs either
  `#ifndef Q_UTEST` guards on the Arduino copies (a new documented delta) or a
  second library layout, and is left for later. The host-driven procedures in
  `tools/port_validation_rp2040.py` cover the same behaviours against the real
  QV kernel instead; the upstream host QUTest baseline from July stays the
  framework-level reference.
- **QS transport defect found and fixed** with DPP's higher record rate: the
  idle-loop writer took a block from the QS buffer before knowing the port
  would accept all of it, and dropped the remainder, so QSPY saw 60 checksum
  errors and sequence gaps in 25 s. The writer now asks the port for its free
  space first and takes at most that much (`qsTransmitSome`), and the flush
  path drains in the same way with a bound (`qsFlushBounded`). After the fix:
  1,624 records in 30 s with no errors. Both RP2040 BSPs and the validator use
  the same code.
- DPP on the fixture: tick period 10,000.00 µs (min 9,983, max 10,015), no
  missed or doubled ticks; the tick callback takes 34 µs typical and 49 to
  62 µs when a philosopher's time event expires, with QSPY attached or not.
  With five armed time events the per-tick cost is therefore about 34 µs
  against 1.7 µs for Blinky's single time event on non-expiring ticks; the
  cost is in the tick path and is host independent. Root cause still open.

## RP2040 Port-Validation Results — 2026-09-30

`examples/port_validation-rp2040` on the fixture, driven by
`tools/port_validation_rp2040.py` through QSPY's UDP front-end (no QUTest
build), with all six QS record groups streaming over USB CDC. Evidence:
`qp-arduino-example/evidence/2026-09-30-rp2040-port-validation/`.

- **Host-driven procedures: 8 of 8 pass** (`port_validation_results.json`):
  QS-RX round trip (`QS_RX_INFO` answered), tick rate 100.0 ticks/s from the
  Validator's own reports with one published event and one pool event per
  tick, publish/subscribe from a host command, host-posted pool event
  (pool get, dispatch, pool put), probe-role switching, counter reset, and
  the assertion path: `Q_ERROR_ID(400)` produced an `=ASSERT=` record, the
  USB port disappeared on the watchdog reset and reappeared, and the target
  answered again with `Trg-RST` and target info. The first attempt failed
  three checks through driver defects, all fixed (binary target-info packet,
  periodic reports mistaken for replies, device node held open across the
  reboot so the board re-enumerated as another tty; use the
  `/dev/serial/by-id/...` path).
- **ISR-to-active-object dispatch latency under the validation load** (tick
  ISR rising edge to the first dispatch's rising edge, exactly three
  dispatches per tick): QSPY attached p99 121 µs, maximum 133 µs over 599
  ticks at 1 MSa/s (41.3 / 91.0 / 121.1 µs min / p50 / max at 20 MSa/s over
  50 ticks); host detached 40 / 52 / 63 / 66 µs min / p50 / p99 / max.
  Tick to dynamic-event receipt (third dispatch) 148 to 244 µs with the host
  attached. Dispatch durations 20 µs typical, 37 µs maximum detached, 83 µs
  maximum attached. All below 3 % of one tick.
- **Load profile:** idle callback high 85.4 % of the time (about 1,089 idle
  passes of 8 µs per tick), dispatch 0.6 %; the remainder is the tick callback
  and the idle loop's USB CDC service. Tick period 9,999.99 to 10,000.00 µs
  mean, min 9,995, max 10,005 µs, no missed or doubled tick in any capture.
- **QS over UART0** (Blinky, `rp2040_mouse_joystick_uart`): the LA5032
  captured GP0 on channel 8 alongside the probes, sigrok decoded the 115200
  8N1 stream (1,081 bytes in 6 s) and `qspy -f` reconstructed 48 records with
  no checksum error. With the UART transport the posting-tick ISR cost falls
  to 10 µs and tick-to-LED latency to 34 to 46 µs, against 35 µs and 95 to
  123 µs with the USB CDC transport and no host; the open post-path cost
  therefore tracks the USB CDC transport in the idle loop, not QP. (Resolved
  later the same day: see the measurements in "Tick Source Defect Found And
  Fixed"; the tick source itself carried most of the cost.)
- **Anomaly, open:** in the second post-soak run, after the (then defective)
  assertion test and three role sessions, the target stopped: both probes
  low, no QS output, USB CDC still enumerated, recovered by the 1200 baud
  touch reflash (`captures/pv_wedged_snapshot.sr`, `second-run.log`). The
  follow-up run repeated the sequence with the fixed driver and did not
  reproduce it: the liveness session received 19,143 records and the tick was
  running (`pv_after_roles_snapshot.sr`). **Reproduced and root-caused later
  the same day; see "Tick Source Defect Found And Fixed".** `Trg-ERR 0x42` in
  these logs is the target reporting a QS-RX sequence restart at the start of
  each new host session and is benign.

## Tick Source Defect Found And Fixed — 2026-09-30

The single stall recorded above was reproduced deliberately and root-caused.

- **Reproduction:** `tools/port_validation_stress.py` cycles a trace host
  against `port_validation-rp2040` (attach QSPY, check QS-RX and the tick
  counters, hold a few seconds, detach; every fifth cycle a deliberately slow
  reader instead). Run 1 stalled at 141 s, cycle 13: from then on every cycle
  still attached, still answered `QS_RX_INFO` and still returned the counters,
  but the tick counter stayed at 17,841
  (`qp-arduino-example/evidence/2026-09-30-rp2040-port-validation/stress/stress-run1.*`).
- **Diagnosis** (`stall-diagnosis-transcript.txt`): with the target stalled,
  no `Tick<0>` records arrived in 3 s, yet three host-injected `QS_RX_TICK`
  commands each drove the complete chain (time-event post, publish, pool
  event, three dispatches, pool put) and advanced the counters to 17,844. QP,
  QS-RX, QS-TX and the QV loop were healthy; only the tick interrupt had
  stopped. The tick came from a pico-sdk repeating timer in the default alarm
  pool, which the Arduino core shares with its USB task and `delay()`; a
  failed re-add ends a repeating timer silently.
- **Fix:** every RP2040 BSP now claims a hardware timer alarm of its own
  (`hardware_alarm_claim_unused`), re-arms it from its own interrupt before
  calling `QTimeEvt::tick()`, and catches up rather than stopping if a target
  is already in the past. The idle loop watches the tick and, should it ever
  stop, re-arms it and emits a `TICK_LOST` user record (tick count, re-arms,
  catch-ups), so a stopped tick can no longer be silent. The stress driver is
  `pixi run`-able through `tools/port_validation_stress.py`; run 2 with the
  new tick is recorded below when it completes.
- **Also in this change:** the idle loop services the trace transport only
  when at least `QP_ARDUINO_QS_SERVICE_BYTES` (64) are pending or
  `QP_ARDUINO_QS_SERVICE_US` (1000) have passed, because every call into the
  USB CDC port runs TinyUSB's task; and Blinky gained the transmit-only
  profile (`rp2040_mouse_joystick_txonly`: UART0, `QP_ARDUINO_QS_TX_ONLY`
  never reads QS-RX, dictionaries re-emitted every `QP_ARDUINO_QS_DICT_PERIOD_MS`
  through `BSP::setDictionaryHook`, and a `TRACE_STAT` record every
  `QP_ARDUINO_QS_STAT_PERIOD_MS` with the QS fill high-water mark, buffer-full
  count and tick count).
- **Measurements with the hardware-alarm tick and the throttled service**
  (Blinky, 1 MSa/s, `qp-arduino-example/evidence/2026-09-30-rp2040-blinky/`, images F to
  I): the typical tick ISR is 4 µs (was 2; the callback now re-arms its own
  alarm). The posting tick fell from 35 to 20 µs without QS and the tick→LED
  latency from 93–150 to 24–25 µs, so the "open" post-path cost of the
  morning was mostly the alarm-pool handler around the old repeating-timer
  callback. With QSPY attached over USB CDC the posting tick fell from 85–92
  to 68 µs and the latency p99 from 314 to 218 µs; what remains is TinyUSB's
  task running inside the idle loop's transport service. QS over UART0:
  posting tick 29 µs, latency 45–86 µs. The transmit-only profile, captured
  and decoded by the analyzer over 30 s (6,709 bytes, 270 records, 60
  transitions, no checksum error): the dictionaries arrived three times so a
  capture that missed boot was fully named after 10 s, and the 30
  `TRACE_STAT` records reported a high-water mark of 90 to 102 of 2,048
  bytes with zero buffer-full events; posting tick 31 µs, latency 28 to
  67 µs. No `TICK_LOST` in any of these captures.
- **Stress run 2 (hardware-alarm tick, first version): 316 cycles in 3,604 s,
  63 of them slow-reader cycles, 0 flagged**, tick rate 99.8 to 100.1 per
  second in every cycle. The first analyzer capture spans both runs: 17,840
  exact ticks, silence from 181 s until the reflash at 724 s, then 293,391
  ticks of the new firmware without a gap over 49 minutes.
- **Second defect, in the new watchdog itself, found from that capture:** the
  period histogram was bimodal, a main lobe at 10,000 µs plus about 6 % of
  intervals at +60 to +110 µs with no matching short ones, and the run-2
  traces held 20,097 `TICK_LOST` records although the tick never stopped (the
  firmware's own re-arm counter in the last record read 23,856 spurious
  re-arms in 359,702 ticks, 6.6 %). The watchdog read the clock before the
  tick's timestamp; when the tick interrupt landed between the two reads, the
  unsigned difference wrapped, the watchdog "recovered" a running tick and
  re-based the next target by the idle loop's latency. The second analyzer
  capture (`pv_stress2_100kHz_1200s`, `analysis/tick_periods.py`) quantifies
  the defect: 6.42 % of periods more than 40 µs late while the host cycled,
  2.34 % with no host, maximum 10,160 µs, none missed or doubled. Fixed by
  reading the timestamp first and comparing a signed elapsed time.
- **Stress run 3 (hardware-alarm tick, watchdog race fixed): 78 cycles in
  876 s, 15 of them slow-reader cycles, 0 flagged**, tick rate 99.9 to 100.1
  per second in every cycle, and not one `TICK_LOST` record in the 78
  session logs (`stress/stress-run3.*`, counted by
  `analysis/stress_summary.py`). The parallel analyzer capture
  (`pv_stress3_100kHz_900s`, 89,717 consecutive periods over 897 s) is
  unimodal: mean 9,999.99 µs, stdev 7.8 µs, min 9,960, max 10,030 µs on the
  10 µs sampling grid, none more than 40 µs late, none missed or doubled
  (`analysis/result_pv_stress3_100kHz_900s.json`). The tick source and its
  watchdog are closed. Across runs 2 and 3 the target survived 394 host
  attach/detach cycles and 78 slow-reader cycles (1.8 MB read at a trickle
  while its QS buffer overflowed) with the tick unaffected.

## Active RP2040 Hardware Acceptance

- [ ] Upload the fixture firmware and establish the target QSPY/QUTest
      handshake with qtools `v7.3.4`; retain exact tool/configuration logs.
      **QSPY done 2026-09-30** with Blinky, DPP and the port-validation
      firmware (upload logs, QSPY logs and versions in the evidence
      directories). QUTest on the target is deferred; see "On-target QUTest
      is deferred" above.
- [x] Pass the RP2040 target coverage for queues, publish/subscribe, dynamic
      events, time events, active-object dispatch, idle behavior, assertions,
      QS TX and bench QS RX. Record expected assertion tests separately from
      the assertion-free soak. **Met 2026-09-30** by the host-driven
      procedures (8 of 8) plus the role captures; the expected assertion is
      its own procedure and the soak above ran assertion-free. The single
      stall seen in the second run was reproduced, root-caused to the tick
      source and fixed the same day ("Tick Source Defect Found And Fixed");
      stress run 3 and its analyzer capture verify the fix.
- [x] Use a qualified analyzer path on the fixture's actual probe map to
      measure a 100 Hz tick average within +/-1%, with no missed or doubled
      ticks during a 10-minute run. **Met 2026-09-30 with Blinky**: tick
      average 10,000.02 µs over 600 s derived from 1,199 consecutive 50-tick
      LED intervals (largest deviation 0.26 ms, so no tick was missed or
      doubled), plus direct edge captures of 600 ticks at ±13 µs. **Direct
      600 s edge capture done later the same day with DPP running and QSPY
      attached** (2 MSa/s, 1.2 G samples): 60,000 consecutive ticks, mean
      9,999.998 µs (−0.00002 %), min 9,981, max 10,015.5, stdev 4.6 µs, no
      missed or doubled tick; and the 2 h soak capture at 100 kSa/s,
      720,001 ticks, mean 9,999.995 µs, min 9,980, max 10,020, none missed
      or doubled; `qp-arduino-example/evidence/2026-09-30-rp2040-dpp/`.
- [x] Record ISR-to-active-object dispatch p99 and maximum latency under
      documented validation load; maximum must remain below one 100 Hz tick.
      **Met 2026-09-30 under the port-validation load**: p99 121 µs,
      maximum 133 µs with QSPY attached (599 ticks); 66 µs maximum detached.
      Blinky baseline earlier the same day: p99 360 µs, maximum 430 µs over
      600 s attached.
- [x] Complete a two-hour soak without unexpected port assertions, recording
      load, resets and trace/capture coverage rather than assuming silence
      establishes success. **Met 2026-09-30 with DPP**: 2 h 15 min of
      continuous QSPY trace (420,595 records, 0 `=ASSERT=`, 0 `Trg-RST`, the
      only two `<COMMS>` lines at the session-start boundary, largest gap
      between records 1.45 s from DPP's own rhythm) while the analyzer
      recorded 720,001 consecutive ticks over 7,200 s at 100 kSa/s (mean
      9,999.995 µs, min 9,980, max 10,020, none missed or doubled) and
      19,074 LED edges with at least 151 per minute. Load: five Philo and one
      Table active object, pool events, QS at about 55 records/s.
      `qp-arduino-example/evidence/2026-09-30-rp2040-dpp/`.
- [ ] Demonstrate bounded application response and continued local control
      with the trace reader absent, slow or disconnected and under declared
      trace/queue pressure. **Reader absent and disconnected shown 2026-09-30**
      with Blinky (LED period unchanged, records dropped not blocked). **Slow
      reader shown 2026-09-30** by stress runs 2 and 3 against the
      port-validation firmware: 78 slow-reader cycles in which the host read
      at a trickle until the target's QS buffer overflowed, with the tick at
      99.8 to 100.1 per second in every cycle and QS-RX answering afterwards.
      Still open: declared queue/pool pressure, and recording RAM/flash,
      queue/pool margins, trace loss, handler duration and transport overhead
      against budgets declared before the run (the transmit-only profile's
      `TRACE_STAT` record now reports trace loss; budgets are not yet
      declared).
- [x] Qualify a transmit-only observation profile separately from the bench
      QS RX/QUTest profile. Experiment observation must not accept QS reset,
      memory-write, event-injection or command input. Preserve dictionaries,
      clock/reset interpretation and explicit loss reporting in saved traces.
      **Met 2026-09-30 on the Blinky fixture build**
      (`rp2040_mouse_joystick_txonly`: QS on UART0, `QP_ARDUINO_QS_TX_ONLY`
      compiles the QS-RX path out of the idle loop, so no reset, command,
      event-injection or memory-write input exists; dictionaries re-emitted
      every 10 s; `TRACE_STAT` every second with the buffer high-water mark
      and buffer-full count; `Trg-RST` and the 32-bit `micros()` timestamp
      carry reset and clock interpretation). Verified by a 30 s analyzer
      capture decoded offline: a capture that missed boot was fully named
      after one dictionary period, zero buffer-full events, no checksum
      error. Still to do before experiment use: apply the same profile to
      the application firmware and declare its trace budget; the fixture
      profile does not claim the joystick's record rate.

These gates use a standalone fixture and available qualified bench connections.
Their completion does not depend on a new debug connector, reusable probe
product, Digital-50 PCB revision, or migration of the joystick application.
Measurements on new board/probe/cable combinations require their own `OBS-HW`
qualification before experiment use.

## Status And Next Steps — End Of 2026-09-30

Where the RP2040 qualification stands after this day's work, for whoever
picks it up next (the gate lists above are authoritative; this is the
summary).

- **Met on the fixture:** QSPY handshake, target coverage (8 of 8
  host-driven procedures, expected assertion included), 100 Hz tick within
  ±1 % with no missed or doubled tick (10 min direct, 2 h soak, 15 min under
  host cycling), dispatch latency below one tick (p99 121 µs, max 133 µs
  attached), two-hour assertion-free soak, reader absent/slow/disconnected,
  transmit-only observation profile. One defect was found and fixed in the
  BSP (tick from the shared alarm pool) plus a race in its first watchdog;
  none in the QP port itself.
- **Still open before an RP2040-scoped release:** on-target QUTest
  (deferred; `qv.cpp` is always compiled, see "On-target QUTest is
  deferred"); declared queue/pool budgets and RAM/flash margins recorded
  against them; the release/support statement itself. The architecture
  selector, the PlatformIO manifest and the version policy were settled on
  2026-10-01 ("Package Metadata Decision — 2026-10-01").
- **Recommended order (revised 2026-10-01):** first a transmit-only QS
  profile on a PIO UART driving GP21 (`FW_TRIG1`), measured on the fixture
  through analyzer channel 17 (**done the same day**, see "PIO UART Trace On
  FW_TRIG1 — 2026-10-01"). Reason: the `mouse-joystick-pcb` v1.0 board
  has no spare GPIO and no second hardware UART, GP21 and GP25 are both
  UART1 RX in the RP2040 mux, and `FW_TRIG0`/`FW_TRIG1` are the only
  MCU-driven nets besides the pyControl pair that reach its Digital-50 port
  (`DIG0`/`DIG1`), so this is the only way to trace while UART0 serves
  pyControl. The same GPIO is already on the rev 0 fixture's analyzer, so the
  measurement transfers. arduino-pico 4.2.0 `SerialPIO` supports a TX-only
  instance (`NOPIN` RX); its `availableForWrite()` is the free space of the
  8-entry PIO FIFO and `write()` blocks only when that FIFO is full, so the
  existing `qsTransmitSome()` discipline holds; the ceiling of eight bytes
  per idle pass must be measured (a DMA-fed PIO transmitter is the
  fallback). Using `FW_TRIG1` this way is an `OBS-IF` overlay decision
  still to be recorded: it keeps `FW_TRIG0` as the SYNC marker, runs the
  identity beacon before the PIO UART starts, and turns the FREEWHEEL level
  marker into a QS record. Then the joystick slice (protocol board, UART
  wrapper and simulated motion in one active object on the fixture) with a
  declared trace budget (**done the same day**, see "Joystick Link Pilot
  And Two BSP Defects — 2026-10-01"), then the STM32G0B1CEU6 BSP. Teensy
  hardware gates stay deferred.
- **Evidence location (2026-10-01):** every evidence file of every run now
  lives in `qp-arduino-example/evidence/`; this repository holds none, by
  the user's rule that it must stay quick to clone (`AGENTS.md`,
  "Completion Discipline"). `.gitignore` refuses `.sr`, `.vcd`, `.bin`,
  `.gz` and `.tar` files here.
- **Fixture state at hand-off (updated 2026-10-01, evening):**
  `mouse-joystick-pcb` rev 0 unit E4654C159F2C2534 on the LA5032 adapter,
  36 V unplugged, flashed with the standard Blinky `rp2040_mouse_joystick`
  built from the tree with the drain and flush fixes (sha256
  `822164b9c019bff1`, 180,224 bytes); no host process attached. The bench scripts that produced the evidence are kept
  verbatim in `qp-arduino-example/evidence/bench-scripts/`; the
  `pixi run` tasks `validate-rp2040` and `stress-rp2040` are the supported
  entry points.

## Package Metadata Decision — 2026-10-01

Decided before any RP2040-scoped release and applied to `library.properties`,
`src/qp-arduino/library.json`, the README and this ledger. No framework or
port file changed.

- **`architectures=rp2040`.** The port compiles only for ARM Cortex-M:
  `qp_port.hpp` and `qv_port.cpp` select ARMv6-M or ARMv7-M on `__ARM_ARCH`
  with no other branch, so the previous `avr` value promised classic AVR
  boards a build that fails in inline assembly. `avr` was only ever there for
  Teensy: the PJRC package index declares every Teensy board under
  `"architecture": "avr"` (checked 2026-10-01; the local install is
  `~/.arduino15/packages/teensy/hardware/avr`). `rp2040` is the arduino-pico
  core's string (`package_rp2040_index.json` for 4.2.0, checked the same
  day). The field is advisory: a mismatch produces the Arduino IDE's "may be
  incompatible" warning and stops neither installation nor compilation, so
  the retained Teensy and Due examples still build. `avr` is added back, as a
  recorded delta meaning Teensy 4.x, only when the deferred Teensy hardware
  gates pass. `sam` stays out (Due is best effort, not a support claim); the
  Arduino Mbed RP2040 core (`mbed_rp2040`) has no build evidence; upstream's
  `*` is wrong for architecture-specific code.
- **`library.json` `platforms` is `["raspberrypi"]`** (was `*`), the name of
  the installed PlatformIO platform that carries arduino-pico. PlatformIO's
  default `lib_compat_mode = soft` checks frameworks only, and this repository
  links the library by `symlink://` with `lib_ldf_mode = off`, so the `teensy`
  and `atmelsam` environments are unaffected; see the verification below.
  The `description` and the Arduino `paragraph` now name the imported QP/C++
  version and the qualified core.
- **Version policy.** The library version is a semver `MAJOR.MINOR.PATCH`.
  `MAJOR.MINOR` equals the imported qpcpp `MAJOR.MINOR`, because the API a
  sketch programs against is QP's. `PATCH` starts at the imported qpcpp patch
  level and advances by one for every qp-arduino release on that line,
  whether the change is a port fix or an upstream patch import; an upstream
  patch import takes the next free `PATCH`, which is never below the upstream
  patch number. The exact imported qpcpp tag is recorded in this ledger, the
  README and the `paragraph`, and is never derived from the version number or
  from `QP_VERSION_STR`: upstream tag `v7.3.4` ships `include/qp.hpp` with
  `QP_VERSION 733U` and `"7.3.3"`, and that file is kept byte-identical for
  provenance. Rejected forms: a pre-release suffix (semver orders
  `7.3.4-x.2` below `7.3.4`, so the IDE would offer a downgrade), build
  metadata (ignored for precedence, so no update is ever seen) and four-part
  versions (not semver; the Library Manager indexer rejects them).
  `library.json` carries the same number. The first v7 release is 7.3.4 and
  the next port-only release is 7.3.5. Release tags follow upstream
  qp-arduino: `v` plus the library version.
- **Verification (2026-10-01):** `pixi run provenance` passed against
  qpcpp `v7.3.4`. With the new manifests, `blinky_bsp-teensy4` (`teensy41`,
  rebuilt from clean), `blinky_bsp-sam` (`due`) and `blinky_bsp-rp2040`
  (`rp2040_mouse_joystick`) all build; each dependency graph lists
  `qp-arduino @ 7.3.4`, so the narrowed `platforms` list excludes nothing
  under the default compatibility mode. Metadata only: no firmware was
  uploaded and no target result changes.

## PIO UART Trace On FW_TRIG1 — 2026-10-01

The transmit-only observation profile of 2026-09-30 was moved from UART0 to
a **PIO UART driving GP21 (`FW_TRIG1`)** and measured on the fixture.
Evidence and procedure: `qp-arduino-example/evidence/2026-10-01-rp2040-trig1-trace/`
(README, analysis, decodes, captures); bench script
`qp-arduino-example/evidence/bench-scripts/trig1_run.sh`.

- **Why.** The `mouse-joystick-pcb` v1.0 board has no spare GPIO, one
  hardware UART (UART0 is pyControl), GP21 and GP25 are both UART1 RX in the
  RP2040 mux, and `FW_TRIG0`/`FW_TRIG1` are its only MCU-driven nets besides
  the pyControl pair that reach the Digital-50 port (`DIG0`/`DIG1`). A trace
  the analyzer can see while pyControl owns UART0 therefore has to leave on a
  marker line, which only a PIO state machine can drive as a UART. The rev 0
  fixture exposes the same GPIO on channel 17.
- **Change (Blinky only).** `examples/blinky_bsp-rp2040/bsp.cpp` gains
  `QP_ARDUINO_QS_PIO_TX_PIN` (an arduino-pico `SerialPIO` with `NOPIN` RX
  becomes `QP_ARDUINO_QS_SERIAL`), `QP_ARDUINO_QS_BAUD` (default 115200) and
  honours `QP_ARDUINO_TICK_PROBE_PIN` on the fixture, whose safe state now
  drives `FW_TRIG1` low explicitly. The edit was made in the model
  (`blinky_bsp-rp2040.qm`) and `qmc` regenerates the identical file. New
  environment `rp2040_mouse_joystick_trig1`: PIO trace on GP21, tick probe on
  GP0 (the unused UART0 TX pad, `DIG8`, channel 8), otherwise the `txonly`
  settings. `SerialPIO::availableForWrite()` is the free space of the joined
  8-entry PIO FIFO and `write()` blocks only when it is full, so
  `qsTransmitSome()` keeps its never-block discipline; the hardware
  `SerialUART::availableForWrite()` reports 1 or 0, so the UART0 profiles
  moved one byte per idle pass. The `rp2040_mouse_joystick`, `_txonly` and
  `_trig1` Blinky builds pass; the DPP and port-validation BSPs do not carry
  the option yet.
- **Results (trig1 image sha256 `7a05ce783d3b7d15`, 1 MSa/s, CH8/CH16/CH17).**
  30 s: 6,702 trace bytes decoded by the analyzer, 60 transitions, 30
  `TRACE_STAT` records with high-water 90–91 of 2,048 bytes and zero
  buffer-full events, dictionaries at 10, 20 and 30 s, no checksum error.
  600 s: 134,042 bytes, 1,200 transitions, 600 `TRACE_STAT` records
  (high-water 90–95 bytes, zero buffer-full), dictionaries 60 times, no
  `TICK_LOST`, no decode error; 60,000 tick periods mean 9999.99 µs,
  stdev 0.81, min 9977, max 10021, 0 missed, 0 doubled, histogram
  −20: 22, −10: 95, 0: 59,768, +10: 91, +20: 24 (no second lobe); ISR width
  p99 12 / max 17 µs; tick→LED latency min 47, p50 53, p99 73, max 75 µs
  over 1,200 edges; LED interval within ±26 µs of 500 ms. Against the UART0
  transmit-only run of 2026-09-30 the tick is identical and the latency is
  20–25 µs higher at every percentile (28/30/62/67 µs there), consistent with
  the idle pass pushing up to eight bytes through the driver's mutex-protected
  byte writes instead of one; not root-caused, and under one percent of a
  tick.
- **What it does not establish.** Blinky's load only (about 220 bytes/s plus
  1.1 kB of dictionaries every 10 s); no higher baud rate; nothing about the
  v1.0 board's Digital-50 buffers, biases or the v2.0 analyzer adapter. The
  "declared queue/pool budgets" item stays open: Blinky's 95-byte high-water
  is a measurement, not the joystick application's budget.
- **Decision owed elsewhere.** Carrying the trace on `FW_TRIG1` is an
  `OBS-IF` overlay decision for the joystick board: `FW_TRIG0` stays the SYNC
  marker, the identity beacon must run before the PIO UART starts, and the
  FREEWHEEL level marker becomes a trace record. This branch records the
  transport, not that contract.
- **Next.** The joystick slice as a qp-arduino example on the fixture
  (`ProtocolBoard`, `PycUart` and `SimulatedMotion` inside one active object,
  36 V unplugged) on this transport, with a declared trace budget; then the
  STM32G0B1CEU6 BSP.

## Joystick Link Pilot And Two BSP Defects — 2026-10-01

The trace pilot of `docs/observability-plan.md` (`OBS-QP-3`/`OBS-QP-4`) ran
on the fixture the same afternoon as the first application-shaped load on
this port: the joystick firmware's protocol state machine (`ProtocolBoard`)
and its simulated motion backend, unchanged and compiled straight from
`mouse-joystick-firm/src` (commit 2f243d4), inside one active object, with
the protocol over USB CDC from a host script and the QS trace on the PIO UART
at GP21. It lives in `qp-arduino-example/rp2040/joystick_link_bsp-rp2040/`
(this library holds no application code); evidence, procedure and the
per-pass analysis are in
`qp-arduino-example/evidence/2026-10-01-rp2040-joystick-link/`. Nothing on
the board was energised.

- **Result.** Every declared budget was met on the observation trace set
  (user records, transitions, entry/exit, assertions; the per-event
  dispatch/post/get records off): 971 protocol lines dispatched and 1,174
  replies and events sent over 20 s with no line or reply dropped; QS
  high-water 505 of 2,048 bytes and zero buffer-full; event queue minimum
  free 15 of 16, line pool minimum free 6 of 8; tick exact with no lost
  tick; the host-loss fault 108 ms after the last heartbeat; 200 simulated
  18° moves completing in 102–104 ms (100 ms simulated plus one tick) with
  the operation marker on `FW_TRIG0` matching every `OP_START`/`OP_DONE`
  pair; host round trip p50 2.3 ms for a heartbeat and 3.7 ms for a command,
  p99 under 6 ms. Per dispatched line the protocol code itself costs
  0.4–2 ms on this Cortex-M0+ (reply formatting with `double`), the USB CDC
  write 0.2–1 ms, and QP's records a few microseconds; the full record set
  saturates 115200 baud at a 100 Hz tick exactly as declared (599
  buffer-full seconds, 35 % of lines lost from the trace) while the
  application runs unaffected.
- **Defect 1, example BSPs: the trace drain raced interrupt-written
  records.** `qsTransmitSome()`/`qsFlushBounded()` called `QS::getBlock()`
  with interrupts enabled. Every upstream Cortex-M BSP calls it under
  `QF_INT_DISABLE()`, because it changes the ring buffer's shared state and
  interrupts write records too. With the full record set (the tick's
  `AO-Post` comes from the interrupt) the pilot corrupted records and reset
  within seconds on two passes; the observation set, which writes nothing
  from interrupts, never showed it, and neither did yesterday's Blinky and
  DPP runs, whose default filters do include `AO-Post`, in two hours of soak
  (a narrow window, not a safe one). Fixed in all three RP2040 BSPs
  (`qsTakeBlock()`), models regenerated identically by `qmc`; the pilot's
  full set then ran its whole sequence twice without a reset.
- **Defect 2, example BSPs: periodic dictionary re-emission stalled the
  idle loop.** Every QS dictionary function ends with `QS::onFlush()`, which
  this BSP implements as a bounded synchronous drain; at 115200 baud the
  10 s re-emission therefore held the idle loop, and with it every active
  object, for about 30 ms (late completions and 60 ms heartbeat gaps every
  10 s in the first pass). Fixed by suppressing the flush during the
  re-emission so the copies drain through the normal idle-loop service
  (Blinky BSP and model, pilot BSP). Yesterday's transmit-only measurements
  were taken with this stall present; it does not touch the tick or the
  decoded trace and only could have added an LED-latency outlier that did
  not occur in those samples.
- **Found and avoided, library (upstream).** `QActive::getQueueMin()` is
  declared in `qp.hpp` at `v7.3.4` but defined nowhere in `src/`, so
  `QF::getQueueMin()` is a link error; the pilot reads its queue's
  `getNMin()` through the AO instead. Also: a QF query with its own critical
  section (`QF::getPoolMin()`) must not be called inside `QS_BEGIN`/`QS_END`,
  since this port's critical sections do not nest; the first pilot attempt
  asserted there on every boot.
- **Observations for the joystick decision.** The trace answers "did the
  real protocol code do what it should" with request identity: arrival
  (`LINK_RX` seq/verb), state change, simulated start and completion with a
  marker edge the analyzer sees, reply hand-over, host-loss fault; the
  non-QP delivery firmware would need an equivalent record set and transport
  to offer the same. Cost on the board is dominated by `ProtocolBoard`'s own
  work and the USB CDC port, not by QP. The tick jitter rises from ±20 µs
  (Blinky) to ±180 µs with USB CDC traffic because the USB interrupt shares
  the alarm interrupt's priority; raising the tick alarm's priority is a BSP
  refinement for later. The hardware UART the real board uses for pyControl
  would replace the CDC cost.
- **Fixture state.** Standard Blinky `rp2040_mouse_joystick` from the fixed
  tree (sha256 `822164b9c019bff1`) is flashed; 36 V unplugged; no host
  attached.
- **Next.** Decide with `OBS-IF` the `FW_TRIG1` trace overlay for the v1.0
  board; then either the STM32G0B1CEU6 BSP or, if the joystick firmware
  adopts this structure, its real `TmcMotion` backend behind the same AO on
  the delivery board with UART0 as the protocol port.

## Deferred Teensy 4.1 Hardware Gates — Nonblocking For RP2040

No Teensy hardware result is implied by the software results above. The original
hardware checks remain unperformed, deferred by the 2026-09-30 target decision:

- [ ] Target QSPY/QUTest handshake with qtools `v7.3.4` and the Teensy
      port-validation firmware.
- [ ] Target tests for queues, pub/sub, dynamic events, time events,
      active-object dispatch, idle, assertions, QS TX and QS RX.
- [ ] LA5032 captures on the fixture's verified Teensy probe pins 2-6:
      100 Hz tick average within +/-1%; no missed or doubled ticks for
      10 minutes; ISR-to-dispatch latency below one tick under validation
      load, with p99 and maximum recorded.
- [ ] Two-hour soak without unexpected port assertions.

Retain these criteria if Teensy hardware qualification resumes. Their deferred
status neither erases the previous build evidence nor constitutes a Teensy
support claim. Retained SAM examples remain best-effort sources, not a
qualification target.

## Release And Support Claims

The branch remains unreleased/unvalidated for its active RP2040 target until
the active software and hardware gates pass. A later release must identify the
exact qualified target/core/BSP configurations and metadata scope; deferred
Teensy hardware checks do not block an RP2040-scoped release and cannot be
reported as passed. STM32G0B1CEU6 requires its later independent gates.

For every supported configuration, maintain strict v7 APIs, no stale v6
framework files, documented minimal deltas, passing provenance/build checks,
and the imported tag's license/exception provenance. Preserve raw evidence and
measured units; tighten timing thresholds only after recording a baseline.

## Continuing Assumptions

- The upstream sources remain local `../qpcpp` tag `v7.3.4` and `../qtools`
  tag `v7.3.4`; exact commits are recorded above.
- QM 6.1.1 remains the editor/generator reference. If unavailable, update source
  and model snippets consistently without claiming regeneration.
- No electrical support is inferred from package architecture selectors or
  source-level ARMv6-M compatibility. The RP2040 fixture/core versions and
  STM32G0B1CEU6 follow-on BSP remain explicit open selections.
