#!/usr/bin/env bash
# Thin wrapper kept for the documented command name. The check itself lives in
# tools/check_upstream_provenance.py so it runs the same on Linux, macOS and
# Windows (`pixi run provenance`). QPCPP_DIR and QPCPP_TAG still work.
set -euo pipefail
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python="${PYTHON:-python3}"
command -v "$python" >/dev/null 2>&1 || python=python
exec "$python" "$repo_root/tools/check_upstream_provenance.py" "$@"
