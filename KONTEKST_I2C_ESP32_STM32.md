# KONTEKST: komunikacja I2C ESP32 (slave) → STM32 (master) z GPIO interrupt request

> **Dla Claude na nowym komputerze:** przeczytaj ten plik w całości, zanim zaczniesz cokolwiek robić.
> Plik opisuje stan prac z 2026-09-29 (ESP32), 2026-10-28 (poprawki `pData`), 2026-11-03: RX STM32←ESP32 ZROBIONY (kroki 1–13, commit `b4e6bba9`) oraz **2026-11-03 (test na sprzęcie): RX STM32←ESP32 DZIAŁA w trybie debugowania** — po poprawce wykrywania odczytu w ESP32 (`tx_fifo_cnt` zamiast `tx_fifo_start_addr`, sekcja 3.6).
> **Zacznij od sekcji 7 („Gdzie jesteśmy / co dalej”)** — tam: co jest zrobione, jak testować na sprzęcie, plan TX/RX (`waitForTransferEnd`, W1–W7) i otwarte decyzje.
> Sekcje 2–4 to tło; sekcja 5 to opis już wprowadzonych kroków 1–13; stary plan z `xTaskNotify` został ZASTĄPIONY.
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
- `dataSize` obejmuje całą ramkę z nagłówkiem `i2cFrame_commonHeader {crcSum, commandGroup, size_t dataSize}`; zakres: `sizeof(i2cFrame_commonHeader) ≤ dataSize ≤ ESP32_SLAVE_RECEIVE_BUFFER_LEN (52)`.
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

### 7.1 Stan na 2026-11-03
| Commit | Co |
|---|---|
| `0eb76373` | `pData`: inicjalizacja pól `i2cFrame_transmitQueue`, alokacja `char[]` (sekcja 6) |
| `400d8a48` | ten plik: zaakceptowany plan kroków 1–13 |
| `b4e6bba9` | **RX STM32←ESP32 (kroki 1–13)** — „komunikacja stm32<->esp32 poprawiona (na sucho)” |
| (2026-11-03, gałąź `esp32-jlink-adapter-speed`) | J-Link `adapter speed` w `.launch`; **ESP32: stos „I2C slave rx”, wykrywanie odczytu przez `tx_fifo_cnt` (3.6)** — przetestowane na sprzęcie: **RX STM32←ESP32 działa w trybie debugowania** |

Co działa w kodzie (RX, odczyt STM32 z ESP32):
- 1 impuls INT z ESP32 = 1 token semafora = 1 wywołanie `masterReceiveData()` = 1 faza: `idle` → odczyt 4 B długości do `i2cRxMaster.dataSizeToRead` → `lenDataReceived`; kolejny INT → `new char[len]`, odczyt całej ramki → `idle`, ramka do parsera.
- Koniec odczytu: callback `HAL_I2C_MasterRxCpltCallback`/`HAL_I2C_ErrorCallback` (`tasksFunctions.cpp`) → `pESP32->rxCompleteFromISR()/rxErrorFromISR()`; task czeka w `masterReceiveFromESP32_DMA()` (pętla `vTaskDelay(1)`, limit `ESP32_I2C_RX_TIMEOUT_MS` = 600 ms, timeout → `I2C_Master_Abort()`).
- Błąd / timeout / długość poza `sizeof(i2cFrame_commonHeader)…52` / brak pamięci → `feedPrintf` + `//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32` + `assert(0)` (STM32 staje). ESP32 przy timeout/error: printf + `assert(0)` → panic → **reset** ESP32.
- TX (STM32→ESP32: LED, silnik) **NIE był zmieniany** — nadal ślepe `vTaskDelay(7)` zamiast czekania na koniec (patrz 7.3).

### 7.2 Test na sprzęcie (użytkownik, w domu) — czego oczekiwać
> **2026-11-03:** test wykonany w trybie debugowania — RX działa po zmianach z 3.6. Do zrobienia: test bez debuggera (normalna praca, wiele ramek/klawiszy z rzędu, obce transakcje STM32 na szynie), potem 7.3. W punkcie 3 poniżej kryterium `tx_fifo_start` jest nieaktualne — patrz log oczekiwany w 3.4.

