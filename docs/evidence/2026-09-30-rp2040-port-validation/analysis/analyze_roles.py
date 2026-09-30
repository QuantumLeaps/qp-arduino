#!/usr/bin/env python3
"""Analyze an LA5032 capture of examples/port_validation-rp2040.

CH17 is the tick-ISR probe (GP21). CH16 (GP25) carries the selected role:
  1 dispatch: high while the Validator dispatches an event (3 per tick)
  2 idle:     high while the QV idle callback runs
  3 event:    pulse on receipt of each dynamic event
Reports the tick statistics, the pulses per tick on CH16, their widths, the
latency from each tick rising edge to the first CH16 rising edge after it
(role 1: ISR-to-dispatch latency), and CH16's duty cycle (role 2: idle share).
"""
import json
import statistics
import sys
from bisect import bisect_right
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "2026-09-30-rp2040-blinky" / "analysis"))
from analyze_blinky_capture import parse_vcd, stats  # noqa: E402


def main(path):
    scale, edges = parse_vcd(path)
    us = scale * 1e6
    tick = [(t * us, v) for t, v in edges.get("CH17", []) if t > 0]
    role = [(t * us, v) for t, v in edges.get("CH16", []) if t > 0]
    tick_rise = [t for t, v in tick if v == 1]
    role_rise = [t for t, v in role if v == 1]
    role_fall = [t for t, v in role if v == 0]

    out = {"file": path}
    periods = [b - a for a, b in zip(tick_rise, tick_rise[1:])]
    out["tick_period_us"] = stats(periods)
    if periods:
        out["tick_period_us"]["missed_intervals"] = sum(1 for p in periods if p > 15000)
        out["tick_period_us"]["doubled_intervals"] = sum(1 for p in periods if p < 5000)

    # role pulse widths
    widths = []
    j = 0
    for r in role_rise:
        while j < len(role_fall) and role_fall[j] <= r:
            j += 1
        if j < len(role_fall):
            widths.append(role_fall[j] - r)
    out["role_pulse_width_us"] = stats(widths)

    # pulses per tick and latency to the first rising edge after each tick
    lat = []
    per_tick = []
    for a, b in zip(tick_rise, tick_rise[1:]):
        i = bisect_right(role_rise, a)
        k = bisect_right(role_rise, b)
        per_tick.append(k - i)
        if k > i:
            lat.append(role_rise[i] - a)
    out["role_pulses_per_tick"] = stats(per_tick)
    out["tick_to_first_role_rise_us"] = stats(lat)

    # duty cycle of CH16 over the capture
    high = 0.0
    level = None
    last = None
    for t, v in role:
        if level == 1 and last is not None:
            high += t - last
        level, last = v, t
    span = (role[-1][0] - role[0][0]) if len(role) > 1 else 0.0
    out["role_high_fraction"] = (high / span) if span > 0 else None
    out["counts"] = {"tick_rising_edges": len(tick_rise), "role_rising_edges": len(role_rise)}
    return out


if __name__ == "__main__":
    print(json.dumps(main(sys.argv[1]), indent=2))
