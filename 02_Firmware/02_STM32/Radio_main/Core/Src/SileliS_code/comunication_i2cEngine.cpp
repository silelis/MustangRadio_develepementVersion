/*
 * i2cEngine.cpp
 *
 *  Created on: Jul 21, 2024
 *      Author: dbank
 */

//#include <i2cEngine.h>
#include <SileliS_code/comunication_i2cEngine.h>

extern myPrintfTask *pPrintf;

/********************************************************************
 * @brief  Konstruktor klasy i2cMaster
 *
 * Klasa i2cMaster obsługuje komunikację stm32 jako master po
 * magistrali i2c: kolejki danych do wysłania / odebranych / adresów do
 * odczytu, semafor dostępu do magistrali oraz transmisje DMA.
 *
 * @param  [hi2c1] [I2C_HandleTypeDef*] uchwyt HAL interfejsu i2c, po
 * 			którym odbywa się komunikacja
 * @param  [sclGpioPort] [GPIO_TypeDef*] port pinu SCL tego interfejsu
 * @param  [sclPin] [uint16_t] pin SCL tego interfejsu
 * @param  [sdaGpioPort] [GPIO_TypeDef*] port pinu SDA tego interfejsu
 * @param  [sdaPin] [uint16_t] pin SDA tego interfejsu
 *
 * @return [NONE]
 *
 * @note   Piny SCL/SDA służą WYŁĄCZNIE do diagnostyki: gdy transmisja
 * 			i2c się nie kończy, waitForTransferEnd() co
 * 			I2C_MASTER_STALL_PRINT_PERIOD_MS odczytuje stan linii
 * 			(rejestr IDR - działa także w trybie alternatywnym i2c) i
 * 			wypisuje go, aby rozpoznać przyczynę zawieszenia: SDA lub
 * 			SCL trzymane nisko przez slave (sprzęt i HAL tego nie
 * 			zgłaszają). Klasa nie konfiguruje tych pinów i nimi nie
 * 			steruje - robi to HAL_I2C_MspInit() (i2c.c). Wartości podawać
 * 			z main.h (etykiety CubeMX, np. I2C1_SCL_Pin /
 * 			I2C1_SCL_GPIO_Port), zgodne z pinami interfejsu hi2c1.
 * @warning Podanie pinów innych niż piny interfejsu hi2c1 nie psuje
 * 			komunikacji, ale diagnostyka pokaże stan niewłaściwych linii.
 *******************************************************************/
i2cMaster::i2cMaster(I2C_HandleTypeDef *hi2c1, GPIO_TypeDef *sclGpioPort,
		uint16_t sclPin, GPIO_TypeDef *sdaGpioPort, uint16_t sdaPin) {
	// TODO Auto-generated constructor stub
	this->p_hi2c1 = hi2c1;
	this->p_sclGpioPort = sclGpioPort;
	this->sclPin = sclPin;
	this->p_sdaGpioPort = sdaGpioPort;
	this->sdaPin = sdaPin;
	this->pI2C_MasterReceiveFromSlave_DataQueue = nullptr;
	this->pI2C_MasterTransmitToSlave_DataQueue = nullptr;
	this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue = nullptr;

	configASSERT(
			this->pI2C_MasterReceiveFromSlave_DataQueue = new (std::nothrow) i2cQueue4DynamicData(DEFAULT_RECEIVE_QUEUE_SIZE));
	configASSERT(
			this->pI2C_MasterTransmitToSlave_DataQueue = new (std::nothrow) i2cQueue4DynamicData(DEFAULT_RECEIVE_QUEUE_SIZE));
	configASSERT(
			this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue = new (std::nothrow) i2cQueue4DynamicData(DEFAULT_RECEIVE_QUEUE_SIZE));

	assert(this->handle_i2cBinarySemaphore = xSemaphoreCreateBinary());
	this->i2cMasterSemaphoreGive();
//	this->i2cMasterSemaphoreTake();

	pPrintf->feedPrintf("%s bus had been initialized.", this->TAG);
//	this->i2cMasterSemaphoreGive();
}

