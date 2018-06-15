/**
* @file
* @brief QP-nano public interface adapted for Arduino
* @cond
******************************************************************************
* Last updated for version 6.3.0
* Last updated on  2018-05-10
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2005-2018 Quantum Leaps, www.state-machine.com.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* https://www.state-machine.com
* mailto:info@state-machine.com
******************************************************************************
* @endcond
*/
#ifndef qpn_h
#define qpn_h

/**
* @description
* This header file must be included directly or indirectly
* in all application modules (*.c files) that use QP-nano.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*##########################################################################*/
/* #include "qpn_conf.h" */ /* QP-nano configuration file */

/* QP-nano configuration macros... */
#define Q_PARAM_SIZE            4
#define QF_MAX_TICK_RATE        1
#define QF_TIMEEVT_CTR_SIZE     2
#define QF_TIMEEVT_PERIODIC

/* fake inclusion of the QV-nano header file "qvn.h" */
#define qvn_h


/*##########################################################################*/
#define Q_ROM                PROGMEM
#define Q_ROM_BYTE(rom_var_) pgm_read_byte_near(&(rom_var_))
#define Q_ROM_PTR(rom_var_)  pgm_read_word_near(&(rom_var_))

/* QF-nano interrupt disable/enable... */
#define QF_INT_DISABLE() __asm__ __volatile__ ("cli" ::)
#define QF_INT_ENABLE()  __asm__ __volatile__ ("sei" ::)

/* QF-nano interrupt disabling policy for interrupt level */
/*#define QF_ISR_NEST*/  /* nesting of ISRs not allowed */

/* QV sleep mode
*
* As described in the "AVR Datasheet" in Section "Reset and Interrupt
* Handling", when using the SEI instruction to enable interrupts, the
* instruction following SEI will be executed before any pending interrupts.
* As the Datasheet shows in the assembly example, the pair of instructions
*     SEI       ; enable interrupts
*     SLEEP     ; go to the sleep mode
* executes ATOMICALLY, and so no interrupt can be serviced between these
* instructins. You should NEVER separate these two lines.
*/
#define QV_CPU_SLEEP()          do { \
    __asm__ __volatile__ ("sei" ::); \
    __asm__ __volatile__ ("sleep" ::); \
    SMCR = 0U; \
} while (0)

/* QF CPU reset for AVR */
#define QF_RESET()       __asm__ __volatile__ ("jmp 0x0000" ::)

#include <avr/pgmspace.h>    /* accessing data in program memory (PROGMEM) */
#include <avr/interrupt.h>   /* AVR interrupt support */
#include <avr/io.h>          /* SREG/SMCR definitions */

#include <stdint.h>      /* Exact-width types. WG14/N843 C99 Standard */
#include <stdbool.h>     /* Boolean type.      WG14/N843 C99 Standard */

/*##########################################################################*/
/* #include "qepn.h" */  /* QEP-nano platform-independent public interface */

/****************************************************************************/
/*! The current QP version as a decimal constant XYZ, where X is a 1-digit
* major version number, Y is a 1-digit minor version number, and Z is
* a 1-digit release number.
*/
#define QP_VERSION      630

/*! The current QP version number string of the form X.Y.Z, where X is
* a 1-digit major version number, Y is a 1-digit minor version number,
* and Z is a 1-digit release number.
*/
#define QP_VERSION_STR  "6.3.0"

/*! Tamperproof current QP release (6.3.0) and date (2018-05-10) */
#define QP_RELEASE      0x946859A9U


/****************************************************************************/
/* typedefs for basic numerical types; MISRA-C 2004 rule 6.3(req). */

/*! typedef for character strings. */
/**
* @description
* This typedef specifies character type for exclusive use in character
* strings. Use of this type, rather than plain 'char', is in compliance
* with the MISRA-C 2004 Rules 6.1(req), 6.3(adv).
*/
typedef char char_t;

/*! typedef for line numbers in assertions and return from QF_run() */
typedef int int_t;

/*! typedef for enumerations used for event signals */
typedef int enum_t;

/*! IEEE 754 32-bit floating point number, MISRA-C 2004 rule 6.3(req) */
/**
* @note QP-nano does not use floating-point types anywhere in the internal
* implementation.
*/
typedef float float32_t;

/*! IEEE 754 64-bit floating point number, MISRA-C 2004 rule 6.3(req) */
/**
* @note QP-nano does not use floating-point types anywhere in the internal
* implementation.
*/
typedef double float64_t;

/*! Scalar type describing the signal of an event. */
typedef uint8_t QSignal;

#ifndef Q_PARAM_SIZE
    /*! The size of event parameter Valid values 0, 1, 2, or 4; default 0 */
    #define Q_PARAM_SIZE 0
#endif
#if (Q_PARAM_SIZE == 0)
#elif (Q_PARAM_SIZE == 1)
    typedef uint8_t QParam;
#elif (Q_PARAM_SIZE == 2)
    typedef uint16_t QParam;
#elif (Q_PARAM_SIZE == 4)
    /*! type of the event parameter. */
    /**
    * @description
    * This typedef is configurable via the preprocessor switch #Q_PARAM_SIZE.
    * The other possible values of this type are as follows: @n
    * none when (Q_PARAM_SIZE == 0); @n
    * uint8_t when (Q_PARAM_SIZE == 1); @n
    * uint16_t when (Q_PARAM_SIZE == 2); and @n
    * uint32_t when (Q_PARAM_SIZE == 4).
    */
    typedef uint32_t QParam;
#else
    #error "Q_PARAM_SIZE defined incorrectly, expected 0, 1, 2, or 4"
#endif

/****************************************************************************/
/*! Event structure. */
/**
* @description
* ::QEvt represents events, optionally with a single scalar parameter.
* @sa Q_PARAM_SIZE
* @sa ::QParam
*/
typedef struct {
    QSignal sig; /*!< signal of the event */
#if (Q_PARAM_SIZE != 0)
    QParam par;  /*!< scalar parameter of the event */
#endif
} QEvt;

/****************************************************************************/
/*! Macro to access the signal of the current event of a state machine */
/**
* @param[in,out] me_ pointer to a subclass of ::QHsm (see @ref oop)
*/
#define Q_SIG(me_)  (((QHsm *)(me_))->evt.sig)

