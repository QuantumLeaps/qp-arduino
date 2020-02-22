/// @file
/// @brief QS/C++ platform-independent public interface.
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.7.0
/// Last updated on  2019-12-29
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QS_HPP
#define QS_HPP

#ifndef Q_SPY
    #error "Q_SPY must be defined to include qs.hpp"
#endif

//****************************************************************************
namespace QP {

//! Quantum Spy record types.
/// @description
/// This enumeration specifies the record types used in the QP components.
/// You can specify your own record types starting from QP::QS_USER offset.
/// Currently, the maximum of all records cannot exceed 125.
///
/// @note
/// The QS records labeled as "not maskable" are always enabled and cannot
/// be turend off with the QS_FILTER_OFF() macro. Other QS trace records
/// can be disabled by means of the "global filters"
///
/// @sa QS_FILTER_ON() and QS_FILTER_OFF() macros

enum QSpyRecords {
    // [0] QS session (not maskable)
    QS_EMPTY,             //!< QS record for cleanly starting a session

    // [1] SM records
    QS_QEP_STATE_ENTRY,   //!< a state was entered
    QS_QEP_STATE_EXIT,    //!< a state was exited
    QS_QEP_STATE_INIT,    //!< an initial transition was taken in a state
    QS_QEP_INIT_TRAN,     //!< the top-most initial transition was taken
    QS_QEP_INTERN_TRAN,   //!< an internal transition was taken
    QS_QEP_TRAN,          //!< a regular transition was taken
    QS_QEP_IGNORED,       //!< an event was ignored (silently discarded)
    QS_QEP_DISPATCH,      //!< an event was dispatched (begin of RTC step)
    QS_QEP_UNHANDLED,     //!< an event was unhandled due to a guard

    // [10] Active Object (AO) records
    QS_QF_ACTIVE_DEFER,   //!< AO deferred an event
    QS_QF_ACTIVE_RECALL,  //!< AO recalled an event
    QS_QF_ACTIVE_SUBSCRIBE, //!< an AO subscribed to an event
    QS_QF_ACTIVE_UNSUBSCRIBE,//!< an AO unsubscribed to an event
    QS_QF_ACTIVE_POST_FIFO, //!< an event was posted (FIFO) directly to AO
    QS_QF_ACTIVE_POST_LIFO, //!< an event was posted (LIFO) directly to AO
    QS_QF_ACTIVE_GET,     //!< AO got an event and its queue is not empty
    QS_QF_ACTIVE_GET_LAST,//!< AO got an event and its queue is empty
    QS_QF_ACTIVE_RECALL_ATTEMPT, //!< AO attempted to recall an event

    // [19] Event Queue (EQ) records
    QS_QF_EQUEUE_POST_FIFO, //!< an event was posted (FIFO) to a raw queue
    QS_QF_EQUEUE_POST_LIFO, //!< an event was posted (LIFO) to a raw queue
    QS_QF_EQUEUE_GET,     //!< get an event and queue still not empty
    QS_QF_EQUEUE_GET_LAST,//!< get the last event from the queue

    QS_QF_RESERVED2,

    // [24] Memory Pool (MP) records
    QS_QF_MPOOL_GET,      //!< a memory block was removed from memory pool
    QS_QF_MPOOL_PUT,      //!< a memory block was returned to memory pool

    // [26] QF records
    QS_QF_PUBLISH,        //!< an event was published
    QS_QF_NEW_REF,        //!< new event reference was created
    QS_QF_NEW,            //!< new event was created
    QS_QF_GC_ATTEMPT,     //!< garbage collection attempt
    QS_QF_GC,             //!< garbage collection
    QS_QF_TICK,           //!< QP::QF::tickX() was called

    // [32] Time Event (TE) records
    QS_QF_TIMEEVT_ARM,    //!< a time event was armed
    QS_QF_TIMEEVT_AUTO_DISARM, //!< a time event expired and was disarmed
    QS_QF_TIMEEVT_DISARM_ATTEMPT,//!< attempt to disarm a disarmed QTimeEvt
    QS_QF_TIMEEVT_DISARM, //!< true disarming of an armed time event
    QS_QF_TIMEEVT_REARM,  //!< rearming of a time event
    QS_QF_TIMEEVT_POST,   //!< a time event posted itself directly to an AO

    // [38] Additional QF records
    QS_QF_DELETE_REF,     //!< an event reference is about to be deleted
    QS_QF_CRIT_ENTRY,     //!< critical section was entered
    QS_QF_CRIT_EXIT,      //!< critical section was exited
    QS_QF_ISR_ENTRY,      //!< an ISR was entered
    QS_QF_ISR_EXIT,       //!< an ISR was exited
    QS_QF_INT_DISABLE,    //!< interrupts were disabled
    QS_QF_INT_ENABLE,     //!< interrupts were enabled

    // [45] Additional Active Object (AO) records
    QS_QF_ACTIVE_POST_ATTEMPT, //!< attempt to post an evt to AO failed

    // [46] Additional Event Queue (EQ) records
    QS_QF_EQUEUE_POST_ATTEMPT, //!< attempt to post an evt to QEQueue failed

    // [47] Additional Memory Pool (MP) records
    QS_QF_MPOOL_GET_ATTEMPT,   //!< attempt to get a memory block failed

    // [48] Scheduler (SC) records
    QS_MUTEX_LOCK,        //!< a mutex was locked
    QS_MUTEX_UNLOCK,      //!< a mutex was unlocked
    QS_SCHED_LOCK,        //!< scheduler was locked
    QS_SCHED_UNLOCK,      //!< scheduler was unlocked
    QS_SCHED_NEXT,        //!< scheduler found next task to execute
    QS_SCHED_IDLE,        //!< scheduler became idle
    QS_SCHED_RESUME,      //!< scheduler resumed previous task (not idle)

    // [55] Additional QEP records
    QS_QEP_TRAN_HIST,     //!< a tran to history was taken
    QS_QEP_TRAN_EP,       //!< a tran to entry point into a submachine
    QS_QEP_TRAN_XP,       //!< a tran to exit  point out of a submachine

