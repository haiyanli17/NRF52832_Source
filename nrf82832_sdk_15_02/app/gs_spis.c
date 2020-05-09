/*************************************************************************
  > File Name: gs_spis.c
  > Author: spchen
  > Mail: spchen@grandstream.cn 
  > Created Time: 2019年03月18日 星期一 12时10分42秒
 ************************************************************************/
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#include "gs_prx.h"
#include "gs_timer.h"

struct gs_spis *g_sdev = NULL;
static void spi_slave_event_handle(nrf_drv_spis_event_t event);
static void gs_spis_send_sample(void *contex)
{
	uint32_t err_code;
	static uint8_t next_tx_devid= 0;
	struct spis_packet *packet	= NULL;
	struct gs_ringbuf *ringbuf	= NULL;
	struct gs_spis	*sdev	= (struct gs_spis*)contex;
	uint8_t availabe_devcnt = gs_ringbuf_get_availabe_count();

	while( next_tx_devid < availabe_devcnt )
	{
		//step 1 :check last packet status
		if (!sdev->xfer_done && sdev->spis_lock) {
			//NRF_LOG_INFO("last spis packet remined!");
			return;
		}
	
		//step 2: get tx addr from ringbuf & set spis buff
		ringbuf	= gs_ringbuf_get_by_devid(next_tx_devid);
		packet	= gs_ringbuf_get_txpacket(ringbuf);
		if (packet) {
			sdev->tx_ptr = packet->addr;
			sdev->rx_ptr = sdev->rx_buf;
			sdev->length = packet->availed + 1;
			/*setp 3:set packet head*/
			spis_packet_set_head(packet,0xab,next_tx_devid);

			/*step 4 : set ready line to host*/
			err_code = nrf_drv_spis_buffers_set(&sdev->spis,sdev->tx_ptr,
					sdev->length,sdev->rx_ptr,sdev->length);
			if (err_code == NRF_SUCCESS) {
				sdev->xfer_done = false;
				sdev->spis_lock = true;
				sdev->txpacket = packet;
			}
			
			//NRF_LOG_INFO("spi set txaddr %x rxaddr %x length %d",
			//	sdev->tx_ptr, sdev->rx_ptr,sdev->length);

			break;
		}

		
		else {
			//NRF_LOG_INFO("spis get id %d send packet error!\n",next_tx_devid);
		}
	
		next_tx_devid++;
	}
		

	/*send device sample sequence like pipe0,pipe1,pipe2 ...*/
	if(++next_tx_devid >= availabe_devcnt)
	{
		next_tx_devid = 0;
	}
}

static void gs_set_spis_idel(struct gs_spis *sdev)
{
	nrf_drv_spis_event_t event = {
		.tx_amount	= 0,
		.rx_amount	= 0,
		.evt_type	= NRF_DRV_SPIS_EVT_TYPE_MAX,
	};

	sdev->state = GS_SPIS_STATE_SETUP;
	spi_slave_event_handle(event);
}


/*TODO: remove later*/
static void spis_parse_cmdline(struct gs_spis_cmdline *cmdline)
{
	static bool gs_rf_start = false;
	switch(cmdline->cmd)
	{
		case DEVICE_CMD_START_RF:
			if (g_sdev->state != DEVICE_CMD_START_RF) {
				gs_rf_start		= true;
				g_sdev->cmd		= DEVICE_CMD_START_RF;
				g_sdev->state	= GS_SPIS_STATE_AUDIO;
				
				gs_rf_enable();
			}
			else {
				gs_set_spis_idel(g_sdev);
			}

			break;

		case DEVICE_CMD_STOP_RF:
			if (g_sdev->cmd != DEVICE_CMD_STOP_RF) {
				gs_rf_start = false;
				g_sdev->cmd = DEVICE_CMD_STOP_RF;
				
				//gs_rf_disable();
				//delay 1s to makesure ack send out
				NRF_LOG_INFO("stop rf after %d ticks",cmdline->delay);
				//if (!cmdline->delay)
					gs_rf_disable();
				//else 
				//	gs_set_delay_task(gs_rf_disable,NULL,cmdline->delay);
				
				gs_set_spis_idel(g_sdev);
			}

			else {
				gs_set_spis_idel(g_sdev);
			}

			break;

		case DEVICE_CMD_RF_DATA:
			 //Do Nothing
			if (gs_rf_start) {
				g_sdev->state = GS_SPIS_STATE_AUDIO;
			}

			else {
				gs_set_spis_idel(g_sdev);
			}
		
			break;

		case DEVICE_CMD_START_I2S:
			gs_set_spis_idel(g_sdev);
			break;

		case DEVICE_CMD_STOP_I2S:
			gs_set_spis_idel(g_sdev);
			break;

		case DEVICE_CMD_I2S_DATA:
			gs_set_spis_idel(g_sdev);
			break;

		default:
			gs_set_spis_idel(g_sdev);
	}
}