#if (Q_PARAM_SIZE != 0)
/*! Macro to access the parameter of the current event of a state machine */
/**
* @param[in,out] me_ pointer to a subclass of ::QHsm (see @ref oop)
*/
#define Q_PAR(me_)  (((QHsm *)(me_))->evt.par)
#endif  /* (Q_PARAM_SIZE != 0) */

/****************************************************************************/
/*! Type returned from  a state-handler function. */
typedef uint_fast8_t QState;

/*! the signature of a state handler function */
typedef QState (*QStateHandler)(void * const me);

/****************************************************************************/
/*! virtual table for the ::QHsm class. */
typedef struct QHsmVtbl QHsmVtbl;

/*! Hierarchical State Machine */
/**
* @description
* QHsm represents a Hierarchical Finite State Machine (HSM) with full
* support for hierarchical nesting of states, entry/exit actions,
* and initial transitions in any composite state.
*
* @note QHsm is not intended to be instantiated directly, but rather serves
* as the base structure for derivation of state machines in the application
* code.
*
* @usage
* The following example illustrates how to derive a state machine structure
* from QHsm. Please note that the QHsm member 'super' is defined as the
* _first_ member of the derived struct.
* @include qepn_qhsm.c
*
* @sa @ref oop
*/
typedef struct {
    QHsmVtbl const *vptr; /*!< virtual pointer */
    QStateHandler state;  /*!< current active state (state-variable) */
    QStateHandler temp;   /*!< temporary: tran. chain, target state, etc. */
    QEvt evt;  /*!< currently processed event in the HSM (protected) */
} QHsm;

/*! Virtual table for the QHsm class */
struct QHsmVtbl {
    /*! Triggers the top-most initial transition in a HSM. */
    void (*init)(QHsm * const me);

    /*! Dispatches an event to a HSM. */
    void (*dispatch)(QHsm * const me);
};

/*! Polymorphically executes the top-most initial transition in a SM. */
/**
* @param[in,out] me_ pointer (see @ref oop)
*
* @note Must be called only ONCE after the SM "constructor".
*
* @usage
* The following example illustrates how to initialize a SM, and dispatch
* events to it:
* @include qepn_qhsm_use.c
*/
#define QHSM_INIT(me_) ((*(me_)->vptr->init)((me_)))

/*! Polymorphically dispatches an event to a HSM. */
/**
* @description
* Processes one event at a time in Run-to-Completion fashion.
*
* @param[in,out] me_ pointer (see @ref oop)
*
* @note Must be called after the "constructor" and after QHSM_INIT().
*/
#define QHSM_DISPATCH(me_) ((*(me_)->vptr->dispatch)((me_)))

/* public methods */
/*! protected "constructor" of a HSM. */
void QHsm_ctor(QHsm * const me, QStateHandler initial);

/*! Obtain the current active state from a HSM (read only). */
/**
* @param[in] me_ pointer (see @ref oop)
*
* @returns the current active state of a HSM
*/
#define QHsm_state(me_) (Q_STATE_CAST(Q_HSM_UPCAST(me_)->state))

/*! Obtain the current active child state of a given parent in QHsm */
/**
* @param[in] me_     pointer (see @ref oop)
* @param[in] parent_ pointer to the parent state-handler
* @returns the current active child state-handler of a given parent
* @note this macro is used in QM for auto-generating code for state history
*/
#define QHsm_childState(me_, parent_) \
    QHsm_childState_(Q_HSM_UPCAST(me_), Q_STATE_CAST(parent_))

/*! Helper function to obtain the current active child state of a parent */
QStateHandler QHsm_childState_(QHsm * const me,
                               QStateHandler const parent);

/*! Implementation of the top-most initial transition in QHsm. */
void QHsm_init_(QHsm * const me);

/*! Implementation of dispatching events to QHsm. */
void QHsm_dispatch_(QHsm * const me);

/*! the top-state. */
QState QHsm_top(void const * const me);


/****************************************************************************/
/*! All possible values returned from state/action handlers */
enum {
    /* unhandled and need to "bubble up" */
    Q_RET_SUPER,     /*!< event passed to superstate to handle */
    Q_RET_SUPER_SUB, /*!< event passed to submachine superstate */
    Q_RET_UNHANDLED, /*!< event unhandled due to a guard */

    /* handled and do not need to "bubble up" */
    Q_RET_HANDLED,   /*!< event handled (internal transition) */
    Q_RET_IGNORED,   /*!< event silently ignored (bubbled up to top) */

    /* entry/exit */
    Q_RET_ENTRY,     /*!< state entry action executed */
    Q_RET_EXIT,      /*!< state exit  action executed */

    /* no side effects */
    Q_RET_NULL,      /*!< return value without any effect */

    /* transitions need to execute transition-action table in QHsm */
    Q_RET_TRAN,      /*!< event handled (regular transition) */
    Q_RET_TRAN_INIT, /*!< initial transition in a state or submachine */
    Q_RET_TRAN_HIST  /*!< event handled (transition to history) */
};

/*! Perform upcast from a subclass of ::QHsm to the base class ::QHsm */
/**
* @description
* Upcasting from a subclass to superclass is a very frequent and __safe__
* operation in object-oriented programming and object-oriented languages
* (such as C++) perform such upcasting automatically. However, OOP is
* implemented in C just as a set of coding conventions (see @ref oop),
* and the C compiler does not "know" that certain types are related by
* inheritance. Therefore for C, the upcast must be performed explicitly.
* Unfortunately, pointer casting violates the advisory MISRA-C 2004 rule 11.4
* "cast pointer to pointer". This macro encapsulates this deviation and
* provides a descriptive name for the reason of this cast.
*/
#define Q_HSM_UPCAST(ptr_) ((QHsm *)(ptr_))

/*! Perform cast to ::QStateHandler. */
/**
* @description
* This macro encapsulates the cast of a specific state handler function
* pointer to QStateHandler, which violates MISRA-C 2004 rule 11.4(advisory).
* This macro helps to localize this deviation.
*
* @usage
* @include qepn_qhsm_ctor.c
*/
#define Q_STATE_CAST(handler_)  ((QStateHandler)(handler_))

