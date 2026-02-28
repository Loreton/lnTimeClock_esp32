//
// updated by ...: Loreto Notarantonio
// Date .........: 27-02-2026 15.03.21
//

#pragma once
#include <Arduino.h>
#include "lnTimeClock.h"

#define MAX_TIME_EVENTS 10

typedef void (*TimeCallback)();

class lnTimeScheduler {
    private:
        lnTimeClock* m_clock;

        enum EventType {
            EVT_SECOND,
            EVT_MINUTE,
            EVT_HOUR,
            EVT_AT_HMS,
            EVT_EVERY_SEC
        };

        struct TimeEvent_t {
            EventType type;
            uint32_t  param;
            TimeCallback cb;
            uint32_t  lastTrigger;
        };

        TimeEvent_t m_events[MAX_TIME_EVENTS];
        uint8_t   m_eventCount = 0;

        uint32_t secondsToday(struct tm &t);

    public:
        lnTimeScheduler(lnTimeClock* clock);

        void update();

        void onSecond(TimeCallback cb);
        void onMinute(TimeCallback cb);
        void onHour(TimeCallback cb);
        void at(uint8_t H, uint8_t M, uint8_t S, TimeCallback cb);
        void everySeconds(uint32_t sec, TimeCallback cb);
};