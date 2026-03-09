//
// updated by ...: Loreto Notarantonio
// Date .........: 27-02-2026 15.03.21
//

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>

class lnTimeClock {
    private:
        const char* m_ntpServer1 = "pool.ntp.org";
        const char* m_ntpServer2 = "time.google.com";

        bool     m_ntpActive    = false;
        bool     m_timeValid     = false;
        uint8_t  m_lastSyncStatus = SNTP_SYNC_STATUS_RESET;
        uint32_t m_lastAttempt   = 0;

        // Intervallo di sync forzato (es. ogni 1 ora)
        const uint32_t m_syncInterval = 60 * 60 * 1000UL;

        void startNTP();
        static void sntpCallback(struct timeval *tv);

    public:
        void begin();
        void update();

        bool isTimeValid() const { return m_timeValid; }
        void getLocalTime(struct tm &info);
        void getNow(char* buffer, size_t buf_len);
        const char* msecToHMS(char *buffer, uint8_t buffer_len, uint32_t millisec, bool withMilliSec=false, bool stripHours=false);

        bool isSynced(void);

        // Ritorna una stringa leggibile dello stato attuale
        const char* getSyncStatus(void);
};