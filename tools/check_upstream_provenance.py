#!/usr/bin/env python3
"""Verify that qp-arduino's imported framework files still match upstream qpcpp.

This is the single implementation of the provenance check;
``scripts/check_upstream_provenance.sh`` just calls it. It needs only Python 3
and ``git`` and runs the same on Linux, macOS and Windows (``pixi run
provenance``).

What it checks, in this order:

1. no obsolete v6 files remain under ``src/qp-arduino``;
2. every directly imported header/source is byte-identical to the file at the
   upstream tag (read with ``git show``, so the upstream checkout is never
   touched);
3. ``qs_port.hpp`` differs from upstream only by the removal of one trailing
   blank line;
4. ``qv_port.cpp`` differs from upstream only by ``#ifndef`` guards around the
   ``SCB_CPACR``/``FPU_FPCCR`` register aliases plus that same EOF cleanup;
5. the Arduino-local port settings that ``docs/provenance.md`` documents are
   still present.

Exit codes: 0 pass, 1 mismatch, 2 upstream tag not found.

Environment overrides: ``QPCPP_DIR`` (default ``../qpcpp`` next to this
repository) and ``QPCPP_TAG`` (default ``v7.3.4``). Command-line options take
precedence over the environment.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path

DEFAULT_TAG = "v7.3.4"

OBSOLETE = [
    "src/qp-arduino/qassert.h",
    "src/qp-arduino/qep.hpp",
    "src/qp-arduino/qep_port.hpp",
    "src/qp-arduino/qf.hpp",
    "src/qp-arduino/qf_pkg.hpp",
    "src/qp-arduino/qf_port.hpp",
    "src/qp-arduino/qpset.hpp",
    "src/qp-arduino/qv_port.hpp",
]

# (upstream path inside qpcpp, local path inside this repository)
EXACT = [
    ("include/qequeue.hpp", "src/qp-arduino/qequeue.hpp"),
    ("include/qmpool.hpp", "src/qp-arduino/qmpool.hpp"),
    ("include/qp.hpp", "src/qp-arduino/qp.hpp"),
    ("include/qp_pkg.hpp", "src/qp-arduino/qp_pkg.hpp"),
    ("include/qs.hpp", "src/qp-arduino/qs.hpp"),
    ("include/qs_dummy.hpp", "src/qp-arduino/qs_dummy.hpp"),
    ("include/qs_pkg.hpp", "src/qp-arduino/qs_pkg.hpp"),
    ("include/qsafe.h", "src/qp-arduino/qsafe.h"),
    ("include/qstamp.hpp", "src/qp-arduino/qstamp.hpp"),
    ("include/qv.hpp", "src/qp-arduino/qv.hpp"),
    ("src/qf/qep_hsm.cpp", "src/qp-arduino/qep_hsm.cpp"),
    ("src/qf/qep_msm.cpp", "src/qp-arduino/qep_msm.cpp"),
    ("src/qf/qf_act.cpp", "src/qp-arduino/qf_act.cpp"),
    ("src/qf/qf_actq.cpp", "src/qp-arduino/qf_actq.cpp"),
    ("src/qf/qf_defer.cpp", "src/qp-arduino/qf_defer.cpp"),
    ("src/qf/qf_dyn.cpp", "src/qp-arduino/qf_dyn.cpp"),
    ("src/qf/qf_mem.cpp", "src/qp-arduino/qf_mem.cpp"),
    ("src/qf/qf_ps.cpp", "src/qp-arduino/qf_ps.cpp"),
    ("src/qf/qf_qact.cpp", "src/qp-arduino/qf_qact.cpp"),
    ("src/qf/qf_qeq.cpp", "src/qp-arduino/qf_qeq.cpp"),
    ("src/qf/qf_qmact.cpp", "src/qp-arduino/qf_qmact.cpp"),
    ("src/qf/qf_time.cpp", "src/qp-arduino/qf_time.cpp"),
    ("src/qs/qs.cpp", "src/qp-arduino/qs.cpp"),
    ("src/qs/qs_64bit.cpp", "src/qp-arduino/qs_64bit.cpp"),
    ("src/qs/qs_fp.cpp", "src/qp-arduino/qs_fp.cpp"),
    ("src/qs/qs_rx.cpp", "src/qp-arduino/qs_rx.cpp"),
    ("src/qs/qstamp.cpp", "src/qp-arduino/qstamp.cpp"),
    ("src/qv/qv.cpp", "src/qp-arduino/qv.cpp"),
    # license texts are packaged verbatim from the imported tag
    ("LICENSES/GPL-3.0-or-later.txt", "LICENSE"),
    ("LICENSES/GPL-3.0-or-later.txt", "LICENSES/GPL-3.0-or-later.txt"),
    ("LICENSES/QP-Arduino_GPL_Exception.txt", "QP-Arduino_GPL_Exception.txt"),
    ("LICENSES/QP-Arduino_GPL_Exception.txt", "LICENSES/QP-Arduino_GPL_Exception.txt"),
    ("LICENSES/LicenseRef-QL-commercial.txt", "LICENSES/LicenseRef-QL-commercial.txt"),
    ("LICENSES/License.txt", "LICENSES/License.txt"),
]

QS_PORT_UPSTREAM = "ports/arm-cm/qv/gnu/qs_port.hpp"
QS_PORT_LOCAL = "src/qp-arduino/qs_port.hpp"
QV_PORT_UPSTREAM = "ports/arm-cm/qv/gnu/qv_port.cpp"
QV_PORT_LOCAL = "src/qp-arduino/qv_port.cpp"

# (local file, required substring)
REQUIRED_TEXT = [
    ("src/qpcpp.hpp", "#define QP_API_VERSION 9999"),
    ("src/qpcpp.hpp", '#include "./qp-arduino/qp_port.hpp"'),
    ("src/qp-arduino/qp_port.hpp", "#define Q_SPY  1U"),
    ("src/qp-arduino/qp_port.hpp", '#include "Arduino.h"'),
    ("src/qp-arduino/qp_port.hpp", "#define QF_MAX_TICK_RATE       2U"),
    ("src/qp-arduino/qp_port.hpp", "#define QF_MAX_ACTIVE       8U"),
    ("src/qp-arduino/qp_port.hpp", "#define QF_MAX_ACTIVE       16U"),
]


def fail(message: str, code: int = 1) -> int:
    print(message, file=sys.stderr)
    return code


def git_show(qpcpp_dir: Path, tag: str, upstream_path: str) -> bytes:
    result = subprocess.run(
        ["git", "-C", str(qpcpp_dir), "show", f"{tag}:{upstream_path}"],
        check=True,
        capture_output=True,
    )
    return result.stdout


def tag_exists(qpcpp_dir: Path, tag: str) -> bool:
    result = subprocess.run(
        ["git", "-C", str(qpcpp_dir), "rev-parse", "--verify", "--quiet", tag],
        capture_output=True,
    )
    return result.returncode == 0


def read_local(path: Path) -> bytes:
    data = path.read_bytes()
    if os.name == "nt":
        # A Windows checkout with core.autocrlf converts line endings on the
        # way out of git; that is not a provenance difference.
        data = data.replace(b"\r\n", b"\n")
    return data


def drop_trailing_blank_line(data: bytes) -> bytes:
    """Equivalent of ``sed '${/^$/d;}'``: delete the last line if it is empty."""
    if data.endswith(b"\n\n"):
        return data[:-1]
    return data


def guard_register_aliases(data: bytes) -> bytes:
    """Wrap the SCB_CPACR/FPU_FPCCR aliases in #ifndef guards, like the Arduino copy."""
    out: list[bytes] = []
    for line in data.split(b"\n"):
        for name in (b"SCB_CPACR", b"FPU_FPCCR"):
            if line.startswith(b"#define " + name):
                out.extend([b"#ifndef " + name, line, b"#endif"])
                break
        else:
            out.append(line)
    return b"\n".join(out)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    default_root = Path(__file__).resolve().parents[1]
    parser.add_argument("--repo-root", type=Path, default=default_root,
                        help="qp-arduino checkout to check (default: this repository)")
    parser.add_argument("--qpcpp-dir", type=Path,
                        default=Path(os.environ.get("QPCPP_DIR", "")) if os.environ.get("QPCPP_DIR") else None,
                        help="upstream qpcpp git checkout (default: $QPCPP_DIR or ../qpcpp)")
    parser.add_argument("--tag", default=os.environ.get("QPCPP_TAG", DEFAULT_TAG),
                        help=f"upstream qpcpp tag (default: $QPCPP_TAG or {DEFAULT_TAG})")
    args = parser.parse_args(argv)

    root = args.repo_root.resolve()
    qpcpp_dir = (args.qpcpp_dir or (root.parent / "qpcpp")).resolve()
    tag = args.tag

    if not tag_exists(qpcpp_dir, tag):
        return fail(f"missing qpcpp tag {tag} in {qpcpp_dir}", 2)

    for rel in OBSOLETE:
        if (root / rel).exists():
            return fail(f"obsolete v6 file still present: {rel}")

    for upstream, local in EXACT:
        if git_show(qpcpp_dir, tag, upstream) != read_local(root / local):
            return fail(f"local file differs from qpcpp {tag}: {local}\nupstream path: {upstream}")

    expected = drop_trailing_blank_line(git_show(qpcpp_dir, tag, QS_PORT_UPSTREAM))
    if expected != read_local(root / QS_PORT_LOCAL):
        return fail(f"local file differs from qpcpp {tag} beyond EOF cleanup: {QS_PORT_LOCAL}\n"
                    f"upstream path: {QS_PORT_UPSTREAM}")

    expected = drop_trailing_blank_line(guard_register_aliases(git_show(qpcpp_dir, tag, QV_PORT_UPSTREAM)))
    if expected != read_local(root / QV_PORT_LOCAL):
        return fail(f"local file differs from qpcpp {tag} beyond Arduino register guards: {QV_PORT_LOCAL}\n"
                    f"upstream path: {QV_PORT_UPSTREAM}")

    for local, needle in REQUIRED_TEXT:
        if needle.encode() not in read_local(root / local):
            return fail(f"required port setting missing from {local}: {needle}")

    print(f"qpcpp {tag} provenance check passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
