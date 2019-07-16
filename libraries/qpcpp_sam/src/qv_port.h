/// @file
/// @brief QV/C++ port to ARM Cortex-M, GNU-ARM toolset
/// @cond
///***************************************************************************
/// Last updated for version 6.0.3
/// Last updated on  2017-12-09
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qv_port_h
#define qv_port_h

#if (__ARM_ARCH == 6) // Cortex-M0/M0+/M1 ?, see NOTE02

    // macro to put the CPU to sleep inside QV_onIdle()
    #define QV_CPU_SLEEP() do { \
        __asm volatile ("wfi"); \
        QF_INT_ENABLE(); \
    } while (0)

#else // Cortex-M3/M4/M4F

    // macro to put the CPU to sleep inside QV_onIdle()
    #define QV_CPU_SLEEP() do { \
        QF_PRIMASK_DISABLE(); \
        QF_INT_ENABLE(); \
        __asm volatile ("wfi"); \
        QF_PRIMASK_ENABLE(); \
    } while (0)

    // initialization of the QV kernel for Cortex-M3/M4/M4F
    #define QV_INIT() QV_init()
    extern "C" void QV_init(void);

#endif

#include "qv.h" // QV platform-independent public interface

#endif // qv_port_h
