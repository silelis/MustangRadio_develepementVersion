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
                    INCLUDE_DIRS ".")
