// qp-arduino Teensy 4.1 port validation sketch
//
// Define QP_ARDUINO_TEST_PROBES to enable timing probes:
// pin 2 timer ISR, pin 3 active dispatch, pin 4 idle, pin 5 event receipt,
// pin 6 validation marker.

#include "qpcpp.hpp"
#include <TimerOne.h>

using namespace QP;

Q_DEFINE_THIS_FILE

namespace {

enum Signals {
    TIMEOUT_SIG = QP::Q_USER_SIG,
    VALIDATE_SIG,
    DYN_SIG,
    MAX_PUB_SIG
};

enum AppRecords {
    VALIDATION_STAT = QP::QS_USER
};

struct ValidationEvt : public QP::QEvt {
    std::uint32_t seq;
};

QP::QSpyId const l_timerId = { 0U };
QP::QSpyId const l_commandId = { 0U };

constexpr std::uint32_t TICKS_PER_SEC = 100U;
constexpr std::uint32_t TIMER1_CLCK_HZ = 1000000U;

constexpr std::uint8_t PROBE_TIMER = 2U;
constexpr std::uint8_t PROBE_DISPATCH = 3U;
constexpr std::uint8_t PROBE_IDLE = 4U;
constexpr std::uint8_t PROBE_EVENT = 5U;
constexpr std::uint8_t PROBE_MARKER = 6U;

#ifdef QP_ARDUINO_TEST_PROBES
inline void probeInit() {
    pinMode(PROBE_TIMER, OUTPUT);
    pinMode(PROBE_DISPATCH, OUTPUT);
    pinMode(PROBE_IDLE, OUTPUT);
    pinMode(PROBE_EVENT, OUTPUT);
    pinMode(PROBE_MARKER, OUTPUT);
}

inline void probeHigh(std::uint8_t pin) {
    digitalWriteFast(pin, HIGH);
}

inline void probeLow(std::uint8_t pin) {
    digitalWriteFast(pin, LOW);
}

inline void probePulse(std::uint8_t pin) {
    probeHigh(pin);
    probeLow(pin);
}
#else
inline void probeInit() {}
inline void probeHigh(std::uint8_t) {}
inline void probeLow(std::uint8_t) {}
inline void probePulse(std::uint8_t) {}
#endif

class Validator : public QP::QActive {
private:
    QP::QTimeEvt m_timeEvt;
    std::uint32_t m_ticks;
    std::uint32_t m_events;
    std::uint32_t m_dynamicEvents;

public:
    static Validator inst;

    Validator()
      : QActive(Q_STATE_CAST(&Validator::initial)),
        m_timeEvt(this, TIMEOUT_SIG, 0U),
        m_ticks(0U),
        m_events(0U),
        m_dynamicEvents(0U)
    {}

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(running);
};

Validator Validator::inst;
QP::QActive * const AO_Validator = &Validator::inst;

Q_STATE_DEF(Validator, initial) {
    static_cast<void>(e);

    subscribe(VALIDATE_SIG);

    QS_OBJ_DICTIONARY(&Validator::inst);
    QS_OBJ_DICTIONARY(&Validator::inst.m_timeEvt);
    QS_OBJ_DICTIONARY(&l_timerId);
    QS_OBJ_DICTIONARY(&l_commandId);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, nullptr);
    QS_SIG_DICTIONARY(VALIDATE_SIG, nullptr);
    QS_SIG_DICTIONARY(DYN_SIG, nullptr);
    QS_USR_DICTIONARY(VALIDATION_STAT);

    return tran(&running);
}

Q_STATE_DEF(Validator, running) {
    QP::QState status;

    probeHigh(PROBE_DISPATCH);
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(1U, 1U);
            probePulse(PROBE_MARKER);
            status = Q_RET_HANDLED;
            break;
        }
        case TIMEOUT_SIG: {
            ++m_ticks;
            static QP::QEvt const validateEvt(VALIDATE_SIG);
            QP::QActive::publish_(&validateEvt, this, getPrio());
            status = Q_RET_HANDLED;
            break;
        }
        case VALIDATE_SIG: {
            ++m_events;
            ValidationEvt * const evt = Q_NEW(ValidationEvt, DYN_SIG);
            evt->seq = m_events;
            POST(evt, this);
            probePulse(PROBE_EVENT);
            status = Q_RET_HANDLED;
            break;
        }
        case DYN_SIG: {
            ValidationEvt const * const evt = Q_EVT_CAST(ValidationEvt);
            Q_ASSERT(evt->seq <= m_events);
            ++m_dynamicEvents;
            if ((m_dynamicEvents % 100U) == 0U) {
                QS_BEGIN_ID(VALIDATION_STAT, getPrio())
                    QS_U32(0U, m_ticks);
                    QS_U32(0U, m_events);
                    QS_U32(0U, m_dynamicEvents);
                QS_END()
            }
            status = Q_RET_HANDLED;
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    probeLow(PROBE_DISPATCH);
    return status;
}

} // namespace

