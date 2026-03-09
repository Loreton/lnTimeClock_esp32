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

        bool     m_ntpActive        = false;
        uint32_t m_lastNtpStart     = 0;

        // Timeouts
        const uint32_t m_syncInterval = 3600000; // 1 ora (intervallo normale)
        const uint32_t m_retryTimeout = 60000;   // 60 sec (se fallisce o resta in attesa)

        void startNTP();
        void stopNTP();

    public:
        void begin();
        void update();

        // Ritorna true se l'anno è > 2020 (indica che NTP ha funzionato almeno una volta)
        bool isTimeValid() const;

        const char* getSyncStatusStr() const;

        void getLocalTime(struct tm &info);
        void getNow(char* buffer, size_t buf_len);
        const char* msecToHMS(char *buffer, uint8_t buffer_len, uint32_t millisec, bool withMilliSec=false, bool stripHours=false);
};