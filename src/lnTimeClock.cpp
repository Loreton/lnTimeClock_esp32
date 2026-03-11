//
// updated by ...: Loreto Notarantonio
// Date .........: 11-03-2026 17.06.48
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


bool lnTimeClock::isTimeValid() const {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // tm_year sono gli anni dal 1900. 120 = anno 2020.
    return (timeinfo.tm_year > 120);
}

// void lnTimeClock::startNTP() {
//     if (WiFi.status() != WL_CONNECTED) return;

//     lnLOG_INFO("Initializing NTP connection...");
//     sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
//     sntp_set_time_sync_notification_cb(lnTimeClock::sntpCallback);

//     // Configurazione nativa: gestisce fino a 3 server
//     configTime(0, 0, m_ntpServer1, m_ntpServer2);

//     m_ntpActive = true;
//     m_lastNtpStart = millis();
// }

void lnTimeClock::startNTP() {
    // Non controlliamo più il WiFi qui, ci fidiamo di chi ci chiama
    lnLOG_INFO("Initializing NTP connection...");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_set_time_sync_notification_cb(lnTimeClock::sntpCallback);

    configTime(0, 0, m_ntpServer1, m_ntpServer2);
    m_ntpActive = true;
    m_lastNtpStart = millis();
}




// void lnTimeClock::stopNTP() {
//     if (m_ntpActive) {
//         sntp_stop();
//         m_ntpActive = false;
//         lnLOG_WARNING("SNTP stopped.");
//     }
// }

void lnTimeClock::stopNTP() {
    if (m_ntpActive) {
        sntp_stop();
        m_ntpActive = false;
        lnLOG_WARNING("SNTP stopped (Network unavailable or forced).");
    }
}
/*
void lnTimeClock::update() {
    if (WiFi.status() != WL_CONNECTED) {
        if (m_ntpActive) {
	       stopNTP();
	       lnLOG_WARNING("WiFi lost: SNTP stopping...");
	    }
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
}*/


void lnTimeClock::update(bool isNetworkAvailable) {
    // 1. GESTIONE CADUTA RETE
    if (!isNetworkAvailable) {
        if (m_ntpActive) {
            stopNTP(); // Mettiamo in pausa NTP se la rete sparisce
        }
        return; // Non facciamo altro finché non torna la rete
    }

    // 2. RETE DISPONIBILE: Se non siamo attivi, partiamo
    if (!m_ntpActive) {
        startNTP();
        return;
    }

    // 3. LOGICA DI REFRESH (Rete presente e NTP attivo)
    uint32_t now = millis();
    bool valid = isTimeValid();

    if (valid) {
        // Refresh programmato (es. ogni 12 o 24 ore)
        if (now - m_lastNtpStart > m_syncInterval) {
            lnLOG_INFO("Scheduled NTP refresh...");
            startNTP();
        }
    } else {
        // Se non è ancora valido dopo il timeout (es. DNS fallito o server down)
        if (now - m_lastNtpStart > m_retryTimeout) {
            lnLOG_ERROR("NTP sync timeout. Retrying...");
            stopNTP(); // Reset per riprovare al prossimo update
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
    time_t now = time(nullptr);
    localtime_r(&now, &info);
}

void lnTimeClock::getNow(char* buffer, size_t buf_len) {
    struct tm timeinfo;
    getLocalTime(timeinfo);
    strftime(buffer, buf_len, "%H:%M:%S", &timeinfo);
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
