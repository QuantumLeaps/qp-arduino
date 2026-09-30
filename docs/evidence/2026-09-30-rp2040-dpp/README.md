# RP2040 DPP on the qualification fixture — 2026-09-30

Second target run of qp-arduino (QP/C++ v7.3.4, QV kernel) on the recorded
RP2040 fixture, this time with `examples/dpp_bsp-rp2040`: five Philo active
objects and one Table exchanging published, posted and pool-allocated events.
Fixture, host tools and channel map are as in
`../2026-09-30-rp2040-blinky/README.md` (LA5032 CH16 = GP25 = LED, high while
the last reported philosopher eats; CH17 = GP21 = tick-ISR probe).
Interpretation is in `UPGRADE_QPCPP_V7.md`.

## Captures and logs

| File | Rate | Length | Trace host | Firmware |
|------|------|--------|------------|----------|
| `captures/dpp_1MHz_6s.sr` | 1 MSa/s | 6 s | attached | first transport (defective) |
| `captures/dpp_1MHz_6s_host.sr` | 1 MSa/s | 6 s | attached | fixed transport |
| `captures/dpp_1MHz_6s_nohost.sr` | 1 MSa/s | 6 s | detached | fixed transport |
| `captures/dpp_2MHz_600s.sr` | 2 MSa/s | 600 s | attached | fixed transport, direct tick edges |
| `captures/dpp_10kHz_7200s.sr` | 10 kSa/s | 7200 s | attached | fixed transport, soak (LED activity only) |
| `qspy/qspy-first-transport-25s-with-checksum-errors.txt` | | 25 s | | the defect: 60 `<COMMS>` errors |
| `qspy/qspy-fixed-transport-30s.txt` | | 30 s | | 1,624 records, no errors |
| `qspy/qspy-soak.txt` | | ~2.25 h | | the soak trace |

Each `.sr` has a matching `.vcd` and `analysis/result_*.json`; the analyzer is
`../2026-09-30-rp2040-blinky/analysis/analyze_blinky_capture.py` (the LED
statistics are meaningless for DPP, whose LED follows the eating rhythm).

## Transport defect

The first DPP trace showed `Bad checksum` and `Discontinuity` errors every few
hundred milliseconds. Cause: the idle-loop writer called `QS::getBlock()` for
as much as the QS buffer held, then wrote only what the USB CDC FIFO accepted
and dropped the rest, so records were cut mid-frame. Blinky's low record rate
had hidden it. Fix: ask `availableForWrite()` first and take at most that much
from the QS buffer; the flush path drains the same way with a time bound.
Both RP2040 BSPs and the port-validation example share the fixed code.

## Results so far

Interpretation and gate status are in `UPGRADE_QPCPP_V7.md`.

- Direct tick capture, 2 MSa/s, 600 s, DPP running, QSPY attached
  (`result_dpp_2MHz_600s.json`): 60,000 consecutive ticks, period mean
  9,999.998 µs, min 9,981.0, max 10,015.5, standard deviation 4.6 µs, no
  missed or doubled tick. Tick callback 34 µs typical, 51 µs p99, 70 µs max.
- 6 s captures at 1 MSa/s (`result_dpp_1MHz_6s_host.json`,
  `result_dpp_1MHz_6s_nohost.json`): period 10,000.00 µs mean, min 9,983,
  max 10,012; callback 34 µs typical, 49 µs p99, 60 to 62 µs max, the same
  with and without the trace host.
- The two-hour soak capture and its QSPY log are added when the run finishes.
