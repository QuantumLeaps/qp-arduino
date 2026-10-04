# How the Arduino port works, and what a board support package must do

This page is for anyone writing a QP/C++ application on an Arduino board with
this library: what the port assumes, what the sketch and its board support
package (BSP) have to provide, the lessons learned while qualifying the port
on RP2040, and the measurements behind the support claim.

## What the library is

- QP/C++ 7.3.4 with the QV cooperative kernel and QS tracing, imported flat
  from the upstream tag; see [`provenance.md`](provenance.md).
- The port includes `Arduino.h`, forces `Q_SPY`, and keeps small defaults:
  `QF_MAX_ACTIVE` 8 on ARMv6-M (RP2040) and 16 on ARMv7-M, `QF_MAX_TICK_RATE`
  2. Raise them in your own port only if you know the RAM cost.
- Critical sections: PRIMASK (`cpsid i`) on ARMv6-M, BASEPRI `0x3F` on
  ARMv7-M. They do not nest. A QF query that takes its own critical section
  must not be called inside `QS_BEGIN`/`QS_END`.
- Only QV is included. `QF::run()` never returns: it dispatches events to
  active objects in priority order and calls `QV::onIdle()` when every queue
  is empty.

## The shape of a sketch

```cpp
#include "qpcpp.hpp"
#include "myapp.hpp"   // signals, event types, AO pointers
#include "bsp.hpp"

void setup() {
    QP::QF::init();
    BSP::init();                                  // pins, QS, nothing that waits forever
    QP::QF::poolInit(poolSto, sizeof(poolSto), sizeof(MyEvt));   // smallest events first
    QP::QActive::psInit(subscrSto, MAX_PUB_SIG);  // if anything publishes
    AO_MyActive->start(1U, queueSto, Q_DIM(queueSto), nullptr, 0U);
}

void loop() {
    QP::QF::run();   // does not return
}
```

Because `loop()` never completes, the Arduino core's `serialEvent()` hooks are
never called. On the arduino-pico core that costs nothing: USB is serviced
from an interrupt that a periodic alarm pends, so `Serial` keeps working inside
`QF::run()`. On other cores, check how USB and `millis()` are serviced before
assuming the same.

## What the BSP must provide

| Callback | Purpose | What the examples do |
|---|---|---|
| `BSP::init()` | pins, clocks, the trace transport, `QS_INIT` | wait a bounded time for a USB host before starting QS, then emit the dictionaries |
| a tick source calling `QP::QTimeEvt::tick(0U, &l_TICK_ID)` | the time base for time events | a hardware timer alarm the BSP owns, 100 Hz |
| `QV::onIdle()` | called with interrupts disabled when every queue is empty; must re-enable them | service the trace transport, then `QV_CPU_SLEEP()` |
| `QS::onStartup()`, `onCleanup()`, `onFlush()`, `onGetTime()`, `onCommand()` | the QS transport and clock | USB CDC, a UART or a PIO UART; `micros()` as the 32-bit timestamp |
| `Q_onError()` | the assertion handler | disable interrupts, show a distinct signature on the LED, reboot through the watchdog |
| `QF::onStartup()`, `onCleanup()` | kernel hooks | start the tick; nothing |

Keep every board detail here. The active objects should compile unchanged for
another board or another transport.

## Lessons from RP2040

Each of these came from a defect found with a logic analyzer or a trace
host cycling against the target, and each is fixed in the example BSPs.

**Own the tick timer.** A `repeating_timer` from the pico-sdk default alarm
pool, which the Arduino core shares with its USB task and `delay()`, stopped
silently after a few minutes of trace-host attach and detach: a failed
re-add ends a repeating timer without a word. The BSPs now claim a hardware
alarm (`hardware_alarm_claim_unused`), re-arm it from their own interrupt
before calling `QTimeEvt::tick()`, catch up if a target is already in the
past, and have the idle loop re-arm the alarm and emit a `TICK_LOST` record
if the tick ever stops. A watchdog of that kind must read the tick's
timestamp before the clock and compare a signed elapsed time, or a tick that
lands between the two reads looks like a stopped tick.

**Stay on core 0.** The core launches core 1 only when `setup1()` or
`loop1()` is defined. PRIMASK masks only the executing core, so QP calls from
core 1 would race the kernel; nothing in this package uses core 1.

**Service the USB CDC transport sparingly.** Every call into the CDC port
runs the USB stack's task. Serviced on every idle pass it inflated the tick
callback and the dispatch latency; the BSPs touch the port only when at least
64 QS bytes are pending or 1 ms has passed (`QP_ARDUINO_QS_SERVICE_BYTES`,
`QP_ARDUINO_QS_SERVICE_US`). USB CDC also discards everything written before
a host opens the port, so `BSP::init()` waits a bounded time for a host
(`QP_ARDUINO_QS_HOST_WAIT_MS`) before emitting the dictionaries.

**Take QS blocks with interrupts disabled.** `QS::getBlock()` changes the
ring buffer's shared state, and records are written from interrupts too, so
the drain must call it under `QF_INT_DISABLE()`, as every upstream Cortex-M
BSP does. With it unprotected, the trace corrupted and the target reset within
seconds under a full record set, while the default filters never showed it.

