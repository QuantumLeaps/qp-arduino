# qp-arduino Agent Guidance

Read `UPGRADE_QPCPP_V7.md` before changing the v7 migration branch. That
document and the current branch are authoritative. Material under
`orb-weaver-ai/context/qp/` is historical source context, not live project
instruction; extract useful constraints without replacing decisions already
made and implemented here.

## Scope

- Target upstream QP/C++ tag `v7.3.4` and the QV cooperative kernel only.
- Do not add QK, QXK, QP 8.x support, or an application transport framework.
- Preserve the public `#include <qpcpp.hpp>` entry point.
- Keep QWeave-generated and handwritten application code independent of
  Arduino BSP, tick, and trace-transport details.

## Port Discipline

- Import upstream files first; keep every Arduino-local change minimal and
  documented in `UPGRADE_QPCPP_V7.md`.
- Run `scripts/check_upstream_provenance.sh` after framework or port changes.
- Keep board-specific timer, BSP, and trace hookup in examples or applications.
- RP2040 is the primary qualification target as of 2026-09-30, using the
  Earle Philhower Arduino core family already used by `mouse-joystick-firm`.
  Exact fixture/core/platform/toolchain versions must be selected and recorded.
  Start on a simple non-actuating fixture, separate from the working joystick.
- Preserve Teensy 4.1 build/reference regressions. Its hardware gates are
  explicitly deferred and do not block RP2040 qualification. STM32G0B1CEU6 is
  the later second target; retained SAM examples are not a support claim.
- Do not broaden board or architecture claims without build and target
  evidence. Imported ARMv6-M QV code is a starting point, not an RP2040 port
  validation. Review PRIMASK/global interrupt masking on Cortex-M0+, not
  BASEPRI; initially keep QP/QS and application drivers on core 0, with no
  application core-1 work, QP calls or shared drivers.
- Neither RP2040 nor STM32G0B1CEU6 provides SWO/ITM. Keep UART/QS transport
  decisions in the BSP/application and the separate optional debug/trace
  contract; do not add control paths to passive InstrumentPort-Digital-50.
- The migration branch intentionally defaults `QP_API_VERSION` to `9999`.
  Reintroducing v6 compatibility requires an explicit decision and dedicated
  tests.

## Completion Discipline

- Do not describe the active RP2040 target as released or validated while its
  software and hardware criteria in `UPGRADE_QPCPP_V7.md` remain unmet. Report
  deferred Teensy hardware work separately; it is not an RP2040 release gate
  and must not be reported as passed.
- Record exact upstream tags and commits, intentional deltas, build logs, and
  hardware evidence.
- Reconcile package metadata and license-file provenance with the imported
  upstream tag before release.
- Keep `UPGRADE_QPCPP_V7.md` current as gates pass or decisions change.
