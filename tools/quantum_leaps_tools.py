#!/usr/bin/env python3
"""
quantum_leaps_tools.py

Install and run the Quantum Leaps host tools that are pinned to match the
qpcpp version used by this repository (qpcpp v7.3.4):

- QM      6.1.1  (binary releases exist for Linux and Windows only)
- QTools  7.3.4  (QSPY, QUTest, QView). This version has no binary release,
                 so it is cloned from its git tag and qspy is built from source
                 with make and a C compiler. The qspy built from that tag
                 reports version 7.3.3, as generated upstream.

The tools are installed locally into:
  <repo>/.tools/quantum-leaps/          (override with QL_TOOLS_DIR)

This keeps the repository usable on Linux/macOS/Windows via `pixi`, without
requiring system-wide installs.

Typical usage (via pixi tasks):

  # Install QM and build QTools
  pixi run ql-install

  # Launch QM
  pixi run qm

  # Run QSPY (arguments are forwarded to qspy)
  pixi run qspy -c /dev/ttyACM0 -b 115200

Notes:
  * This wrapper is intentionally self-contained (Python stdlib only); it needs
    `git`, `make` and a C compiler on PATH, which the pixi environment provides.
  * QSPY command-line options (like -c, -b, -u, -t, etc.) are passed through.
    If you need QSPY's own help, use the explicit separator:
        pixi run qspy -- -h
"""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import textwrap
import urllib.error
import urllib.request
import zipfile
from pathlib import Path
from typing import Iterable, Optional

QM_VERSION = "6.1.1"
QTOOLS_VERSION = "7.3.4"
QTOOLS_GIT_URL = "https://github.com/QuantumLeaps/qtools.git"


def _repo_root() -> Path:
    # This file is intended to live at <repo>/tools/quantum_leaps_tools.py
    return Path(__file__).resolve().parents[1]


def _tools_base_dir() -> Path:
    # Allow override if a user doesn't want installs inside the repo.
    # Example:
    #   QL_TOOLS_DIR=~/.cache/qp-arduino/quantum-leaps pixi run ql-install
    override = os.environ.get("QL_TOOLS_DIR")
    if override:
        return Path(override).expanduser().resolve()
    return _repo_root() / ".tools" / "quantum-leaps"


def _cache_dir() -> Path:
    return _tools_base_dir() / "_cache"


def _sys_id() -> tuple[str, str]:
    sysname = platform.system().lower()  # linux / windows / darwin
    machine = platform.machine().lower()
    return sysname, machine


def _print(msg: str) -> None:
    print(msg, flush=True)


def _eprint(msg: str) -> None:
    print(msg, file=sys.stderr, flush=True)


def _is_probably_html(path: Path) -> bool:
    try:
        head = path.read_bytes()[:512].lstrip()
    except Exception:
        return False
    return head.startswith(b"<!doctype html") or head.startswith(b"<html") or head.startswith(b"<HTML")


def _download(urls: Iterable[str], dest: Path, *, force: bool = False, validate_zip: bool = False) -> Path:
    """Download from the first working URL into dest.

    validate_zip=True performs zipfile.is_zipfile() validation to avoid saving
    HTML redirect pages into a .zip file.
    """
    dest.parent.mkdir(parents=True, exist_ok=True)

    if dest.exists() and not force:
        return dest

    tmp = dest.with_suffix(dest.suffix + ".tmp")
    if tmp.exists():
        tmp.unlink()

    last_err: Optional[BaseException] = None

    for url in urls:
        try:
            _print(f"Downloading: {url}")
            req = urllib.request.Request(
                url,
                headers={
                    # Some hosts behave differently depending on UA.
                    "User-Agent": "qp-arduino/quantum_leaps_tools.py (+https://github.com/QuantumLeaps/qp-arduino)",
                },
            )
            with urllib.request.urlopen(req) as r, open(tmp, "wb") as f:
                shutil.copyfileobj(r, f)

            if validate_zip:
                if not zipfile.is_zipfile(tmp) or _is_probably_html(tmp):
                    raise RuntimeError("downloaded file is not a valid zip archive (maybe an HTML redirect page)")

            tmp.replace(dest)
            return dest

        except (urllib.error.HTTPError, urllib.error.URLError, RuntimeError, OSError) as e:
            last_err = e
            _eprint(f"  failed: {e}")
            if tmp.exists():
                try:
                    tmp.unlink()
                except OSError:
                    pass
            continue

    raise SystemExit(f"ERROR: Unable to download {dest.name}. Last error: {last_err}")


