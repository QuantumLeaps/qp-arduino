#!/usr/bin/env python3
"""Tick-period distribution of a long LA5032 capture (the watchdog-race check).

CH17 is the tick-ISR probe (GP21). A healthy tick gives one narrow lobe at the
nominal period. The first hardware-alarm watchdog produced a second lobe 60 to
110 us late (a spurious re-arm re-based the next tick) with no short partner,
so the period distribution, not just its mean, is the evidence here.

Usage: tick_periods.py CAPTURE.vcd [--window NAME:START_S:END_S ...]
                       [--nominal-us 10000] [--late-us 40] [--json OUT]
Without --window the whole capture is one window. 100 kSa/s captures resolve
10 us, so bins are 10 us wide.
"""
import argparse
import json
import statistics
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "2026-09-30-rp2040-blinky" / "analysis"))
from analyze_blinky_capture import parse_vcd  # noqa: E402


def summarize(periods, nominal, late):
    v = sorted(periods)
    n = len(v)
    if n == 0:
        return {"n": 0}
    q = lambda p: v[min(n - 1, int(round(p * (n - 1))))]  # noqa: E731
    hist = Counter(int(round((p - nominal) / 10.0)) * 10 for p in v)
    return {
        "n": n,
        "mean": statistics.mean(v),
        "stdev": statistics.pstdev(v),
        "min": v[0],
        "p50": q(0.5),
        "p99": q(0.99),
        "p999": q(0.999),
        "max": v[-1],
        "late": sum(1 for p in v if p > nominal + late),
        "late_percent": 100.0 * sum(1 for p in v if p > nominal + late) / n,
        "early": sum(1 for p in v if p < nominal - late),
        "missed": sum(1 for p in v if p > 1.5 * nominal),
        "doubled": sum(1 for p in v if p < 0.5 * nominal),
        "histogram_offset_us": {str(k): hist[k] for k in sorted(hist)},
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("vcd")
    ap.add_argument("--window", action="append", default=[], help="NAME:START_S:END_S")
    ap.add_argument("--nominal-us", type=float, default=10000.0)
    ap.add_argument("--late-us", type=float, default=40.0, help="a period this much above nominal counts as late")
    ap.add_argument("--json")
    a = ap.parse_args()

    scale, edges = parse_vcd(a.vcd)
    us = scale * 1e6
    rise = [t * us for t, v in edges.get("CH17", []) if t > 0 and v == 1]
    out = {"file": a.vcd, "ticks": len(rise), "span_s": (rise[-1] - rise[0]) / 1e6 if rise else 0.0,
           "nominal_us": a.nominal_us, "late_us": a.late_us, "windows": {}}
    windows = [w.split(":") for w in a.window] or [["all", "0", str(1e12)]]
    for name, lo, hi in windows:
        lo_us, hi_us = float(lo) * 1e6, float(hi) * 1e6
        periods = [b - x for x, b in zip(rise, rise[1:]) if lo_us <= x < hi_us]
        out["windows"][name] = summarize(periods, a.nominal_us, a.late_us)

    print(f"{a.vcd}: {out['ticks']} ticks over {out['span_s']:.1f} s")
    for name, w in out["windows"].items():
        if w["n"] == 0:
            print(f"  {name}: no ticks")
            continue
        print(f"  {name}: n={w['n']} mean {w['mean']:.2f} stdev {w['stdev']:.1f} min {w['min']:.0f} "
              f"p50 {w['p50']:.0f} p99 {w['p99']:.0f} p99.9 {w['p999']:.0f} max {w['max']:.0f} us; "
              f"late(>+{a.late_us:.0f}) {w['late']} ({w['late_percent']:.2f} %), early {w['early']}, "
              f"missed {w['missed']}, doubled {w['doubled']}")
        print("    histogram (offset us: count): " + ", ".join(f"{k}: {c}" for k, c in w["histogram_offset_us"].items()))
    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1) + "\n")


if __name__ == "__main__":
    main()