void T1_Handler() {
    probeHigh(PROBE_TIMER);
    QP::QTimeEvt::tick(0U, &l_timerId);
    QV_ARM_ERRATUM_838869();
    probeLow(PROBE_TIMER);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    probeInit();

    QF::init();
    QS_INIT(nullptr);

    static QSubscrList subscrSto[MAX_PUB_SIG];
    QActive::psInit(subscrSto, Q_DIM(subscrSto));

    static QF_MPOOL_EL(ValidationEvt) evtPoolSto[16];
    QF::poolInit(evtPoolSto, sizeof(evtPoolSto), sizeof(evtPoolSto[0]));

    static QEvt const *validatorQueueSto[16];
    AO_Validator->start(1U,
        validatorQueueSto, Q_DIM(validatorQueueSto),
        nullptr, 0U);
}

void loop() {
    QF::run();
}

void QF::onStartup() {
    Timer1.initialize(TIMER1_CLCK_HZ / TICKS_PER_SEC);
    Timer1.attachInterrupt(T1_Handler);
}

void QV::onIdle() {
    probeHigh(PROBE_IDLE);
    QF_INT_ENABLE();

    std::uint16_t len = Serial.availableForWrite();
    if (len > 0U) {
        std::uint8_t const * const buf = QS::getBlock(&len);
        if (buf != nullptr) {
            Serial.write(buf, len);
        }
    }

    len = Serial.available();
    while (len > 0U) {
        QP::QS::rxPut(Serial.read());
        --len;
    }
    QS::rxParse();
    probeLow(PROBE_IDLE);
}

extern "C" Q_NORETURN Q_onError(char const * const module, int_t const id) {
    static_cast<void>(module);
    static_cast<void>(id);

    noInterrupts();
    digitalWrite(LED_BUILTIN, HIGH);
    for (;;) {
    }
}

bool QP::QS::onStartup(void const * arg) {
    static_cast<void>(arg);

    static std::uint8_t qsTxBuf[2048];
    static std::uint8_t qsRxBuf[256];

    initBuf(qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    Serial.begin(115200);

    QS_GLB_FILTER(QP::QS_SM_RECORDS);
    QS_GLB_FILTER(QP::QS_AO_RECORDS);
    QS_GLB_FILTER(QP::QS_EQ_RECORDS);
    QS_GLB_FILTER(QP::QS_MP_RECORDS);
    QS_GLB_FILTER(QP::QS_TE_RECORDS);
    QS_GLB_FILTER(QP::QS_UA_RECORDS);

    return true;
}

void QP::QS::onCleanup() {
}

QP::QSTimeCtr QP::QS::onGetTime() {
    return micros();
}

void QP::QS::onFlush() {
    std::uint16_t len = 0xFFFFU;
    std::uint8_t const *buf = QS::getBlock(&len);
    while (buf != nullptr) {
        Serial.write(buf, len);
        len = 0xFFFFU;
        buf = QS::getBlock(&len);
    }
    Serial.flush();
}

void QP::QS::onReset() {
    noInterrupts();
    _reboot_Teensyduino_();
}

void QP::QS::onCommand(
    std::uint8_t cmdId,
    std::uint32_t param1,
    std::uint32_t param2,
    std::uint32_t param3)
{
    static_cast<void>(param2);
    static_cast<void>(param3);

    switch (cmdId) {
        case 1U: {
            static QEvt const validateEvt(VALIDATE_SIG);
            QActive::publish_(&validateEvt, &l_commandId, 0U);
            break;
        }
        case 2U: {
            ValidationEvt * const evt = Q_NEW(ValidationEvt, DYN_SIG);
            evt->seq = param1;
            AO_Validator->POST(evt, &l_commandId);
            break;
        }
        case 3U: {
            probePulse(PROBE_MARKER);
            break;
        }
        case 4U: {
            Q_ERROR();
            break;
        }
        default: {
            break;
        }
    }
}
