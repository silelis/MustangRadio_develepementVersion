# Kontekst: HardFault (double free) + protokół I2C STM32↔ESP32 "sequence"

Plik roboczy — cel: móc szybko odtworzyć/zweryfikować te poprawki po revercie repo
do commita `3c849d34` ("BUCKUP przed poprawkami robionymi przez AI"), do którego
zresetowano gałęzie `main` i `TESTOWANIE_ZMIAN_I_FIXOW` (2026-09-15).

---

## 1. HardFault na STM32 — double free + brak `return` w `myPrintfTask`

**Status: do ponownego wprowadzenia** (poprawka istniała jako commit `4a3ef0a1`,
ale ten commit został usunięty razem z resztą historii po `3c849d34`).

### Przyczyna
`myPrintfTask::myPrintf()` samo zwalnia bufor (`QueueDeleteDataFromPointer`), a wywołujący
task `printfTask()` w `tasksFunctions.cpp` zwalniał go **jeszcze raz** zaraz po powrocie
z `myPrintf()` → `delete[]` na tym samym wskaźniku dwukrotnie → heap corruption → HardFault
przy praktycznie każdym logu przez UART (`feedPrintf`).

Dodatkowo `myPrintf()` był zadeklarowany jako `HAL_StatusTypeDef`, ale nie miał `return`
(undefined behavior, choć nieużywane realnie przez wywołującego).

### Pliki do poprawienia (identycznie w OBU projektach STM32)
- `02_Firmware/02_STM32/Radio_main/Core/Src/SileliS_code/myPrintfTask.cpp`
- `02_Firmware/02_STM32/Radio_main/Core/Src/SileliS_code/tasksFunctions.cpp`
- `02_Firmware/02_STM32/Radio_main_I2C_SLAVE_V2_5.5.2/Core/Src/SileliS_code/myPrintfTask.cpp`
- `02_Firmware/02_STM32/Radio_main_I2C_SLAVE_V2_5.5.2/Core/Src/SileliS_code/tasksFunctions.cpp`

**`myPrintfTask.cpp` — `myPrintf()`, docelowy kod:**
```cpp
HAL_StatusTypeDef myPrintfTask::myPrintf(i2cFrame_transmitQueue itemToPrint) {
	HAL_StatusTypeDef status = HAL_UART_Transmit(this->pHuart, (const uint8_t*) itemToPrint.pData,
			itemToPrint.dataSize, 150);
	HAL_UART_Transmit(this->pHuart, (const uint8_t*) "\r\n", 2, 150);
	this->QueueDeleteDataFromPointer(itemToPrint);
	return status;
}
```

**`tasksFunctions.cpp` — `printfTask()`, docelowy kod:**
```cpp
static void printfTask(void *noThing) {
	i2cFrame_transmitQueue itemToPrint;
	pPrintf->feedPrintf("Radio firmware version: %.2f", FW_VERSION);
	while (1) {
		if (pPrintf->QueueReceive(&itemToPrint, portMAX_DELAY) == pdTRUE) {
			pPrintf->myPrintf(itemToPrint);	// myPrintf() sam zwalnia itemToPrint.pData - nie wolno usuwać go tu ponownie (double free)
		}
	}
}
```

### Jak znaleźć miejsce do poprawy (aktualny/oryginalny kod w `3c849d34`)
Szukać w obu plikach `tasksFunctions.cpp` wzorca w funkcji `printfTask()`:
```cpp
pPrintf->myPrintf(itemToPrint);
pPrintf->QueueDeleteDataFromPointer(itemToPrint);   // <- USUNĄĆ tę linię (double free)
```
i w `myPrintfTask.cpp` w `myPrintf()` dodać `HAL_StatusTypeDef status = ...` + `return status;`
(funkcja obecnie kończy się bez `return`).

---

## 2. Protokół I2C STM32↔ESP32 — tryb "sequence" (wymaga decyzji, jeszcze nie naprawione)

