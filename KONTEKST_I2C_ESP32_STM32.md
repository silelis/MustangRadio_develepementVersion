# KONTEKST: komunikacja I2C ESP32 (slave) → STM32 (master) z GPIO interrupt request

> **Dla Claude na nowym komputerze:** przeczytaj ten plik w całości, zanim zaczniesz cokolwiek robić.
> Plik opisuje stan prac z 2026-09-29 (ESP32), 2026-10-28 (poprawki `pData`), 2026-11-03: RX STM32←ESP32 ZROBIONY (kroki 1–13, commit `b4e6bba9`), **2026-11-03 (test na sprzęcie): RX STM32←ESP32 DZIAŁA w trybie debugowania** (poprawka ESP32 `tx_fifo_cnt`, sekcja 3.6) oraz **2026-11-04: wspólny mechanizm czekania na koniec transmisji w `i2cMaster` (`waitForTransferEnd`) dla TX i RX — NIEtestowany na sprzęcie** (sekcja 7.3) oraz **2026-11-08: crash ESP32 (Interrupt WDT / Double exception) = błąd sterownika I2C slave ESP-IDF 5.2 — obejście: max długość ramki 32 B w obu kierunkach (sekcja 3.7)**; **okno uzbrojenia odbioru ESP32 — `portYIELD_FROM_ISR` w callbacku (sekcja 3.8)**.
> **Zacznij od sekcji 7 („Gdzie jesteśmy / co dalej”)** — tam: co jest zrobione, jak testować, zasady użycia `i2cMaster` i otwarte tematy (m.in. synchronizacja INT / wykrycie startu ESP32).
> Sekcje 2–4 to tło; sekcja 5 to opis kroków 1–13 (część zastąpiona w 7.3 — tam jest aktualny stan RX); stary plan z `xTaskNotify` został ZASTĄPIONY.
> Numery linii są orientacyjne — przed zmianą zawsze przeczytaj aktualny plik.

---

## 0. Zasady pracy z użytkownikiem (OBOWIĄZKOWE)

1. **Nigdy nie zmieniaj kodu z własnej inicjatywy.** Kolejność: plan → akceptacja planu → **diff pokazany w czacie** → akceptacja diffa → dopiero wtedy `Edit`.
2. **Krok po kroku:** jedna logiczna zmiana = jeden krok = osobna akceptacja. Po każdym kroku pokaż diff następnego i czekaj.
3. **Akceptacja musi być jednoznaczna** („akceptuję krok X”, „tak”, „zatwierdzam”). Samo „ok”, „dalej”, „no i?” lub powtórzenie prośby („dodaj X”) **NIE jest akceptacją** — dopytaj.
4. **Problemy zauważone przy okazji — zgłaszaj, nie poprawiaj.**
5. **Reużywaj istniejący kod** (funkcje, uchwyty tasków, `#define` z `main.h`/`hwConfigFile.h`) — użytkownik nie chce mnożyć nowych tasków/funkcji/zmiennych.
6. Komunikacja po polsku. Styl kodu jak w otoczeniu (taby, komentarze po polsku, nagłówki funkcji w stylu pliku).

---

## 1. Środowisko

| | |
|---|---|
| Repo | `MustangRadio_develepementVersion` (GitLab) |
| ESP32 | `02_Firmware/01_ESP32/HMI_and_bluetooth` — ESP-IDF **v5.2.8** (`C:/esp/v5.2.8/esp-idf`), Espressif-IDE; kompiluje się (potwierdzone przez użytkownika). **Komputer domowy (ze sprzętem):** ESP-IDF `release-v5.2` w `C:/esp/release-v5.2/esp-idf`, build: `. C:\Espressif\tools\Microsoft.release-v5.2.PowerShell_profile.ps1; idf.py build` |
| Debug ESP32 | J-Link CE V9.60 (klon) przez OpenOCD (`interface/jlink.cfg`, `target/esp32.cfg`, `adapter speed` w `HMI_and_bluetooth Configuration.launch`). Interfejs 2 J-Linka (`1366 0105 02`, w Zadigu „BULK interface”) musi mieć sterownik **WinUSB** (Zadig; J-Link Configurator nie umie przełączyć tego modelu), inaczej OpenOCD: `LIBUSB_ERROR_NOT_SUPPORTED`. Interfejs 0 (CDC, COM12) zostaje na sterowniku SEGGER |
| STM32 | `02_Firmware/02_STM32/Radio_main` — STM32H7A3RITx, STM32CubeIDE, HAL, FreeRTOS **V10.3.1** (CMSIS-RTOS v2, heap_4) |
| Pliki wspólne | `.../HMI_and_bluetooth/main/common/` (`comunicationProtocol.h`, `comunicationStructures.h`, `i2c_slave_master_queueClass`, checksum) — używane przez oba projekty |
| Sprzęt | użytkownik testuje w domu; w pracy nie ma dostępu do sprzętu |

---

## 2. Protokół ESP32 → STM32 (nowa wersja, dwufazowa)

- Adres ESP32: `I2C_SLAVE_ADDRESS_ESP32 = 0x3C` (7 bit). Linia INT: ESP32 GPIO → STM32 **PB5** (`esp32i2cInterruptReqest_Pin`, EXTI9_5, zbocze opadające, prio 5).
- **Start:** ESP32 po inicjalizacji daje jeden impuls INT 100 ms (`esp32i2cBusInitialised()`) — STM32 traktuje go jako „szyna gotowa” (`esp32I2cInitialised = true`), bez danych.
- **Jedna ramka = 2 fazy, każda z własnym impulsem INT (20 µs) i jednym odczytem I2C:**

```
 ESP32 (i2cSlaveTransmit)                         STM32 (master)
 idle: ramka z kolejki
 i2c_slave_transmit(&dataSize, 4 B) → czekaj tx_fifo_cnt>0 → lenDataInTransmition
 INT ▼ ─────────────────────────────────────────► token #1 → read 4 B (dataSize, size_t LE)
 ISR (STOP, slave_rw=1, tx_fifo_cnt==0) → lenDataTransmited   (zapas: to samo sprawdza task co tick)
 i2c_slave_transmit(pData, dataSize) → czekaj tx_fifo_cnt>0 → packageDataInTransmition
 INT ▼ ─────────────────────────────────────────► token #2 → read dataSize B (cała ramka)
 ISR → packageDataTransmited → delete[] → idle          parser: CRC + commandGroup
```

- STM32 **musi czytać dokładnie 4 B, potem dokładnie `dataSize` B** (ESP przesuwa stan po każdym zakończonym odczycie, niezależnie od liczby bajtów).
- `dataSize` obejmuje całą ramkę z nagłówkiem `i2cFrame_commonHeader {crcSum, commandGroup, size_t dataSize}`; zakres: `sizeof(i2cFrame_commonHeader) ≤ dataSize ≤ ESP32_SLAVE_RECEIVE_BUFFER_LEN` (**od 2026-11-08: 32**, wcześniej 52 — sekcja 3.7).
- **ESP32 czeka na każdy odczyt max 500 ms** (`tx_timeout_ms`), potem przechodzi w `timeoutInTransmition` — **stan końcowy** (ESP przestaje nadawać; na razie tylko printf, obsługa błędów później).
- **Debugowanie:** ESP32 odlicza 500 ms także wtedy, gdy STM32 stoi na breakpoincie. Breakpoint w STM32 w środku protokołu (np. `masterReceiveData()` `case lenDataReceived`, przed odczytem ramki) ⇒ ESP32 `TIMEOUT_IN_TRANSMITION` + `assert` — to NIE jest błąd komunikacji (potwierdzone 2026-11-03: `stateBefore:3`, `tx_fifo_cnt=12` = ramka czekała w FIFO). Breakpointy stawiać po zakończeniu odczytu (np. `parseReceivedData()`).

---

## 3. Co zostało zrobione po stronie ESP32 (zacommitowane; od `b4e6bba9` dodatkowo `assert(0)` w stanach timeout/error i wspólny `ESP32_I2C_TX_TIMEOUT_MS`)

Plik: `02_Firmware/01_ESP32/HMI_and_bluetooth/main/i2c_engine/i2c_engine_slave.cpp` (+ `.h`)

### 3.1 `i2cEngin_slave::i2cSlaveTransmit()` — maszyna stanów TX
- Przejście `packageDataTransmited → idle` + `pData = nullptr` (wcześniej double free w pętli).
- Printfy każdego stanu (z `\n`); dla stanów `lenDataInTransmition`, `packageDataInTransmition`, `errorInTransmition`/`timeoutInTransmition` — przy wejściu w stan i potem co 1000 ms (lambda `isTimeToPrintTxState`, zmienne lokalne `txStatePrintPeriod_ms`, `txLastPrintedState`, `txLastPrintTick`).
- Timeout w stanach `...InTransmition`: `txStateEntryTick` zapisywany po impulsie INT; po `tx_timeout_ms` (500 ms) → `timeoutInTransmition`.
- `errorInTransmition`/`timeoutInTransmition`: bez `assert`, zwolnienie `pData` raz, `vTaskDelay(1)`, wewnętrzny `switch` z `default:`.

### 3.2 Callback ISR `i2c_slave_rx_done_callback`
- `switch ((i2c_slave_read_write_status_t)I2C0.status_reg.slave_rw)` z enumem IDF `I2C_SLAVE_WRITE_BY_MASTER` / `I2C_SLAVE_READ_BY_MASTER`.
- Enum `i2cCallbackState`: `recpeptionNotToMe`, `recpeptionToMe`, `transmitionNotFromMe`, `transmitionFromMe`.
- **Rozpoznawanie obcych transakcji:** `on_recv_done` przychodzi na każdy STOP na szynie (także do innych slave), a `slave_rw` zmienia się tylko przy adresowaniu ESP32. Dlatego:
  - zapis: jak wcześniej — zmiana `I2C0.fifo_st.rx_fifo_end_addr` ⇒ `recpeptionToMe`,
  - ~~odczyt: zmiana `I2C0.fifo_st.tx_fifo_start_addr` (wskaźnik odczytu TX FIFO) ⇒ `transmitionFromMe`~~ — **BŁĘDNE, zastąpione 2026-11-03 (sekcja 3.6):** `tx_fifo_start_addr` na ESP32 nie przesuwa się przy odczycie przez mastera. Teraz odczyt = `slave_rw==1` + stan `...InTransmition` + `status_reg.tx_fifo_cnt == 0`.
  - **Stan TX przesuwa się tylko przy `transmitionFromMe`** → `sInstance->i2cTxStateNextFromISR()` (prywatna metoda `IRAM_ATTR`; od 2026-11-03 zwraca `bool` i sama sprawdza warunki).