/********************************************************************
 * @brief  Blokuje dostęp do szyny i2c np. podczas czytania lub pisania
 *  przez jakiś task.
 *
 * [Należy użyć w każdym tasku przed rozpoczęciem zadań związanych z
 * fizyczną komunikacją po szyni ei2c]
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t i2cMaster::i2cMasterSemaphoreTake(void) {
	return xSemaphoreTake(this->handle_i2cBinarySemaphore, portMAX_DELAY);
}

/********************************************************************
 * @brief  Oddaje dostęp do szyny i2c np. po czas czytaniu lub pisaniu
 *  przez jakiś task.
 *
 * [Należy użyć w każdym tasku po zakończeniu zadań związanych z
 * fizyczną komunikacją po szyni ei2c]
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t i2cMaster::i2cMasterSemaphoreGive(void) {
	return xSemaphoreGive(this->handle_i2cBinarySemaphore);
}

/********************************************************************
 * @brief  Odpowiednik pinga (z ethernet) dla i2c.
 *
 * [NONE]
 *
 * @param  uint16_t DevAddress_7bit - adres urządzenia i2c w notacji
 * 					7bit
 *
 * @return HAL_StatusTypeDef retVal - HAL_OK dla wykrytego urządzenia
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
HAL_StatusTypeDef i2cMaster::ping(uint16_t DevAddress_7bit) {
	HAL_StatusTypeDef retVal;
	retVal = HAL_I2C_IsDeviceReady(this->p_hi2c1, DevAddress_7bit << 1, 100,
			1000);
	if (retVal == HAL_OK) {
		pPrintf->feedPrintf("%s i2c slave avaliable on address: 0x%x (7bit).",
				this->TAG, DevAddress_7bit/*<<1*/);
	} else {
		pPrintf->feedPrintf(
				"%s i2c slave NOT avaliable on address: 0x%x (7bit).",
				this->TAG, DevAddress_7bit/*<<1*/);
		assert(0);
	}
	return retVal;
}

/********************************************************************
 * @brief  Ustawia addres i2c urządzenia slave z jakiego mają być
 * czytane dane. Adres wpisywany jest do klejki, ktora przekazuje
 * informacje do taska odpowiedzialnego za odczyt danych ze slave.
 *
 * [Taskiem czytającym dane ze slave jest ten uruchamiający funckę
 * static void i2cMasterReceiveFromSlaveDataTask(void *pNothing)]
 *
 * @param  uint16_t DevAddress_7bit - adres urządzenia i2c w notacji
 * 					7bit
 *
 * @return BaseType_t - pdTRUE jeśłi udało się wpisać dane do kolejni
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
BaseType_t i2cMaster::setI2cAdressToAdressQueue(uint16_t DevAddress_7bit) {
	i2cFrame_transmitQueue I2CFrameWithAdressOfSlaveToRead;
	I2CFrameWithAdressOfSlaveToRead.slaveDevice7bitAddress = DevAddress_7bit;
	return this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue->QueueSendFromISR(
			&I2CFrameWithAdressOfSlaveToRead);
}

/********************************************************************
 * @brief  W tasku odpowiedzialnym za odczyt danych z i2c slave
 * obsługującym funkcję:
 * static void i2cMasterReceiveFromSlaveDataTask(void *pNothing)pobiera
 * z kolejki adres i2c slave z jakiego należy odczytać dane.
 *
 * [NONE]
 *
 * @return BaseType_t - pdTRUE jeśłi udało się odczytać dane do kolejni
 *
 * @note   NONE
 * @warning i2cFrame_transmitQueue* I2CFrameWithAdressOfSlaveToRead -
 * 						jest to wskaźnik do zmiennej przechwującej adres
 * 						i2c salve
 *******************************************************************/
//w tasku odpowiedzialnym za odczyt danych z i2c slave pobiera adres z kolejki
BaseType_t i2cMaster::getI2cAdressFromAdressQueue(
		i2cFrame_transmitQueue *I2CFrameWithAdressOfSlaveToRead) {
	return this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue->QueueReceive(
			I2CFrameWithAdressOfSlaveToRead, portMAX_DELAY);
}

