#!/usr/bin/env python3
"""Summarize a tools/port_validation_stress.py run (its JSON) and, given the
directory where its QSPY sessions wrote their `qspy<date>_<time>.txt` logs,
count the TICK_LOST records those sessions saw. A session attached after the
target's dictionaries prints the record as USER+009, so both names count.

Usage: stress_summary.py stress-runN.json [--qspy-logs DIR]"""
import argparse
import datetime
import glob
import json
import os
import re

ap = argparse.ArgumentParser()
ap.add_argument("json")
ap.add_argument("--qspy-logs")
a = ap.parse_args()

d = json.load(open(a.json))
cycles = d["cycles"]
slow = [c for c in cycles if c["kind"] == "slow"]
rates = [c["rate"] for c in cycles if c.get("rate") is not None]
flagged = [c for c in cycles if not c.get("ok", True)]
t0 = d["started"]
seconds = d["summary"]["seconds"] if "summary" in d else cycles[-1]["t"] + cycles[-1].get("hold", 0.0)
t1 = t0 + seconds
print(f"{a.json}: {len(cycles)} cycles in {seconds:.1f} s{'' if 'summary' in d else ' (run interrupted, no summary)'}, "
      f"{len(slow)} slow-reader cycles ({sum(c.get('slow_bytes', 0) for c in slow)} bytes read at a trickle), "
      f"{len(flagged)} flagged; tick rate {min(rates):.1f} to {max(rates):.1f} per second; "
      f"last tick count {cycles[-1].get('ticks')}")
print(f"  window {datetime.datetime.fromtimestamp(t0):%Y-%m-%d %H:%M:%S} to {datetime.datetime.fromtimestamp(t1):%H:%M:%S}")
for c in flagged:
    print(f"  flagged cycle {c['n']}: {c.get('detail')}")

if a.qspy_logs:
    named = unnamed = 0
    files = []
    last = None
    for f in sorted(glob.glob(os.path.join(a.qspy_logs, "qspy*_*.txt"))):
        m = re.search(r"qspy(\d{6})_(\d{6})\.txt$", f)
        if not m:
            continue
        ts = datetime.datetime.strptime(m.group(1) + m.group(2), "%y%m%d%H%M%S").timestamp()
        if not (t0 - 2 <= ts <= t1 + 2):
            continue
        files.append(f)
        txt = open(f, errors="replace").read()
        named += len(re.findall(r"^\d{10} TICK_LOST ", txt, re.M))
        unnamed += len(re.findall(r"^\d{10} USER\+009 ", txt, re.M))
        for mm in re.finditer(r"^\d{10} (?:TICK_LOST|USER\+009) (\d+) (\d+) (\d+)", txt, re.M):
            last = tuple(int(x) for x in mm.groups())
    print(f"  QSPY session logs in the window: {len(files)}; TICK_LOST records: {named + unnamed} "
          f"({named} named, {unnamed} as USER+009)")
    if last:
        print(f"  last TICK_LOST record: tick count {last[0]}, firmware re-arm counter {last[1]}, catch-ups {last[2]}")