- Struktury:
  - `i2cCallbackData { rxToEsp32; rx_fifo_end_addrLast; tx_fifo_start_addrLast; diag_slave_rw; diag_slave_addressed; diag_rx_fifo_end_addr; diag_tx_fifo_start_addrPrev; diag_tx_fifo_start_addr; diag_tx_fifo_end_addr; }`
  - `i2cSlaveRxEvent { i2cCallbackData cbData; uint8_t data[ESP32_SLAVE_RECEIVE_BUFFER_LEN]; }` — **element kolejki `s_receive_queue`** (~72 B, 10 elementów).
  - `static i2cSlaveRxEvent rxEvent` (poziom pliku) — budowany w ISR; `cbData.*_Last` = pamięć ISR między callbackami (dawne globalne `i2cCbData` usunięte).
- ISR: migawka rejestrów do `rxEvent.cbData.diag_*`, klasyfikacja, `memcpy(rxEvent.data, edata->buffer, 52)`, `xQueueSendFromISR(&rxEvent)` → wynik do zmiennej → `assert(queueSendResult == pdTRUE)`.
- Konstruktor: `rxEvent.cbData.rx_fifo_end_addrLast` / `tx_fifo_start_addrLast` inicjowane z rejestrów; `xQueueCreate(10, sizeof(i2cSlaveRxEvent))`.

### 3.3 Task `i2cEngin_slave::i2cSlaveReceive()`
- `data_rd` zostaje **tylko jako bufor roboczy sterownika IDF** (komentarz przy `new`); task go nie czyta.
- `i2cSlaveRxEvent rx_data` — task pracuje wyłącznie na tej kopii; `fakeCommHeader = (i2cFrame_commonHeader*)rx_data.data`.
- Usunięty `memset(data_rd, …)` na początku pętli (kasował ramkę, która już przyszła).
- `assert(fakeCommHeader->dataSize <= ESP32_SLAVE_RECEIVE_BUFFER_LEN)` przed `new`.
- Printfy DIAG: `Reception to me` / `Reception NOT to me` / `Transmition from me` / `Transmition NOT from me` + wiersz `DIAG cb:… slave_rw:… slave_addressed:… rx_fifo_end:… tx_fifo_start:prev->now tx_fifo_end:…`.

### 3.4 Wynik testu na sprzęcie (2026-11-03) — założenie z `tx_fifo_start_addr` OBALONE
Pomiar (log DIAG + rejestry I2C0 w chwili timeoutu):
| Rejestr / pole | Po odczycie 4 B len przez STM32 | Wniosek |
|---|---|---|
| `fifo_st.tx_fifo_start_addr` | `0->0` (bez zmian) | **nie nadaje się** do wykrywania odczytu |
| `fifo_st.tx_fifo_end_addr` | 4 (i nadal 4, gdy w FIFO było już 12 B ramki) | wskaźniki `fifo_st` dla TX nie odzwierciedlają transmisji |
| `status_reg.tx_fifo_cnt` (bity 23:18) | **0** (przed odczytem 4, w fazie danych 12) | **wiarygodny** — używany od 3.6 |
| `status_reg.slave_addressed` | 0 zawsze | nie nadaje się |
| `int_ena` | `0x880` (`trans_complete` bit 7 + `rx_rec_full` bit 11) | ponowne uzbrajanie przez `i2c_slave_receive()` działa |
| `int_raw` | `0x413` (m.in. `ack_err` bit 10) | `ack_err` normalny — master daje NACK na ostatnim bajcie odczytu |
| `fifo_st.rx_fifo_end_addr` | 28→24→20→16 przy ramkach LED 28 B | OK — 5-bitowy wskaźnik przewija się co 32 |

Oczekiwany log poprawnej fazy (potwierdzony): `LEN_DATA_IN_TRANSMITION` → `DIAG Transmition from me` / `DIAG cb:3 slave_rw:1 … tx_fifo_cnt:0` → `LEN_DATA_TRANSMITED` → `PACKAGE_DATA_IN_TRANSMITION` → (`from me`) → `PACKAGE_DATA_TRANSMITED`.

### 3.5 Znane, zgłoszone, NIE poprawione (ESP32)
- ~~Ramka o długości dokładnie 32 B → wskaźnik FIFO mod 32 bez zmian → błędnie `NotFromMe`.~~ — nieaktualne od 3.6 (nie porównujemy wskaźników).
- Okno między callbackiem a ponownym `i2c_slave_receive()` w tasku: transakcja zakończona w tym czasie ginie (sterownik kasuje zaległe flagi przy uzbrajaniu). **Dla TX (odczyt z ESP32) pokryte od 3.6** sprawdzaniem w tasku (`i2cTxStateNextIfReadDone`); dla RX (zapis do ESP32) nadal aktualne. Krótkie, mało prawdopodobne przy odstępach ≥7 ms.
- `if (tempData != nullptr)` po zwykłym `new` to martwy kod (IDF przy braku pamięci robi `abort()`).
- Okno INT#1 → odczyt len nie jest chronione przed obcymi transakcjami (częściowo pokrywa krok 8 planu STM32).
- Printfy DIAG do usunięcia po testach (brak `#ifdef`) — od 3.6 dochodzi `DIAG TX FAIL` przed `assert` w stanie timeout/error.

### 3.6 Zmiany po teście na sprzęcie (2026-11-03, ESP32)
1. **Stack overflow taska „I2C slave rx”** (`app_main.cpp`): stos `128 * 10` (1280 B — w ESP-IDF `xTaskCreate` przyjmuje **bajty**) za mały dla printf DIAG → `128 * 32` (jak „I2C slave tx”). Objaw: `panic_abort` z `***ERROR*** A stack overflow in task I2C slave rx`.
2. **Wykrywanie odczytu z ESP32** (`i2c_engine_slave.cpp`, pełny opis w bloku komentarza nad `i2c_slave_rx_done_callback`):
   - odczyt = JEDNOCZEŚNIE `slave_rw == 1` + `i2cTxSlaveState` ∈ {`lenDataInTransmition`, `packageDataInTransmition`} + `I2C0.status_reg.tx_fifo_cnt == 0`; stary warunek `tx_fifo_start_addr` zakomentowany jako `[BUG]`,
   - `i2cTxWaitForFifoLoaded()`: po `i2c_slave_transmit()` czeka (max 1 ms, `esp_rom_delay_us`) aż `tx_fifo_cnt > 0`, dopiero potem stan `...InTransmition` + INT — puste FIFO w `...InTransmition` oznacza więc „odczytane”, a nie „jeszcze nie załadowane”; brak załadowania → `errorInTransmition`,
   - `i2cTxStateNextIfReadDone()`: zapas w tasku — co tick w `...InTransmition` sprawdza `tx_fifo_cnt == 0 && bus_busy == 0` (callback mógł przepaść, 3.5),
   - spinlock `s_i2cTxStateMux` (ESP32 dwurdzeniowy): sprawdź-i-zmień stanu w ISR i tasku atomowe; timeout ustawiany tylko, gdy stan nadal `...InTransmition` (ISR nie zostanie nadpisany),
   - `static_assert(I2C_SLAVE_PORT == 0)` — callback czyta rejestry `I2C0` na sztywno (master ESP32 jest na porcie 1 / `I2C1` i na to nie wpływa).
   - **Obce transakcje STM32** (z innym układem na tej samej szynie, także między fazą len a fazą danych) są ignorowane: albo stan ≠ `...InTransmition`, albo FIFO ESP32 pełne (obca transakcja go nie opróżni). Jedyne ograniczenie: każda faza musi być odczytana w `ESP32_I2C_TX_TIMEOUT_MS`.
3. **Diagnostyka:** `DIAG cb:` + `tx_fifo_cnt`; przed `assert` w timeout/error linia `DIAG TX FAIL: stateBefore:N retVal status_reg fifo_st int_raw int_ena …` (`stateBefore`: 1 = len, 3 = dane, 0/2 = błąd `i2c_slave_transmit`/ładowania FIFO).
4. **Debugowanie — uwagi praktyczne:**
   - Odczyt rejestrów `I2C0` przez gdb (`p/x I2C0.fifo_st`, `monitor mdw 0x3ff53…`) po panice **zrywał połączenie OpenOCD** — rejestry czytać printfem w kodzie (pełne słowa `.val`). Nigdy nie czytać `fifo_data` (`0x3ff5301C`) — odczyt zdejmuje bajt z RX FIFO.
   - Konsola gdb w Espressif IDE: *Console* → „Display Selected Console” → konsola `gdb` (istnieje tylko w trakcie sesji); wklejone kilka komend naraz bywa gubione — wpisywać pojedynczo. Prywatne pola przez `p p_i2cSlave->i2cTxSlaveState`.
   - Wynik: **komunikacja RX STM32←ESP32 działa w trybie debugowania** (obie fazy `from me`); jedyny timeout pochodził od breakpointu w STM32 (sekcja 2, „Debugowanie”).

