# KONTEKST: Mustang Radio – ESP32 (HMI + Bluetooth) ↔ STM32 (radio, menu)

> **Dla Claude:** przeczytaj cały plik, zanim cokolwiek zrobisz. To przypomnienie: co jest do zrobienia, jakie są zasady pracy, a na końcu – co już zrobione (sekcja **ZROBIONE**).
> Numery linii są orientacyjne – przed każdą zmianą przeczytaj aktualny plik.
> Stan na: **2026-11-25**. Poprzedni kontekst (`KONTEKST_I2C_ESP32_STM32.md`, szczegóły I2C, analizy, testy) jest w historii gita:
> `git show 887b0444:KONTEKST_I2C_ESP32_STM32.md`

---

## !!! PRIORYTET 1 – STM32: dioda statusu (`errorLed`) dla BT (kolejność zaakceptowana 2026-11-24)

Marker: `//#error Tutaj dać świecenie LED trzeciego` (zakomentowany przez użytkownika 2026-11-24) w `device_btAudio.cpp` ~94 (`case BT_SUBCOMMAND_STATE_CONNECTION`). Trzecia dioda (`ledsController::errorLed`) = od razu **dioda statusu**:
BT → stan połączenia; AM/FM/DAB → „Active antena”; SD/USB → „Media In” (komentarze w `radioMenuAudio*_ReturnInit`).
**Zasada (użytkownik 2026-11-24):** dioda statusu pokazuje stan TYLKO aktualnie granego źródła. Ramka stanu od urządzenia,
które nie jest aktywnym źródłem (np. BT połączony z telefonem, ale gra FM), zapisuje się do struktury, ale diody NIE zmienia.
Przy przełączeniu źródła dioda ustawiana od nowa wg stanu nowego źródła (`ReturnInit`/`Init`).
Do ustalenia przy kroku 5: skąd brać „aktywne źródło” – własna flaga w urządzeniu (obecny plan) czy bieżący węzeł audio w `radioMenu`.

**Kolejność tworzenia obiektów w `initTaskFunctions()` (`tasksFunctions.cpp`):** `pi2cMaster` (wcześniej, w `startUpTask_initTaskFunctions`) → `pESP32` → `pHmiLeds` → `pStepperMotor` → `pBtAudio` (dostanie `pHmiLeds`) → taski I2C (odbiór, nadawanie, `esp32IntReq`, parser) → `pRadioMenu` + jego taski.
Zasada: najpierw WSZYSTKIE obiekty urządzeń ESP32, potem taski, które z nich korzystają; przyszłe AM/FM/DAB/SD/USB – za BT. Bezpieczne, bo konstruktory `ledsController`, `StepperOptoPowerOFF`, `btAudioDevice` są pasywne (nic nie wysyłają, potrzebują tylko `pi2cMaster->getTransmitQueue()`). (Krok 1 – ta kolejność – ZROBIONY, patrz ZROBIONE.)

**Kroki (każdy osobno: diff w czacie → akceptacja):**
2. **NASTĘPNY:** mutex w `ledsController` – ustawienie diody + wysyłka chronione razem (menu i parser na jednym obiekcie). Nazwy wg zasady „Mutex” (patrz ZROBIONE – mutexy), np. `MutexTake/Give`. Wariant API do wyboru: A – publiczne take/give z timeoutem (rekomendacja), B – metody z mutexem w środku.
3. Timeout wysyłki diod jako parametr z wartością domyślną = dzisiejsza (menu bez zmian), parser woła z 0 (parser nie może czekać do 700 ms).
4. Konstruktor `btAudioDevice(btAudioState*, ledsController*)` + `assert` + wywołanie w `tasksFunctions.cpp`.
5. Flaga „BT aktywnym źródłem” w `btAudioDevice`: true w `radioMenuAudioBluetooth_Init`, false w `_DestroyInit` (inaczej DISCONNECTED po przejściu na FM zgasi „Active antena”).
6. `device_btAudio.cpp` ~94: gdy BT aktywny – dioda statusu wg stanu połączenia, wysyłka z timeoutem 0, błąd → log; usunąć zakomentowany marker `//#error`.
7. `radioMenuAudioBluetooth_ReturnInit`: dioda wg `isConnectionState()` (komentarz „ustawienie diody BT connection”).
8. Później ten sam wzorzec: Active antena (AM/FM/DAB), Media In (SD/USB) – gdy będą klasy tych urządzeń.
- Zmiana nazwy `errorLed` → `statusLed`: pominąć albo na samym końcu, osobny krok.
- Kolory (propozycja, potwierdzić przy kroku 6): CONNECTED – niebieski ciągły; CONNECTING / DISCONNECTING – miga niebieski/czarny; DISCONNECTED – zgaszona.

