/*************************************************************************
  > File Name: gs_timer.c
  > Author: spchen
  > Mail: spchen@grandstream.cn 
  > Created Time: 2019年03月15日 星期五 10时49分11秒
 ************************************************************************/
#include "gs_prx.h"
#include "nrf_log.h"

volatile uint32_t rf_recived_bytes_per_sec = 0;
volatile uint32_t gs_rf_drop_packets_total = 0;
volatile uint32_t pipe_recived_bytes_per_sec[8] = {0};
//const nrf_drv_timer_t GS_RF_TIMER = NRF_DRV_TIMER_INSTANCE(GS_RF_TIMER_INSTANCE);

struct gs_timer *g_timer = NULL;
static struct gs_prx *get_gs_prx(void)
{
	return &g_prx;
}

void gs_rf_timer_handler(nrf_timer_event_t event_type,void *context)
{
	static uint32_t rf_statistics_count = 0;
	static uint64_t rf_recived_total_bytes = 0;
	struct gs_timer *timer = g_timer;
	struct gs_prx * prx = get_gs_prx();
	int devno = 0;				
	switch(event_type)
	{
		case NRF_TIMER_EVENT_COMPARE0:
			break;

		case NRF_TIMER_EVENT_COMPARE1:
			//statistics every 1000ms 
			if (timer->ticks == timer->ticks_count) {
				rf_statistics_count++;
				rf_recived_total_bytes += rf_recived_bytes_per_sec;
				#if 0
				NRF_LOG_INFO("RX SPEED Avg=%dB/s RFDrop %d pipe0 %d pipe1 %d pipe2 %d pipe3 %d", \
						rf_recived_total_bytes  / rf_statistics_count / 10, \
						gs_rf_drop_packets_total, \
						pipe_recived_bytes_per_sec[0], \
						pipe_recived_bytes_per_sec[1], \
						pipe_recived_bytes_per_sec[2], \
						pipe_recived_bytes_per_sec[3]);
				#endif 
				pipe_recived_bytes_per_sec[0] = 0;
				pipe_recived_bytes_per_sec[1] = 0;
				pipe_recived_bytes_per_sec[2] = 0;
				pipe_recived_bytes_per_sec[3] = 0;
				rf_recived_bytes_per_sec = 0;

				timer->ticks = 0;
			}
			else {
				timer->ticks++;
				/*if (timer->ticks == timer->task_delay) {
					if (timer->callback) {
						timer->callback(timer->contex);
						timer->callback = NULL;
						timer->contex = NULL;
					}
				}*/
			}

			/*sync device info to reglist*/

			for (devno = 0; devno < prx->devcnt; devno++) {
				struct device_info *devinfo = prx->devlist + devno;
				if (devinfo->heartbeat )
				devinfo->heartbeat--;

				if (timer->ticks < 1 * timer->ticks_count / 3) {	
					devinfo->sync = true;
				}
				else if ((timer->ticks > 1 * timer->ticks_count / 3) && 
					(timer->ticks < 2 * timer->ticks_count / 3)) {
					if (devinfo->sync) {
						if (devinfo->heartbeat) {
							prx->twis.reglist->connect |= (1 << devno);
							if (!devinfo->connect) {
								devinfo->connect = 1;
								NRF_LOG_INFO("<<===device %d connect\n",devno);
							} 
						}
						else {
							//gs_rf_clean_txfifo(0);
							//gs_rf_clean_txfifo(1);
							prx->cmdval &= (0 << devno);
							prx->twis.reglist->connect &= ~(1 << devno);

							if (devinfo->connect) {
								devinfo->connect = 0;
								NRF_LOG_INFO("===>>device %d disconnect\n",devno);
							}
						}
						prx->twis.reglist->batstate[devno] = devinfo->charge;
						prx->twis.reglist->batinfo[devno] = 
							(devinfo->battery_h << 8) | devinfo->battery_l;
						devinfo->sync = false;
					}
				}
			}
			break;

		default:
			//Do nothing
			break;
	}
}

int gs_set_delay_task(task_callback callback,void *contex,uint32_t delay)
{
	struct gs_timer *timer = g_timer;
	uint32_t task_delay = 0;
	task_delay = (timer->ticks + delay) % timer->ticks_count;
	if (!timer->callback) {
		timer->task_delay = task_delay;
		timer->callback = callback;
		timer->contex = contex;
		return 0;
	}

	return -1;
}

int gs_timer_enable()
{
	struct gs_timer *timer = g_timer;

	if (!timer->enable) {
		timer->enable = true;
		nrf_drv_timer_enable(&timer->dev);
		return 0;
	}

	return -1;
}

int gs_timer_disable()
{
	struct gs_timer *timer = g_timer;

	if (timer->enable) {
		timer->enable = false;
		nrf_drv_timer_disable(&timer->dev);
	}

	return -1;
}

void gs_rf_timer_init(struct gs_timer *timer)
{
	uint32_t time_ticks;

	nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
	nrf_drv_timer_init(&timer->dev,&timer_cfg,gs_rf_timer_handler);

	//rf rx-statistics
	time_ticks = nrf_drv_timer_ms_to_ticks(&timer->dev,timer->period);
	nrf_drv_timer_extended_compare(&timer->dev,NRF_TIMER_CC_CHANNEL1,
			time_ticks,NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK,true);

	gs_timer_enable();
}

void gs_app_timer_init(struct gs_timer *timer)
{
	g_timer = timer;
	gs_rf_timer_init(timer);
}


void gs_app_timer_uninit(struct gs_timer *timer)
{
	g_timer = NULL;
	nrf_drv_timer_uninit(&timer->dev);
}
