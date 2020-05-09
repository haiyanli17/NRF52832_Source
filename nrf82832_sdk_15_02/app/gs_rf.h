/*************************************************************************
	> File Name: gs_rf.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月12日 星期二 11时32分32秒
 ************************************************************************/

#ifndef __GS_RF_H__
#define __GS_RF_H__

#define GS_DEFAULT_BASE_ADDRESS_0  	{'G','S','R','F'}
#define GS_DEFAULT_BASE_ADDRESS_1  	{'A','B','C','D'}
#define GS_DEFAULT_ADDR_PREFIX		{0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8}
#define GS_DEFAULT_CHANNEL_TABLE		{79,2,77,4}
#define GS_DEFAULT_CHNNUM 			4

struct gs_nrf_device {
	uint8_t baseaddr0[8];
	uint8_t baseaddr1[8];
	uint8_t prefix[8];
	uint8_t channel_table[10];
	uint8_t channel_set;
};

//functions define
void gs_rf_disable();
uint32_t gs_rf_init();
uint32_t gs_rf_enable();
uint32_t gs_rf_enabled();
void gs_rf_clean_txfifo(uint32_t pipe);
void gs_rf_clean_rxfifo(uint32_t pipe);
uint32_t gs_set_base_address0(struct gs_nrf_device *prx);
uint32_t gs_set_base_address1(struct gs_nrf_device *prx);
uint32_t gs_rf_set_channel_table(struct gs_nrf_device *prx);
uint32_t gs_rf_set_prefix_table(struct gs_nrf_device *prx);
bool gs_rf_set_rx_pipe_enabled(uint32_t pipes);
#endif 