def _extract_zip_move_top(
    archive: Path,
    dest_dir: Path,
    *,
    expected_top_dir_name: Optional[str] = None,
    force: bool = False,
) -> Path:
    """Extract a zip archive and move a top-level directory into dest_dir."""
    if dest_dir.exists() and any(dest_dir.iterdir()) and not force:
        return dest_dir

    if dest_dir.exists() and force:
        shutil.rmtree(dest_dir)

    dest_dir.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="ql_zip_") as td:
        tmp_root = Path(td)
        with zipfile.ZipFile(archive, "r") as z:
            z.extractall(tmp_root)

        src_dir: Optional[Path] = None

        if expected_top_dir_name:
            for p in tmp_root.iterdir():
                if p.is_dir() and p.name.lower() == expected_top_dir_name.lower():
                    src_dir = p
                    break

        if src_dir is None:
            dirs = [p for p in tmp_root.iterdir() if p.is_dir()]
            files = [p for p in tmp_root.iterdir() if p.is_file()]
            if len(dirs) == 1 and not files:
                src_dir = dirs[0]

        if src_dir is None:
            # Fall back: copy whole extracted tree into dest_dir.
            dest_dir.mkdir(parents=True, exist_ok=True)
            for item in tmp_root.iterdir():
                dst = dest_dir / item.name
                if item.is_dir():
                    shutil.copytree(item, dst, dirs_exist_ok=True)
                else:
                    shutil.copy2(item, dst)
            return dest_dir

        shutil.move(str(src_dir), str(dest_dir))

    return dest_dir


def _qm_candidate_urls(sysname: str, machine: str) -> list[str]:
    """Return candidate download URLs for the pinned QM release.

    QM 6.1.1 publishes qm_6.1.1-linux64.zip and qm_6.1.1-win64.zip (plus a
    Windows installer .exe, not used here). There is no macOS build.
    """
    tag = f"v{QM_VERSION}"
    gh_base = f"https://github.com/QuantumLeaps/qm/releases/download/{tag}/"

    if sysname == "linux":
        return [gh_base + f"qm_{QM_VERSION}-linux64.zip"]
    if sysname == "windows":
        return [gh_base + f"qm_{QM_VERSION}-win64.zip"]
    if sysname == "darwin":
        raise SystemExit(
            f"QM {QM_VERSION} has no macOS release (only Linux and Windows binaries are published).\n"
            "Run QM on a Linux or Windows machine or VM; QTools/QSPY still install and build on macOS."
        )
    raise SystemExit(f"Unsupported OS: {sysname}")


def _clone_qtools(install_dir: Path, *, force: bool = False) -> Path:
    """Clone the pinned QTools git tag into install_dir (no binary release exists)."""
    if install_dir.exists() and any(install_dir.iterdir()) and not force:
        return install_dir
    if install_dir.exists():
        shutil.rmtree(install_dir)
    install_dir.parent.mkdir(parents=True, exist_ok=True)
    tag = f"v{QTOOLS_VERSION}"
    _print(f"Cloning QTools {tag} from {QTOOLS_GIT_URL} ...")
    try:
        subprocess.run(
            ["git", "clone", "--depth", "1", "--branch", tag, QTOOLS_GIT_URL, str(install_dir)],
            check=True,
        )
    except FileNotFoundError:
        raise SystemExit("ERROR: 'git' not found. Run inside the pixi environment or install git.")
    except subprocess.CalledProcessError as e:
        raise SystemExit(f"ERROR: cloning QTools {tag} failed: {e}")
    return install_dir


