/// @file
/// @brief QP/C++ public interface including backwards-compatibility layer
/// @ingroup qep qf qv qk qxk qs
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-03-03
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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

#ifndef qpcpp_h
#define qpcpp_h

/// @description
/// This header file must be included directly or indirectly
/// in all application modules (*.cpp files) that use QP/C++.

//****************************************************************************
#include "qf_port.hpp"      // QF/C++ port from the port directory
#include "qassert.h"        // QP assertions
#ifdef Q_SPY                // software tracing enabled?
    #include "qs_port.hpp"  // QS/C++ port from the port directory
#else
    #include "qs_dummy.hpp" // QS/C++ dummy (inactive) interface
#endif

//****************************************************************************
#ifndef QP_API_VERSION

//! Macro that specifies the backwards compatibility with the
//! QP/C++ API version.
/// @description
/// For example, QP_API_VERSION=540 will cause generating the compatibility
/// layer with QP/C++ version 5.4.0 and newer, but not older than 5.4.0.
/// QP_API_VERSION=0 causes generation of the compatibility layer "from the
/// begining of time", which is the maximum backwards compatibilty. This is
/// the default.@n
/// @n
/// Conversely, QP_API_VERSION=9999 means that no compatibility layer should
/// be generated. This setting is useful for checking if an application
/// complies with the latest QP/C++ API.
#define QP_API_VERSION 0

#endif  // QP_API_VERSION

// QP/C++ API compatibility layer...

//****************************************************************************
#if (QP_API_VERSION < 680)

//! @deprecated
//! Macro to specify a transition in the "me->" impl-strategy.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// you call tran(Q_STATE_CAST(target_)).
#define Q_TRAN(target_)       (me->tran(Q_STATE_CAST(target_)))

//! @deprecated
//! Macro to specify a tran-to-history in the "me->" impl-strategy.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// you call tran_hist(Q_STATE_CAST(hist_)).
#define Q_TRAN_HIST(hist_)    (me->tran_hist((hist_)))

//! @deprecated
//! Macro to specify the superstate in the "me->" impl-strategy.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// you call super(state_)).
#define Q_SUPER(state_)       (me->super((state_)))

//! @deprecated
//! Macro to call in a QM state entry-handler. Applicable only to QMSMs.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_entry(Q_STATE_CAST(state_)).
#define QM_ENTRY(state_)      (me->qm_entry((state_)))

//! @deprecated
//! Macro to call in a QM state exit-handler. Applicable only to QMSMs.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_exit(Q_STATE_CAST(state_)).
#define QM_EXIT(state_)       (me->qm_exit((state_)))

//! @deprecated
//! Macro to call in a QM submachine exit-handler. Applicable only to QMSMs.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_sm_exit(Q_STATE_CAST(state_)).
#define QM_SM_EXIT(state_)    (me->qm_sm_exit((state_)))

//! @deprecated
//! Macro to call in a QM state-handler when it executes a transition.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_tran((tatbl_)).
#define QM_TRAN(tatbl_)       (me->qm_tran((tatbl_)))

//! @deprecated
//! Macro to call in a QM state-handler when it executes an initial tran.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_tran_init((tatbl_)).
#define QM_TRAN_INIT(tatbl_)  (me->qm_tran_init((tatbl_)))

//! @deprecated
//! Macro to call in a QM state-handler when it executes a tran-to-history.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_tran_hist((history_), (tatbl_)).
#define QM_TRAN_HIST(history_, tatbl_) \
    (me->qm_tran_hist((history_), (tatbl_)))

//! @deprecated
//! Macro to call in a QM state-handler when it executes an initial tran.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_tran_ep((tatbl_)).
#define QM_TRAN_EP(tatbl_)    (me->qm_tran_ep((tatbl_)))

//! @deprecated
//! Macro to call in a QM state-handler when it executes a tran-to-exit-point.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_tran_xp((xp_), (tatbl_)).
#define QM_TRAN_XP(xp_, tatbl_) (me->qm_tran_xp((xp_), (tatbl_)))

//! @deprecated
//! Designates the superstate of a given state in a subclass of QP::QMsm.
/// Instead use the new impl-strategy without the "me->" pointer, where
/// the QM-generated code calls qm_super_sub((state_)).
#define QM_SUPER_SUB(state_)  (me->qm_super_sub((state_)))

#endif // QP_API_VERSION < 680

#endif // qpcpp_h

