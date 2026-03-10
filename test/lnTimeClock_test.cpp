//
// updated by ...: Loreto Notarantonio
// Date .........: 27-02-2026 15.44.11
//


// --- Project
// #define LOG_MODULE_LEVEL LOG_MODULE_INFO
#define  __I_AM_MAIN_CPP__
#include "lnLogger_Class.h"
#include "lnWiFiManager.h"
#include "lnTimeClock.h"


// Variabili di stato
bool canUseNetwork = false;
uint32_t lastRetryTime = 0;
const uint32_t retryInterval = 30000; // 30 secondi tra i tentativi di scansione se disconnesso

// istanze
lnWiFiManagerNB wifiManager;
lnTimeClock  ln_clock;


// --- WIFI-CALLBACK: Qui gestiamo gli eventi di rete
void onConnectionChanged(bool connected) {
    canUseNetwork = connected;

    if (connected) {
        lnLOG_NOTIFY("SISTEMA: Rete ripristinata. Avvio servizi...");
        // Qui puoi chiamare funzioni "una tantum" al momento della connessione:
        // configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
        // myTelegramBot.sendMessage(CHAT_ID, "Sistema Online!", "");
    } else {
        lnLOG_ERROR("SISTEMA: Connessione persa. Servizi in pausa.");
    }
}

void wifiInit() {
    // --- wifi CREDENTIALS
    #include <ssid_credentials_esp32.h>
    // - prima dell'init()
    for (int8_t i = 0; i < loretoNetworksCount; i++) {
        wifiManager.addSSID(loretoNetworks[i].ssid, loretoNetworks[i].password);
    }

    wifiManager.setConnectionCallback(onConnectionChanged);
    wifiManager.init(8); // rssiGap di 8dB

    // 2. Lanciamo la prima scansione manuale
    wifiManager.startScan();
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


uint8_t last_sec = 0;
uint8_t last_min = 0;
bool new_sec     = false;
bool new_min     = false;
bool secModulo5  = false;
bool secModulo10 = false;
bool first_run   = true;

void refreshTime() {
    static struct tm t;
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
}



void loop() {
    refreshTime();

    // Aggiorna lo stato del WiFi (gestisce i risultati dello scan)
    wifiManager.update();

    // --- LOGICA DEI SERVIZI ---
    if (canUseNetwork) {
        ln_clock.update();

        // Esegui Telegram solo se la rete è pronta
        // myTelegramBot.handleMessages();

        // Esegui Logica NTP ogni ora
        // if (now - lastNtpUpdate > 3600000) { ... }

    } else {

        // --- LOGICA DI RICONNESSIONE MANUALE ---
        // Se non siamo connessi, riproviamo a scansionare ogni 30s
        uint32_t now = millis();
        if (now - lastRetryTime > retryInterval) {
            lnLOG_NOTIFY("SISTEMA: Tentativo di riconnessione manuale...");
            wifiManager.startScan();
            lastRetryTime = now;
        }
    }


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