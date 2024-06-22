#pragma once

#include "stdio.h"
#include "string.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_partition.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "hwConfigFile.h"
#include "./../../../03_Common/comunicationProtocol.h"
//#include "comunicationProtocol.h"
#include <cstdint>
#include <inttypes.h>

class NVS
{
public:
	NVS(const char*);
	~NVS();
	
	void CAUTION_NVS_ereaseAndInit(uint8_t);
	void nvsPartitionStatsPrintf();
	esp_err_t set_i8(const char *,	int8_t);
	esp_err_t set_u8(const char *,	uint8_t);
	esp_err_t set_i16(const char *, int16_t);
	esp_err_t set_u16(const char *, uint16_t);
	esp_err_t set_i32(const char *, int32_t);
	esp_err_t set_u32(const char *, uint32_t);
	esp_err_t set_i64(const char *, int64_t);
	esp_err_t set_u64(const char *, uint64_t);
	esp_err_t set_str(const char *, const char *);
	esp_err_t set_blob(const char *, const void *, size_t);
		
	esp_err_t get_i8(const char *, int8_t *);
	esp_err_t get_u8(const char *, uint8_t*);
	esp_err_t get_i16(const char *, int16_t*);
	esp_err_t get_u16(const char *, uint16_t*);
	esp_err_t get_i32(const char *, int32_t*);
	esp_err_t get_u32(const char *, uint32_t*);
	esp_err_t get_i64(const char *, int64_t*);
	esp_err_t get_u64(const char *, uint64_t*);
	esp_err_t get_str(const char *, char *);//, size_t *);
	esp_err_t get_blob(const char *, void *);//, size_t *);
	
	
	
protected:
	esp_err_t getPartitionStats();
	void commitIsNotRequired();
	void commitWillBeRequired();
	bool isCommitRequired();
	void doCommitIfRequired();

private:
	const char* namespaceName;
	nvs_handle_t nvsHandle;
	nvs_stats_t nvsPartitionStats;
	esp_err_t err;
	void ereaseAndInit();
	bool commitRequirmentState;
	

};
