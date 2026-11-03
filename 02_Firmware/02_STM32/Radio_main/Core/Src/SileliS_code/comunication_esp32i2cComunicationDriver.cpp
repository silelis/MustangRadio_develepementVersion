/*
 * esp32i2sComunicationDriver.cpp
 *
 *  Created on: Aug 2, 2024
 *      Author: dbank
 */

//#include "esp32i2cComunicationDriver.h"
#include <SileliS_code/comunication_esp32i2cComunicationDriver.h>

extern myPrintfTask *pPrintf;
/********************************************************************
 * @brief  Konstruktor klasy esp32_i2cComunicationDriver
 *
 * Klasa esp32_i2cComunicationDriver służy do zapewnienia komunikacji
 * pomięszy stm32 (master), a esp32 (svlave) po magistrali i2c.
 *
 * @param  [pointer_to_i2cMasterObject] [i2cMaster*] Wskaźnik do obiektu
 * 			klasy i2cMaster, odpowiadającej za kontrolę komunikacji stm32
 * 			jako master po magistrali i2c.
 *
 * @return [NONE]
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
esp32_i2cComunicationDriver::esp32_i2cComunicationDriver(
		i2cMaster *pointer_to_i2cMasterObject) {
	// TODO Auto-generated constructor stub
	this->pi2cMaster = pointer_to_i2cMasterObject;
	configASSERT(
			this->esp32IntrrruptRequest_CountingSemaphore = xSemaphoreCreateCounting(this->esp32InterruptRequestCountingSemaphore_MAX, 0));
	this->esp32DynamicmMemeoryAlocationError =
			this->esp32InrerruptRequest_CountingSemaphoreOverflowError =
			pdFALSE;
	this->esp32CrcSumCounterError = 0;
}

/********************************************************************
 * @brief  Oblicza sume kontrolną
 *
 * Metoda oblicza sume kontrolną danych zawartych w adresie wskazanym
 * przez pole pData typu I2CReceivedFrame i porównuje ją z sumą kontrolną
 * jaka została przesłana przez esp32.
 *
 * @param  [I2CReceivedFrame] [i2cFrame_transmitQueue] struktura, która
 * 			zawiera informacje na temat parsametrów danych (m.in. sume
 * 			kontrolną, długość, adres w pamięci gdzie zapisano dane)
 * 			otrzymanych z esp32. NIE zawiera samych danych
 * @param  [crcSum] [uint8_t] wartość sumy kontrolnej, jaka została
 * 			wyliczona przez slave
 *
 * @return [BaseType]
 * @retval [pdPASS] zwracana jeśli suma kontrolna wyliczona i otrzymana
 * 			sa sobie równe
 * @retval [pdFAIL] Zwracana w przypadku różnicy w sumach kontrolnych.
 *
 * @note   Pierwszy byte ramki znajdujący się pod adresem wskazanym przez
 * pData typu i2cFrame_transmitQueue zawiera informację na temat sumy
 * kontrolnej, dlatego metoda liczy sume kontrolną od drugiego byte'u.
 * @warning i2cFrame_transmitQueue nie zawiera otrzymanych danych tylko
 * informacje na temat otrzymanych danych i miejsce (wskaźnik) ich
 * przechowywania w RAM.
 *******************************************************************/
BaseType_t esp32_i2cComunicationDriver::isCrcSumCorreect(
		i2cFrame_transmitQueue I2CReceivedFrame, uint8_t crcSum) {

	if (crcSum
			== calculate_checksum(I2CReceivedFrame.pData,
					I2CReceivedFrame.dataSize/*sizeof(i2cFrame_keyboardFrame)*/)) {
		this->esp32CrcSumCounterError = 0;
		return pdPASS;
	} else {
		this->esp32CrcSumCounterError++;
		//printf("%sCRC sum NOT correct: %d time(s)\r\n", this->TAG, this->esp32CrcSumCounterError);
		pPrintf->feedPrintf("%s CRC sum NOT correct: %d time(s).", this->TAG,
				this->esp32CrcSumCounterError);
		return pdFAIL;
	}
}