def install_qm(*, force: bool = False) -> Path:
    sysname, machine = _sys_id()
    base = _tools_base_dir()
    cache = _cache_dir()
    cache.mkdir(parents=True, exist_ok=True)

    install_dir = base / f"qm-{QM_VERSION}"

    if True:
        zip_path = cache / f"qm_{QM_VERSION}_{sysname}.zip"
        _download(_qm_candidate_urls(sysname, machine), zip_path, force=force, validate_zip=True)
        _extract_zip_move_top(zip_path, install_dir, expected_top_dir_name="qm", force=force)

        # Ensure executable bits on Linux (zip archives can lose them)
        if sysname == "linux":
            for exe in (install_dir / "bin" / "qm", install_dir / "bin" / "qm.sh",
                        install_dir / "bin" / "qmc", install_dir / "bin" / "qmc.sh"):
                if exe.exists():
                    exe.chmod(exe.stat().st_mode | 0o111)

        return install_dir


def _is_file(p: Path) -> bool:
    try:
        return p.is_file()
    except OSError:
        return False


def _find_qspy_binary(qtools_dir: Path) -> Optional[Path]:
    """Locate the qspy executable (NOT the source directory).

    Important: QTools ships the qspy *source* under qspy/, and after building on
    POSIX, the qspy executable typically ends up in <qtools>/bin/qspy.
    """
    sysname, _ = _sys_id()
    if sysname == "windows":
        candidates = [
            qtools_dir / "bin" / "qspy.exe",
            qtools_dir / "qspy" / "bin" / "qspy.exe",
            qtools_dir / "qspy.exe",
        ]
        name = "qspy.exe"
    else:
        candidates = [
            qtools_dir / "bin" / "qspy",
            qtools_dir / "qspy" / "posix" / "qspy",
            qtools_dir / "qspy" / "bin" / "qspy",
        ]
        name = "qspy"

    for p in candidates:
        if _is_file(p):
            return p

    # Fallback: search (filter out directories)
    matches = [p for p in qtools_dir.rglob(name) if _is_file(p)]
    return matches[0] if matches else None


def _ensure_executable(p: Path) -> None:
    try:
        p.chmod(p.stat().st_mode | 0o111)
    except OSError:
        # Best-effort: ignore chmod failures on some filesystems
        pass


def _build_qspy_posix(qtools_dir: Path) -> None:
    """Build qspy from source (qspy/posix on Linux and macOS, qspy/win32 with MinGW gcc on Windows)."""
    sysname, _ = _sys_id()
    posix_dir = qtools_dir / "qspy" / ("win32" if sysname == "windows" else "posix")
    if not posix_dir.exists():
        _eprint(f"WARNING: expected directory not found: {posix_dir}")
        return

    _print("Building qspy from source (make) ...")
    try:
        subprocess.run(["make"], cwd=str(posix_dir), check=True)
    except FileNotFoundError:
        _eprint("ERROR: 'make' not found. Ensure your pixi environment includes build tools and try again.")
        return
    except subprocess.CalledProcessError as e:
        _eprint(f"ERROR: qspy build failed: {e}")
        return

    # After building, qspy is expected in <qtools>/bin/qspy (per qp-arduino docs),
    # but keep a fallback copy if it's produced elsewhere.
    qspy_bin = qtools_dir / "bin" / "qspy"
    if _is_file(qspy_bin):
        _ensure_executable(qspy_bin)
        return

    # Look for a freshly built qspy inside the build dir
    built_candidates = [
        posix_dir / "qspy",
        posix_dir / "bin" / "qspy",
        posix_dir / "../bin/qspy",
    ]
    for cand in built_candidates:
        cand_path = cand.resolve()
        if _is_file(cand_path):
            (qtools_dir / "bin").mkdir(parents=True, exist_ok=True)
            shutil.copy2(cand_path, qspy_bin)
            _ensure_executable(qspy_bin)
            return

    # As last resort, search within qspy/posix
    matches = [p for p in posix_dir.rglob("qspy") if _is_file(p)]
    if matches:
        (qtools_dir / "bin").mkdir(parents=True, exist_ok=True)
        shutil.copy2(matches[0], qspy_bin)
        _ensure_executable(qspy_bin)


