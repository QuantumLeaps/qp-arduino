# RP2040 port validation on the fixture — 2026-09-30

Host-driven validation of the qp-arduino QV port (QP/C++ v7.3.4) on the
recorded RP2040 fixture with `examples/port_validation-rp2040`: one Validator
active object that, every 100 Hz tick, publishes to itself, allocates a pool
event, posts it to itself and checks it, with all six QS record groups
enabled and QSPY attached over USB CDC. Fixture, host tools and channel map as
in `../2026-09-30-rp2040-blinky/README.md`; here CH17 = GP21 is the tick-ISR
probe and CH16 = GP25 carries a role selected by QS command 5 (1 dispatch,
2 idle, 3 event receipt). Interpretation and gate status are in
`UPGRADE_QPCPP_V7.md`.

> **Raw captures and large logs live outside this repository** to keep it
> small: `../qp-arduino-example/evidence/2026-09-30-rp2040-port-validation/` mirrors this directory
> and holds every `.sr`, `.vcd`, `.gz`, `.bin` and log over 100 KB named
> below. This directory keeps the READMEs, the analysis scripts, the
> `result_*.json` outputs, summaries and short logs.


## Host-driven procedures

`tools/port_validation_rp2040.py` (run as `pixi run validate-rp2040`) starts
`qspy`, attaches to its UDP front-end and drives the target over QS-RX.
`port_validation_results.json` is the passing run; the transcript tail and
QSPY session logs are alongside.

| Procedure | Checks | Result |
|---|---|---|
| attach | QSPY UDP front-end attach | pass |
| info | QS-RX `QS_RX_INFO` answered with target info (QP 7.3.3) | pass |
| ticking | tick rate from the Validator's own periodic reports | 100.0 ticks/s, events and dynamic events advance one per tick |
| publish | command 1 publishes `VALIDATE_SIG`; the subscriber dispatches it | pass |
| dynamic | command 2 posts a pool event: pool get, dispatch of `DYN_SIG`, pool put | pass |
| roles | command 5 switches the GP25 role; `PROBE_ROLE` echoes 0, 2, 3, 1 | pass |
| reset-count | command 7 resets, command 6 reports counters | pass |
| assertion | command 4 → `Q_ERROR_ID(400)`: `=ASSERT=` record, USB port disappears (watchdog reset), reappears, `Trg-RST` on re-attach, QS-RX alive again | pass |

`port_validation_results_first_run.json` is the first attempt, whose three
failures were driver defects (text-line expectation for a binary target-info
packet, periodic reports mistaken for query replies, and waiting for the
port while the old `qspy` still held the device node, which made the rebooted
board re-enumerate as a different tty). Use the stable
`/dev/serial/by-id/...` path with this driver.

## Analyzer captures

| File | Rate | GP25 role | Trace host |
|---|---|---|---|
| `captures/pv_role1_1MHz_6s_host.sr` | 1 MSa/s, 6 s | 1 dispatch | attached |
| `captures/pv_role1_20MHz_500ms_host.sr` | 20 MSa/s, 0.5 s | 1 dispatch | attached |
| `captures/pv_role1_1MHz_6s_nohost.sr` | 1 MSa/s, 6 s | 1 dispatch | detached |
| `captures/pv_role2_1MHz_6s_host.sr` | 1 MSa/s, 6 s | 2 idle | attached |
| `captures/pv_role3_1MHz_6s_host.sr` | 1 MSa/s, 6 s | 3 event receipt | attached |
| `captures/pv_role3_20MHz_500ms_host.sr` | 20 MSa/s, 0.5 s | 3 event receipt | attached |
| `captures/pv_wedged_snapshot.sr` | 1 MSa/s, 2 s untriggered | (see anomaly) | none |

Analyzed with `analysis/analyze_roles.py`; results in `analysis/result_*.json`.

### Results

Tick period in every capture: 9,999.99 to 10,000.00 µs mean, min 9,995, max
10,005 µs, no missed or doubled tick (601 ticks at 1 MSa/s, 51 at 20 MSa/s).

| Measurement | Trace host attached | Detached |
|---|---|---|
| dispatches per tick (role 1) | exactly 3 (TIMEOUT, VALIDATE, DYN) | exactly 3 |
| dispatch duration p50 / p99 / max | 20.5 / 53.7 / 82.6 µs (20 MSa/s) | 20 / 29 / 37 µs |
| tick ISR rise → first dispatch start, min / p50 / p99 / max | 41.3 / 91.0 / 121.1 / 121.1 µs (20 MSa/s); 40 / 92 / 121 / 133 µs (1 MSa/s, 599 ticks) | 40 / 52 / 63 / 66 µs |
| tick ISR rise → dynamic-event receipt (role 3, 20 MSa/s) | 148 / 209 / 244 / 244 µs | not captured |
| event pulses per tick (role 3) | 1.00 at 20 MSa/s (0.74 at 1 MSa/s: the 0.75 µs pulse is under-sampled) | |
| idle callback share of time (role 2) | 85.4 % high; ≈1,089 idle passes per tick, 8 µs each | |
| dispatch share of time (role 1) | 0.6 % | 0.5 % |

The idle share means the fixture spends about 15 % of its time outside the idle
callback under this load with all QS groups streaming to QSPY; dispatch itself
is under 1 %, so the remainder is the tick callback and the idle loop's USB CDC
service. The trace host roughly doubles the ISR-to-dispatch latency (66 → 133
µs maximum) because the QV loop finishes the idle callback's USB write before
it re-checks the ready set. Every latency stays below 3 % of the 10 ms tick.

## Anomaly

After the first (defective) driver run's assertion test and three role
sessions, the board stopped: both probes low, no QS output, but USB CDC still
enumerated and the 1200 baud touch still worked, which recovered it by
reflashing. `captures/pv_wedged_snapshot.sr` shows the silent state. The
follow-up run repeated the same sequence with the fixed driver and a final
liveness check; its outcome is recorded below and in the ledger. Cause not
yet identified.

The follow-up run (`followup-run.log`) passed all eight procedures, then held
role 1 and role 3 sessions and a liveness session; each attached QSPY received
tens of thousands of records (the liveness session 19,143 in 8 s, no errors),
and `captures/pv_after_roles_snapshot.sr` shows the tick and three dispatches
per tick still running. So the follow-up did **not** reproduce the stall; the
first-run stall remains a single unexplained occurrence. The first-run
transcript shows the target still executing QS-RX commands (role pulses
appeared) after its trace output had stopped, then the tick stopping within
the next ten seconds.

Two benign artefacts seen in these logs: `Trg-ERR 0x42` is the target
reporting a QS-RX sequence-number mismatch on the first packet of each new
host session (each session restarts its sequence counter), and a QSPY attached
after boot prints the `PROBE_ROLE` user record as `USER+001` because it never
received the dictionaries.
