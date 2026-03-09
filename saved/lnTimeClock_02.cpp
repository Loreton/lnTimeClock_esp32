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
#include <lnLogger_Class.h>



void lnTimeClock::begin() {
    // Italia: CET (1h) / CEST (2h, da Marzo a Ottobre)
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
}


bool lnTimeClock::isTimeValid() const {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // tm_year sono gli anni dal 1900. 120 = anno 2020.
    return (timeinfo.tm_year > 120);
}

void lnTimeClock::startNTP() {
    if (WiFi.status() != WL_CONNECTED) return;

    lnLOG_INFO("Initializing SNTP...");
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    configTime(0, 0, m_ntpServer1, m_ntpServer2);

    m_ntpActive = true;
    m_lastNtpStart = millis();
}

void lnTimeClock::stopNTP() {
    if (m_ntpActive) {
        sntp_stop();
        m_ntpActive = false;
        lnLOG_WARNING("SNTP stopped.");
    }
}

void lnTimeClock::update() {
    if (WiFi.status() != WL_CONNECTED) {
        if (m_ntpActive) stopNTP();
        return;
    }

    if (!m_ntpActive) {
        startNTP();
        return;
    }

    uint32_t now = millis();
    bool valid = isTimeValid();

    // Gestione refresh e tentativi
    if (valid) {
        // Se il tempo è valido, facciamo un refresh solo ogni ora
        if (now - m_lastNtpStart > m_syncInterval) {
            lnLOG_INFO("Scheduled NTP refresh...");
            startNTP();
        }
    } else {
        // Se il tempo NON è valido e siamo in attesa da troppo (es. 60s)
        if (now - m_lastNtpStart > m_retryTimeout) {
            lnLOG_ERROR("NTP sync failed to validate time. Retrying...");
            stopNTP(); // Forza il riavvio al prossimo ciclo
        }
    }
}

const char* lnTimeClock::getSyncStatusStr() const {
    if (isTimeValid()) return "TIME_OK";

    sntp_sync_status_t s = sntp_get_sync_status();
    if (s == SNTP_SYNC_STATUS_IN_PROGRESS) return "SYNCING...";
    return "WAITING_VALID_TIME";
}

void lnTimeClock::getLocalTime(struct tm &info) {
    time_t t = time(nullptr);
    localtime_r(&t, &info);
}

void lnTimeClock::getNow(char* buffer, size_t buf_len) {
    struct tm ti;
    getLocalTime(ti);
    strftime(buffer, buf_len, "%H:%M:%S", &ti);
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
