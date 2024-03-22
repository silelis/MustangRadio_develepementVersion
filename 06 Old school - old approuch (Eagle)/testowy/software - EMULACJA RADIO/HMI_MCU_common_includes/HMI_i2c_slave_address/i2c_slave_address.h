#define i2c_slave_address        0x10
#define i2c_slave_fake_address   0x12

#define i2c_slave_ready()            I2C_SlaveAddr(i2c_slave_address)            //ustawia w�a�ciwy adres slave, wtedy master otrzymuje ACK = 0 czyli ok
#define i2c_slave_not_ready()        I2C_SlaveAddr(i2c_slave_fake_address)       //ustawia fa�szywy adres slave (jak �adnego innego urz�dzenia w uk�adzie), wtedy master nie otrzymuje �adnego ACK

