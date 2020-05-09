/*************************************************************************
	> File Name: gs_timer.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月15日 星期五 14时13分16秒
 ************************************************************************/

#ifndef __GS_TIMER_H__
#define __GS_TIMER_H__

#include "nrf.h"
#include "sdk_common.h"
#include "nrf_timer.h"
#include "nrf_drv_timer.h"

#define SPIS_TX_PERIOD 1
#define RF_STATISTICS_PERIOD 1000

#define GS_RF_TIMER_INSTANCE	1
#define GS_SPIS_TIMER_INSTANCE	2

extern volatile uint32_t gs_rf_drop_packets_total;
extern volatile uint32_t rf_recived_bytes_per_sec;
extern volatile uint32_t pipe_recived_bytes_per_sec[8];
typedef void (*task_callback)(void *contex);

struct gs_timer {
	bool enable;
	uint32_t period;  //ms
	uint32_t ticks;
	uint32_t ticks_count;
	uint32_t task_delay; //ticks
	nrf_drv_timer_t dev;
	void *contex;
	void (*callback)(void *contex);
};

int gs_timer_disable(void);
int gs_timer_enable(void);
int gs_set_delay_task(task_callback callback,void *contex,uint32_t delay);
void gs_app_timer_init(struct gs_timer *timer);
void gs_app_timer_uninit(struct gs_timer *timer);
#endif 

