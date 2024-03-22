#include "i2c_master.h"

/*----------------------------------------------------------------------------------
*uint8_t i2cInstanceCounter
*Zmienna statyczna, o takiej samej wartości, dla wszystkich 
*obiektów posługujących się klasą i2cMaster.
*Zadaniem zmiennej jest regulowanie poprawnym działaniem konstruktora i destruktora
*obiektów.
* 
* Za każdym razem gdy obiekt posługujący się kalsą i2cMaster jest konstrukowany
* zmienna jest inkrementowana, a gdy obiekt jest niszczony zmienna jest dekrementowana.
* Gdy zmienna ma warość:
* - 0 i obiekt jest konstrukowany następuje pełna konstgrukcja interfejsu i2c, gdy
*			wartość jest większa niż 0 konstrukcja nie następuje.
* - 1 i obiekt jest niszczony następuje zniszczenie uruchomienie destruktora i2cmaster,
*			gdy zmienna ma wartośc większa niż 1 destruktor nie jest uruchamiany, bo
*			istnieją jeszcze obiekty posługujące się interfejsem i2c.
*----------------------------------------------------------------------------------*/
uint8_t i2cMaster::i2cInstanceCounter = 0;

/*----------------------------------------------------------------------------------
* int i2cMasterPort;
* Zmienna statyczna, o takiej samej wartości, dla wszystkich obiektów posługujących
* się klasą i2cMaster. Numer portu, którym można zarządzać komunikacją I2C w trybie master. 
*----------------------------------------------------------------------------------*/
int i2cMaster::i2cMasterPort;

/*----------------------------------------------------------------------------------
* i2c_config_t i2cConfig;
* Struktura statyczna, o takiej samej wartości, dla wszystkich obiektów posługujących
* się klasą i2cMaster, która jest przechowuje konfigurację dla interfejsu komunikacyjnego
* I2C. 
*----------------------------------------------------------------------------------*/
i2c_config_t i2cMaster::i2cConfig;

/*----------------------------------------------------------------------------------
* SemaphoreHandle_t xI2CMasterMutex;
* Semafor statyczny o takiej samej wartości, dla wszystkich obiektów posługujących
* się klasą i2cMaster. Zadaniem semafora jest zabezpieczenie przed błędem komunikacji
* spowodowanym jednoczesnym posługiwaniem się interfejsem i2c przez wiele obiektów
* posługujących się klasą i2cMaster.
*----------------------------------------------------------------------------------*/
SemaphoreHandle_t i2cMaster::xI2CMasterMutex=NULL;


/*----------------------------------------------------------------------------------
* Konstruktor interfejsu komunikacji ESP32 po i2c z peryferiami typu slave.
* ESP32 działa jako master.
* Klasa zaprojektowana w sposób pozwalający na jej dziedziczenie w wielu równoległych
* klasach, jak również konstruowanie wielu obiektow. Kluczowe do działania komunikacji
* i2c zmienne zdefiniowano jako static, co powoduje, że w obiektach, w których klasa jest
* wywoływana deklarowane sa tylko raz. Mechanizm ten kontrolowany jest poprzez zmienną 
* "i2cInstanceCounter". Jeżleli zmienna "i2cInstanceCounter" ma wartość większą od 0 to
* znaczy, że nastąła już inicjalizacja interfejsu i2c i nie należy jej powtażać.
*
* Parameters:
* int pinSDA 		- pin SDA interfejsu i2c
* int pinSCL 		- pin SCL interfejsu i2c
* uint32_t i2cSpeed	- prętkość transmisji i2c
* size_t rxBuffLen	- wielkość bufora nadawania
* size_t txBuffLen	- wielkość bufora odbioru
* Returns:
* NONE
*----------------------------------------------------------------------------------*/
i2cMaster::i2cMaster(int pinSDA, int pinSCL, uint32_t i2cSpeed, size_t rxBuffLen, size_t txBuffLen)
{
	//esp_err_t retVal;
	if (i2cInstanceCounter>0)
	{
		ESP_LOGI(this->TAG, "I2C had already been configured.");
		i2cInstanceCounter++;
	}
	else if (i2cInstanceCounter==0)
	{
		xI2CMasterMutex = xSemaphoreCreateMutex();
		
		ESP_LOGI(this->TAG, "Create I2C master configuration.");
		i2cMasterPort = 1;
					
		i2cConfig.mode = I2C_MODE_MASTER;
		i2cConfig.sda_io_num = pinSDA;	//GPIO_NUM_19;
		i2cConfig.scl_io_num = pinSCL;	//GPIO_NUM_18;
		//i2c_conf.sda_pullup_en = 0;
		//i2c_conf.scl_pullup_en = 0;
		i2cConfig.master.clk_speed = i2cSpeed; //400khz 
		i2cConfig.clk_flags = 0;
		
		i2c_param_config(i2cMasterPort, &i2cConfig);
 
		
		if ((i2c_driver_install(i2cMasterPort, i2cConfig.mode, rxBuffLen, txBuffLen, 0) == ESP_OK)&& (xI2CMasterMutex !=NULL))
		{
			
			/*if (i2cSpeed > 400000)
			{
				i2c_filter_enable(i2cMasterPort, 5);
			}*/		
			i2cInstanceCounter++;			
			ESP_LOGI(this->TAG, "I2C has been configured.");
			xSemaphoreGive(xI2CMasterMutex);
		}
		else
		{
			ESP_LOGI(this->TAG, "I2C configuration error.");
			if (i2c_driver_install(i2cMasterPort, i2cConfig.mode, rxBuffLen, txBuffLen, 0) != ESP_OK)
			{
				ESP_LOGI(this->TAG, "I2C hardware configuration error");
				assert(!i2c_driver_install(i2cMasterPort, i2cConfig.mode, rxBuffLen, txBuffLen, 0));
			}
			if (xI2CMasterMutex == NULL)
			{
				ESP_LOGI(this->TAG, "I2C mutex configuration error");
				assert(xI2CMasterMutex);
			}
		}
	}

}

