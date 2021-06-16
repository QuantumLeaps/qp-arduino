//.$file${.::dpp_bsp.ino} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp_bsp.qm
// File:  ${.::dpp_bsp.ino}
//
// This code has been generated by QM 5.1.2 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::dpp_bsp.ino} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"  // QP-C++ framework
#include "dpp.hpp"    // DPP application
#include "bsp.hpp"    // Board Support Package

using namespace QP;

Q_DEFINE_THIS_FILE

//............................................................................
void setup() {
    QF::init(); // initialize the framework
    BSP::init(); // initialize the Board Support Package

    // init publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QF::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO];
    QF::poolInit(smlPoolSto,
                 sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start all active objects...

    // start Philos
    static QP::QEvt const *philoQueueSto[10][N_PHILO];
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start((uint_fast8_t)(n + 1U), // priority
            philoQueueSto[n], Q_DIM(philoQueueSto[n]),
            (void *)0, 0U);
    }
    // start Table
    static QP::QEvt const *tableQueueSto[N_PHILO];
    AO_Table->start((uint_fast8_t)(N_PHILO + 1U), // priority
        tableQueueSto, Q_DIM(tableQueueSto),
        (void *)0, 0U);
}

//............................................................................
void loop() {
    QF::run(); // run the QF/C++ framework
}