//metoda przesuwa otrzymane z i2c slave dane to kolejki taska parsera
BaseType_t i2cMaster::passReceivedI2cDataToParsingQueue(
		i2cFrame_transmitQueue *I2CFrameToParsingTask) {
	return this->pI2C_MasterReceiveFromSlave_DataQueue->QueueSend(
			I2CFrameToParsingTask);
}

//metoda pozwala na pobranie z kolejki parsera (przez taks parsera)  danych do parsowania
BaseType_t i2cMaster::takeReceivedI2cDataFromParsingQueue(
		i2cFrame_transmitQueue *I2CFrameToParsing) {
	return this->pI2C_MasterReceiveFromSlave_DataQueue->QueueReceive(
			I2CFrameToParsing, portMAX_DELAY);
}

//metoda pobiera z kolejki danych do transmisji so I2C slave dane jakie mają być wysłane
BaseType_t i2cMaster::takeTransmitI2cDataToI2C_buffer(
		i2cFrame_transmitQueue *I2CBufferToTransmit) {
	return this->pI2C_MasterTransmitToSlave_DataQueue->QueueReceive(
			I2CBufferToTransmit, portMAX_DELAY);
}

//po zakończeniu parsowania należy zwolnić dynamicznie alokowane dane
void i2cMaster::deleteAlocatedDataAfterParsing(
		i2cFrame_transmitQueue I2CFrameWithPointerToDelete) {
	this->pI2C_MasterReceiveFromSlave_DataQueue->QueueDeleteDataFromPointer(
			I2CFrameWithPointerToDelete);
}

i2cMaster::~i2cMaster() {
	// TODO Auto-generated destructor stub
	this->i2cMasterSemaphoreTake();
	this->p_hi2c1 = NULL;
	delete this->pI2C_MasterReceiveFromSlave_DataQueue;
	delete this->pI2C_MasterTransmitToSlave_DataQueue;
	delete this->pI2C_MasterInitialiseReadFromSlave_AdressessQueue;
	vSemaphoreDelete(this->handle_i2cBinarySemaphore);
}

/********************************************************************
 * @brief  Odczytuje dane ze slave i2c (DMA) i czeka na koniec transmisji.
 *
 * Startuje HAL_I2C_Master_Receive_DMA() i wraca dopiero po
 * zakończeniu transmisji (waitForTransferEnd) - wywołujący nie musi
 * sprawdzać stanu i2c ani czekać; po powrocie z HAL_OK dane są już
 * w pData.
 *
 * @param  [DevAddress_7bit] [uint16_t] 7-bitowy adres slave
 * @param  [pData] [uint8_t*] bufor na odebrane dane
 * @param  [Size] [uint16_t] liczba bajtów do odczytu
 *
 * @return [HAL_StatusTypeDef]
 * @retval [HAL_OK] dane odebrane
 * @retval [HAL_ERROR] błąd transmisji (NACK, BERR, ARLO, OVR, DMA) -
 * 			kod przez HAL_I2C_GetError(); obsługę robi driver urządzenia
 *
 * @note   Wywoływać z tasku, przy wziętym semaforze szyny
 * 			(i2cMasterSemaphoreTake). Szyna zajęta przy starcie
 * 			(HAL_BUSY: SDA/SCL trzymane nisko lub uchwyt HAL nie READY),
 * 			błąd startu DMA oraz transmisja, która się nie kończy,
 * 			kończą się printf + assert(0) wewnątrz i2cMaster.
 * @warning NONE
 *******************************************************************/
