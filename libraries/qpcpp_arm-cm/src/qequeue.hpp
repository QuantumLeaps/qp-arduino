//$file${include::qequeue.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${include::qequeue.hpp}
//
// This code has been generated by QM 6.1.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2024-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${include::qequeue.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef QEQUEUE_HPP_
#define QEQUEUE_HPP_

#ifndef QF_EQUEUE_CTR_SIZE
    #define QF_EQUEUE_CTR_SIZE 1U
#endif

namespace QP {

#if (QF_EQUEUE_CTR_SIZE == 1U)
    using QEQueueCtr = std::uint8_t;
#elif (QF_EQUEUE_CTR_SIZE == 2U)
    using QEQueueCtr = std::uint16_t;
#elif (QF_EQUEUE_CTR_SIZE == 4U)
    using QEQueueCtr = std::uint32_t;
#else
    #error "QF_EQUEUE_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

class QEvt; // forward declaration

} // namespace QP

//$declare${QF::QEQueue} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QEQueue} .............................................................
class QEQueue {
private:
    QEvt const * volatile m_frontEvt;
    QEvt const ** m_ring;
    QEQueueCtr m_end;
    QEQueueCtr volatile m_head;
    QEQueueCtr volatile m_tail;
    QEQueueCtr volatile m_nFree;
    QEQueueCtr m_nMin;

    // friends...
    friend class QActive;
    friend class QTicker;
    friend class QXMutex;
    friend class QXThread;

public:
    QEQueue() noexcept
      : m_frontEvt(nullptr),
        m_ring(nullptr),
        m_end(0U),
        m_head(0U),
        m_tail(0U),
        m_nFree(0U),
        m_nMin(0U)
    {}

#ifdef Q_XTOR
    ~QEQueue();
#endif // def Q_XTOR
    void init(
        QEvt const * qSto[],
        std::uint_fast16_t const qLen) noexcept;
    bool post(
        QEvt const * const e,
        std::uint_fast16_t const margin,
        std::uint_fast8_t const qsId) noexcept;
    void postLIFO(
        QEvt const * const e,
        std::uint_fast8_t const qsId) noexcept;
    QEvt const * get(std::uint_fast8_t const qsId) noexcept;
    QEQueueCtr getNFree() const noexcept {
        return m_nFree;
    }
    QEQueueCtr getNMin() const noexcept {
        return m_nMin;
    }
    bool isEmpty() const noexcept {
        return m_frontEvt == nullptr;
    }

private:
    QEQueue(QEQueue const & other) = delete;
    QEQueue & operator=(QEQueue const & other) = delete;
}; // class QEQueue

} // namespace QP
//$enddecl${QF::QEQueue} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif // QEQUEUE_HPP_
