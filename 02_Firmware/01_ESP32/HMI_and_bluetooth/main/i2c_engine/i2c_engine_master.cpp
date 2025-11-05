#include "i2c_engine_master.h"

static i2c_master_bus_handle_t handler_i2c_bus_master;
static i2c_master_bus_config_t i2c_bus_config_master;
 
/*---------------------------------------------------------------
 * Konstruktor klasy odpwiadającej za komunikację ESP32 po i2c z
 * urządzeniami i2c slave.
 * Parameters:
 * i2c_port_num_t i2c_port	- numer portu i2c w kontrolerze ESP32
 * gpio_num_t sda_io_num	- numer pinu kontrolera ESP32 do którego
 *							  przypisano sygnał SDA szyny i2c
 * gpio_num_t scl_io_num	- numer pinu kontrolera ESP32 do którego
 *							  przypisano sygnał SCL szyny i2c
 * Returns:
 * NONE
*---------------------------------------------------------------*/
i2cEngin_master::i2cEngin_master(i2c_port_num_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num)
{
	i2c_bus_config_master.clk_source = I2C_CLK_SRC_DEFAULT;
	i2c_bus_config_master.i2c_port = i2c_port;
	i2c_bus_config_master.scl_io_num = scl_io_num;
	i2c_bus_config_master.sda_io_num = sda_io_num;
	i2c_bus_config_master.glitch_ignore_cnt = 7;
	i2c_bus_config_master.flags.enable_internal_pullup = false;

	phandler_i2c_bus = &handler_i2c_bus_master;
	assert(!i2c_new_master_bus(&i2c_bus_config_master, phandler_i2c_bus));
	this->devicesOnBus = 0;
		
	vTaskDelay(pdMS_TO_TICKS(50));
	this->xI2CMasterMutex = NULL;
	this->xI2CMasterMutex = xSemaphoreCreateMutex();
	assert(this->xI2CMasterMutex);
	//xSemaphoreGive(this->xI2CMasterMutex);
	this->semaphoreGive();
	printf("%s bus has been initialised on port %d.\r\n", this->TAG, i2c_port);

}

/*---------------------------------------------------------------
 * Metoda pozwalająca sprawdzić czy urządzenie slave o adresie 	
 * i2c_address jest dostępne na szynie i2c.
 * Parameters:
 * uint8_t i2c_address	- adres i2c urządzenia slave 8-bit 
 * Returns:
 * esp_err_t ret		-  ESP_OK, ESP_FAIL, etc
*---------------------------------------------------------------*/
esp_err_t i2cEngin_master::i2cPing(uint8_t i2c_address)
{
	//xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
	this->semaphoreTake();
	esp_err_t ret = i2c_master_probe(*phandler_i2c_bus, i2c_address, 150);
	//xSemaphoreGive(this->xI2CMasterMutex);
	this->semaphoreGive();
	if (ret == ESP_OK) {
		printf("%s, master have detected I2C slave device with address 0x%x.\r\n", this->TAG, i2c_address);
	}
	else {
		printf("%s master HAVE NOT detected I2C slave device with address 0x%x.\r\n", this->TAG, i2c_address);
	}
	return ret;
}


/*---------------------------------------------------------------
* Metoda rejestruje dodane do szyny i2c urządzenie typu slave.
* Jest to metoda pomocnicza dostarczająca dane wykorzystywane
* przy destruktorze obiektu.
* Parameters:
* NONE
* Returns:
* uint16 devicesOnBus	-  aktualna ilość urządzeń na szynie i2c
*---------------------------------------------------------------*/
uint16_t i2cEngin_master::devicesOnBusIncrement()
{
	if (this->devicesOnBus == UINT16_MAX)
		assert(0);
		#warning napisać jakąś leprzą pbsługę błędów
		//TODO:	 napisać jakąś leprzą pbsługę błędów
	else
		this->devicesOnBus++;	
	return this->devicesOnBus;
}


/*---------------------------------------------------------------
* Metoda rejestruje usunięte z szyny i2c urządzenie typu slave.
* Jest to metoda pomocnicza dostarczająca dane wykorzystywane
* przy destruktorze obiektu.
* Parameters:
* NONE
* Returns:
* uint16 devicesOnBus	-  aktualna ilość urządzeń na szynie i2c
*---------------------------------------------------------------*/
uint16_t i2cEngin_master::devicesOnBusDecrement()
{
	if (this->devicesOnBus == 0)
		assert(0);
		#warning napisać jakąś leprzą pbsługę błędów
		//TODO:	 napisać jakąś leprzą pbsługę błędów
	else
		this->devicesOnBus--;
	return this->devicesOnBus;
}

/*---------------------------------------------------------------
* Destruktor klasy odpwiadającej za komunikację ESP32 po i2c z
* urządzeniami i2c slave.
* Parameters:
* NONE
* Returns:
* NONE
*---------------------------------------------------------------*/
i2cEngin_master::~i2cEngin_master()
{
	if (this->devicesOnBus == 0) {
		//xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
		this->semaphoreTake();
		i2c_del_master_bus(*phandler_i2c_bus);
		vSemaphoreDelete(this->xI2CMasterMutex);
		printf("%s bus has been destructed.\r\n", this->TAG);
	}
	else
	{
		printf("%s there is  %d devices on bus. You CAN NOT use object destructor (~i2cEngin_master).\r\n", this->TAG, this->devicesOnBus);
		assert(0);
	}	
}

BaseType_t i2cEngin_master::semaphoreTake(void)
{
	return xSemaphoreTake(this->xI2CMasterMutex, portMAX_DELAY);
}
BaseType_t i2cEngin_master::semaphoreGive(void)
{
	return xSemaphoreGive(this->xI2CMasterMutex);
}