static void spi_slave_event_handle(nrf_drv_spis_event_t event)
{
	uint32_t err_code = NRF_SUCCESS;

	switch(g_sdev->state)
	{
		case GS_SPIS_STATE_SETUP :
			g_sdev->tx_ptr = g_sdev->tx_buf;
			g_sdev->rx_ptr = g_sdev->rx_buf;
			err_code = nrf_drv_spis_buffers_set(&g_sdev->spis,g_sdev->tx_ptr,
					g_sdev->length,g_sdev->rx_ptr,g_sdev->length);
			if (err_code == NRF_SUCCESS) {
				g_sdev->state = GS_SPIS_STATE_RXCMD;
			}

			break;
		
		/*maybe remove later*/
		case GS_SPIS_STATE_RXCMD :
			if (event.evt_type == NRF_DRV_SPIS_BUFFERS_SET_DONE) {
				/*release spis semaphore*/
				g_sdev->spis.p_reg->TASKS_RELEASE = 1;
			}

			if (event.evt_type == NRF_DRV_SPIS_XFER_DONE) {
				//struct gs_spis_cmdline *cmdline = 
				//	(struct gs_spis_cmdline*)g_sdev->rx_ptr;

				NRF_LOG_INFO("CMD Recive %d Byte data!",event.rx_amount);
				gs_set_spis_idel(g_sdev);
				//spis_parse_cmdline(cmdline);
			}

			break;
		
		case GS_SPIS_STATE_AUDIO :
			if (event.evt_type == NRF_DRV_SPIS_BUFFERS_SET_DONE) {
				/*release spis semaphore*/
				g_sdev->spis.p_reg->TASKS_RELEASE = 1;
				//NRF_LOG_ERROR("set ready line!\n");
				gs_set_ready_line();
			}

			if (event.evt_type == NRF_DRV_SPIS_XFER_DONE) {
				g_sdev->xfer_done = true;
				gs_clear_ready_line();
				//NRF_LOG_ERROR("clear ready line!\n");
			}
			
			break;

		default:
			NRF_LOG_ERROR("UNKNOWN SPIS STATE");
			err_code = NRF_ERROR_INVALID_STATE;
	}

	/*spis packet send done*/
	if (g_sdev->spis_lock) {
		struct spis_packet *packet = g_sdev->txpacket;
		spis_packet_set_status(packet,BUF_FREE);
		g_sdev->spis_lock = false;
	}

	APP_ERROR_CHECK(err_code);
}

void gs_spis_stop_audio(uint8_t sleep)
{
	struct gs_spis_cmdline spis_cmd = {
		.cmd = DEVICE_CMD_STOP_RF,
		.delay = sleep,
	};

	g_sdev->xfer_done = true;
	g_sdev->spis_lock = false;
	gs_clear_ready_line();
	spis_parse_cmdline(&spis_cmd);
}

void gs_spis_start_audio(void)
{
	struct gs_spis_cmdline spis_cmd = {
		.cmd = DEVICE_CMD_START_RF,
	};

	spis_parse_cmdline(&spis_cmd);
}

uint32_t gs_spis_init(struct gs_spis *sdev)
{
	uint32_t err_code;
	nrf_drv_spis_config_t gs_spis_config;

	gs_spis_config.miso_pin		= sdev->config.miso;
	gs_spis_config.mosi_pin		= sdev->config.mosi;
	gs_spis_config.sck_pin			= sdev->config.sck;
	gs_spis_config.csn_pin			= sdev->config.csn;
	gs_spis_config.mode			= sdev->config.mode;
	gs_spis_config.def          	= sdev->config.def;
	gs_spis_config.orc          	= sdev->config.orc;
	gs_spis_config.irq_priority 	= sdev->config.priority;
	gs_spis_config.csn_pullup   	= NRF_GPIO_PIN_PULLUP;
	gs_spis_config.miso_drive   	= NRF_DRV_SPIS_DEFAULT_MISO_DRIVE;
	gs_spis_config.bit_order    	= NRF_DRV_SPIS_BIT_ORDER_MSB_FIRST;

	//keep /CS high when init
	nrf_gpio_cfg_input(gs_spis_config.csn_pin, NRF_GPIO_PIN_PULLUP);
	err_code = nrf_drv_spis_init(&sdev->spis, &gs_spis_config, spi_slave_event_handle);
	APP_ERROR_CHECK(err_code);

	if (err_code != NRF_SUCCESS){
		return err_code;
	}

	g_sdev = sdev;
	gs_set_spis_idel(sdev);
	sdev->xfer_callback = gs_spis_send_sample;
	NRF_LOG_INFO("GrandStream SPIS INIT!");
	return 0;

	// init spi ready line ,and gpiote if need.
	
}