/*! Macro to call in a state-handler when it executes a regular
* or and initial transition. Applicable to both HSMs and FSMs.
* @include qepn_qtran.c
*/
#define Q_TRAN(target_)  \
    ((Q_HSM_UPCAST(me))->temp = Q_STATE_CAST(target_), (QState)Q_RET_TRAN)

/*! Macro to call in a state-handler when it executes a transition
* to history. Applicable only to HSMs.
*
* @usage
* @include qepn_qhist.c
*/
#define Q_TRAN_HIST(hist_)  \
    ((Q_HSM_UPCAST(me))->temp = (hist_), (QState)Q_RET_TRAN_HIST)

/*! Macro to call in a state-handler when it designates the
* superstate of a given state. Applicable only to HSMs.
*
* @usage
* @include qepn_qtran.c
*/
#define Q_SUPER(super_)  \
    ((Q_HSM_UPCAST(me))->temp = Q_STATE_CAST(super_), (QState)Q_RET_SUPER)

/*! Macro to call in a state-handler when it handles an event.
*  Applicable to both HSMs and FSMs.
*/
#define Q_HANDLED()      ((QState)Q_RET_HANDLED)

/*! Macro to call in a state-handler when it attempts to handle
* an event but a guard condition evaluates to 'false' and there is no other
* explicit way of handling the event. Applicable only to HSMs.
*/
#define Q_UNHANDLED()    ((QState)Q_RET_UNHANDLED)


/*! QP reserved signals */
enum {
    Q_ENTRY_SIG = 1,  /*!< signal for coding entry actions */
    Q_EXIT_SIG,       /*!< signal for coding exit actions */
    Q_INIT_SIG,       /*!< signal for coding nested initial transitions */
    Q_TIMEOUT_SIG,    /*!< timeout signal at the default tick rate 0 */
    Q_TIMEOUT1_SIG,   /*!< timeout signal at tick rate 1 */
    Q_TIMEOUT2_SIG,   /*!< timeout signal at tick rate 2 */
    Q_TIMEOUT3_SIG,   /*!< timeout signal at tick rate 3 */
    Q_USER_SIG        /*!< first signal for the user applications */
};

/*! Perform cast from unsigned integer to a pointer of type @a type_ */
/**
* @description
* This macro encapsulates the cast to (type_ *), which QP ports or
* application might use to access embedded hardware registers.
* Such uses can trigger PC-Lint "Note 923: cast from int to pointer" and
* this macro helps to encapsulate this deviation.
*/
#define Q_UINT2PTR_CAST(type_, uint_)  ((type_ *)(uint_))

/****************************************************************************/
/* macros for accessing data in ROM */
#ifndef Q_ROM /* if NOT defined, provide the default definition */

    /*! Macro to specify compiler-specific directive for placing a
    * constant object in ROM. */
    /**
    * @description
    * Many compilers for 8-bit Harvard-architecture MCUs provide non-standard
    * extensions to support placement of objects in different memories.
    * In order to conserve the precious RAM, QP-nano uses the Q_ROM macro for
    * all constant objects that can be allocated in ROM.
    *
    * @note
    * To override the following empty definition, you need to define the
    * Q_ROM macro in the qpn_port.h header file. Some examples of valid
    * Q_ROM macro definitions are: __code (IAR 8051 compiler), code (Keil
    * 8051 compiler), PROGMEM (gcc for AVR), __flash (IAR for AVR).
    */
    #define Q_ROM
#endif

#ifndef Q_ROM_BYTE
    /*! Macro to access a byte allocated in ROM */
    /**
    * Some compilers for Harvard-architecture MCUs, such as gcc for AVR, do
    * not generate correct code for accessing data allocated in the program
    * space (ROM). The workaround for such compilers is to explicitly add
    * assembly code to access each data element allocated in the program
    * space. The macro Q_ROM_BYTE() retrieves a byte from the given ROM
    * address.
    *
    * @note
    * The Q_ROM_BYTE() macro should be defined in the qpn_port.h header file
    * for each compiler that cannot handle correctly data allocated in ROM
    * (such as the gcc). If the macro is left undefined, the default
    * definition simply returns the parameter and lets the compiler
    * synthesize the correct code.
    */
    #define Q_ROM_BYTE(rom_var_)   (rom_var_)
#endif

#ifndef Q_ROM_PTR
    /*! Macro to access a pointer allocated in ROM */
    /**
    * Some compilers for Harvard-architecture MCUs, such as gcc for AVR, do
    * not generate correct code for accessing data allocated in the program
    * space (ROM). The workaround for such compilers is to explicitly add
    * assembly code to access each data element allocated in the program
    * space. The macro Q_ROM_PTR() retrieves an object-pointer from the given
    * ROM address. Please note that the pointer can be pointing to the object
    * in RAM or ROM.
    *
    * @note
    * The Q_ROM_PTR() macro should be defined in the qpn_port.h header file
    * for each compiler that cannot handle correctly data allocated in ROM
    * (such as the gcc). If the macro is left undefined, the default
    * definition simply returns the parameter and lets the compiler
    * synthesize the correct code.
    */
    #define Q_ROM_PTR(rom_var_)    (rom_var_)
#endif


/****************************************************************************/
/*! the current QP version number string in ROM, based on QP_VERSION_STR */
extern char_t const Q_ROM QP_versionStr[6];

/*! get the current QP-nano version number string of the form "X.Y.Z" */
#define QP_getVersion() (QP_versionStr)


/****************************************************************************/
/*! Helper macro to calculate static dimension of a 1-dim array @a array_ */
#define Q_DIM(array_) (sizeof(array_) / sizeof((array_)[0]))


/*##########################################################################*/
/* #include "qfn.h" */   /* QF-nano  platform-independent public interface */

/****************************************************************************/
#ifndef QF_TIMEEVT_CTR_SIZE
    /*! macro to override the default QTimeEvtCtr size.
    * Valid values 0, 1, 2, or 4; default 0
    */
    #define QF_TIMEEVT_CTR_SIZE 0
#endif
#if (QF_TIMEEVT_CTR_SIZE == 0)
    /* no time events */