### 3.7 Błąd sterownika I2C slave ESP-IDF 5.2 i obejście (2026-11-08, test na sprzęcie)
**Objaw:** po commicie STM32 `fb7d1d4c` (TX STM32→ESP32 zaczął działać) ESP32: kilka × `DIAG Reception to me` (ramki LED 28 B), potem `i2c slave transmit IDLE` (klawisz) i natychmiast `Guru Meditation: Interrupt wdt timeout on CPU0` albo `Double exception` (PC w `_xt_context_save`, ślad spinlocka). Wtórnie: ESP32 się resetuje → impuls startowy INT → STM32 czyta len z pustego FIFO → `RX frame length out of range` + assert (`comunication_esp32i2cComunicationDriver.cpp:228`) — problem 7.5. Inny przebieg: STM32 już stał na tym assercie → ESP32 `TIMEOUT_IN_TRANSMITION` w `lenDataInTransmition` (`tx_fifo_cnt = 4`, `int_raw = 0x2` — na szynie cisza).
**Przyczyna (sterownik, nie nasz kod):** `C:\esp\release-v5.2\esp-idf\components\driver\i2c\i2c_slave.c`, `s_i2c_handle_complete()`: po STOP czyta `t->rcv_fifo_cnt` bajtów (reszta z rozmiaru podanego w `i2c_slave_receive()`, a nie liczba bajtów w FIFO) do `i2c_slave->data_buf[SOC_I2C_FIFO_LEN = 32]`. Przy `receive_size` 52 i krótkiej ramce → przepełnienie → nadpisane pola `i2c_slave_dev_t` za `data_buf`: `trans_data_length` (przerwanie TX FIFO nigdy nie gaśnie → burza przerwań → Interrupt WDT), dalej `callbacks.on_recv_done`, `user_ctx` (→ Double exception). Odbiór działa poprawnie, uszkodzenie wychodzi dopiero przy pierwszej wysyłce ESP32. Wcześniej niewidoczne, bo TX STM32→ESP32 nie działał. IDF 6.x (`esp_driver_i2c`, sterownik slave v2) — kod przepisany, błędu nie ma.
**Obejście (decyzja użytkownika: bez zmian w ESP-IDF; migracja na 6.x — na sam koniec, gdy nic innego nie pomoże):**
- `comunicationProtocol.h` (wspólny): `ESP32_SLAVE_RECEIVE_BUFFER_LEN` **52 → 32** = max długość ramki w **obu kierunkach** (liczba, bo STM32 nie zna `SOC_I2C_FIFO_LEN`); nowe `I2C_SLAVE_DRIVER_RECEIVE_LEN` (= `ESP32_SLAVE_RECEIVE_BUFFER_LEN`) — rozmiar uzbrajania `i2c_slave_receive()`.
- `i2c_engine_slave.cpp`: oba `i2c_slave_receive(..., I2C_SLAVE_DRIVER_RECEIVE_LEN)`; komentarz z opisem błędu; `static_assert`: `ESP32_SLAVE_RECEIVE_BUFFER_LEN == SOC_I2C_FIFO_LEN`, `I2C_SLAVE_DRIVER_RECEIVE_LEN <= SOC_I2C_FIFO_LEN`, `sizeof(i2cFrame_hmiLeds/stepper/btAudio) <= I2C_SLAVE_DRIVER_RECEIVE_LEN` (nowy typ ramki STM32→ESP32 → dopisać `static_assert`).
- Funkcje nadające sprawdzają długość: STM32 `masterTransmitData()` (było), ESP32 `i2cSendDataToTransisionQueue()` (nowe) → `TX frame too long: N > 32` + `//todo` + `assert(0)`.
- Ramki < 32 B działają (sterownik doczytuje śmieci z pustego FIFO, ale w granicach bufora; długość z nagłówka). Ramki > 32 B przy IDF 5.2 niemożliwe (reszta zostaje w FIFO). Gdyby były potrzebne: migracja na IDF 6.x / dzielenie ramek na części ≤ 32 B / stała długość ramki 52 B (dopełnienie).
- Kompilacja ESP32 (`idf.py build`, release-v5.2) i STM32 (`make -j8 all`, GCC 13.3 z CubeIDE 2.0.0): OK.
- **Test na sprzęcie:** wersja z uzbrajaniem odbioru na 32 B (bez zmiany 52 → 32 we wspólnym pliku) — działa (LED + klawisze, bez crasha). Wersja końcowa (32 we wspólnym pliku + sprawdzenie w ESP32) — skompilowana, do wgrania na OBA układy.

**Uwagi przy okazji (2026-11-08):**
- Poprawka „deadlocku” I2C master/slave między rdzeniami (init mastera na Core 1 w `stepperMotorDataParser`, przypięcie tasków slave do Core 0 / parsera silnika do Core 1, semafor `xMotorInitDone`) była TYLKO w folderze `01_ESP32/HMI_and_bluetooth - FIX POPRAWKI` — **folder usunięty 2026-11-08** (commit „przywrócono pracę stepper motorua”); kod do odzyskania z historii gita (folder dodany w `d4bc765d`, np. `git show d4bc765d:"02_Firmware/01_ESP32/HMI_and_bluetooth - FIX POPRAWKI/main/app_main.cpp"`). Aktywny projekt ma `xTaskCreate` bez przypięcia (`app_main.cpp:94-95, 118`) i init mastera w `tasksFunctions.cpp`.
- **`GURU_FIX` usunięty (2026-11-08)**: makro w `hwConfigFile.h` wyłączało I2C master, MCP23008 i silnik krokowy (`pMotor = nullptr`, taski silnika `vTaskDelete`, brak `radioPowerOff`, ramki silnika w parserze nieobsługiwane i NIEzwalniane = wyciek pamięci). Usunięte wszystkie `#ifndef GURU_FIX` (`hwConfigFile.h`, `tasksFunctions.cpp`, `tasksFunctionsStepperMotor.cpp`) — kod silnika przywrócony do stanu sprzed blokady, przypięcia rdzeni bez zmian. Hipoteza: pierwotny crash (Interrupt WDT CPU0 po klawiszu Power, który startował kalibrację) to ten sam błąd sterownika IDF 5.2 co wyżej (klawisz = TX ESP32, potem zapisy STM32 LED/silnik = przepełnienie `data_buf`, następna wysyłka ESP32 = burza przerwań), a nie deadlock (`xKernelLock` trzymany jest przez µs, nie 300 ms). Kompilacja OK (nowe ostrzeżenia z przywróconego kodu: nieużywane `retVal1` `tasksFunctionsStepperMotor.cpp:18`, `handlerTask_stepperMotorDataPasrser` `:98`). **Test na sprzęcie: do zrobienia** — przy Guru Meditation: core dump → ewentualnie przeniesienie przypięć z historii (krok S2).
- Usunięte też nieużywane katalogi: `01_ESP32/HMI_and_bluetooth_I2C_SLAVE_V2`, `01_ESP32/VisualGDB_TEST`, `02_STM32/Radio_main_I2C_SLAVE_V2_5.5.2`.
- **Silnik krokowy po usunięciu `GURU_FIX` — PRZETESTOWANY 2026-11-08: działa, bez crasha** (bez przenoszenia przypięć rdzeni) → hipoteza potwierdzona, dawna diagnoza „deadlocku” była błędna; krok S2 niepotrzebny.

### 3.8 Okno ponownego uzbrojenia odbioru ESP32 (3.5) — przyczyna i poprawka (2026-11-08, test na sprzęcie)
**Objaw:** po usunięciu przez użytkownika `vTaskDelay(3000)` w tasku TX STM32 i (błędnej) zmianie `ESP_I2C_BUS_DELAY` 7 → 2 ms: STM32 na starcie `TX ERROR, dataSize: 28, HAL status: 1, err: 0x04 (NACK)` + assert; ESP32 tylko 1× `Reception to me`, potem `TIMEOUT_IN_TRANSMITION` (`DIAG TX FAIL … status_reg:0x05102000` → `rx_fifo_cnt` = 32 = RX FIFO pełne, `tx_fifo_cnt` = 4).
**Przebieg:** ramka LED #1 odebrana → sterownik wyłącza przerwania odbioru do `i2c_slave_receive()` w tasku → ramka #2 po 2 ms trafia do FIFO, ale zdarzenie przepada (okno 3.5) → ramka #3 przepełnia FIFO → ESP32 NACK → STM32 assert → ESP32 timeout przy klawiszu.
**Przyczyna:** sterownik IDF 5.2 (`i2c_slave.c`, `s_i2c_handle_complete`) **ignoruje wartość zwracaną przez callback `on_recv_done`** — po `xQueueSendFromISR` nie ma przełączenia zadania, task „I2C slave rx” budzi się dopiero przy następnym ticku ESP32 (`CONFIG_FREERTOS_HZ=100` → do **10 ms**), niezależnie od priorytetu. Stare 7 ms działało „zwykle”; z `vTaskDelay(3000)` ramki szły co 3 s, więc okno nie miało znaczenia.
**Zmiany:**
- **Y1** (ESP32 `i2c_engine_slave.cpp`, koniec `i2c_slave_rx_done_callback`): `if (high_task_wakeup == pdTRUE) portYIELD_FROM_ISR();` — na ESP32 (Xtensa) ustawia tylko flagę, przełączenie przy wyjściu z przerwania → odbiór uzbrajany zaraz po ISR.
- **B1** (ESP32 `app_main.cpp:94`): task „I2C slave rx” priorytet `+2` → `+3` (najwyższy z tasków I2C), działa razem z Y1.
- **B2** (`ESP_I2C_BUS_DELAY` 7 → 2) — **WYCOFANE (R1)**, wartość 7 ms z komentarzem: zmniejszać dopiero osobnym krokiem, po teście szybkich serii ramek (Y1 + B1 skracają okno do µs).
- Test 2026-11-08 (Y1 + B1 + 7 ms, bez `vTaskDelay(3000)`): **działa** — jak się okazało, przypadkiem (patrz sprostowanie niżej).

**SPROSTOWANIE (2026-11-08, później) — właściwa przyczyna NACK na starcie:** błąd wracał (po Y1 i A1) przy starcie: ESP32 tylko 1× `Reception to me`, potem NACK na STM32 (`TX ERROR … err: 0x04`), przy klawiszu timeout ESP32. Przyczyna w kodzie: task „I2C slave rx” po uzbrojeniu odbioru wołał `esp32i2cBusInitialised()` = INT low + **`vTaskDelay(100 ms)`** + INT high. STM32 zaczyna zapisy już na **zboczu opadającym** impulsu (menu → kilka ramek LED co 7 ms) → ramka #1 odebrana, ale task rx śpi 100 ms i nie uzbraja ponownie odbioru → kolejne ramki zapychają RX FIFO (`rx_fifo_cnt` = 32) → NACK. Ujawniło się po usunięciu `vTaskDelay(3000)` w TX STM32 (wcześniej pierwsza ramka szła 3 s po starcie, daleko po impulsie). Przerywane, bo zależy od czasu tworzenia menu STM32.
- **G1** (`i2c_engine_slave.cpp`): wywołanie `esp32i2cBusInitialised()` przeniesione z `i2cSlaveReceive()` (po `i2c_slave_receive`) na początek `i2cSlaveTransmit()` (przed `while (1)`). Task rx (+3, tworzony w `app_main` przed tx) uzbraja odbiór i od razu obsługuje ramki także w trakcie impulsu; task tx może spać 100 ms (na starcie nic nie wysyła). Impuls 100 ms zostaje (pod 7.5 opcja 4), moment zbocza praktycznie bez zmian. **Przetestowane: działa.**
- **F1** (zerowanie RX FIFO `i2c_ll_rxfifo_rst(&I2C0)` w callbacku po zapisie do ESP32 — hipoteza „rozjechany licznik FIFO po nadmiarowym odczycie sterownika”) — wprowadzone i **WYCOFANE** (nie pomogło; leczyło objaw). Nadmiarowy odczyt pustego FIFO przez sterownik IDF 5.2 nadal istnieje, ale w praktyce nie psuje kolejnych ramek.
- **Y1 i B1 zostają** (poprawne: bez yield task rx budzi się dopiero przy ticku ESP32 do 10 ms), ale nie były właściwą przyczyną. `ESP_I2C_BUS_DELAY` zostaje 7 ms.
- Wniosek na przyszłość: żaden task, który ponownie uzbraja odbiór I2C slave, nie może blokować się (`vTaskDelay`, długi printf przed `i2c_slave_receive`) — odbiór musi być uzbrojony przed kolejnym zapisem STM32 (7 ms).

