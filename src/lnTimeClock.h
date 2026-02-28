//
// updated by ...: Loreto Notarantonio
// Date .........: 27-02-2026 15.03.21
//

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

class lnTimeClock {
    private:
        const char* m_ntpServer = "pool.ntp.org";

        bool m_ntpStarted = false;
        bool m_timeValid  = false;

        unsigned long m_lastCheck = 0;
        unsigned long m_lastSync  = 0;

        const unsigned long m_syncInterval = 4UL * 60UL * 60UL * 1000UL; // 4 ore

        void startNTP();

    public:
        void begin(const char* ntpServer = "pool.ntp.org");
        void update();

        bool isTimeValid() const { return m_timeValid; }

        time_t now();
        void getLocalTime(struct tm &info);
};