# KONTEKST: Mustang Radio – ESP32 (HMI + Bluetooth) ↔ STM32 (radio, menu)

> **Dla Claude:** przeczytaj cały plik, zanim cokolwiek zrobisz. To przypomnienie: co jest zrobione, co do zrobienia, jakie są zasady pracy.
> Numery linii są orientacyjne – przed każdą zmianą przeczytaj aktualny plik.
> Stan na: **2026-11-22**. Poprzedni kontekst (`KONTEKST_I2C_ESP32_STM32.md`, szczegóły I2C, analizy, testy) jest w historii gita:
> `git show 887b0444:KONTEKST_I2C_ESP32_STM32.md`

---

## 0. Zasady pracy z użytkownikiem (OBOWIĄZKOWE)

1. **Nigdy nie zmieniaj kodu z własnej inicjatywy.** Kolejność: ustalenia (tylko otwarte kwestie) → plan → **diff w czacie (ścieżka + numery linii)** → wyraźna akceptacja → dopiero wtedy edycja.
2. **Krok po kroku:** jedna logiczna zmiana = jeden krok = osobna akceptacja.
3. **Akceptacja musi być jednoznaczna** („akceptuję”, „tak, wprowadź krok X”). „Poprawmy to”, „dodaj X”, „robimy i tak” = dopisanie do planu, **NIE akceptacja diffa** – dopytaj.
4. **Nie usuwaj kodu bez pytania** (także „duplikatów”). Zakomentowany kod użytkownika zostaje.
5. Problemy zauważone przy okazji – **zgłaszaj, nie poprawiaj**.
6. Reużywaj istniejący kod; styl jak w otoczeniu (taby, komentarze po polsku).
7. Asercje: **operacja zawsze poza makrem, w `assert`/`configASSERT` tylko sprawdzenie wyniku** (wzorzec w obu projektach, patrz 3.3).
8. Spójność STM32 ↔ ESP32: enumy w ramkach I2C jako `typedef enum : uint8_t` we wspólnym pliku.
9. Podczas debugowania na żywo pytaj zwykłym tekstem (nie narzędziem wielokrotnego wyboru).
10. Commit / push – **tylko na prośbę użytkownika**.

---

## 1. Środowisko

| | |
|---|---|
| ESP32 | `02_Firmware/01_ESP32/HMI_and_bluetooth` – ESP-IDF `release-v5.2` (`C:/esp/release-v5.2/esp-idf`), Espressif-IDE, biblioteka **ESP32-A2DP** (pschatzmann) + arduino-audio-tools |
| STM32 | `02_Firmware/02_STM32/Radio_main` – STM32H7A3RITx, STM32CubeIDE 1.15.1, HAL, FreeRTOS 10.3.1 |
| Pliki wspólne | `HMI_and_bluetooth/main/common/` (`comunicationProtocol.h`, `comunicationStructures.h`, `i2c_slave_master_queueClass`, checksum) – kompilowane w obu projektach |
| Debug ESP32 | J-Link CE + OpenOCD. Działa konfiguracja z **ręcznie wpisanym gdb 12.1**; `${GDB_CLIENT_EXECUTABLE}` (gdb 16.3) wywala „Process not Terminated” (timeout 10 s wtyczki Espressif) |
| F3 w Espressif-IDE | wymaga `HMI_and_bluetooth/build/compile_commands.json` (znika po Clean → Build) i edytora „C/C++ Editor (LSP)” |
| Sprawdzanie kompilacji bez IDE | ESP32: `clangd --check=<plik>` z `--compile-commands-dir=build`; STM32: `make <obj>` w `Radio_main/Debug` z `make` i `arm-none-eabi` z pluginów STM32CubeIDE 1.15.1 |

---

## 2. Bluetooth – ZROBIONE (2026-11-21/22, ESP32, tylko kompilacja – NIEtestowane na sprzęcie)

Plik: `HMI_and_bluetooth/main/bt_audio/bt_audio.{h,cpp}`, klasa `bt_audio_sink`.