**Inne zmiany (ten sam commit):**
- STM32 `hwConfigFile.h`: usunięte nieużywane `ESP32_INTERRUPT_REQUEST_COUNTING_SEMAPHORE_MAX 21` (licznik INT ma `esp32InterruptRequestCountingSemaphore_MAX = 25` w driverze).
- ESP32 **N1**: alokacje `pData` → `new (std::nothrow) char[...]` + `#include <new>` (`i2c_engine_slave.cpp` odbiór ramki, `tasksFunctions.cpp` klawiatura, `bt_audio.cpp` BT). `std::nothrow` wybiera przeciążenie `operator new[](size_t, const std::nothrow_t&) noexcept`, które przy braku pamięci zwraca `nullptr`; zwykłe `new` rzuca `std::bad_alloc`, a przy wyłączonych wyjątkach (ESP32 `CONFIG_COMPILER_CXX_EXCEPTIONS` off, STM32 `-fno-exceptions`) kończy się `abort()` — wtedy `assert(pData)` po `new` jest martwe. Zasada alokacji w `comunicationProtocol.h` — `nothrow` dla obu układów. Startowe `new` obiektów (ESP32: `i2cEngin_slave`, `NVS`, `LEDS_BACKLIGHT`, kolejki, `KEYBOARD`, BT; STM32: `menu_myList.cpp:52`) zostają zwykłe (brak pamięci przy starcie → reset jest OK).
- **N2** (`comunicationProtocol.h`, gałąź `#else` ESP32): `typedef struct i2cFrame_transmitQueue{` — nazwana struktura (C++20: anonimowa struktura nazwana przez typedef nie może mieć inicjalizatorów pól; ostrzeżenie clangd). Typ i użycie bez zmian.
- Plik `main/bt_audio/bt_audio — kopia.cpp` — przypadkowa kopia, niekompilowana (do usunięcia przez użytkownika).

**Inne zmiany 2026-11-08 (ten sam commit):**
- STM32 `tasksFunctions.cpp`: usunięte `vTaskDelay(pdMS_TO_TICKS(3000))` w `i2cMasterTransmitToSlaveDataTask` (użytkownik); komentarz `//ta funkcja nie działa` → opis; usunięte 2 zakomentowane `HAL_I2C_Master_Transmit_DMA`; „(nasza ścieżka timeoutu)” → „dziś nieużywane…”.
- STM32 **E1**: `i2cMaster::getI2cError()` (`HAL_I2C_GetError`, opis bitów: 0x01 BERR, 0x02 ARLO, **0x04 AF = NACK**, 0x08 OVR, 0x10 DMA, 0x20 TIMEOUT); `TX ERROR` / `RX ERROR` w driverze ESP32 drukują `err: 0x..`.
- STM32: usunięty martwy komentarz `//const uint8_t ESP_I2C_BUS_DELAY = 7;` (`comunication_esp32i2cComunicationDriver.h`).
- ESP32 **C1/C2** (`tasksFunctionsStepperMotor.cpp`): usunięta martwa gałąź `else` z `if (pMotor->QueueMessagesWaiting() == 0)` (zakomentowane `vTaskSuspend`, nic nie robiła) i zbędne `retVal1` — znikają 2 ostrzeżenia. Plik w Windows-1250 + CRLF — edytować bajtowo, nie przekodowywać.
- **`assert` na STM32 — jak działał do 3.9:** newlib `__assert_func()` drukuje `assertion … failed` bezpośrednio (`_write` → `__io_putchar` w `printfRedirect.c`), potem `abort()` → `_exit()` (`syscalls.c` — plik generowany przez CubeIDE, NIE edytować) → `while(1)`. Komunikat `feedPrintf` sprzed assert idzie przez `printfTask` (prio 0) — wypisze się tylko, jeśli task z assertem ma prio 0.

### 3.9 Priorytety tasków I2C STM32 + własny `__assert_func` (2026-11-08, przetestowane)
- **A1** (`Core/Src/SileliS_code/tasksFunctions.cpp`, tworzenie tasków): `i2cReceive` i `i2cTransmit` `tskIDLE_PRIORITY` → **+2**, `i2cMastRecQue, Pars` (parser) → **+1**, `esp32IntReq` bez zmian (+1); menu, `periTimeout`, `Printf`, `initTasks` zostają 0. Cel: odczyt z ESP32 w `ESP32_I2C_TX_TIMEOUT_MS` (500 ms) niezależnie od obciążenia (menu, przyszły si4689); taski I2C prawie cały czas czekają (kolejki, semafor, `vTaskDelay(1)` w `waitForTransferEnd`).
- **A2** (`Core/Src/SileliS_code/printfRedirect.c`): własne `__assert_func()` zastępuje funkcję z newlib — linker dołącza obiekt z `libc.a` tylko dla nierozwiązanych symboli, więc bierze naszą definicję (to NIE jest `weak`; przy konflikcie byłby błąd `multiple definition` przy budowaniu). Działa dla wszystkich `assert()` w C i C++ bez zmian w miejscach wywołania. Wypisuje `assertion "…" failed: file …, line …, function …` (stderr → `__io_putchar`, blokująco), w tasku (`__get_IPSR() == 0`, scheduler działa) obniża priorytet do `tskIDLE_PRIORITY` (żeby `printfTask` wypisał komunikat `feedPrintf` sprzed assert — bez tego task +2 w `while(1)` zagłodziłby `printfTask`), potem `while(1)`. Kolejność w logu: najpierw linia assert, potem linia z kolejki. `syscalls.c` nietknięty. Sprawdzone w `Radio_main.elf`: użyta nasza wersja.
- **`configASSERT()` FreeRTOS** (`FreeRTOSConfig.h:158`: `taskDISABLE_INTERRUPTS(); for(;;);`) — NIE dotyczy, nic nie wypisuje (ewentualnie osobny temat).
- Test 2026-11-08 (użytkownik): działa.
- Flashowanie ESP32 przez J-Link przy `adapter speed 12000` przerwało się w połowie (`[esp32.cpu1] Examination failed`) → pusta partycja aplikacji (`invalid magic byte`). Przy 2000 kHz bez błędów. `.launch` ma nadal 12000 (propozycja zmiany na 2000 — czeka na decyzję).
- Espressif-IDE, nowy workspace: trzeba wybrać ESP-IDF w ESP-IDF Manager (inaczej „No Toolchain found for Target esp32” / „No board selected”), potem zbudować projekt przed Debug (inaczej `build\null not found`).
- Core dump ESP32 z UART: `esp_coredump info_corefile -t b64 -c core.b64 build/HMI_and_bluetooth.elf` (python z venv release-v5.2, `IDF_PATH` ustawiony, GDB 16.3 w PATH, `--gdb-timeout-sec 20`); zmienne lokalne: `-s core.elf` + `xtensa-esp32-elf-gdb -batch -ex "frame 3" -ex "info locals"`.

---

## 4. STM32 — stan obecny (przed zmianami)

Łańcuch (bez zmian architektury):
`EXTI PB5` → `HAL_GPIO_EXTI_Callback` (`tasksFunctions.cpp`) → semafor zliczający (`esp32_i2cComunicationDriver`) → task `esp32I2cIntrrruptRequest` → `setI2cAdressToAdressQueue(0x3C)` → task `i2cMasterReceiveFromSlaveDataTask` → `pESP32->masterReceiveData()` → kolejka parsera → `i2cMasterParseReceivedData` → `parseReceivedData()` (CRC, commandGroup).

**Problem:** `masterReceiveData()` na KAŻDY token czyta obie fazy (4 B, `vTaskDelay(7 ms)`, `dataSize` B) i nie czeka na koniec DMA (tylko ślepy delay). Z nowym ESP32 token #2 czyta śmieci jako len.

Kluczowe pliki STM32 (`02_Firmware/02_STM32/Radio_main/Core/`):
- `Src/SileliS_code/tasksFunctions.cpp` — taski, `HAL_GPIO_EXTI_Callback`, uchwyty tasków (m.in. `taskHandle_i2cMasterReceiveFromSlaveDataTask`).
- `Src|Inc/SileliS_code/comunication_esp32i2cComunicationDriver.*` — `masterReceiveData()`, `masterReceiveFromESP32_DMA()`, `giveESP32I2CInterfaceTime()`, semafor zliczający.
- `Src|Inc/SileliS_code/comunication_i2cEngine.*` — `i2cMaster`, `I2C_Master_Receive_DMA()`, `getI2cAdressFromAdressQueue()`, `i2cMasterSemaphoreTake/Give()` (binarny semafor szyny).
- `Inc/hwConfigFile.h` — `ESP_I2C_BUS_DELAY 7`.

Fakty sprzętowe/HAL (sprawdzone):
- I2C1: RX = **DMA1_Stream0** (`hdma_i2c1_rx`), TX = DMA1_Stream1; IRQ I2C1_EV/ER i DMA prio 6; EXTI prio 5; `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5` → `...FromISR` dozwolone.
- Brak D-Cache, heap FreeRTOS w AXI SRAM (0x24000000) → bufory z `new` OK dla DMA.
- `USE_HAL_I2C_REGISTER_CALLBACKS 0` → callbacki przez `__weak` (nadpisanie przez zwykłą definicję w `.cpp`; linkage C z nagłówka HAL).
- Koniec odbioru: `DMA1_Stream0_IRQHandler` → `I2C_DMAMasterReceiveCplt` (tylko włącza przerwanie STOP) → STOPF → `I2C1_EV_IRQHandler` → `I2C_ITMasterCplt` → **`HAL_I2C_MasterRxCpltCallback`** (po STOP — to samo zdarzenie, na które reaguje ESP32).
- Błędy → `HAL_I2C_ErrorCallback`; po `HAL_I2C_Master_Abort_IT()` → `HAL_I2C_AbortCpltCallback` (NIE ErrorCallback).
- `Master_*` → `MasterRx/TxCpltCallback`; `Mem_*` → `MemRx/TxCpltCallback` (wybór po `hi2c->Mode`).
- W projekcie nikt nie używa task notifications (`xTaskNotify*`, `osThreadFlags*`).
- STM32CubeIDE: Outline dla `stm32h7xx_hal_i2c.c` wymaga wyłączenia Scalability mode (Preferences → C/C++ → Editor → Scalability, próg np. 20000).