### Dwie równoległe implementacje transferu slave→master

| | ESP32 (`i2c_engine_slave.cpp`) | STM32 (`comunication_esp32i2cComunicationDriver.cpp`) |
|---|---|---|
| **Standardowa (działająca)** | branch `#else` (bez `STM32_2_ESP32_I2C_IN_SEQUENCE`) | `masterReceiveData()` |
| **"Sequence" (HAL reload, eksperymentalna)** | branch `#ifdef STM32_2_ESP32_I2C_IN_SEQUENCE` | `masterReceiveDataInSequence()` |

Wybór, która implementacja jest kompilowana **po obu stronach**, sterowany jest JEDNYM
makrem: `STM32_2_ESP32_I2C_IN_SEQUENCE`, zdefiniowanym w:
```
02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol/comunicationProtocol.h
```
**UWAGA:** ten sam plik nagłówkowy jest fizycznie includowany przez projekt STM32
`Radio_main` (potwierdzone przez `.d` dependency file w `Radio_main/Debug/`) — czyli to
jest jedno wspólne źródło prawdy dla obu firmware'ów, mimo że leży w katalogu ESP32.

### Standardowa ścieżka (`#else` / `masterReceiveData`) — DZIAŁAJĄCA, dopasowana
ESP32 (`i2c_engine_slave.cpp`, branch `#else`):
1. `interruptRequestSet()` (GPIO LOW) — sygnał "mam dane"
2. `i2c_slave_transmit(&dataSize, sizeof(dataSize))`
3. `i2c_slave_transmit(pData, dataSize)`
4. `interruptRequestReset()`

STM32 `masterReceiveData()` (wywoływana po zliczeniu przerwania GPIO od ESP32):
1. `masterReceiveFromESP32_DMA(&dataFrame->dataSize, sizeof(size_t))`
2. alokacja bufora
3. `masterReceiveFromESP32_DMA(dataFrame->pData, dataFrame->dataSize)`

To pasuje 1:1. Ta gałąź ESP32 powinna też mieć fix na WDT/timeout 5500ms → 500ms
(`i2c_engine_slave.h`, `tx_timeout_ms`) i poprawną kolejność `interruptRequestSet()`
PRZED transmisją (jeśli jest po — powoduje "no space in ringbuffer" co 5500ms).

### Ścieżka "sequence" (`#ifdef` / `masterReceiveDataInSequence`) — NIEDZIAŁAJĄCA
Oznaczona jako zepsuta w wielu miejscach:
- `HMI_and_bluetooth - FIX POPRAWKI/.../comunicationProtocol.h`:
  `//#define STM32_2_ESP32_I2C_IN_SEQUENCE - nie działa po stronie STM32 i nie wiem jak naprawić nie właczać!!!`
- STM32 `comunication_esp32i2cComunicationDriver.cpp` (w niektórych wersjach aktywny `#error`):
  `"DO NOT WORK ON STM32 site I do not knw how to solve it. Do not define STM32_2_ESP32_I2C_IN_SEQUENCE."`
- usunięty już commit `8cadb925`: "CLAUDE poprawki i2c sequence - NIE SPRAWDZONE!!! !!! !!!"
  (ten commit siedział na `main`, 1 po `3c849d34` — to WŁAŚNIE ten "niesprawdzony" eksperyment
  z i2c sequence był bezpośrednim powodem revertu całego repo do `3c849d34`)

### Rekomendowana poprawka (do zrobienia świadomie, jako nowy, przetestowany commit)
W `02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol/comunicationProtocol.h`
sprawdzić stan makra `STM32_2_ESP32_I2C_IN_SEQUENCE` i **zostawić je wyłączone / zakomentowane**,
dopóki tryb "sequence" nie zostanie faktycznie przetestowany na sprzęcie:
```c
//#define STM32_2_ESP32_I2C_IN_SEQUENCE
```
Wtedy oba firmware'y (ESP32 `HMI_and_bluetooth` i STM32 `Radio_main`) używają dopasowanej,
działającej ścieżki standardowej (`masterReceiveData` / `#else`). **Nie wprowadzać trybu
"sequence" ponownie bez faktycznego testu na fizycznym sprzęcie STM32+ESP32 — poprzednia
próba (`8cadb925`) była niesprawdzona i to ona spowodowała decyzję o pełnym revercie repo.**