---

## !!! PRIORYTET 2 – ESP32: panic I2C slave TX przy starcie BT

- **Objaw (commit `e6e4edba`):** bez `vTaskDelay(pdMS_TO_TICKS(100))` w `app_main.cpp` przed `bt_audio_sink testBT(...)` → panic
  `assert failed: void i2cEngin_slave::i2cSlaveTransmit() i2c_engine_slave.cpp:639 (0)` (wspólna gałąź `errorInTransmition` / `timeoutInTransmition`).
  Z opóźnieniem 100 ms działa. Marker w `app_main.cpp` ~135 – zakomentowany przez użytkownika 2026-11-24 (`/* … */`), ESP32 się kompiluje.
- **Kontekst:** konstruktor `bt_audio_sink` (`bt_audio.cpp` ~270, `btAudioDeviceOff()`) + `btAudioDeviceOn()` wrzucają do kolejki TX ~11 ramek BT naraz,
  kilka ms po utworzeniu tasków I2C; `start()` A2DP równolegle inicjuje kontroler BT (NVS/flash/przerwania).
- **DIAGNOZA (2026-11-24, log UART ESP32) – wyścig przy starcie, hipoteza (a) potwierdzona, (b) i (c) odrzucone:**
  - log: `TIMEOUT_IN_TRANSMITION`, `DIAG TX FAIL: stateBefore:1 retVal:0x0 status_reg:0x03100000 fifo_st:0x00000380 int_raw:0x00000013 int_ena:0x00000880 | … slave_rw:0`;
    wcześniej `DIAG Reception to me` (STM32 pisał do ESP32, np. LED z menu);
  - `stateBefore:1` = `lenDataInTransmition`, `retVal:0` → `i2c_slave_transmit` + `i2cTxWaitForFifoLoaded` OK, impuls INT wysłany;
    `tx_fifo_cnt = 4` (bity 18-23 `status_reg`) → `len` leży w FIFO NIEodczytany; `slave_rw:0` → STM32 tylko pisał, nie czytał → po 500 ms timeout → `assert(0)` `:639`;
  - przyczyna: ESP32 odpowiada ACK na ping zaraz po konstruktorze `i2cEngin_slave`; STM32 po ACK czeka `ESP32_READY_SETTLE_MS` (150 ms, `hwConfigFile.h:20`),
    dopiero potem `esp32I2cInitialised = true` (`tasksFunctions.cpp` ~241) i tworzy `pESP32` (`initTaskFunctions()`); do tego czasu `HAL_GPIO_EXTI_Callback`
    celowo ignoruje INT. ESP32 w tym czasie tworzy taski i `bt_audio_sink` (~11 ramek) → pierwszy impuls INT trafia w „głuche okno” STM32 i przepada;
    impuls nie jest powtarzany → ESP32 czeka do timeoutu. `vTaskDelay(100)` przesuwa impuls za okno – zależność czasowa, nie poprawka;
  - problem ogólny: zgubienie JEDNEGO impulsu INT (start, restart ESP32 – pkt 5, zakłócenie) = zawsze panic.
- **Poprawka – propozycje (NIE zaakceptowane):** (1, rekomendacja) ramka „STM32 gotowy” wysyłana przez STM32 po `initTaskFunctions()`;
  `i2cSlaveTransmit` na ESP32 nic nie nadaje, dopóki jej nie dostanie – pomoże też przy restarcie ESP32;
  (2) ESP32 ponawia impuls INT (np. co 100 ms) do odczytu – wymaga zmiany zliczania impulsów na STM32 (dziś impuls = osobny odczyt);
  (3) dłuższe opóźnienia po którejś stronie – tylko obejście.