def install_qtools(*, force: bool = False, build_qspy: bool = True) -> Path:
    """Clone QTools at the pinned tag and build qspy if it is not present yet."""
    base = _tools_base_dir()
    install_dir = base / f"qtools-{QTOOLS_VERSION}"

    _clone_qtools(install_dir, force=force)

    if build_qspy:
        qspy_bin = _find_qspy_binary(install_dir)
        if qspy_bin is None or force:
            _build_qspy_posix(install_dir)

    return install_dir


def run_qm(qm_args: list[str]) -> int:
    sysname, _ = _sys_id()
    qm_dir = install_qm(force=False)

    if sysname == "linux":
        entry = qm_dir / "bin" / "qm.sh"
        if not entry.exists():
            entry = qm_dir / "bin" / "qm"
        if not entry.exists():
            raise SystemExit(f"ERROR: QM executable not found under: {qm_dir}/bin")
        _ensure_executable(entry)
        proc = subprocess.run([str(entry)] + qm_args, check=False)
        return proc.returncode

    if sysname == "windows":
        candidates = [
            qm_dir / "bin" / "qm.exe",
            qm_dir / "qm.exe",
            qm_dir / "bin" / "qm.bat",
            qm_dir / "qm.bat",
        ]
        entry = next((p for p in candidates if p.exists()), None)
        if entry is None:
            matches = list(qm_dir.rglob("qm.exe")) + list(qm_dir.rglob("qm.bat"))
            entry = matches[0] if matches else None
        if entry is None:
            raise SystemExit(f"ERROR: QM executable not found under: {qm_dir}")
        proc = subprocess.run([str(entry)] + qm_args, check=False)
        return proc.returncode

    if sysname == "darwin":
        apps = list(qm_dir.glob("*.app")) or list(qm_dir.rglob("*.app"))
        if not apps:
            raise SystemExit(f"ERROR: QM app bundle (*.app) not found under: {qm_dir}")
        app = apps[0]
        # Pass args to the app via --args
        cmd = ["open", "-a", str(app)]
        if qm_args:
            cmd += ["--args"] + qm_args
        proc = subprocess.run(cmd, check=False)
        return proc.returncode

    raise SystemExit(f"Unsupported OS: {sysname}")


def _augment_runtime_env(env: dict[str, str], qtools_dir: Path) -> dict[str, str]:
    """Add qtools/bin to PATH and library search paths (best-effort)."""
    sysname, _ = _sys_id()
    bin_dir = qtools_dir / "bin"
    if bin_dir.exists():
        # PATH
        old_path = env.get("PATH", "")
        env["PATH"] = str(bin_dir) + (os.pathsep + old_path if old_path else "")

        if sysname == "linux":
            ld = env.get("LD_LIBRARY_PATH", "")
            env["LD_LIBRARY_PATH"] = str(bin_dir) + (os.pathsep + ld if ld else "")
        elif sysname == "darwin":
            dyld = env.get("DYLD_LIBRARY_PATH", "")
            env["DYLD_LIBRARY_PATH"] = str(bin_dir) + (os.pathsep + dyld if dyld else "")
    return env