HAL_StatusTypeDef i2cMaster::I2C_Master_Receive_DMA(uint16_t DevAddress_7bit,
		uint8_t *pData, uint16_t Size) {
	HAL_StatusTypeDef retVal = HAL_I2C_Master_Receive_DMA(this->p_hi2c1,
			DevAddress_7bit << 1, pData, Size);	//start - bez czekania PRZED: poprzednia transmisja zakończyła się w waitForTransferEnd()
	switch (retVal) {
	case HAL_OK:
		retVal = this->waitForTransferEnd(DevAddress_7bit);	//wraca po zakończeniu transmisji
		break;
	case HAL_BUSY:	//flaga BUSY (szyna zajęta - SDA/SCL trzymane nisko) lub uchwyt HAL nie READY
		pPrintf->feedPrintf("%s RX BUSY addr:0x%02x state:0x%02x ISR:0x%08lx SCL:%d SDA:%d", this->TAG,
				(unsigned int) DevAddress_7bit, (unsigned int) HAL_I2C_GetState(this->p_hi2c1),
				(unsigned long) this->p_hi2c1->Instance->ISR,
				(int) HAL_GPIO_ReadPin(this->p_sclGpioPort, this->sclPin),
				(int) HAL_GPIO_ReadPin(this->p_sdaGpioPort, this->sdaPin));
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
		break;
	default:	//HAL_ERROR - nie udało się uruchomić DMA (konfiguracja DMA/I2C, nie urządzenie); inna wartość - nie powinna wystąpić
		pPrintf->feedPrintf("%s RX START ERROR addr:0x%02x HAL status:%d err:0x%lx", this->TAG,
				(unsigned int) DevAddress_7bit, (int) retVal,
				(unsigned long) HAL_I2C_GetError(this->p_hi2c1));
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
		break;
	}
	return retVal;
}

/********************************************************************
 * @brief  Przerywa trwającą transmisję/odbiór i2c w trybie master.
 *
 * Wywołuje HAL_I2C_Master_Abort_IT() - używane gdy odbiór/nadawanie
 * nie zakończyło się w zadanym czasie (timeout).
 *
 * @param  [DevAddress_7bit] [uint16_t] 7-bitowy adres slave, z którym
 * 			trwa transmisja
 *
 * @return [HAL_StatusTypeDef] aka. wartość zwracana przez
 * 			HAL_I2C_Master_Abort_IT
 *
 * @note   Po zakończeniu przerwania HAL wywołuje
 * 			HAL_I2C_AbortCpltCallback (NIE HAL_I2C_ErrorCallback).
 * @warning Zwraca HAL_ERROR, gdy interfejs nie jest w trakcie
 * 			transmisji master.
 *******************************************************************/
HAL_StatusTypeDef i2cMaster::I2C_Master_Abort(uint16_t DevAddress_7bit) {
	return HAL_I2C_Master_Abort_IT(this->p_hi2c1, DevAddress_7bit << 1);
}

/********************************************************************
 * @brief  Czeka na zakończenie bieżącej transmisji i2c (DMA).
 *
 * Sprawdza co 1 ms (vTaskDelay(1)) stan uchwytu HAL. HAL ustawia
 * HAL_I2C_STATE_READY po zakończeniu transmisji (po STOP) - poprawnym
 * lub z błędem (NACK, BERR, ARLO, OVR, DMA) - dla odczytu i zapisu.
 * Gdy transmisja się nie kończy, co I2C_MASTER_STALL_PRINT_PERIOD_MS
 * wypisuje diagnostykę i rozpoznaje przyczyny, których sprzęt ani HAL
 * nie zgłaszają:
 * - SDA trzymane nisko (stan pinu SDA),
 * - SCL trzymane nisko (stan pinu SCL),
 * - przerwanie I2C nieobsłużone (w rejestrze ISR ustawiona flaga
 *   STOPF/NACKF/TCR/BERR/ARLO/OVR, a HAL nadal zajęty - przerwanie
 *   I2C ma wyższy priorytet niż task, więc obsłużone zdjęłoby flagę),
 * - DMA nie dostarcza danych (TXIS/RXNE ustawione, a licznik DMA bez
 *   zmian od poprzedniej diagnostyki).
 * Po I2C_MASTER_STALL_ASSERT_MS - printf + assert(0).
 *
 * @param  [DevAddress_7bit] [uint16_t] 7-bitowy adres slave (tylko do
 * 			komunikatów diagnostycznych)
 *
 * @return [HAL_StatusTypeDef]
 * @retval [HAL_OK] transmisja zakończona poprawnie
 * @retval [HAL_ERROR] transmisja zakończona błędem - kod błędu przez
 * 			HAL_I2C_GetError(); obsługę (printf + assert) robi driver
 * 			urządzenia, bo tylko on wie, czego dotyczyła transmisja
 *
 * @note   Metoda prywatna - wywoływana WYŁĄCZNIE wewnątrz metod
 * 			transmisji i2cMaster (I2C_Master_Transmite_DMA,
 * 			I2C_Master_Receive_DMA), bezpośrednio PO starcie transmisji
 * 			DMA (HAL_I2C_..._DMA zwróciło HAL_OK). Nie wywoływać przed
 * 			transmisją: wyłączność szyny zapewnia semafor
 * 			(i2cMasterSemaphoreTake), a zajętą szynę zgłasza HAL
 * 			(HAL_BUSY). Drivery urządzeń jej nie wołają - metoda
 * 			transmisji wraca do nich dopiero po zakończeniu transmisji.
 * @warning Wywoływać tylko z tasku (vTaskDelay).
 *******************************************************************/
