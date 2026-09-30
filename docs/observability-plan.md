# QP observability development plan

Status: revised development intent, 2026-09-30. This document records `OBS-QP`;
it does not establish new target support or pass a validation gate.
The shared [architecture](../../orb-weaver-context/context/experimental-system-observability.md)
owns cross-project intent, and the shared
[roadmap](../../orb-weaver-context/context/experimental-system-observability-plan.md)
owns stage dependencies and integration status. Stage IDs `G0`–`G5` refer to
that roadmap, not to this repository's migration acceptance criteria.

## Scope and authority

[UPGRADE_QPCPP_V7.md](../UPGRADE_QPCPP_V7.md) remains authoritative for exact
provenance, historical results, and the active/deferred gate ledger. The scope
remains QP/C++ `v7.3.4`, QV only, with `<qpcpp.hpp>` the public entry point.
QK, QXK, QP 8.x, an application transport framework and v6 compatibility remain
outside this work.

RP2040 is now the primary qualification target, using the Earle Philhower
Arduino core family already used by `mouse-joystick-firm`. Exact fixture, core,
platform and toolchain versions are open selections, not recorded results.
Start on a simple non-actuating RP2040 fixture, separate from the working
joystick. Teensy 4.1 builds remain reference/regression evidence; its hardware
gates are deferred and do not block RP2040 work. STM32G0B1CEU6 is the later
second target and will need its own core/BSP/build/target evidence.

Both planned targets are Cortex-M0+ without SWO/ITM. The imported ARMv6-M QV
path is a starting point, not a verified RP2040 port. Review PRIMASK/global
interrupt masking, core ISR/timer ownership and background servicing. Initially
run QP/QS and application drivers on core 0 only; no application core-1 work,
QP calls or shared drivers. PRIMASK is not a lock against another core.

QP is being evaluated for event/state visibility and the ability to explain
instrument behavior. Application adoption depends on measured diagnostic value,
bounded execution and resource cost. Existing non-QP firmware can supply
structured events to the same system. This does not require rewriting
`mouse-joystick-firm` or delaying its delivery.

Keep QWeave-generated and handwritten application logic independent of Arduino
BSP, tick, marker pins and trace transport. Those belong in examples or consuming
applications. `OBS-IF` in `board-interfaces` owns electrical contracts;
`OBS-HOST` in `instrument-control` owns capture, QS decoding, replay,
correlation and deterministic checks.

The selected hardware direction retains passive InstrumentPort-Digital-50 and
a separate optional debug/trace connector, with a reusable external RP2040
probe. The probe MCU is distinct from the DUT MCU. Combined 60-pin observation
and debug wiring, and an onboard second MCU, are deferred. These choices do
not allocate pins or qualify throughput. `OBS-IF` and board owners own the
electrical connection; [OBS-PROBE](../../instrument-control/docs/probe-development-plan.md)
owns external-probe transport and session qualification.

## Steps and dependencies

All steps below are open. Software preparation can run independently of new
boards/connectors and deferred Teensy hardware work. Measured exits require
the named fixture and saved evidence.

| Step | Work and deliverable | Dependencies | Exit evidence |
| --- | --- | --- | --- |
| `OBS-QP-1` | Select a simple RP2040 fixture; pin Earle Philhower core/platform/toolchain; audit ARMv6-M/QV and core-0 ownership; implement BSP/tick/trace and build strict-v7 Blinky, DPP and port-validation examples. | Ready now; active software gates in the migration ledger. | Reproducible build logs, real probe map, timer/ISR/USB/UART servicing analysis, provenance and retained Teensy build regressions; package architecture review remains explicit. |
| `OBS-QP-2` | Execute RP2040 target acceptance: framework coverage, tick/dispatch timing, trace behavior and soak. | `OBS-QP-1`, attached standalone fixture and qualified measurement path; available bench probes/cabling may be used. No Digital-50 or Teensy hardware prerequisite. | Handshake, test results, captures and measurements close only the corresponding active gate entries in the migration ledger. |
| `OBS-QP-3` | Define a non-actuating control-sequence trace pilot: request, dispatch, state change, bounded mock/device operation, completion/cancel/fault. Add application records only where framework records lack meaning. | Design ready now alongside `OBS-QP-1` with `OBS-FW`; agree G0 schema/fixture versions before exchange. | Record dictionary and fixtures identify request/trial, boot/session, ordering and what each event establishes; `OBS-HOST` decodes the fixtures. |
| `OBS-QP-4` | Run that pilot on qualified RP2040 with trace load, slow/disconnected reader, queue pressure, reset and deliberate missing evidence. | `OBS-QP-2`, `OBS-QP-3`, deterministic good/fault/gap checks (G2). | G3 contribution: measured latency, handler duration, queue/pool margins, RAM/flash, throughput/loss and overhead; diagnostic-value decision. Mock completion is labeled as such, not as motor/physical validation. |
| `OBS-QP-5` | Supply real trace and matched marker captures for later command/event/bus/physical correlation. | `OBS-QP-4`, qualified marker/clock contract (`OBS-IF`), host correlation (`OBS-HOST`), physical evidence for the selected application/board (`OBS-HW`). | G4 contribution: replayable evidence bundle, alignment uncertainty and coverage gaps; trace alone cannot prove physical effect. |