    // [58] Miscellaneous QS records (not maskable)
    QS_TEST_PAUSED,       //!< test has been paused
    QS_TEST_PROBE_GET,    //!< reports that Test-Probe has been used
    QS_SIG_DICT,          //!< signal dictionary entry
    QS_OBJ_DICT,          //!< object dictionary entry
    QS_FUN_DICT,          //!< function dictionary entry
    QS_USR_DICT,          //!< user QS record dictionary entry
    QS_TARGET_INFO,       //!< reports the Target information
    QS_TARGET_DONE,       //!< reports completion of a user callback
    QS_RX_STATUS,         //!< reports QS data receive status
    QS_QUERY_DATA,        //!< reports the data from "current object" query
    QS_PEEK_DATA,         //!< reports the data from the PEEK query
    QS_ASSERT_FAIL,       //!< assertion failed in the code

    // [70] Reserved QS records
    QS_RESERVED_70,
    QS_RESERVED_71,
    QS_RESERVED_72,
    QS_RESERVED_73,
    QS_RESERVED_74,
    QS_RESERVED_75,
    QS_RESERVED_76,
    QS_RESERVED_77,
    QS_RESERVED_78,
    QS_RESERVED_79,
    QS_RESERVED_80,
    QS_RESERVED_81,
    QS_RESERVED_82,
    QS_RESERVED_83,
    QS_RESERVED_84,
    QS_RESERVED_85,
    QS_RESERVED_86,
    QS_RESERVED_87,
    QS_RESERVED_88,
    QS_RESERVED_89,
    QS_RESERVED_90,
    QS_RESERVED_91,
    QS_RESERVED_92,
    QS_RESERVED_93,
    QS_RESERVED_94,
    QS_RESERVED_95,
    QS_RESERVED_96,
    QS_RESERVED_97,
    QS_RESERVED_98,
    QS_RESERVED_99,

    // [100] Application-specific (User) QS records
    QS_USER               //!< the first record available to QS users
};

//! QS record groups for QS_FILTER_ON() and QS_FILTER_OFF()
enum QSpyRecordGroups {
    QS_ALL_RECORDS = 0xF0,//!< all maskable QS records
    QS_SM_RECORDS,        //!< State Machine QS records
    QS_AO_RECORDS,        //!< Active Object QS records
    QS_EQ_RECORDS,        //!< Event Queues QS records
    QS_MP_RECORDS,        //!< Memory Pools QS records
    QS_TE_RECORDS,        //!< Time Events QS records
    QS_QF_RECORDS,        //!< QF QS records
    QS_SC_RECORDS,        //!< Scheduler QS records
    QS_U0_RECORDS,        //!< User Group 100-104 records
    QS_U1_RECORDS,        //!< User Group 105-109 records
    QS_U2_RECORDS,        //!< User Group 110-114 records
    QS_U3_RECORDS,        //!< User Group 115-119 records
    QS_U4_RECORDS,        //!< User Group 120-124 records
    QS_UA_RECORDS         //!< All User records
};

//! QS user record group offsets
enum QSpyUserRecords {
    QS_USER0 = static_cast<int_t>(QS_USER),      //!< offset for User Group 0
    QS_USER1 = static_cast<int_t>(QS_USER0) + 5, //!< offset for User Group 1
    QS_USER2 = static_cast<int_t>(QS_USER1) + 5, //!< offset for User Group 2
    QS_USER3 = static_cast<int_t>(QS_USER2) + 5, //!< offset for User Group 3
    QS_USER4 = static_cast<int_t>(QS_USER3) + 5  //!< offset for User Group 4
};

} // namespace QP ************************************************************

#ifndef QS_TIME_SIZE

    //! The size (in bytes) of the QS time stamp. Valid values: 1, 2,
    //! or 4; default 4.
    /// @description
    /// This macro can be defined in the QS port file (qs_port.hpp) to
    /// configure the QP::QSTimeCtr type. Here the macro is not defined so
    /// the default of 4 byte is chosen.

    #define QS_TIME_SIZE 4
#endif

#if (QS_TIME_SIZE == 1)
    #define QS_TIME_PRE_() (QP::QS::u8_raw_(QP::QS::onGetTime()))
#elif (QS_TIME_SIZE == 2)
    #define QS_TIME_PRE_() (QP::QS::u16_raw_(QP::QS::onGetTime()))
#elif (QS_TIME_SIZE == 4)
    //! Internal macro to output time stamp to a QS record
    #define QS_TIME_PRE_() (QP::QS::u32_raw_(QP::QS::onGetTime()))
#else
    #error "QS_TIME_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

//! access element at index @p i_ from the base pointer @p base_
///
/// @note This macro encapsulates MISRA-C++ 2008 Rule 5-0-15 (pointer
/// arithmetic other than array indexing).
#define QS_PTR_AT_(base_, i_) ((base_)[(i_)])


//****************************************************************************
namespace QP {

#if (QS_TIME_SIZE == 1)
    typedef uint8_t QSTimeCtr;
#elif (QS_TIME_SIZE == 2)
    typedef uint16_t QSTimeCtr;
#elif (QS_TIME_SIZE == 4)
    //! The type of the QS time stamp. This type determines the dynamic
    // range of QS time stamps
    //
    typedef uint32_t QSTimeCtr;
#endif

//! QS ring buffer counter and offset type
typedef uint_fast16_t QSCtr;

//! Constant representing End-Of-Data condition returned from the
//! QP::QS::getByte() function.
uint16_t const QS_EOD  = static_cast<uint16_t>(0xFFFF);

//! QS logging facilities
/// @description
/// This class groups together QS services. It has only static members and
/// should not be instantiated.
class QS {
public:
    //! Initialize the QS data buffer.
    static void initBuf(uint8_t * const sto, uint_fast16_t const stoSize);

    //! Turn the global Filter on for a given record type @p rec.
    static void filterOn_(uint_fast8_t const rec);

    //! Turn the global Filter off for a given record type @p rec.
    static void filterOff_(uint_fast8_t const rec);

    //! Mark the begin of a QS record @p rec
    static void beginRec_(uint_fast8_t const rec);

    //! Mark the end of a QS record @p rec
    static void endRec_(void);

    // raw (unformatted) output of data elements .............................

    //! output uint8_t data element without format information
    static void u8_raw_(uint8_t const d);

    //! output two uint8_t data elements without format information
    static void u8u8_raw_(uint8_t const d1, uint8_t const d2);

    //! Output uint16_t data element without format information
    static void u16_raw_(uint16_t d);

    //! Output uint32_t data element without format information
    static void u32_raw_(uint32_t d);

    //! Output zero-terminated ASCII string element without format information
    static void str_raw_(char_t const *s);


    // formatted data elements output ........................................

    //! Output uint8_t data element with format information
    static void u8_fmt_(uint8_t const format, uint8_t const d);

    //! output uint16_t data element with format information
    static void u16_fmt_(uint8_t format, uint16_t d);