**Never block on the transport.** The drain asks the port for its free space
and takes at most that much from QS (`qsTransmitSome()`); `QS::onFlush()`
drains with a bound. The same discipline works for a hardware UART, whose
`availableForWrite()` reports one byte, and for a PIO UART, whose FIFO holds
eight.

**Do not flush inside dictionary re-emission.** Every QS dictionary function
ends in `QS::onFlush()`. With a synchronous flush, re-emitting the
dictionaries every 10 s at 115200 baud held the idle loop, and with it every
active object, for about 30 ms. The BSPs suppress the flush during
re-emission and let the copies drain through the idle loop.

**One record per burst.** A 2,048-byte QS buffer saturates a 115200 baud link
when an application emits one record for each of a few dozen lines queued in
the same millisecond. Summarise a burst in one record.

**Expect jitter from USB traffic.** The USB interrupt shares the alarm
interrupt's priority, so the tick jitter grows from about ±20 µs with no host
to about ±180 µs under heavy USB CDC traffic. The tick count is unaffected.
Raising the alarm interrupt's priority is a possible refinement.

**Known upstream gap.** `QActive::getQueueMin()` is declared in `qp.hpp` at
`v7.3.4` and defined nowhere, so `QF::getQueueMin()` fails to link. Read the
minimum through the active object's queue: `m_eQueue.getNMin()`.

## Observing a running system

For debugging on the bench, QS over USB CDC with QSPY attached, QS-RX enabled,
is the normal setup; QView and QUTest's command channel work over the same
link. For observing a deployed system, the examples offer a transmit-only
profile (`QP_ARDUINO_QS_TX_ONLY=1`): QS-RX is never read, so nothing on the
trace link can reset, command or poke the target; the dictionaries are
re-emitted every `QP_ARDUINO_QS_DICT_PERIOD_MS` so a capture that misses boot
is still decodable; and a `TRACE_STAT` record every
`QP_ARDUINO_QS_STAT_PERIOD_MS` carries the QS buffer's high-water mark and
the number of times it overflowed, so trace loss is reported by the target
rather than inferred by the host. The profile runs over UART0 or over a PIO
UART on any free GPIO (`QP_ARDUINO_QS_PIO_TX_PIN`), which lets a board whose
hardware UARTs are all spoken for still carry a trace to a logic analyzer.
A capture decoded offline with `qspy -f` is fully named after one dictionary
period.

## Measurements behind the support claim

Taken on an RP2040 board at 133 MHz with arduino-pico 4.2.0 and GCC 14.2.0,
with a logic analyzer at 1 MSa/s or more on the tick-probe and LED pins and
QSPY on USB CDC, between 2026-09-30 and 2026-10-01.

| Measure | Result |
|---|---|
| 100 Hz tick, Blinky, 600 s direct edge capture | mean 10,000.0 µs, stdev under 5 µs, none missed or doubled |
| 100 Hz tick, DPP, 2 h 15 min soak | 720,001 ticks, mean 9,999.995 µs, min 9,980, max 10,020, none missed or doubled |
| tick under 394 host attach/detach and 78 slow-reader cycles | 99.8 to 100.1 ticks/s in every cycle, no `TICK_LOST` |
| tick callback duration | about 4 µs typical; 12 µs p99 / 17 µs max with the PIO trace |
| interrupt to active-object dispatch, validation load, QSPY attached | p99 121 µs, max 133 µs |
| the same, host detached | max 66 µs |
| assertion-free soak, DPP | 2 h 15 min, 420,595 records, no assertion, no reset |
| transmit-only profile, Blinky, 600 s on a PIO UART | 134,042 bytes decoded offline, no checksum error, QS high-water 95 of 2,048 bytes |
| flash / RAM, Blinky, DPP, port validation, built for the Pico | 77,808 / 12,024 B; 81,060 / 12,588 B; 78,952 / 14,372 B |

An application-shaped load on this port, a line protocol inside one active
object with a 1 kHz tick, met the budgets it declared beforehand (QS
high-water 505 of 2,048 bytes with no overflow, queue minimum free 15 of 16,
pool minimum free 6 of 8) and saturated the trace exactly as predicted when
the full record set was enabled, with the application unaffected. Its cost
was dominated by its own formatting on the Cortex-M0+ and by the USB CDC
write, not by the kernel: QP's own records cost a few microseconds each.

What was not measured: Teensy 4.1 and Arduino Due on hardware (builds only),
on-target QUTest (the library always compiles `qv.cpp`, which a QUTest build
replaces; the host-driven procedures and the upstream host QUTest suites stand
in), and anything on core 1.

## Checking a new board or core version

Build and run `port_validation-rp2040` (or its Teensy counterpart) and drive
it with `tools/port_validation_rp2040.py`; then run
`tools/port_validation_stress.py` for an hour with a logic analyzer on the
probes. Pass means every procedure passes, no cycle stops the tick, and the
tick period histogram has one lobe with nothing missed or doubled. Record the
exact core, platform and toolchain versions with the result.