- **Dwie OSOBNE sprawy (ustalone 2026-11-24):**
  1. **Synchronizacja startu** (ramka „STM32 gotowy”, propozycja 1) – usuwa PRZYCZYNĘ tego panicu; to nie obsługa błędów, tylko uzgodnienie gotowości obu stron. Szybkie – robić najpierw.
     **Zachowanie ESP32 w czasie czekania (ustalone 2026-11-25)** – NIE trzymać ramek w kolejce TX do „gotowy”: kolejka TX ma 20 ramek (`i2c_engine_slave.cpp:344`), `i2cSendDataToTransisionQueue()` czeka max 10 ms (`:488`),
     a okno czekania może być długie (ping + `ESP32_READY_SETTLE_MS` + init STM32, w przyszłości ładowanie si4689 ~48 s) → przepełnienie. Dlatego do czasu „gotowy”:
     - **klawiatura** – ramki odrzucać od razu (stare wciśnięcia po starcie i tak nie powinny działać); dziś przy pełnej kolejce: log `Queue feeding error` + klawisz przepada (`tasksFunctions.cpp` ~253-256, długi POWER → `radioPowerOff()`), bez panicu;
     - **BT** – to informacja o STANIE (liczy się ostatni): nic nie wkładać do kolejki, po „gotowy” ESP32 wysyła jedną migawkę stanu (`sendToMaster*()`); dziś pełna kolejka przy ramce BT = panic (patrz pkt 2).
     Ramka „gotowy” NIE rozwiązuje wyścigu klawiatury na STM32 – ten jest już zabezpieczony osobno (patrz ZROBIONE).
  2. **Obsługa błędów zamiast `assert`** – potrzebna I TAK: impuls INT może zginąć w trakcie pracy (zakłócenie, restart ESP32, zajęta szyna), dziś każdy taki przypadek = panic.
     Docelowo: ESP32 po timeoucie – wyczyścić FIFO TX, zwolnić `pData` (albo ponowić N razy), wrócić do `idle`, zalogować; STM32 – przy braku odpowiedzi / śmieciach
     (zła suma, długość) odrzucić ramkę i ponownie się zsynchronizować, bez `assert`. Później, osobny większy temat (oba kontrolery) – patrz pkt 5 „Obsługa błędów”.
     **Także strona NADAWCY ramki (dopisane 2026-11-25):** ESP32 `bt_audio_sink::sendToMasterDeviceOnOffState()` / `...ConnectionState()` / `...AVRCPlayStatus()` / `...I2sPinsState()`
     mają `assert(frameQueued == pdTRUE)` (`bt_audio.cpp` ~49/58/67/76) → każde chwilowe zapchanie kolejki TX (20 ramek, np. STM32 zajęty dłuższą transmisją; start BT wrzuca ~11 ramek naraz) = panic.
     Docelowo: przy pełnej kolejce – log i pominięcie ramki stanu (`pData` zwalnia już `QueueSend`, brak wycieku); następna zmiana stanu albo migawka po „STM32 gotowy” przyniesie aktualny stan.
     To inna warstwa niż błędy transmisji na szynie (`i2c_engine_slave.cpp` / sterownik STM32) – przy realizacji objąć oba miejsca.
- **Na razie (decyzja użytkownika 2026-11-24):** obejście opóźnieniem `vTaskDelay` w `app_main.cpp` przed `bt_audio_sink`; poprawka później.
- Przy okazji: `testBT` nadal lokalny w `app_main` (nie `static`, patrz 4.2).
- Potem: pozostałe sprawy z kontekstu (decyzja użytkownika: najpierw LED, potem reszta).

---

## 0. Zasady pracy z użytkownikiem (OBOWIĄZKOWE)

