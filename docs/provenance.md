# Provenance of the framework files

qp-arduino is a repackaging of QP/C++, not a fork of it. This document says
exactly how the files under `src/` relate to the upstream qpcpp tag, which
local deltas exist and why, and how to verify both.

## Upstream source

The 7.3.4 port was created from qpcpp at tag `v7.3.4`
(`10441f7a03fa956b9e4772a073a05a96487983e2`). The Arduino package keeps the
framework files flat under `src/qp-arduino` so Arduino builders can compile the
library without the upstream directory layout. The imported set is:

- the public and internal headers from qpcpp `include/`;
- the QF and QEP sources from `src/qf/`;
- the QS sources from `src/qs/`;
- the QV kernel from `src/qv/`;
- the ARM Cortex-M QV GNU port files from `ports/arm-cm/qv/gnu/`.

QK, QXK and the QUTest runtime are not imported, so a sketch never compiles an
unused kernel or test-only code.

The upstream `v7.3.4` tag ships generated headers that still report
`QP_VERSION_STR "7.3.3"` and `QP_VERSION 733U`. They are imported unchanged;
the version a build was made from is identified by the tag, never by that
string.

## Local deltas

Every difference from the upstream files is intentional and listed here.

- `src/qpcpp.hpp`: the public Arduino include. It keeps the historical name,
  rewrites the includes to the flat `src/qp-arduino` paths, and defaults
  `QP_API_VERSION` to `9999` for the strict v7 API.
- `src/qp-arduino/qp_port.hpp`: includes `Arduino.h`, forces `Q_SPY`, keeps
  the low-RAM Arduino defaults (`QF_MAX_ACTIVE` 8 on ARMv6-M and 16 otherwise,
  `QF_MAX_TICK_RATE` 2), and keeps the upstream `QF_BASEPRI` value `0x3F` for
  ARMv7-M.
- `src/qp-arduino/qs_port.hpp`: differs from upstream only by one removed
  trailing blank line.
- `src/qp-arduino/qv_port.cpp`: wraps the upstream `SCB_CPACR` and
  `FPU_FPCCR` register aliases in `#ifndef` guards so the Arduino core's
  definitions are reused instead of producing redefinition warnings, and
  removes one trailing blank line.
- `src/qp-arduino/library.json`: a PlatformIO-only manifest, not an upstream
  file. The repository's own builds link the library with
  `symlink://src/qp-arduino` so edits compile without a copy step, and
  PlatformIO requires a manifest for a linked library. Arduino builds read
  `library.properties` at the repository root instead.
- `LICENSE`, `QP-Arduino_GPL_Exception.txt` and the four files under
  `LICENSES/` are byte-identical to the texts in the upstream tag. The
  exception text prohibits changes, so the upstream text is packaged verbatim
  even though it also names QP products this package does not contain.

Everything else in the repository (examples, tools, `platformio.ini`,
`pixi.toml`, documentation) is Arduino-specific and has no upstream
counterpart.

## Verifying

```sh
pixi run provenance                      # or: python tools/check_upstream_provenance.py
```

The check reads the tagged files with `git show` from a qpcpp checkout
(`../qpcpp` by default; override with `QPCPP_DIR` and `QPCPP_TAG`), compares
every imported file byte for byte, allows exactly the deltas above in the
three port files, verifies the six license texts, and fails on anything else:
a wrong tag, a modified import, an edit outside the documented deltas, a stale
v6 file, or a missing port setting. `scripts/check_upstream_provenance.sh` is a
thin wrapper for shells.

Run it after any change under `src/`, and before every release.
