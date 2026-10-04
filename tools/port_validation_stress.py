#!/usr/bin/env python3
"""Reader-cycling stress for examples/port_validation-rp2040.

Repeatedly attaches and detaches a trace host while the target runs its full
validation load with every QS group streaming, to exercise the "reader absent,
slow or disconnected" condition and to hunt the one-off stall seen on
2026-09-30. Each cycle:

  fast cycle   start qspy, attach over UDP, send QS_RX_INFO (QS-RX alive),
               read the counters (command 6), optionally switch the GP25 role
               (command 5), hold for a few seconds, stop qspy
  slow cycle   (every --slow-every cycles) open the serial port directly and
               read at a trickle so the target's QS buffer overflows, then
               close it and check with qspy that the target still ticks and
               answers

Between cycles the port stays closed for a random gap. Counters must keep
increasing at 100 ticks/s across every cycle; a cycle that gets no target info
or whose counters stall is flagged, and the run continues so recovery (or a
permanent stall) is visible. Results go to --out as JSON, one entry per cycle.
"""

from __future__ import annotations

import argparse
import json
import os
import random
import select
import struct
import sys
import time
import tty
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from port_validation_rp2040 import QSpyLink, TO_TRG_INFO  # noqa: E402


def slow_read(port: str, seconds: float, bytes_per_read: int, interval_s: float) -> int:
    """Open the CDC port raw and read it slowly; returns bytes read."""
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    total = 0
    try:
        tty.setraw(fd)
        deadline = time.time() + seconds
        while time.time() < deadline:
            r, _, _ = select.select([fd], [], [], interval_s)
            if r:
                try:
                    total += len(os.read(fd, bytes_per_read))
                except BlockingIOError:
                    pass
            time.sleep(interval_s)
    finally:
        os.close(fd)
    return total


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--port", required=True, help="/dev/serial/by-id/... path of the board")
    ap.add_argument("--qspy", default=".tools/quantum-leaps/qtools-7.3.4/bin/qspy")
    ap.add_argument("--udp-port", type=int, default=7701)
    ap.add_argument("--duration", type=float, default=3600.0, help="seconds to run")
    ap.add_argument("--slow-every", type=int, default=5, help="every Nth cycle is a slow-reader cycle")
    ap.add_argument("--out", default=".tools/port_validation_stress.json")
    ap.add_argument("--seed", type=int, default=1)
    args = ap.parse_args(argv)

    rnd = random.Random(args.seed)
    out = Path(args.out)
    workdir = out.parent if out.parent != Path("") else Path(".")
    workdir.mkdir(parents=True, exist_ok=True)
    cycles: list[dict] = []
    t_start = time.time()
    last_ticks: int | None = None
    last_time: float | None = None
    flagged = 0

    def log(entry: dict) -> None:
        cycles.append(entry)
        out.write_text(json.dumps({"port": args.port, "started": t_start, "cycles": cycles}, indent=1))
        flag = "" if entry.get("ok", True) else "  <-- FLAG"
        print(f"[{entry['t']:7.1f}s] cycle {entry['n']:3d} {entry['kind']:4s} {entry.get('detail', '')}{flag}", flush=True)

    n = 0
    while time.time() - t_start < args.duration:
        n += 1
        kind = "slow" if (n % args.slow_every) == 0 else "fast"
        entry: dict = {"n": n, "kind": kind, "t": round(time.time() - t_start, 1)}
        if kind == "slow":
            secs = rnd.uniform(6.0, 12.0)
            got = slow_read(args.port, secs, 256, 0.1)
            entry["slow_seconds"] = round(secs, 1)
            entry["slow_bytes"] = got
            time.sleep(0.5)
        # attach with qspy and check the target
        link = QSpyLink(args.qspy, args.port, 115200, args.udp_port, workdir, f"cycle{n}")
        if not link.start(timeout_s=6.0):
            link.stop()
            entry.update(ok=False, detail="qspy attach failed")
            flagged += 1
            log(entry)
            time.sleep(2.0)
            continue
        time.sleep(0.3)
        link.drain()
        link.target_info = None
        link.send(struct.pack("<B", TO_TRG_INFO))
        deadline = time.time() + 3.0
        while time.time() < deadline and link.target_info is None:
            time.sleep(0.05)
        info_ok = link.target_info is not None
        link.drain()
        link.command(6)
        m = link.expect(r"(?:VALIDATION_STAT|USER\+000)\s+(\d+)\s+(\d+)\s+(\d+)", 3.0, [])
        ticks = int(m.group(1)) if m else None
        now = time.time()
        rate = None
        if ticks is not None and last_ticks is not None and last_time is not None:
            rate = (ticks - last_ticks) / (now - last_time)
        if ticks is not None:
            last_ticks, last_time = ticks, now
        if kind == "fast" and rnd.random() < 0.5:
            role = rnd.choice([0, 1, 2, 3])
            link.command(5, role)
            link.expect(r"(?:PROBE_ROLE|USER\+001)\s+\d+", 2.0, [])
            entry["role"] = role
        hold = rnd.uniform(1.0, 8.0)
        time.sleep(hold)
        link.command(5, 1)  # leave the dispatch marker selected
        time.sleep(0.2)
        link.stop()
        ok = info_ok and ticks is not None and (rate is None or 80.0 <= rate <= 120.0)
        if not ok:
            flagged += 1
        entry.update(ok=ok, info=info_ok, ticks=ticks, rate=None if rate is None else round(rate, 1),
                     hold=round(hold, 1),
                     detail=f"info={info_ok} ticks={ticks} rate={None if rate is None else round(rate, 1)}/s hold={hold:.1f}s"
                            + (f" slow={entry.get('slow_seconds')}s/{entry.get('slow_bytes')}B" if kind == 'slow' else ''))
        log(entry)
        gap = rnd.uniform(0.5, 6.0)
        time.sleep(gap)

    summary = {"cycles": n, "flagged": flagged, "seconds": round(time.time() - t_start, 1)}
    out.write_text(json.dumps({"port": args.port, "started": t_start, "summary": summary, "cycles": cycles}, indent=1))
    print(f"\nDONE: {n} cycles, {flagged} flagged, {summary['seconds']} s -> {out}")
    return 0 if flagged == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
