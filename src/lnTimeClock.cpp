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

#include "lnTimeClock.h"

void lnTimeClock::begin(const char* ntpServer) {
    m_ntpServer = ntpServer;

    // Timezone Italia con DST automatico
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();
}

void lnTimeClock::startNTP() {
    configTime(0, 0, m_ntpServer);
    m_ntpStarted = true;
    m_lastSync   = millis();
}

void lnTimeClock::update() {
    if (WiFi.status() != WL_CONNECTED) {
        m_ntpStarted = false;
        m_timeValid  = false;
        return;
    }

    if (!m_ntpStarted) {
        startNTP();
        return;
    }

    // Verifica validità tempo
    if (!m_timeValid && millis() - m_lastCheck > 2000) {
        m_lastCheck = millis();
        if (time(nullptr) > 100000) {
            m_timeValid = true;
        }
    }

    // Re-sync automatico ogni xx ore
    if (millis() - m_lastSync > m_syncInterval) {
        startNTP();
    }
}

time_t lnTimeClock::now() {
    return time(nullptr);
}

void lnTimeClock::getLocalTime(struct tm &info) {
    time_t t = now();
    localtime_r(&t, &info);
}