def run_qspy(qspy_args: list[str]) -> int:
    sysname, _ = _sys_id()
    qtools_dir = install_qtools(force=False, build_qspy=True)
    qspy = _find_qspy_binary(qtools_dir)

    if qspy is None:
        raise SystemExit(
            "ERROR: qspy binary was not found after installing QTools.\n\n"
            "On Linux/macOS you may need build tools (make + a C compiler).\n"
            "Try:\n"
            "  pixi install\n"
            "  pixi run qtools-install\n"
        )

    if sysname != "windows":
        _ensure_executable(qspy)

    # Some QTools utilities look for QTOOLS/QTOOLS_HOME.
    env = os.environ.copy()
    env.setdefault("QTOOLS", str(qtools_dir))
    env.setdefault("QTOOLS_HOME", str(qtools_dir))
    env = _augment_runtime_env(env, qtools_dir)

    cmd = [str(qspy)] + qspy_args
    _print("Running: " + " ".join(cmd))
    proc = subprocess.run(cmd, env=env, check=False)
    return proc.returncode


def install_all(*, force: bool = False) -> None:
    install_qm(force=force)
    install_qtools(force=force, build_qspy=True)
    _print(f"Installed QM {QM_VERSION} and QTools {QTOOLS_VERSION} into: {_tools_base_dir()}")
    _print("Consider adding '.tools/' (or '.tools/quantum-leaps/') to your .gitignore.")


def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        prog="quantum_leaps_tools.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent(
            f"""\
            Install and run Quantum Leaps host tools pinned for this repository:

              QM      {QM_VERSION}
              QTools  {QTOOLS_VERSION} (QSPY, QUTest, QView; built from the git tag)

            Tools are installed locally into:
              {str(_tools_base_dir())}

            Examples:
              python tools/quantum_leaps_tools.py install
              python tools/quantum_leaps_tools.py qm
              python tools/quantum_leaps_tools.py qspy -c /dev/ttyACM0 -b 115200
              python tools/quantum_leaps_tools.py qspy -- -h    # show qspy help
            """
        ),
    )

    sub = parser.add_subparsers(dest="command", required=True)

    p_install = sub.add_parser("install", help="Install both QM and QTools")
    p_install.add_argument("--force", action="store_true", help="Re-download and reinstall from scratch")

    p_qm = sub.add_parser("qm", help="Launch QM (installs first if needed)")
    p_qm.add_argument("--install-only", action="store_true", help="Only install QM (do not launch)")
    p_qm.add_argument("--force", action="store_true", help="Re-download and reinstall QM from scratch")

    p_qtools = sub.add_parser("qtools-install", help="Clone QTools and build qspy if needed")
    p_qtools.add_argument("--force", action="store_true", help="Re-clone and rebuild QTools from scratch")

    p_qspy = sub.add_parser("qspy", help="Run QSPY (installs QTools first if needed)")
    p_qspy.add_argument("--force", action="store_true", help="Re-clone and rebuild QTools from scratch")
    # NOTE: We intentionally do NOT declare qspy's options here.
    # We parse *known* wrapper args and forward everything else to qspy.

    args, remainder = parser.parse_known_args(argv)

    if args.command == "install":
        if remainder:
            parser.error("unrecognized arguments: " + " ".join(remainder))
        install_all(force=args.force)
        return 0

    if args.command == "qm":
        install_qm(force=args.force)
        if remainder:
            # For qm, forward any extra args to the QM executable (best-effort)
            qm_args = remainder
            if qm_args and qm_args[0] == "--":
                qm_args = qm_args[1:]
        else:
            qm_args = []
        if not args.install_only:
            return run_qm(qm_args)
        return 0

    if args.command == "qtools-install":
        if remainder:
            parser.error("unrecognized arguments: " + " ".join(remainder))
        install_qtools(force=args.force, build_qspy=True)
        return 0

    if args.command == "qspy":
        # remainder contains qspy flags like -c, -b, etc.
        qspy_args = remainder
        if qspy_args and qspy_args[0] == "--":
            qspy_args = qspy_args[1:]
        if args.force:
            install_qtools(force=True, build_qspy=True)
        return run_qspy(qspy_args)

    raise SystemExit("unreachable")


if __name__ == "__main__":
    raise SystemExit(main())
