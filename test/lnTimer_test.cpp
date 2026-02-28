//
// updated by ...: Loreto Notarantonio
// Date .........: 27-02-2026 15.44.11
//


// --- Project
// #define LOG_MODULE_LEVEL LOG_MODULE_INFO
#define  __I_AM_MAIN_CPP__
#include "WiFiManager.h"
#include "lnLogger_Class.h"

// --- wifi CREDENTIALS
#include <esp32_ssid_credentials.h>


#include "lnTimeClock.h"
#include "lnTimeScheduler.h"
WiFiManagerNB wifiManager;
lnTimeClock  ln_clock;
lnTimeScheduler scheduler(&ln_clock);


void wifiInit() {
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

    scheduler.onMinute(onMinuteCB);
    scheduler.at(12,0,0,onNoon);
    scheduler.everySeconds(10, [](){ // senza callback
        lnLOG_INFO("Ogni 10 secondi");
    });
}

void loop() {
    ln_clock.update();
    scheduler.update();
    wifiManager.update();
}