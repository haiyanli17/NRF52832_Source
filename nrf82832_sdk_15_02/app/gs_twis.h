/*************************************************************************
	> File Name: gs_twis.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月20日 星期三 20时08分52秒
 ************************************************************************/

#ifndef __GS_TWIS_H__
#define __GS_TWIS_H__

#include "nrf_drv_twis.h"

/*defines*/
#define GS_TWIS_INSTANCE				0
#define GS_DEFAULT_TWIS_ADDR			0x40
#define GS_DEFAULT_TWIS_SCL			13
#define GS_DEFAULT_TWIS_SDA			14
#define GS_DEFAULT_TWIS_PRIORITY		6
#define TWIS_MAX_PAYLOAD_LENGTH		128


/*struct define*/
struct gs_twis_reg{
/*------------common info------------- 0x00--0x0f*/
#define NRF_DEVICE_PID			0x00
	char device_pid;
#define NRF_DEVICE_VID			0x01
	char device_vid;
#define NRF_DEVICE_SID				0x02
	char device_sid;
#define NRF_FLASH_INIT				0x03
	char flash_init;
#define NRF_DEVICE_CNNT			0x04
	char connect;  					//connect info
#define NRF_DEVICE_DISC			0x05
	char disconnect;             //set which to disconnect
#define NRF_DEVICE_VER			0x06
	char version[6];
	char resved[4];

/*------------system info------------- 0x10--0x1f*/
#define NRF_SYSTEM_CTRL			0x10
#define NRF_SYSTEM_START			(1 << 0)
#define NRF_SYSTEM_UPDATE			(1 << 1)
#define NRF_SYSTEM_ERASE 			(1 << 7)
	char system_ctrl;
#define NRF_SYSTEM_STATE			0x11
	char system_state;
#define NRF_SYSTEM_TYPE			0x12
	char system_samtype;
	char system_resv[13];

/*------------device  batinfo--------- 0x20--0x3f*/
#define NRF_DEVICE_BATINFO		0x20
	short batinfo[8];						
#define  NRF_DEVICE_BATSTATE		0x30
	char  batstate[8];
	char  bat_rsv[8];

/*------------temped  resv------------ 0x40--0x5f*/
#define NRF_USER_DEFINE1			0x40
	char temped_rsv[32];

#define NRF_SAVED_REG_OFFSET		0x60
/*------------host rf info------------ 0x60--0xff*/ 
#define NRF_HOST_RBCNT				0x60	
	char ringbuf_count;
#define NRF_HOST_SPUNIT			0x61
	char sample_unit;
#define NRF_HOST_SPCNT				0x62
	char sample_count;
#define NRF_HOST_DEVCNT			0x63
	char device_count;
#define NRF_HOST_ADDR0				0x64
	char host_base_addr0[4];
#define NRF_HOST_ADDR1				0x68
	char host_base_addr1[4];
#define NRF_HOST_PREFIX			0x6c
	char host_addr_prefix[8];
#define NRF_HOST_RESV				0x74
	char host_resv[12];

/*------------chntab------------------ 0x80--0x8f*/
#define NRF_HOST_CHNTAB			0x80
	char host_chn_table[10];			
#define NRF_HOST_CHNSET			0x8a
	char host_chn_set;				
	char host_chn_rsv[5];
/*------------parinfo----------------- 0x90--0xaf*/
#define NRF_HOST_PARINFO			0x90
	char parinfo[32];
/*------------user define------------- 0xb0--0xff*/
#define NRF_USER_DEFINE2			0xb0
	char resv[80];
};

struct twis_config {
	uint8_t addr;
	uint8_t scl;
	uint8_t sda;
	uint8_t priority;
};

enum system_status {
	NRF_SYSTEM_STATUS_OFF,
	NRF_SYSTEM_STATUS_ON,
	NRF_SYSTEM_STATUS_UPDATE,
};

struct gs_twis {
	char err;
	char rw;
	char reg;
	char status;
	nrf_drv_twis_t twis;
	struct gs_twis_reg *reglist;
	struct twis_config config;
	struct gs_twis_reg *regbase;
	char rwbuf[TWIS_MAX_PAYLOAD_LENGTH]; //support max per packet
};

uint32_t gs_twis_init(struct gs_twis *);
#endif