HAL_StatusTypeDef i2cMaster::waitForTransferEnd(uint16_t DevAddress_7bit) {
	const uint32_t isrIrqFlags = I2C_ISR_STOPF | I2C_ISR_NACKF | I2C_ISR_TCR
			| I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR;	//flagi zdejmowane przez przerwanie I2C (HAL)
	const uint32_t isrDmaFlags = I2C_ISR_TXIS | I2C_ISR_RXNE;	//flagi zdejmowane przez DMA (zapis TXDR / odczyt RXDR)
	TickType_t startTick = xTaskGetTickCount();
	TickType_t lastPrintTick = startTick;
	uint32_t lastDmaCounter = UINT32_MAX;	//pierwsza diagnostyka nie zgłasza "DMA nie dostarcza danych"

	while (HAL_I2C_GetState(this->p_hi2c1) != HAL_I2C_STATE_READY) {
		vTaskDelay(1);
		TickType_t now = xTaskGetTickCount();
		if ((now - lastPrintTick) >= pdMS_TO_TICKS(I2C_MASTER_STALL_PRINT_PERIOD_MS)) {
			lastPrintTick = now;
			HAL_I2C_StateTypeDef state = HAL_I2C_GetState(this->p_hi2c1);
			uint32_t isr = this->p_hi2c1->Instance->ISR;
			GPIO_PinState scl = HAL_GPIO_ReadPin(this->p_sclGpioPort, this->sclPin);
			GPIO_PinState sda = HAL_GPIO_ReadPin(this->p_sdaGpioPort, this->sdaPin);
			DMA_HandleTypeDef *pDma = (state == HAL_I2C_STATE_BUSY_TX) ? this->p_hi2c1->hdmatx : this->p_hi2c1->hdmarx;
			uint32_t dmaCounter = (pDma != nullptr) ? __HAL_DMA_GET_COUNTER(pDma) : 0;
			bool dmaStalled = ((isr & isrDmaFlags) != 0) && (dmaCounter == lastDmaCounter);
			lastDmaCounter = dmaCounter;
			pPrintf->feedPrintf("%s STALL addr:0x%02x %lu ms, state:0x%02x err:0x%lx ISR:0x%08lx SCL:%d SDA:%d DMA cnt:%lu%s%s%s%s",
					this->TAG, (unsigned int) DevAddress_7bit,
					(unsigned long) ((now - startTick) * portTICK_PERIOD_MS), (unsigned int) state,
					(unsigned long) HAL_I2C_GetError(this->p_hi2c1), (unsigned long) isr,
					(int) scl, (int) sda, (unsigned long) dmaCounter,
					(sda == GPIO_PIN_RESET) ? " | SDA trzymane nisko" : "",
					(scl == GPIO_PIN_RESET) ? " | SCL trzymane nisko" : "",
					((isr & isrIrqFlags) != 0) ? " | przerwanie I2C nieobsluzone" : "",
					dmaStalled ? " | DMA nie dostarcza danych" : "");
		}
		if ((now - startTick) >= pdMS_TO_TICKS(I2C_MASTER_STALL_ASSERT_MS)) {
			pPrintf->feedPrintf("%s STALL addr:0x%02x - transmisja nie zakonczyla sie w %u ms", this->TAG,
					(unsigned int) DevAddress_7bit, (unsigned int) I2C_MASTER_STALL_ASSERT_MS);
			//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
			assert(0);
		}
	}
	return (HAL_I2C_GetError(this->p_hi2c1) == HAL_I2C_ERROR_NONE) ? HAL_OK : HAL_ERROR;
}

