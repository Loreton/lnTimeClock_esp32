//
// updated by ...: Loreto Notarantonio
// Date .........: 27-02-2026 15.44.11
//


// --- Project
// #define LOG_MODULE_LEVEL LOG_MODULE_INFO
#define  __I_AM_MAIN_CPP__
#include "WiFiManager.h"
#include "lnTimeClock.h"
#include "lnLogger_Class.h"



WiFiManagerNB wifiManager;
lnTimeClock  ln_clock;


void wifiInit() {
    // --- wifi CREDENTIALS
    #include <ssid_credentials_esp32.h>
    // - prima dell'init()
    for (int8_t i = 0; i < loretoNetworksCount; i++) {
        wifiManager.addSSID(loretoNetworks[i].ssid, loretoNetworks[i].password);
    }

    wifiManager.init(
        60,   // scan ogni 60s se connesso
        30,   // scan ogni 30s se non connesso
        5*60,  // timeout max 5 minuti (5*60)
        8        // rssi gap
    );
}


void onMinuteCB() {
    lnLOG_INFO("Nuovo minuto!");
}

void onNoon() {
    lnLOG_INFO("È mezzogiorno!");
}

void setup() {
    Serial.begin(115200);
    lnLog.init(128, 25);  // line_buffer_len, filename_buffer_len
    wifiInit();

    // supponiamo WiFi già gestito altrove
    ln_clock.begin();

}

void loop() {
    static struct tm t;
    static uint8_t last_sec = 0;
    static uint8_t last_min = 0;
    static bool new_sec     = false;
    static bool new_min     = false;
    static bool secModulo5  = false;
    static bool secModulo10 = false;
    static bool first_run   = true;

    ln_clock.getLocalTime(t);
    new_sec     = false;
    new_min     = false;
    secModulo5  = false;
    secModulo10 = false;


    if (t.tm_sec != last_sec) {
        last_sec = t.tm_sec;
        new_sec=true;
    }

    if (t.tm_min != last_min) {
        last_min = t.tm_min;
        new_min=true;
    }


    if (new_sec) {
        if (t.tm_sec%5  == 0) {secModulo5 = true; }
        if (t.tm_sec%10 == 0) {secModulo10 = true; }
    }


    ln_clock.update();
    wifiManager.update();

    // --- every 5 seconds
    if (secModulo5) {
        lnLOG_INFO("modulo 5 seconds");
    }


    // --- every minutes
    if (new_min) {
        char buff[16];
        ln_clock.getNow(buff, sizeof(buff));
        lnLOG_INFO("time: %s", buff);

    }

    if (secModulo10) {
        if (ln_clock.isTimeValid()) {
            char buff[32];
            ln_clock.getNow(buff, sizeof(buff));
            lnLOG_INFO("Tempo sincronizzato: %s", buff);
        } else {
            lnLOG_WARNING("In attesa di sincronizzazione NTP... (Stato: %s)", ln_clock.getSyncStatusStr());
        }
    }


    if (first_run) {
        first_run=false;
        char buff2[16];
        ln_clock.msecToHMS(buff2, sizeof(buff2), millis(), true, false);
        lnLOG_INFO("msetToHMS: %s", buff2);
    }


    delay(100);


}