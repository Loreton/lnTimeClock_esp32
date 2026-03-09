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

// Status sync come stringhe per il log
#define EUROPE_ROME_TZ "CET-1CEST,M3.5.0,M10.5.0/3" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* sntp_status_names[] = {"RESET", "COMPLETED", "IN_PROGRESS"};
volatile uint8_t g_ntpSyncStatus = 0; // devo far uso di una variabile globale per catturare lo status di cbSyncTime()


void lnTimeClock::sntpCallback(struct timeval *tv) {
    g_ntpSyncStatus = sntp_get_sync_status();
    lnLOG_NOTIFY("NTP time synched: %d [%s]", g_ntpSyncStatus, sntp_status_names[g_ntpSyncStatus]);
}

void lnTimeClock::begin() {
    // Imposta il fuso orario Italiano (Roma)
    setenv("TZ", EUROPE_ROME_TZ, 1);
    tzset();
}

void lnTimeClock::startNTP() {
    if (WiFi.status() != WL_CONNECTED) return;

    lnLOG_INFO("Initializing NTP connection...");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_set_time_sync_notification_cb(lnTimeClock::sntpCallback);

    // Configurazione nativa: gestisce fino a 3 server
    configTime(0, 0, m_ntpServer1, m_ntpServer2);

    m_ntpActive = true;
    m_lastAttempt = millis();
}

void lnTimeClock::update() {
    // Gestione connettività
    if (WiFi.status() != WL_CONNECTED) {
        if (m_ntpActive) {
            sntp_stop();
            m_ntpActive = false;
            m_timeValid = false;
            lnLOG_WARNING("WiFi lost: SNTP stopped");
        }
        return;
    }

    if (!m_ntpActive) {
        startNTP();
        return;
    }

    // Monitoraggio stato tramite API ESP-IDF
    sntp_sync_status_t status = sntp_get_sync_status();

    if (status != m_lastSyncStatus) {
        lnLOG_INFO("SNTP Status changed: %s", sntp_status_names[status]);
        m_lastSyncStatus = status;

        if (status == SNTP_SYNC_STATUS_COMPLETED) {
            m_timeValid = true;
            m_lastAttempt = millis(); // Reset timer per il prossimo intervallo
        }
    }

    // Re-sync periodico o gestione fallimento iniziale (dopo 30s)
    if (millis() - m_lastAttempt > m_syncInterval || (!m_timeValid && millis() - m_lastAttempt > 30000)) {
        lnLOG_INFO("Refreshing NTP sync...");
        startNTP();
    }
}

void lnTimeClock::getLocalTime(struct tm &info) {
    time_t now = time(nullptr);
    localtime_r(&now, &info);
}

void lnTimeClock::getNow(char* buffer, size_t buf_len) {
    struct tm timeinfo;
    getLocalTime(timeinfo);
    strftime(buffer, buf_len, "%H:%M:%S", &timeinfo);
}



bool lnTimeClock::isSynced() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Se l'anno è maggiore del 1970 (tm_year è anni dal 1900),
    // significa che NTP ha risposto almeno una volta con successo.
    return (timeinfo.tm_year > 70);
}

const char* lnTimeClock::getSyncStatus() {
    if (isSynced()) {
        return "SYNCED (Time Valid)";
    }

    switch (sntp_get_sync_status()) {
        case SNTP_SYNC_STATUS_IN_PROGRESS: return "SYNCING...";
        case SNTP_SYNC_STATUS_RESET:       return "IDLE (Time Not Set)";
        default:                           return "UNKNOWN";
    }
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