#elif (QF_TIMEEVT_CTR_SIZE == 1)
    typedef uint8_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 2)
    /*! type of the Time Event counter, which determines the dynamic
    * range of the time delays measured in clock ticks.
    */
    /**
    * @description
    * This typedef is configurable via the preprocessor switch
    * #QF_TIMEEVT_CTR_SIZE. The other possible values of this type are
    * as follows: @n
    * none when (QF_TIMEEVT_CTR_SIZE not defined or == 0), @n
    * uint8_t  when (QF_TIMEEVT_CTR_SIZE == 1); @n
    * uint16_t when (QF_TIMEEVT_CTR_SIZE == 2); and @n
    * uint32_t when (QF_TIMEEVT_CTR_SIZE == 4).
    */
    typedef uint16_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 4)
    typedef uint32_t QTimeEvtCtr;
#else
    #error "QF_TIMER_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

#if (QF_TIMEEVT_CTR_SIZE != 0)
    /*! Timer structure the active objects */
    typedef struct {
        QTimeEvtCtr nTicks;   /*!< timer tick counter */
#ifdef QF_TIMEEVT_PERIODIC
        QTimeEvtCtr interval; /*!< timer interval */
#endif /* QF_TIMEEVT_PERIODIC */
    } QTimer;
#endif /* (QF_TIMEEVT_CTR_SIZE != 0) */

#ifndef QF_MAX_TICK_RATE
    /*! Default value of the macro configurable value in qpn_port.h */
    #define QF_MAX_TICK_RATE     1
#elif (QF_MAX_TICK_RATE > 4)
    #error "QF_MAX_TICK_RATE exceeds the 4 limit"
#endif

/****************************************************************************/
/*! QActive active object (based on QHsm-implementation) */
/**
* @description
* QActive is the base structure for derivation of active objects. Active
* objects in QF-nano are encapsulated tasks (each embedding a state machine
* and an event queue) that communicate with one another asynchronously by
* sending and receiving events. Within an active object, events are
* processed sequentially in a run-to-completion (RTC) fashion, while QF
* encapsulates all the details of thread-safe event exchange and queuing.
*
* @note ::QActive is not intended to be instantiated directly, but rather
* serves as the base structure for derivation of active objects in the
* application code.
*
* @usage
* The following example illustrates how to derive an active object from
* ::QActive. Please note that the ::QActive member super_ is defined as
* the __first__ member of the derived struct.
* @include qfn_qactive.c
*/
typedef struct QActive {
    QHsm super; /**< derives from the ::QHsm base class */

#if (QF_TIMEEVT_CTR_SIZE != 0)
    /*! Timer for the active object */
    QTimer tickCtr[QF_MAX_TICK_RATE];
#endif /* (QF_TIMEEVT_CTR_SIZE != 0) */

    /*! priority of the active object (1..8) */
    uint8_t prio;

    /*! offset to where next event will be inserted into the buffer */
    uint8_t volatile head;

    /*! offset of where next event will be extracted from the buffer */
    uint8_t volatile tail;

    /*! number of events currently present in the queue
    * (events in the ring buffer + 1 event in the state machine)
    */
    uint8_t volatile nUsed;

} QActive;

/*! Virtual table for the QActive class */
typedef struct {
    QHsmVtbl super; /*!< inherits QHsmVtbl */

#if (Q_PARAM_SIZE != 0)
    /*! virtual function to asynchronously post (FIFO) an event to an AO
    * (task context).
    */
    /** @sa QACTIVE_POST() and QACTIVE_POST_X() */
    bool (*post)(QActive * const me, uint_fast8_t const margin,
                 enum_t const sig, QParam const par);

    /*! virtual function to asynchronously post (FIFO) an event to an AO
    * (ISR context).
    */
    /** @sa QACTIVE_POST_ISR() and QACTIVE_POST_X_ISR() */
    bool (*postISR)(QActive * const me, uint_fast8_t const margin,
                    enum_t const sig, QParam const par);
#else
    bool (*post)   (QActive * const me, uint_fast8_t const margin,
                    enum_t const sig);
    bool (*postISR)(QActive * const me, uint_fast8_t const margin,
                    enum_t const sig);
#endif
} QActiveVtbl;

/*! protected "constructor" of an QActive active object. */
void QActive_ctor(QActive * const me, QStateHandler initial);


/*! special value of margin that causes asserting failure in case
* event posting fails.
*/
#define QF_NO_MARGIN ((uint_fast8_t)0xFF)

