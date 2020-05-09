/*************************************************************************
	> File Name: gs_prx.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年07月15日 星期一 15时59分23秒
 ************************************************************************/
#ifndef __GS_PRX_H__
#define __GS_PRX_H__

#include "gs_twis.h"
#include "gs_spis.h"
#include "gs_gpio.h"
#include "gs_rf.h"
#include "gs_timer.h"
#include "gs_ringbuf.h"
#include "gs_flash.h"
enum prx_cmd {
	NRFCMD_UNKNOWN = 0,
	NRFCMD_DISCONNECT = 1,
};

struct device_info {
	bool sync;
	uint8_t pipe;
	uint8_t charge;
	uint8_t battery_h;
	uint8_t battery_l;
	uint8_t connect;
	uint8_t heartbeat;
};

//typedef void (*spis_xfer) (void *contex);
typedef struct gs_prx {
	uint8_t cmdarg;
	uint8_t cmdval;
	volatile uint8_t heartbeat;
	struct gs_spis	spis;
	struct gs_twis	twis;
	struct gs_gpios gpio;
	struct gs_nrf_device	nrf;
	struct gs_flash	flash;
	struct gs_timer timer;
	uint8_t devcnt;

	char  *version;
	struct device_info  *devlist;
	void (*spis_xfer) (void *contex);
}gs_prx_t;

extern struct gs_prx g_prx;
#endif 
