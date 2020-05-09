#include "nrf_esb.h"
#include <stdbool.h>
#include <stdint.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "boards.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "gs_timer.h"
#include "gs_ringbuf.h"

#define ESB_DEVICE_PIPE			0
#define ESB_RX_PAYLOAD_LENGTH	224

nrf_esb_payload_t rx_payload;
static bool g_esb_start = false;
void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
	struct gs_ringbuf *ringbuf = gs_ringbuf_get_by_devid(ESB_DEVICE_PIPE);
	uint8_t *rx_payload_addr = rf_request_payload_addr(ringbuf,ESB_RX_PAYLOAD_LENGTH);
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            //NRF_LOG_INFO("TX SUCCESS EVENT");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            //NRF_LOG_INFO("TX FAILED EVENT");
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            //NRF_LOG_INFO("RX RECEIVED EVENT");
            if (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {
				pipe_recived_bytes_per_sec[rx_payload.pipe] += rx_payload.length;
			
				if (rx_payload_addr != NULL) {
					memcpy(rx_payload_addr,rx_payload.data,rx_payload.length);
				}
				else {
					//NRF_LOG_INFO("Receiving packet error,No memory");
					gs_rf_drop_packets_total++;
				}
					rf_recived_bytes_per_sec += rx_payload.length;
                // Set LEDs identical to the ones on the PTX.
                //NRF_LOG_DEBUG("Receiving packet: %02x", rx_payload.data[1]);
            }
            break;
    }
}

uint32_t gs_esb_init( void )
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
    nrf_esb_config.payload_length           = 252;
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.selective_auto_ack       = false;

	//clocks_start();
    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

	//err_code =  nrf_esb_start_rx();
    NRF_LOG_INFO("esb init code = %d\n",err_code);
    return err_code;
}

uint32_t gs_esb_enable(void)
{
	uint32_t err_code = 0;
	g_esb_start = true;
	err_code =  nrf_esb_start_rx();
    NRF_LOG_INFO("esb enable code = %d\n",err_code);
	return err_code;
}

uint32_t gs_esb_disable(void)
{
	g_esb_start = false;
	return nrf_esb_stop_rx();
}

bool gs_esb_enabled(void)
{
	return g_esb_start;
}

/*lint -restore */
