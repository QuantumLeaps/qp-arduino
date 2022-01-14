/// @file
/// @brief QS receive channel services
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2a
/// Last updated on  2021-01-28
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses/>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qs_port.hpp"    // QS port
#include "qs_pkg.hpp"     // QS package-scope internal interface
#include "qf_pkg.hpp"     // QF package-scope internal interface
#include "qassert.h"      // QP assertions

namespace QP {

Q_DEFINE_THIS_MODULE("qs_rx")

//****************************************************************************
struct QS::QSrxPriv QS::rxPriv_; // QS-RX private data

//****************************************************************************
#if (QS_OBJ_PTR_SIZE == 1U)
    using QSObj = std::uint8_t;
#elif (QS_OBJ_PTR_SIZE == 2U)
    using QSObj = std::uint16_t;
#elif (QS_OBJ_PTR_SIZE == 4U)
    using QSObj = std::uint32_t;
#elif (QS_OBJ_PTR_SIZE == 8U)
    using QSObj = std::uint64_t;
#endif

#if (QS_FUN_PTR_SIZE == 1U)
    using QSFun = std::uint8_t;
#elif (QS_FUN_PTR_SIZE == 2U)
    using QSFun = std::uint16_t;
#elif (QS_FUN_PTR_SIZE == 4U)
    using QSFun = std::uint32_t;
#elif (QS_FUN_PTR_SIZE == 8U)
    using QSFun = std::uint64_t;
#endif

/// @cond
/// Exlcude the following internals from the Doxygen documentation
/// Extended-state variables used for parsing various QS-RX Records
struct CmdVar {
    std::uint32_t param1;
    std::uint32_t param2;
    std::uint32_t param3;
    std::uint8_t  idx;
    std::uint8_t  cmdId;
};

struct TickVar {
    std::uint_fast8_t rate;
};

struct PeekVar {
    std::uint16_t offs;
    std::uint8_t  size;
    std::uint8_t  num;
    std::uint8_t  idx;
};

struct PokeVar {
    std::uint32_t data;
    std::uint16_t offs;
    std::uint8_t  size;
    std::uint8_t  num;
    std::uint8_t  idx;
    std::uint8_t  fill;
};

struct FltVar {
    std::uint8_t data[16];
    std::uint8_t idx;
    std::uint8_t recId; // global/local
};

struct ObjVar {
    QSObj   addr;
    std::uint8_t idx;
    std::uint8_t kind; // see qs.hpp, enum QSpyObjKind
    std::uint8_t recId;
};

struct TPVar {  // Test-Probe
    QSFun    addr;
    std::uint32_t data;
    std::uint8_t  idx;
};

struct AFltVar {
    std::uint8_t prio;
};

struct EvtVar {
    QEvt    *e;
    std::uint8_t *p;
    QSignal  sig;
    std::uint16_t len;
    std::uint8_t  prio;
    std::uint8_t  idx;
};

// extended-state variables for the current QS-RX state
static struct ExtState {
    union Variant {
        CmdVar   cmd;
        TickVar  tick;
        PeekVar  peek;
        PokeVar  poke;
        FltVar   flt;
        AFltVar  aFlt;
        ObjVar   obj;
        EvtVar   evt;
        TPVar    tp;
    } var;
    std::uint8_t state;
    std::uint8_t esc;
    std::uint8_t seq;
    std::uint8_t chksum;
} l_rx;

enum RxStateEnum : std::uint8_t {
    WAIT4_SEQ,
    WAIT4_REC,
    WAIT4_INFO_FRAME,
    WAIT4_CMD_ID,
    WAIT4_CMD_PARAM1,
    WAIT4_CMD_PARAM2,
    WAIT4_CMD_PARAM3,
    WAIT4_CMD_FRAME,
    WAIT4_RESET_FRAME,
    WAIT4_TICK_RATE,
    WAIT4_TICK_FRAME,
    WAIT4_PEEK_OFFS,
    WAIT4_PEEK_SIZE,
    WAIT4_PEEK_NUM,
    WAIT4_PEEK_FRAME,
    WAIT4_POKE_OFFS,
    WAIT4_POKE_SIZE,
    WAIT4_POKE_NUM,
    WAIT4_POKE_DATA,
    WAIT4_POKE_FRAME,
    WAIT4_FILL_DATA,
    WAIT4_FILL_FRAME,
    WAIT4_FILTER_LEN,
    WAIT4_FILTER_DATA,
    WAIT4_FILTER_FRAME,
    WAIT4_OBJ_KIND,
    WAIT4_OBJ_ADDR,
    WAIT4_OBJ_FRAME,
    WAIT4_QUERY_KIND,
    WAIT4_QUERY_FRAME,
    WAIT4_EVT_PRIO,
    WAIT4_EVT_SIG,
    WAIT4_EVT_LEN,
    WAIT4_EVT_PAR,
    WAIT4_EVT_FRAME,
    WAIT4_TEST_SETUP_FRAME,
    WAIT4_TEST_TEARDOWN_FRAME,
    WAIT4_TEST_PROBE_DATA,
    WAIT4_TEST_PROBE_ADDR,
    WAIT4_TEST_PROBE_FRAME,
    WAIT4_TEST_CONTINUE_FRAME,
    ERROR_STATE
};

#ifdef Q_UTEST
    static struct TestData {
        TPVar     tpBuf[16]; // buffer of Test-Probes received so far
        std::uint8_t tpNum;  // current number of Test-Probes
        QSTimeCtr testTime;  // test time (tick counter)
    } l_testData;
#endif // Q_UTEST

// internal helper functions...
static void rxParseData_(std::uint8_t const b) noexcept;
static void rxHandleBadFrame_(std::uint8_t const state) noexcept;
static void rxReportAck_(enum QSpyRxRecords const recId) noexcept;
static void rxReportError_(std::uint8_t const code) noexcept;
static void rxReportDone_(enum QSpyRxRecords const recId) noexcept;
static void rxPoke_(void) noexcept;

//! Internal QS-RX function to take a transition in the QS-RX FSM
static inline void tran_(RxStateEnum const target) noexcept {
    l_rx.state = static_cast<std::uint8_t>(target);
}
/// @endcond


//****************************************************************************
/// @description
/// This function should be called from QS::onStartup() to provide QS-RX with
/// the receive data buffer.
///
/// @param[in]  sto[]   the address of the memory block
/// @param[in]  stoSize the size of this block [bytes]. The size of the
///                     QS RX buffer cannot exceed 64KB.
///
/// @note QS-RX can work with quite small data buffers, but you will start
/// losing data if the buffer is not drained fast enough in the idle task.
///
/// @note If the data input rate exceeds the QS-RX processing rate, the data
/// will be lost, but the QS protocol will notice that:
/// (1) that the checksum in the incomplete QS records will fail; and
/// (2) the sequence counter in QS records will show discontinuities.
///
/// The QS-RX channel will report any data errors by sending the
/// QS_RX_DATA_ERROR trace record.
///
void QS::rxInitBuf(std::uint8_t * const sto,
                   std::uint16_t const stoSize) noexcept
{
    rxPriv_.buf  = &sto[0];
    rxPriv_.end  = static_cast<QSCtr>(stoSize);
    rxPriv_.head = 0U;
    rxPriv_.tail = 0U;

    rxPriv_.currObj[QS::SM_OBJ] = nullptr;
    rxPriv_.currObj[QS::AO_OBJ] = nullptr;
    rxPriv_.currObj[QS::MP_OBJ] = nullptr;
    rxPriv_.currObj[QS::EQ_OBJ] = nullptr;
    rxPriv_.currObj[QS::TE_OBJ] = nullptr;
    rxPriv_.currObj[QS::AP_OBJ] = nullptr;

    tran_(WAIT4_SEQ);
    l_rx.esc    = 0U;
    l_rx.seq    = 0U;
    l_rx.chksum = 0U;

    beginRec_(static_cast<std::uint_fast8_t>(QS_OBJ_DICT));
        QS_OBJ_PRE_(&rxPriv_);
        QS_STR_PRE_("QS_RX");
    endRec_();
    // no QS_REC_DONE(), because QS is not running yet

#ifdef Q_UTEST
    l_testData.tpNum    = 0U;
    l_testData.testTime = 0U;
#endif // Q_UTEST
}

//****************************************************************************
/// @description
/// This function is intended to be called from the ISR that reads the QS-RX
/// bytes from the QSPY application. The function returns the conservative
/// number of free bytes currently available in the buffer, assuming that
/// the head pointer is not being moved concurrently. The tail pointer might
/// be moving, meaning that bytes can be concurrently removed from the buffer.
///
std::uint16_t QS::rxGetNfree(void) noexcept {
    QSCtr head = rxPriv_.head;
    if (head == rxPriv_.tail) { // buffer empty?
        return static_cast<std::uint16_t>(rxPriv_.end - 1U);
    }
    else if (head < rxPriv_.tail) {
        return static_cast<std::uint16_t>(rxPriv_.tail - head - 1U);
    }
    else {
        return static_cast<std::uint16_t>(rxPriv_.end + rxPriv_.tail
                                          - head - 1U);
    }
}

//****************************************************************************
///
/// @description
/// This function programmatically sets the "current object" in the Target.
///
void QS::setCurrObj(std::uint8_t obj_kind, void *obj_ptr) noexcept {

    Q_REQUIRE_ID(100, obj_kind < Q_DIM(rxPriv_.currObj));
    rxPriv_.currObj[obj_kind] = obj_ptr;
}

//****************************************************************************
///
/// @description
/// This function programmatically generates the response to the query for
/// a "current object".
///
void QS::queryCurrObj(std::uint8_t obj_kind) noexcept {
    Q_REQUIRE_ID(200, obj_kind < Q_DIM(rxPriv_.currObj));

    if (QS::rxPriv_.currObj[obj_kind] != nullptr) {
        QS_CRIT_STAT_
        QS_CRIT_E_();
        QS::beginRec_(static_cast<std::uint_fast8_t>(QS_QUERY_DATA));
            QS_TIME_PRE_(); // timestamp
            QS_U8_PRE_(obj_kind);  // object kind
            QS_OBJ_PRE_(QS::rxPriv_.currObj[obj_kind]); // object pointer
            switch (obj_kind) {
                case SM_OBJ: // intentionally fall through
                case AO_OBJ:
                    QS_FUN_PRE_(
                        reinterpret_cast<QHsm *>(
                            QS::rxPriv_.currObj[obj_kind])->getStateHandler());
                    break;
                case QS::MP_OBJ:
                    QS_MPC_PRE_(reinterpret_cast<QMPool *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_nFree);
                    QS_MPC_PRE_(reinterpret_cast<QMPool *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_nMin);
                    break;
                case QS::EQ_OBJ:
                    QS_EQC_PRE_(reinterpret_cast<QEQueue *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_nFree);
                    QS_EQC_PRE_(reinterpret_cast<QEQueue *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_nMin);
                    break;
                case QS::TE_OBJ:
                    QS_OBJ_PRE_(reinterpret_cast<QTimeEvt *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_act);
                    QS_TEC_PRE_(reinterpret_cast<QTimeEvt *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_ctr);
                    QS_TEC_PRE_(reinterpret_cast<QTimeEvt *>(
                                   QS::rxPriv_.currObj[obj_kind])->m_interval);
                    QS_SIG_PRE_(reinterpret_cast<QTimeEvt *>(
                                   QS::rxPriv_.currObj[obj_kind])->sig);
                    QS_U8_PRE_ (reinterpret_cast<QTimeEvt *>(
                                   QS::rxPriv_.currObj[obj_kind])->refCtr_);
                    break;
                default:
                    break;
            }
        QS::endRec_();
        QS_CRIT_X_();

        QS_REC_DONE(); // user callback (if defined)
    }
    else {
        rxReportError_(static_cast<std::uint8_t>(QS_RX_AO_FILTER));
    }
    // no need to report Done
}

//****************************************************************************
void QS::rxParse(void) {
    QSCtr tail = rxPriv_.tail;
    while (rxPriv_.head != tail) { // QS-RX buffer NOT empty?
        std::uint8_t b = rxPriv_.buf[tail];

        ++tail;
        if (tail == rxPriv_.end) {
            tail = 0U;
        }
        rxPriv_.tail = tail; // update the tail to a *valid* index

        if (l_rx.esc != 0U) {  // escaped byte arrived?
            l_rx.esc = 0U;
            b ^= QS_ESC_XOR;

            l_rx.chksum += b;
            rxParseData_(b);
        }
        else if (b == QS_ESC) {
            l_rx.esc = 1U;
        }
        else if (b == QS_FRAME) {
            // get ready for the next frame
            b = l_rx.state; // save the current state in b
            l_rx.esc = 0U;
            tran_(WAIT4_SEQ);

            if (l_rx.chksum == QS_GOOD_CHKSUM) {
                l_rx.chksum = 0U;
                rxHandleGoodFrame_(b);
            }
            else { // bad checksum
                l_rx.chksum = 0U;
                rxReportError_(0x00U);
                rxHandleBadFrame_(b);
            }
        }
        else {
            l_rx.chksum += b;
            rxParseData_(b);
        }
    }
}

//****************************************************************************
static void rxParseData_(std::uint8_t const b) noexcept {
    switch (l_rx.state) {
        case WAIT4_SEQ: {
            ++l_rx.seq;
            if (l_rx.seq != b) { // not the expected sequence?
                rxReportError_(0x42U);
                l_rx.seq = b; // update the sequence
            }
            tran_(WAIT4_REC);
            break;
        }
        case WAIT4_REC: {
            switch (b) {
                case QS_RX_INFO:
                    tran_(WAIT4_INFO_FRAME);
                    break;
                case QS_RX_COMMAND:
                    tran_(WAIT4_CMD_ID);
                    break;
                case QS_RX_RESET:
                    tran_(WAIT4_RESET_FRAME);
                    break;
                case QS_RX_TICK:
                    tran_(WAIT4_TICK_RATE);
                    break;
                case QS_RX_PEEK:
                    if (QS::rxPriv_.currObj[QS::AP_OBJ] != nullptr) {
                        l_rx.var.peek.offs = 0U;
                        l_rx.var.peek.idx  = 0U;
                        tran_(WAIT4_PEEK_OFFS);
                    }
                    else {
                        rxReportError_(static_cast<std::uint8_t>(QS_RX_PEEK));
                        tran_(ERROR_STATE);
                    }
                    break;
                case QS_RX_POKE:
                case QS_RX_FILL:
                    l_rx.var.poke.fill =
                        (b == static_cast<std::uint8_t>(QS_RX_FILL))
                            ? 1U
                            : 0U;
                    if (QS::rxPriv_.currObj[QS::AP_OBJ] != nullptr) {
                        l_rx.var.poke.offs = 0U;
                        l_rx.var.poke.idx  = 0U;
                        tran_(WAIT4_POKE_OFFS);
                    }
                    else {
                        rxReportError_(
                            (l_rx.var.poke.fill != 0U)
                                ? static_cast<std::uint8_t>(QS_RX_FILL)
                                : static_cast<std::uint8_t>(QS_RX_POKE));
                        tran_(ERROR_STATE);
                    }
                    break;
                case QS_RX_GLB_FILTER: // intentionally fall-through
                case QS_RX_LOC_FILTER:
                    l_rx.var.flt.recId = b;
                    tran_(WAIT4_FILTER_LEN);
                    break;
                case QS_RX_AO_FILTER: // intentionally fall-through
                case QS_RX_CURR_OBJ:
                    l_rx.var.obj.recId = b;
                    tran_(WAIT4_OBJ_KIND);
                    break;
                case QS_RX_QUERY_CURR:
                    l_rx.var.obj.recId =
                        static_cast<std::uint8_t>(QS_RX_QUERY_CURR);
                    tran_(WAIT4_QUERY_KIND);
                    break;
                case QS_RX_EVENT:
                    tran_(WAIT4_EVT_PRIO);
                    break;

#ifdef Q_UTEST
                case QS_RX_TEST_SETUP:
                    tran_(WAIT4_TEST_SETUP_FRAME);
                    break;
                case QS_RX_TEST_TEARDOWN:
                    tran_(WAIT4_TEST_TEARDOWN_FRAME);
                    break;
                case QS_RX_TEST_CONTINUE:
                    tran_(WAIT4_TEST_CONTINUE_FRAME);
                    break;
                case QS_RX_TEST_PROBE:
                    if (l_testData.tpNum
                        < static_cast<std::uint8_t>(
                              (sizeof(l_testData.tpBuf)
                               / sizeof(l_testData.tpBuf[0]))))
                    {
                        l_rx.var.tp.data = 0U;
                        l_rx.var.tp.idx  = 0U;
                        tran_(WAIT4_TEST_PROBE_DATA);
                    }
                    else { // the number of Test-Probes exceeded
                        rxReportError_(
                            static_cast<std::uint8_t>(QS_RX_TEST_PROBE));
                        tran_(ERROR_STATE);
                    }
                    break;
#endif // Q_UTEST

                default:
                    rxReportError_(0x43U);
                    tran_(ERROR_STATE);
                    break;
            }
            break;
        }
        case WAIT4_INFO_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_CMD_ID: {
            l_rx.var.cmd.cmdId  = b;
            l_rx.var.cmd.idx    = 0U;
            l_rx.var.cmd.param1 = 0U;
            l_rx.var.cmd.param2 = 0U;
            l_rx.var.cmd.param3 = 0U;
            tran_(WAIT4_CMD_PARAM1);
            break;
        }
        case WAIT4_CMD_PARAM1: {
            l_rx.var.cmd.param1 |=
                (static_cast<std::uint32_t>(b) << l_rx.var.cmd.idx);
            l_rx.var.cmd.idx    += 8U;
            if (l_rx.var.cmd.idx == (8U*4U)) {
                l_rx.var.cmd.idx = 0U;
                tran_(WAIT4_CMD_PARAM2);
            }
            break;
        }
        case WAIT4_CMD_PARAM2: {
            l_rx.var.cmd.param2 |=
                static_cast<std::uint32_t>(b) << l_rx.var.cmd.idx;
            l_rx.var.cmd.idx    += 8U;
            if (l_rx.var.cmd.idx == (8U*4U)) {
                l_rx.var.cmd.idx = 0U;
                tran_(WAIT4_CMD_PARAM3);
            }
            break;
        }
        case WAIT4_CMD_PARAM3: {
            l_rx.var.cmd.param3 |=
                static_cast<std::uint32_t>(b) << l_rx.var.cmd.idx;
            l_rx.var.cmd.idx    += 8U;
            if (l_rx.var.cmd.idx == (8U*4U)) {
                l_rx.var.cmd.idx = 0U;
                tran_(WAIT4_CMD_FRAME);
            }
            break;
        }
        case WAIT4_CMD_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_RESET_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TICK_RATE: {
            l_rx.var.tick.rate = static_cast<std::uint_fast8_t>(b);
            tran_(WAIT4_TICK_FRAME);
            break;
        }
        case WAIT4_TICK_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_PEEK_OFFS: {
            if (l_rx.var.peek.idx == 0U) {
                l_rx.var.peek.offs = static_cast<std::uint16_t>(b);
                l_rx.var.peek.idx += 8U;
            }
            else {
                l_rx.var.peek.offs |= static_cast<std::uint16_t>(
                                         static_cast<std::uint16_t>(b) << 8U);
                tran_(WAIT4_PEEK_SIZE);
            }
            break;
        }
        case WAIT4_PEEK_SIZE: {
            if ((b == 1U) || (b == 2U) || (b == 4U)) {
                l_rx.var.peek.size = b;
                tran_(WAIT4_PEEK_NUM);
            }
            else {
                rxReportError_(static_cast<std::uint8_t>(QS_RX_PEEK));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_PEEK_NUM: {
            l_rx.var.peek.num = b;
            tran_(WAIT4_PEEK_FRAME);
            break;
        }
        case WAIT4_PEEK_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_POKE_OFFS: {
            if (l_rx.var.poke.idx == 0U) {
                l_rx.var.poke.offs = static_cast<std::uint16_t>(b);
                l_rx.var.poke.idx  = 1U;
            }
            else {
                l_rx.var.poke.offs |= static_cast<std::uint16_t>(
                                         static_cast<std::uint16_t>(b) << 8U);
                tran_(WAIT4_POKE_SIZE);
            }
            break;
        }
        case WAIT4_POKE_SIZE: {
            if ((b == 1U)
                || (b == 2U)
                || (b == 4U))
            {
                l_rx.var.poke.size = b;
                tran_(WAIT4_POKE_NUM);
            }
            else {
                rxReportError_((l_rx.var.poke.fill != 0U)
                                  ? static_cast<std::uint8_t>(QS_RX_FILL)
                                  : static_cast<std::uint8_t>(QS_RX_POKE));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_POKE_NUM: {
            if (b > 0U) {
                l_rx.var.poke.num  = b;
                l_rx.var.poke.data = 0U;
                l_rx.var.poke.idx  = 0U;
                tran_((l_rx.var.poke.fill != 0U)
                            ? WAIT4_FILL_DATA
                            : WAIT4_POKE_DATA);
            }
            else {
                rxReportError_((l_rx.var.poke.fill != 0U)
                                  ? static_cast<std::uint8_t>(QS_RX_FILL)
                                  : static_cast<std::uint8_t>(QS_RX_POKE));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_FILL_DATA: {
            l_rx.var.poke.data |=
                static_cast<std::uint32_t>(b) << l_rx.var.poke.idx;
            l_rx.var.poke.idx += 8U;
            if ((l_rx.var.poke.idx >> 3U) == l_rx.var.poke.size) {
                tran_(WAIT4_FILL_FRAME);
            }
            break;
        }
        case WAIT4_POKE_DATA: {
            l_rx.var.poke.data |=
                static_cast<std::uint32_t>(b) << l_rx.var.poke.idx;
            l_rx.var.poke.idx += 8U;
            if ((l_rx.var.poke.idx >> 3U) == l_rx.var.poke.size) {
                rxPoke_();
                --l_rx.var.poke.num;
                if (l_rx.var.poke.num == 0U) {
                    tran_(WAIT4_POKE_FRAME);
                }
            }
            break;
        }
        case WAIT4_FILL_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_POKE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_FILTER_LEN: {
            if (b == static_cast<std::uint8_t>(sizeof(l_rx.var.flt.data))) {
                l_rx.var.flt.idx = 0U;
                tran_(WAIT4_FILTER_DATA);
            }
            else {
                rxReportError_(l_rx.var.flt.recId);
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_FILTER_DATA: {
            l_rx.var.flt.data[l_rx.var.flt.idx] = b;
            ++l_rx.var.flt.idx;
            if (l_rx.var.flt.idx == sizeof(l_rx.var.flt.data)) {
                tran_(WAIT4_FILTER_FRAME);
            }
            break;
        }
        case WAIT4_FILTER_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_OBJ_KIND: {
            if (b <= static_cast<std::uint8_t>(QS::SM_AO_OBJ)) {
                l_rx.var.obj.kind = b;
                l_rx.var.obj.addr = 0U;
                l_rx.var.obj.idx  = 0U;
                tran_(WAIT4_OBJ_ADDR);
            }
            else {
                rxReportError_(l_rx.var.obj.recId);
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_OBJ_ADDR: {
            l_rx.var.obj.addr |=
                static_cast<QSObj>(b) << l_rx.var.obj.idx;
            l_rx.var.obj.idx += 8U;
            if (l_rx.var.obj.idx
                == (8U * static_cast<unsigned>(QS_OBJ_PTR_SIZE)))
            {
                tran_(WAIT4_OBJ_FRAME);
            }
            break;
        }
        case WAIT4_OBJ_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_QUERY_KIND: {
            if (b < static_cast<std::uint8_t>(QS::MAX_OBJ)) {
                l_rx.var.obj.kind = b;
                tran_(WAIT4_QUERY_FRAME);
            }
            else {
                rxReportError_(l_rx.var.obj.recId);
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_QUERY_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_EVT_PRIO: {
            l_rx.var.evt.prio = b;
            l_rx.var.evt.sig  = 0U;
            l_rx.var.evt.idx  = 0U;
            tran_(WAIT4_EVT_SIG);
            break;
        }
        case WAIT4_EVT_SIG: {
            l_rx.var.evt.sig |= static_cast<QSignal>(
                          static_cast<std::uint32_t>(b) << l_rx.var.evt.idx);
            l_rx.var.evt.idx += 8U;
            if (l_rx.var.evt.idx
                == (8U *static_cast<unsigned>(Q_SIGNAL_SIZE)))
            {
                l_rx.var.evt.len = 0U;
                l_rx.var.evt.idx = 0U;
                tran_(WAIT4_EVT_LEN);
            }
            break;
        }
        case WAIT4_EVT_LEN: {
            l_rx.var.evt.len |= static_cast<std::uint16_t>(
                                static_cast<unsigned>(b) << l_rx.var.evt.idx);
            l_rx.var.evt.idx += 8U;
            if (l_rx.var.evt.idx == (8U * 2U)) {
                if ((l_rx.var.evt.len + sizeof(QEvt))
                    <= static_cast<std::uint16_t>(QF::poolGetMaxBlockSize()))
                {
                    // report Ack before generating any other QS records
                    rxReportAck_(QS_RX_EVENT);

                    l_rx.var.evt.e = QF::newX_(
                        (static_cast<std::uint_fast16_t>(l_rx.var.evt.len)
                         + sizeof(QEvt)),
                        0U, // margin
                        static_cast<enum_t>(l_rx.var.evt.sig));
                    // event allocated?
                    if (l_rx.var.evt.e != nullptr) {
                        l_rx.var.evt.p =
                            reinterpret_cast<std::uint8_t *>(l_rx.var.evt.e);
                        l_rx.var.evt.p += sizeof(QEvt);
                        if (l_rx.var.evt.len > 0U) {
                            tran_(WAIT4_EVT_PAR);
                        }
                        else {
                            tran_(WAIT4_EVT_FRAME);
                        }
                    }
                    else {
                        rxReportError_(static_cast<std::uint8_t>(QS_RX_EVENT));
                        tran_(ERROR_STATE);
                    }
                }
                else {
                    rxReportError_(static_cast<std::uint8_t>(QS_RX_EVENT));
                    tran_(ERROR_STATE);
                }
            }
            break;
        }
        case WAIT4_EVT_PAR: { // event parameters
            *l_rx.var.evt.p = b;
            ++l_rx.var.evt.p;
            --l_rx.var.evt.len;
            if (l_rx.var.evt.len == 0U) {
                tran_(WAIT4_EVT_FRAME);
            }
            break;
        }
        case WAIT4_EVT_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }

#ifdef Q_UTEST
        case WAIT4_TEST_SETUP_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_TEARDOWN_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_CONTINUE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_PROBE_DATA: {
            l_rx.var.tp.data |=
                (static_cast<QSFun>(b) << l_rx.var.tp.idx);
            l_rx.var.tp.idx += 8U;
            if (l_rx.var.tp.idx == (8U * sizeof(std::uint32_t))) {
                l_rx.var.tp.addr = 0U;
                l_rx.var.tp.idx  = 0U;
                tran_(WAIT4_TEST_PROBE_ADDR);
            }
            break;
        }
        case WAIT4_TEST_PROBE_ADDR: {
            l_rx.var.tp.addr |=
                (static_cast<std::uint32_t>(b) << l_rx.var.tp.idx);
            l_rx.var.tp.idx += 8U;
            if (l_rx.var.tp.idx
                == (8U * static_cast<unsigned>(QS_FUN_PTR_SIZE)))
            {
                tran_(WAIT4_TEST_PROBE_FRAME);
            }
            break;
        }
        case WAIT4_TEST_PROBE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
#endif // Q_UTEST

        case ERROR_STATE: {
            // keep ignoring the data until a good frame is collected
            break;
        }
        default: { // unexpected or unimplemented state
            rxReportError_(0x45U);
            tran_(ERROR_STATE);
            break;
        }
    }
}

//****************************************************************************
void QS::rxHandleGoodFrame_(std::uint8_t const state) {
    std::uint8_t i;
    std::uint8_t *ptr;
    QS_CRIT_STAT_

    switch (state) {
        case WAIT4_INFO_FRAME: {
            // no need to report Ack or Done
            QS_CRIT_E_();
            QS_target_info_(0U); // send only Target info
            QS_CRIT_X_();
            break;
        }
        case WAIT4_RESET_FRAME: {
            // no need to report Ack or Done, because Target resets
            QS::onReset(); // reset the Target
            break;
        }
        case WAIT4_CMD_PARAM1: // intentionally fall-through
        case WAIT4_CMD_PARAM2: // intentionally fall-through
        case WAIT4_CMD_PARAM3: // intentionally fall-through
        case WAIT4_CMD_FRAME: {
            rxReportAck_(QS_RX_COMMAND);
            QS::onCommand(l_rx.var.cmd.cmdId, l_rx.var.cmd.param1,
                          l_rx.var.cmd.param2, l_rx.var.cmd.param3);
#ifdef Q_UTEST
            QS::processTestEvts_(); // process all events produced
#endif
            rxReportDone_(QS_RX_COMMAND);
            break;
        }
        case WAIT4_TICK_FRAME: {
            rxReportAck_(QS_RX_TICK);
#ifdef Q_UTEST
            QS::tickX_(l_rx.var.tick.rate, &QS::rxPriv_); // process tick
            QS::processTestEvts_(); // process all events produced
#else
            QF::tickX_(static_cast<std::uint_fast8_t>(l_rx.var.tick.rate),
                       &QS::rxPriv_);
#endif
            rxReportDone_(QS_RX_TICK);
            break;
        }
        case WAIT4_PEEK_FRAME: {
            // no need to report Ack or Done
            QS_CRIT_E_();
            QS::beginRec_(static_cast<std::uint_fast8_t>(QS_PEEK_DATA));
                ptr = (static_cast<std::uint8_t*>(
                           QS::rxPriv_.currObj[QS::AP_OBJ])
                       + l_rx.var.peek.offs);
                QS_TIME_PRE_();                  // timestamp
                QS_U16_PRE_(l_rx.var.peek.offs); // data offset
                QS_U8_PRE_(l_rx.var.peek.size);  // data size
                QS_U8_PRE_(l_rx.var.peek.num);   // number of data items
                for (i = 0U; i < l_rx.var.peek.num; ++i) {
                    switch (l_rx.var.peek.size) {
                        case 1:
                            QS_U8_PRE_(*(ptr + i));
                            break;
                        case 2:
                            QS_U16_PRE_(
                                *(reinterpret_cast<std::uint16_t*>(ptr) + i));
                            break;
                        case 4:
                            QS_U32_PRE_(
                                *(reinterpret_cast<std::uint32_t*>(ptr) + i));
                            break;
                        default:
                            break;
                    }
                }
            QS::endRec_();
            QS_CRIT_X_();

            QS_REC_DONE(); // user callback (if defined)
            break;
        }
        case WAIT4_POKE_DATA: {
            // received less than expected poke data items
            rxReportError_(static_cast<std::uint8_t>(QS_RX_POKE));
            break;
        }
        case WAIT4_POKE_FRAME: {
            rxReportAck_(QS_RX_POKE);
            // no need to report done
            break;
        }
        case WAIT4_FILL_FRAME: {
            rxReportAck_(QS_RX_FILL);
            ptr = (static_cast<std::uint8_t *>(
                       QS::rxPriv_.currObj[QS::AP_OBJ])
                   + l_rx.var.poke.offs);
            for (i = 0U; i < l_rx.var.poke.num; ++i) {
                switch (l_rx.var.poke.size) {
                    case 1:
                        *(ptr + i) =
                            static_cast<std::uint8_t>(l_rx.var.poke.data);
                        break;
                    case 2:
                        *(reinterpret_cast<std::uint16_t *>(ptr) + i) =
                            static_cast<std::uint16_t>(l_rx.var.poke.data);
                        break;
                    case 4:
                        *(reinterpret_cast<std::uint32_t *>(ptr) + i) =
                            l_rx.var.poke.data;
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case WAIT4_FILTER_FRAME: {
            rxReportAck_(static_cast<enum QSpyRxRecords>(l_rx.var.flt.recId));

            // apply the received filters
            if (l_rx.var.flt.recId
                == static_cast<std::uint8_t>(QS_RX_GLB_FILTER))
            {
                for (i = 0U;
                     i < static_cast<std::uint8_t>(sizeof(priv_.glbFilter));
                     ++i)
                {
                    priv_.glbFilter[i] = l_rx.var.flt.data[i];
                }
                // leave the "not maskable" filters enabled,
                // see qs.h, Miscellaneous QS records (not maskable)
                //
                priv_.glbFilter[0] |= 0x01U;
                priv_.glbFilter[7] |= 0xFCU;
                priv_.glbFilter[8] |= 0x7FU;

                // never enable the last 3 records (0x7D, 0x7E, 0x7F)
                priv_.glbFilter[15] &= 0x1FU;
            }
            else if (l_rx.var.flt.recId
                     == static_cast<std::uint8_t>(QS_RX_LOC_FILTER))
            {
                for (i = 0U; i < Q_DIM(priv_.locFilter); ++i) {
                    priv_.locFilter[i] = l_rx.var.flt.data[i];
                }
                // leave QS_ID == 0 always on
                priv_.locFilter[0] |= 0x01U;
            }
            else {
                rxReportError_(l_rx.var.flt.recId);
            }

            // no need to report Done
            break;
        }
        case WAIT4_OBJ_FRAME: {
            i = l_rx.var.obj.kind;
            if (i < static_cast<std::uint8_t>(QS::MAX_OBJ)) {
                if (l_rx.var.obj.recId
                    == static_cast<std::uint8_t>(QS_RX_CURR_OBJ))
                {
                    rxPriv_.currObj[i] =
                        reinterpret_cast<void *>(l_rx.var.obj.addr);
                    rxReportAck_(QS_RX_CURR_OBJ);
                }
                else if (l_rx.var.obj.recId
                         == static_cast<std::uint8_t>(QS_RX_AO_FILTER))
                {
                    if (l_rx.var.obj.addr != 0U) {
                        std::int_fast16_t filter =
                            static_cast<std::int_fast16_t>(
                                reinterpret_cast<QActive *>(
                                    l_rx.var.obj.addr)->m_prio);
                        locFilter_((i == 0)
                            ? filter
                            :-filter);
                        rxReportAck_(QS_RX_AO_FILTER);
                    }
                    else {
                        rxReportError_(QS_RX_AO_FILTER);
                    }
                }
                else {
                    rxReportError_(l_rx.var.obj.recId);
                }
            }
            // both SM and AO
            else if (i == static_cast<std::uint8_t>(QS::SM_AO_OBJ)) {
                if (l_rx.var.obj.recId
                    == static_cast<std::uint8_t>(QS_RX_CURR_OBJ))
                {
                    rxPriv_.currObj[SM_OBJ] = (void *)l_rx.var.obj.addr;
                    rxPriv_.currObj[AO_OBJ] = (void *)l_rx.var.obj.addr;
                }
                rxReportAck_(
                    static_cast<enum QSpyRxRecords>(l_rx.var.obj.recId));
            }
            else {
                rxReportError_(l_rx.var.obj.recId);
            }
            break;
        }
        case WAIT4_QUERY_FRAME: {
            queryCurrObj(l_rx.var.obj.kind);
            break;
        }
        case WAIT4_EVT_FRAME: {
            // NOTE: Ack was already reported in the WAIT4_EVT_LEN state
#ifdef Q_UTEST
            QS::onTestEvt(l_rx.var.evt.e); // "massage" the event, if needed
#endif // Q_UTEST
            // use 'i' as status, 0 == success,no-recycle
            i = 0U;

            if (l_rx.var.evt.prio == 0U) { // publish
                QF::publish_(l_rx.var.evt.e, &QS::rxPriv_, 0U);
            }
            else if (l_rx.var.evt.prio < QF_MAX_ACTIVE) {
                if (!QF::active_[l_rx.var.evt.prio]->POST_X(
                                l_rx.var.evt.e,
                                0U, // margin
                                &QS::rxPriv_))
                {
                    // failed QACTIVE_POST() recycles the event
                    i = 0x80U; // failure, no recycle
                }
            }
            else if (l_rx.var.evt.prio == 255U) {
                // dispatch to the current SM object
                if (QS::rxPriv_.currObj[QS::SM_OBJ] != nullptr) {
                    // increment the ref-ctr to simulate the situation
                    // when the event is just retreived from a queue.
                    // This is expected for the following QF::gc() call.
                    //
                    QF_EVT_REF_CTR_INC_(l_rx.var.evt.e);

                    static_cast<QHsm *>(QS::rxPriv_.currObj[QS::SM_OBJ])
                            ->dispatch(l_rx.var.evt.e, 0U);
                    i = 0x01U;  // success, recycle
                }
                else {
                    i = 0x81U;  // failure, recycle
                }
            }
            else if (l_rx.var.evt.prio == 254U) {
                // init the current SM object"
                if (QS::rxPriv_.currObj[QS::SM_OBJ] != nullptr) {
                    // increment the ref-ctr to simulate the situation
                    // when the event is just retreived from a queue.
                    // This is expected for the following QF::gc() call.
                    //
                    QF_EVT_REF_CTR_INC_(l_rx.var.evt.e);

                    static_cast<QHsm *>(QS::rxPriv_.currObj[QS::SM_OBJ])
                            ->init(l_rx.var.evt.e, 0U);
                    i = 0x01U;  // success, recycle
                }
                else {
                    i = 0x81U;  // failure, recycle
                }
            }
            else if (l_rx.var.evt.prio == 253U) {
                // post to the current AO
                if (QS::rxPriv_.currObj[QS::AO_OBJ] != nullptr) {
                    if (!static_cast<QActive *>(
                            QS::rxPriv_.currObj[QS::AO_OBJ])->POST_X(
                                l_rx.var.evt.e,
                                0U, // margin
                                &QS::rxPriv_))
                    {
                        // failed QACTIVE_POST() recycles the event
                        i = 0x80U; // failure, no recycle
                    }
                }
                else {
                    i = 0x81U;  // failure, recycle
                }
            }
            else {
                i = 0x81U;  // failure, recycle
            }

            // recycle needed?
            if ((i & 1U) != 0U) {
                QF::gc(l_rx.var.evt.e);
            }
            // failure?
            if ((i & 0x80U) != 0U) {
                rxReportError_(static_cast<std::uint8_t>(QS_RX_EVENT));
            }
            else {
#ifdef Q_UTEST
                QS::processTestEvts_(); // process all events produced
#endif
                rxReportDone_(QS_RX_EVENT);
            }
            break;
        }

#ifdef Q_UTEST
        case WAIT4_TEST_SETUP_FRAME: {
            rxReportAck_(QS_RX_TEST_SETUP);
            l_testData.tpNum = 0U; // clear Test-Probes
            l_testData.testTime = 0U; //clear time tick
            // don't clear current objects
            QS::onTestSetup(); // application-specific test setup
            // no need to report Done
            break;
        }
        case WAIT4_TEST_TEARDOWN_FRAME: {
            rxReportAck_(QS_RX_TEST_TEARDOWN);
            QS::onTestTeardown(); // application-specific test teardown
            // no need to report Done
            break;
        }
        case WAIT4_TEST_CONTINUE_FRAME: {
            rxReportAck_(QS_RX_TEST_CONTINUE);
            QS::rxPriv_.inTestLoop = false; // exit the QUTest loop
            // no need to report Done
            break;
        }
        case WAIT4_TEST_PROBE_FRAME: {
            rxReportAck_(QS_RX_TEST_PROBE);
            Q_ASSERT_ID(815,
                l_testData.tpNum
                   < (sizeof(l_testData.tpBuf) / sizeof(l_testData.tpBuf[0])));
            l_testData.tpBuf[l_testData.tpNum] = l_rx.var.tp;
            ++l_testData.tpNum;
            // no need to report Done
            break;
        }
#endif // Q_UTEST

        case ERROR_STATE: {
            // keep ignoring all bytes until new frame
            break;
        }
        default: {
            rxReportError_(0x47U);
            break;
        }
    }
}

//****************************************************************************
static void rxHandleBadFrame_(std::uint8_t const state) noexcept {
    rxReportError_(0x50U); // error for all bad frames
    switch (state) {
        case WAIT4_EVT_FRAME: {
            Q_ASSERT_ID(910, l_rx.var.evt.e != nullptr);
            QF::gc(l_rx.var.evt.e); // don't leak an allocated event
            break;
        }
        default: {
            break;
        }
    }
}

//****************************************************************************
static void rxReportAck_(enum QSpyRxRecords const recId) noexcept {
    QS_CRIT_STAT_
    QS_CRIT_E_();
    QS::beginRec_(static_cast<std::uint_fast8_t>(QS_RX_STATUS));
        QS_U8_PRE_(recId); // record ID
    QS::endRec_();
    QS_CRIT_X_();

    QS_REC_DONE(); // user callback (if defined)
}

//****************************************************************************
static void rxReportError_(std::uint8_t const code) noexcept {
    QS_CRIT_STAT_
    QS_CRIT_E_();
    QS::beginRec_(static_cast<std::uint_fast8_t>(QS_RX_STATUS));
        QS_U8_PRE_(0x80U | code); // error code
    QS::endRec_();
    QS_CRIT_X_();

    QS_REC_DONE(); // user callback (if defined)
}

//****************************************************************************
static void rxReportDone_(enum QSpyRxRecords const recId) noexcept {
    QS_CRIT_STAT_
    QS_CRIT_E_();
    QS::beginRec_(static_cast<std::uint_fast8_t>(QS_TARGET_DONE));
        QS_TIME_PRE_();    // timestamp
        QS_U8_PRE_(recId); // record ID
    QS::endRec_();
    QS_CRIT_X_();

    QS_REC_DONE(); // user callback (if defined)
}

//****************************************************************************
static void rxPoke_(void) noexcept {
    std::uint8_t * const ptr =
        (static_cast<std::uint8_t *>(QS::rxPriv_.currObj[QS::AP_OBJ])
         + l_rx.var.poke.offs);
    switch (l_rx.var.poke.size) {
        case 1:
            *ptr = static_cast<std::uint8_t>(l_rx.var.poke.data);
            break;
        case 2:
            *reinterpret_cast<std::uint16_t *>(ptr)
                = static_cast<std::uint16_t>(l_rx.var.poke.data);
            break;
        case 4:
            *reinterpret_cast<std::uint32_t *>(ptr) = l_rx.var.poke.data;
            break;
        default:
            Q_ERROR_ID(900);
            break;
    }

    l_rx.var.poke.data = 0U;
    l_rx.var.poke.idx  = 0U;
    l_rx.var.poke.offs += static_cast<std::uint16_t>(l_rx.var.poke.size);
}

//============================================================================
#ifdef Q_UTEST

//****************************************************************************
/// @description
/// This function obtains the Test-Probe for a given API.
///
/// @param[in]  api  pointer to the API function that requests its Test-Probe
///
/// @returns Test-Probe data that has been received for the given API
/// from the Host (running qutest). For any ginve API, the function returns
/// the Test-Probe data in the same order as it was received from the Host.
/// If there is no Test-Probe for a ginve API, or no more Test-Probes for
/// a given API, the function returns zero.
///
std::uint32_t QS::getTestProbe_(void (* const api)(void)) noexcept {
    std::uint32_t data = 0U;
    for (std::uint8_t i = 0U; i < l_testData.tpNum; ++i) {
        if (l_testData.tpBuf[i].addr == reinterpret_cast<QSFun>(api)) {
            data = l_testData.tpBuf[i].data;

            QS_CRIT_STAT_
            QS_CRIT_E_();
            QS::beginRec_(static_cast<std::uint_fast8_t>(QS_TEST_PROBE_GET));
                QS_TIME_PRE_();    // timestamp
                QS_FUN_PRE_(api);  // the calling API
                QS_U32_PRE_(data); // the Test-Probe data
            QS::endRec_();
            QS_CRIT_X_();

            QS_REC_DONE(); // user callback (if defined)

            --l_testData.tpNum; // one less Test-Probe
            // move all remaining entries in the buffer up by one
            for (std::uint8_t j = i; j < l_testData.tpNum; ++j) {
                l_testData.tpBuf[j] = l_testData.tpBuf[j + 1U];
            }
            break; // we are done (Test-Probe retreived)
        }
    }
    return data;
}

//****************************************************************************
QSTimeCtr QS::onGetTime(void) {
    return (++l_testData.testTime);
}

#endif // Q_UTEST

} // namespace QP

