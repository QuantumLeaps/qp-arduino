# Migrating from qp-arduino 6.9.x to 7.3.4

qp-arduino 7.3.4 packages QP/C++ 7.3.4 with the strict v7 API. A sketch or QM
model written for the 6.9.x line needs the changes below. QP's own
[release notes](https://www.state-machine.com/qpcpp/history.html) describe the
framework-level changes between 6.9 and 7.3 in full; this page lists what
actually shows up in an Arduino sketch and its BSP.

## Headers

Include `qpcpp.hpp` and nothing else from the framework. The v6 split headers
(`qep.hpp`, `qf.hpp`, `qpset.hpp`, `qassert.h`, `qep_port.hpp`, `qf_port.hpp`,
`qv_port.hpp`) no longer exist; the v7 port is `qp_port.hpp`, which
`qpcpp.hpp` includes for you.

## API changes that affect sketches and BSPs

| 6.9.x | 7.3.4 |
|---|---|
| `Q_onAssert(module, id)` | `Q_onError(module, id)` |
| static event `QEvt const e = { SIG, 0U, 0U };` | `static QEvt const e(SIG);` (constructor form) |
| `QF::TICK_X(rate, sender)` / `QF::tickX_(...)` | `QTimeEvt::tick(rate, sender)` |
| `QF::psInit(subscrSto, maxSignal)` | `QActive::psInit(subscrSto, maxSignal)` |
| `QF::PUBLISH(e, sender)` | `QActive::publish_(e, sender, qsId)` with an explicit QS id |
| `QF::poolInit(...)`, `QF::init()`, `QF::run()` | unchanged |
| `QActive::start(prio, qSto, qLen, stkSto, stkSize)` | unchanged for QV (pass `nullptr`, `0U` for the stack) |

`QP_API_VERSION` is `9999` by default, which disables every deprecated v6
name. Setting it lower is not supported by this package.

## Tracing

`Q_SPY` is always compiled in, as in 6.9.x. The examples trace by default
(the RP2040 BSPs define `QS_ON` unless `QP_ARDUINO_QS_OFF` is set, the Teensy
and SAM BSPs define it outright), and every BSP provides `QS::onStartup()`, `QS::onCleanup()`,
`QS::onFlush()`, `QS::onGetTime()` and `QS::onCommand()`. Note that this
port's critical sections do not nest: do not call a QF query that takes its
own critical section inside `QS_BEGIN`/`QS_END`.

## Kernel

Only the QV cooperative kernel is included, as before. `QV::onIdle()` is
called with interrupts disabled and must re-enable them, as upstream
documents; the examples service the trace transport there and then sleep
with `QV_CPU_SLEEP()` when nothing is pending.

## QM models

QM 6.1.1 generates v7 code. A 6.9.x model opened in QM 6.1.1 needs the same
API changes applied in its action code and BSP file sections; the framework
item's version in the model is updated by QM itself. The `blinky_bsp-*` and
`dpp_bsp-*` examples are the migrated versions of the 6.9.x models and show
each change in place.

## Board and architecture

`library.properties` now says `architectures=rp2040`. The Arduino IDE shows
"may be incompatible" on other boards; the library still compiles for Teensy
4.x and the Due, and the port itself requires an ARM Cortex-M target as it
always did. Classic AVR boards never worked and still do not.

## Version numbers

The package version equals the imported QP/C++ version plus a release count
in the patch digit: 7.3.4 is the first release of the 7.3 line, 7.3.5 would be
the next port-only release. `QP_VERSION_STR` inside the framework reports
`7.3.3` on this tag, which is an upstream artefact of its generated headers,
not a different framework version.