1. **Nigdy nie zmieniaj kodu z własnej inicjatywy.** Kolejność: ustalenia (tylko otwarte kwestie) → plan → **diff w czacie (ścieżka + numery linii)** → wyraźna akceptacja → dopiero wtedy edycja.
2. **Krok po kroku:** jedna logiczna zmiana = jeden krok = osobna akceptacja.
3. **Akceptacja musi być jednoznaczna** („akceptuję”, „tak, wprowadź krok X”). „Poprawmy to”, „dodaj X”, „robimy i tak” = dopisanie do planu, **NIE akceptacja diffa** – dopytaj.
4. **Nie usuwaj kodu bez pytania** (także „duplikatów”). Zakomentowany kod użytkownika zostaje.
5. Problemy zauważone przy okazji – **zgłaszaj, nie poprawiaj**.
6. Reużywaj istniejący kod; styl jak w otoczeniu (taby, komentarze po polsku).
7. Asercje: **operacja zawsze poza makrem, w `assert`/`configASSERT` tylko sprawdzenie wyniku** (wzorzec w obu projektach, patrz ZROBIONE – asercje).
8. Spójność STM32 ↔ ESP32: enumy w ramkach I2C jako `typedef enum : uint8_t` we wspólnym pliku.
9. Podczas debugowania na żywo pytaj zwykłym tekstem (nie narzędziem wielokrotnego wyboru).
10. Commit / push – **tylko na prośbę użytkownika**. Przed commitem **zaktualizuj ten plik kontekstu** (wchodzi w ten sam commit); rzeczy zrobione przenoś do sekcji ZROBIONE na końcu.
11. **„Zaczynamy krok X” = pokaż diff kroku X w czacie i czekaj** – NIE zapisuj od razu. Odpowiedzi na pytania projektowe też nie są zgodą na pisanie kodu.
12. **Wzorce z TEGO SAMEGO kontrolera:** na STM32 rób jak w kodzie STM32, na ESP32 jak w ESP32 (nie przenoś mechanizmów między nimi).
13. **Pamięć dynamiczna:** w każdym kroku wskaż, kto jest właścicielem `pData` i gdzie jest zwalniane – po użyciu ORAZ na każdej ścieżce błędu (przed `assert`); pilnuj też podwójnego zwolnienia.
14. **Nazwy blokad (2026-11-25):** uchwyt/funkcja/metoda obsługująca mutex ma „Mutex” w nazwie, nie „Semaphore” (także w komentarzach i zakomentowanym kodzie).

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
| Pełny build z terminala (działa 2026-11-23) | STM32: `make -j8 all` w `Radio_main/Debug` (toolchain + make z pluginów `C:\ST\STM32CubeIDE_2.0.0`; nowy plik .cpp trafia do makefile dopiero po odświeżeniu/buildzie w CubeIDE). ESP32: ESP-IDF `C:\esp\v5.2.8\esp-idf` (EIM, `IDF_TOOLS_PATH=C:\Espressif`), `idf.py -B <katalog w scratchpad> build` – nie rusza `build/` IDE. clangd bez ścieżek include projektu zgłasza fałszywe błędy – miarodajny jest tylko build |

---

## 4. DO ZROBIENIA – Bluetooth

### 4.1 TEST NA SPRZĘCIE (NAJPIERW – wgrać OBA firmware'y; wszystko z sekcji ZROBIONE jest tylko skompilowane)
- **Start radia:** printf, ping ESP32 (`isDeviceReady` – teraz pod mutexem szyny), LED, stepper, klawiatura/enkodery, menu (obiekty statyczne).
- **Klawisz/enkoder w trakcie startu STM32:** log `keyboard frame dropped - radioMenu not created yet`, bez crasha.
- **Mutexy:** EQ → powrót po ~12 s do tego samego urządzenia; długie EQ (powrót od razu); krótkie VOL w EQ (następne urządzenie); kilka cykli wejścia/wyjścia z EQ;
  ESP32: podświetlenie przy starcie, mignięcie przy klawiszu, diody stałe/migające po ramkach LED ze STM32.
- **BT odbiór na STM32:** log UART `BT audio device log: ...` przy starcie ESP32 (On), połączeniu telefonu, play/pause, rozłączeniu; podgląd `radioStruct.audioDevices.bluetooth` / `*pBtAudio` w Live Expressions.
- **BT na ESP32:** On → telefon łączy się → Off: telefon traci radio z listy wyszukiwania, STM32 dostaje DISCONNECTED/STOPPED/piny/OFF;
  On ponownie: telefon łączy się sam (auto-reconnect po Off→On); podwójne On / Off, rozłączenie z telefonu, wyłączenie telefonu; log ramek I2C po stronie ESP32 i kolejność high-Z → OFF.

### 4.2 ESP32 – task / obsługa komend od STM32 (połączenie, granie)
- **Brakuje całej ścieżki odbioru ramek BT na ESP32** (parser → kolejka → task BT):
  - parser `i2cReceivedDataParser` (`tasksFunctions.cpp` ~633) ma `case` dla KEYBOARD/NVS/LEDS/STEPPER – **brak `I2C_COMMAND_GROUP_BTAUDIO`**;
  - klasa `bt_audio_sink` **nie ma kolejki** na ramki od parsera ani taska, który by je obsługiwał.
  - Wzorzec do powielenia (już działa): LED – `Leds_backlight.{h,cpp}`: `i2cQueue4DynamicData* ParserDataToLedsDataQueue` + `QueueSendDataToLedTask()` / `QueueReceive()` / `QueueDeleteDataFromPointer()`, w parserze `case I2C_COMMAND_GROUP_LEDS` → `pLedDisplay->QueueSendDataToLedTask(&parsingData)` + `vTaskResume(handlerTask_ledDisplay)`; analogicznie silnik – `StepperOptoPowerOFF` (`ParserDataToStepperMotorDataQueue`) + task `stepperMotorDataParser`.