    //! Output uint32_t data element with format information
    static void u32_fmt_(uint8_t format, uint32_t d);

    //! Output 32-bit floating point data element with format information
    static void f32_fmt_(uint8_t format, float32_t const d);

    //! Output 64-bit floating point data element with format information
    static void f64_fmt_(uint8_t format, float64_t const d);

    //! Output zero-terminated ASCII string element with format information
    static void str_fmt_(char_t const *s);

    //! Output memory block of up to 255-bytes with format information
    static void mem_fmt_(uint8_t const *blk, uint8_t size);

#if (QS_OBJ_PTR_SIZE == 8) || (QS_FUN_PTR_SIZE == 8)
    //! Output uint64_t data element without format information
    static void u64_raw_(uint64_t d);

    //! Output uint64_t data element with format information
    static void u64_fmt_(uint8_t format, uint64_t d);
#endif  // (QS_OBJ_PTR_SIZE == 8) || (QS_FUN_PTR_SIZE == 8)

    //! Output signal dictionary record
    static void sig_dict_pre_(enum_t const sig, void const * const obj,
                          char_t const *name);

    //! Output object dictionary record
    static void obj_dict_pre_(void const * const obj,
                          char_t const *name);

    //! Output function dictionary record
    static void fun_dict_pre_(void (* const fun)(void),
                          char_t const *name);

    //! Output user dictionary record
    static void usr_dict_pre_(enum_t const rec,
                          char_t const * const name);

    //! Initialize the QS RX data buffer
    static void rxInitBuf(uint8_t sto[], uint16_t const stoSize);

    //! Parse all bytes present in the QS RX data buffer
    static void rxParse(void);

    //! Obtain the number of free bytes in the QS RX data buffer
    static uint16_t rxGetNfree(void);

    //! put one byte into the QS RX lock-free buffer
    static void rxPut(uint8_t const b);

    // QS buffer access ......................................................
    //! Byte-oriented interface to the QS data buffer.
    static uint16_t getByte(void);

    //! Block-oriented interface to the QS data buffer.
    static uint8_t const *getBlock(uint16_t * const pNbytes);

    // platform-dependent callback functions to be implemented by clients ....

    //! Callback to startup the QS facility
    static bool onStartup(void const *arg);

    //! Callback to cleanup the QS facility
    static void onCleanup(void);

    //! Callback to flush the QS trace data to the host
    static void onFlush(void);

    //! Callback to obtain a timestamp for a QS record.
    static QSTimeCtr onGetTime(void);

    //! callback function to reset the Target (to be implemented in the BSP)
    static void onReset(void);

    //! Callback function to execute user commands (to be implemented in BSP)
    static void onCommand(uint8_t cmdId,
                          uint32_t param1, uint32_t param2, uint32_t param3);

    //! internal function to handle incoming (QS-RX) packet
    static void rxHandleGoodFrame_(uint8_t state);

#ifdef Q_UTEST
    //! callback to setup a unit test inside the Target
    static void onTestSetup(void);

    //! callback to teardown after a unit test inside the Target
    static void onTestTeardown(void);

    //! callback to "massage" the test event before dispatching/posting it
    static void onTestEvt(QEvt *e);

    // callback to examine an event that is about to be posted
    static void onTestPost(void const *sender, QActive *recipient,
                           QEvt const *e, bool status);

    //! callback to run the test loop
    static void onTestLoop(void);

    //! internal function to process posted events during test
    static void processTestEvts_(void);

    //! internal function to process armed time events during test
    static void tickX_(uint_fast8_t const tickRate, void const * const sender);

    //! internal function to get the Test-Probe for a given API
    static uint32_t getTestProbe_(void (* const api)(void));

#endif // Q_UTEST

    //! Enumerates data formats recognized by QS
    /// @description
    /// QS uses this enumeration is used only internally for the formatted
    /// user data elements.
    enum QSType {
        I8_T,         //!< signed 8-bit integer format
        U8_T,         //!< unsigned 8-bit integer format
        I16_T,        //!< signed 16-bit integer format
        U16_T,        //!< unsigned 16-bit integer format
        I32_T,        //!< signed 32-bit integer format
        U32_T,        //!< unsigned 32-bit integer format
        F32_T,        //!< 32-bit floating point format
        F64_T,        //!< 64-bit floating point format
        STR_T,        //!< zero-terminated ASCII string format
        MEM_T,        //!< up to 255-bytes memory block format
        SIG_T,        //!< event signal format
        OBJ_T,        //!< object pointer format
        FUN_T,        //!< function pointer format
        I64_T,        //!< signed 64-bit integer format
        U64_T,        //!< unsigned 64-bit integer format
        U32_HEX_T     //!< unsigned 32-bit integer in hex format
    };

    //! Kinds of objects used in QS
    enum QSpyObjKind {
        SM_OBJ,       //!< state machine object for QEP
        AO_OBJ,       //!< active object
        MP_OBJ,       //!< event pool object
        EQ_OBJ,       //!< raw queue object
        TE_OBJ,       //!< time event object
        AP_OBJ,       //!< generic Application-specific object
        MAX_OBJ
    };

    enum OSpyObjCombnation {
        SM_AO_OBJ = MAX_OBJ //!< combination of SM and AO
    };

    //! template for forcing cast of member functions for function
    //! dictionaries and test probes.
    template<typename T_OUT, typename T_IN>
    static T_OUT force_cast(T_IN in) {
        union TCast {
            T_IN  in;
            T_OUT out;
        } u = { in };
        return u.out;
    }

    // private QS attributes .................................................
    uint8_t glbFilter[16];          //!< global on/off QS filter
    void const *locFilter[MAX_OBJ]; //!< local QS filters
    uint8_t *buf;     //!< pointer to the start of the ring buffer
    QSCtr    end;     //!< offset of the end of the ring buffer
    QSCtr    head;    //!< offset to where next byte will be inserted
    QSCtr    tail;    //!< offset of where next record will be extracted
    QSCtr    used;    //!< number of bytes currently in the ring buffer
    uint8_t  seq;     //!< the record sequence number
    uint8_t  chksum;  //!< the checksum of the current record
    uint8_t  full;    //!< the ring buffer is temporarily full

    uint_fast8_t critNest; //!< critical section nesting level

    static QS priv_;

