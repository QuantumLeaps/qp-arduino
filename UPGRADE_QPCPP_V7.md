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
- [ ] Adapt validation probe roles to that pin map; Teensy pins 2-6 are not an
      RP2040 allocation. Only GP25 and GP21 reach the analyzer as free markers,
      so the RP2040 port-validation example must define how the timer,
      dispatch, idle, event and marker roles share them. Blinky uses GP25 as
      the LED and GP21 as the tick-ISR probe.
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
      **Blinky built 2026-09-30** (see "RP2040 Software Build Results");
      DPP, port-validation and every host-driven procedure remain open.
- [ ] Run provenance checks after framework/port changes and retain the
      existing Teensy example build regressions. Re-run the recorded upstream
      host baseline when framework/test-tool changes warrant it; distinguish
      those host results from RP2040 target evidence.
- [ ] Review Arduino package architecture metadata against the selected core
      and actual build evidence before publishing RP2040 support. The current
      `library.properties` still says `architectures=avr` for the historical
      Teensy packaging decision. It is unchanged by this documentation update
      and does not advertise or establish RP2040 support. Record any later
      metadata change as an intentional delta; do not broaden it speculatively.

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
and analyzer path recorded above. The evidence directories under
`docs/evidence/` hold READMEs, analysis scripts, `result_*.json` outputs,
summaries and short logs; the raw analyzer captures (`.sr`), exported VCDs,
compressed trace dumps and logs over 100 KB live in the sibling repository
`qp-arduino-example` under `evidence/<same directory name>/` so this library
stays small to clone. Blinky: `docs/evidence/2026-09-30-rp2040-blinky/`. The
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
  post path's cost is above expectation and remains an open item for the
  port-validation work; it is 0.35 % of the tick.
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
`docs/evidence/2026-09-30-rp2040-port-validation/`.

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
  therefore tracks the USB CDC transport in the idle loop, not QP.
- **Anomaly, open:** in the second post-soak run, after the (then defective)
  assertion test and three role sessions, the target stopped: both probes
  low, no QS output, USB CDC still enumerated, recovered by the 1200 baud
  touch reflash (`captures/pv_wedged_snapshot.sr`, `second-run.log`). The
  follow-up run repeated the sequence with the fixed driver and did not
  reproduce it: the liveness session received 19,143 records and the tick was
  running (`pv_after_roles_snapshot.sr`). One occurrence, cause unknown; a
  long host attach/detach cycling test is the next step. `Trg-ERR 0x42` in
  these logs is the target reporting a QS-RX sequence restart at the start of
  each new host session and is benign.

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
      its own procedure and the soak above ran assertion-free. Open beside it:
      the single unexplained stall in the second run.
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
      or doubled; `docs/evidence/2026-09-30-rp2040-dpp/`.
- [ ] Record ISR-to-active-object dispatch p99 and maximum latency under
      documented validation load; maximum must remain below one 100 Hz tick.
      **Met 2026-09-30 under the port-validation load**: p99 121 µs,
      maximum 133 µs with QSPY attached (599 ticks); 66 µs maximum detached.
      Blinky baseline earlier the same day: p99 360 µs, maximum 430 µs over
      600 s attached.
- [ ] Complete a two-hour soak without unexpected port assertions, recording
      load, resets and trace/capture coverage rather than assuming silence
      establishes success. **Met 2026-09-30 with DPP**: 2 h 15 min of
      continuous QSPY trace (420,595 records, 0 `=ASSERT=`, 0 `Trg-RST`, the
      only two `<COMMS>` lines at the session-start boundary, largest gap
      between records 1.45 s from DPP's own rhythm) while the analyzer
      recorded 720,001 consecutive ticks over 7,200 s at 100 kSa/s (mean
      9,999.995 µs, min 9,980, max 10,020, none missed or doubled) and
      19,074 LED edges with at least 151 per minute. Load: five Philo and one
      Table active object, pool events, QS at about 55 records/s.
      `docs/evidence/2026-09-30-rp2040-dpp/`.
- [ ] Demonstrate bounded application response and continued local control
      with the trace reader absent, slow or disconnected and under declared
      trace/queue pressure. **Reader absent and disconnected shown 2026-09-30**
      with Blinky (LED period unchanged, records dropped not blocked); slow
      reader and declared queue pressure still open. Record RAM/flash, queue/pool margins, trace loss,
      handler duration and transport overhead against budgets declared before
      the run.
- [ ] Qualify a transmit-only observation profile separately from the bench
      QS RX/QUTest profile. Experiment observation must not accept QS reset,
      memory-write, event-injection or command input. Preserve dictionaries,
      clock/reset interpretation and explicit loss reporting in saved traces.
      **Transport shown 2026-09-30**: QS over UART0 captured and decoded by
      the analyzer (`rp2040_mouse_joystick_uart`). Still open: a build that
      disables QS-RX processing, dictionary preservation in saved captures,
      and loss reporting.

These gates use a standalone fixture and available qualified bench connections.
Their completion does not depend on a new debug connector, reusable probe
product, Digital-50 PCB revision, or migration of the joystick application.
Measurements on new board/probe/cable combinations require their own `OBS-HW`
qualification before experiment use.

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
