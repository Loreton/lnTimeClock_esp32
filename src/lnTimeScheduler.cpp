//
// updated by ...: Loreto Notarantonio
// Date .........: 30-08-2025 10.58.13
//


/*
    Cosa fa ora questa classe
    ✔ Avvia NTP solo quando WiFi è connesso
    ✔ Si ri-sincronizza ogni 12 ore
    ✔ Se WiFi cade → invalida il tempo
    ✔ Gestisce automaticamente ora legale
    ✔ Non blocca mai il loop
*/
#include "lnTimeScheduler.h"

lnTimeScheduler::lnTimeScheduler(lnTimeClock* clock) {
    m_clock = clock;
}

uint32_t lnTimeScheduler::secondsToday(struct tm &t) {
    return t.tm_hour * 3600UL + t.tm_min  * 60UL + t.tm_sec;
}

void lnTimeScheduler::onSecond(TimeCallback cb) {
    if (m_eventCount >= MAX_TIME_EVENTS) return;
    m_events[m_eventCount++] = {EVT_SECOND, 0, cb, 0}; // popola la struttura TimeEvent_t
}

void lnTimeScheduler::onMinute(TimeCallback cb) {
    if (m_eventCount >= MAX_TIME_EVENTS) return;
    m_events[m_eventCount++] = {EVT_MINUTE, 0, cb, 0}; // popola la struttura TimeEvent_t
}

void lnTimeScheduler::onHour(TimeCallback cb) {
    if (m_eventCount >= MAX_TIME_EVENTS) return;
    m_events[m_eventCount++] = {EVT_HOUR, 0, cb, 0}; // popola la struttura TimeEvent_t
}

void lnTimeScheduler::at(uint8_t H, uint8_t M, uint8_t S, TimeCallback cb) {
    if (m_eventCount >= MAX_TIME_EVENTS) return;

    uint32_t sec = H*3600UL + M*60UL + S;
    m_events[m_eventCount++] = {EVT_AT_HMS, sec, cb, 0}; // popola la struttura TimeEvent_t
}

void lnTimeScheduler::everySeconds(uint32_t sec, TimeCallback cb) {
    if (m_eventCount >= MAX_TIME_EVENTS) return;
    m_events[m_eventCount++] = {EVT_EVERY_SEC, sec, cb, 0}; // popola la struttura TimeEvent_t
}



void lnTimeScheduler::update() {
    if (!m_clock->isTimeValid()) return;

    struct tm t;
    m_clock->getLocalTime(t);

    uint32_t nowSec = secondsToday(t);

    for (uint8_t i = 0; i < m_eventCount; i++) {
        TimeEvent_t &ev = m_events[i];

        switch (ev.type) {
            case EVT_SECOND:
                if (nowSec != ev.lastTrigger) {
                    ev.lastTrigger = nowSec;
                    ev.cb();
                }
                break;

            case EVT_MINUTE:
                if (t.tm_sec == 0 && nowSec != ev.lastTrigger) {
                    ev.lastTrigger = nowSec;
                    ev.cb();
                }
                break;

            case EVT_HOUR:
                if (t.tm_min == 0 && t.tm_sec == 0 && nowSec != ev.lastTrigger) {
                    ev.lastTrigger = nowSec;
                    ev.cb();
                }
                break;

            case EVT_AT_HMS:
                if (nowSec == ev.param && nowSec != ev.lastTrigger) {
                    ev.lastTrigger = nowSec;
                    ev.cb();
                }
                break;

            case EVT_EVERY_SEC:
                if ((nowSec % ev.param) == 0 && nowSec != ev.lastTrigger) {
                    ev.lastTrigger = nowSec;
                    ev.cb();
                }
                break;
        }
    }
}