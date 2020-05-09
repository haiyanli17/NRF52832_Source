/*************************************************************************
	> File Name: gs_spis.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月12日 星期二 11时32分45秒
 ************************************************************************/
#ifndef __GS_SPIS_H__
#define __GS_SPIS_H__
#include "nrf_drv_spis.h"

enum nrf_type {
	NRF_GZLL = 1,
	NRF_ESB
};

enum GS_DEVICE_CMD_STATE {
	DEVICE_CMD_UNKNOWN 	= -1,
	DEVICE_CMD_START_RF 	= 0,
	DEVICE_CMD_STOP_RF 	= 1,
	DEVICE_CMD_START_I2S 	= 2,
	DEVICE_CMD_STOP_I2S 	= 3,
	DEVICE_CMD_RF_DATA 	= 4,
	DEVICE_CMD_I2S_DATA 	= 5,
};

enum GS_SPIS_STATE {
	GS_SPIS_STATE_IDEL 	= -1,
	GS_SPIS_STATE_SETUP 	= (1 << 0),
	GS_SPIS_STATE_RXCMD 	= (1 << 1),
	GS_SPIS_STATE_AUDIO 	= (1 << 2),
};

#define GS_SPIS_INSTANCE					1
#define GS_SPIS_MAX_LENGTH					256
#define GS_SPIS_SCK_PIN					15
#define GS_SPIS_MOSI_PIN					17
#define GS_SPIS_MISO_PIN					16
#define GS_SPIS_CSN_PIN					19
#define GS_SPIS_DEFAULT_DEF_CHARACTER 	0x55
#define GS_SPIS_DEFAULT_ORC_CHARACTER 	0xAA
#define GS_SPIS_IRQ_PRIORITY_LEVEL		2

struct gs_spis_cmdline {
	uint8_t head[2];
	uint8_t cmd;
	uint8_t delay;
	uint8_t payload_len;
	uint8_t payload_data[128];
};

struct spis_config {
	uint8_t miso;
	uint8_t mosi;
	uint8_t sck;
	uint8_t csn;
	uint8_t mode;
	uint8_t def;
	uint8_t orc;
	uint8_t priority;
};

struct gs_spis {
	uint8_t state;
	uint8_t cmd;

	bool xfer_done;
	bool spis_lock;
	
	uint8_t *rx_ptr;
	uint8_t *tx_ptr;
	uint8_t length;
	uint8_t tx_buf[GS_SPIS_MAX_LENGTH];
	uint8_t rx_buf[GS_SPIS_MAX_LENGTH];
	
	nrf_drv_spis_t spis;
	struct spis_config config;
	struct spis_packet *txpacket;

	void (*xfer_callback)(void *contex);
};

uint32_t gs_spis_init(struct gs_spis *sdev);
void gs_spis_send_packet(void);
void gs_spis_start_audio(void);
void gs_spis_stop_audio(uint8_t sleep);
#endif 
