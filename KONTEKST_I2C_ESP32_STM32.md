# KONTEKST: komunikacja I2C ESP32 (slave) → STM32 (master) z GPIO interrupt request

> **Dla Claude na nowym komputerze:** przeczytaj ten plik w całości, zanim zaczniesz cokolwiek robić.
> Plik opisuje stan prac z 2026-09-29, co zostało zrobione po stronie ESP32 i jaki jest plan dla STM32.
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

## 5. PLAN dla STM32 (przedstawiony, **CZEKA NA AKCEPTACJĘ**)

Założenia: bez nowych tasków i metod w `i2cMaster`; callback HAL tylko budzi istniejący task odbioru; **`rxPhase` zmienia wyłącznie task, i to dopiero po potwierdzonym `HAL_OK`**; błędy → printf (bez `assert`, bez resetów); szyna I2C trzymana przez całą ramkę ESP32 (len + dane), żeby żaden obcy STOP nie wpadł między fazy.

| # | Plik | Zmiana |
|---|---|---|
| 1 | `tasksFunctions.cpp` (obok `HAL_GPIO_EXTI_Callback`) | `HAL_I2C_MasterRxCpltCallback` i `HAL_I2C_ErrorCallback` → `xTaskNotifyFromISR(taskHandle_i2cMasterReceiveFromSlaveDataTask, HAL_OK / HAL_ERROR, eSetValueWithOverwrite, &woken)` + `portYIELD_FROM_ISR`; warunek `hi2c == &hi2c1` i uchwyt ≠ nullptr |
| 1b (opc.) | j.w. | `HAL_I2C_AbortCpltCallback` z printf |
| 2 | `hwConfigFile.h` | `ESP32_I2C_RX_TIMEOUT_MS 50` (jeden transfer), `ESP32_I2C_DATA_PHASE_TIMEOUT_MS 500` (czekanie na INT#2) |
| 3 | `comunication_esp32i2cComunicationDriver.h` | `enum esp32RxResult { frameComplete, lenPhaseDone, rxError }`, `enum esp32RxPhase { waitForLen, waitForData }`, pola `rxPhase`, `size_t rxPendingDataSize`, `static_assert(sizeof(size_t)==4)`, `esp32RxResult masterReceiveData(...)` |
| 4 | `.cpp` konstruktor | `rxPhase = waitForLen; rxPendingDataSize = 0;` |
| 5 | `.cpp` `masterReceiveFromESP32_DMA()` | `xTaskNotifyStateClear(NULL)` → start przez istniejące `pi2cMaster->I2C_Master_Receive_DMA()` (≠HAL_OK → return) → `xTaskNotifyWait(0, 0xFFFFFFFF, &status, pdMS_TO_TICKS(ESP32_I2C_RX_TIMEOUT_MS))` → timeout: `HAL_I2C_Master_Abort_IT()` + `HAL_TIMEOUT` → `giveESP32I2CInterfaceTime()` → return status |
| 6 | `.cpp` `masterReceiveData()` | **waitForLen:** read 4 B do `rxPendingDataSize`; ≠HAL_OK → printf, `rxError`; zły zakres → printf, `rxError`; OK → `rxPhase = waitForData`, `lenPhaseDone`. **waitForData:** `new(std::nothrow) char[len]` (brak → printf, `rxError`); read `len` B (≠HAL_OK → printf, `delete[]`, `rxError`); OK → `pData/dataSize`, `rxPhase = waitForLen`, `frameComplete`. Usunąć nieużywane `uint8_t dataSize`; `retVal != HAL_ERROR` → `== HAL_OK` |
| 7 | `comunication_i2cEngine.h/.cpp` | `getI2cAdressFromAdressQueue(..., TickType_t xTicksToWait = portMAX_DELAY)` (parametr domyślny, bez nowej funkcji) |
| 8 | `tasksFunctions.cpp` `i2cMasterReceiveFromSlaveDataTask` | lokalna `bool esp32FrameInProgress`; `i2cMasterSemaphoreTake()` tylko gdy `!esp32FrameInProgress` (binarny semafor!); po `lenPhaseDone` semafor zostaje wzięty; po `frameComplete`/`rxError` → Give; przy `esp32FrameInProgress` czekanie na adres z timeoutem `ESP32_I2C_DATA_PHASE_TIMEOUT_MS` → printf + Give + flaga false, **`rxPhase` bez zmian**; `frameComplete` → kolejka parsera; usunąć `assert` „error with memory allocation” dla ESP32 |
| 9 | `tasksFunctions.cpp` `HAL_GPIO_EXTI_Callback` | `if (GPIO_Pin == esp32i2cInterruptReqest_Pin)` + guard `pESP32 != nullptr` z printf (okno do 150 ms między impulsem init a utworzeniem `pESP32`) |

**Otwarte decyzje użytkownika (zapytaj przed startem):**
- akceptacja kroków 1–9,
- czy robić 1b,
- wartości timeoutów 50 ms / 500 ms,
- czy przy timeoucie fazy danych wystarczy printf + zwolnienie szyny (bez zmiany `rxPhase`).

Przyszłość (poza zakresem): TX do ESP32 na tym samym mechanizmie (`MasterTxCpltCallback` + notify taska nadawczego `taskHandle_i2cMasterTransmitToSlaveDataTask`), `Mem_*` dla EEPROM/MCP23008; przy kolejnych slave'ach w tasku odbioru — obcy adres przy `esp32FrameInProgress` wraca na koniec kolejki.

### Znane, zgłoszone, NIE poprawione (STM32)
- `i2cMasterTransmitToSlaveDataTask` ma `vTaskDelay(3000)` w każdej iteracji (wysyłka LED/silnik opóźniona do 3 s).
- `ESP32_INTERRUPT_REQUEST_COUNTING_SEMAPHORE_MAX 21` nieużywane (sterownik ma własne `= 25`; przy 2 tokenach/ramkę ≈ 12 ramek bufora).
- Task odbioru ma `tskIDLE_PRIORITY` (0).

---

## 6. Kolejność dalszych prac

1. (Użytkownik, w domu) test diagnostyki ESP32 wg p. 3.4 — możliwe dopiero sensownie po zmianach STM32 lub prostym teście `ping()`.
2. STM32: akceptacja planu z p. 5 → diff kroku 1 → … → krok 9 (każdy osobno).
3. Test pełnego protokołu na sprzęcie.
4. Usunięcie printfów DIAG po stronie ESP32 (lub przełącznik `#ifdef`) — po uzgodnieniu z użytkownikiem.
5. Commit zmian (tylko na prośbę użytkownika).
6. **OSTATNI KROK: usunąć ten plik (`KONTEKST_I2C_ESP32_STM32.md`) z repozytorium** po zakończeniu prac.
