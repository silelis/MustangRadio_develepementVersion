#include "my_i2c_slave.h"


i2c_slave::i2c_slave()
{
	//inicjalizowanie drivera i2c slave
	this->i2cSlavePort = I2C_SLAVE_PORT;
	assert(!my_i2c_slave_init(I2C_MODE_SLAVE, I2C_SLAVE_ADDRESS, this->i2cSlavePort, I2C_SLAVE_PIN_SDA, I2C_SLAVE_PIN_SCL, I2C_SLAVE_RX_BUFFER_LEN, I2C_SLAVE_TX_BUFFER_LEN));
	assert(!i2c_reset_tx_fifo(this->i2cSlavePort));
	assert(!i2c_reset_rx_fifo(this->i2cSlavePort));
	
	//tworzenie semafora kontrolującego komunikację i2c slave
	this->xI2CSlaveMutex = xSemaphoreCreateMutex();
	xSemaphoreGive(this->xI2CSlaveMutex);
	
	//inicjalizuje piun i2cSlave communication/ interrupt request
	
	
	
	
	printf("I2C slave communication on port %d initialized.\r\n", this->i2cSlavePort);
	
}

i2c_slave::~i2c_slave()
{
	assert(!i2c_driver_delete (this->i2cSlavePort));
	vSemaphoreDelete(!this->xI2CSlaveMutex);
	printf("I2C slave communication on port %d deinitialized.\r\n", this->i2cSlavePort);
}