- **`btConnectionState`** (prywatna struktura) – cały stan BT: `btAudioDeviceOn_Off`, `highZstateState` (4 stany pinów I2S), `onConnCallbackStates`, `onAVRCPlayStatusCallbackStates`, `audioState`, `sampleRate`, `peerName`, `peerAddress`, `avrcConnected`, `volume`, `playPosMs`, `title/artist/album`.
- **Callbacki** biblioteki = prywatne metody statyczne (dostęp przez `static bt_audio_sink* instance`). Callback **tylko zapisuje do struktury i woła metodę wysyłającą**.
- **Publiczne metody wysyłające ze struktury** (gotowe pod żądania STM32): `sendToMasterDeviceOnOffState()`, `sendToMasterConnectionState()`, `sendToMasterAVRCPlayStatus()`, `sendToMasterI2sPinsState()`. Ramki zerowane (`= {}`).
- **`setI2sPinsState()`** – jedyne miejsce zmiany stanu pinów (i2sState + struktura + wysyłka).
- **On/Off:** sink i I2S tworzone RAZ (pierwsze On), potem `start()` / `end(false)`; `delete` tylko w destruktorze (biblioteka trzyma globalne wskaźniki na sink). Guardy podwójnego On/Off (`btAudioDeviceOn_Off`).
- **Off:** `end(false)` → `esp_bt_gap_set_scan_mode(NON_CONNECTABLE, NON_DISCOVERABLE)` (radio niewidoczne) → do STM32 brakujące DISCONNECTED / STOPPED → zerowanie danych sesji (peerAddress/peerName zostają) → `i2sNotConfigured` → **high-Z** → `highZenabled` → **OFF** (high-Z ZAWSZE przed OFF – wspólna szyna I2S do DAC).
- **On:** high-Z off → `i2sNotConfigured` → `begin(cfg)` → `i2sConfigured` → `start()` → ON → migawka (połączenie, odtwarzanie, piny).
- **Konstruktor:** high-Z jako PIERWSZE (wymóg sprzętowy), potem `btAudioDeviceOff()` (STM32 dostaje OFF).
- **Auto-reconnect:** tylko podczas On. `BluetoothA2DPSinkExt` (klasa pochodna) przywraca adres ostatniego telefonu przed `start()` (bo `end()` czyści go w NVS). `start()` czeka 1000 ms (`reconnect_delay`).
- **Guardy komend AVRC** (`btAudioPlay/Stop/Pause/Next/Previous/FastForward/Rewind`) – nic nie robią przy BT wyłączonym.
- **Protokół:** wspólny `i2sPinStates : uint8_t` + `static_assert` w `comunicationStructures.h`; odkomentowany `BT_SUBCOMMAND_I2S_PINS_STATE 0x04`. Subcommandy BT: `deviceOnOff 0x00`, `onConnCallback 0x01`, `onAVRCPlayStatusCallback 0x03`, `I2S_PINS_STATE 0x04`.
- Profil rozmów (HFP) wyłączony w `sdkconfig` – radio nie obsługuje połączeń telefonicznych. GAP musi zostać (bez niego nie ma A2DP).

### 3.3 Asercje (2026-11-21, oba projekty) – ZROBIONE
Wszystkie `assert(x = new …)`, `configASSERT(xTaskCreate(…))`, `assert(!esp_fun(…))` przepisane na: operacja → `assert(wynik)` / `== pdPASS` / `err == ESP_OK`. Uwaga: w ESP-IDF `assert` przy NDEBUG i tak wykonuje wyrażenie; na STM32 (newlib) – NIE, stąd zmiana.

---

## 4. DO ZROBIENIA – Bluetooth

### 4.1 Test na sprzęcie (NAJPIERW)
- On → telefon łączy się → Off: telefon traci radio z listy wyszukiwania, STM32 dostaje DISCONNECTED/STOPPED/piny/OFF.
- On ponownie: telefon łączy się sam (auto-reconnect po Off→On).
- Podwójne On / Off, rozłączenie z telefonu, wyłączenie telefonu.
- Log ramek I2C po stronie ESP32 i kolejność high-Z → OFF.

### 4.2 ESP32 – task / obsługa komend od STM32 (połączenie, granie)
- **Brakuje całej ścieżki odbioru ramek BT na ESP32** (parser → kolejka → task BT):
  - parser `i2cReceivedDataParser` (`tasksFunctions.cpp` ~633) ma `case` dla KEYBOARD/NVS/LEDS/STEPPER – **brak `I2C_COMMAND_GROUP_BTAUDIO`**;
  - klasa `bt_audio_sink` **nie ma kolejki** na ramki od parsera ani taska, który by je obsługiwał.
  - Wzorzec do powielenia (już działa): LED – `Leds_backlight.{h,cpp}`: `i2cQueue4DynamicData* ParserDataToLedsDataQueue` + `QueueSendDataToLedTask()` / `QueueReceive()` / `QueueDeleteDataFromPointer()`, w parserze `case I2C_COMMAND_GROUP_LEDS` → `pLedDisplay->QueueSendDataToLedTask(&parsingData)` + `vTaskResume(handlerTask_ledDisplay)`; analogicznie silnik – `StepperOptoPowerOFF` (`ParserDataToStepperMotorDataQueue`) + task `stepperMotorDataParser`.