#if (Q_PARAM_SIZE != 0)
    /*! Polymorphically posts an event to an active object (FIFO)
    * with delivery guarantee (task context).
    */
    /**
    * @description
    * This macro asserts if the queue overflows and cannot accept the event.
    *
    * @param[in,out] me_   pointer (see @ref oop)
    * @param[in]     sig_  signal of the event to post
    * @param[in]     par_  parameter of the event to post.
    *
    * @sa QACTIVE_POST_X(), QActive_postX_(),
    * QACTIVE_POST_ISR(), QActive_postXISR_().
    *
    * @usage
    * @include qfn_post.c
    */
    #define QACTIVE_POST(me_, sig_, par_) \
        ((void)(*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->post)( \
                QF_ACTIVE_CAST((me_)), QF_NO_MARGIN, \
                (enum_t)(sig_), (QParam)(par_)))

    /*! Polymorphically posts an event to an active object (FIFO)
    * without delivery guarantee (task context).
    */
    /**
    * @description
    * This macro does not assert if the queue overflows and cannot accept
    * the event with the specified margin of free slots remaining.
    *
    * @param[in,out] me_     pointer (see @ref oop)
    * @param[in]     margin_ the minimum free slots in the queue, which
    *                must still be available after posting the event.
    *                The special value #QF_NO_MARGIN causes asserting failure
    *                in case event allocation fails.
    * @param[in]     sig_    signal of the event to post
    * @param[in]     par_    parameter of the event to post.
    *
    * @returns
    * 'true' if the posting succeeded, and 'false' if the posting failed
    * due to insufficient margin of free slots available in the queue.
    *
    * @usage
    * @include qfn_postx.c
    */
    #define QACTIVE_POST_X(me_, margin_, sig_, par_) \
        ((*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->post)( \
                QF_ACTIVE_CAST((me_)), \
                (margin_), (enum_t)(sig_), (QParam)(par_)))

    /*! Polymorphically posts an event to an active object (FIFO)
    * with delivery guarantee (ISR context).
    */
    /**
    * @description
    * This macro asserts if the queue overflows and cannot accept the event.
    *
    * @param[in,out] me_   pointer (see @ref oop)
    * @param[in]     sig_  signal of the event to post
    * @param[in]     par_  parameter of the event to post.
    *
    * @sa QACTIVE_POST_X(), QActive_postX_().
    *
    * @usage
    * @include qfn_post.c
    */
    #define QACTIVE_POST_ISR(me_, sig_, par_) \
        ((void)(*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                QF_ACTIVE_CAST((me_)), QF_NO_MARGIN, \
                (enum_t)(sig_), (QParam)(par_)))

    /*! Polymorphically posts an event to an active object (FIFO)
    * without delivery guarantee (ISR context).
    */
    /**
    * @description
    * This macro does not assert if the queue overflows and cannot accept
    * the event with the specified margin of free slots remaining.
    *
    * @param[in,out] me_     pointer (see @ref oop)
    * @param[in]     margin_ the minimum free slots in the queue, which
    *                must still be available after posting the event.
    *                The special value #QF_NO_MARGIN causes asserting failure
    *                in case event allocation fails.
    * @param[in]     sig_    signal of the event to post
    * @param[in]     par_    parameter of the event to post.
    *
    * @returns
    * 'true' if the posting succeeded, and 'false' if the posting failed
    * due to insufficient margin of free slots available in the queue.
    *
    * @usage
    * @include qfn_postx.c
    */
    #define QACTIVE_POST_X_ISR(me_, margin_, sig_, par_) \
        ((*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                QF_ACTIVE_CAST((me_)), (margin_), \
                (enum_t)(sig_), (QParam)(par_)))

    /*! Implementation of the task-level event posting */
    bool QActive_postX_(QActive * const me, uint_fast8_t margin,
                        enum_t const sig, QParam const par);

    /*! Implementation of the ISR-level event posting */
    bool QActive_postXISR_(QActive * const me, uint_fast8_t margin,
                           enum_t const sig, QParam const par);