1. Wgrać oba firmware (STM32 z `b4e6bba9`, ESP32 z commitu po 3.6).
2. Start: ESP32 daje impuls INT inicjalizacji; STM32 wypisuje „waiting for esp32 i2c bus to be initialized.” aż go dostanie.
3. Naciśnięcie klawisza HMI (ESP32 → STM32, ramka klawiatury):
   - UART STM32: **brak** komunikatów `RX ERROR` / `RX TIMEOUT` / `frame length out of range` / `error with memory allocation` / `unexpected state`; klawisz trafia do menu (`keyboardToRadioMeny`).
   - ESP32 (DIAG): 2× „Transmition from me” na ramkę (len, potem dane), `tx_fifo_start` przesuwa się o 4, potem o `dataSize` (tabela w 3.4); brak `TIMEOUT_IN_TRANSMITION`.
4. Jeśli STM32 stanie na `assert` — komunikat przed nim mówi: `Size: 4` = faza len, inny = faza danych; `HAL status`; `dataSizeToRead`. Jeśli ESP32 resetuje się z `TIMEOUT_IN_TRANSMITION` i DIAG pokazuje `tx_fifo_start:X->X` → założenie o `tx_fifo_start_addr` błędne (3.4).
5. Znane ryzyka do obserwacji: ramka dokładnie 32 B (3.5), okno ponownego uzbrojenia `i2c_slave_receive` po stronie ESP32 (3.5), TX LED/silnik nadal bez czekania na koniec (7.3).

### 7.3 Następny etap: wspólny `waitForTransferEnd()` dla TX i RX (plan, NIEWPROWADZONY)
**Problem TX:** `esp32_i2cComunicationDriver::masterTransmitData()` startuje `HAL_I2C_Master_Transmit_DMA` (wraca od razu) i robi ślepe `vTaskDelay(ESP_I2C_BUS_DELAY = 7 ms)`. Task TX (`tasksFunctions.cpp` `i2cMasterTransmitToSlaveDataTask`) potem robi `i2cMasterSemaphoreGive()` i `deleteAlocatedDataAfterParsing()` — gdy wysyłka trwa > 7 ms, semafor oddany przy zajętej szynie, a bufor zwolniony w trakcie DMA. Kolejność `Take → funkcja drivera → Give → delete` w taskach jest DOBRA — zła jest funkcja drivera, która wraca przed końcem transmisji.
**Ustalenia (z analizy HAL `stm32h7xx_hal_i2c.c`):** po końcu transmisji HAL ustawia `State = HAL_I2C_STATE_READY` (sukces: l. 6181 TX / 6216 RX, PRZED callbackiem; błąd: l. 6566, `ErrorCode` l. 6539) — także dla `Mem_*` (EEPROM). Więc stan `READY` + `HAL_I2C_GetError()` wystarczą dla KAŻDEGO urządzenia, RX i TX, bez callbacków. HAL DMA/IT nie ma własnego timeoutu (tylko wersje blokujące).
**Rozważone i ODRZUCONE:** czekanie w `i2cMasterSemaphoreTake()` (nic nie daje) i jako główny mechanizm w `i2cMasterSemaphoreGive()` (za późno — driver RX potrzebuje wyniku przed Give, np. walidacja długości; brak adresu do abort; błąd nie trafia do drivera); `Give` w ISR (bufor/7 ms/błędy rozproszone); osobne stany+callbacki per urządzenie (nie skaluje się); blokujące `HAL_I2C_Master_Receive(…, Timeout)` (polling CPU).
**Wybrany kierunek (zaakceptowany ogólnie 2026-11-03):** czekanie WEWNĄTRZ metod transmisji `i2cMaster`, `waitForTransferEnd()` jako `private`:
| # | Plik | Zmiana |
|---|---|---|
| W1 | `comunication_i2cEngine.h/.cpp` | `private HAL_StatusTypeDef waitForTransferEnd(uint16_t DevAddress_7bit, uint32_t timeout_ms, uint32_t *pErrorCode)`: pętla `while (HAL_I2C_GetState(p_hi2c1) != HAL_I2C_STATE_READY)` z `vTaskDelay(1)` i limitem; timeout → `I2C_Master_Abort(DevAddress_7bit)` + `HAL_TIMEOUT`; potem `HAL_I2C_GetError()` → `HAL_ERROR` gdy ≠ `HAL_I2C_ERROR_NONE`; kod błędu przez `pErrorCode` (opcjonalny) |
| W2 | j.w. | `I2C_Master_Receive_DMA()` / `I2C_Master_Transmite_DMA()`: nowe parametry `uint32_t timeout_ms, uint32_t *pErrorCode = nullptr`; start DMA → `waitForTransferEnd()`; dzisiejsze `while_I2C_STATE_READY()` przed startem → usunąć lub `return HAL_BUSY` (decyzja 2). `while_I2C_STATE_READY()` ZOSTAJE dla `initTaskFunctions()` (`tasksFunctions.cpp`) |
| W3 | driver ESP32 `.cpp` | `masterReceiveFromESP32_DMA()`: pętla + sekcja krytyczna + abort z kroku 7 → jedno wywołanie; stany `lenDataReceived`/`packageDataReceived`/error/timeout ustawia task z wyniku; printf + kod błędu HAL + `//todo` + `assert`; `giveESP32I2CInterfaceTime()` |
| W4 | driver ESP32 `.cpp` (+`.h`) | `masterTransmitData()`: wywołanie z limitem; error/timeout → printf + `//todo` + `assert`; ślepe `vTaskDelay(7)` → `giveESP32I2CInterfaceTime()` PO końcu wysyłki (7 ms ZOSTAJE — ESP32 musi ponownie uzbroić `i2c_slave_receive`, 3.5); typ zwracany `BaseType_t` → `HAL_StatusTypeDef` (dziś `HAL_OK`=0 → `pdFALSE`, czyli sukces wygląda jak porażka; wynik nieużywany) |
| W5 | driver ESP32 `.h/.cpp` + `tasksFunctions.cpp` | usunąć zbędne: `rxCompleteFromISR()`, `rxErrorFromISR()`, `HAL_I2C_MasterRxCpltCallback`, `HAL_I2C_ErrorCallback` |
| W6 | `tasksFunctions.cpp` | dostosowanie do typu z W4 (jeśli zmieniony) |
| W7 | (opcja) `comunication_i2cEngine.cpp` | asekuracja w `i2cMasterSemaphoreGive()`: krótkie czekanie na `READY` z limitem + ogólny printf |
Po W1–W7: kompilacja STM32 + ESP32, commit (na prośbę), test TX (LED/silnik) i RX na sprzęcie.

