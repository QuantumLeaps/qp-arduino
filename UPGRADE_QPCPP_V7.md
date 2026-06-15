# qp-arduino qpcpp v7.3.4 Upgrade Plan

## Summary

- Upgrade `qp-arduino` from qpcpp v6-derived files to qpcpp tag `v7.3.4` (`10441f7a03fa956b9e4772a073a05a96487983e2`).
- Use strict v7 API by default: `QP_API_VERSION` defaults to `9999`; do not add v6 public-header compatibility shims.
- Preserve upstream generated framework metadata exactly, including the known `QP_VERSION_STR "7.3.3"` value in qpcpp `v7.3.4`; document this mismatch instead of locally patching generated files.
- Teensy 4.1 is the required target. SAM examples are updated best-effort but are not hardware acceptance gates.

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
- Examples and `.qm` snippets are migrated from v6 APIs to v7 APIs without
  claiming QM regeneration unless the model is actually regenerated later.

## Key Changes

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

## Verification Work

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

## Acceptance Criteria

- No stale v6 framework files remain in `src/qp-arduino`.
- Strict v7 default builds pass without relying on `QP_API_VERSION < 730` compatibility macros.
- Provenance check passes with only documented local Arduino deltas.
- Teensy 4.1 Blinky, DPP, and port-validation builds pass.
- QSPY/QUTest handshake works with qtools `v7.3.4`.
- Target QUTest suite passes on Teensy 4.1.
- LA5032 validation shows:
  - 100 Hz tick average within +/-1%.
  - No missed or doubled ticks during a 10-minute run.
  - ISR-to-active-object dispatch latency remains below one tick under validation load.
  - No port assertions during a 2-hour soak.
- Timing captures and measured p99/max latency are documented after the first hardware run; thresholds may be tightened only after the baseline is recorded.

## Assumptions

- The upgrade uses the local `../qpcpp` tag `v7.3.4` and local `../qtools` branch/tag `v7.3.4`.
- QM 6.1.1 is the target editor/generator version, but if QM is unavailable during implementation, example sources and `.qm` text snippets are patched together without falsely claiming regeneration.
- Teensy pins 2-6 are available for validation probes.
- SAM examples are kept source-compatible where straightforward, but Teensy 4.1 is the only required electrical target.