---

## 5. PLAN dla STM32 — maszyna stanów `i2cReceiveStates` (aktualizacja 2026-10-28)

> Stary plan z `xTaskNotify`/`rxPhase`/`esp32FrameInProgress` został **porzucony**. Użytkownik napisał szkic oparty o istniejący enum
> `esp32_i2cComunicationDriver::i2cReceiveStates` (`idle, lenDataInReception, lenDataReceived, packageDataInReception, packageDataReceived,
> errorpackageDataInReception, timeoutpackageDataInReception` — **nazw nie zmieniać**) i `getI2cRxMasaterState()`, używany w tasku
> `esp32I2cIntrrruptRequest` (`tasksFunctions.cpp`): czeka (`vTaskDelay(1)`) gdy stan = `lenDataInReception`/`packageDataInReception`/`packageDataReceived`,
> a przy `idle`/`lenDataReceived` wrzuca adres 0x3C do kolejki odczytu. Ten kierunek jest obowiązujący.

### 5.1 Idea
- **1 token (1 impuls INT) = 1 faza** = jedno wywołanie `masterReceiveData()` w tasku `i2cMasterReceiveFromSlaveDataTask`.
- `masterReceiveData()` robi `switch` po stanie, startuje DMA i **czeka na koniec** (pętla `vTaskDelay(1)` + timeout), więc semafor szyny I2C jest zwalniany po faktycznym STOP.
- Stan przesuwają callbacki HAL (ISR) przez nowe metody drivera; task ustawia stan „w trakcie” **PRZED** startem DMA.
- Długość ramki z fazy len musi przetrwać do tokenu #2 → pole klasy (ramka `I2CFrameToReadFromSlave` jest nadpisywana przy każdym `getI2cAdressFromAdressQueue`). Bufor danych — wprost `dataFrame->pData` (faza danych mieści się w jednym wywołaniu; `pRxData` NIE jest potrzebne).

