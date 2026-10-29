# KONTEKST: komunikacja I2C ESP32 (slave) → STM32 (master) z GPIO interrupt request

> **Dla Claude na nowym komputerze:** przeczytaj ten plik w całości, zanim zaczniesz cokolwiek robić.
> Plik opisuje stan prac z 2026-09-29 (ESP32) oraz **aktualizację z 2026-10-28** (sekcje 5–7: nowy, zaakceptowany kierunek dla STM32 i zrobione poprawki `pData`).
> **Zacznij od sekcji 7 („Gdzie jesteśmy / co dalej”).** Sekcje 2–4 to tło; stary plan z `xTaskNotify` (dawna sekcja 5) został ZASTĄPIONY.
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
| ESP32 | `02_Firmware/01_ESP32/HMI_and_bluetooth` — ESP-IDF **v5.2.8** (`C:/esp/v5.2.8/esp-idf`), Espressif-IDE; kompiluje się (potwierdzone przez użytkownika) |
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
 i2c_slave_transmit(&dataSize, 4 B) → lenDataInTransmition
 INT ▼ ─────────────────────────────────────────► token #1 → read 4 B (dataSize, size_t LE)
 ISR (STOP, slave_rw=1, tx_fifo_start_addr się ruszył) → lenDataTransmited
 i2c_slave_transmit(pData, dataSize) → packageDataInTransmition
 INT ▼ ─────────────────────────────────────────► token #2 → read dataSize B (cała ramka)
 ISR → packageDataTransmited → delete[] → idle          parser: CRC + commandGroup
```

- STM32 **musi czytać dokładnie 4 B, potem dokładnie `dataSize` B** (ESP przesuwa stan po każdym zakończonym odczycie, niezależnie od liczby bajtów).
- `dataSize` obejmuje całą ramkę z nagłówkiem `i2cFrame_commonHeader {crcSum, commandGroup, size_t dataSize}`; zakres: `sizeof(i2cFrame_commonHeader) ≤ dataSize ≤ ESP32_SLAVE_RECEIVE_BUFFER_LEN (52)`.
- **ESP32 czeka na każdy odczyt max 500 ms** (`tx_timeout_ms`), potem przechodzi w `timeoutInTransmition` — **stan końcowy** (ESP przestaje nadawać; na razie tylko printf, obsługa błędów później).

---

## 3. Co zostało zrobione po stronie ESP32 (NIEZACOMMITOWANE w chwili pisania)

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
  - odczyt: zmiana `I2C0.fifo_st.tx_fifo_start_addr` (wskaźnik odczytu TX FIFO) ⇒ `transmitionFromMe`, inaczej `transmitionNotFromMe`.
  - **Stan TX przesuwa się tylko przy `transmitionFromMe`** → `sInstance->i2cTxStateNextFromISR()` (nowa prywatna metoda `IRAM_ATTR`, deklaracja w `.h`).
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

### 3.4 Do sprawdzenia na sprzęcie (diagnostyka w domu)
| Zdarzenie | Oczekiwane `tx_fifo_start` | Klasyfikacja |
|---|---|---|
| STM32 czyta len | przesuw o 4 (mod 32) | Transmition from me |
| STM32 czyta dane | przesuw o `dataSize` (mod 32) | Transmition from me |
| obcy STOP (np. `pi2cMaster->ping()` na nieużywany adres) | bez zmian | Transmition NOT from me |
| STM32 pisze do ESP32 | — | Reception to me |

Jeśli każda ramka kończy się `TIMEOUT_IN_TRANSMITION` i DIAG pokazuje `tx_fifo_start:X->X` → założenie o `tx_fifo_start_addr` jest błędne (alternatywa do sprawdzenia: `status_reg.slave_addressed`).

### 3.5 Znane, zgłoszone, NIE poprawione (ESP32)
- Ramka o długości dokładnie 32 B → wskaźnik FIFO mod 32 bez zmian → błędnie `NotFromMe`.
- Okno między callbackiem a ponownym `i2c_slave_receive()` w tasku: transakcja zakończona w tym czasie ginie (sterownik kasuje zaległe flagi przy uzbrajaniu). Krótkie, mało prawdopodobne przy odstępach ≥7 ms.
- `if (tempData != nullptr)` po zwykłym `new` to martwy kod (IDF przy braku pamięci robi `abort()`).
- Okno INT#1 → odczyt len nie jest chronione przed obcymi transakcjami (częściowo pokrywa krok 8 planu STM32).
- Printfy DIAG do usunięcia po testach (brak `#ifdef`).

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

### 5.4 Kroki — **ZAAKCEPTOWANE 2026-10-29 (plan); każdy diff akceptowany osobno**
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

Przyszłość (poza zakresem): TX do ESP32 przez `MasterTxCpltCallback`, `Mem_*` dla EEPROM/MCP23008.

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

1. Plan (kroki 1–13 z p. 5.4) **zaakceptowany 2026-10-29**. Żaden krok nie jest jeszcze wprowadzony.
2. **Następny: krok 1** — diff 1a/1b był pokazany w czacie, czeka na „akceptuję krok 1”. Pokaż go ponownie (treść w 5.4) i czekaj na akceptację.
3. Realizować kroki 1–13 pojedynczo: diff w czacie → akceptacja → `Edit`.
4. Po krokach: kompilacja STM32 i ESP32 (jak w 6.3).
5. Otwarte (niezaakceptowane, opcjonalne): nazwa struktury w `#else` z 6.1.
4. (Użytkownik, w domu) test pełnego protokołu na sprzęcie + diagnostyka ESP32 wg p. 3.4.
5. Usunięcie printfów DIAG po stronie ESP32 (lub `#ifdef`) — po uzgodnieniu.
6. Commit — tylko na prośbę użytkownika.
7. **OSTATNI KROK: usunąć ten plik (`KONTEKST_I2C_ESP32_STM32.md`) z repozytorium** po zakończeniu prac.
