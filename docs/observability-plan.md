# QP observability development plan

Status: planned work, 2026-09-29. This document records the `OBS-QP`
workstream; it does not establish new target support or pass a validation gate.
The shared [architecture](../../orb-weaver-context/context/experimental-system-observability.md)
owns cross-project intent, and the shared
[roadmap](../../orb-weaver-context/context/experimental-system-observability-plan.md)
owns stage dependencies and integration status. Stage IDs `G0`–`G5` refer to
that roadmap, not to this repository's migration acceptance criteria.

## Scope and authority

[UPGRADE_QPCPP_V7.md](../UPGRADE_QPCPP_V7.md) remains authoritative for the port,
exact upstream provenance, software results, and outstanding hardware gates.
The target remains QP/C++ `v7.3.4`, QV only, with Teensy 4.1 the required
validation target and `<qpcpp.hpp>` the public entry point. QK, QXK, QP 8.x,
an application transport framework, and v6 compatibility are outside this work.

QP is being evaluated for its event/state visibility and ability to explain
instrument behavior. Adoption in an application depends on measured diagnostic
value, bounded execution, and resource cost. Existing non-QP firmware can supply
structured events to the same observation system. This work does not require
rewriting `mouse-joystick-firm` or delaying its delivery.

Application logic, including QWeave-generated logic, must remain independent of
Arduino BSP, tick, marker-pin, and trace-transport details. Those belong in
examples or the consuming application. Electrical contracts belong to
`OBS-IF` in `board-interfaces`; capture, QS decoding, replay, correlation, and
continuous deterministic checks belong to `OBS-HOST` in `instrument-control`.

## Steps and dependencies

All steps below are open. Preparation can proceed while hardware is unavailable;
measured exits require the named target and saved evidence.

| Step | Work and deliverable | Dependencies | Exit evidence |
| --- | --- | --- | --- |
| `OBS-QP-1` | Prepare a reproducible Teensy validation run: inventory existing fixture coverage, fill missing host test procedures, record build/tool versions and capture settings. | Ready now; existing migration plan. | Reviewed run procedure maps each migration acceptance criterion to an actual check; missing checks stay explicitly open. |
| `OBS-QP-2` | Execute the existing port hardware acceptance work and diagnose failures. | `OBS-QP-1`, attached Teensy 4.1 and a qualified measurement path for that fixture. This may use existing probes/cabling; it does not wait for Digital-50 or a new DUT board. | Target handshake, test results, timing captures and soak evidence recorded against the existing migration ledger. All of that ledger's criteria must pass before a validation/release claim. |
| `OBS-QP-3` | Define a small representative control-sequence trace pilot: request, dispatch, state change, bounded device operation, completion/cancel/fault. Add application records only where framework records lack meaning. | Design ready now with `OBS-FW`; interface/schema version agreed through G0 before fixture exchange. | Versioned record dictionary and fixtures identify request/trial, boot/session, expected ordering, and what each event actually establishes. Host decoder accepts the fixture (`OBS-HOST`). |
| `OBS-QP-4` | Run that pilot on Teensy with trace load, slow/disconnected reader, queue pressure, reset, and deliberate missing evidence. | `OBS-QP-2`, `OBS-QP-3`, deterministic good/fault/gap fixture checks (G2). | G3 contribution: measured response latency, handler duration, queue/pool margins, RAM/flash, trace throughput/loss and observer overhead; a decision records whether QP's added visibility justifies its cost. |
| `OBS-QP-5` | Hand off real trace and matched marker captures for command/event/bus/physical correlation. | `OBS-QP-4`, qualified marker/clock contract (`OBS-IF`), host correlation (`OBS-HOST`), physical evidence from `OBS-HW`. | G4 contribution: replayable evidence bundle, alignment uncertainty and known coverage gaps; no physical-effect claim from trace alone. |

The experiment pilot (G5, `OBS-RUN`) consumes these results. It is separate
from port qualification. An RP2040 application pilot requires its own explicit
port/BSP/build/target acceptance plan and evidence after the Teensy baseline;
Teensy results confer no RP2040 or general Arduino support claim.

## Trace pilot requirements

- Distinguish command arrival, event posting, event processing, operation start,
  and verified completion. Framework events do not automatically carry trial
  identifiers or prove a physical response. Agree application semantics with
  `OBS-FW`, without adding those semantics to imported framework files.
- Preserve native QS records and dictionaries. Host adaptation can associate
  them with the shared evidence model. Specify clock frequency, width, wrap,
  resets and boot/session identity; identify marker edges and quantify drift.
  USB receive time is not an MCU timestamp. Identity beacons do not substitute
  for an agreed timing marker.
- Measure filtering and transport configurations explicitly. The current port
  forces `Q_SPY`; a transport-disabled or filtered run is not a build with all
  instrumentation compiled out. Declare comparison profiles and timing/resource
  budgets before measuring them.
- Bound normal trace production and draining; characterize loss under overload
  and when the host stops reading. Audit flush/error paths too. Current example
  callbacks are not evidence that transport is nonblocking in every path.
  Missing dictionaries, sequence gaps and resets must invalidate affected
  interpretations or yield an explicit incomplete result.
- Keep the bench QS RX/QUTest profile distinct from an experiment profile that
  only transmits trace. The current port-validation example receives commands
  and reset requests. An observation-only build/setup must not expose those
  actions or QS memory-write/event-injection input during an experiment. SWD
  halt/program operations are controlled bench diagnosis, not passive evidence.

## Parallel agent ownership

Claim the step and exact files before implementation. One integrator owns
`UPGRADE_QPCPP_V7.md`, `README.org`, this plan, and shared build configuration.
The following are proposed implementation boundaries, not files created by this
planning change:

| Agent role | Exclusive path scope | Handoff |
| --- | --- | --- |
| Port validation | `examples/port_validation-teensy4/`, `scripts/check_upstream_provenance.sh`, `scripts/build_teensy_examples.sh`; any `src/qp-arduino/` fix requires explicit claim | Minimal documented port delta and migration-gate evidence to integrator. |
| Trace pilot | A new `examples/observability_pilot-teensy4/` subtree, with BSP/transport confined there | Sample QS stream, dictionary, build configuration, measured budgets and fixtures to `OBS-HOST`/`OBS-FW`. |
| Bench operator | One assigned target/analyzer/cable and evidence bundle at a time | Immutable capture IDs and measurements to validation/pilot owner; do not run concurrent uploads or captures on the same setup. |

The trace pilot agent can prepare fixtures while the port agent works, but
both must agree any build-configuration changes through the integrator.
Upstream `qpcpp`/`qpcpp-examples` checkouts remain reference inputs. No changes
to `kicad-libs` are included in this initiative's current documentation task.

## Handoff and validation

For each completed step record: owner and commit; exact target/board serial;
upstream/tool versions; configuration/dictionaries/filters; evidence locations
and hashes; commands and measured results with units; failed or unrun checks;
clock/loss limitations; and the next consumer/step. Keep one hardware gate
ledger in `UPGRADE_QPCPP_V7.md`, linking evidence rather than copying its status
here. Board-specific measurements stay in the owning board's bring-up log.

For this documentation change, check links and `git diff --check`; no new build
or hardware result is implied. Future framework or port changes require
`scripts/check_upstream_provenance.sh`, the applicable build checks, and an
updated delta record. Hardware results require the migration plan's actual
target tests and measurements.
