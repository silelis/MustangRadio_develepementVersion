#include "MCP23008.h"

/*---------------------------------------------------------------
 * Konstruktor klasy ekspandera portów GPIO przez i2c.
 * Parameters:
 * uint8_t i2cDeviceOpcode - adres MCP23008 na magistrali i2c
 * int pinSDA - pin SDA interfejsu i2c (parmetr niezbędny dla
 *              konstruktora dziedziczonego klasy i2cMaster)
 * int pinSCL - pin SCL interfejsu i2c (parmetr niezbędny
 *              dla konstruktora dziedziczonego klasy i2cMaster)
 * uint32_t i2cSpeed - prętkość transmisji i2c (parmetr niezbędny
 *               dla konstruktora dziedziczonego klasy i2cMaster)
 * size_t rxBuffLen  - wielkość bufora nadawania (parmetr niezbędny
 *               dla konstruktora dziedziczonego klasy i2cMaster)
 * size_t txBuffLen  - wielkość bufora odbioru (parmetr niezbędny
 *               dla konstruktora dziedziczonego klasy i2cMaster)
 * Returns:
 * NONE
*---------------------------------------------------------------*/
MCP23008::MCP23008(uint8_t i2cDeviceOpcode, int pinSDA, int pinSCL, uint32_t i2cSpeed, size_t rxBuffLen, size_t txBuffLen)
	: i2cMaster(pinSDA, pinSCL, i2cSpeed, rxBuffLen, txBuffLen)
{
	//todo: get instance counter value,
	//todo: PANIC ERROR IF I2C MASTER NOT INIOTIALISED
	//TOTO:	BROWNOUT DETECTION
	
	ESP_LOGI(this->TAG, "Create MCP23008 configuration.");
	if (this->geti2cInstanceCounterState() == 0)
	{
		ESP_LOGE(TAG, "I2C master configuration error.");
	}
	else
	{
		//TODO: i2cPing to MCP23008 w ifie
		//this->writeBuffoLen = sizeof(this->writeBuffor);
		//this->readBufforLen = sizeof(this->readBuffor);
		this->DeviceOpcode = i2cDeviceOpcode;
	}	
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
	ESP_LOGI(this->TAG, "MCP23008 destruction.");	

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
	i2c_cmd_handle_t cmd_write = i2c_cmd_link_create();
	i2c_master_start(cmd_write);
	i2c_master_write_byte(cmd_write, (this->DeviceOpcode << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	
	i2c_master_write(cmd_write, data, len, I2C_MASTER_ACK/*1*/);
	i2c_master_stop(cmd_write);
	
	esp_err_t retVal = this->i2cWriteData(cmd_write);
	i2c_cmd_link_delete(cmd_write);
	return retVal;	
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
	i2c_cmd_handle_t cmdRead = i2c_cmd_link_create();
	i2c_master_start(cmdRead);
	i2c_master_write_byte(cmdRead, (this->DeviceOpcode << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmdRead, data[0], I2C_MASTER_NACK);
	i2c_master_start(cmdRead);
	i2c_master_write_byte(cmdRead, (this->DeviceOpcode << 1) | I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_read_byte(cmdRead, data+1, I2C_MASTER_ACK);
	i2c_master_stop(cmdRead);
	
	esp_err_t retVal = this->i2cWriteData(cmdRead);
	i2c_cmd_link_delete(cmdRead);
	return retVal;
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
	*(data+1) = value;
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
	return *(data+1);
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
