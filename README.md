# lnTime_esp32
Wrapper per time e time-sched per ESP32

# ESP32 Time Scheduler & Clock Management

Questo progetto fornisce un sistema leggero e non bloccante per la gestione del tempo e la pianificazione di eventi (scheduler) su microcontrollori ESP32. Integra la sincronizzazione automatica via **NTP**, la gestione dell'**ora legale (DST)** e un sistema di **callback** per eseguire azioni a intervalli regolari o a orari specifici.

## 🚀 Caratteristiche

- **Sincronizzazione NTP**: Gestione automatica della connessione ai server NTP.
- **Resilienza WiFi**: Il sistema monitora lo stato della connessione WiFi; se cade, invalida il tempo e tenta la risincronizzazione al ripristino.
- **Timezone & DST**: Configurato di default per l'Italia (CET/CEST) con cambio automatico ora legale/solare.
- **Scheduler ad Eventi**: Supporta diversi tipi di trigger:
  - Ogni secondo, minuto o ora.
  - Ad un orario preciso (`HH:MM:SS`).
  - Ogni *n* secondi (intervalli regolari).
- **Non-blocking**: Progettato per girare nel `loop()` principale senza l'uso di `delay()`.

## 🛠 Struttura del Progetto

Il cuore del sistema è diviso in due classi principali:

1.  **`lnTimeClock`**: Gestisce l'interfaccia con il sistema di sistema dell'ESP32, la configurazione NTP e il mantenimento dell'ora.
2.  **`lnTimeScheduler`**: Riceve un riferimento a `lnTimeClock` e gestisce una lista di eventi (massimo 10 per impostazione predefinita) associati a funzioni di callback.

## 📋 Requisiti & Dipendenze

Il progetto dipende dalle seguenti librerie (gestite via PlatformIO):
- [lnLogger_esp32](https://github.com/Loreton/lnLogger_esp32.git): Per la gestione dei log.
- [wifiManager_esp32](https://github.com/Loreton/wifiManager_esp32.git): Per la gestione della connettività WiFi.

## ⚙️ Configurazione PlatformIO

Assicurati che il tuo `platformio.ini` includa i percorsi corretti per le librerie e i flag di compilazione.
**Nota**: Il progetto utilizza un file esterno per le credenziali WiFi (`esp32_ssid_credentials.h`).

```ini
[env:esp32]
platform = espressif32
framework = arduino
board = esp32dev
lib_deps =
    https://github.com/Loreton/lnLogger_esp32.git#v1.1.5
    https://github.com/Loreton/wifiManager_esp32.git#v1.0.1

build_flags =
    -I /percorso/ai/tuoi/header_privati
```

## 💻 Esempio d'uso

```cpp
#include "lnTimeClock.h"
#include "lnTimeScheduler.h"

lnTimeClock  ln_clock;
lnTimeScheduler scheduler(&ln_clock);

void onMinuteCB() {
    Serial.println("È scoccato un nuovo minuto!");
}

void setup() {
    Serial.begin(115200);

    // Inizializza l'orologio (Timezone Italia di default)
    ln_clock.begin();

    // Registra gli eventi
    scheduler.onMinute(onMinuteCB);

    scheduler.at(12, 0, 0, [](){
        Serial.println("Mezzogiorno di fuoco!");
    });

    scheduler.everySeconds(30, [](){
        Serial.println("Trigger ogni 30 secondi");
    });
}

void loop() {
    // Necessari per il funzionamento asincrono
    ln_clock.update();
    scheduler.update();

    // ... altro codice non bloccante ...
}
```

## 📖 API Reference

### `lnTimeClock`
- `begin(const char* ntpServer)`: Inizializza l'ambiente temporale e imposta la TZ.
- `update()`: Gestisce la macchina a stati (connessione WiFi -> NTP start -> Sync).
- `isTimeValid()`: Restituisce `true` se l'ora è stata sincronizzata correttamente.

### `lnTimeScheduler`
- `onSecond(cb)`, `onMinute(cb)`, `onHour(cb)`: Esegue la callback allo scoccare dell'unità di tempo.
- `at(H, M, S, cb)`: Esegue la callback a un orario preciso ogni giorno.
- `everySeconds(n, cb)`: Esegue la callback ogni `n` secondi.
- `update()`: Controlla se i trigger sono attivi (da chiamare nel loop).

## ⚠️ Note Importanti
- **Limite Eventi**: Il numero massimo di eventi registrabili è definito da `MAX_TIME_EVENTS` (default: 10) in `lnTimeScheduler.h`.
- **Timezone**: Attualmente hardcoded in `lnTimeClock.cpp` per l'Italia: `CET-1CEST,M3.5.0/2,M10.5.0/3`.

---
**Autore:** Loreto Notarantonio
**Data Ultimo Aggiornamento:** 27-02-2026