/********************************************************************
 * @brief  Wysyła dane do slave i2c (DMA) i czeka na koniec transmisji.
 *
 * Startuje HAL_I2C_Master_Transmit_DMA() i wraca dopiero po
 * zakończeniu transmisji (waitForTransferEnd) - wywołujący nie musi
 * sprawdzać stanu i2c ani czekać.
 *
 * @param  [DevAddress_7bit] [uint16_t] 7-bitowy adres slave
 * @param  [pData] [uint8_t*] dane do wysłania (bufor musi istnieć do
 * 			powrotu z metody)
 * @param  [Size] [uint16_t] liczba bajtów
 *
 * @return [HAL_StatusTypeDef]
 * @retval [HAL_OK] dane wysłane
 * @retval [HAL_ERROR] błąd transmisji (NACK, BERR, ARLO, OVR, DMA) -
 * 			kod przez HAL_I2C_GetError(); obsługę robi driver urządzenia
 *
 * @note   Wywoływać z tasku, przy wziętym semaforze szyny
 * 			(i2cMasterSemaphoreTake). Szyna zajęta przy starcie
 * 			(HAL_BUSY: SDA/SCL trzymane nisko lub uchwyt HAL nie READY),
 * 			błąd startu DMA oraz transmisja, która się nie kończy,
 * 			kończą się printf + assert(0) wewnątrz i2cMaster.
 * @warning NONE
 *******************************************************************/
HAL_StatusTypeDef i2cMaster::I2C_Master_Transmite_DMA(uint16_t DevAddress_7bit,
		uint8_t *pData, uint16_t Size) {
	HAL_StatusTypeDef retVal = HAL_I2C_Master_Transmit_DMA(this->p_hi2c1,
			DevAddress_7bit << 1, pData, Size);	//start - bez czekania PRZED: poprzednia transmisja zakończyła się w waitForTransferEnd()
	switch (retVal) {
	case HAL_OK:
		retVal = this->waitForTransferEnd(DevAddress_7bit);	//wraca po zakończeniu transmisji
		break;
	case HAL_BUSY:	//flaga BUSY (szyna zajęta - SDA/SCL trzymane nisko) lub uchwyt HAL nie READY
		pPrintf->feedPrintf("%s TX BUSY addr:0x%02x state:0x%02x ISR:0x%08lx SCL:%d SDA:%d", this->TAG,
				(unsigned int) DevAddress_7bit, (unsigned int) HAL_I2C_GetState(this->p_hi2c1),
				(unsigned long) this->p_hi2c1->Instance->ISR,
				(int) HAL_GPIO_ReadPin(this->p_sclGpioPort, this->sclPin),
				(int) HAL_GPIO_ReadPin(this->p_sdaGpioPort, this->sdaPin));
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
		break;
	default:	//HAL_ERROR - nie udało się uruchomić DMA (konfiguracja DMA/I2C, nie urządzenie); inna wartość - nie powinna wystąpić
		pPrintf->feedPrintf("%s TX START ERROR addr:0x%02x HAL status:%d err:0x%lx", this->TAG,
				(unsigned int) DevAddress_7bit, (int) retVal,
				(unsigned long) HAL_I2C_GetError(this->p_hi2c1));
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
		break;
	}
	return retVal;
}

i2cQueue4DynamicData* i2cMaster::getTransmitQueue() {
	return pI2C_MasterTransmitToSlave_DataQueue;
}
