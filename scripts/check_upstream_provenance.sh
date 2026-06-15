#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
qpcpp_dir="${QPCPP_DIR:-"$repo_root/../qpcpp"}"
qpcpp_tag="${QPCPP_TAG:-v7.3.4}"

cd "$repo_root"

if ! git -C "$qpcpp_dir" rev-parse --verify --quiet "$qpcpp_tag" >/dev/null; then
    echo "missing qpcpp tag $qpcpp_tag in $qpcpp_dir" >&2
    exit 2
fi

obsolete=(
    src/qp-arduino/qassert.h
    src/qp-arduino/qep.hpp
    src/qp-arduino/qep_port.hpp
    src/qp-arduino/qf.hpp
    src/qp-arduino/qf_pkg.hpp
    src/qp-arduino/qf_port.hpp
    src/qp-arduino/qpset.hpp
    src/qp-arduino/qv_port.hpp
)

for path in "${obsolete[@]}"; do
    if [[ -e "$path" ]]; then
        echo "obsolete v6 file still present: $path" >&2
        exit 1
    fi
done

exact=(
    include/qequeue.hpp:src/qp-arduino/qequeue.hpp
    include/qmpool.hpp:src/qp-arduino/qmpool.hpp
    include/qp.hpp:src/qp-arduino/qp.hpp
    include/qp_pkg.hpp:src/qp-arduino/qp_pkg.hpp
    include/qs.hpp:src/qp-arduino/qs.hpp
    include/qs_dummy.hpp:src/qp-arduino/qs_dummy.hpp
    include/qs_pkg.hpp:src/qp-arduino/qs_pkg.hpp
    include/qsafe.h:src/qp-arduino/qsafe.h
    include/qstamp.hpp:src/qp-arduino/qstamp.hpp
    include/qv.hpp:src/qp-arduino/qv.hpp
    src/qf/qep_hsm.cpp:src/qp-arduino/qep_hsm.cpp
    src/qf/qep_msm.cpp:src/qp-arduino/qep_msm.cpp
    src/qf/qf_act.cpp:src/qp-arduino/qf_act.cpp
    src/qf/qf_actq.cpp:src/qp-arduino/qf_actq.cpp
    src/qf/qf_defer.cpp:src/qp-arduino/qf_defer.cpp
    src/qf/qf_dyn.cpp:src/qp-arduino/qf_dyn.cpp
    src/qf/qf_mem.cpp:src/qp-arduino/qf_mem.cpp
    src/qf/qf_ps.cpp:src/qp-arduino/qf_ps.cpp
    src/qf/qf_qact.cpp:src/qp-arduino/qf_qact.cpp
    src/qf/qf_qeq.cpp:src/qp-arduino/qf_qeq.cpp
    src/qf/qf_qmact.cpp:src/qp-arduino/qf_qmact.cpp
    src/qf/qf_time.cpp:src/qp-arduino/qf_time.cpp
    src/qs/qs.cpp:src/qp-arduino/qs.cpp
    src/qs/qs_64bit.cpp:src/qp-arduino/qs_64bit.cpp
    src/qs/qs_fp.cpp:src/qp-arduino/qs_fp.cpp
    src/qs/qs_rx.cpp:src/qp-arduino/qs_rx.cpp
    src/qs/qstamp.cpp:src/qp-arduino/qstamp.cpp
    src/qv/qv.cpp:src/qp-arduino/qv.cpp
)

for mapping in "${exact[@]}"; do
    upstream="${mapping%%:*}"
    local_path="${mapping#*:}"
    if ! git -C "$qpcpp_dir" show "$qpcpp_tag:$upstream" | diff -u - "$local_path" >/dev/null; then
        echo "local file differs from qpcpp $qpcpp_tag: $local_path" >&2
        echo "upstream path: $upstream" >&2
        exit 1
    fi
done

# qpcpp v7.3.4's qs_port.hpp has a trailing blank line at EOF, which fails
# git diff --check. The Arduino copy only removes that final blank line.
if ! git -C "$qpcpp_dir" show "$qpcpp_tag:ports/arm-cm/qv/gnu/qs_port.hpp" \
    | sed '${/^$/d;}' \
    | diff -u - src/qp-arduino/qs_port.hpp >/dev/null; then
    echo "local file differs from qpcpp $qpcpp_tag beyond EOF cleanup: src/qp-arduino/qs_port.hpp" >&2
    echo "upstream path: ports/arm-cm/qv/gnu/qs_port.hpp" >&2
    exit 1
fi

# The Arduino/Teensy core already defines SCB_CPACR. Guard the upstream register
# aliases to avoid macro redefinition warnings while retaining upstream fallback
# values for cores that do not provide them.
if ! git -C "$qpcpp_dir" show "$qpcpp_tag:ports/arm-cm/qv/gnu/qv_port.cpp" \
    | awk '
        /^#define SCB_CPACR/ {
            print "#ifndef SCB_CPACR"
            print
            print "#endif"
            next
        }
        /^#define FPU_FPCCR/ {
            print "#ifndef FPU_FPCCR"
            print
            print "#endif"
            next
        }
        { print }
    ' \
    | sed '${/^$/d;}' \
    | diff -u - src/qp-arduino/qv_port.cpp >/dev/null; then
    echo "local file differs from qpcpp $qpcpp_tag beyond Arduino register guards: src/qp-arduino/qv_port.cpp" >&2
    echo "upstream path: ports/arm-cm/qv/gnu/qv_port.cpp" >&2
    exit 1
fi

grep -q '#define QP_API_VERSION 9999' src/qpcpp.hpp
grep -q '#include "./qp-arduino/qp_port.hpp"' src/qpcpp.hpp
grep -q '#define Q_SPY  1U' src/qp-arduino/qp_port.hpp
grep -q '#include "Arduino.h"' src/qp-arduino/qp_port.hpp
grep -q '#define QF_MAX_TICK_RATE       2U' src/qp-arduino/qp_port.hpp
grep -q '#define QF_MAX_ACTIVE       8U' src/qp-arduino/qp_port.hpp
grep -q '#define QF_MAX_ACTIVE       16U' src/qp-arduino/qp_port.hpp

echo "qpcpp $qpcpp_tag provenance check passed"