- Dodać obsługę:
  - On / Off BT (`btAudioDeviceOn/Off`),
  - komendy odtwarzania (`audioSinkCommand`: Play/Stop/Pause/Next/Previous/FF/Rew → metody `btAudio*`, dziś `protected`),
  - **żądania stanu** (STM32 pyta → `sendToMaster*()`); do zaprojektowania subcommand(y) „request”.
- Obiekt `testBT` jest **lokalny w `app_main`** (żyje, bo `app_main` kończy się pętlą `vTaskDelay`) – parser nie ma do niego dostępu → globalny wskaźnik jak `pMotor`/`pLedDisplay` w `tasksFunctions.cpp`.
- `app_main.cpp`: `testBT.btAudioDeviceOn()` przy starcie – docelowo On/Off decyduje STM32 (menu).
- Obsługa błędów połączenia z telefonem (nieudany reconnect, ESP_AVRC_PLAYBACK_ERROR, rozłączenie w trakcie grania) – ustalić, co raportować do STM32.
- Opcjonalnie: wysyłanie pozostałych pól (`audioState`, metadane, głośność, pozycja) – nowe subcommandy, tylko jeśli STM32 ich potrzebuje.

### 4.3 STM32 – task / obsługa komunikatów BT z ESP32
- **Brakuje całej ścieżki odbioru ramek BT na STM32** (parser → kolejka → klasa/task BT):
  - parser `esp32_i2cComunicationDriver::parseReceivedData()` (`comunication_esp32i2cComunicationDriver.cpp` ~370): `case I2C_COMMAND_GROUP_BTAUDIO: break;` – **ramki BT są IGNOROWANE** (parser ich nie przekazuje dalej);
  - **nie ma klasy obsługującej BT** po stronie STM32 (odpowiednik `bt_audio_sink`: stan BT, wysyłanie komend do ESP32 – On/Off, Play/Next…, żądania stanu);
  - **nie ma kolejki** na ramki BT od parsera ani taska, który by je obsługiwał.
  - Wzorzec na STM32: klawiatura – parser `case I2C_COMMAND_GROUP_KEYBOARD` → `parserFunction::keyboardToRadioMeny()` → `pRadioMenu->queueRadioMenuKbrdSend()`; wysyłka do ESP32 – `ledsController` / `StepperOptoPowerOFF` przez `pi2cMaster->getTransmitQueue()` (`tasksFunctions.cpp` ~203).
- Napisać obsługę wszystkich subcommandów i miejsce przechowywania stanu BT na STM32.
- **Reakcja na rozłączenie telefonu** (np. telefon się wyłączy) – po to ESP32 wysyła stan połączenia.
- **Odpytywanie:** gdy brak połączenia, STM32 co jakiś czas pyta ESP32 o stan BT (mechanizm z 4.2).
- STM32 wie z ramek, że BT nie jest połączony i trzeba go połączyć.
- Przełączanie źródła audio dopiero po `highZenabled` / OFF od ESP32 (wspólna szyna I2S do DAC).
- **KOLEJNOŚĆ (decyzja użytkownika 2026-11-23):** (1) obsługa informacji PRZYCHODZĄCYCH z ESP32 → kompilacja → sprawdzenie działania na sprzęcie, (2) **zmiana tworzenia OBIEKTÓW KLAS (np. `pESP32`, `pi2cMaster`, `pRadioMenu`, `pHmiLeds`, `pStepperMotor`, `pBtAudio`, `pPrintf`; ESP32: `pMotor`, `pLedDisplay` itd.) z `new (std::nothrow)` na statyczne – w STM32 i ESP32; dynamiczne bufory danych (`pData` ramek I2C, printf – `new char[]` / `delete[]`) ZOSTAJĄ dynamiczne** (statyczny obiekt lokalny w funkcji inicjalizującej, np. `static btAudioDevice btAudio(...); pBtAudio = &btAudio;` – pamięć w `.bss` widoczna przy linkowaniu, konstruktor w tasku po starcie schedulera; globalne wskaźniki/`extern` zostają jako wskaźniki na obiekty statyczne; NIE zwykłe obiekty globalne – konstruktor przed `main()`, nieokreślona kolejność między plikami), (3) dopiero potem obsługa informacji WYCHODZĄCYCH (STM32 → ESP32).

