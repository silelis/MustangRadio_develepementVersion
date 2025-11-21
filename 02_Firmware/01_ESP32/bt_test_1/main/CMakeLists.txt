idf_component_register(SRCS "exception_example_main.cpp"
                    bt_audio/bt_audio.cpp
                    PRIV_REQUIRES spi_flash
                    INCLUDE_DIRS "."
                        REQUIRES
                        bt
                        nvs_flash
                        esp_ringbuf
                        ESP32-A2DP
                        arduino-audio-tools
                    )