#else /* no event parameter */
    #define QACTIVE_POST(me_, sig_) \
        ((void)(*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->post)( \
                QF_ACTIVE_CAST((me_)), (uint_fast8_t)0, (enum_t)(sig_)))

    #define QACTIVE_POST_X(me_, margin_, sig_) \
        ((*((QActiveVtbl const *)((me_)->super.vptr))->post)((me_), \
                                   (margin_), (sig_)))

    bool QActive_postX_(QActive * const me, uint_fast8_t margin,
                        enum_t const sig);

    #define QACTIVE_POST_ISR(me_, sig_) \
        ((void)(*((QActiveVtbl const*)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                 QF_ACTIVE_CAST((me_)), (uint_fast8_t)0, (enum_t)(sig_)))

    #define QACTIVE_POST_X_ISR(me_, margin_, sig_) \
        ((*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                QF_ACTIVE_CAST((me_)), (margin_), (enum_t)(sig_)))

    bool QActive_postXISR_(QActive * const me, uint_fast8_t margin,
                           enum_t const sig);
#endif

#if (QF_TIMEEVT_CTR_SIZE != 0)

    /*! Processes all armed time events at every clock tick. */
    void QF_tickXISR(uint_fast8_t const tickRate);

#ifdef QF_TIMEEVT_PERIODIC
    /*! Arm the QP-nano one-shot time event. */
    void QActive_armX(QActive * const me, uint_fast8_t const tickRate,
                      QTimeEvtCtr const nTicks, QTimeEvtCtr const interval);
#else
    /*! Arm the QP-nano one-shot time event. */
    void QActive_armX(QActive * const me, uint_fast8_t const tickRate,
                      QTimeEvtCtr const nTicks);
#endif

    /*! Disarm a time event. Since the tick counter */
    void QActive_disarmX(QActive * const me, uint_fast8_t const tickRate);

#endif /* (QF_TIMEEVT_CTR_SIZE != 0) */


/****************************************************************************/
/* QF-nano protected methods ...*/

/*! QF-nano initialization. */
void QF_init(uint_fast8_t maxActive);

/*! QF-nano termination. */
/**
* @description
* This function terminates QF and performs any necessary cleanup.
* In QF-nano this function is defined in the BSP. Many QF ports might not
* require implementing QF_stop() at all, because many embedded applications
* don't have anything to exit to.
*/
void QF_stop(void);

/*! Startup QF-nano callback. */
/**
* @description
* The time line for calling QF_onStartup() depends on the particular
* QF port. In most cases, QF_onStartup() is called from QF_run(), right
* before starting any multitasking kernel or the background loop.
*
* @sa QF initialization example for ::QActiveCB.
*/
void QF_onStartup(void);

/*! Transfers control to QF-nano to run the application. */
int_t QF_run(void);


/****************************************************************************/
/*! QActive Control Block
*
* QActiveCB represents the read-only information that the QF-nano needs to
* manage the active object. QActiveCB objects are grouped in the array
* QF_active[], which typically can be placed in ROM.
*
* @usage
* The following example illustrates how to allocate and initialize the
* ::QActive control blocks in the array QF_active[].
* @include qfn_main.c
*/
typedef struct {
    QActive  *act;   /*!< pointer to the active object structure */
    QEvt     *queue; /*!< pointer to the event queue buffer */
    uint8_t   qlen;  /*!< the length of the queue ring buffer */
} QActiveCB;

/** active object control blocks */
/*lint -save -e960    MISRA-C:2004 8.12, extern array declared without size */
extern QActiveCB const Q_ROM QF_active[];
/*lint -restore */

/*! number of active objects in the application (# elements in QF_active[]) */
extern uint_fast8_t QF_maxActive_;

/*! Ready set of QF-nano. */
extern uint_fast8_t volatile QF_readySet_;

#ifndef QF_LOG2

    /*! Lookup table for (log2(n) + 1), where n is the index into the table.
    * This lookup delivers the 1-based number of the most significant 1-bit
    * of a nibble.
    */
    extern uint8_t const Q_ROM QF_log2Lkup[16];

#endif


#ifdef QF_TIMEEVT_USAGE

    /*! Timer set of QF-nano. */
    extern uint_fast8_t volatile QF_timerSetX_[QF_MAX_TICK_RATE];

#endif  /* QF_TIMEEVT_USAGE */


/*! Lookup table for ~(1 << (n - 1)), where n is the index into the table. */
extern uint8_t const Q_ROM QF_invPow2Lkup[9];


/****************************************************************************/
/*! This macro encapsulates accessing the active object queue at a
* given index, which violates MISRA-C 2004 rules 17.4(req) and 11.4(adv).
* This macro helps to localize this deviation.
*/
#define QF_ROM_QUEUE_AT_(ao_, i_) (((QEvt *)Q_ROM_PTR((ao_)->queue))[(i_)])

/*! This macro encapsulates accessing the active object control block,
* which violates MISRA-C 2004 rule 11.4(adv). This macro helps to localize
* this deviation.
*/
#define QF_ROM_ACTIVE_GET_(p_) ((QActive *)Q_ROM_PTR(QF_active[(p_)].act))

/*! This macro encapsulates the upcast to QActive*
*
* This macro encapsulates up-casting a pointer to a subclass of ::QActive
* to the base class ::QActive, which violates MISRA-C 2004 rule 11.4(adv).
* This macro helps to localize this deviation.
*/
#define QF_ACTIVE_CAST(a_)     ((QActive *)(a_))


/*##########################################################################*/
/* #include "qvn.h" */  /* QV-nano  platform-independent public interface */

/*! Preprocessor switch for configuring cooperative real-time kernel
* (QV-nano). The macro is automatically defined by including the qvn.h file
* in qpn_port.h.
*/
/**
* @note If defined, this macro eliminates the code for the preemptive
* QK-nano kernel provided in QF-nano. Instead, the fully preemptive QK-nano
* real-time kernel is used.
*/
#define QV_COOPERATIVE  1

/*! Ready set of QV-nano. */
extern uint_fast8_t volatile QV_readySet_;

/*! QV idle callback (customized in BSPs for QK)
*
* QV_onIdle() is called continuously by the QV-nano scheduler. This callback
* gives the application an opportunity to enter a power-saving CPU mode,
* or perform some other idle processing.
*
* @note QV_onIdle() is invoked with interrupts disabled, but must return
* with interrupts enabled. This is in contrast to the callback QK_onIdle(),
* which is used by the preemptive QK-nano scheduler.
*/
void QV_onIdle(void);


/*##########################################################################*/
/* #include "qassert.h" */  /* embedded systems-friendly assertions */

/**
* @note
* This header file can be used in C, C++, and mixed C/C++ programs.
*
* @note The preprocessor switch #Q_NASSERT disables checking assertions.
* However, it is generally __not__ advisable to disable assertions,
* __especially__ in the production code. Instead, the assertion handler
* Q_onAssert() should be very carefully designed and tested.
*/

#ifdef Q_NASSERT /* Q_NASSERT defined--assertion checking disabled */

    #define Q_DEFINE_THIS_FILE
    #define Q_DEFINE_THIS_MODULE(name_)
    #define Q_ASSERT(test_)             ((void)0)
    #define Q_ASSERT_ID(id_, test_)     ((void)0)
    #define Q_ALLEGE(test_)             ((void)(test_))
    #define Q_ALLEGE_ID(id_, test_)     ((void)(test_))
    #define Q_ERROR()                   ((void)0)
    #define Q_ERROR_ID(id_)             ((void)0)
    #define Q_REQUIRE(test_)            ((void)0)
    #define Q_REQUIRE_ID(id_, test_)    ((void)0)
    #define Q_ENSURE(test_)             ((void)0)
    #define Q_ENSURE_ID(id_, test_)     ((void)0)
    #define Q_INVARIANT(test_)          ((void)0)
    #define Q_INVARIANT_ID(id_, test_)  ((void)0)

#else  /* Q_NASSERT not defined--assertion checking enabled */

    /*! Define the file name (with `__FILE__`) for assertions in this file. */
    /**
    * @description
    * Macro to be placed at the top of each C/C++ module to define the
    * single instance of the file name string to be used in reporting
    * assertions in this module.
    *
    * @note The file name string literal is defined by means of the standard
    * preprocessor macro `__FILE__`. However, please note that, depending
    * on the compiler, the `__FILE__` macro might contain the whole path name
    * to the file, which might be inconvenient to log assertions.
    * @note This macro should __not__ be terminated by a semicolon.
    * @sa Q_DEFINE_THIS_MODULE()
    */
    #define Q_DEFINE_THIS_FILE \
        static char_t const Q_ROM Q_this_module_[] = __FILE__;

    /*! Define the user-specified module name for assertions in this file. */
    /**
    * @description
    * Macro to be placed at the top of each C/C++ module to define the
    * single instance of the module name string to be used in reporting
    * assertions in this module. This macro takes the user-supplied parameter
    * @p name_ instead of `__FILE__` to precisely control the name of the
    * module.
    *
    * @param[in] name_ string constant representing the module name
    *
    * @note This macro should __not__ be terminated by a semicolon.
    */
    #define Q_DEFINE_THIS_MODULE(name_) \
        static char_t const Q_ROM Q_this_module_[] = name_;

    /*! General purpose assertion. */
    /**
    * @description
    * Makes sure the @p test_ parameter is TRUE. Calls the Q_onAssert()
    * callback if the @p test_ expression evaluates to FALSE. This
    * macro identifies the assertion location within the file by means
    * of the standard `__LINE__` macro.
    *
    * @param[in] test_ Boolean expression
    *
    * @note the @p test_ is __not__ evaluated if assertions are disabled
    * with the #Q_NASSERT switch.
    */
    #define Q_ASSERT(test_) ((test_) \
        ? (void)0 : Q_onAssert(&Q_this_module_[0], (int_t)__LINE__))

    /*! General purpose assertion with user-specified assertion-id. */
    /**
    * @description
    * Makes sure the @p test_ parameter is TRUE. Calls the Q_onAssert()
    * callback if the @p test_ evaluates to FALSE. This assertion takes the
    * user-supplied parameter @p id_ to identify the location of this
    * assertion within the file. This avoids the volatility of using line
    * numbers, which change whenever a line of code is added or removed
    * upstream from the assertion.
    *
    * @param[in] id_   ID number (unique within the module) of the assertion
    * @param[in] test_ Boolean expression
    *
    * @note the @p test_ expression is __not__ evaluated if assertions are
    * disabled with the #Q_NASSERT switch.
    */
    #define Q_ASSERT_ID(id_, test_) ((test_) \
        ? (void)0 : Q_onAssert(&Q_this_module_[0], (int_t)(id_)))

    /*! General purpose assertion that __always__ evaluates the @p test_
    * expression. */
    /**
    * @description
    * Like the Q_ASSERT() macro, except it __always__ evaluates the @p test_
    * expression even when assertions are disabled with the #Q_NASSERT macro.
    * However, when the #Q_NASSERT macro is defined, the Q_onAssert()
    * callback is __not__ called, even if @p test_ evaluates to FALSE.
    *
    * @param[in] test_ Boolean expression (__always__ evaluated)
    *
    * @sa #Q_ALLEGE_ID
    */
    #define Q_ALLEGE(test_)    Q_ASSERT(test_)

    /*! General purpose assertion with user-specified assertion-id that
    * __always__ evaluates the @p test_ expression. */
    /**
    * @description
    * Like the Q_ASSERT_ID() macro, except it __always__ evaluates the
    * @p test_ expression even when assertions are disabled with the
    * #Q_NASSERT macro. However, when the #Q_NASSERT macro is defined, the
    * Q_onAssert() callback is __not__ called, even if @p test_ evaluates
    * to FALSE.
    *
    * @param[in] id_   ID number (unique within the module) of the assertion
    * @param[in] test_ Boolean expression
    */
    #define Q_ALLEGE_ID(id_, test_) Q_ASSERT_ID((id_), (test_))

    /*! Assertion for a wrong path through the code. */
    /**
    * @description
    * Calls the Q_onAssert() callback if ever executed.
    *
    * @note Does noting if assertions are disabled with the #Q_NASSERT switch.
    */
    #define Q_ERROR() \
        Q_onAssert(&Q_this_module_[0], (int_t)__LINE__)

    /*! Assertion with user-specified assertion-id for a wrong path. */
    /**
    * @description
    * Calls the Q_onAssert() callback if ever executed. This assertion
    * takes the user-supplied parameter @p id_ to identify the location of
    * this assertion within the file. This avoids the volatility of using
    * line numbers, which change whenever a line of code is added or removed
    * upstream from the assertion.
    *
    * @param[in] id_   ID number (unique within the module) of the assertion
    *
    * @note Does noting if assertions are disabled with the #Q_NASSERT switch.
    */
    #define Q_ERROR_ID(id_) \
        Q_onAssert(&Q_this_module_[0], (int_t)(id_))