### 4.4 STM32 – menu Bluetooth: Init / ReturnInit / DestroyInit + bindowanie klawiszy
- Węzeł „Bluetooth” w `menu_radioMenu.cpp:75-76` → `radioMenuAudioBluetooth_Init / _ReturnInit / _DestroyInit` w `menu_radioMenu_audio_bluetooth.cpp` (szkielet, 60 linii; „bindowanie klawiatury” w ReturnInit ~37).
- Init → wysłać On BT do ESP32; DestroyInit → Off (i czekać na high-Z/OFF przed następnym źródłem); ReturnInit → stan zachowany/odświeżony.
- Bindowanie klawiszy: Play/Pause, Next, Previous, FF/Rew → komendy do ESP32.
- **Zabezpieczenia komend odtwarzania po OBU stronach** (STM32: nie wysyłać przy BT niepołączonym; ESP32: guardy już są, rozszerzyć o stan połączenia, jeśli trzeba).
- Obsługa błędów połączenia BT z telefonem i innych (pokazać w menu / LED?).

### 4.5 Na koniec: usunięcie BTstack – **ZAPYTAJ UŻYTKOWNIKA**
- `app_main.cpp` ~149: `#if USED_BLUETOOTH_STACK == USE_BTSTACK btstack_init(); btstack_main(); btstack_run_loop_execute(); #endif` (+ `//TODO: ten stosu susunąć`), include `a2dpSikn/a2dp_sink_demo_main_incl_err.h` (~29-31), `hwConfigFile.h:16-18` (`USE_BTSTACK`), komponent `Components/btstack/`, `main/a2dpSikn/`, wpisy w `main/CMakeLists.txt`.
- **Przed zmianą zapytaj:** usunąć całkiem, zakomentować, czy zostawić za `#if` (dziś wyłączone przez `USED_BLUETOOTH_STACK = USE_ESP32A2DP`).

### 4.6 Drobne (zgłoszone, czekają na decyzję)
- `menu_myList.cpp:52` (`addAtEnd`): `new myList` **bez** `(std::nothrow)` (reszta ma).
- `menu_myList.cpp:17`: `assert(this)` – nic nie sprawdza.
- `bt_audio.cpp`: brak wcięcia w `btAudioNext()`; literówki w nazwach typu `Transision`, `Intrrrupt` – tylko jeśli użytkownik zechce.
- Pliki `bt_audio — kopia.cpp/.h` w `main/bt_audio/` – do decyzji użytkownika.

---

## 5. DO ZROBIENIA – I2C / ogólne (przeniesione z poprzedniego kontekstu)

- **Restart ESP32 w trakcie pracy STM32:** ping jest tylko przy starcie STM32; reset ESP32 później (panic, flashowanie, EN) → zbocza z bootu liczone jako tokeny → śmieci → assert. (dawne 7.5.8)
- `ESP_I2C_BUS_DELAY` 7 ms – próba zmniejszenia osobnym krokiem, z testem szybkich serii ramek LED.
- Obsługa błędów zamiast `assert` (`//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32`).
- Opcja (niezaakceptowana): semafor szyny → mutex rekurencyjny.
- si4689: obrazy ~520 kB, I2C1 ~100 kHz → ~48 s ładowania; `HOST_LOAD` porcjami, oddawać semafor po porcji; rozważyć 400 kHz lub SPI flash.
- ESP32 (znane, niepoprawione): okno między callbackiem a ponownym `i2c_slave_receive()` dla RX; okno INT#1 → odczyt len niechronione.
- STM32 (znane, niepoprawione): `i2cMasterTransmitToSlaveDataTask` ma `vTaskDelay(3000)` w każdej iteracji.
- Migracja ESP32 na ESP-IDF 6.x – **na sam koniec** (usuwa błąd sterownika I2C slave 5.2 i obejścia: ramka max 32 B).
- Decyzje użytkownika: printfy DIAG ESP32 zostają; IWDG – nic nie robimy; `adapter speed` 12000 (przy problemach z flashowaniem 2000).
- **Ostatni krok projektu:** usunąć ten plik z repozytorium.
