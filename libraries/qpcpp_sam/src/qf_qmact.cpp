/// @file
/// @brief QMActive::QMActive() and virtual functions
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

#define QP_IMPL           // this is QP implementation
#include "qf_port.hpp"    // QF port
#include "qassert.h"        // QP embedded systems-friendly assertions

//! Internal macro to cast a QP::QMActive pointer @p qact_ to QP::QMsm*
/// @note
/// Casting pointer to pointer pointer violates the MISRA-C++ 2008 Rule 5-2-7,
/// cast from pointer to pointer. Additionally this cast violates the MISRA-
/// C++ 2008 Rule 5-2-8 Unusual pointer cast (incompatible indirect types).
/// Encapsulating these violations in a macro allows to selectively suppress
/// this specific deviation.
#define QF_QMACTIVE_TO_QMSM_CAST_(qact_) \
    reinterpret_cast<QMsm *>((qact_))

//! Internal macro to cast a QP::QMActive pointer @p qact_ to QP::QMsm const *
#define QF_QMACTIVE_TO_QMSM_CONST_CAST_(qact_) \
    reinterpret_cast<QMsm const *>((qact_))

namespace QP {

Q_DEFINE_THIS_MODULE("qf_qmact")

//............................................................................
QMActive::QMActive(QStateHandler const initial)
  : QActive(initial)
{
    m_temp.fun  = initial;
}

//............................................................................
void QMActive::init(void const * const e) {
    m_state.obj = &QMsm::msm_top_s;
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::init(e);
}
//............................................................................
void QMActive::init(void) {
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::init();
}
//............................................................................
void QMActive::dispatch(QEvt const * const e) {
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::dispatch(e);
}
//............................................................................
bool QMActive::isInState(QMState const * const st) const {
    return QF_QMACTIVE_TO_QMSM_CONST_CAST_(this)->QMsm::isInState(st);
}
//............................................................................
QMState const *QMActive::childStateObj(QMState const * const parent) const {
    return QF_QMACTIVE_TO_QMSM_CONST_CAST_(this)->QMsm::childStateObj(parent);
}


//****************************************************************************
// operations inherited from QP::QHsm, but disallowed in subclasses
// of QP::QMActive...

//............................................................................
bool QMActive::isIn(QStateHandler const s) {
    (void)s; // unused parameter

    /// @pre isIn() is disallowed in subclasses of QMActive.
    /// Use QP::QMActive::isInState() instead.
    Q_ERROR_ID(700);
    return false;
}
//............................................................................
QStateHandler QMActive::state(void) const {
    /// @pre state() is disallowed in subclasses of QMActive.
    /// Use QP::QMActive::stateObj() instead.
    Q_ERROR_ID(800);
    return static_cast<QStateHandler>(0);
}
//............................................................................
QStateHandler QMActive::childState(QStateHandler const parent) {
    (void)parent; // unused parameter

    /// @pre childState() is disallowed in subclasses of QMActive.
    /// Use QP::QMActive::childStateObj() instead.
    Q_ERROR_ID(900);
    return static_cast<QStateHandler>(0);
}

} // namespace QP

