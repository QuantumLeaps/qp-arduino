#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

pio_bin="${PIO:-}"
if [[ -z "$pio_bin" ]]; then
    if command -v pio >/dev/null 2>&1; then
        pio_bin="pio"
    elif [[ -x "$repo_root/.venv/bin/pio" ]]; then
        pio_bin="$repo_root/.venv/bin/pio"
    else
        echo "PlatformIO is not on PATH and .venv/bin/pio was not found." >&2
        echo "Install PlatformIO or set PIO=/path/to/pio." >&2
        exit 127
    fi
fi

examples=(
    examples/blinky_bsp-teensy4
    examples/dpp_bsp-teensy4
    examples/port_validation-teensy4
)

for example in "${examples[@]}"; do
    if [[ ! -d "$example" ]]; then
        echo "missing required example: $example" >&2
        exit 1
    fi
    echo "building $example"
    rm -rf \
        "$repo_root/.pio/libdeps/teensy41/qp-arduino" \
        "$repo_root/.pio/libdeps/teensy41/qp-arduino.pio-link"
    tmp_conf="$(mktemp)"
    awk -v src_dir="$example" '
        /^[[:space:]]*src_dir[[:space:]]*=/ {
            print "src_dir = " src_dir
            next
        }
        { print }
    ' platformio.ini > "$tmp_conf"
    if ! PLATFORMIO_CORE_DIR="$repo_root/.pio-core" \
        PLATFORMIO_SETTING_ENABLE_TELEMETRY=no \
            "$pio_bin" run -e teensy41 \
        --project-dir "$repo_root" \
        --project-conf "$tmp_conf"; then
        rm -f "$tmp_conf"
        exit 1
    fi
    rm -f "$tmp_conf"
done
