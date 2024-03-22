#pragma once

#include "i2c_master/i2c_master.h"
#include "esp_log.h"

#define MCP23008_IODIR		0x00		//I/O DIRECTION (IODIR) REGISTER
#define MCP23008_IPOL		0x01		//INPUT POLARITY (IPOL) REGISTER
#define MCP23008_GPINTEN	0x02		//INTERRUPT-ON-CHANGE CONTROL (GPINTEN) REGISTER
#define MCP23008_DEFVAL		0x03		//DEFAULT COMPARE (DEFVAL) REGISTER FOR INTERRUPT-ON-CHANGE 
#define MCP23008_INTCON		0x04		//INTERRUPT CONTROL (INTCON) REGISTER 		
#define MCP23008_IOCON		0x05		//CONFIGURATION (IOCON) REGISTER 
#define MCP23008_GPPU		0x06		//PULL-UP RESISTOR CONFIGURATION (GPPU) REGISTER
#define MCP23008_INTF		0x07		//INTERRUPT FLAG (INTF) REGISTER
#define MCP23008_INTCAP		0x08		//(Read-only) INTERRUPT CAPTURE (INTCAP) REGISTER
#define MCP23008_GPIO		0x09		//PORT (GPIO) REGISTER
#define MCP23008_OLAT		0x0A		//OUTPUT LATCH REGISTER (OLAT)


class MCP23008:public i2cMaster
{
	public:
	MCP23008(uint8_t i2cDeviceOpcode, int pinSDA, int pinSCL, uint32_t i2cSpeed, size_t rxBuffLen, size_t txBuffLen);
	~MCP23008();
	
	esp_err_t writeIODIR(uint8_t value);
	esp_err_t writeIPOL(uint8_t value);
	esp_err_t writeGPINTEN(uint8_t value);
	esp_err_t writeDEFVAL(uint8_t value);
	esp_err_t writeINTCON(uint8_t value);
	esp_err_t writeIOCON(uint8_t value);
	esp_err_t writeGPPU(uint8_t value);
	esp_err_t writeINTF(uint8_t value);
	//esp_err_t writeINTCAP(uint8_t);		// INTCAP register read-only
	esp_err_t writeGPIO(uint8_t value);
	esp_err_t writeOLAT(uint8_t value);
	
	
	uint8_t readIODIR();
	uint8_t readIPOL();
	uint8_t readGPINTEN();
	uint8_t readDEFVAL();
	uint8_t readINTCON();
	uint8_t readIOCON();
	uint8_t readINTCAP();
	uint8_t readGPPU();
	uint8_t readINTF();
	uint8_t readGPIO();
	uint8_t readOLAT();
	
	
	
protected:
	esp_err_t writeData(uint8_t* data, size_t len);
	esp_err_t readData(uint8_t* data);
	
private:
	const char *TAG = "MCP23008 log:";
	uint8_t DeviceOpcode;
};