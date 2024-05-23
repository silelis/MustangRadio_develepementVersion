idf_component_register(
                    SRCS
                    	app_main.cpp
                    	keyboard/keyboard.cpp
                    	i2c_master/i2c_master.cpp
                    	MCP23008/MCP23008.cpp
                    	Leds/Leds_backlight.cpp
                    	tasksFunctions/tasksFunctions.cpp
                    	NVSeeprom/NVSeeprom.cpp
                    	StepperOptoPowerOFF/StepperOptoPowerOFF.cpp
                    	a2dpSikn/a2dp_sink_demo.cpp
                    	i2c_slave/i2c_slave.cpp
                    	i2c_slave/i2c_slave_hw_init.c
                    INCLUDE_DIRS ".")
