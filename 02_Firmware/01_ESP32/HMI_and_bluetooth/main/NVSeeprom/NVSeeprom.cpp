#include "NVSeeprom.h"

/*---------------------------------------------------------------
 * Konstruktor klasy obsługującej NVS.
 * Parameters:
 * const char* nameSpace - wskaźnik char do tablicy tekstowej
 *       zawierającej nazwę wokrspace NVS, w ktorym zapisywane
 *       są dane.
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
NVS::NVS(const char* nameSpace)
{
	this->err=nvs_flash_init();
	if (this->err == ESP_ERR_NVS_NO_FREE_PAGES || this->err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		switch (this->err)
		{
		case ESP_ERR_NVS_NO_FREE_PAGES:
			printf("ESP_ERR_NVS_NO_FREE_PAGES\n");
		case ESP_ERR_NVS_NEW_VERSION_FOUND:
			printf("ESP_ERR_NVS_NEW_VERSION_FOUND\n");
		}
		this->ereaseAndInit();
	}
	this->nvsPartitionStatsPrintf();
	
	
	namespaceName = new char[strlen(nameSpace) + 1];
	strcpy(const_cast<char*>(namespaceName), nameSpace);
	this->err= nvs_open(namespaceName, NVS_READWRITE, &this->nvsHandle);
	assert(!this->err);
	this->commitIsNotRequired();
}

/*---------------------------------------------------------------
 * Funkcja sprawdza czy wymagany jest commit (dane w NVS zostały
 * zapisane), a jesli tak to wykonje commit i uaktualnia
 * commitRequirmentState na FALSE.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void NVS::doCommitIfRequired()
{
	if (this->isCommitRequired()) {
		nvs_commit(this->nvsHandle);
		this->commitIsNotRequired();
	}
}

/*---------------------------------------------------------------
 *Funkcja ustawia commitRequirmentState na FALSE.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void NVS::commitIsNotRequired()
{
	this->commitRequirmentState = pdFALSE;
}

/*---------------------------------------------------------------
 * Funkcja sprawdza czy wymagany jest commit.
 * Parameters:
 * NONE
 * Returns:
 * bool commitRequirmentState - TRUE jeśli commit jest wymagany,
 *            FALSE jeśli commit jest niewymagany.
 *---------------------------------------------------------------*/
bool NVS::isCommitRequired()
{
	return this->commitRequirmentState;
}