- Dodać obsługę:
  - On / Off BT (`btAudioDeviceOn/Off`),
  - komendy odtwarzania (`audioSinkCommand`: Play/Stop/Pause/Next/Previous/FF/Rew → metody `btAudio*`, dziś `protected`),
  - **żądania stanu** (STM32 pyta → `sendToMaster*()`); do zaprojektowania subcommand(y) „request”.
- Obiekt `testBT` jest **lokalny w `app_main`** (żyje, bo `app_main` kończy się pętlą `vTaskDelay`) – parser nie ma do niego dostępu → globalny wskaźnik jak `pMotor`/`pLedDisplay` w `tasksFunctions.cpp`, obiekt **`static`** (dziś leży na stosie taska `main`, a `bt_audio_sink` ma dużą strukturę stanu); wzorzec: `static bt_audio_sink btAudioObject(...); pBtAudio = &btAudioObject;`.
- `app_main.cpp`: `testBT.btAudioDeviceOn()` przy starcie – docelowo On/Off decyduje STM32 (menu).
- Obsługa błędów połączenia z telefonem (nieudany reconnect, ESP_AVRC_PLAYBACK_ERROR, rozłączenie w trakcie grania) – ustalić, co raportować do STM32.
- Opcjonalnie: wysyłanie pozostałych pól (`audioState`, metadane, głośność, pozycja) – nowe subcommandy, tylko jeśli STM32 ich potrzebuje.

### 4.3 STM32 – obsługa komunikatów BT (odbiór ZROBIONY – patrz ZROBIONE)
- Po teście: wysyłka STM32 → ESP32 (On/Off, Play/Next…, żądania stanu `get...()`) – w tej samej klasie `btAudioDevice`, jak `ledsController` / `StepperOptoPowerOFF` przez `pi2cMaster->getTransmitQueue()`.
  `get...()` = TYLKO wysłanie żądania do ESP32 (zwraca, czy ramka trafiła do kolejki TX), odpowiedź asynchronicznie przez parser → struktura → `is...()`; **jak potwierdzić, że odpowiedź przyszła – do ustalenia** (I2C asynchroniczne, dziś brak takiej flagi).
- **Reakcja na rozłączenie telefonu** (np. telefon się wyłączy) – po to ESP32 wysyła stan połączenia.
- **Odpytywanie:** gdy brak połączenia, STM32 co jakiś czas pyta ESP32 o stan BT (mechanizm z 4.2).
- STM32 wie z ramek, że BT nie jest połączony i trzeba go połączyć.
- Przełączanie źródła audio dopiero po `highZenabled` / OFF od ESP32 (wspólna szyna I2S do DAC).

### 4.4 STM32 – menu Bluetooth: Init / ReturnInit / DestroyInit + bindowanie klawiszy
- Węzeł „Bluetooth” w `menu_radioMenu.cpp:75-76` → `radioMenuAudioBluetooth_Init / _ReturnInit / _DestroyInit` w `menu_radioMenu_audio_bluetooth.cpp` (szkielet; „bindowanie klawiatury” w ReturnInit ~37).
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
- Pliki `bt_audio — kopia.cpp/.h` w `main/bt_audio/` – do decyzji użytkownika (w `.cpp:37` została stara nazwa `BT_SUBCOMMAND_onConnCallback` – plik niekompilowany, nie ruszany).
- **STM32 – `malloc`/`free` newlib prawdopodobnie NIE są thread-safe:** `configUSE_NEWLIB_REENTRANT 1`, ale brak `newlib_lock_glue.c` / własnych `__malloc_lock`/`__malloc_unlock`; `new`/`delete` `pData` działa równolegle w kilku taskach (odbiór I2C, parser, TX, printf) → ryzyko rzadkiego uszkodzenia sterty. Do sprawdzenia (osobny temat). `new` idzie na stertę newlib (`_sbrk`), NIE na stertę FreeRTOS (`heap_4`, 30 KB).
- **STM32 – parser może stać do 700 ms:** `keyboardToRadioMeny()` ignoruje wynik `queueRadioMenuKbrdSend()`, które czeka do 700 ms (`menu_radioMenu.cpp:178-179`) – przy zapchanym menu task parsera blokuje też ramki BT i resztę odbioru z ESP32.
- `device_StepperOptoPowerOFF.cpp:59-74` (STM32): luźne deklaracje funkcji spoza klasy i powtórzone `#define MOTOR_SUBCOMMAND_*`.

---

