# qp-arduino qpcpp v7.3.4 Upgrade Plan

## Summary

- Upgrade `qp-arduino` from qpcpp v6-derived files to qpcpp tag `v7.3.4` (`10441f7a03fa956b9e4772a073a05a96487983e2`).
- Use strict v7 API by default: `QP_API_VERSION` defaults to `9999`; do not add v6 public-header compatibility shims.
- Preserve upstream generated framework metadata exactly, including the known `QP_VERSION_STR "7.3.3"` value in qpcpp `v7.3.4`; document this mismatch instead of locally patching generated files.
- As decided 2026-09-30, RP2040 with the Earle Philhower Arduino core family is the primary qualification target. Exact fixture/core versions and all RP2040 software/hardware gates remain open.
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
  PlatformIO links the QP library with `lib_ldf_mode = off`.
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

## Active RP2040 Software Gates — All Open

- [ ] Select a simple non-actuating fixture with accessible programming,
      trace and measurement pins. Record its identity, revision, actual wiring
      and clock setup. Adapt validation probe roles to that pin map; Teensy
      pins 2-6 are not an RP2040 allocation.
- [ ] Pin and record the Earle Philhower core version/commit, PlatformIO
      platform, compiler, SDK dependencies and build configuration. The core
      family is decided; exact versions and fixture are still open.
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

## Active RP2040 Hardware Acceptance — All Open

- [ ] Upload the fixture firmware and establish the target QSPY/QUTest
      handshake with qtools `v7.3.4`; retain exact tool/configuration logs.
- [ ] Pass the RP2040 target coverage for queues, publish/subscribe, dynamic
      events, time events, active-object dispatch, idle behavior, assertions,
      QS TX and bench QS RX. Record expected assertion tests separately from
      the assertion-free soak.
- [ ] Use a qualified analyzer path on the fixture's actual probe map to
      measure a 100 Hz tick average within +/-1%, with no missed or doubled
      ticks during a 10-minute run.
- [ ] Record ISR-to-active-object dispatch p99 and maximum latency under
      documented validation load; maximum must remain below one 100 Hz tick.
- [ ] Complete a two-hour soak without unexpected port assertions, recording
      load, resets and trace/capture coverage rather than assuming silence
      establishes success.
- [ ] Demonstrate bounded application response and continued local control
      with the trace reader absent, slow or disconnected and under declared
      trace/queue pressure. Record RAM/flash, queue/pool margins, trace loss,
      handler duration and transport overhead against budgets declared before
      the run.
- [ ] Qualify a transmit-only observation profile separately from the bench
      QS RX/QUTest profile. Experiment observation must not accept QS reset,
      memory-write, event-injection or command input. Preserve dictionaries,
      clock/reset interpretation and explicit loss reporting in saved traces.

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
