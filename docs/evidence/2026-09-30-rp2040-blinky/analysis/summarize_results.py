#!/usr/bin/env python3
"""Print a Markdown table from the result_*.json files in this directory."""
import glob
import json
import os

ROWS = [
    ("result_1MHz_6s.json", "A", "1 MSa/s, 6 s", "attached"),
    ("result_20MHz_500ms.json", "A", "20 MSa/s, 0.5 s", "attached"),
    ("result_100kHz_600s.json", "A", "100 kSa/s, 600 s", "attached"),
    ("result_blinky_1MHz_6s_nohost.json", "A", "1 MSa/s, 6 s", "detached"),
    ("result_blinky_1MHz_6s_noqs.json", "C (no QS)", "1 MSa/s, 6 s", "n/a"),
    ("result_blinky_20MHz_500ms_noqs.json", "C (no QS)", "20 MSa/s, 0.5 s", "n/a"),
    ("result_blinky_1MHz_6s_fastflash_nohost.json", "D (quad-lane boot2)", "1 MSa/s, 6 s", "detached"),
    ("result_blinky_1MHz_6s_fastflash_host.json", "D (quad-lane boot2)", "1 MSa/s, 6 s", "attached"),
    ("result_blinky_uart_1MHz_6s.json", "E (QS on UART0)", "1 MSa/s, 6 s", "analyzer"),
    # Throttled idle-loop transport service (afternoon builds, images F to I)
    ("result_thr_std_host_1MHz_6s.json", "F (throttled, USB CDC)", "1 MSa/s, 6 s", "attached"),
    ("result_thr_std_nohost_1MHz_6s.json", "F (throttled, USB CDC)", "1 MSa/s, 6 s", "detached"),
    ("result_thr_noqs_1MHz_6s.json", "G (no QS)", "1 MSa/s, 6 s", "n/a"),
    ("result_thr_uart_1MHz_6s.json", "H (QS on UART0)", "1 MSa/s, 6 s", "analyzer"),
    ("result_thr_txonly_1MHz_30s.json", "I (transmit-only, UART0)", "1 MSa/s, 30 s", "analyzer"),
]

here = os.path.dirname(os.path.abspath(__file__))
print("| Capture | Firmware | Trace host | Ticks | Tick period mean / min / max (µs) | ISR width typ / posting tick (µs) | LED interval mean / min / max (µs) | Tick→LED latency min / p99 / max (µs) |")
print("|---|---|---|---|---|---|---|---|")
for fn, fw, cap, host in ROWS:
    path = os.path.join(here, fn)
    if not os.path.exists(path):
        continue
    r = json.load(open(path))
    tp, w, led, lat = r["tick_period_us"], r["tick_pulse_width_us"], r["led_toggle_interval_us"], r["tick_to_led_latency_us"]
    note = ""
    if "100kHz" in fn:
        note = " (tick pulses under-sampled at 10 µs; LED column is the valid evidence)"
    tick = f'{tp["n"]+1}' if tp else "-"
    period = f'{tp["mean"]:.2f} / {tp["min"]:.0f} / {tp["max"]:.0f}' if tp else "-"
    width = f'{w["p50"]:.1f} / {w["max"]:.0f}' if w else "-"
    ledc = f'{led["mean"]:.1f} / {led["min"]:.0f} / {led["max"]:.0f}' if led else "-"
    latc = f'{lat["min"]:.0f} / {lat["p99"]:.0f} / {lat["max"]:.0f}' if lat else "-"
    print(f"| {cap}{note} | {fw} | {host} | {tick} | {period} | {width} | {ledc} | {latc} |")
