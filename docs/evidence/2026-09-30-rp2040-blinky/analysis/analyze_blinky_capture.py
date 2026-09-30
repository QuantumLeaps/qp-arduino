#!/usr/bin/env python3
"""Analyze an LA5032 capture of the qp-arduino RP2040 Blinky fixture.

Input: a VCD exported by sigrok-cli (-O vcd) with the InstrumentPort-40 channels
CH16 (TRIG0 = FW_TRIG0 = GP25, the Blinky LED) and CH17 (TRIG1 = FW_TRIG1 = GP21,
the 100 Hz tick-ISR probe).

Reports, in microseconds unless stated:
- tick: rising-edge count, period mean/min/max/stdev, deviation from 10 ms,
  missed (> 1.5 periods) and doubled (< 0.5 period) intervals, ISR pulse width
- LED: toggle count, toggle interval mean/min/max (expect 500 ms = 50 ticks),
  latency from the tick rising edge that caused it to the LED edge (p50/p99/max)
"""
import json
import statistics
import sys
from bisect import bisect_right


def parse_vcd(path):
    scale = None
    names = {}
    edges = {}
    t = 0
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith("$timescale"):
                # "$timescale 1 us $end" or split across lines; handle inline form
                parts = line.replace("$end", "").split()
                num, unit = parts[1], parts[2] if len(parts) > 2 else "s"
                scale = float(num) * {"s": 1.0, "ms": 1e-3, "us": 1e-6, "ns": 1e-9, "ps": 1e-12}[unit]
            elif line.startswith("$var"):
                parts = line.split()
                ident, name = parts[3], parts[4]
                names[ident] = name
                edges[name] = []
            elif line.startswith("#") or (line[0] in "01b" and not line.startswith("$")):
                # libsigrok writes "#<time>  1! 0\"" with the changes on the same line
                tokens = line.split()
                k = 0
                while k < len(tokens):
                    tok = tokens[k]
                    if tok.startswith("#"):
                        t = int(tok[1:])
                    elif tok[0] in "01" and tok[1:] in names:
                        edges[names[tok[1:]]].append((t, int(tok[0])))
                    elif tok.startswith("b") and k + 1 < len(tokens) and tokens[k + 1] in names:
                        edges[names[tokens[k + 1]]].append((t, int(tok[1:], 2)))
                        k += 1
                    k += 1
    if scale is None:
        raise SystemExit("no $timescale in VCD")
    return scale, edges


def stats(values):
    if not values:
        return {}
    v = sorted(values)
    return {
        "n": len(v),
        "mean": statistics.mean(v),
        "min": v[0],
        "max": v[-1],
        "stdev": statistics.pstdev(v) if len(v) > 1 else 0.0,
        "p50": v[len(v) // 2],
        "p99": v[min(len(v) - 1, int(round(0.99 * (len(v) - 1))))],
    }


def main(path, tick_period_us=10000.0, led_ticks=50):
    scale, edges = parse_vcd(path)
    us = scale * 1e6  # VCD time units -> microseconds
    tick = edges.get("CH17", [])
    led = edges.get("CH16", [])
    # drop the initial-value entries at t=0 that only state the starting level
    tick_rise = [t * us for t, v in tick if v == 1]
    tick_fall = [t * us for t, v in tick if v == 0]
    led_edges = [t * us for t, v in led]
    # first samples may carry the initial level at t=0; a real edge never sits at t=0
    tick_rise = [t for t in tick_rise if t > 0]
    tick_fall = [t for t in tick_fall if t > 0]
    led_edges = [t for t in led_edges if t > 0]

    out = {"file": path, "vcd_time_unit_us": us}

    periods = [b - a for a, b in zip(tick_rise, tick_rise[1:])]
    ps = stats(periods)
    if ps:
        ps["deviation_from_nominal_pct"] = (ps["mean"] - tick_period_us) / tick_period_us * 100.0
        ps["missed_intervals"] = sum(1 for p in periods if p > 1.5 * tick_period_us)
        ps["doubled_intervals"] = sum(1 for p in periods if p < 0.5 * tick_period_us)
        ps["span_s"] = (tick_rise[-1] - tick_rise[0]) / 1e6
    out["tick_period_us"] = ps

    # ISR pulse width: pair each rising edge with the next falling edge
    widths = []
    j = 0
    for r in tick_rise:
        while j < len(tick_fall) and tick_fall[j] <= r:
            j += 1
        if j < len(tick_fall):
            widths.append(tick_fall[j] - r)
    out["tick_pulse_width_us"] = stats(widths)

    toggles = [b - a for a, b in zip(led_edges, led_edges[1:])]
    ts = stats(toggles)
    if ts:
        ts["expected_us"] = led_ticks * tick_period_us
        ts["deviation_from_expected_pct"] = (ts["mean"] - led_ticks * tick_period_us) / (led_ticks * tick_period_us) * 100.0
    out["led_toggle_interval_us"] = ts

    # latency from the most recent tick rising edge to each LED edge
    lat = []
    for e in led_edges:
        i = bisect_right(tick_rise, e) - 1
        if i >= 0:
            lat.append(e - tick_rise[i])
    out["tick_to_led_latency_us"] = stats(lat)
    out["counts"] = {"tick_rising_edges": len(tick_rise), "led_edges": len(led_edges)}
    return out


if __name__ == "__main__":
    result = main(sys.argv[1])
    print(json.dumps(result, indent=2))
