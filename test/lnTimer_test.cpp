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
    lnLog.init(128, 20);  // line_buffer_len, filename_buffer_len
    wifiInit();

    // supponiamo WiFi già gestito altrove
    ln_clock.begin();

}

void loop() {
    ln_clock.update();
    wifiManager.update();
    struct tm t;
    ln_clock.getLocalTime(t);
    lnLOG_INFO("t: %d", t.tm_sec);

    char buff[16];
    ln_clock.getNow(buff, sizeof(buff));
    lnLOG_INFO("time: %s", buff);

    char buff2[16];
    ln_clock.msecToHMS(buff2, sizeof(buff2), millis(), true, false);
    lnLOG_INFO("msetToHMS: %s", buff2);

}