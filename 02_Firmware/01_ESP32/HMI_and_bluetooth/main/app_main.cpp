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
#include "Leds/Leds_backlight.h"
#include "StepperOpto/StepperOpto.h"
#include "tasksFunctions/tasksFunctions.h"
#include "NVSeeprom/NVSeeprom.h"
#include "a2dpSikn/a2dp_sink_demo_main_incl_err.h"


#include "driver/uart.h"
void init_uart();


extern QueueHandle_t handlerQueue_MainKeyboard;
extern QueueHandle_t handlerQueue_i2cFrameTransmittBuffer;
extern TaskHandle_t handlerTask_keyboardQueueParametersParser;
extern TaskHandle_t handlerTask_ledDisplay;
extern TaskHandle_t handlerTask_backlightDisplay;	
extern TaskHandle_t handlerTask_stepperMotor;	
extern hmiDisplay displayLedsColors;
extern SemaphoreHandle_t handlerMutex_ledDisplay_Backlight;



extern "C" void app_main(void)
{
	/* CAUTION */
	//this project had been created with:
	//- Visual Studio 2019
	//- VisualGDB-5.6r9.msi
	//- esp32-gcc11.2.0-r2.exe (ESP32 toolchain)
	/* CAUTION */
		
	init_uart();
	i2sPinsHighImpedanceEnabled();
	
	const char *main_TAG = "Main function:";
	printf("\n\n\n\n\n\n\n\n\n\n%s: starting...\n", main_TAG);

	
	
	displayLedsColors.equaliserLed.primary.blue = 0;
	displayLedsColors.equaliserLed.primary.green = 0;
	displayLedsColors.equaliserLed.primary.red = 25;
	displayLedsColors.equaliserLed.secondary.blue = 0;
	displayLedsColors.equaliserLed.secondary.green = 0;
	displayLedsColors.equaliserLed.secondary.red = 0;
	
	
	displayLedsColors.backlightLeds.primary.red = 0;
	displayLedsColors.backlightLeds.primary.green=0;
	displayLedsColors.backlightLeds.primary.blue=0;
			
	
	
	
	
	
	//tworzy obiekt obsługujący NVS flash radio
	printf("%s: NVS storage init\n", main_TAG);
	NVS * storage = NULL;
	assert(storage = new NVS(NVS_RADIO_CONFIG_NAMESPACE));
	//storage->CAUTION_NVS_ereaseAndInit(NVS_EREASE_COUNTDOWN_TIME);
	
	
	
	
	//tworzenie kolejki bufora nadawczego i2c
	handlerQueue_i2cFrameTransmittBuffer = NULL;
	handlerQueue_i2cFrameTransmittBuffer = xQueueCreate(QueueHandleri2cFrameTransmittBuffer, sizeof(i2cFrame)); 
	assert(handlerQueue_i2cFrameTransmittBuffer);
	
	

	
	
	//tworzy obiekt obsługujący ledy sygnalizacyjne i podświetlenia
	printf("%s: Backlight and display leds init\n", main_TAG);
	LEDS_BACKLIGHT *ledDisplay = NULL;
	assert(ledDisplay = new LEDS_BACKLIGHT(LED_DISPLAY_GPIO, LED_DISPLAY_LEDS_QUANTITY, LED_PIXEL_FORMAT_GRB, LED_MODEL_WS2812));
	ledDisplay->ledStripClearAll();
	handlerMutex_ledDisplay_Backlight = NULL;																								//czyści wskaźnik mutex'u dla podświetlenia	i diód sygnalizacyjnych, bo kilka tasków bedzi ekorzystać z linii komunikacyjnej WS2812 		
	assert(handlerMutex_ledDisplay_Backlight = xSemaphoreCreateBinary());																	//tworzy mutex dla podświetlenia
	xSemaphoreGive(handlerMutex_ledDisplay_Backlight);																						//oddaje mutex, zasób jest dostępny dla pierwszego tasku, który się po niego zgłosi
	printf("%s: Display leds task starting\n", main_TAG);
	assert(xTaskCreate(humanMahineDisplayLeds, "Leds control", 128 * 7, ledDisplay, tskIDLE_PRIORITY, &handlerTask_ledDisplay)); //tworzy task dla diód sygnalizacyjnych (korzystają z WS2812)
	printf("%s: Backlight leds task starting\n", main_TAG);
	assert(xTaskCreate(humanMahineBacklightLeds, "Backlight control", 128*7, ledDisplay, tskIDLE_PRIORITY, &handlerTask_backlightDisplay));	//tworzy task dla dod podświetlenia (korzystają z WS2812)
	
	
	
	//konfiguruje kolejkę, która będzie zawierać elementy odpowiedzi z debounceAndGpiosCheckCallback
	printf("%s: Buttons and encoders (aka keyboard) init\n", main_TAG);
	handlerQueue_MainKeyboard = NULL;
	handlerQueue_MainKeyboard = xQueueCreate(QueueHandlerMainKeyboard_len, sizeof(keyboardUnion));
	assert(handlerQueue_MainKeyboard);
	
	//tworzy obiekt obsługujący klawiaturę
	KEYBOARD *klawiatura = NULL;
	assert(klawiatura = new KEYBOARD(handlerQueue_MainKeyboard, handlerTask_backlightDisplay));
	handlerTask_keyboardQueueParametersParser = NULL;
	printf("%s: Keyboard queue pareser task starting\n", main_TAG);
	//ESP_LOGI(main_TAG, "Keyboard queue pareser task starting");
	
	//tworzenie struktury zawierającej parametry przekazywane do taska parsującego/ sprawdzajacego poprawnośc danych z klawaitury i przekazujących informacje do bufora nadawczego i2c
	taskParameters_keyboardQueueParametersParser taskParameters_keyboardQueueParametersParserTask = { 
		.handlerQueue_mainKeyboard = handlerQueue_MainKeyboard,
		.handlerQueue_i2cFrameTransmitt = handlerQueue_i2cFrameTransmittBuffer
	};
	assert(xTaskCreate(keyboardQueueParametersParser, "Keyboard Param", 128 * 20, &taskParameters_keyboardQueueParametersParserTask, tskIDLE_PRIORITY, &handlerTask_keyboardQueueParametersParser)); //tworzy taska, który parsuje, sprawdza dane które przerwania od klawiatury wipsały w kolejkę: handlerQueue_MainKeyboard, w przerwaniach nie można tego zrobić, bo zajęło by to za dużo czasu
	
	
	
	
	
	
	
	StepperOpto * motor = NULL;
	assert(motor = new StepperOpto());
	
	motorTaskParam motorTaskParamStruct;
	motorTaskParamStruct.motorPointer = motor;
	motorTaskParamStruct.storagePointer = storage;
	//assert(xTaskCreate(stepperMotor, "Stepper morot", 2048, &motorTaskParamStruct, tskIDLE_PRIORITY+2, &handlerTask_stepperMotor));
	assert(xTaskCreatePinnedToCore(stepperMotor, "Stepper morot", 3048, &motorTaskParamStruct, tskIDLE_PRIORITY + 2, &handlerTask_stepperMotor, TASK_TO_CORE1));
		
	btstack_init();
	btstack_main(0, NULL);
	btstack_run_loop_execute();
	
	
	
	while (true)
	{
	         
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
