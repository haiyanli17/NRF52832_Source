/*************************************************************************
	> File Name: gs_twis.c
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月20日 星期三 20时30分22秒
 ************************************************************************/

#include "gs_prx.h"
#include "nrf_log.h"

struct gs_twis *g_twis;
struct gs_twis_reg g_sensor_reg;
static void gs_twis_device_reset(struct gs_twis *dev)
{
	struct gs_twis_reg *reg = dev->regbase;
	/*TODO read device info from flash*/

	/*set register to default*/
	reg->system_ctrl = 0x00;

	/*set device status to default*/
	dev->status = NRF_SYSTEM_STATUS_OFF;
}

#if 1
static void gs_twis_start_system(void)
{
	struct gs_twis_reg *reg 	= g_twis->regbase;
	struct gs_prx *prx			= &g_prx;
	uint8_t ringbuf_count		= reg->ringbuf_count? :GS_DEFAULT_TXRINGBUF_COUNT;
	uint8_t sample_unit			= reg->sample_unit? :GS_DEFAULT_SAMPLE_UNIT;
	uint8_t sample_count		= reg->sample_count? :GS_DEFAULT_SAMPLE_COUNT;
	uint8_t device_count		= reg->device_count ? : GS_DEFAULT_DEVICE_NUMBER;
	uint8_t ringbuf_size		= 0;
	NRF_LOG_INFO("in func %s %d %d %d %d %d!",__func__,\
		ringbuf_count,sample_unit,sample_count,device_count,ringbuf_size);

	if(ringbuf_count > 50)
		reg->ringbuf_count = ringbuf_count = 50;

	if (sample_unit > 32)
		reg->sample_unit = sample_unit = 32;
	
	if (sample_count > 7)
		reg->sample_count = sample_count = 7;

	ringbuf_size = sample_unit * sample_count + 5;
	NRF_LOG_INFO("init ringbuf size %d (sample %d x %d)",\
		ringbuf_size,sample_unit,sample_count);

	/*init ringbuf for sample read write.*/
	gs_ringbuf_list_init(ringbuf_count,ringbuf_size,device_count);

	/*start spis to transfer*/
	gs_spis_start_audio();

	//reset device version & flash init	
	g_twis->reglist->flash_init = 0x01;
	memcpy(g_twis->reglist->version,prx->version,6);
}

static void device_disconnect(uint8_t devid)
{
	struct gs_prx *prx = &g_prx;
	prx->cmdarg 		= NRFCMD_DISCONNECT;
	prx->cmdval = devid & prx->twis.reglist->connect; 
	NRF_LOG_INFO("disconnect device bitmap %x=>%x",devid,prx->cmdval);
}

static void gs_system_erase()
{
	struct gs_prx *prx = &g_prx;
	fstorage_erase(prx->flash.flash_start,1);
	
	NRF_LOG_INFO("force erase,factory reset done!");
}
static void gs_system_update(void)
{
	struct gs_prx *prx = &g_prx;
	struct gs_twis_reg *reg 	= g_twis->regbase;
	//void *saved_reg = reg + NRF_SAVED_REG_OFFSET;

	//erase page
	fstorage_erase(prx->flash.flash_start,1);
	wait_for_flash_ready();
	fstorage_write(prx->flash.flash_start,reg, 256);
	NRF_LOG_INFO("update reglist done");
}

static void gs_twis_stop_system(uint8_t sleep)
{
	/*clean all read only reginfo*/
	memset(g_twis->reglist,0x00,0x60);

	/*send cmd to rf stop*/
	gs_spis_stop_audio(sleep);

	/*clean ringbuf*/
	gs_ringbuf_list_uninit();
	
	NRF_LOG_INFO("gs_twis_stop_system");
}
#endif
static void gs_twis_write_start(void)
{
	//NRF_LOG_INFO("in func %s !",__func__);

	(void)nrf_drv_twis_rx_prepare(&g_twis->twis, g_twis->rwbuf,TWIS_MAX_PAYLOAD_LENGTH);
}

