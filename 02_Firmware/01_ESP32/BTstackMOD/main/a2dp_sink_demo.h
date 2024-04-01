#pragma once

#include "btstack_port_esp32.h"
#include "btstack_run_loop.h"
#include "btstack_stdio_esp32.h"
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"
#include <stddef.h>
#include "esp_err.h"
#include <inttypes.h>
#include <stdint.h>
//nclude <stdio.h>
#include <string.h>
#include "btstack.h"

#include "btstack_resample.h"

//functions required by Mustang radio hardware
#include "driver/gpio.h"

#if !CONFIG_BT_ENABLED
#error "Bluetooth disabled - please set CONFIG_BT_ENABLED via menuconfig -> Component Config -> Bluetooth -> [x] Bluetooth"
#endif
#if !CONFIG_BT_CONTROLLER_ONLY
#error "Different Bluetooth Host stack selected - please set CONFIG_BT_CONTROLLER_ONLY via menuconfig -> Component Config -> Bluetooth -> Host -> Disabled"
#endif
#if ESP_IDF_VERSION_MAJOR >= 5
#if !CONFIG_BT_CONTROLLER_ENABLED
#error "Different Bluetooth Host stack selected - please set CONFIG_BT_CONTROLLER_ENABLED via menuconfig -> Component Config -> Bluetooth -> Controller -> Enabled"
#endif
#endif


int btstack_main(int argc, const char * argv[]);
esp_err_t i2sPinsHighImpedanceDisabled();
esp_err_t i2sPinsHighImpedanceEnabled();
void stdin_process(char cmd, bd_addr_t device_addr);