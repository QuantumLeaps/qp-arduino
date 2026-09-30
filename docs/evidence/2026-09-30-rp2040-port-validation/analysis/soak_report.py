#!/usr/bin/env python3
"""Summarize a long QSPY text log: duration, record count, resets, assertions,
COMMS errors and the largest gaps between consecutive timestamped records.

QS timestamps here are micros() truncated to 32 bits, so they wrap every
4,294.967 s; the script unwraps them. The first --settle seconds are reported
separately because a QSPY session attached to a running target first receives
whatever stale output the USB CDC FIFO still held (see the Blinky evidence)."""
import argparse
import re

ap = argparse.ArgumentParser()
ap.add_argument("log")
ap.add_argument("--settle", type=float, default=60.0, help="seconds at the start to exclude from the gap statistics")
a = ap.parse_args()

txt = open(a.log, errors="replace").read()
raw = [int(m.group(1)) for m in re.finditer(r"^(\d{10}) ", txt, re.M)]
ts = []
offset = 0
for i, t in enumerate(raw):
    if i and t + offset < ts[-1] - 2**31:
        offset += 2**32
    ts.append(t + offset)
print(f"timestamped records: {len(ts)}")
if ts:
    print(f"target time span (unwrapped): {ts[0]/1e6:.1f} s -> {ts[-1]/1e6:.1f} s ({(ts[-1]-ts[0])/1e6:.1f} s)")
    t0 = ts[0] + a.settle * 1e6
    gaps = [(b - x, x) for x, b in zip(ts, ts[1:]) if x >= t0]
    if gaps:
        top = sorted(gaps)[-3:]
        print(f"largest gaps after the first {a.settle:.0f} s (s, at target time): " +
              ", ".join(f"{g/1e6:.3f} @ {x/1e6:.1f}" for g, x in top))
    early = [(b - x, x) for x, b in zip(ts, ts[1:]) if x < t0]
    if early:
        g, x = max(early)
        print(f"largest gap inside the first {a.settle:.0f} s: {g/1e6:.3f} s at {x/1e6:.1f} s (session-start boundary)")
print(f"Trg-RST (target resets seen): {len(re.findall(r'Trg-RST', txt))}")
print(f"=ASSERT= records: {len(re.findall(r'=ASSERT=', txt))}")
print(f"<COMMS> lines: {len(re.findall(r'<COMMS>', txt))}")
for line in re.findall(r"^.*<COMMS>.*$", txt, re.M)[:5]:
    print("   ", line.strip())