static void gs_twis_write_stop(size_t cnt)
{
	/*whether read or write mode ,
	 * The first byte indicate register number*/
	g_twis->reg = g_twis->rwbuf[0];

	/*In i2c read mode :payload is  addr + register
	 * and next i2c status enter to i2c read*/
	if (cnt == 1) {
		g_twis->rw = 1;
		NRF_LOG_INFO("prepare to read register %x\n",g_twis->reg);
	}
	
	/*i2c write : payload is i2c addr + register + value + ...*/
	else if (cnt >= 2) {
		int tmp = 0;
		struct gs_twis_reg *sensor_reg = g_twis->regbase;
		if (cnt + g_twis->reg > sizeof(struct gs_twis_reg)){
			NRF_LOG_INFO("cnt + reg big than reglist count!\n")
			return;
		}
			
		for (tmp = 1; tmp < cnt; tmp++) {
			char val = g_twis->rwbuf[tmp];
			char *regbase = (char *)g_twis->regbase;	
			char reg = g_twis->reg + tmp -1;
			*(regbase + reg) = val;
			NRF_LOG_INFO("write %x to  register %x",val,reg);
		}

		/*parse to start system*/
		if (sensor_reg->system_ctrl & NRF_SYSTEM_UPDATE) {
			/*write reginfo to flash*/
			gs_system_update();
			/*if system startup , restart it*/
			if (!(sensor_reg->system_ctrl & NRF_SYSTEM_START)) {
				gs_twis_stop_system(0);
				g_twis->status = NRF_SYSTEM_STATUS_OFF;
				gs_twis_start_system();
				g_twis->status = NRF_SYSTEM_STATUS_ON;
			}
			sensor_reg->system_ctrl &= ~NRF_SYSTEM_UPDATE;
		}

		else if (sensor_reg->system_ctrl & NRF_SYSTEM_ERASE) { 
			gs_system_erase();
			sensor_reg->system_ctrl &= ~NRF_SYSTEM_ERASE;
		}
		
		else if (sensor_reg->system_ctrl & NRF_SYSTEM_START) {
			if (g_twis->status == NRF_SYSTEM_STATUS_OFF) {	
				gs_twis_start_system();
				g_twis->status = NRF_SYSTEM_STATUS_ON;
			}
		}

		else if (!(sensor_reg->system_ctrl & NRF_SYSTEM_START)) { 
			if (g_twis->status == NRF_SYSTEM_STATUS_ON) {
				gs_twis_stop_system(10);
				g_twis->status = NRF_SYSTEM_STATUS_OFF;
			}
		}
		
		if (sensor_reg->disconnect) {
			device_disconnect(sensor_reg->disconnect);
			sensor_reg->disconnect = 0x00;
		}
	}
}

static void gs_twis_read_start(void)
{
	char * read_reg = NULL;
	static char read_dummy[6];
	char remind,length = 128;
	NRF_LOG_INFO("in func %s !",__func__);

	remind = sizeof(struct gs_twis_reg) - g_twis->reg;
	if (remind >= 128)
		length = 128;
	else 
		length = remind;
	
	/*i2c read request must followed by an write*/
	if (g_twis->rw) {
		read_reg = (char *)g_twis->regbase + g_twis->reg;
		(void) nrf_drv_twis_tx_prepare(&g_twis->twis, read_reg, length); //max 127Byte
	}

	else {
		(void) nrf_drv_twis_tx_prepare(&g_twis->twis, read_dummy, 1);
	}
}

static void gs_twis_read_stop(size_t cnt)
{
	g_twis->rw	= 0;
	g_twis->reg = -1;
	
	NRF_LOG_INFO("in func %s !",__func__);
}

static void gs_twis_event_handler(nrf_drv_twis_evt_t const * const p_event)
{
	switch (p_event->type)
	{	
		case TWIS_EVT_READ_REQ:
		if (p_event->data.buf_req) {
			gs_twis_read_start();
		}

		break;

		case TWIS_EVT_READ_DONE:
			gs_twis_read_stop(p_event->data.tx_amount);
			break;

		case TWIS_EVT_WRITE_REQ:
			if (p_event->data.buf_req) {
				gs_twis_write_start();
			}

			break;

		case TWIS_EVT_WRITE_DONE:
			gs_twis_write_stop(p_event->data.rx_amount);
			break;

		case TWIS_EVT_READ_ERROR:
		case TWIS_EVT_WRITE_ERROR:
		case TWIS_EVT_GENERAL_ERROR:
			NRF_LOG_INFO("gs twis event error!\n");
			g_twis->err = true;
			break;

		default:
			break;
	}
}

uint32_t gs_twis_init(struct gs_twis *tdev)
{
	uint32_t ret = -1;
	const nrf_drv_twis_config_t gs_twis_config =
	{	
		.addr				= {tdev->config.addr,0},
		.scl				= tdev->config.scl,
		.sda				= tdev->config.sda,
		.interrupt_priority	= tdev->config.priority,
		.scl_pull			= NRF_GPIO_PIN_PULLUP,
		.sda_pull			= NRF_GPIO_PIN_PULLUP,
	};

	ret = nrf_drv_twis_init(&tdev->twis,&gs_twis_config,gs_twis_event_handler);
	if (ret != NRF_SUCCESS) {
		NRF_LOG_ERROR("init gs twis error!");
		return ret;
	}
	
	g_twis = tdev;
	tdev->regbase = tdev->reglist;
	gs_twis_device_reset(tdev);
	nrf_drv_twis_enable(&tdev->twis);
	NRF_LOG_INFO("gs twis init done! regbase = %x",tdev->regbase);
	return ret;
}
