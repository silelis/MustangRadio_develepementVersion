 #include "MCP23008.h"

static i2c_device_config_t MCP23008_dev_cfg;
static i2c_master_dev_handle_t MCP23008_dev_handle;

MCP23008::MCP23008(uint8_t i2cDeviceOpcode, my_i2c_master* i2cMasterBus)
{
	this->pI2cMasterBus = i2cMasterBus; //inaczej w destruktorze nie zadzia³aj¹ metody z my_i2c_master
	DeviceOpcode = 	i2cDeviceOpcode;
	MCP23008_dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
	MCP23008_dev_cfg.device_address = DeviceOpcode;
	MCP23008_dev_cfg.scl_speed_hz = I2C_MASTER_SPEED;
	
	
	assert(!i2c_master_bus_add_device(*(this->pI2cMasterBus->phandler_i2c_bus)/*phandler_i2c_bus*/, &MCP23008_dev_cfg, &MCP23008_dev_handle));
	
	this->pI2cMasterBus->devicesOnBusIncrement();
	//i2cMasterBus->devicesOnBusIncrement();
	printf("MCP23008 GPIO expander has been initialised on i2c bus with address 0x%x.\r\n", DeviceOpcode);
}

MCP23008::~MCP23008()
{
	assert(!i2c_master_bus_rm_device(MCP23008_dev_handle));
	this->pI2cMasterBus->devicesOnBusDecrement();
	printf("MCP23008 GPIO  expander with address 0x%x has been deleted from i2c bus.\r\n", DeviceOpcode);
	
}