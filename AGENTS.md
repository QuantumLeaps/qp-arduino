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
- Do not broaden board or architecture claims without build and target
  evidence. Teensy 4.1 is the required validation target; the retained SAM
  examples are not a support claim.
- The migration branch intentionally defaults `QP_API_VERSION` to `9999`.
  Reintroducing v6 compatibility requires an explicit decision and dedicated
  tests.

## Completion Discipline

- Do not describe the branch as released or validated while the acceptance
  criteria in `UPGRADE_QPCPP_V7.md` remain unmet.
- Record exact upstream tags and commits, intentional deltas, build logs, and
  hardware evidence.
- Reconcile package metadata and license-file provenance with the imported
  upstream tag before release.
- Keep `UPGRADE_QPCPP_V7.md` current as gates pass or decisions change.
