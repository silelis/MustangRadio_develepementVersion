#include "my_i2c_slave.h"


i2c_slave::i2c_slave()
{
	//inicjalizowanie drivera i2c slave
	this->i2cSlavePort = I2C_SLAVE_PORT;
	assert(!i2c_slave_init(I2C_MODE_SLAVE, I2C_SLAVE_ADDRESS, this->i2cSlavePort, I2C_SLAVE_PIN_SDA, I2C_SLAVE_PIN_SCL, I2C_SLAVE_RX_BUFFER_LEN, I2C_SLAVE_TX_BUFFER_LEN));
	assert(!i2c_reset_tx_fifo(this->i2cSlavePort));
	assert(!i2c_reset_rx_fifo(this->i2cSlavePort));
	
	//tworzenie semafora kontrolującego komunikację i2c slave
	this->xI2CSlaveMutex = xSemaphoreCreateMutex();
	xSemaphoreGive(this->xI2CSlaveMutex);
	
	//inicjalizuje piun i2cSlave communication/ interrupt request
	gpio_config_t pinInterruptRequest;
	/*this->*/pinInterruptRequest.pin_bit_mask	= (1 << I2C_SLAVE_PIN_INTERRUPT_REQUEST);
	/*this->*/pinInterruptRequest.mode = GPIO_MODE_OUTPUT_OD;
	/*this->*/pinInterruptRequest.pull_up_en = GPIO_PULLUP_ENABLE;
	/*this->*/pinInterruptRequest.pull_down_en = GPIO_PULLDOWN_DISABLE;
	/*this->*/pinInterruptRequest.intr_type	= GPIO_INTR_DISABLE;
	assert(!gpio_config(/*this->*/&pinInterruptRequest));
	this->resetInterruptRequest();
	
	printf("I2C slave communication on port %d initialized.\r\n", this->i2cSlavePort);
	
}

void i2c_slave::setInterruptRequest()
{
	gpio_set_level(I2C_SLAVE_PIN_INTERRUPT_REQUEST, I2C_SLAVE_INTERRUPT_REQUEST_LEVEL_ON);
}
void i2c_slave::resetInterruptRequest()
{
	gpio_set_level(I2C_SLAVE_PIN_INTERRUPT_REQUEST, I2C_SLAVE_INTERRUPT_REQUEST_LEVEL_OFF);
}

i2c_slave::~i2c_slave()
{
	assert(!i2c_driver_delete (this->i2cSlavePort));
	vSemaphoreDelete(!this->xI2CSlaveMutex);
	printf("I2C slave communication on port %d deinitialized.\r\n", this->i2cSlavePort);
}