/*----------------------------------------------------------------------------------
* Destruktor interfejsu komunikacji ESP32 po i2c z peryferiami typu slave.
* Klasa zaprojektowana w sposób pozwalający na jej dziedziczenie w wielu równoległych
* klasach, jak również destrukcję kolejnych obiektów. Kluczowe do działania komunikacji
* i2c zmienne zdefiniowano jako static, co powoduje, że w obiektach, w których klasa jest
* wywoływana nie są one niszczone dopuki istnieje conajmniej jeden obiekt dziedzicączy tę
* klasę. Mechanizm ten kontrolowany jest poprzez zmienną "i2cInstanceCounter". Jeżleli
* zmienna "i2cInstanceCounter" ma wartość 1 to znaczy, że pozostał tylko jeden obiekt
* wykorzystujący interfejc i2c i należy w pełni wykonac destrukcję.
* Parameters:
* NONE
* Returns:
* NONE
*----------------------------------------------------------------------------------*/	
i2cMaster::~i2cMaster()
{
	if (i2cInstanceCounter == 1)
	{
		//i2c_cmd_link_delete_static	// - to consider if it is required
		//i2c_cmd_link_delete			// - to consider if it is required
		if (xSemaphoreTake(xI2CMasterMutex, portMAX_DELAY) == pdTRUE)
		{
			vSemaphoreDelete(xI2CMasterMutex);
			i2c_reset_tx_fifo(i2cMasterPort);
			i2c_reset_rx_fifo(i2cMasterPort);
			i2c_driver_delete(i2cMasterPort);			
		}

	}
	i2cInstanceCounter--;
}

/*-----------------------------------------------------------------------------------
* Funkcja komunikuje doponuje próby komunikacji z urządzeniem slave. jest to funkcja
* testowa, która sprawdza czy urządzenie o danym adresie istniej/ działa/ odpowiada na
* komunikajcę.
* Parameters:
* uint8_t i2c_address - adres i2c urządzenia (7 bit'ów)
* Returns:
* esp_err_t retVa - ESP_OK		0 	No error
*		  ESP_FAIL		-1	General failure
*		  ESP_ERR_NO_MEM	0x101	Out of memory
*		  ESP_ERR_INVALID_ARG	0x102	Invalid argument
*----------------------------------------------------------------------------------*/
esp_err_t i2cMaster::i2cPing(uint8_t i2c_address)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (i2c_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_stop(cmd);
	
	xSemaphoreTake(xI2CMasterMutex, portMAX_DELAY);
	esp_err_t retVal = i2c_master_cmd_begin(i2cMasterPort, cmd, 10000);
	xSemaphoreGive(xI2CMasterMutex);
	i2c_cmd_link_delete(cmd);
	
	if (retVal == ESP_OK)
	{
		ESP_LOGI(TAG, "Device found at address 0x%02X", i2c_address);
		//return true;
	}
	else if (retVal == ESP_ERR_TIMEOUT)
	{
		ESP_LOGW(TAG, "No response from device at address 0x%02X", i2c_address);
		//return false;
	}
	else
	{
		ESP_LOGE(TAG, "Error %d during I2C communication", retVal);
		//return false;
	}
	
	return retVal;
}

/*----------------------------------------------------------------------------------
* Zadaniem funkcji jest wykonanie komunikacji po interfejcie i2c, w trybie master.
* W zależności od informacji zawartych w otrzymywanej zmiennej funkcja może dokonać
* odczytu (read) lub zapisu (write) do urządzenia typu slave, co zależy jedynie od
* sposobu zkonstruowania zmiennej.
* Funkcja wyposażona jest w kontrolę semafora blokującego możliwość rónoległego diostępu
* do interfejsu i2c kontrolera przez wiele wątków. Zapobiega to błedom w komunikacji.
* Parameters:
* i2c_cmd_handle_t cmd_handle - zmienna, a w rzeczywistości wskaźnik na strukturę, która
* 		przechowuje zestaw poleceń związanych z operacjami i2c, takimi jak
*  		zapisy i odczyty danych. Ta struktura jest używana do tworzenia
*     		sekwencji operacji i2c.
* Returns:      
* esp_err_t retVal - ESP_OK		0 	No error
* 		  ESP_FAIL		-1	General failure
* 		  ESP_ERR_NO_MEM	0x101	Out of memory
* 		  ESP_ERR_INVALID_ARG	0x102	Invalid argument	
*----------------------------------------------------------------------------------*/
esp_err_t i2cMaster::i2cWriteData(i2c_cmd_handle_t cmd_handle)
{
	xSemaphoreTake(xI2CMasterMutex, portMAX_DELAY);
	esp_err_t retVal = i2c_master_cmd_begin(i2cMasterPort, cmd_handle, portMAX_DELAY);
	xSemaphoreGive(xI2CMasterMutex);
	return retVal;
}


/*----------------------------------------------------------------------------------
* Funkcja zwraca wartość i2cInstanceCounter, który jest zmienna prywatną klasy.
* Parameters:
* NONE
* Returns:
* uint8_t i2cInstanceCounter - wartość przechowywana przez tą zmienna jest ilością
* 		obiektów, które posługują się klasą i2cMaster. 
*----------------------------------------------------------------------------------*/
uint8_t i2cMaster::geti2cInstanceCounterState()
{
	return i2cMaster::i2cInstanceCounter;
}