/********************************************************************
 * @brief  Zlicza ilość wywołań przerwania od esp32 informującego, że
 * ma dane do wysłania.
 *
 * Metoda wywoływana jest podczas wykrycia przerwania na GPIO do którego
 * podłączony jest sygnał interrupt request z esp32. Esp32 po przez ten
 * sygnał informuje stm32 o tym, że ma przygotowane dane do wysłąnia.
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   Mechanizm interrupt request został użyty dlatego, że odczyt
 * 			danych może zająć wiecej czasu niż kolejne ich dostarczenie
 * 			do bufora nadawczego esp32. Stm32 zlicza ilość wystąpień od
 * 			przerwania interrupt request, a przy każdorazowym pobraniu
 * 			danych dekrementuje semafor zliczający. Dodatkowo gdy zemafor
 * 			jest pusty to wymusza to uśpienie tasku obsługującego odczyt
 * 			danych.
 * @warning [NONE
 *******************************************************************/
void esp32_i2cComunicationDriver::incrementInterruptRequestCountingSemaphore(
		void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(this->esp32IntrrruptRequest_CountingSemaphore,
			&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

esp32_i2cComunicationDriver::~esp32_i2cComunicationDriver() {
	// TODO Auto-generated destructor stub
#warning zrobic porzadny destruktor np. w destruktorze ma wyłączy ć się radio
}

/********************************************************************
 * @brief  Sprawdza czy wartość semafora zliczającego nie jest równa
 * maksymalnej. Może to oznaczać za dużą ilość danych w buforze nadawczym
 * esp32, które nie zostały odebrane.
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   	NONE
 * @warning NONE
 *******************************************************************/
void esp32_i2cComunicationDriver::isCountingSemaphoreOverflowed(void) {
	if ( uxSemaphoreGetCount(this->esp32IntrrruptRequest_CountingSemaphore)
			== this->esp32InterruptRequestCountingSemaphore_MAX) { //sprawdza czy licznik esp32 interrupt request nie jest przepełniony
		this->esp32InrerruptRequest_CountingSemaphoreOverflowError = pdTRUE;
		//printf("!!! ESP32 interrupt request counter overflowed   !!!\r\n");
		pPrintf->feedPrintf(
				"!!! ESP32 interrupt request counter overflowed !!!");
	}
}

/********************************************************************
 * @Oczekuje na semafor esp32IntrrruptRequest_CountingSemaphore i
 * dekrementuje jego wartość
 *
 * @param  [NONE
 *
 * @return NONE
 *******************************************************************/
BaseType_t esp32_i2cComunicationDriver::semaphoreTake__CountingSemaphore(void) {
	return xSemaphoreTake(this->esp32IntrrruptRequest_CountingSemaphore,
			portMAX_DELAY) == pdTRUE;
}

BaseType_t esp32_i2cComunicationDriver::masterTransmitData(
		i2cFrame_transmitQueue *dataFrame) {
	BaseType_t retVal = pdFALSE;
	if (dataFrame->dataSize <= ESP32_SLAVE_RECEIVE_BUFFER_LEN) {
		retVal = this->pi2cMaster->I2C_Master_Transmite_DMA(
				this->esp32i2cSlaveAdress_7bit, (uint8_t*) dataFrame->pData,
				dataFrame->dataSize);
		vTaskDelay(pdMS_TO_TICKS(ESP_I2C_BUS_DELAY));
	} else {
		assert(0);
	}
	return retVal;
}

/********************************************************************
 * @brief  Odczytuje z esp32 jedną fazę ramki (len lub dane)
 *
 * Jedno wywołanie = jeden impuls INT z esp32 = jedna faza:
 * - stan idle (token #1): odczyt długości ramki (4 B) do
 *   i2cRxMaster.dataSizeToRead, stan -> lenDataReceived,
 * - stan lenDataReceived (token #2): alokacja bufora i odczyt całej
 *   ramki do dataFrame->pData, stan -> idle.
 *
 * @param  [dataFrame] [i2cFrame_transmitQueue*] ramka, do której po
 * 			fazie danych trafia wskaźnik i długość odebranej ramki
 *
 * @return [HAL_StatusTypeDef] wartość z masterReceiveFromESP32_DMA
 *
 * @note   Po fazie len dataFrame->pData zostaje nullptr (nic do
 * 			parsera). Po fazie danych dataFrame->pData wskazuje
 * 			bufor alokowany jako char[] (zwalnia parser).
 * @warning Błędy kończą się printf + assert(0) (brak obsługi błędów -
 * 			patrz //todo).
 *******************************************************************/
HAL_StatusTypeDef esp32_i2cComunicationDriver::masterReceiveData(
		i2cFrame_transmitQueue *dataFrame) {
	HAL_StatusTypeDef retVal = HAL_ERROR;
	switch (this->i2cRxMaster.state) {
	case i2cReceiveStates::idle:	//token #1 - faza len: odczyt długości ramki
		this->i2cRxMaster.dataSizeToRead = 0;	//zerowanie przed odczytem nowej długości
		this->i2cRxMaster.state = i2cReceiveStates::lenDataInReception;	//PRZED startem DMA
		retVal = this->masterReceiveFromESP32_DMA(
				(uint8_t*) &this->i2cRxMaster.dataSizeToRead, sizeof(size_t));	//błąd/timeout -> assert w środku
		if ((this->i2cRxMaster.dataSizeToRead < sizeof(i2cFrame_commonHeader))
				|| (this->i2cRxMaster.dataSizeToRead > ESP32_SLAVE_RECEIVE_BUFFER_LEN)) {
			this->i2cRxMaster.state = i2cReceiveStates::errorpackageDataInReception;
			pPrintf->feedPrintf("%s RX frame length out of range: %u", this->TAG,
					(unsigned int) this->i2cRxMaster.dataSizeToRead);
			//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
			assert(0);
			break;	//bez dalszego odczytu, gdy assert wyłączony (NDEBUG)
		}
		break;	//stan lenDataReceived - dane czytane po kolejnym impulsie INT (token #2)
	case i2cReceiveStates::lenDataReceived:	//token #2 - faza danych: odczyt całej ramki
		dataFrame->pData = new (std::nothrow) char[this->i2cRxMaster.dataSizeToRead];	//char[] - zasada alokacji pData (comunicationProtocol.h)
		if (dataFrame->pData == nullptr) {
			this->i2cRxMaster.state = i2cReceiveStates::errorpackageDataInReception;
			pPrintf->feedPrintf("%s error with memory allocation.", this->TAG);
			//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
			assert(0);
			break;	//bez dalszego odczytu, gdy assert wyłączony (NDEBUG)
		}
		this->i2cRxMaster.state = i2cReceiveStates::packageDataInReception;	//PRZED startem DMA
		retVal = this->masterReceiveFromESP32_DMA((uint8_t*) dataFrame->pData,
				(uint16_t) this->i2cRxMaster.dataSizeToRead);	//błąd/timeout -> assert w środku
		dataFrame->dataSize = this->i2cRxMaster.dataSizeToRead;
		this->i2cRxMaster.dataSizeToRead = 0;	//ramka kompletna - długość już niepotrzebna
		this->i2cRxMaster.state = i2cReceiveStates::idle;	//gotowość na kolejną ramkę
		break;
	default:	//...InReception, packageDataReceived, error, timeout - nie powinno wystąpić (esp32I2cIntrrruptRequest czeka na idle / lenDataReceived)
		pPrintf->feedPrintf("%s masterReceiveData() in unexpected state: %d", this->TAG,
				(int) this->i2cRxMaster.state);
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
		break;
	}
	return retVal;
}

/********************************************************************
 * @brief  Odczytuje dane z esp32 (i2c slave) i czeka na koniec odczytu
 *
 * Startuje odczyt DMA i czeka (vTaskDelay(1)), aż callback HAL zmieni
 * stan z ...InReception na ...Received (sukces) lub
 * errorpackageDataInReception (błąd). Po ESP32_I2C_RX_TIMEOUT_MS
 * ustawia timeoutpackageDataInReception i przerywa transmisję.
 *
 * @param  [*pData] [uint8_t] Wskaźnik do adresu pamięci pod jakim mają
 * 			być zapisane otrzymane dane
 * @param  [Size] [uint16_t] Ilość danych jakie mają być odczytane
 *
 * @return [HAL_StatusTypeDef]
 * @retval [HAL_OK] odczyt zakończony poprawnie
 * @retval [HAL_ERROR / HAL_BUSY] błąd startu lub transmisji
 * @retval [HAL_TIMEOUT] odczyt nie zakończył się w czasie
 *
 * @note	Stan ...InReception ustawia wywołujący (masterReceiveData)
 * 			PRZED wywołaniem tej metody.
 * @warning	Błąd i timeout kończą się printf + assert(0) (brak obsługi
 * 			błędów - patrz //todo).
 *******************************************************************/
HAL_StatusTypeDef esp32_i2cComunicationDriver::masterReceiveFromESP32_DMA(
		uint8_t *pData, uint16_t Size) {

	HAL_StatusTypeDef retVal = this->pi2cMaster->I2C_Master_Receive_DMA(
			this->esp32i2cSlaveAdress_7bit, pData, Size);
	if (retVal != HAL_OK) {
		this->i2cRxMaster.state = i2cReceiveStates::errorpackageDataInReception;	//odczyt nie wystartował
	} else {
		TickType_t startTick = xTaskGetTickCount();
		//czekanie, aż callback HAL (ISR) zmieni stan ...InReception
		while ((this->i2cRxMaster.state == i2cReceiveStates::lenDataInReception)
				|| (this->i2cRxMaster.state == i2cReceiveStates::packageDataInReception)) {
			if ((xTaskGetTickCount() - startTick) >= pdMS_TO_TICKS(ESP32_I2C_RX_TIMEOUT_MS)) {
				taskENTER_CRITICAL();	//sprawdzenie i zmiana stanu bez wyścigu z callbackiem HAL (ISR)
				if ((this->i2cRxMaster.state == i2cReceiveStates::lenDataInReception)
						|| (this->i2cRxMaster.state == i2cReceiveStates::packageDataInReception)) {
					this->i2cRxMaster.state = i2cReceiveStates::timeoutpackageDataInReception;	//NAJPIERW stan - spóźniony callback już go nie zmieni
				}
				taskEXIT_CRITICAL();
				if (this->i2cRxMaster.state == i2cReceiveStates::timeoutpackageDataInReception) {
					this->pi2cMaster->I2C_Master_Abort(this->esp32i2cSlaveAdress_7bit);	//POTEM zwolnienie szyny i DMA
					retVal = HAL_TIMEOUT;
				}
				break;
			}
			vTaskDelay(1);
		}
	}

	switch (this->i2cRxMaster.state) {
	case i2cReceiveStates::errorpackageDataInReception:
	case i2cReceiveStates::timeoutpackageDataInReception:
		if (retVal == HAL_OK) {
			retVal = HAL_ERROR;	//błąd zgłoszony przez callback HAL (ISR)
		}
		pPrintf->feedPrintf("%s RX %s, Size: %u, dataSizeToRead: %u, HAL status: %d", this->TAG,
				(this->i2cRxMaster.state == i2cReceiveStates::timeoutpackageDataInReception) ? "TIMEOUT" : "ERROR",
				(unsigned int) Size, (unsigned int) this->i2cRxMaster.dataSizeToRead, (int) retVal);
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
		break;
	default:	//lenDataReceived / packageDataReceived - odczyt poprawny
		break;
	}
	this->giveESP32I2CInterfaceTime();
	return retVal;
}

void esp32_i2cComunicationDriver::giveESP32I2CInterfaceTime(void) {
	vTaskDelay(pdMS_TO_TICKS(ESP_I2C_BUS_DELAY));
}

/********************************************************************
 * @brief  Oczekuje do czasu, aż interfejs i2c nie bedzie dostępny.
 *
 * [Dłuższy opis funkcji, w którym można wyjaśnić szczegóły działania,
 *  algorytmy, przypadki brzegowe itp.]
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
void esp32_i2cComunicationDriver::while_I2C_STATE_READY(void) {
	pi2cMaster->while_I2C_STATE_READY();
}

/********************************************************************
 * @brief  Metoda parsuje,a dokłądnie sprawdza z którego peryferium esp32
 * pochodfzi otrzymana ramka danych i przekazuje tę dane (już konkretne
 * dane) do odpowiedzialnej za odczyt funkcji z namespace "parserFunction"
 *
 * @param  [I2CReceivedFrame] [i2cFrame_transmitQueue] ramka danych  otrzymanych z i2c slave
 *
 * @return NONE
 *
 * @note   NONE
 * @warning NONE
 *******************************************************************/
void esp32_i2cComunicationDriver::parseReceivedData(
		i2cFrame_transmitQueue I2CReceivedFrame) {
	i2cFrame_commonHeader tempI2cFrameCommandHeader; //tymczasowa zmienna, do któej będa kopiowane otrzymane dane (aby zawsze uzyskać sumę crc z prawidłowego miejsca, nawert jeśli zmieni się typredef i2cFrame_commonHeader)
	memcpy(&tempI2cFrameCommandHeader, I2CReceivedFrame.pData,
			sizeof(i2cFrame_commonHeader));

	//kopiowanie danych z otrzymanego bufora do zmiennej tymczasowej
	if (this->isCrcSumCorreect(I2CReceivedFrame,
			tempI2cFrameCommandHeader.crcSum)) {
		switch (tempI2cFrameCommandHeader.commandGroup) {

		case I2C_COMMAND_GROUP_KEYBOARD:
			parserFunction::keyboardToRadioMeny(
					(i2cFrame_keyboardFrame*) I2CReceivedFrame.pData);
			break;
		case I2C_COMMAND_GROUP_BTAUDIO:

			break;
		default:
			//printf("%sunknown commandGroup value:0x%x\r\n",this->TAG, tempI2cFrameCommandHeader.commandGroup);
			pPrintf->feedPrintf("%sunknown commandGroup value:0x%x", this->TAG,
					tempI2cFrameCommandHeader.commandGroup);
			assert(0);
		}
	}
}

enum esp32_i2cComunicationDriver::i2cReceiveStates esp32_i2cComunicationDriver::getI2cRxMasaterState(void){
	return this->i2cRxMaster.state;
}

/********************************************************************
 * @brief  Przesuwa stan odbioru z esp32 po zakończeniu odczytu (STOP).
 *
 * Metoda wywoływana z przerwania (HAL_I2C_MasterRxCpltCallback) po
 * zakończeniu odczytu DMA. Zmienia stan TYLKO gdy trwa odbiór z esp32
 * (stan ...InReception).
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   Inne stany (np. spóźniony callback po timeoucie lub odczyt
 * 			z innego slave) nie są zmieniane.
 * @warning Wywoływać wyłącznie z ISR (bez printf).
 *******************************************************************/
void esp32_i2cComunicationDriver::rxCompleteFromISR(void) {
	switch (this->i2cRxMaster.state) {
	case i2cReceiveStates::lenDataInReception:
		this->i2cRxMaster.state = i2cReceiveStates::lenDataReceived;
		break;
	case i2cReceiveStates::packageDataInReception:
		this->i2cRxMaster.state = i2cReceiveStates::packageDataReceived;
		break;
	default:	//callback nie dotyczy odbioru z esp32 - stan bez zmian
		break;
	}
}

/********************************************************************
 * @brief  Ustawia stan błędu odbioru z esp32.
 *
 * Metoda wywoływana z przerwania (HAL_I2C_ErrorCallback) przy błędzie
 * transmisji (NACK, BERR, ARLO, OVR, DMA). Zmienia stan TYLKO gdy
 * trwa odbiór z esp32 (stan ...InReception).
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   Błąd wykrywa i zgłasza (printf + assert) task odbioru w
 * 			masterReceiveFromESP32_DMA().
 * @warning Wywoływać wyłącznie z ISR (bez printf).
 *******************************************************************/
void esp32_i2cComunicationDriver::rxErrorFromISR(void) {
	switch (this->i2cRxMaster.state) {
	case i2cReceiveStates::lenDataInReception:
	case i2cReceiveStates::packageDataInReception:
		this->i2cRxMaster.state = i2cReceiveStates::errorpackageDataInReception;
		break;
	default:	//błąd nie dotyczy odbioru z esp32 - stan bez zmian
		break;
	}
}

#include <SileliS_code/menu_radioMenu.h>

extern radioMenu *pRadioMenu;		//from tasksFunctions.cpp
namespace parserFunction {
void keyboardToRadioMeny(i2cFrame_keyboardFrame *kbrdFrame) {
	pRadioMenu->queueRadioMenuKbrdSend(kbrdFrame->keyboardData.array);
}

}
