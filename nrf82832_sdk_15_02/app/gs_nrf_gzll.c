/*************************************************************************
	> File Name: gs_rf.c
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月12日 星期二 11时15分38秒
 ************************************************************************/
#include <string.h>
#include "nrf_gzll.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_gzll_error.h"
#include "gs_timer.h"
#include "gs_prx.h"

static uint8_t		m_data_payload[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];
static uint8_t 	m_ack_payload[6] = {0xef,0x01,0x00,0x00,0x55,0xaa};
//static uint8_t 	m_ack_payload[] = "STOP";

static struct gs_prx *get_gs_prx(void)
{
	return &g_prx;
}

void gs_rf_clean_txfifo(uint32_t pipe)
{
	nrf_gzll_flush_tx_fifo	(pipe);
}

void gs_rf_clean_rxfifo(uint32_t pipe)
{
	nrf_gzll_flush_rx_fifo	(pipe);
}

void nrf_gzll_host_rx_data_ready(uint32_t pipe, nrf_gzll_host_rx_info_t rx_info)
{
	bool result_value = false;
	uint32_t rx_payload_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
	struct gs_ringbuf *ringbuf = gs_ringbuf_get_by_devid(pipe);
	uint8_t *rx_payload_addr = rf_request_payload_addr(ringbuf,rx_payload_length);
	struct gs_prx *prx = get_gs_prx();

	if (!rx_payload_addr) {
		rx_payload_addr = m_data_payload;
		gs_rf_drop_packets_total++;
	}
	else {
		//NRF_LOG_INFO("==>rf get device %d rx addr %p",pipe,rx_payload_addr);
	}

	// Pop packet and write first byte of the payload to the GPIO port.
	result_value = nrf_gzll_fetch_packet_from_rx_fifo(pipe,
			rx_payload_addr,&rx_payload_length);
	if (!result_value) {
		NRF_LOG_ERROR("RX fifo error ");
	}

	if (rx_payload_length != NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH) {
		/*heartbeat, sync device info */

		/*feedback timer to indicate connect infomation */
		//prx->timer.device[pipe].heartbeat = 0xff;
	}
	else {  // this is an audio data package
		if ( rx_payload_length > 0 ) {
			pipe_recived_bytes_per_sec[pipe] += rx_payload_length;
			rf_recived_bytes_per_sec += rx_payload_length;
		}
	}

	/*if heartbeat not used, sync device info here*/ 
	if (!prx->heartbeat) {		
		struct device_info *devinfo = prx->devlist + pipe;
		if (prx->timer.ticks > 2 * prx->timer.ticks_count / 3) {
			if (!devinfo->sync) {
				//NRF_LOG_INFO("count device %d info",pipe);
				devinfo->charge 		= *(rx_payload_addr + 0);
				devinfo->battery_h 	= *(rx_payload_addr + 1);
				devinfo->battery_l 	= *(rx_payload_addr + 2);
				devinfo->heartbeat		= 0xff;
				//devinfo->connect = 1;
				devinfo->sync = true;
			}
		}

		/*send ack to device pipe*/
		if (prx->cmdarg == NRFCMD_DISCONNECT) {
			if (prx->cmdval & (1 << pipe)) {
				m_ack_payload[2] = pipe;
				m_ack_payload[3] = NRFCMD_DISCONNECT;
				result_value = 
				nrf_gzll_add_packet_to_tx_fifo(pipe,m_ack_payload,sizeof(m_ack_payload));
				if (! result_value) {
					NRF_LOG_ERROR("send ack to device %d error!",pipe);
				}
				else {
					/*clean this bit in timer 
					when device pipe really disconnected*/
					//prx->cmdval &= (0 << pipe);
					//NRF_LOG_ERROR("===>disconnect device %d !",pipe);
				}
			}
		
			if (prx->cmdval == 0x00) {
				prx->cmdarg = NRFCMD_UNKNOWN;
			}
		}
	}

}

void nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
}

void nrf_gzll_device_tx_failed(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
}

#if 0
uint32_t gs_gzll_set_base_address0(struct gs_nrf_device *dev)
{
	uint8_t base_address0[] = GS_DEFAULT_BASE_ADDRESS_0;
	struct gs_prx *prx = get_gs_prx();
	struct gs_twis_reg *reg = prx->twis.regbase;
	if (reg) {
		/*base addr0 must be not 0x00*/
		if (reg->host_base_addr0[0]) {
			memcpy(base_address0,reg->host_base_addr0,sizeof(base_address0));
		}
	}

	return nrf_gzll_set_base_address_0(base_address0);
}

uint32_t gs_gzll_set_base_address1(struct gs_nrf_device *dea)
{
	struct gs_prx *prx = get_gs_prx();
	struct gs_twis_reg *reg = prx->twis.regbase;
	uint8_t base_address1[] = GS_DEFAULT_BASE_ADDRESS_1;
	
	if ( reg ) {
		/*base addr0 must be not 0x00*/
		if (reg->host_base_addr0[0]) {
			memcpy(base_address1,reg->host_base_addr1,sizeof(base_address1));
		}
	}

	return nrf_gzll_set_base_address_1(base_address1);
}
#endif 

uint32_t gs_gzll_set_channel_table()
{
	uint8_t channel_table[] = GS_DEFAULT_CHANNEL_TABLE;
	uint8_t chnnum = GS_DEFAULT_CHNNUM;
	struct gs_prx *prx = get_gs_prx();
	struct gs_twis_reg *reg = prx->twis.regbase;
	
	if ( reg ) {
		/*base addr0 must be not 0x00*/
		if (reg->host_chn_table[0] && reg->host_chn_table[0] != 0xff) {
			memcpy(channel_table,reg->host_chn_table,sizeof(channel_table));
		}

		if (reg->host_chn_set && reg->host_chn_set != 0xff) {
			int tmp = 0;
			chnnum = reg->host_chn_set;
			for (tmp = 0 ; tmp < chnnum; tmp++) {
				NRF_LOG_ERROR("Nrf set host channel %d = %d",\
					tmp,channel_table[tmp]);
			}
		}
	}
	
	return nrf_gzll_set_channel_table(channel_table,chnnum);
}

void nrf_gzll_disabled()
{
}

void gs_gzll_disable(void)
{
	if(nrf_gzll_is_enabled())
	{
		nrf_gzll_disable();
	}
}

uint32_t gs_gzll_enabled(void)
{
	return nrf_gzll_is_enabled();
}

uint32_t gs_gzll_enable(void)
{
	uint32_t err_code = 0;
	if(!nrf_gzll_is_enabled())
	{
		err_code = nrf_gzll_enable();
	}
	return err_code;
}

uint32_t gs_gzll_init()
{
	// Initialize Gazell.
	bool result_value = nrf_gzll_init(NRF_GZLL_MODE_HOST);
	GAZELLE_ERROR_CODE_CHECK(result_value);

	/*timeslot set*/
	//nrf_gzll_set_timeslot_period(300);
	//nrf_gzll_set_timeslots_per_channel(2);
	
	//gs_gzll_set_host_prefix(dev);
	//gs_gzll_set_base_address0(dev);
	//gs_gzll_set_base_address1(dev);
	
	/*channeltab set*/
	gs_gzll_set_channel_table();

	// Enable Gazell to start sending over the air.
	result_value = nrf_gzll_enable();
	GAZELLE_ERROR_CODE_CHECK(result_value);

	NRF_LOG_INFO("gsrf gzll start!.");
	return 0;
}