---

## 3. Rozjazd katalogów ESP32: `HMI_and_bluetooth` vs `HMI_and_bluetooth - FIX POPRAWKI`

`HMI_and_bluetooth - FIX POPRAWKI` to backup z commita `517fb822` (2026-03-28,
"backup dla błędów cLAUDE cODE") — **ten commit jest sprzed `3c849d34`, więc powinien
nadal istnieć w historii obu zresetowanych branchy.** Zawiera poprawki, których może
brakować w aktualnym stanie `HMI_and_bluetooth`:

**Tylko w `HMI_and_bluetooth - FIX POPRAWKI` (sprawdzić czy brakuje w bieżącym `HMI_and_bluetooth`):**
- fix Guru Meditation / cross-core deadlock: `i2c_new_master_bus()` przeniesione na Core 1
  (`stepperMotorDataParser` w `tasksFunctionsStepperMotor.cpp`, `xTaskCreatePinnedToCore(...TASK_TO_CORE1)`),
  semafor `xMotorInitDone` synchronizujący inicjalizację. Zamiast tego bywa `GURU_FIX` —
  workaround, który **całkowicie wyłącza I2C master / stepper motor** (silnik krokowy nie
  działa w ogóle, patrz `tasksFunctions.cpp`, `#ifndef GURU_FIX`).
- fix self-assignment w `keyboard.cpp` (`::taskHandler_onPeriodLongButtonPressNotification` —
  nazwa zmiennej globalnej kolidowała z parametrem konstruktora, global zostawał `NULL`,
  `xTaskResumeFromISR(NULL)` → assert).
- fix `vTaskSuspend()` wywoływanego z ISR (niezdefiniowane zachowanie w FreeRTOS) —
  zamieniony na flagę `volatile suspendLongPressFeeder`.

**Do zrobienia (jeśli chcesz mieć wszystkie poprawki naraz):** porównać
`HMI_and_bluetooth/main/` z `HMI_and_bluetooth - FIX POPRAWKI/main/` (pliki: `app_main.cpp`,
`tasksFunctions/tasksFunctions.cpp`, `tasksFunctions/tasksFunctionsStepperMotor.cpp/.h`,
`keyboard/keyboard.cpp/.h`, `hwConfigFile.h`) i świadomie zdecydować, które wersje wziąć.

---

## 4. Konfiguracja debuggera VS Code (osobny wątek, nie firmware)
Wcześniej ustalono (weryfikacja ręczna przez GDB/OpenOCD, działa):
- sonda: **ESP-Prog** (FTDI FT2232H) zamiast J-Link →
  `.vscode/settings.json`: `idf.openOcdLaunchArgs` → `"interface/ftdi/esp32_devkitj_v1.cfg"`
- port UART ESP-Prog: **COM8** (nie COM10) → `.vscode/settings.json`: `"idf.portWin": "COM8"`
- `.vscode/launch.json` (konfiguracja `gdbtarget`/"Eclipse CDT GDB Adapter"), w
  `target.connectCommands` dodać `"monitor reset halt"` (żeby `attach` faktycznie
  resetował chip i breakpointy w `app_main()` i dalej miały szansę się odpalić),
  w `initCommands` dodać `"set remote verbose-resume-packet off"` (zabezpieczenie przed
  błędem `Unknown vCont packet` przy resume przez adapter VS Code).

To nie wpływa na firmware, tylko na narzędzia deweloperskie. Trzeba to odtworzyć ręcznie,
jeśli plik `.vscode/launch.json`/`settings.json` też wrócił do starszej wersji po revercie.
