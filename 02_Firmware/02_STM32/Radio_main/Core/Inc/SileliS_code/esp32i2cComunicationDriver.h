/*
 * esp32i2sComunicationDriver.h
 *
 *  Created on: Aug 2, 2024
 *      Author: dbank
 */

#ifndef SRC_ESP32I2CCOMUNICATIONDRIVER_H_
#define SRC_ESP32I2CCOMUNICATIONDRIVER_H_
#include "i2cEngine.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "comunication_calculate_checksum.h"
//#include <iostream>
#include "SileliS_code/myPrintfTask.h"

class esp32_i2cComunicationDriver {
public:
	static const uint8_t esp32i2cSlaveAdress_7bit = I2C_SLAVE_ADDRESS_ESP32;		//7bit address
	esp32_i2cComunicationDriver(i2cMaster* pointer_to_i2cMasterObject);
	virtual ~esp32_i2cComunicationDriver();
	BaseType_t semaphoreTake__CountingSemaphore(void);

	void incrementInterruptRequestCountingSemaphore(void); //metoda wywoływana podczas wykrycia sygnały interrupt request, aby inkrementować semafor zliczający wystąpienia
	void isCountingSemaphoreOverflowed(void);
	//void seteDynamicmMemeoryAlocationError();


	//BaseType_t i2cMasterSemaphoreTake(void);
	//BaseType_t i2cMasterSemaphoreGive(void);
	//HAL_StatusTypeDef ping(void);
	BaseType_t masterReceiveData(i2cFrame_transmitQueue* dataFrame);
	BaseType_t masterTransmitData(i2cFrame_transmitQueue* dataFrame);
	void while_I2C_STATE_READY(void);

	void parseReceivedData(i2cFrame_transmitQueue I2CReceivedFrame);
protected:


private:
	BaseType_t masterReceiveFromESP32_DMA(uint8_t *pData, uint16_t Size);
	const char *TAG = "ESP32 communication driver log:";
	BaseType_t isCrcSumCorreect(i2cFrame_transmitQueue I2CReceivedFrame, uint8_t	crcSum);


	const uint8_t esp32InterruptRequestCountingSemaphore_MAX = 25;
	const uint16_t esp32CrcSumCounterError_MAX=5;
	i2cMaster* pi2cMaster;												//wskaźnik do obiektu obsługuj ącego komunikację stm'a32 jako i2c master
	SemaphoreHandle_t esp32IntrrruptRequest_CountingSemaphore;			//uchwyt semafora zliczającego ilość wsytąpień esp32 interrupt request i ilość odczytów danych z esp32

	//zmienne kontrolujące wystąpieniue błędów
	BaseType_t esp32InrerruptRequest_CountingSemaphoreOverflowError;	//zmienna informująca o tym, że nastąpiło przepełnienie "esp32IntrrruptRequest_CountingSemaphore", aka. zbyt wiele oczekujących komunikatów, co może wskazywać na błąd.
	BaseType_t esp32DynamicmMemeoryAlocationError;						//zmienna mówiąza o tytm, że nastąpił błąd w dynamicxznej alokacji pamięcia
	uint16_t esp32CrcSumCounterError;
	#warning zrobic obsługę błędów komunikacji (jeśli są błędy komunikacji i jeśli nie ma sygnału keep alive
	//const uint8_t ESP_I2C_BUS_DELAY = 7;
};


namespace parserFunction{
	void keyboardToRadioMeny(i2cFrame_keyboardFrame* kbrdFrame);
}


#endif /* SRC_ESP32I2CCOMUNICATIONDRIVER_H_ */