/*---------------------------------------------------------------
 *Funkcja ustawia commitRequirmentState na TRUE.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void NVS::commitWillBeRequired()
{
	this->commitRequirmentState = pdTRUE;
}

/*---------------------------------------------------------------
 *Funkcja wypisuje na UART parametry partycji NVS
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void NVS::nvsPartitionStatsPrintf()
{
	this->getPartitionStats();
	printf("NVS partition stats:\n\t Total entries = %lu,\n\t Free entries = %lu,\n\t Used entries = %lu,\n\t Namespace count= %lu\n",
		(long unsigned) this->nvsPartitionStats.total_entries,
		(long unsigned) this->nvsPartitionStats.free_entries,
		(long unsigned) this->nvsPartitionStats.used_entries,
		(long unsigned) this->nvsPartitionStats.namespace_count);
}

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość int8_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * int8_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_i8(const char *key, int8_t value)
{
	esp_err_t retVal = nvs_set_i8(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	;
	return retVal;	
}

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość uint8_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * uint8_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_u8(const char *key, uint8_t value)
{
	esp_err_t retVal = nvs_set_u8(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość int16_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * int16_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_i16(const char *key, int16_t value)
{
	esp_err_t retVal = nvs_set_i16(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}	

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość uint16_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * uint16_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_u16(const char *key, uint16_t value)
{
	esp_err_t retVal = nvs_set_u16(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość int32_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * int32_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_i32(const char *key, int32_t value)
{
	esp_err_t retVal = nvs_set_i32(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}	

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość uint32_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * uint32_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_u32(const char *key, uint32_t value)
{
	esp_err_t retVal = nvs_set_u32(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}	

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość int64_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * int64_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_i64(const char *key, int64_t value)
{
	esp_err_t retVal = nvs_set_i64(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}	

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość uint64_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * uint64_t value - wartość zmiennej, jaka zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_u64(const char *key, uint64_t value)
{
	esp_err_t retVal = nvs_set_u64(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość tablicy (array) typu char (array).
 * Długość tablicy rozpoznawana jest po wystąpieniu na końcu
 * tablicy "\n".
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * const char *value - wskaźnik to tablicy typu char, której
 *             wartość zostanie zapisana w NVS
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_str(const char *key, const char *value)
{
	esp_err_t retVal = nvs_set_str(this->nvsHandle, key, value);
	this->commitWillBeRequired();
	return retVal;	
}	

/*---------------------------------------------------------------
 * Funkcja zapisuje wartość typu bloob, tzn. dowolną wartość o
 * zdefiniowanej długości.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zostanie wartość.
 * const void *value - wskaźnik to pamięci zawierającej dane do
                zapisania,
 * size_t length - długość danych do zapisania,
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::set_blob(const char *key, const void *value, size_t length) {
	esp_err_t retVal = nvs_set_blob(this->nvsHandle, key, value, length);
	this->commitWillBeRequired();
	return retVal;	
}	

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość int8_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * int8_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu int8_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_i8(const char *key, int8_t *outValue)
{
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_i8(this->nvsHandle, key, outValue);
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość uint8_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * uint8_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu uint8_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_u8(const char *key, uint8_t* outValue)
{
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_u8(this->nvsHandle, key, outValue);
	return retVal;
}	

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość int16_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * int16_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu int16_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_i16(const char *key, int16_t*outValue)
{
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_i16(this->nvsHandle, key, outValue);
	return retVal;
}	

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość uint16_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * uint16_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu uint16_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_u16(const char *key, uint16_t*outValue) {
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_u16(this->nvsHandle, key, outValue);
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość int32_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * int32_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu int32_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_i32(const char *key, int32_t*outValue)
{
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_i32(this->nvsHandle, key, outValue);
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość uint32_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * uint32_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu uint32_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_u32(const char *key, uint32_t* outValue) {
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_u32(this->nvsHandle, key, outValue);
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość int64_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * int64_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu int64_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_i64(const char *key, int64_t*outValue) {
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_i64(this->nvsHandle, key, outValue);
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość uint64_t.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * uint64_t *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu uint64_t,
 *              w innym przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_u64(const char *key, uint64_t*outValue) {
	this->doCommitIfRequired();
	esp_err_t retVal = nvs_get_u64(this->nvsHandle, key, outValue);
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość str.
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * char *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej typu str wraz
 *              z "/n", w innym przypadku nastąpić może nadpisanie
 *              pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_str(const char *key, char *out_value) {//, size_t *length) {
	this->doCommitIfRequired();
	size_t length;
	esp_err_t retVal = nvs_get_str(this->nvsHandle, key, NULL, &length);		//jeżeli length=0, to w pierwszej iteracji nvs_get_str sprawdza długość stringu
	retVal = nvs_get_str(this->nvsHandle, key, out_value, &length);				//w kolejnej iteracji, gdy length>0 nvs_get_str odczytuje dane i miejsca w pamięci wskazanego przez wsaźnik
	return retVal;
}

/*---------------------------------------------------------------
 * Funkcja odczytuje wartość bloob (zmienną o dowolnej długości).
 * Parameters:
 * const char *key - wskaźnik do tablicy znakowej zawierającej
 *              nazwę klucza (aka. nazwę zmiennej) pod jaką w NVS
 *              zapisana zapisana wartość.
 * void *outValue - wskaźnik do pamięci, w której powinna zostać
 *              umieszczona odczywana z NVS wartość.
 *              UWAGA - należy zadbać, aby długość pamięci outValue
 *              była odpowiednia do umieszczenia danej, w innym
 *              przypadku nastąpić może nadpisanie pamięci.
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::get_blob(const char *key, void *out_value){//, size_t *length)

	this->doCommitIfRequired();
	size_t length;
	esp_err_t retVal = nvs_get_blob(this->nvsHandle, key, NULL, &length);		//jeżeli length=0, to w pierwszej iteracji nvs_get_blob sprawdza długość bloob'a
	retVal = nvs_get_blob(this->nvsHandle, key, out_value, &length);			//w kolejnej iteracji, gdy length>0 nvs_get_blob odczytuje dane i miejsca w pamięci wskazanego przez wsaźnik
	return retVal;
}


/*---------------------------------------------------------------
 * Funkcja odczytuje parametry partycji NVS.
 * Parameters:
 * NONE
 * Returns:
 * esp_err_t retVal - 0x0 jeśli operacja przebiegła pomyślnie
 *---------------------------------------------------------------*/
esp_err_t NVS::getPartitionStats()
{
	return nvs_get_stats(NVS_PARTITION_NAME_DEFAULT, &this->nvsPartitionStats);
}

/*---------------------------------------------------------------
 * Funkcja czyści partycję NVS ze wszelkich danych i dokonuje
 * inicjalizacji pratycji NVS. Jest to metoda publiczna, w związku
 * z czym na wszelki wypadek dodano count down podczas którego
 * użytkownik ma pozliwość przerwania doliczania.
 * Parameters:
 * uint8_t countdownTime - czas jaki użytkownik ma na zatrzymanie
 *				odliczania do momenty rozpoczęcia wymazywania 
 *				danych z NVS
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void NVS::CAUTION_NVS_ereaseAndInit(uint8_t countdownTime)
{
	printf("!!!CAUTION!!!, this function will erase all data from NVS flash in:.\n");
	for (int timeout = countdownTime; timeout > 0; timeout--)
	{
		printf(" %i second(s)\n", timeout);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	this->ereaseAndInit();
}

/*---------------------------------------------------------------
 * Funkcja czyści partycję NVS ze wszelkich danych i dokonuje
 * inicjalizacji pratycji NVS. Jest to metoda prywatna.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
void NVS::ereaseAndInit()
{
	printf("NVS flash is eraseing...\n");
	this->err = nvs_flash_erase();
	printf("NVS flash erased.\n");
	assert(!this->err);
	this->err = nvs_flash_init();
	printf("NVS flash initialised.\n");
	assert(!this->err);
}


/*---------------------------------------------------------------
 * Destruktor klasy.
 * Parameters:
 * NONE
 * Returns:
 * NONE
 *---------------------------------------------------------------*/
NVS::~NVS()
{
	//todo: commit all data if required
	this->doCommitIfRequired();
	nvs_close(this->nvsHandle);
	assert(!this->err);
	this->err = nvs_flash_deinit();
	assert(!this->err);
}
