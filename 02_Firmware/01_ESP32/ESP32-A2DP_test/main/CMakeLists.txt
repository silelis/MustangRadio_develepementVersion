idf_component_register(SRCS 
                        "hello_world_main.cpp"
#                        src/BluetoothA2DPSource.cpp
#                        src/BluetoothA2DPSinkQueued.cpp
#                        src/BluetoothA2DPSink.cpp
#                        src/BluetoothA2DPOutput.cpp
#                        src/BluetoothA2DPCommon.cpp
                        ESP32-A2DP_class/ESP32-A2DP_class.cpp
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS
			"."
#			 "src"
                       REQUIRES
                        bt
                        nvs_flash
                        esp_ringbuf
                        ESP32-A2DP
                        arduino-audio-tools
                        )