**Otwarte decyzje użytkownika przed W1 (ZAPYTAJ):**
1. Asekuracja w `Give` (W7): tak/nie.
2. Sprawdzenie przed startem transmisji w metodach `i2cMaster`: usunąć czy `return HAL_BUSY` (rekomendacja: `HAL_BUSY`).
3. Wspólny limit RX/TX = 600 ms (użytkownik chciał jak przy odczycie — OK); przemianować `ESP32_I2C_RX_TIMEOUT_MS` → np. `ESP32_I2C_MASTER_TIMEOUT_MS`: tak/nie.
4. Typ zwracany `masterTransmitData()` → `HAL_StatusTypeDef`: tak/nie.

### 7.4 Pozostałe (później)
- Zamiana semafora binarnego szyny na mutex w `i2cMaster` (dziedziczenie priorytetu) — osobna decyzja.
- Obsługa błędów zamiast `assert` (wszystkie miejsca z `//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32`). Uwaga z HAL: przy błędzie DMA `READY` ustawia się zanim zakończy się abort strumienia DMA.
- Otwarte, opcjonalne: nazwa struktury w `#else` z 6.1 (ostrzeżenie tylko w clang/edytorze).
- Usunięcie printfów DIAG po stronie ESP32 (lub `#ifdef`) — po testach, po uzgodnieniu.
- Znane z 3.5 i z „Znane, zgłoszone” w sekcji 5 (m.in. `vTaskDelay(3000)` w tasku TX, priorytety tasków, „ta funkcja nie działa” przy `masterTransmitData` — sprawdzić po W4). `ESP_I2C_BUS_DELAY 7` bez opisu w kodzie — wartość empiryczna, do sprawdzenia na sprzęcie.
- Commit / push — tylko na prośbę użytkownika.
- **OSTATNI KROK: usunąć ten plik (`KONTEKST_I2C_ESP32_STM32.md`) z repozytorium** po zakończeniu prac.
