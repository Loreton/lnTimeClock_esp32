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

time_t lnTimeClock::_now() {
    return time(nullptr);
}

void lnTimeClock::getLocalTime(struct tm &info) {
    time_t t = _now();
    localtime_r(&t, &info);
}




void lnTimeClock::getNow(char* buffer, size_t buf_len) {
    time_t now = time(nullptr); // get now

    if (now < 100000) { // Se NTP non è ancora sincronizzato
        // fallback su millis()
        uint32_t s = millis() / 1000;
        uint8_t hh = (s / 3600) % 24;
        uint8_t mm = (s / 60) % 60;
        uint8_t ss = s % 60;

        snprintf(buffer, buf_len, "%02d:%02d:%02d", hh, mm, ss);
        return;
    }

    struct tm timeinfo; // crea struttura
    localtime_r(&now, &timeinfo); // convert
    strftime(buffer, buf_len, "%H:%M:%S", &timeinfo); // copy to buffer
}


// ################################################################
// Converte millisecondi in HH:MM:SS.ms
// ritorna il timestamp del giorno
//    addMilliSec = true: aggiunge .xxx alla fine della stringa
//    stripHeader = true: rimuove hour o minutes se == 0
// ################################################################

const char* lnTimeClock::msecToHMS(char *buffer, uint8_t buffer_len, uint32_t millisec, bool withMilliSec, bool stripHours) {

    uint16_t msec    = (millisec % 1000UL);
    uint32_t seconds = (millisec / 1000UL);

    uint8_t sec      = (seconds  % 60);
    uint8_t min      = (seconds / 60) % 60;
    uint8_t hour     = (seconds / 3600);

    if (withMilliSec) {
        snprintf(buffer, buffer_len, "%02d:%02d:%02d.%03lu", hour, min, sec, msec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
    }
    else {
        snprintf(buffer, buffer_len, "%02d:%02d:%02d", hour, min, sec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
    }

    if (stripHours && hour == 0)  {
        return buffer+3;
    }

    return buffer;
}