#endif /* Q_NASSERT */

#ifdef __cplusplus
    extern "C" {
#endif

/*! Callback function invoked in case of any assertion failure. */
/**
* @description
* This is an application-specific callback function needs to be defined in
* the application to perform the clean system shutdown and perhaps a reset.
*
* @param[in] module name of the file/module in which the assertion failed
*                   (constant ROM-based, zero-terminated C string)
* @param[in] loc    location of the assertion within the module. This could
*                   be a line number or a user-specified ID-number.
*
* @note This callback function should _not_ return, as continuation after
* an assertion failure does not make sense.
*
* @note The Q_onAssert() function is the last line of defense after the
* system failure and its implementation shouild be very __carefully__
* designed and __tested__ under various fault conditions, including but
* not limited to: stack overflow, stack corruption, or calling Q_onAssert()
* from an interrupt.
*
* @note It is typically a __bad idea__ to implement Q_onAssert() as an
* endless loop that ties up the CPU. During debuggin, Q_onAssert() is an
* ideal place to put a breakpoint.
*
* Called by the following macros: #Q_ASSERT, #Q_REQUIRE, #Q_ENSURE,
* #Q_ERROR, #Q_ALLEGE as well as #Q_ASSERT_ID, #Q_REQUIRE_ID, #Q_ENSURE_ID,
* #Q_ERROR_ID, and #Q_ALLEGE_ID.
*/
void Q_onAssert(char_t const Q_ROM * const module, int_t location);

#ifdef __cplusplus
    }
#endif

/*! Assertion for checking preconditions. */
/**
* @description
* This macro is equivalent to #Q_ASSERT, except the name provides a better
* documentation of the intention of this assertion.
*
* @param[in] test_ Boolean expression
*/
#define Q_REQUIRE(test_)         Q_ASSERT(test_)

/*! Assertion for checking preconditions with user-specified assertion-id. */
/**
* @description
* Equivalent to #Q_ASSERT_ID, except the macro name provides a better
* documentation of the intention of this assertion.
*
* @param[in] id_   ID number (unique within the module) of the assertion
* @param[in] test_ Boolean expression
*/
#define Q_REQUIRE_ID(id_, test_) Q_ASSERT_ID((id_), (test_))

/*! Assertion for checking postconditions. */
/** Equivalent to #Q_ASSERT, except the macro name provides a better
* documentation of the intention of this assertion.
*
* @param[in] test_ Boolean expression
*/
#define Q_ENSURE(test_) Q_ASSERT(test_)

/*! Assertion for checking postconditions with user-specified assertion-id. */
/**
* @description
* Equivalent to #Q_ASSERT_ID, except the name provides a better documentation
* of the intention of this assertion.
*
* @param[in] id_   ID number (unique within the module) of the assertion
* @param[in] test_ Boolean expression
*/
#define Q_ENSURE_ID(id_, test_) Q_ASSERT_ID((id_), (test_))

/*! Assertion for checking invariants. */
/**
* @description
* Equivalent to #Q_ASSERT, except the macro name provides a better
* documentation of the intention of this assertion.
*
* @param[in] test_ Boolean expression
*/
#define Q_INVARIANT(test_) Q_ASSERT(test_)

