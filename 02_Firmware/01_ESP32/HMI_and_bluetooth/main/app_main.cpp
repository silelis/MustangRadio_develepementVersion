 /* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//#include <iostream>
#include "hwConfigFile.h"
#include <inttypes.h>
#include "cstdio"
#include "driver/gpio.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "keyboard/keyboard.h"
//#include "Leds/Leds_backlight.h"
//#include "StepperOpto/StepperOptoPowerOFF.h"
#include "tasksFunctions/tasksFunctions.h"
#include "tasksFunctions/tasksFunctionsLeds.h"
#include "tasksFunctions/tasksFunctionsStepperMotor.h"
//#include "NVSeeprom/NVSeeprom.h"

#if USED_BLUETOOTH_STACK == USE_BTSTACK
#include "a2dpSikn/a2dp_sink_demo_main_incl_err.h"
#error "Btstack had been tested and is working but code had not been implemented"
#elif USED_BLUETOOTH_STACK == USE_ESP32A2DP
#include "bt_audio/bt_audio.h"
#endif

#include "driver/uart.h"
void init_uart();



/********************************************************************
* @brief  [Krótki opis funkcji]
*
* [Dłuższy opis funkcji, w którym można wyjaśnić szczegóły działania,
*  algorytmy, przypadki brzegowe itp.]
*
* @param  [parametr1] [Typ parametru] Opis pierwszego parametru.
* @param  [parametr2] [Typ parametru] Opis drugiego parametru.
* @param  [parametr3] [Typ parametru] Opis trzeciego parametru.
*
* @return [Typ zwracanej wartości] Opis tego, co funkcja zwraca.
* @retval [wartość_1] Opis pierwszej możliwej wartości zwracanej.
* @retval [wartość_2] Opis drugiej możliwej wartości zwracanej.
*
* @note   [Dodatkowe informacje, jeśli są istotne]
* @warning [Ostrzeżenia dotyczące używania funkcji, jeśli istnieją]
*******************************************************************/




TaskHandle_t handlerTask_ledDisplay;
TaskHandle_t handlerTask_backlightDisplay;	
TaskHandle_t handlerTask_stepperMotorDataPasrser;	
//TaskHandle_t handlerTask_i2cSlaveTransmit; //uchwyt do taska obsługującego transmisję z i2c slave to i2c master
TaskHandle_t handlerTask_i2cSlaveReceive; //uchwyt do taska obsługającego odbieranie danych z i2c slave to i2c master
TaskHandle_t handlerTask_i2cReceivedDataParser; //uchwyt do taska obsługającego parsowanie otrzymanych z i2c danych
//#include "driver/i2c_slave.h"

extern "C" void app_main(void)
{
	/* CAUTION */
	//this project had been created with:
	//- Visual Studio 2019
	//- VisualGDB-6.0
	//- esp-idf 5.2 ==? 5.2.2
	/* CAUTION */

	bt_audio_sink::i2sHighImpedanceEnabled(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DATA);		//musi być na samym początku, aby nie uszkodzić urządzeń szyny I2S

	QueueHandle_t handlerQueue_MainKeyboard;
	TaskHandle_t handlerTask_keyboardQueueParametersParser;
	
	init_uart();	
	

	const char *main_TAG = "Main function:";
	printf("\n\n\n\n\n\n\n\n\n\n%s starting...\n", main_TAG);
	
	//funkcja inicjalizująca handlery (static, widoczne tylkow headerze) funkcji tasków 
	taskFunctionsStaticHandlersInit();
	
	//configASSERT(xTaskCreate(i2cSlaveTransmit, "I2C slave tx", 128 * 8, NULL, tskIDLE_PRIORITY+1, &handlerTask_i2cSlaveTransmit));
	configASSERT(xTaskCreate(i2cSlaveReceive, "I2C slave rx", 128 * 10, NULL, tskIDLE_PRIORITY+2, &handlerTask_i2cSlaveReceive));
	configASSERT(xTaskCreate(i2cSlaveTransmit, "I2C slave tx", 128 * 32, NULL, tskIDLE_PRIORITY + 2, &handlerTask_i2cSlaveReceive));
	
	
	//oddaje mutex, zasób jest dostępny dla pierwszego tasku, który się po niego zgłosi
	printf("%s Display leds task starting\n", main_TAG);
	configASSERT(xTaskCreate(humanMahineDisplayLeds, "Leds control", 128 * 8, NULL, tskIDLE_PRIORITY, &handlerTask_ledDisplay)); //tworzy task dla diód sygnalizacyjnych (korzystają z WS2812)
	printf("%s Backlight leds task starting\n", main_TAG);
	configASSERT(xTaskCreate(humanMahineBacklightLeds, "Backlight control", 128 * 8, NULL, tskIDLE_PRIORITY, &handlerTask_backlightDisplay)); //tworzy task dla dod podświetlenia (korzystają z WS2812)
	
	//konfiguruje kolejkę, która będzie zawierać elementy odpowiedzi z debounceAndGpiosCheckCallback
	printf("%s Buttons and encoders (aka keyboard) init\n", main_TAG);
	handlerQueue_MainKeyboard = NULL;
	handlerQueue_MainKeyboard = xQueueCreate(MAIN_KEYBOARD_QUEUE_LEN, sizeof(keyboardUnion));
	configASSERT(handlerQueue_MainKeyboard);
	
	//tworzy obiekt obsługujący klawiaturę
	KEYBOARD *klawiatura = NULL;
	configASSERT(klawiatura = new KEYBOARD(handlerQueue_MainKeyboard, handlerTask_backlightDisplay));
	handlerTask_keyboardQueueParametersParser = NULL;
	printf("%s Keyboard queue pareser task starting\n", main_TAG);

	configASSERT(xTaskCreate(keyboardQueueParametersParser, "Keyboard Param", 128 * 20, handlerQueue_MainKeyboard/*&taskParameters_keyboardQueueParametersParserTask*/, tskIDLE_PRIORITY+1, &handlerTask_keyboardQueueParametersParser)); //tworzy taska, który parsuje, sprawdza dane które przerwania od klawiatury wipsały w kolejkę: handlerQueue_MainKeyboard, w przerwaniach nie można tego zrobić, bo zajęło by to za dużo czasu
	
	assert(xTaskCreate(stepperMotorDataParser, "Stepper morot", 128 * 20, NULL, tskIDLE_PRIORITY, &handlerTask_stepperMotorDataPasrser));
	//configASSERT(xTaskCreatePinnedToCore(stepperMotorDataParser, "StepMotParser", 3048, NULL, tskIDLE_PRIORITY + 1, &handlerTask_stepperMotor, TASK_TO_CORE1));
		
	configASSERT(xTaskCreate(i2cReceivedDataParser, "I2C parser", 128 * 20, NULL, tskIDLE_PRIORITY + 2, &handlerTask_i2cReceivedDataParser)); //tworzy taska, który parsuje, sprawdza dane otrzymane z i2c
	




	

	bt_audio_sink testBT(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DATA);
	testBT.btAudioDeviceOn();





#if USED_BLUETOOTH_STACK == USE_BTSTACK
	btstack_init();
	btstack_main(0, NULL);
	btstack_run_loop_execute();
#endif
	
	while (true)
	{
	         vTaskDelay(portMAX_DELAY);
			 
	}
		
}




void init_uart() {
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 122,
		.source_clk =UART_SCLK_DEFAULT
	};

	uart_param_config(UART_NUM_0, &uart_config);
	uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);
}