## 5. DO ZROBIENIA – I2C / ogólne (przeniesione z poprzedniego kontekstu)

- **Restart ESP32 w trakcie pracy STM32:** ping jest tylko przy starcie STM32; reset ESP32 później (panic, flashowanie, EN) → zbocza z bootu liczone jako tokeny → śmieci → assert. (dawne 7.5.8)
- `ESP_I2C_BUS_DELAY` 7 ms – próba zmniejszenia osobnym krokiem, z testem szybkich serii ramek LED.
- Obsługa błędów zamiast `assert` (`//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32`). Konkretny przypadek i zakres – PRIORYTET 2 („Dwie OSOBNE sprawy”, pkt 2).
- Opcja (niezaakceptowana): mutex szyny I2C → mutex rekurencyjny.
- si4689: obrazy ~520 kB, I2C1 ~100 kHz → ~48 s ładowania; `HOST_LOAD` porcjami, oddawać mutex szyny po porcji; rozważyć 400 kHz lub SPI flash.
- ESP32 (znane, niepoprawione): okno między callbackiem a ponownym `i2c_slave_receive()` dla RX; okno INT#1 → odczyt len niechronione.
- STM32 (znane, niepoprawione): `i2cMasterTransmitToSlaveDataTask` ma `vTaskDelay(3000)` w każdej iteracji.
- Migracja ESP32 na ESP-IDF 6.x – **na sam koniec** (usuwa błąd sterownika I2C slave 5.2 i obejścia: ramka max 32 B).
- Decyzje użytkownika: printfy DIAG ESP32 zostają; IWDG – nic nie robimy; `adapter speed` 12000 (przy problemach z flashowaniem 2000).
- **Ostatni krok projektu:** usunąć ten plik z repozytorium.

---
---

## ZROBIONE (wszystko tylko skompilowane – NIEtestowane na sprzęcie, test w 4.1)

### Mutexy zamiast semaforów binarnych (2026-11-25, STM32 + ESP32)
Zasada: blokada zasobu (Take i Give w tym samym tasku, nigdy z ISR) = **mutex** (`xSemaphoreCreateMutex`, startuje WOLNY – bez początkowego `Give`, dziedziczenie priorytetu);
sygnalizacja (Give z innego taska / ISR) = semafor binarny / zliczający. Mutex: Give tylko przez task-właściciela (inaczej `configASSERT` w `xTaskPriorityDisinherit`), nie w ISR.
Początkowe `Give` po utworzeniu – usunięte we wszystkich miejscach.
- **STM32 szyna I2C (`i2cMaster`, taski `i2cTransmit`, `i2cReceive`, `initTasks`/`isDeviceReady`):** `handle_i2cBinarySemaphore` → `handle_i2cBusMutex`, metody `i2cMasterSemaphoreTake/Give()` → **`i2cMasterMutexTake/Give()`** (`comunication_i2cEngine.{h,cpp}`, `tasksFunctions.cpp`; także komentarze i zakomentowany kod, w tym `comunication_esp32i2cComunicationDriver.h:43-44`).
- **STM32 licznik timeoutu menu peryferiów (`radioMenu`, taski `RadioMenu`, `periTimeout`):** `peripheryMenu_TimeoutCounterSemaphore` → `peripheryMenu_TimeoutCounterMutex` (`menu_radioMenu.{h,cpp}`).
- **STM32 zawieszanie taska timeoutu (`radioMenu`, taski `RadioMenu`, `periTimeout`):** `peripheryMenu_TaskSuspendAllowedSemaphore` → `peripheryMenu_TaskSuspendAllowedMutex` (`menu_radioMenu.{h,cpp}`, `tasksFunctions.cpp`). Usunięte martwe bloki `if (eTaskGetState(...) == eSuspended) { Give }` w `menuFunction_returnForEqualizerMenu()` i `menuFunction_volButShortPressed()`.
  **ZASADY, KTÓRE MUSZĄ ZOSTAĆ (inaczej zakleszczenie – „ratunek” Give z innego taska przy mutexie niemożliwy):** (a) task timeoutu bierze mutex tylko na chwilę (`tasksFunctions.cpp` ~162-165) i ZAWSZE go oddaje przed czymkolwiek innym; (b) task menu zawiesza task timeoutu (`vTaskSuspend`) TYLKO trzymając ten mutex. Kolejność blokad stała: TaskSuspend → licznik.