/*! Assertion for checking invariants with user-specified assertion-id. */
/**
* @description
* Equivalent to #Q_ASSERT_ID, except the macro name provides a better
* documentation of the intention of this assertion.
*
* @param[in] id_   ID number (unique within the module) of the assertion
* @param[in] test_ Boolean expression
*/
#define Q_INVARIANT_ID(id_, test_) Q_ASSERT_ID((id_), (test_))

/*! Compile-time assertion. */
/**
* @description
* This type of assertion deliberately causes a compile-time error when
* the @p test_ evaluates to FALSE. The macro exploits the fact that in C/C++
* a dimension of an array cannot be negative. The compile-time assertion has
* no runtime side effects.
*
* @param[in] test_ Compile-time Boolean expression
*/
#define Q_ASSERT_COMPILE(test_) \
    extern int_t Q_assert_compile[(test_) ? 1 : -1]


/****************************************************************************/
/****************************************************************************/
/* QP API compatibility layer */
#ifndef QP_API_VERSION

/*! Macro that specifies the backwards compatibility with the
* QP-nano API version.
*/
/**
* @description
* For example, QP_API_VERSION=450 will cause generating the compatibility
* layer with QP-nano version 4.5.0 and newer, but not older than 4.5.0.
* QP_API_VERSION=0 causes generation of the compatibility layer "from the
* begining of time", which is the maximum backwards compatibilty. This is
* the default.@n
* @n
* Conversely, QP_API_VERSION=9999 means that no compatibility layer should
* be generated. This setting is useful for checking if an application
* complies with the latest QP-nano API.
*/
#define QP_API_VERSION 0

#endif  /* QP_API_VERSION */

/****************************************************************************/
#if (QP_API_VERSION < 580)

/*! @deprecated QMActive Control Block; instead use: ::QActiveCB. */
typedef QActiveCB QMActiveCB;

/*! @deprecated QMActive; instead use: ::QActive. */
typedef QActive QMActive;

/*! @deprecated QMsm state machine; instead use: ::QHsm. */
typedef QHsm QMsm;

/*! @deprecated QMActive constructor; instead use: QActive_ctor() */
#define QMActive_ctor QActive_ctor

/*! @deprecated QMsm state machine constructor; instead use: QHsm_ctor() */
#define QMsm_ctor     QHsm_ctor

/*! @deprecated execute the top-most initial transition in QMsm;
* instead use QHSM_INIT()
*/
#define QMSM_INIT(me_)      QHSM_INIT((me_))

/*! @deprecated dispatch an event to QMsm; instead use QHSM_DISPATCH() */
#define QMSM_DISPATCH(me_)  QHSM_DISPATCH((me_))

/*! @deprecated macro for cooperativew "Vanilla" kernel;
* instead use: QV_onIdle() */
#define QF_onIdle   QV_onIdle

#ifdef Q_TIMERSET
    #define QF_TIMEEVT_USAGE
#endif /* Q_TIMERSET */

/****************************************************************************/
#if (QP_API_VERSION < 500)

/*! @deprecated macro for odd 8-bit CPUs. */
#define Q_ROM_VAR

/*! @deprecated macro for odd 8-bit CPUs. */
#define Q_REENTRANT

/*! @deprecated execute the top-most initial transition in QMsm;
* instead use QMSM_INIT()
*/
#define QMsm_init(me_)      QHSM_INIT((me_))

/*! @deprecated dispatch an event to QMsm; instead use QMSM_DISPATCH() */
#define QMsm_dispatch(me_)  QMSM_DISPATCH((me_))

#if (Q_PARAM_SIZE != 0)
    /*! Deprecated API defined for backwards-compatibility */
    #define QActive_post(me_, sig_, par_) \
        QACTIVE_POST((me_), (sig_), (par_))

    /*! Deprecated API defined for backwards-compatibility */
    #define QActive_postISR(me_, sig_, par_) \
        QACTIVE_POST_ISR((me_), (sig_), (par_))
#else
    #define QActive_post(me_, sig_)    QACTIVE_POST((me_), (sig_))
    #define QActive_postISR(me_, sig_) QACTIVE_POST_ISR((me_), (sig_))
#endif

#ifdef QF_TIMEEVT_PERIODIC
    /*! Arming a one-shot time event for the default tick rate (rate 0);
    * instead use QActive_armX()
    */
    #define QActive_arm(me_, ticks_) \
        (QActive_armX((me_), (uint_fast8_t)0, (ticks_), 0U))
#else
    #define QActive_arm(me_, ticks_) \
        (QActive_armX((me_), (uint_fast8_t)0, (ticks_)))
#endif /* QF_TIMEEVT_PERIODIC */

/*! Disarming a time event for the default tick rate (rate 0);
* instead use QActive_disarmX()
*/
#define QActive_disarm(me_)     (QActive_disarmX((me_), (uint_fast8_t)0))


/****************************************************************************/
#if (QP_API_VERSION < 450)

/*! @deprecated QP-nano event type; instead use QEvt */
typedef QEvt QEvent;

#ifdef QF_INT_LOCK
    #define QF_INT_DISABLE()  QF_INT_LOCK()
#endif

#ifdef QF_INT_UNLOCK
    #define QF_INT_ENABLE()   QF_INT_UNLOCK()
#endif

#ifdef QF_ISR_NEST
    #ifdef QF_ISR_KEY_TYPE
        #define QF_ISR_STAT_TYPE  QF_ISR_KEY_TYPE
    #endif
    #ifdef QF_ISR_LOCK
        #define QF_ISR_DISABLE(stat_)  QF_ISR_LOCK(stat_)
    #endif
    #ifdef QF_ISR_UNLOCK
        #define QF_ISR_RESTORE(stat_)  QF_ISR_UNLOCK(stat_)
    #endif
#endif /* QF_ISR_NEST */

#if (QF_TIMEEVT_CTR_SIZE != 0)
    /*! System clock tick processing for the default tick rate 0;
    * instead use QF_tickXISR()
    */
    #define QF_tickISR() (QF_tickXISR((uint_fast8_t)0))

    /*! Legacy API defined for backwards compatibility */
    #define QF_tick()    (QF_tickXISR((uint_fast8_t)0))
#endif

#endif /* QP_API_VERSION < 450 */
#endif /* QP_API_VERSION < 500 */
#endif /* QP_API_VERSION < 540 */
/****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* qpn_h */
