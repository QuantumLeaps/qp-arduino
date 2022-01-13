#include "qpcpp.hpp"

Q_DEFINE_THIS_FILE

#ifdef Q_SPY

void QP::QS::onCleanup(void) 
{

}

QP::QSTimeCtr QP::QS::onGetTime(void) 
{   
    return millis();
}

void QP::QS::onFlush(void)
{
    Serial.flush();
}

void QP::QS::onReset(void)
{
    NVIC_SystemReset();
}

#endif // Q_SPY