    static struct QSrxPriv {
        void *currObj[MAX_OBJ]; //!< current objects
        uint8_t *buf; //!< pointer to the start of the ring buffer
        QSCtr end;    //!< offset of the end of the ring buffer
        QSCtr head;   //!< offset to where next byte will be inserted
        QSCtr tail;   //!< offset of where next byte will be extracted
#ifdef Q_UTEST
        QP::QPSet readySet; //!< QUTEST ready-set of active objects
        bool  inTestLoop;   //!< QUTest event loop is running
#endif
    } rxPriv_;
};

//! Quantum Spy Receive (RX) record types
/// @description
/// This enumeration specifies the record types for the QS receive channel
enum QSpyRxRecords {
    QS_RX_INFO,       //!< query Target info (ver, config, tstamp)
    QS_RX_COMMAND,    //!< execute a user-defined command in the Target
    QS_RX_RESET,      //!< reset the Target
    QS_RX_TICK,       //!< call QF_tick()
    QS_RX_PEEK,       //!< peek Target memory
    QS_RX_POKE,       //!< poke Target memory
    QS_RX_FILL,       //!< fill Target memory
    QS_RX_TEST_SETUP,    //!< test setup
    QS_RX_TEST_TEARDOWN, //!< test teardown
    QS_RX_TEST_PROBE, //!< set a Test-Probe in the Target
    QS_RX_GLB_FILTER, //!< set global filters in the Target
    QS_RX_LOC_FILTER, //!< set local  filters in the Target
    QS_RX_AO_FILTER,  //!< set local AO filter in the Target
    QS_RX_CURR_OBJ,   //!< set the "current-object" in the Target
    QS_RX_TEST_CONTINUE, //!< continue a test after QS_RX_TEST_WAIT()
    QS_RX_QUERY_CURR,    //!< query the "current object" in the Target
    QS_RX_EVENT       //!< inject an event to the Target (post/publish)
};

//! put one byte into the QS RX lock-free buffer
inline void QS::rxPut(uint8_t const b) {
    if (rxPriv_.head != static_cast<QSCtr>(0)) {
        if ((rxPriv_.head - rxPriv_.tail) != static_cast<QSCtr>(1)) {
            QS_PTR_AT_(rxPriv_.buf, rxPriv_.head) = b;
            --rxPriv_.head;
        }
    }
    else {
        if (rxPriv_.tail != rxPriv_.end) {
            QS_PTR_AT_(rxPriv_.buf, 0) = b;
            rxPriv_.head = rxPriv_.end;
        }
    }
}


//****************************************************************************
#ifdef Q_UTEST

//! Dummy Active Object class
/// @description
/// QActiveDummy is a test double for the role of collaborating active
/// objects in QUTest unit testing.
///
class QActiveDummy : public QActive {
public:
    QActiveDummy(void); // ctor

    virtual void start(uint_fast8_t const prio,
                       QEvt const * * const qSto, uint_fast16_t const qLen,
                       void * const stkSto, uint_fast16_t const stkSize,
                       void const * const par);

    //! Overloaded start function (no initialization event)
    virtual void start(uint_fast8_t const prio,
                       QEvt const * * const qSto, uint_fast16_t const qLen,
                       void * const stkSto, uint_fast16_t const stkSize)
    {
        this->start(prio, qSto, qLen, stkSto, stkSize,
                    static_cast<void *>(0));
    }

    virtual void init(void const * const e);
    virtual void init(void) { this->init(static_cast<void *>(0)); }
    virtual void dispatch(QEvt const * const e);
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin,
                       void const * const sender);
    virtual void postLIFO(QEvt const * const e);
};

enum QUTestUserRecords {
    QUTEST_ON_POST = 124
};

#endif // Q_UTEST

} // namespace QP

//****************************************************************************
// Macros for adding QS instrumentation to the client code

//! Initialize the QS facility.
/// @description
/// This macro provides an indirection layer to invoke the QS initialization
/// routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
/// @sa QP::QS::onStartup(), example of setting up a QS filter in
/// QS_FILTER_ON()
#define QS_INIT(arg_)           (QP::QS::onStartup(arg_))

//! Cleanup the QS facility.
/// @description
/// This macro provides an indirection layer to invoke the QS cleanup
/// routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
/// @sa QP::QS::onCleanup()
#define QS_EXIT()               (QP::QS::onCleanup())

//! Global Filter ON for a given record type @p rec.
/// @description
/// This macro provides an indirection layer to call QP::QS::filterOn()
/// if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
///
/// @usage
/// The following example shows how to use QS filters:
/// @include qs_filter.cpp
#define QS_FILTER_ON(rec_) \
    (QP::QS::filterOn_(static_cast<uint_fast8_t>(rec_)))

//! Global filter OFF for a given record type @p rec.
/// @description
/// This macro provides an indirection layer to call QP::QS::filterOff()
/// if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_OFF(rec_) \
    (QP::QS::filterOff_(static_cast<uint_fast8_t>(rec_)))

//! Local Filter for a given state machine object @p obj_.
/// @description
/// This macro sets up the state machine object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the state machine object that you want to monitor.@n
/// @n
/// The state machine object filter allows you to filter QS records pertaining
/// only to a given state machine object. With this filter disabled, QS will
/// output records from all state machines in your application. The object
/// filter is disabled by setting the state machine pointer to NULL.@n
/// @n
/// The state machine filter affects the following QS records:
/// QP::QS_QEP_STATE_ENTRY, QP::QS_QEP_STATE_EXIT, QP::QS_QEP_STATE_INIT,
/// QP::QS_QEP_INTERN_TRAN, QP::QS_QEP_TRAN, and QP::QS_QEP_IGNORED.
///
/// @note
/// Because active objects are state machines at the same time, the state
/// machine filter (QS_FILTER_SM_OBJ) pertains to active objects as well.
/// However, the state machine filter is more general, because it can be
/// used only for state machines that are not active objects, such as
/// "Orthogonal Components".
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_SM_OBJ(obj_) \
    (QP::QS::priv_.locFilter[QP::QS::SM_OBJ] = (obj_))

