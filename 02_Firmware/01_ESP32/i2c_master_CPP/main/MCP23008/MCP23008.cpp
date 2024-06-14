 #include "MCP23008.h"

static i2c_device_config_t MCP23008_dev_cfg;			//I2C device configuration.	 Musi być globalny, a nie w kalsie, bo inaczej kod nie działa. static, aby nie był widoczny poza ten plik.



static i2c_master_dev_handle_t MCP23008_dev_handle;		//Type of I2C master bus device handle. Musi być globalny, a nie w kalsie, bo inaczej kod nie działa. static, aby nie był widoczny poza ten plik.

/*---------------------------------------------------------------
 * Konstruktor klasy ekspandera portów GPIO przez i2c.
 * Parameters:
 * uint8_t i2cDeviceOpcode - adres MCP23008 na magistrali i2c
 * my_i2c_master* i2cMasterBus - wskaźnik do obiektu obsługującego
 *				transmisję i2c mastera
 * Returns:
 * NONE
*---------------------------------------------------------------*/
MCP23008::MCP23008(uint8_t i2cDeviceOpcode, my_i2c_master* i2cMasterBus)
{
	this->pI2cMasterBus = i2cMasterBus; //inaczej w destruktorze nie zadziałają metody z my_i2c_master
	DeviceOpcode = 	i2cDeviceOpcode;
	MCP23008_dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
	MCP23008_dev_cfg.device_address = DeviceOpcode;
	MCP23008_dev_cfg.scl_speed_hz = I2C_MASTER_SPEED;
	
	
	assert(!i2c_master_bus_add_device(*(this->pI2cMasterBus->phandler_i2c_bus)/*phandler_i2c_bus*/, &MCP23008_dev_cfg, &MCP23008_dev_handle));
	
	this->pI2cMasterBus->devicesOnBusIncrement();
	//i2cMasterBus->devicesOnBusIncrement();
	printf("MCP23008 GPIO expander has been initialised on i2c bus with address 0x%x.\r\n", DeviceOpcode);
}

/*---------------------------------------------------------------
 * Destruktor klasy ekspandera portów GPIO przez i2c.
 * Parameters:
 * NONE
 * Returns:
 * NONE
*---------------------------------------------------------------*/
MCP23008::~MCP23008()
{
	assert(!i2c_master_bus_rm_device(MCP23008_dev_handle));
	this->pI2cMasterBus->devicesOnBusDecrement();
	printf("MCP23008 GPIO  expander with address 0x%x has been deleted from i2c bus.\r\n", DeviceOpcode);
	
}

/*---------------------------------------------------------------
 * Metoda buduje ramkę danych i2c (patrz dokumentacja protokołu i2c
 * w ESP-IDF), która jest wysyłana po i2c do ekspandera GPIO w celu
 * odczytania wartości z rejestru.
 * Parameters:
 * uint8_t* data - wskaźnik do pamięci zawierającej dane:
 * 		data[0] - adres rejestru jaki ma być odczytany,
 *  		data[1] - wartość danej jaka została odczytana
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::readData(uint8_t* data)
{
	uint8_t* write_buffer = &data[0];
	uint8_t* read_buffer = &data[1];
	return i2c_master_transmit_receive(MCP23008_dev_handle, write_buffer, sizeof(uint8_t), read_buffer, sizeof(uint8_t), 150);
}


/*---------------------------------------------------------------
 * Metoda buduje ramkę danych i2c (patrz dokumentacja protokołu i2c
 * w ESP-IDF), która jest wysyłana po i2c do ekspandera GPIO w celu
 * aktualizacji (zapisania) wartości w rejestrze).
 * Parameters:
 * uint8_t* data - wskaźnik do pamięci zawierający:
 *		data[0] - adres rejestru jaki ma być zapisany,
 *  		data[1] - wartość danej jaka ma być zapisana w rejestrze
 * size_t len - długość ramki danych
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeData(uint8_t* data, size_t len)
{
	uint8_t* write_buffer = &data[0];	
	return i2c_master_transmit(MCP23008_dev_handle, write_buffer, len, 150);
}


/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze IODIR.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeIODIR(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_IODIR;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze IPOL.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeIPOL(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_IPOL;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze GPINTEN.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeGPINTEN(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_GPINTEN;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze DEFVAL.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeDEFVAL(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_DEFVAL;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze INTCON.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeINTCON(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_INTCON;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze IOCON.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeIOCON(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_IOCON;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze GPPU.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeGPPU(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_GPPU;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze INTF.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeINTF(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_INTF;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze GPIO.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeGPIO(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_GPIO;
	*(data + 1) = value;
	return this->writeData(data, 2);
}

/*---------------------------------------------------------------
 * Metoda zapisuje (aktualizuje) wartości w rejestrze OLAT.
 * Parameters:
 * uint8_t value - wartość jaka ma być zapisana w rejestrze
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie 
*---------------------------------------------------------------*/
esp_err_t MCP23008::writeOLAT(uint8_t value)
{
	uint8_t data[2];
	*data = MCP23008_OLAT;
	*(data + 1) = value;
	return this->writeData(data, 2);
}


/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru IODIR.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readIODIR()
{
	uint8_t data[2];
	data[0] = MCP23008_IODIR;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru IPOL.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readIPOL()
{
	uint8_t data[2];
	data[0] = MCP23008_IPOL;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru GPINTEN.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readGPINTEN()
{
	uint8_t data[2];
	data[0] = MCP23008_GPINTEN;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru DEFVAL.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readDEFVAL()
{
	uint8_t data[2];
	data[0] = MCP23008_DEFVAL;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru INTCON.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readINTCON()
{
	uint8_t data[2];
	data[0] = MCP23008_INTCON;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru IOCON.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readIOCON()
{
	uint8_t data[2];
	data[0] = MCP23008_IOCON;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru GPPU.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readGPPU()
{
	uint8_t data[2];
	data[0] = MCP23008_GPPU;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru INTF.
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readINTF()
{
	uint8_t data[2];
	data[0] = MCP23008_INTF;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru INTCAP(
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readINTCAP()
{
	uint8_t data[2];
	data[0] = MCP23008_INTCAP;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru GPIO
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readGPIO()
{
	uint8_t data[2];
	data[0] = MCP23008_GPIO;
	this->readData(&data[0]);
	return *(data + 1);
}

/*---------------------------------------------------------------
 * Metoda odczytuje wartości z rejestru OLAT
 * Parameters:
 * NONE
 * Returns:
 *  uint8_t *(data+1) - wartośc odczytana z rejestru 
*---------------------------------------------------------------*/
uint8_t MCP23008::readOLAT()
{
	uint8_t data[2];
	data[0] = MCP23008_OLAT;
	this->readData(&data[0]);
	return *(data + 1);
}