- **ESP32 taśma WS2812 (`LEDS_BACKLIGHT`, taski `Leds control`, `Backlight control`):** `handlerMutex_ledDisplay_Backlight` teraz naprawdę mutex; metody `SemaphoreTake/Give()` → **`MutexTake/Give()`** (`Leds_backlight.{h,cpp}`, `tasksFunctionsLeds.cpp`, zakomentowany blok `tasksFunctions.cpp:316-462`). Oba taski LED zawieszają się dopiero PO `MutexGive()`. Blokada potrzebna (taski bez przypisania do rdzenia). Alternatywa (NIE teraz): jeden task-właściciel taśmy + kolejka żądań.
- Bez zmian (poprawne): STM32 semafor zliczający INT od ESP32 (sygnalizacja z EXTI); ESP32 `portMUX_TYPE s_i2cTxStateMux` (sekcja krytyczna z ISR); ESP32 `xI2CMasterMutex` (już mutex).

### Wyścig `pRadioMenu` z ramkami klawiatury przy starcie STM32 (2026-11-25, commit `2a6247dd`)
- Task parsera (`tskIDLE_PRIORITY + 1`) startuje przed ustawieniem `pRadioMenu` (`tasksFunctions.cpp` ~211 vs ~217). `radioMenu` nie da się utworzyć przed taskami (konstruktor startuje urządzenie → LED przez kolejkę TX).
- `parserFunction::keyboardToRadioMeny()` (`comunication_esp32i2cComunicationDriver.cpp` ~393): przy `pRadioMenu == nullptr` ramka odrzucona z logiem, bez `assert`; `pData` zwalnia task parsera.
- Przegląd podobnych wyścigów w STM32 i ESP32 – brak innych (EXTI ma guard `esp32I2cInitialised && pESP32 != nullptr`; ESP32 tworzy wszystkie obiekty przed taskami; callbacki BT z guardem `instance == nullptr`).
- Odrzucone: tylko ramka „STM32 gotowy” (przepełnienie kolejki TX ESP32), rozdzielenie konstruktora `radioMenu` (za duża zmiana).

### Kolejność obiektów urządzeń w `initTaskFunctions()` (2026-11-24, PRIORYTET 1 krok 1)
- `hmiLedsObject` i `stepperMotorObject` przeniesione zaraz za `pESP32`, przed `btAudioObject` (sama kolejność, bez zmiany zachowania).

### Obiekty klas z `new` na `static` (2026-11-23, commit `598a396d`)
- Zakres TYLKO obiekty tworzone w STM32 `tasksFunctions.cpp` (`pPrintf`, `pi2cMaster`, `pESP32`, `pBtAudio`, `pHmiLeds`, `pStepperMotor`, `pRadioMenu`), ESP32 `tasksFunctions.cpp` (`p_i2cSlave`, `pSTORAGE`, `pLedDisplay`, `p_i2cMaster`, `p_MCP23008`, `pMotor`) i ESP32 `app_main.cpp` (`klawiatura`). NIE ruszane: `new` wewnątrz klas i dynamiczne bufory danych.
- Obiekt `static` lokalny w funkcji + globalny wskaźnik `pX = &obiekt`; `static` KONIECZNE (STM32 `startUpTask()` z `main()` przed `osKernelStart()`, `startUpTask_initTaskFunctions` usuwa się sam). Martwe `assert(pX)` usunięte. NIE obiekty globalne (konstruktor przed `main()`).
- Kolejność tworzenia bez zmian – konstruktor obiektu `static` wykonuje się w miejscu deklaracji; `radioMenu` musi zostać po taskach I2C i po `pHmiLeds`/`pStepperMotor`.

### STM32 – odbiór ramek stanu BT z ESP32 (2026-11-23, commit `886d11e0`)
- Stan BT: typ `btAudioState` + pole `radioStruct.audioDevices.bluetooth` (`menu_radioMegaStruct.h`); pisze WYŁĄCZNIE `btAudioDevice`.
- Klasa `btAudioDevice` (`device_btAudio.{h,cpp}`): konstruktor dostaje wskaźnik do `radioStruct.audioDevices.bluetooth` (stan startowy OFF / `highZenabled` / DISCONNECTED / STOPPED), `processReceivedData(i2cFrame_btAudio*)` – `switch` po `BT_SUBCOMMAND_STATE_*`, zapis + log, nieznany → `assert(0)`.
- **Konwencja nazw:** `is...()` (`isDeviceOn`, `isConnectionState`, `isPlayStatus`, `isI2sPinsState`) = OD RAZU ostatni stan z lokalnej struktury; `get...()` = (do zrobienia, 4.3) tylko żądanie do ESP32.
- Parser: `case I2C_COMMAND_GROUP_BTAUDIO` → `parserFunction::btAudioToBtAudioDevice()` → `pBtAudio->processReceivedData()` (`extern pBtAudio`, jak klawiatura). **Przetwarzanie w locie – bez kolejki i taska**; w parserze TYLKO zapis stanu i log, nic blokującego; reakcje na zmianę stanu – w menu (4.4).
- `pBtAudio` tworzony przed taskiem parsera.