//! Local Filter for a given active object @p obj_.
/// @description
/// This macro sets up the active object local filter if #Q_SPY is defined,
/// or does nothing if #Q_SPY is not defined. The argument @p obj_ is the
/// pointer to the active object that you want to monitor.@n
/// @n
/// The active object filter allows you to filter QS records pertaining
/// only to a given active object. With this filter disabled, QS will
/// output records from all active objects in your application. The object
/// filter is disabled by setting the active object pointer @p obj_ to NULL.@n
/// @n
/// The active object filter affects the following QS records:
/// ::QS_QF_ACTIVE_DEFER, ::QS_QF_ACTIVE_RECALL, ::QS_QF_ACTIVE_SUBSCRIBE,
/// ::QS_QF_ACTIVE_UNSUBSCRIBE, ::QS_QF_ACTIVE_POST, ::QS_QF_ACTIVE_POST_LIFO,
/// ::QS_QF_ACTIVE_GET, ::QS_QF_ACTIVE_GET_LAST, and
/// ::QS_QF_ACTIVE_RECALL_ATTEMPT.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_AO_OBJ(obj_) \
    (QP::QS::priv_.locFilter[QP::QS::AO_OBJ] = (obj_))

//! Local Filter for a given memory pool object @p obj_.
/// @description
/// This macro sets up the memory pool object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the memory buffer used during the initialization of the
/// event pool with QP::QF::poolInit().@n
/// @n
/// The memory pool filter allows you to filter QS records pertaining
/// only to a given memory pool. With this filter disabled, QS will
/// output records from all memory pools in your application. The object
/// filter is disabled by setting the memory pool pointer @p obj_ to NULL.@n
/// @n
/// The memory pool filter affects the following QS records:
/// QP::QS_QF_MPOOL_GET, and QP::QS_QF_MPOOL_PUT.
///
/// @sa Example of using QS filters in QS_FILTER_ON() documentation
#define QS_FILTER_MP_OBJ(obj_) \
    (QP::QS::priv_.locFilter[QP::QS::MP_OBJ] = (obj_))

//! Filter for a given event queue object @p obj_.
/// @description
/// This macro sets up the event queue object filter if #Q_SPY is defined,
/// or does nothing if #Q_SPY is not defined. The argument @p obj_ is the
/// pointer to the "raw" thread-safe queue object you want to monitor.@n
/// @n
/// The event queue filter allows you to filter QS records pertaining
/// only to a given event queue. With this filter disabled, QS will
/// output records from all event queues in your application. The object
/// filter is disabled by setting the event queue pointer @p obj_ to NULL.@n
/// @n
/// The event queue filter affects the following QS records:
/// QP::QS_QF_EQUEUE_POST_FIFO, QP::QS_QF_EQUEUE_POST_LIFO,
/// QP::QS_QF_EQUEUE_GET, and QP::QS_QF_EQUEUE_GET_LAST.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_EQ_OBJ(obj_) \
    (QP::QS::priv_.locFilter[QP::QS::EQ_OBJ] = (obj_))

//! Local Filter for a given time event object @p obj_.
/// @description
/// This macro sets up the time event object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the time event object you want to monitor.@n
/// @n
/// The time event filter allows you to filter QS records pertaining
/// only to a given time event. With this filter disabled, QS will
/// output records from all time events in your application. The object
/// filter is disabled by setting the time event pointer @p obj_ to NULL.@n
/// @n
/// The time event filter affects the following QS records:
/// QP::QS_QF_TIMEEVT_ARM, QP::QS_QF_TIMEEVT_AUTO_DISARM,
/// QP::QS_QF_TIMEEVT_DISARM_ATTEMPT, QP::QS_QF_TIMEEVT_DISARM,
/// QP::QS_QF_TIMEEVT_REARM, and QP::QS_QF_TIMEEVT_POST.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_TE_OBJ(obj_) \
    (QP::QS::priv_.locFilter[QP::QS::TE_OBJ] = (obj_))

//! Local Filter for a generic application object @p obj_.
/// @description
/// This macro sets up the local application object filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the application object you want to monitor.@n
/// @n
/// The application object filter allows you to filter QS records pertaining
/// only to a given application object. With this filter disabled, QS will
/// output records from all application-records enabled by the global filter.
/// The local filter is disabled by setting the time event pointer @p obj_
/// to NULL.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_AP_OBJ(obj_) \
    (QP::QS::priv_.locFilter[QP::QS::AP_OBJ] = (obj_))


//****************************************************************************
// Macros to generate application-specific (user) QS records

//! helper macro for checking the global QS filter
#define QS_GLB_FILTER_(rec_) \
    ((static_cast<uint_fast8_t>(QP::QS::priv_.glbFilter[ \
            static_cast<uint8_t>(rec_) >> 3]) \
      & static_cast<uint_fast8_t>(static_cast<uint8_t>(1U << \
               (static_cast<uint8_t>(rec_) & static_cast<uint8_t>(7))))) \
             != static_cast<uint_fast8_t>(0))

