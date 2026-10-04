#!/usr/bin/env python3
"""PlatformIO task wrapper used by the Pixi tasks (and usable on its own).

Why this exists:

- Selecting which example to build normally means editing ``platformio.ini``
  (``src_dir = examples/...``). PlatformIO lets ``PLATFORMIO_SRC_DIR`` and
  ``PLATFORMIO_BUILD_DIR`` override that, and this script sets them the same
  way on Windows, macOS and Linux.
- Each example builds in its own directory under ``.pio/build-examples`` so
  switching examples never invalidates another example's cache. It is not
  under ``.pio/build`` because a plain ``pio run`` deletes every directory
  there whose name is not an environment in ``platformio.ini``.
- PlatformIO's own state (platforms, toolchains, frameworks) is kept inside the
  repository in ``.pio-core`` so nothing depends on a writable ``~/.platformio``.
  Set ``PLATFORMIO_CORE_DIR`` yourself to share one core directory instead.

Typical use::

    python tools/pio_task.py build  --example examples/blinky_bsp-rp2040 --env pico
    python tools/pio_task.py upload --example examples/blinky_bsp-rp2040 --env pico --port /dev/ttyACM0
    python tools/pio_task.py monitor --port /dev/ttyACM0

Extra arguments after the known options are passed through to ``pio``.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

DEFAULT_EXAMPLE = "examples/dpp_bsp-teensy4"
DEFAULT_ENV = "teensy41"


def _project_root() -> Path:
    env_root = os.environ.get("PIXI_PROJECT_ROOT")
    if env_root:
        return Path(env_root).resolve()
    return Path(__file__).resolve().parents[1]


def _rel(p: str) -> Path:
    # Accept backslashes from Windows shells.
    return Path(p.replace("\\", "/"))


def _example_src_dir(root: Path, example: str) -> Path:
    src_dir = (root / _rel(example)).resolve()
    if not src_dir.is_dir():
        raise SystemExit(f"example directory does not exist: {example}")
    return src_dir


def _example_build_dir(root: Path, example: str) -> Path:
    return (root / ".pio" / "build-examples" / _rel(example)).resolve()


def _run(cmd: list[str], *, cwd: Path, env: dict[str, str]) -> int:
    print("+ " + " ".join(cmd), flush=True)
    try:
        return int(subprocess.run(cmd, cwd=str(cwd), env=env, check=False).returncode)
    except FileNotFoundError:
        print("ERROR: 'pio' not found. Run `pixi install`, or activate the .venv "
              "described in README.md, or set PIO=/path/to/pio.", file=sys.stderr)
        return 127


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="pio_task.py", description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)

    def with_example_env(sp: argparse.ArgumentParser) -> None:
        sp.add_argument("--example", default=DEFAULT_EXAMPLE,
                        help=f"repository-relative example directory (default: {DEFAULT_EXAMPLE})")
        sp.add_argument("--env", default=DEFAULT_ENV,
                        help=f"PlatformIO environment name (default: {DEFAULT_ENV})")

    sp = sub.add_parser("build", help="compile an example")
    with_example_env(sp)
    sp = sub.add_parser("clean", help="clean an example's build products")
    with_example_env(sp)
    sp = sub.add_parser("rebuild", help="clean, then compile")
    with_example_env(sp)
    sp = sub.add_parser("upload", help="compile and upload an example")
    with_example_env(sp)
    sp.add_argument("--port", default=None, help="upload port (/dev/ttyACM0, COM3, ...); auto-detected if omitted")
    sp = sub.add_parser("flash", help="clean, compile and upload")
    with_example_env(sp)
    sp.add_argument("--port", default=None, help="upload port; auto-detected if omitted")
    sp = sub.add_parser("deepclean", help="delete an example's isolated build directory")
    sp.add_argument("--example", default=DEFAULT_EXAMPLE)
    sp = sub.add_parser("monitor", help="open the serial monitor")
    sp.add_argument("--port", default=None, help="serial port; auto-detected if omitted")
    sp.add_argument("--baud", default="115200")

    args, passthrough = parser.parse_known_args(argv)

    root = _project_root()
    env = os.environ.copy()
    env.setdefault("PLATFORMIO_CORE_DIR", str(root / ".pio-core"))
    env.setdefault("PLATFORMIO_SETTING_ENABLE_TELEMETRY", "no")
    pio = env.get("PIO", "pio")

    if args.command in ("build", "clean", "rebuild", "upload", "flash"):
        env["PLATFORMIO_SRC_DIR"] = str(_example_src_dir(root, args.example))
        env["PLATFORMIO_BUILD_DIR"] = str(_example_build_dir(root, args.example))

    def run_target(target: str | None) -> int:
        cmd = [pio, "run", "-e", args.env, "--project-dir", str(root)]
        if target:
            cmd += ["-t", target]
        if target == "upload" and getattr(args, "port", None):
            cmd += ["--upload-port", args.port]
        return _run(cmd + passthrough, cwd=root, env=env)

    if args.command == "build":
        return run_target(None)
    if args.command == "clean":
        return run_target("clean")
    if args.command == "rebuild":
        return run_target("clean") or run_target(None)
    if args.command == "upload":
        return run_target("upload")
    if args.command == "flash":
        return run_target("clean") or run_target("upload")
    if args.command == "deepclean":
        build_dir = _example_build_dir(root, args.example)
        print(f"+ rm -rf {build_dir}")
        shutil.rmtree(build_dir, ignore_errors=True)
        return 0
    if args.command == "monitor":
        cmd = [pio, "device", "monitor", "--baud", str(args.baud)]
        if args.port:
            cmd += ["--port", args.port]
        return _run(cmd + passthrough, cwd=root, env=env)
    parser.error("unknown command")
    return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