G5 (`OBS-RUN`) is separate from port qualification. A later joystick QP slice
still needs its application/board tests after RP2040 qualification; it is not
required to keep delivering the non-QP firmware. Digital-50 hardware and
debug/probe qualification can progress in parallel with this standalone port
fixture. STM32G0B1CEU6 follows the RP2040 baseline as a separately claimed path;
neither shares a validation result merely because both use Cortex-M0+.

## Trace pilot requirements

- Distinguish command arrival, posting, processing, operation start and verified
  completion. Framework events do not automatically carry trial identifiers or
  prove physical response. Agree semantics with `OBS-FW`, without adding them
  to imported framework files.
- Preserve native QS records/dictionaries. Host adaptation associates them with
  the shared evidence model. Specify clock frequency, width, wrap, resets and
  boot/session identity; identify marker edges and quantify drift. USB receive
  time is not an MCU timestamp. Identity beacons are not timing markers.
- Declare comparison configurations and timing/resource budgets before running.
  The current port forces `Q_SPY`; filtered or transport-disabled output is
  not a build with instrumentation compiled out.
- Bound production, drain and flush/error paths. Measure behavior under overload
  and a stopped reader; current example callbacks do not prove nonblocking
  operation. Missing dictionaries, sequence gaps and resets must invalidate
  affected interpretations or yield explicit incomplete results.
- Keep bench QS RX/QUTest distinct from transmit-only experiment observation.
  The existing Teensy validation sketch accepts commands/reset input and is
  not an experiment profile. The RP2040 fixture must test both intended
  profiles separately. No QS command, memory-write or injected-event path is
  exposed during observation-only experiments. SWD halt/program operations are
  controlled bench diagnosis.
- The optional connector may carry qualified UART QS output plus separate
  debug/control functions; those functions remain distinct from passive
  Digital-50 capture. Do not infer UART capacity, extra pins or lossless
  transport from the external probe's use of an RP2040.

## Parallel agent ownership

Claim the step and exact files before implementation. One integrator owns
`AGENTS.md`, `UPGRADE_QPCPP_V7.md`, `README.org`, this plan, `platformio.ini`
and package metadata. Proposed implementation boundaries, not new files from
this documentation change:

| Agent role | Exclusive path scope | Handoff |
| --- | --- | --- |
| RP2040 port validation | New `examples/port_validation-rp2040/`, RP2040 build helper; any `src/qp-arduino/` or provenance-script change requires explicit claim | Minimal documented deltas, exact versions/builds, core audit and active gate evidence to integrator. |
| Trace pilot | New `examples/observability_pilot-rp2040/`, with BSP/transport confined there | Sample QS stream, dictionary, configuration, fixtures and measured budgets to `OBS-HOST`/`OBS-FW`. |
| Teensy regression maintenance | Existing Teensy examples and `scripts/build_teensy_examples.sh`, when claimed | Preserve cross-build evidence; no implicit restart of deferred hardware work. |
| Bench operator | One assigned target/analyzer/probe/cable and evidence bundle at a time | Immutable capture IDs and measurements; no concurrent uploads/captures on the same setup. |

The pilot agent can prepare semantics and fixtures while the port agent works.
Both coordinate shared build changes through the integrator. Upstream `qpcpp`
and `qpcpp-examples` remain reference inputs. No `kicad-libs` changes are part
of the current documentation task.

## Handoff and validation

Record step/owner/commit; fixture/serial/revision and pin map; exact core,
platform, compiler and upstream versions; core-0/runtime assumptions;
configuration/dictionaries/filters; artifact locations/hashes; commands/results
with units; failed/unrun checks; clock/loss limits; and the next consumer.
Keep the gate ledger in `UPGRADE_QPCPP_V7.md`. Board-specific measurements stay
in the board's bring-up log; the standalone fixture records its own identity.

For this documentation change, check links, target/dependency consistency and
`git diff --check`; no build/hardware result is implied. Future port/framework
changes require `scripts/check_upstream_provenance.sh`, relevant RP2040 builds,
retained Teensy build regressions and updated delta records. Target claims
require the actual active tests and measurements in the gate ledger.