//! Begin a QS user record without entering critical section.
#define QS_BEGIN_NOCRIT(rec_, obj_) \
    if (QS_GLB_FILTER_(rec_) && \
        ((QP::QS::priv_.locFilter[QP::QS::AP_OBJ] == static_cast<void *>(0)) \
            || (QP::QS::priv_.locFilter[QP::QS::AP_OBJ] == (obj_)))) \
    { \
        QP::QS::beginRec_(static_cast<uint_fast8_t>(rec_)); \
        QS_TIME_PRE_();

//! End a QS user record without exiting critical section.
#define QS_END_NOCRIT() \
    QS_END_NOCRIT_PRE_()

#ifndef QS_REC_DONE
    //! macro to hook up user code when a QS record is produced
    #define QS_REC_DONE() ((void)0)
#endif // QS_REC_DONE

// QS-specific critical section ..............................................
#ifdef QS_CRIT_ENTRY // separate QS critical section defined?

#ifndef QS_CRIT_STAT_TYPE
    #define QS_CRIT_STAT_
    #define QS_CRIT_ENTRY_()    QS_CRIT_ENTRY(dummy)
    #define QS_CRIT_EXIT_()     QS_CRIT_EXIT(dummy); QS_REC_DONE()
#else
    #define QS_CRIT_STAT_       QS_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_ENTRY_()    QS_CRIT_ENTRY(critStat_)
    #define QS_CRIT_EXIT_()     QS_CRIT_EXIT(critStat_); QS_REC_DONE()
#endif // QS_CRIT_STAT_TYPE

#else // separate QS critical section not defined--use the QF definition
#ifndef QF_CRIT_STAT_TYPE
    //! This is an internal macro for defining the critical section
    //! status type.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// provides the definition of the critical section status variable.
    /// Otherwise this macro is empty.
    /// @sa #QF_CRIT_STAT_TYPE
    #define QS_CRIT_STAT_

    //! This is an internal macro for entering a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_ENTRY passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_ENTRY is invoked with a dummy parameter.
    /// @sa #QF_CRIT_ENTRY
    #define QS_CRIT_ENTRY_()    QF_CRIT_ENTRY(dummy)

    //! This is an internal macro for exiting a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_EXIT passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_EXIT is invoked with a dummy parameter.
    /// @sa #QF_CRIT_EXIT
    #define QS_CRIT_EXIT_()     QF_CRIT_EXIT(dummy); QS_REC_DONE()

#else  // simple unconditional interrupt disabling used
    #define QS_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_ENTRY_()    QF_CRIT_ENTRY(critStat_)
    #define QS_CRIT_EXIT_()     QF_CRIT_EXIT(critStat_); QS_REC_DONE()
#endif // simple unconditional interrupt disabling used

#endif // separate QS critical section not defined

//! Begin a user QS record with entering critical section.
/// @description
/// The following example shows how to build a user QS record using the
/// macros #QS_BEGIN, #QS_END, and the formatted output macros: #QS_U8 and
/// #QS_STR.
///
/// @note
/// Must always be used in pair with #QS_END
///
/// @include qs_user.cpp
#define QS_BEGIN(rec_, obj_) \
    if (QS_GLB_FILTER_(rec_) && \
        ((QP::QS::priv_.locFilter[QP::QS::AP_OBJ] == static_cast<void *>(0)) \
            || (QP::QS::priv_.locFilter[QP::QS::AP_OBJ] == (obj_)))) \
    { \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP::QS::beginRec_(static_cast<uint_fast8_t>(rec_)); \
        QS_TIME_PRE_();

//! End a QS record with exiting critical section.
/// @sa example for #QS_BEGIN
/// @note Must always be used in pair with #QS_BEGIN
#define QS_END() \
    QS_END_PRE_()


//****************************************************************************
// Macros for use inside other macros or internally in the QP code

//! Internal QS macro to begin a predefined QS record with critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_BEGIN
#define QS_BEGIN_PRE_(rec_, objFilter_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && (((objFilter_) == static_cast<void *>(0)) \
            || ((objFilter_) == (obj_)))) \
    { \
        QS_CRIT_ENTRY_(); \
        QP::QS::beginRec_(static_cast<uint_fast8_t>(rec_));

//! Internal QS macro to end a predefined QS record with critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_END
#define QS_END_PRE_() \
        QP::QS::endRec_(); \
        QS_CRIT_EXIT_(); \
    }

//! Internal QS macro to begin a predefined QS record without critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_BEGIN_NOCRIT_PRE_
#define QS_BEGIN_NOCRIT_PRE_(rec_, objFilter_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && (((objFilter_) == static_cast<void *>(0)) \
            || ((objFilter_) == (obj_)))) \
    { \
        QP::QS::beginRec_(static_cast<uint_fast8_t>(rec_));

//! Internal QS macro to end a predefiend QS record without critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_END_NOCRIT
#define QS_END_NOCRIT_PRE_() \
        QP::QS::endRec_(); \
    }

#if (Q_SIGNAL_SIZE == 1)
    //! Internal QS macro to output an unformatted event signal data element
    /// @note
    /// The size of the pointer depends on the macro #Q_SIGNAL_SIZE.
    #define QS_SIG_PRE_(sig_) (QP::QS::u8_raw_(static_cast<uint8_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 2)
    #define QS_SIG_PRE_(sig_) (QP::QS::u16_raw_(static_cast<uint16_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 4)
    #define QS_SIG_PRE_(sig_) (QP::QS::u32_raw_(static_cast<uint32_t>(sig_)))
#endif

//! Internal QS macro to output an unformatted uint8_t data element
#define QS_U8_PRE_(data_)     (QP::QS::u8_raw_(static_cast<uint8_t>(data_)))

//! Internal QS macro to output 2 unformatted uint8_t data elements
#define QS_2U8_PRE_(data1_, data2_) (QP::QS::u8u8_raw_((data1_), (data2_)))

//! Internal QS macro to output an unformatted uint16_t data element
#define QS_U16_PRE_(data_)    (QP::QS::u16_raw_(static_cast<uint16_t>(data_)))

//! Internal QS macro to output an unformatted uint32_t data element
#define QS_U32_PRE_(data_)    (QP::QS::u32_raw_(static_cast<uint32_t>(data_)))

//! Internal QS macro to output a zero-terminated ASCII string
/// data element
#define QS_STR_PRE_(msg_)     (QP::QS::str_raw_(msg_))

#if (QS_OBJ_PTR_SIZE == 1)
    #define QS_OBJ_PRE_(obj_) (QP::QS::u8_raw_(reinterpret_cast<uint8_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2)
    #define QS_OBJ_PRE_(obj_) (QP::QS::u16_raw_(reinterpret_cast<uint16_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4)
    #define QS_OBJ_PRE_(obj_) (QP::QS::u32_raw_(reinterpret_cast<uint32_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8)
    #define QS_OBJ_PRE_(obj_) (QP::QS::u64_raw_(reinterpret_cast<uint64_t>(obj_)))
#else

    //! Internal QS macro to output a pedefined unformatted object pointer
    //! data element
    /// @note
    /// The size of the pointer depends on the macro #QS_OBJ_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_OBJ_PRE_(obj_) \
        (QP::QS::u32_raw_(reinterpret_cast<uint32_t>(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1)
    #define QS_FUN_PRE_(fun_) (QP::QS::u8_raw_(reinterpret_cast<uint8_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 2)
    #define QS_FUN_PRE_(fun_) (QP::QS::u16_raw_(reinterpret_cast<uint16_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 4)
    #define QS_FUN_PRE_(fun_) (QP::QS::u32_raw_(reinterpret_cast<uint32_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 8)
    #define QS_FUN_PRE_(fun_) (QP::QS::u64_raw_(reinterpret_cast<uint64_t>(fun_)))
#else

    //! Internal QS macro to output an unformatted function pointer
    //! data element
    /// @note
    /// The size of the pointer depends on the macro #QS_FUN_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_FUN_PRE_(fun_) (QP::QS::u32_raw_(reinterpret_cast<uint32_t>(fun_)))
#endif


//****************************************************************************
// Macros for use in the client code

//! Output formatted int8_t to the QS record
#define QS_I8(width_, data_)                           \
    (QP::QS::u8_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))          \
        | static_cast<uint8_t>(QP::QS::I8_T)), (data_)))

//! Output formatted uint8_t to the QS record
#define QS_U8(width_, data_)                           \
    (QP::QS::u8_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))          \
        | static_cast<uint8_t>(QP::QS::U8_T)), (data_)))

