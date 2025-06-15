idf_component_register(
                    SRCS
                    	app_main.cpp
                    	keyboard/keyboard.cpp
                    	MCP23008/MCP23008.cpp
                    	Leds/Leds_backlight.cpp
                    	tasksFunctions/tasksFunctions.cpp
                    	NVSeeprom/NVSeeprom.cpp
                    	StepperOptoPowerOFF/StepperOptoPowerOFF.cpp
                    	a2dpSikn/a2dp_sink_demo.cpp
                    	i2c_engine/i2c_engine.cpp
                    	common/i2c_slave_master_queueClass/i2c_slave_master_queueClass.cpp
                    	common/comunicationProtocol/comunicationProtocol.cpp
                    	common/comunicationStructures/comunicationStructures.cpp
                    	common/comunication_calculate_checksum/comunication_calculate_checksum.cpp
                    INCLUDE_DIRS ".")
