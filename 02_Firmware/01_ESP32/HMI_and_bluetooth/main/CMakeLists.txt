idf_component_register(
                    SRCS
                    	"exception_example_main.cpp"
                    	keyboard/keyboard.cpp
                    	i2c_master/i2c_master.cpp
                    	MCP23008/MCP23008.cpp
                    	Leds/Leds_backlight.cpp
                    	tasksFunctions/tasksFunctions.cpp
                    	NVSeeprom/NVSeeprom.cpp
                    	StepperOpto/StepperOpto.cpp
                    	a2dpSikn/a2dp_sink_demo.cpp
                    INCLUDE_DIRS ".")