//! Output formatted int16_t to the QS record
#define QS_I16(width_, data_)                           \
    (QP::QS::u16_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
        | static_cast<uint8_t>(QP::QS::I16_T)), (data_)))

//! Output formatted uint16_t to the QS record
#define QS_U16(width_, data_)                                \
    (QP::QS::u16_fmt_(static_cast<uint8_t>((((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::U16_T)), (data_)))

//! Output formatted int32_t to the QS record
#define QS_I32(width_, data_)                                      \
    (QP::QS::u32_fmt_(                                             \
        static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::I32_T)), (data_)))

//! Output formatted uint32_t to the QS record
#define QS_U32(width_, data_)                           \
    (QP::QS::u32_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
        | static_cast<uint8_t>(QP::QS::U32_T)), (data_)))

//! Output formatted 32-bit floating point number to the QS record
#define QS_F32(width_, data_)                           \
    (QP::QS::f32_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
        | static_cast<uint8_t>(QP::QS::F32_T)), (data_)))

//! Output formatted 64-bit floating point number to the QS record
#define QS_F64(width_, data_)                           \
    (QP::QS::f64_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
        | static_cast<uint8_t>(QP::QS::F64_T)), (data_)))

//! Output formatted int64_t to the QS record
#define QS_I64(width_, data_)                           \
    (QP::QS::u64_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
        | static_cast<uint8_t>(QP::QS::I64_T)), (data_)))

//! Output formatted uint64_t to the QS record
#define QS_U64(width_, data_)                           \
    (QP::QS::u64_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
            | static_cast<uint8_t>(QP::QS::U64_T)), (data_)))

//! Output formatted uint32_t to the QS record
#define QS_U32_HEX(width_, data_)                       \
    (QP::QS::u32_fmt_(static_cast<uint8_t>(             \
        (static_cast<uint8_t>((width_) << 4))           \
            | static_cast<uint8_t>(QP::QS::U32_HEX_T)), (data_)))

//! Output formatted zero-terminated ASCII string to the QS record
#define QS_STR(str_)        (QP::QS::str_fmt_(str_))

//! Output formatted memory block of up to 255 bytes to the QS record
#define QS_MEM(mem_, size_) (QP::QS::mem_fmt_((mem_), (size_)))


