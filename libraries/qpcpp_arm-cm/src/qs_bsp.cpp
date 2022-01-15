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
    uint16_t len = 0xFFFFU; // big number to get as many bytes as available
    uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
    while (buf != nullptr) { // data available?
        Serial.write(buf, len); // might poll until all bytes fit
        len = 0xFFFFU; // big number to get as many bytes as available
        buf = QS::getBlock(&len); // try to get more data
    }
    Serial.flush(); // wait for the transmission of outgoing data to complete
}

void QP::QS::onReset(void)
{
    NVIC_SystemReset();
}

#endif // Q_SPY
