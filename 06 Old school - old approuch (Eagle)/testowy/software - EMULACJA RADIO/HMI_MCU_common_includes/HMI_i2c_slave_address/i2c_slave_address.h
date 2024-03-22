#define i2c_slave_address        0x10
#define i2c_slave_fake_address   0x12

#define i2c_slave_ready()            I2C_SlaveAddr(i2c_slave_address)            //ustawia w³aœciwy adres slave, wtedy master otrzymuje ACK = 0 czyli ok
#define i2c_slave_not_ready()        I2C_SlaveAddr(i2c_slave_fake_address)       //ustawia fa³szywy adres slave (jak ¿adnego innego urz¹dzenia w uk³adzie), wtedy master nie otrzymuje ¿adnego ACK

