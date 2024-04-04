#pragma once

#include "btstack_port_esp32.h"
#include "btstack_run_loop.h"
#include "btstack_stdio_esp32.h"
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"

#include <stddef.h>

// warn about unsuitable sdkconfig
#include "sdkconfig.h"
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "btstack.h"

#include "btstack_resample.h"

int setup_demo(void);





typedef struct {
	uint8_t  a2dp_local_seid;
	uint8_t  media_sbc_codec_configuration[4];
} a2dp_sink_demo_stream_endpoint_t;

typedef enum {
	STREAM_STATE_CLOSED,
	STREAM_STATE_OPEN,
	STREAM_STATE_PLAYING,
	STREAM_STATE_PAUSED,
} stream_state_t;

typedef struct {
	uint8_t  reconfigure;
	uint8_t  num_channels;
	uint16_t sampling_frequency;
	uint8_t  block_length;
	uint8_t  subbands;
	uint8_t  min_bitpool_value;
	uint8_t  max_bitpool_value;
	btstack_sbc_channel_mode_t      channel_mode;
	btstack_sbc_allocation_method_t allocation_method;
} media_codec_configuration_sbc_t;

typedef struct {
	bd_addr_t addr;
	uint16_t  a2dp_cid;
	uint8_t   a2dp_local_seid;
	stream_state_t stream_state;
	media_codec_configuration_sbc_t sbc_configuration;
} a2dp_sink_demo_a2dp_connection_t;

