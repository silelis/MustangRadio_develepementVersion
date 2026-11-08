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

/********************************************************************
 * @brief  Wysyła ramkę danych do esp32 (i2c slave).
 *
 * Wysyła ramkę przez i2cMaster::I2C_Master_Transmite_DMA(), która
 * wraca dopiero po zakończeniu transmisji, a potem daje esp32
 * ESP_I2C_BUS_DELAY na ponowne uzbrojenie odbiornika
 * (i2c_slave_receive po stronie esp32).
 *
 * @param  [dataFrame] [i2cFrame_transmitQueue*] ramka do wysłania
 * 			(pData - dane, dataSize - długość)
 *
 * @return [HAL_StatusTypeDef]
 * @retval [HAL_OK] ramka wysłana
 *
 * @note   Wywoływać z tasku, przy wziętym semaforze szyny. Po powrocie
 * 			transmisja jest zakończona - bufor pData można zwolnić.
 * @warning Błąd transmisji i za długa ramka kończą się printf +
 * 			assert(0) (brak obsługi błędów - patrz //todo).
 *******************************************************************/
HAL_StatusTypeDef esp32_i2cComunicationDriver::masterTransmitData(
		i2cFrame_transmitQueue *dataFrame) {
	HAL_StatusTypeDef retVal = HAL_ERROR;
	if (dataFrame->dataSize <= ESP32_SLAVE_RECEIVE_BUFFER_LEN) {
		retVal = this->pi2cMaster->I2C_Master_Transmite_DMA(
				this->esp32i2cSlaveAdress_7bit, (uint8_t*) dataFrame->pData,
				dataFrame->dataSize);	//wraca po zakończeniu wysyłki (szyna zajęta / błąd startu / zawieszenie -> assert w i2cMaster)
		if (retVal != HAL_OK) {
			pPrintf->feedPrintf("%s TX ERROR, dataSize: %u, HAL status: %d, err: 0x%02lx (0x04 = NACK)", this->TAG,
					(unsigned int) dataFrame->dataSize, (int) retVal, (unsigned long) this->pi2cMaster->getI2cError());
			//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
			assert(0);
		}
		this->giveESP32I2CInterfaceTimeForDataProcessing();	//przerwa dla esp32 PO zakończeniu wysyłki (waitForTransferEnd czeka na STM32, ta funkcja - na esp32); opis przy giveESP32I2CInterfaceTimeForDataProcessing()
	} else {
		pPrintf->feedPrintf("%s TX frame too long: %u > %u", this->TAG,
				(unsigned int) dataFrame->dataSize, (unsigned int) ESP32_SLAVE_RECEIVE_BUFFER_LEN);
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
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
 * @brief  Odczytuje z esp32 (i2c slave) jedną fazę ramki (len lub dane)
 *
 * Odczyt przez i2cMaster::I2C_Master_Receive_DMA(), która wraca
 * dopiero po zakończeniu transmisji. Na podstawie wyniku przesuwa stan
 * odbioru: lenDataInReception -> lenDataReceived,
 * packageDataInReception -> packageDataReceived (sukces) albo
 * -> errorpackageDataInReception (błąd).
 *
 * @param  [*pData] [uint8_t] Wskaźnik do adresu pamięci pod jakim mają
 * 			być zapisane otrzymane dane
 * @param  [Size] [uint16_t] Ilość danych jakie mają być odczytane
 *
 * @return [HAL_StatusTypeDef]
 * @retval [HAL_OK] odczyt zakończony poprawnie
 *
 * @note	Stan ...InReception ustawia wywołujący (masterReceiveData)
 * 			PRZED wywołaniem tej metody. Szyna zajęta, błąd startu DMA i
 * 			zawieszona transmisja kończą się printf + assert(0) w i2cMaster.
 * @warning	Błąd transmisji kończy się printf + assert(0) (brak obsługi
 * 			błędów - patrz //todo).
 *******************************************************************/
HAL_StatusTypeDef esp32_i2cComunicationDriver::masterReceiveFromESP32_DMA(
		uint8_t *pData, uint16_t Size) {
	HAL_StatusTypeDef retVal = this->pi2cMaster->I2C_Master_Receive_DMA(
			this->esp32i2cSlaveAdress_7bit, pData, Size);	//wraca po zakończeniu odczytu (szyna zajęta / błąd startu / zawieszenie -> assert w i2cMaster)
	if (retVal == HAL_OK) {
		switch (this->i2cRxMaster.state) {
		case i2cReceiveStates::lenDataInReception:
			this->i2cRxMaster.state = i2cReceiveStates::lenDataReceived;
			break;
		case i2cReceiveStates::packageDataInReception:
			this->i2cRxMaster.state = i2cReceiveStates::packageDataReceived;
			break;
		default:	//nie powinno wystąpić - masterReceiveData() ustawia ...InReception przed wywołaniem
			pPrintf->feedPrintf("%s RX unexpected state: %d", this->TAG, (int) this->i2cRxMaster.state);
			//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
			assert(0);
			break;
		}
	} else {
		this->i2cRxMaster.state = i2cReceiveStates::errorpackageDataInReception;
		pPrintf->feedPrintf("%s RX ERROR, Size: %u, dataSizeToRead: %u, HAL status: %d, err: 0x%02lx (0x04 = NACK)", this->TAG,
				(unsigned int) Size, (unsigned int) this->i2cRxMaster.dataSizeToRead, (int) retVal,
				(unsigned long) this->pi2cMaster->getI2cError());
		//todo: zrobić obsługę błędów komunikacji i2c dla stm32 i esp32
		assert(0);
	}
	this->giveESP32I2CInterfaceTimeForDataProcessing();	//przerwa dla esp32 PO zakończeniu odczytu (waitForTransferEnd czeka na STM32, ta funkcja - na esp32); opis przy giveESP32I2CInterfaceTimeForDataProcessing()
	return retVal;
}

/********************************************************************
 * @brief  Daje esp32 czas na przygotowanie się do kolejnej transmisji
 * 			i2c (ESP_I2C_BUS_DELAY).
 *
 * Wywoływać PO każdej zakończonej transmisji z esp32 (zapis i odczyt).
 *
 * @param  NONE
 *
 * @return NONE
 *
 * @note   Dlaczego jest potrzebna, mimo że i2cMaster czeka na koniec
 * 			transmisji (waitForTransferEnd):
 * 			- waitForTransferEnd() czeka na STM32 - aż transmisja na
 * 			  magistrali się skończy (STOP),
 * 			- ta funkcja czeka na ESP32 - aż przetworzy zakończoną
 * 			  transmisję. Sterownik ESP-IDF po każdym STOP wywołuje
 * 			  callback i WYŁĄCZA przerwanie trans_complete; ponownie
 * 			  włącza je dopiero task i2cSlaveReceive (i2c_slave_receive).
 * 			  Zapis STM32 -> ESP32 zakończony w tej przerwie nie wywoła
 * 			  callbacku i ramka PRZEPADA (KONTEKST_I2C_ESP32_STM32.md,
 * 			  p. 3.5).
 * 			Po zapisie chroni następny zapis (np. LED, zaraz po nim
 * 			silnik). Po odczycie chroni zapis wykonany zaraz po odczycie
 * 			(ESP32 po odczycie też wyłącza trans_complete); sam kolejny
 * 			odczyt i tak inicjuje ESP32 impulsem INT.
 * @warning ESP_I2C_BUS_DELAY (hwConfigFile.h) dobrane doświadczalnie.
 * 			Do usunięcia/zmniejszenia, gdy ESP32 będzie uzbrajał
 * 			odbiornik bez opóźnienia lub sygnalizował gotowość.
 *******************************************************************/
void esp32_i2cComunicationDriver::giveESP32I2CInterfaceTimeForDataProcessing(void) {
	vTaskDelay(pdMS_TO_TICKS(ESP_I2C_BUS_DELAY));
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

#include <SileliS_code/menu_radioMenu.h>

extern radioMenu *pRadioMenu;		//from tasksFunctions.cpp
namespace parserFunction {
void keyboardToRadioMeny(i2cFrame_keyboardFrame *kbrdFrame) {
	pRadioMenu->queueRadioMenuKbrdSend(kbrdFrame->keyboardData.array);
}

}