#if (QS_OBJ_PTR_SIZE == 1)
    #define QS_OBJ(obj_) \
        (QP::QS::u8_fmt_(QP::QS::OBJ_T, reinterpret_cast<uint8_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2)
    #define QS_OBJ(obj_) \
        (QP::QS::u16_fmt_(QP::QS::OBJ_T, reinterpret_cast<uint16_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4)
    #define QS_OBJ(obj_) \
        (QP::QS::u32_fmt_(QP::QS::OBJ_T, reinterpret_cast<uint32_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8)
    #define QS_OBJ(obj_) \
        (QP::QS::u64_fmt_(QP::QS::OBJ_T, reinterpret_cast<uint64_t>(obj_)))
#else
    //! Output formatted object pointer to the QS record
    #define QS_OBJ(obj_) \
        (QP::QS::u32_fmt_(QP::QS::OBJ_T, reinterpret_cast<uint32_t>(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1)
    #define QS_FUN(fun_) \
        (QP::QS::u8_fmt_(QP::QS::FUN_T, reinterpret_cast<uint8_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 2)
    #define QS_FUN(fun_) \
        (QP::QS::u16_fmt_(QP::QS::FUN_T, reinterpret_cast<uint16_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 4)
    #define QS_FUN(fun_) \
        (QP::QS::u32_fmt_(QP::QS::FUN_T, reinterpret_cast<uint32_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 8)
    #define QS_FUN(fun_) \
        (QP::QS::u64_fmt_(QP::QS::FUN_T, reinterpret_cast<uint64_t>(fun_)))
#else
    //! Output formatted function pointer to the QS record
    #define QS_FUN(fun_) \
        (QP::QS::u32_fmt_(QP::QS::FUN_T, reinterpret_cast<uint32_t>(fun_)))
#endif


#if (Q_SIGNAL_SIZE == 1)
    #define QS_SIG(sig_, obj_) \
        QP::QS::u8_fmt_(QP::QS::SIG_T, static_cast<uint8_t>(sig_)); \
        QS_OBJ_PRE_(obj_)
#elif (Q_SIGNAL_SIZE == 2)
    #define QS_SIG(sig_, obj_) \
        QP::QS::u16_fmt_(QP::QS::SIG_T, static_cast<uint16_t>(sig_)); \
        QS_OBJ_PRE_(obj_)
#elif (Q_SIGNAL_SIZE == 4)
    #define QS_SIG(sig_, obj_) \
        QP::QS::u32_fmt_(QP::QS::SIG_T, static_cast<uint32_t>(sig_)); \
        QS_OBJ_PRE_(obj_)
#else
    //! Output formatted event signal (of type ::QSignal) and
    //! the state machine object to the user QS record
    #define QS_SIG(sig_, obj_) \
        QP::QS::u16_fmt_(QP::QS::SIG_T, static_cast<uint16_t>(sig_)); \
        QS_OBJ_PRE_(obj_)
#endif


//////////////////////////////////////////////////////////////////////////////

//! Output signal dictionary record
///
/// A signal dictionary record associates the numerical value of the signal
/// and the binary address of the state machine that consumes that signal
/// with the human-readable name of the signal.
///
/// Providing a signal dictionary QS record can vastly improve readability of
/// the QS log, because instead of dealing with cryptic machine addresses the
/// QSpy host utility can display human-readable names.
///
/// A signal dictionary entry is associated with both the signal value @p sig_
/// and the state machine @p obj_, because signals are required to be unique
/// only within a given state machine and therefore the same numerical values
/// can represent different signals in different state machines.
///
/// For the "global" signals that have the same meaning in all state machines
/// (such as globally published signals), you can specify a signal dictionary
/// entry with the @p obj_ parameter set to NULL.
///
/// The following example shows the definition of signal dictionary entries
/// in the initial transition of the Table active object. Please note that
/// signals HUNGRY_SIG and DONE_SIG are associated with the Table state
/// machine only ("me" @p obj_ pointer). The EAT_SIG signal, on the other
/// hand, is global (0 @p obj_ pointer):
/// @include qs_sigDic.cpp
///
/// @note The QSpy log utility must capture the signal dictionary record
/// in order to use the human-readable information. You need to connect to
/// the target before the dictionary entries have been transmitted.
///
/// The following QSpy log example shows the signal dictionary records
/// generated from the Table initial transition and subsequent records that
/// show human-readable names of the signals:
/// @include qs_sigLog.txt
///
/// The following QSpy log example shows the same sequence of records, but
/// with dictionary records removed. The human-readable signal names are not
/// available.
/// @include qs_sigLog0.txt
#define QS_SIG_DICTIONARY(sig_, obj_) do {                \
    static char_t const sig_name_[] = #sig_;              \
    QP::QS::sig_dict_pre_((sig_), (obj_), &sig_name_[0]); \
} while (false)

//! Output object dictionary record
///
/// An object dictionary record associates the binary address of an object
/// in the target's memory with the human-readable name of the object.
///
/// Providing an object dictionary QS record can vastly improve readability of
/// the QS log, because instead of dealing with cryptic machine addresses the
/// QSpy host utility can display human-readable object names.
///
/// The following example shows the definition of object dictionary entry
/// for the Table active object:
/// @include qs_objDic.cpp
#define QS_OBJ_DICTIONARY(obj_) do {              \
    static char_t const obj_name_[] = #obj_;      \
    QP::QS::obj_dict_pre_((obj_), &obj_name_[0]); \
} while (false)

//! Output function dictionary record
///
/// A function dictionary record associates the binary address of a function
/// in the target's memory with the human-readable name of the function.
///
/// Providing a function dictionary QS record can vastly improve readability
/// of the QS log, because instead of dealing with cryptic machine addresses
/// the QSpy host utility can display human-readable function names.
///
/// The example from #QS_SIG_DICTIONARY shows the definition of a function
/// dictionary.
#define QS_FUN_DICTIONARY(fun_) do {                              \
    static char_t const fun_name_[] = #fun_;                      \
    QP::QS::fun_dict_pre_(                                        \
        QP::QS::force_cast<void (*)(void)>(fun_), &fun_name_[0]); \
} while (false)

//! Output user QS record dictionary record
///
/// A user QS record dictionary record associates the numerical value of a
/// user record with the human-readable identifier.
#define QS_USR_DICTIONARY(rec_) do {              \
    static char_t const usr_name_[] = #rec_;      \
    QP::QS::usr_dict_pre_((rec_), &usr_name_[0]); \
} while (false)

//! Output the assertion failure trace record
#define QS_ASSERTION(module_, loc_, delay_) do {             \
    QS_BEGIN_NOCRIT_PRE_(QP::QS_ASSERT_FAIL,                 \
        static_cast<void *>(0), static_cast<void *>(0))      \
        QS_TIME_PRE_();                                      \
        QS_U16_PRE_(static_cast<uint16_t>(loc_));            \
        QS_STR_PRE_(((module_)                               \
            != static_cast<char_t *>(0)) ? (module_) : "?"); \
    QS_END_NOCRIT_PRE_()                                     \
    QP::QS::onFlush();                                       \
    for (uint32_t volatile delay_ctr_ = (delay_);            \
         delay_ctr_ > static_cast<uint32_t>(0); --delay_ctr_)\
    {}                                                       \
    QP::QS::onCleanup();                                     \
} while (false)

//! Flush the QS trace data to the host
///
/// This macro invokes the QP::QS::flush() platform-dependent callback
/// function to flush the QS trace buffer to the host. The function
/// typically busy-waits until all the data in the buffer is sent to
/// the host. This is acceptable only in the initial transient.
#define QS_FLUSH()   (QP::QS::onFlush())

//! Output the critical section entry record
#define QF_QS_CRIT_ENTRY()                              \
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_CRIT_ENTRY,          \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_PRE_();                                 \
        QS_U8_PRE_((uint8_t)(++QS::priv_.critNest));    \
    QS_END_NOCRIT_PRE_()

//! Output the critical section exit record
#define QF_QS_CRIT_EXIT()                               \
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_CRIT_EXIT,           \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_PRE_();                                 \
        QS_U8_PRE_((uint8_t)(QS::priv_.critNest--));    \
    QS_END_NOCRIT_PRE_()

//! Output the interrupt entry record
#define QF_QS_ISR_ENTRY(isrnest_, prio_)                \
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_ISR_ENTRY,           \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_PRE_();                                 \
        QS_U8_PRE_(isrnest_);                           \
        QS_U8_PRE_(prio_);                              \
    QS_END_NOCRIT_PRE_()

//! Output the interrupt exit record
#define QF_QS_ISR_EXIT(isrnest_, prio_)                 \
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_ISR_EXIT,            \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_PRE_();                                 \
        QS_U8_PRE_(isrnest_);                           \
        QS_U8_PRE_(prio_);                              \
    QS_END_NOCRIT_PRE_()

//! Execute an action that is only necessary for QS output
#define QF_QS_ACTION(act_)      (act_)

//! macro to handle the QS output from the application
//! NOTE: if this macro is used, the application must define QS_output().
#define QS_OUTPUT()   (QS_output())

//! macro to handle the QS-RX input to the application
//! NOTE: if this macro is used, the application must define QS_rx_input().
#define QS_RX_INPUT() (QS_rx_input())


//****************************************************************************
// Macros for use in QUTest only

#ifdef Q_UTEST
    //! QS macro to define the Test-Probe for a given @p fun_
    #define QS_TEST_PROBE_DEF(fun_)                                       \
        uint32_t const qs_tp_ =                                           \
            QP::QS::getTestProbe_(QP::QS::force_cast<void (*)(void)>(fun_));

    //! QS macro to apply a Test-Probe
    #define QS_TEST_PROBE(code_)                        \
        if (qs_tp_ != static_cast<uint32_t>(0)) { code_ }

    //! QS macro to apply a Test-Probe
    #define QS_TEST_PROBE_ID(id_, code_)                  \
        if (qs_tp_ == static_cast<uint32_t>(id_)) { code_ }

    //! QS macro to pause test execution and enter the test event loop
    #define QS_TEST_PAUSE() do {                                         \
        QP::QS::beginRec_(static_cast<uint_fast8_t>(QP::QS_TEST_PAUSED)); \
        QP::QS::endRec_();                                                \
        QP::QS::onTestLoop();                                            \
    } while (false)

#else
    // dummy definitions when not building for QUTEST
    #define QS_TEST_PROBE_DEF(fun_)
    #define QS_TEST_PROBE(code_)
    #define QS_TEST_PROBE_ID(id_, code_)
    #define QS_TEST_PAUSE()  ((void)0)
#endif // Q_UTEST

#endif // QS_HPP