### ESP32 – Bluetooth `bt_audio_sink` (2026-11-21/22)
Plik: `HMI_and_bluetooth/main/bt_audio/bt_audio.{h,cpp}`.
- **`btConnectionState`** (prywatna struktura) – cały stan BT: `btAudioDeviceOn_Off`, `highZstateState` (4 stany pinów I2S), `onConnCallbackStates`, `onAVRCPlayStatusCallbackStates`, `audioState`, `sampleRate`, `peerName`, `peerAddress`, `avrcConnected`, `volume`, `playPosMs`, `title/artist/album`.
- **Callbacki** biblioteki = prywatne metody statyczne (dostęp przez `static bt_audio_sink* instance`). Callback **tylko zapisuje do struktury i woła metodę wysyłającą**.
- **Publiczne metody wysyłające ze struktury** (gotowe pod żądania STM32): `sendToMasterDeviceOnOffState()`, `sendToMasterConnectionState()`, `sendToMasterAVRCPlayStatus()`, `sendToMasterI2sPinsState()`. Ramki zerowane (`= {}`).
- **`setI2sPinsState()`** – jedyne miejsce zmiany stanu pinów (i2sState + struktura + wysyłka).
- **On/Off:** sink i I2S tworzone RAZ (pierwsze On), potem `start()` / `end(false)`; `delete` tylko w destruktorze (biblioteka trzyma globalne wskaźniki na sink). Guardy podwójnego On/Off.
- **Off:** `end(false)` → `esp_bt_gap_set_scan_mode(NON_CONNECTABLE, NON_DISCOVERABLE)` → do STM32 brakujące DISCONNECTED / STOPPED → zerowanie danych sesji (peerAddress/peerName zostają) → `i2sNotConfigured` → **high-Z** → `highZenabled` → **OFF** (high-Z ZAWSZE przed OFF – wspólna szyna I2S do DAC).
- **On:** high-Z off → `i2sNotConfigured` → `begin(cfg)` → `i2sConfigured` → `start()` → ON → migawka (połączenie, odtwarzanie, piny).
- **Konstruktor:** high-Z jako PIERWSZE (wymóg sprzętowy), potem `btAudioDeviceOff()` (STM32 dostaje OFF).
- **Auto-reconnect:** tylko podczas On. `BluetoothA2DPSinkExt` przywraca adres ostatniego telefonu przed `start()` (bo `end()` czyści go w NVS). `start()` czeka 1000 ms (`reconnect_delay`).
- **Guardy komend AVRC** (`btAudioPlay/Stop/Pause/Next/Previous/FastForward/Rewind`) – nic nie robią przy BT wyłączonym.
- **Protokół:** wspólny `i2sPinStates : uint8_t` + `static_assert` w `comunicationStructures.h`. Subcommandy STANU BT (ESP32 → STM32; 2026-11-23 zmiana nazw i numeracji, commit `886d11e0` – **wymaga wgrania OBU firmware'ów**): `BT_SUBCOMMAND_STATE_DEVICE_ON_OFF 0x00`, `BT_SUBCOMMAND_STATE_CONNECTION 0x01`, `BT_SUBCOMMAND_STATE_AVRC_PLAY_STATUS 0x02`, `BT_SUBCOMMAND_STATE_I2S_PINS 0x03` (zakomentowany `BT_SUBCOMMAND_STATE_AUDIO 0x04`). `#define` – DUŻYMI LITERAMI z `_`.
- Profil rozmów (HFP) wyłączony w `sdkconfig`. GAP musi zostać (bez niego nie ma A2DP).

### Asercje (2026-11-21, oba projekty)
Wszystkie `assert(x = new …)`, `configASSERT(xTaskCreate(…))`, `assert(!esp_fun(…))` przepisane na: operacja → `assert(wynik)` / `== pdPASS` / `err == ESP_OK`. Uwaga: w ESP-IDF `assert` przy NDEBUG i tak wykonuje wyrażenie; na STM32 (newlib) – NIE, stąd zmiana.
