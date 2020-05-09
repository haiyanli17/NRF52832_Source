/*************************************************************************
  > File Name: gs_main.c
  > Author: spchen
  > Mail: spchen@grandstream.cn 
  > Created Time: 2019年03月12日 星期二 10时49分20秒
 ************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "sdk_common.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "mem_manager.h"
#include "bsp.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "gs_prx.h"

struct device_info g_devinfo[8] = {
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0},
		{false,0,0,0,0,0,0}
};
struct gs_twis_reg  g_twis_reglist;
struct gs_prx g_prx = {
	.spis = {
		.xfer_done	= false,
		.spis_lock	= false,
		.cmd		= DEVICE_CMD_UNKNOWN,
		.state		= GS_SPIS_STATE_IDEL,
		.spis		= NRF_DRV_SPIS_INSTANCE(GS_SPIS_INSTANCE),
		.config		= {
			.miso		= GS_SPIS_MISO_PIN,
			.mosi		= GS_SPIS_MOSI_PIN,
			.sck		= GS_SPIS_SCK_PIN,
			.csn		= GS_SPIS_CSN_PIN,
			.mode		= NRF_DRV_SPIS_MODE_0,
			.priority	= GS_SPIS_IRQ_PRIORITY_LEVEL,
			.def		= GS_SPIS_DEFAULT_DEF_CHARACTER,
			.orc		= GS_SPIS_DEFAULT_ORC_CHARACTER,
		}
	},

	.twis = {
		.config		= {
			.addr		= GS_DEFAULT_TWIS_ADDR,
			.scl		= 11,//GS_DEFAULT_TWIS_SCL,
			.sda		= 12,//GS_DEFAULT_TWIS_SDA,
			.priority	= GS_DEFAULT_TWIS_PRIORITY,
		},

		.err			= 0,
		.reglist 		= &g_twis_reglist,
		.twis			= NRF_DRV_TWIS_INSTANCE(GS_TWIS_INSTANCE),
	},

	.gpio = {
		.ready_line 	= GS_SPIS_READY_PIN,
		.init 			= GS_GPIO_HIGH,
	},

	.nrf = {
		.baseaddr0 		= GS_DEFAULT_BASE_ADDRESS_0,
		.baseaddr1 		= GS_DEFAULT_BASE_ADDRESS_1,
		.prefix 		= GS_DEFAULT_ADDR_PREFIX,
		.channel_table  = GS_DEFAULT_CHANNEL_TABLE,
		.channel_set 	= GS_DEFAULT_CHNNUM,
	},

	.flash = {
		.flash_start 	= 0x3e000,
		.flash_stop 	= 0x3ffff,
		.flash_addr 	= (void*)&g_twis_reglist,
	},


	.timer = {
		.period			= 10,   //10ms
		.enable			= false,
		.callback		= NULL,
		.ticks_count	= 100,  //100 * 10 = 1s
		.dev			= NRF_DRV_TIMER_INSTANCE(GS_RF_TIMER_INSTANCE),
	},

	.cmdarg				= NRFCMD_UNKNOWN,
	.heartbeat			= 0,
	.devcnt				= 8,
	.devlist			= g_devinfo,
	.version			= "V0.01",
};

void clocks_start(void)
{
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}


int main(void)
{
	uint32_t err_code;
	struct gs_prx *prx = &g_prx;

	clocks_start();
	err_code = nrf_mem_init();
	APP_ERROR_CHECK(err_code);

	err_code = NRF_LOG_INIT(NULL);
	NRF_LOG_DEFAULT_BACKENDS_INIT();
	APP_ERROR_CHECK(err_code);

	/*timer init*/
	gs_app_timer_init(&prx->timer);
	
	/*flash init*/
	gs_flash_init(&prx->flash);

	/*gpio init*/
	gs_gpio_init(&prx->gpio);

	/*init spis*/
	gs_spis_init(&prx->spis);

	/*init twis*/
	gs_twis_init(&prx->twis);
	memcpy(prx->twis.reglist->version,prx->version,6);
	NRF_LOG_INFO("GSRF PRX %s Started.",prx->version);
	while (true) {
		NRF_LOG_FLUSH();
		if (gs_rf_enabled()) {
			if (prx->spis.xfer_callback)
				prx->spis.xfer_callback(&prx->spis);
		}
	}
	return 0;
}