### 5.2 Wszystkie miejsca zmiany stanu
| Gdzie | Przejście |
|---|---|
| task, `masterReceiveData` `case idle` (token #1) | `dataSizeToRead = 0`; `idle → lenDataInReception` (przed startem DMA 4 B do `dataSizeToRead`) |
| ISR `rxCompleteFromISR()` (z `HAL_I2C_MasterRxCpltCallback`) | `lenDataInReception → lenDataReceived`; `packageDataInReception → packageDataReceived` (TYLKO gdy stan `…InReception`) |
| ISR `rxErrorFromISR()` (z `HAL_I2C_ErrorCallback`) | `…InReception → errorpackageDataInReception` (TYLKO gdy stan `…InReception`) |
| task, `masterReceiveFromESP32_DMA` | start ≠ `HAL_OK` → `errorpackageDataInReception`; timeout (`ESP32_I2C_RX_TIMEOUT_MS`) → **najpierw** `timeoutpackageDataInReception`, **potem** `I2C_Master_Abort()` (spóźniony callback nic już nie zmieni); po czekaniu stan `errorpackageDataInReception` (z ISR) → wykrywa task |
| task, `case lenDataReceived` (token #2) | len poza `sizeof(i2cFrame_commonHeader) … ESP32_SLAVE_RECEIVE_BUFFER_LEN` → `errorpackageDataInReception`; `new (std::nothrow) char[len]` = nullptr → printf `"error with memory allocation."` + `//todo` + `assert`; `lenDataReceived → packageDataInReception` (przed DMA) |
| task, po `packageDataReceived` | `dataFrame->pData/dataSize` ustawione, `dataSizeToRead = 0`, stan → `idle` |
| task `esp32I2cIntrrruptRequest` | tylko czyta stan; `idle`/`lenDataReceived` → adres do kolejki; `default:` → printf + `//todo` + `assert(0)` (zabezpieczenie) |

**WAŻNE (ustalone 2026-10-29):** `printf` + `//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32` + `assert(0)` MUSZĄ być **w miejscu wykrycia błędu w tasku odbioru** (kroki 7 i 8), a NIE w `esp32I2cIntrrruptRequest` — ten `switch` wykonuje się tylko po nowym impulsie INT, a ESP32 po błędzie/timeoucie przechodzi w stan końcowy `timeoutInTransmition` i INT już nie wyśle (błąd przeszedłby niezauważony). W ISR (`rxErrorFromISR`) tylko zmiana stanu — bez printf (`feedPrintf` nie jest ISR-safe); task wykrywa błąd w pętli czekania ≤1 ms później.
- `feedPrintf` tylko kolejkuje tekst; `assert` → `_exit()` → `while(1)` (`syscalls.c:64`) z włączonymi przerwaniami. Task odbioru i `printfTask` mają ten sam priorytet 0 → time slicing → komunikat się wypisze. Użytkownik **NIE chce** `vTaskDelay` przed `assert`.

### 5.3 Decyzje użytkownika (PODJĘTE)
- Czekanie na koniec DMA: **pętla `vTaskDelay(1)`** sprawdzająca stan (nie `xTaskNotify`).
- Abort: **nowa metoda `i2cMaster::I2C_Master_Abort()`** (`HAL_I2C_Master_Abort_IT(p_hi2c1, …)`).
- Po błędzie/timeoucie: **zostajemy w stanie błędu**, sygnalizacja `printf` + `//todo` + `assert(0)` **w miejscu wykrycia** (patrz „WAŻNE” w 5.2). Obsługi błędów (recovery) na razie NIE ma — użytkownik zrobi ją, gdy błędy zaczną występować.
- Komentarz przy każdym takim `assert` (STM32 i ESP32): `//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32`.
- **ESP32 też dostaje `assert(0)`** w stanach `timeoutInTransmition`/`errorInTransmition` (po istniejącym printf). W ESP-IDF zwykły `assert` jest właściwy (asercje włączone w `sdkconfig`: `CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_ENABLE=y`); po panice ESP32 domyślnie się **resetuje** (nie wisi).
- **Wspólny timeout:** `ESP32_I2C_TX_TIMEOUT_MS 500` w `comunicationProtocol.h` (plik wspólny); ESP32 `tx_timeout_ms = ESP32_I2C_TX_TIMEOUT_MS`; STM32 `ESP32_I2C_RX_TIMEOUT_MS (ESP32_I2C_TX_TIMEOUT_MS + 100)` w `comunication_esp32i2cComunicationDriver.h` — STM32 czeka dłużej niż ESP32 (ESP liczy od INT, STM od startu DMA). Margines +100 ms zaakceptowany.
- `len` + stan → **struktura** w `esp32_i2cComunicationDriver`.
- `assert` przy braku pamięci (`"error with memory allocation."`) zostaje, ale przeniesiony do `new` w fazie danych (`case lenDataReceived`); w tasku po fazie len `pData == nullptr` jest poprawne.
- Usunąć `#ifdef STM32_2_ESP32_I2C_IN_SEQUENCE` i CAŁY martwy kod `_inSequence` (także `i2cMaster::I2C_Master_Receive_DMA_inSequence` i komentarz `comunicationProtocol.h:15`) — zaakceptowane.
- `switch` w `esp32I2cIntrrruptRequest`: `idle`/`lenDataReceived` → kolejka, `default:` z `printf` + `//todo` + `assert(0)`.
- `HAL_GPIO_EXTI_Callback` — dodać sprawdzenie pinu i `pESP32 != nullptr`.
- **Sposób pracy (wymóg użytkownika):** dla każdego kroku Claude pokazuje **diff w czacie**, czeka na „akceptuję krok X”, dopiero wtedy `Edit`. Kroki wieloplikowe/wielomiejscowe → kilka `Edit` (1a, 1b…).

### 5.4 Kroki — **WSZYSTKIE WPROWADZONE 2026-11-03, commit `b4e6bba9` „komunikacja stm32<->esp32 poprawiona (na sucho)”**
Odstępstwa od pierwotnego opisu: krok 7 ma dodatkowo `taskENTER_CRITICAL()` przy ustawianiu stanu timeout (wyścig z callbackiem); krok 8 ma `break;` po każdym `assert(0)` (ochrona przy `NDEBUG`); krok 10 — w `default:` jest też printf. Kompilacja STM32 i ESP32 po krokach: OK, bez nowych ostrzeżeń (zniknęły `#error` z `_inSequence`, `-Wswitch`, nieużywane `dataSize`).
| # | Plik | Zmiana |
|---|---|---|
| 1 | `comunication_esp32i2cComunicationDriver.h` | **1a:** pod `enum i2cReceiveStates` (public): `struct i2cRxMasterControl { volatile i2cReceiveStates state = i2cReceiveStates::idle; size_t dataSizeToRead = 0; };` + `static_assert(sizeof(size_t) == 4, …)`. **1b:** po `getI2cRxMasaterState()` deklaracje `void rxCompleteFromISR(void);` / `void rxErrorFromISR(void);`; w `private` `volatile i2cReceiveStates i2cRxMasaterState` → `i2cRxMasterControl i2cRxMaster;` (projekt nie kompiluje się między krokiem 1 a 2) |
| 2 | `.cpp` | `getI2cRxMasaterState()` zwraca `i2cRxMaster.state` |
| 3 | `.cpp` | definicje `rxCompleteFromISR()` / `rxErrorFromISR()` (przejścia z 5.2, tylko ze stanów `…InReception`) |
| 4 | `comunication_i2cEngine.h/.cpp` | `i2cMaster::I2C_Master_Abort()` |
| 5a | `comunicationProtocol.h` (wspólny) | `#define ESP32_I2C_TX_TIMEOUT_MS 500` |
| 5b | ESP32 `main/i2c_engine/i2c_engine_slave.h:45` | `const int tx_timeout_ms = ESP32_I2C_TX_TIMEOUT_MS;` |
| 5c | `comunication_esp32i2cComunicationDriver.h` | `#define ESP32_I2C_RX_TIMEOUT_MS (ESP32_I2C_TX_TIMEOUT_MS + 100)` |
| 6 | `tasksFunctions.cpp` | `HAL_I2C_MasterRxCpltCallback` + `HAL_I2C_ErrorCallback` (warunki `hi2c == &hi2c1`, `pESP32 != nullptr`) → metody z kroku 3 |
| 7 | `.cpp` `masterReceiveFromESP32_DMA()` | start DMA (≠`HAL_OK` → stan error) → pętla `vTaskDelay(1)` aż stan ≠ `…InReception` lub `ESP32_I2C_RX_TIMEOUT_MS` → timeout: stan + `I2C_Master_Abort()`; error/timeout → printf (faza, stan, `dataSizeToRead`) + `//todo` + `assert(0)`; `giveESP32I2CInterfaceTime()` |
| 8 | `.cpp` `masterReceiveData()` | `switch` po stanie wg 5.2; len poza zakresem / `new == nullptr` → printf + `//todo` + `assert(0)`; `== HAL_OK`; usunąć nieużywane `uint8_t dataSize` |
| 9 | `tasksFunctions.cpp:117-120` | usunąć `else if (pData == nullptr)` → printf + `assert` (przeniesione do kroku 8); `if (pData != nullptr)` → parser zostaje |
| 10 | `tasksFunctions.cpp` `esp32I2cIntrrruptRequest` | `case idle`/`lenDataReceived` → kolejka; `default:` → printf + `//todo` + `assert(0)` (usuwa `-Wswitch`) |
| 11 | `tasksFunctions.cpp` `HAL_GPIO_EXTI_Callback` | `switch (GPIO_Pin) { case esp32i2cInterruptReqest_Pin: if (!esp32I2cInitialised) …=true; else if (pESP32 != nullptr) pESP32->incrementInterruptRequestCountingSemaphore(); break; default: break; }` — bez printf (feedPrintf nie jest ISR-safe) |
| 12 | driver `.h/.cpp`, `tasksFunctions.cpp:101-106`, `comunication_i2cEngine.h:44/.cpp:198`, `comunicationProtocol.h:15` | usunięcie `STM32_2_ESP32_I2C_IN_SEQUENCE`, `masterReceiveDataInSequence`, `masterReceiveFromESP32_DMA_inSequence`, `i2cMaster::I2C_Master_Receive_DMA_inSequence`, komentarz o makrze |
| 13 | ESP32 `main/i2c_engine/i2c_engine_slave.cpp:436-450` | w `case timeoutInTransmition`/`errorInTransmition` po printf stanu: `//todo` + `assert(0)` |

Przyszłość: TX i przebudowa RX na wspólny mechanizm `waitForTransferEnd()` — patrz sekcja 7.3 (plan W1–W7). Pomysł „TX przez `MasterTxCpltCallback`” został PORZUCONY.

### Znane, zgłoszone, NIE poprawione (STM32)
- `i2cMasterTransmitToSlaveDataTask` ma `vTaskDelay(3000)` w każdej iteracji (wysyłka LED/silnik opóźniona do 3 s).
- `ESP32_INTERRUPT_REQUEST_COUNTING_SEMAPHORE_MAX 21` nieużywane (sterownik ma własne `= 25`; przy 2 tokenach/ramkę ≈ 12 ramek bufora).
- Task odbioru ma `tskIDLE_PRIORITY` (0), task INT `+1`.
- `setI2cAdressToAdressQueue()` woła `QueueSendFromISR` z kontekstu taska — na Cortex-M działa (nie blokuje), użytkownik zostawia.

---

## 6. ZROBIONE 2026-10-28 (commit „pData: inicjalizacja i alokacja char[]”)

### 6.1 `i2cFrame_transmitQueue` — inicjalizacja pól (`01_ESP32/.../main/common/comunicationProtocol/comunicationProtocol.h`, wspólny dla obu projektów)
- Gałąź `#ifdef I2C_STM32_TO_ESP32_ROLE_MASTER` (STM32): konstruktor zastąpiony inicjalizatorami `slaveDevice7bitAddress = 0, dataSize = 0, pData = nullptr` (efekt ten sam).
- Gałąź `#else` (ESP32): dodane `dataSize = 0, pData = nullptr` (wcześniej BRAK inicjalizacji; dziś każde miejsce ESP32 ustawia oba pola ręcznie — sprawdzone, ale kruche).
- Nad `pData` komentarz z **zasadą alokacji**: `pData` ZAWSZE `new (std::nothrow) char[rozmiar]` (STM32) / `new char[rozmiar]` (ESP32), nigdy `new T`/`malloc`, bo zwalnia wyłącznie `i2cQueue4DynamicData::QueueDeleteDataFromPointer()` przez `delete[] static_cast<char*>(pData)`; `pData == nullptr` = ramka bez danych (bezpieczne `delete[]` przy pełnej kolejce).
- Clang w edytorze zgłasza `-Wnon-c-typedef-for-linkage` dla `#else` (anonimowy `typedef struct` z inicjalizatorami). **GCC ESP-IDF tego NIE zgłasza.** Propozycja (niezaakceptowana): `typedef struct i2cFrame_transmitQueue{`.

### 6.2 Niezgodne `new` / `delete[]` (STM32)
- `device_ledsController.cpp`: `new (std::nothrow) i2cFrame_hmiLeds` → `new (std::nothrow) char[sizeof(i2cFrame_hmiLeds)]`.
- `device_StepperOptoPowerOFF.cpp`: `new (std::nothrow) i2cFrame_stepper` → `new (std::nothrow) char[sizeof(i2cFrame_stepper)]`.
- Wcześniej działało (proste struktury, wspólny malloc/free newlib, brak nadpisanego `operator new`), ale było UB.

### 6.3 Kompilacja (po zmianach, 2026-10-28)
- STM32: GCC 13.3 z `C:/ST/STM32CubeIDE_2.0.0/.../gnu-tools-for-stm32.13.3.rel1...` + make z CubeIDE, `make -j8 all` w `Radio_main/Debug` → **OK**, 0 błędów, brak nowych ostrzeżeń.
- ESP32: profil EIM `C:\Espressif\tools\Microsoft.v5.2.8.PowerShell_profile.ps1` → `idf.py build` → **OK**, 0 błędów.
- `Debug/` i `build/` są w `.gitignore`.

---

## 7. Gdzie jesteśmy / co dalej (START TUTAJ)

### 7.1 Stan na 2026-11-04
| Commit | Co |
|---|---|
| `0eb76373` | `pData`: inicjalizacja pól `i2cFrame_transmitQueue`, alokacja `char[]` (sekcja 6) |
| `400d8a48` | ten plik: zaakceptowany plan kroków 1–13 |
| `b4e6bba9` | **RX STM32←ESP32 (kroki 1–13)** — „komunikacja stm32<->esp32 poprawiona (na sucho)” |
| `49d5d4b3`, `b5e9dee7`, `9a8d7f3a` (na `main`) | J-Link `adapter speed`; **ESP32: stos „I2C slave rx”, wykrywanie odczytu przez `tx_fifo_cnt` (3.6)**; „GOLDEN SAMPLE” — **RX STM32←ESP32 działa w trybie debugowania** |
| `fb7d1d4c` (2026-11-04) | **`waitForTransferEnd` w `i2cMaster` dla TX i RX (7.3)** |
| (2026-11-08, „GOLDEN SAMPLE - poprawiono długośc buffora ESP32 i2c slave…”) | **obejście błędu sterownika I2C slave IDF 5.2: max ramka 32 B w obu kierunkach, sprawdzenie długości w funkcjach nadających (3.7)** |
| `53f250f4` (2026-11-08, „przywrócono pracę stepper motorua”) | **usunięty `GURU_FIX` — I2C master, MCP23008, silnik krokowy przywrócone (przetestowane); usunięte nieużywane katalogi (m.in. `FIX POPRAWKI`) (3.7)** |
| `4f0d406f` (2026-11-08) | **okno uzbrojenia odbioru ESP32: `portYIELD_FROM_ISR` w callbacku + prio „I2C slave rx” +3; bez `vTaskDelay(3000)` w TX STM32; `getI2cError()`; sprzątanie (3.8)** |
| `abf1ddcb` (2026-11-08, „GOLDEN SAMPLE”) | **priorytety tasków I2C STM32 (+2/+1) i własny `__assert_func` (3.9)** |
| (2026-11-08, ten commit) | **impuls startowy ESP32 z taska tx (G1) — właściwa przyczyna NACK na starcie; `new (std::nothrow)`; nazwana struktura; usunięta nieużywana stała (3.8 sprostowanie)** — przetestowane |

Co działa w kodzie (stan po 7.3):
- RX (odczyt STM32 z ESP32): 1 impuls INT = 1 token = 1 wywołanie `masterReceiveData()` = 1 faza: `idle` → odczyt 4 B długości do `i2cRxMaster.dataSizeToRead` → `lenDataReceived`; kolejny INT → `new char[len]`, odczyt ramki → `idle`, ramka do parsera. Odczyt przez `i2cMaster::I2C_Master_Receive_DMA()`, która wraca po końcu transmisji; stan przesuwa task na podstawie wyniku (bez callbacków HAL).
- TX (STM32→ESP32: LED, silnik): `masterTransmitData()` → `i2cMaster::I2C_Master_Transmite_DMA()` (wraca po końcu wysyłki) → `giveESP32I2CInterfaceTimeForDataProcessing()` (7 ms dla ESP32 po końcu). `masterTransmitData()` zwraca `HAL_StatusTypeDef`.
- Błędy → `feedPrintf` + `//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32` + `assert(0)` (STM32 staje). ESP32 przy timeout/error: printf + `assert(0)` → panic → **reset** ESP32.

### 7.2 Test na sprzęcie (użytkownik, w domu) — czego oczekiwać
> **2026-11-03:** test wykonany w trybie debugowania — RX działa po zmianach z 3.6 (z kodem STM32 `b4e6bba9`).
> **2026-11-04 — DO ZROBIENIA:** test z kodem po 7.3 (STM32 z commitu „poprawiono mechanizm…”, ESP32 bez zmian): RX klawisze jak wcześniej + **TX LED/silnik** (`//ta funkcja nie działa` w `tasksFunctions.cpp` przy `masterTransmitData`), bez debuggera, wiele klawiszy z rzędu. Na UART STM32 nie może być: `STALL`, `TX BUSY`/`RX BUSY`, `TX START ERROR`/`RX START ERROR`, `TX ERROR`/`RX ERROR`, `RX unexpected state`. W punkcie 3 poniżej kryterium `tx_fifo_start` jest nieaktualne — patrz log w 3.4.

1. Wgrać oba firmware (STM32 z ostatniego commitu, ESP32 z commitu po 3.6).
2. Start: ESP32 daje impuls INT inicjalizacji; STM32 wypisuje „waiting for esp32 i2c bus to be initialized.” aż go dostanie.
3. Naciśnięcie klawisza HMI (ESP32 → STM32, ramka klawiatury):
   - UART STM32: **brak** komunikatów `RX ERROR` / `RX TIMEOUT` / `frame length out of range` / `error with memory allocation` / `unexpected state`; klawisz trafia do menu (`keyboardToRadioMeny`).
   - ESP32 (DIAG): 2× „Transmition from me” na ramkę (len, potem dane), `tx_fifo_start` przesuwa się o 4, potem o `dataSize` (tabela w 3.4); brak `TIMEOUT_IN_TRANSMITION`.
4. Jeśli STM32 stanie na `assert` — komunikat przed nim mówi: `RX ERROR … Size: 4` = faza len, inny = faza danych; `HAL status`; `dataSizeToRead`. Zawieszona transmisja: co 1 s `STALL addr:… state ISR SCL SDA DMA cnt | <przyczyna>`, po 10 s `assert`. ESP32 i tak zrobi swój timeout i reset po 500 ms.
5. Znane ryzyka do obserwacji: okno ponownego uzbrojenia `i2c_slave_receive` po stronie ESP32 dla ZAPISU do ESP32 (3.5) — chronione 7 ms po każdej transmisji.

### 7.3 ZROBIONE 2026-11-04: wspólny `waitForTransferEnd()` w `i2cMaster` dla TX i RX (NIEtestowane na sprzęcie)
**Problem, który rozwiązuje:** `masterTransmitData()` startował DMA i robił ślepe `vTaskDelay(7)`, więc task TX oddawał semafor szyny i kasował bufor, zanim wysyłka się skończyła. Kolejność `Take → funkcja drivera → Give → delete` w taskach była dobra — zła była funkcja, która wracała przed końcem transmisji.
**Ustalenia z HAL (`stm32h7xx_hal_i2c.c`):** po końcu transmisji HAL ustawia `State = HAL_I2C_STATE_READY` (sukces: l. 6181 TX / 6216 RX, PRZED callbackiem; błąd: l. 6566, `ErrorCode` l. 6539) — także dla `Mem_*`. HAL DMA/IT nie ma timeoutu. `HAL_I2C_Master_Transmit/Receive_DMA` zwraca `HAL_BUSY`, gdy flaga `I2C_FLAG_BUSY` (l. 1967) albo uchwyt nie `READY`; `HAL_ERROR` przy błędzie startu DMA (l. 2031–2072). `HAL_I2C_ER_IRQHandler` nie obsługuje flagi `TIMEOUT` (SMBus) — nie używamy `TIMEOUTR`.
**Decyzje użytkownika:** bez programowego limitu czasu transmisji (tylko stan peryferium); zawieszenie widoczne przez diagnostykę co 1 s i `assert` po 10 s; mechanizm w OBU metodach (`I2C_Master_Transmite_DMA`, `I2C_Master_Receive_DMA`); RX ESP32 posprzątany (callbacki usunięte); `while_I2C_STATE_READY()` usunięte całkowicie; bez dodatkowego czekania w `Give`; W1b/W1c (`TIMEOUTR` + `stm32h7xx_it.c`) odrzucone.

| Krok | Plik | Zmiana |
|---|---|---|
| W0a | `Core/Inc/hwConfigFile.h` | `I2C_MASTER_STALL_PRINT_PERIOD_MS 1000`, `I2C_MASTER_STALL_ASSERT_MS 10000` |
| CubeMX | `Radio_main.ioc`, `main.h`, `i2c.c` (+ `.cproject`, `.mxproject`) | etykiety PB8 `I2C1_SCL`, PB9 `I2C1_SDA` → `I2C1_SCL_Pin/_GPIO_Port`, `I2C1_SDA_Pin/_GPIO_Port`. Przy otwarciu wybrano **Continue** (zostaje STM32Cube FW_H7 V1.11.2, NIE migrować) |
| W0b | `comunication_i2cEngine.h/.cpp`, `tasksFunctions.cpp` | konstruktor `i2cMaster(hi2c1, sclGpioPort, sclPin, sdaGpioPort, sdaPin)` — piny tylko do diagnostyki (opis w konstruktorze) |
| W1 | `comunication_i2cEngine.h/.cpp` | `private waitForTransferEnd(DevAddress_7bit)`: `vTaskDelay(1)` aż `READY`; co 1 s printf `STALL` (stan HAL, `ErrorCode`, `ISR`, SCL, SDA, licznik DMA) + przyczyny: „SDA trzymane nisko”, „SCL trzymane nisko”, „przerwanie I2C nieobsluzone” (STOPF/NACKF/TCR/BERR/ARLO/OVR w `ISR` przy zajętym HAL), „DMA nie dostarcza danych” (TXIS/RXNE + licznik DMA bez zmian); po 10 s `assert`; zwraca `HAL_OK`/`HAL_ERROR` (`HAL_I2C_GetError`) |
| W2 + W2b | `comunication_i2cEngine.cpp` | `I2C_Master_Transmite_DMA()`: start → `waitForTransferEnd()`; `HAL_BUSY` → printf `TX BUSY` (stan, `ISR`, SCL, SDA) + assert; `HAL_ERROR`/inne przy starcie → printf `TX START ERROR` + assert |
| R1 | j.w. | to samo dla `I2C_Master_Receive_DMA()` (`RX BUSY`, `RX START ERROR`) |
| R5 | `comunication_i2cEngine.*`, driver ESP32, `tasksFunctions.cpp` | usunięte `i2cMaster::while_I2C_STATE_READY()`, nieużywana przelotka w driverze ESP32 i wywołanie w `initTaskFunctions()` |
| W4b | driver ESP32 | `giveESP32I2CInterfaceTime()` → **`giveESP32I2CInterfaceTimeForDataProcessing()`** + opis: 7 ms potrzebne, bo ESP-IDF po STOP wyłącza `trans_complete` do ponownego `i2c_slave_receive()` (3.5) — `waitForTransferEnd` czeka na STM32, ta funkcja na ESP32 |
| W4 | driver ESP32 | `masterTransmitData()` → `HAL_StatusTypeDef`; błąd → printf `TX ERROR` + assert; za długa ramka → printf + assert; 7 ms PO końcu wysyłki |
| R2 | driver ESP32 | `masterReceiveFromESP32_DMA()`: jedno wywołanie; `HAL_OK` → `…InReception` → `…Received` (inny stan → printf `RX unexpected state` + assert); błąd → `errorpackageDataInReception` + printf `RX ERROR` + assert |
| R4 | `tasksFunctions.cpp` | usunięte `HAL_I2C_MasterRxCpltCallback`, `HAL_I2C_ErrorCallback` |
| R3 | driver ESP32 | usunięte `rxCompleteFromISR()`, `rxErrorFromISR()`, `#define ESP32_I2C_RX_TIMEOUT_MS`; komentarze `i2cRxMasterControl` zaktualizowane |

W6 (task TX) — bez zmian w kodzie (wynik `masterTransmitData()` nieużywany). Kompilacja STM32: OK. ESP32 w tym etapie niezmieniany.
Skutki: stan `timeoutpackageDataInReception` zostaje w enumie, ale STM32 go nie ustawia; `i2cMaster::I2C_Master_Abort()` zostaje (bez wywołań) do przyszłej obsługi błędów; zawieszenie RX widać po stronie STM32 po 1 s (diagnostyka), assert po 10 s.

**Zasady użycia `i2cMaster` (ustalone 2026-11-04):**
- Driver urządzenia woła tylko `I2C_Master_Transmite_DMA()` / `I2C_Master_Receive_DMA()` — wracają po końcu transmisji; nie sprawdza stanu i2c, nie czeka, nie używa callbacków. Sprawdza tylko wynik `HAL_ERROR` → printf + `//todo` + `assert`. Własny protokół urządzenia (ESP32: fazy, 7 ms) w driverze.
- `i2cMasterSemaphoreTake/Give` — w tasku, wokół sekwencji transmisji. **Przy założeniu, że całe I2C idzie przez istniejące taski RX/TX (nowe urządzenia = nowe `case` w ich `switch`), nigdzie nie dodaje się nowych `Take/Give`.** Nowy task wołający `i2cMaster` bezpośrednio (np. si4689) → sam robi `Take/Give`. Bez semafora równoległe wywołanie dostanie `HAL_BUSY` → assert.
- Nie brać semafora drugi raz (binarny, nie rekurencyjny → deadlock); oddawać go podczas czekania na urządzenie (CTS si4689, zapis EEPROM); nie trzymać długo (ESP32 czeka na odczyt max 500 ms od INT); nie wołać z ISR.

### 7.4 Do zrobienia / otwarte
- ~~Wgrać i przetestować wersję końcową z 3.7 na OBU układach~~ — ZROBIONE 2026-11-08. Test 2026-11-08: RX klawisze + TX LED działają (z uzbrajaniem odbioru na 32 B); wersja końcowa 3.7 przetestowana przez użytkownika; silnik po usunięciu `GURU_FIX` — działa (3.7).
- ~~Priorytety tasków I2C STM32 (A1 + A2)~~ — ZROBIONE i przetestowane 2026-11-08 (3.9).
- `ESP_I2C_BUS_DELAY` 7 ms — próba zmniejszenia osobnym krokiem (po Y1 + B1, 3.8), z testem szybkich serii ramek LED.
- `adapter speed` w `HMI_and_bluetooth Configuration.launch` — użytkownik: zostaje 12000 (przy problemach z flashowaniem — 2000).
- ~~`ESP32_INTERRUPT_REQUEST_COUNTING_SEMAPHORE_MAX 21`~~, ~~`new (std::nothrow)` w ESP32~~, ~~nazwa struktury w `#else`~~ — ZROBIONE 2026-11-08 (3.8).
- Migracja ESP32 na ESP-IDF 6.x — **na sam koniec**, gdy nic innego nie pomoże (decyzja użytkownika 2026-11-08); usuwa błąd sterownika z 3.7 i większość obejść w `i2c_engine_slave.cpp` (callbacki `on_request`/`on_receive`, ring buffer odbiorczy).
- **Mechanizm synchronizacji ESP32 interrupt request (wykrycie startu ESP32)** — analiza i propozycja w **sekcji 7.5**; czeka na decyzje użytkownika (7.5.6).
- Opcja: semafor szyny → **mutex rekurencyjny** z `Take/Give` wewnątrz metod transmisji (automatyczna ochrona pojedynczej transmisji, dziedziczenie priorytetu; sekwencje nadal `Take/Give` w driverze). Niezaakceptowane.
- Niewykrywalne bez czasu: SDA trzymane nisko PO ostatnim bajcie (STOP się nie wykona) — dziś pokryte diagnostyką co 1 s + assert po 10 s; na przyszłość do rozważenia IWDG.
- si4689 (`02_Firmware/04_si468x/rev 026`): obrazy ~520 kB (DAB/FMHD/AMHD), I2C1 ~100 kHz (`Timing 0x20B0CCFF`, PCLK1 140 MHz) → ~48 s ładowania; `HOST_LOAD` porcjami (≤4 KB ≈ 370 ms); semafor oddawać po każdej porcji (ESP32: 500 ms); rozważyć Fast Mode 400 kHz lub SPI flash + `FLASH_LOAD`; 3 obrazy ≈ 1,58 MB przy 2 MB flash STM32H7A3RI.
- Obsługa błędów zamiast `assert` (wszystkie `//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32`).
- Printfy DIAG po stronie ESP32 — **użytkownik: zostają**.
- IWDG — użytkownik: nic nie robimy.
- Commit / push — tylko na prośbę użytkownika.
- **OSTATNI KROK: usunąć ten plik (`KONTEKST_I2C_ESP32_STM32.md`) z repozytorium** po zakończeniu prac.

### 7.5 Synchronizacja ESP32 interrupt request / wykrycie startu ESP32 (2026-11-04, ANALIZA — kod NIEzmieniany)

#### 7.5.1 Problem (opis użytkownika)
- Pierwszy impuls INT (`esp32i2cBusInitialised()`: LOW 100 ms) oznacza „ESP32 wystartował” — wprowadzone, żeby STM32 nie czytał śmieci / nie pisał do ESP32 w trakcie jego bootowania (zwykły `delay` się nie sprawdzał). STM32: pierwsze zbocze → `esp32I2cInitialised = true` (`HAL_GPIO_EXTI_Callback`), `startUpTask_initTaskFunctions()` czeka na to w pętli, blokując cały start STM32.
- **Podczas flashowania ESP32 pin INT „wariuje”** → STM32 myśli, że ESP32 już pracuje. Przy debugowaniu trzeba: uruchomić STM32 i zatrzymać na `main`, uruchomić ESP32 i zatrzymać na `main`, puścić STM32, puścić ESP32.
- Dodatkowo (z analizy kodu): reset ESP32 w trakcie pracy STM32 (np. po `assert`) → impuls 100 ms STM32 liczy jako zwykły token → odczyt `len` → śmieci → `assert`. STM32 uruchomiony PO ESP32 nie złapie impulsu → czeka w nieskończoność.

#### 7.5.2 Przyczyna
- INT jest na **GPIO0 ESP32** (`chip_config_ESP32_WROOM32D_DEVBOARD.h:19`, `I2C_SLAVE_INTERRUP_REQUEST_PIN GPIO_NUM_0`) — **pin strapping** (tryb bootowania): `esptool` przez auto-reset (DTR/RTS) ściąga go nisko przy flashowaniu; na devboardzie jest też przycisk BOOT. Strapping ESP32: 0, 2, 5, 12, 15; wolne „zwykłe” np. 4, 13, 14, 16–19, 23, 32, 33.
- Na linii INT jest **zewnętrzny pull-up 10 kΩ** (informacja od użytkownika) — linia nie pływa; STM32 PB5 ma `GPIO_NOPULL` (`gpio.c`), pull-up w CubeMX niepotrzebny.

#### 7.5.3 Rozważone opcje — użytkownik wybrał **3 i 4**
1. Przeniesienie INT z GPIO0 na zwykły pin (sprzęt + `#define`) — usuwa przyczynę; nie wybrane (na razie).
2. Pull-up na PB5 — zbędny (jest 10 kΩ).
3. **Potwierdzenie startu przez I2C:** STM32 odpytuje ESP32 (ping adresu 0x3C) i uznaje go za gotowy dopiero po ACK. ESP32 nie odpowiada ACK w bootloaderze / przy flashowaniu / w resecie (sterownik I2C slave nie działa) → zakłócenia na INT nie oszukają STM32; **kolejność uruchamiania dowolna**.
4. **Ponowna synchronizacja po resecie ESP32:** STM32 mierzy długość impulsu INT (EXTI na oba zbocza + czas): ~20 µs = dane, ~100 ms = restart ESP32 → zerowanie stanu odbioru ESP32 (`idle`, `dataSizeToRead = 0`), czyszczenie zaległych tokenów, ponowne potwierdzenie pingiem. Wymaga zmiany PB5 w CubeMX na oba zbocza. Zastrzeżenie: breakpoint ESP32 w środku impulsu wyglądałby jak restart (STM32 i tak potwierdzi pingiem).

**Uzupełnienia 2026-11-08:**
- Impuls startowy 100 ms daje teraz task `i2cSlaveTransmit` (3.8, G1) — dla STM32 bez zmian (ten sam impuls, to samo zbocze). Długość 100 ms zachowana celowo pod opcję 4 (dziś STM32 reaguje tylko na zbocze — sama długość nic nie daje).
- **Zastrzeżenie do opcji 3 (ping):** gdy debugger ZATRZYMA procesor ESP32 już po inicjalizacji I2C slave, sprzęt I2C może nadal odpowiadać ACK (działa niezależnie od CPU) → ping „przejdzie”, choć program nie obsłuży danych. Dotyczy tylko debugowania (STM32 wykryje to timeoutem/NACK), ale uwzględnić w projekcie.
- Ruch silnika krokowego przy programowaniu ESP32 (obserwacja użytkownika) — NIE problem synchronizacji: silnikiem steruje tylko kod ESP32 (I2C master GPIO18/19 → MCP23008: STEP/DIR/nSLEEP); po resecie `stepperMotorDataParser` sam startuje kalibrację (`isCalibrated() == pdFALSE`), a OpenOCD kilka razy resetuje/zatrzymuje ESP32 (flash, `reset`, `mon reset halt`, stop na `app_main`) → między resetami kalibracja rusza, zatrzymanie CPU ją przerywa.

#### 7.5.4 Ping — ustalenia
- `i2cMaster::ping()` → `HAL_I2C_IsDeviceReady()` → `I2C_GENERATE_START` (`stm32h7xx_hal_i2c.h:801-804`) zeruje `RD_WRN`: na szynie `START → 0x3C + W → ACK/NACK → STOP`, 0 bajtów.
- ESP32 (callback `i2c_slave_rx_done_callback`): `slave_rw = 0` → gałąź zapisu; `rx_fifo_end_addr` bez zmian → **`recpeptionNotToMe`**; maszyna stanów TX (`i2cTxStateNextFromISR`, tylko `slave_rw == 1`) i zapas w tasku (`tx_fifo_cnt == 0`) — **bez wpływu**; nic do parsera; w logu `DIAG Reception NOT to me`. Jak po każdym STOP: ESP-IDF wyłącza `trans_complete` do ponownego `i2c_slave_receive()` → po pingu przerwa `giveESP32I2CInterfaceTimeForDataProcessing()` przed zapisem.
- ESP32 odpowiada ACK od `i2c_new_slave_device` (konstruktor `i2cEngin_slave`) — kilka ms PRZED impulsem inicjalizacji (task `i2cSlaveReceive`).
- **Obecny `ping()` się NIE nadaje** (`comunication_i2cEngine.cpp:118-130`): przy NACK `assert(0)`, `Trials = 100` × `Timeout = 1000` ms (blokujący), omija semafor i `waitForTransferEnd`, printf przy każdym wywołaniu. Używany tylko w parserze przy nieznanym adresie (`tasksFunctions.cpp`) — zostaje bez zmian.
- **Propozycja:** nowa `i2cMaster::isDeviceReady(DevAddress_7bit)` — 1 próba, krótki limit, bez `assert`/printf, zwraca `HAL_OK` (ACK) / `HAL_ERROR` (NACK); wołana przy wziętym semaforze (`Take/Give`).

#### 7.5.5 Inne urządzenia na szynie
- Ping do 0x3C ignorują pozostałe slave (inny adres). Adresy planowanych układów (si468x 0x64–0x67, TDA741x 0x44, 24C16 0x50–0x57, TEA5767 0x60, MCP23008 0x20–0x27) nie kolidują z 0x3C. **Uwaga: OLED SSD1306 ma domyślnie 0x3C — nie dokładać bez zmiany adresu.**
- ESP32 już ignoruje obce transakcje (3.6).
- Ping zawsze z `Take/Give` (przy resynchronizacji w trakcie pracy inny task może nadawać → bez semafora `HAL_BUSY` → assert).
- Inne urządzenia nie muszą czekać na ESP32 — czekanie dotyczy tylko komunikacji z ESP32.
- Sprzęt: ESP32 **bez zasilania** przy zasilonej szynie może przez diody ściągać SDA/SCL nisko (cała szyna stoi; diagnostyka `waitForTransferEnd` pokaże „SDA/SCL trzymane nisko”) — sprawa projektu zasilania. Reset/flashowanie ESP32 tego nie powoduje (GPIO21/22 w high-Z).

#### 7.5.6 Proponowana architektura (NIEzaakceptowana) i decyzje do podjęcia
| Element | Rola |
|---|---|
| flaga `esp32Ready` w driverze ESP32 | `false` na starcie i po wykryciu restartu ESP32 (opcja 4) |
| istniejący task `esp32I2cIntrrruptRequest` | na początku (i po restarcie ESP32) odpytuje: `Take` → `isDeviceReady()` → `Give` → `vTaskDelay(~100 ms)` → … aż ACK; potem `esp32Ready = true` i normalna obsługa tokenów (bez nowego taska) |
| `HAL_GPIO_EXTI_Callback` | przy `esp32Ready == false` impulsy INT ignorowane (flashowanie, impuls inicjalizacji) |
| `startUpTask_initTaskFunctions()` | **nie czeka** na ESP32 — STM32 i inne urządzenia startują od razu |
Wysyłka do ESP32 przed gotowością (ramki LED wysyła `manageRadioButtonsAndMenu()` na starcie):
- A: `masterTransmitData()` czeka na `esp32Ready` — blokuje cały task TX (inne urządzenia też czekają),
- **B (rekomendacja):** odrzuć ramkę (zwolnij bufor) + printf,
- C: B + po gotowości ESP32 ponowne wysłanie aktualnego stanu (LED, silnik) — później.

**Decyzje do podjęcia (ZAPYTAJ przed planem):**
1. Nowa `i2cMaster::isDeviceReady()` (1 próba, bez `assert`/printf, z `Take/Give`).
2. Flaga `esp32Ready` + odpytywanie w tasku `esp32I2cIntrrruptRequest` + ignorowanie INT przed gotowością + start STM32 bez czekania na ESP32.
3. Wysyłka do ESP32 przed gotowością: A / B / C.
4. Po pierwszym ACK odczekać ~100 ms (ESP32 kończy start; impuls inicjalizacji i tak ignorowany)?
5. Opcja 4 (pomiar długości impulsu, restart ESP32) w tym samym etapie czy osobno.
