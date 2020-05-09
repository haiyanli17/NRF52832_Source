/*************************************************************************
	> File Name: gs_ringbuf.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月12日 星期二 15时17分48秒
 ************************************************************************/
#ifndef __gs_ringbuf_H__
#define __gs_ringbuf_H__

#define RF_GZLL_PAYLOAD_SIZE		32
#define PAYLOAD_COUNT_PERPACKET		7
#define GS_DEFAULT_RXRINGBUF_COUNT	3
#define GS_DEFAULT_TXRINGBUF_COUNT	10
#define GS_DEFAULT_DEVICE_NUMBER		8

#define GS_SPI_HEAD_START			0xab
#define GS_SPI_HEAD_LENGTH			3		//start + type + devid
//1Byte Data type + 1Byte Devid  + N Packet Payload Data 
#define GS_DEFAULT_SAMPLE_UNIT		32 
#define GS_DEFAULT_SAMPLE_COUNT		7 
//(GS_SPI_HEAD_LENGTH + RF_GZLL_PAYLOAD_SIZE * PAYLOAD_COUNT_PERPACKET)

enum ringbuf_status {
	BUF_FREE = 0,
	BUF_IN_RF = 1,
	BUF_OUT_RF = 2,
	BUF_IN_SPI = 4,
};

struct spis_packet{
	uint8_t *addr;
	uint8_t *start;
	uint8_t *type;
	uint8_t *devid;
	uint8_t index;
	uint8_t status;
	uint8_t availed;
	uint8_t remind;
};

struct gs_ringbuf{
	uint8_t *buff;
	uint8_t tx_index;
	uint8_t rx_index;
	uint8_t spis_packet_count;
	uint8_t spis_packet_size;
	struct spis_packet *current;
	struct spis_packet *packetlist;
};

struct gs_rambuf {
	uint8_t count;
};

extern struct gs_ringbuf *gs_ringbuf_list;
//functions define

struct spis_packet *gs_ringbuf_get_rxpacket(struct gs_ringbuf *ringbuf);
struct spis_packet *gs_ringbuf_get_txpacket(struct gs_ringbuf *ringbuf);
uint8_t *rf_request_payload_addr(struct gs_ringbuf *ringbuf,uint8_t size);
void spis_packet_set_status(struct spis_packet *packet,enum ringbuf_status status);
void spis_packet_set_head(struct spis_packet *packet,uint8_t type,uint8_t devid);
int8_t gs_ringbuf_get_txindex(struct gs_ringbuf *ringbuf);
int8_t gs_ringbuf_get_rxindex(struct gs_ringbuf *ringbuf);
uint8_t gs_ringbuf_get_packet_size(struct gs_ringbuf *ringbuf);
uint8_t gs_ringbuf_get_packet_availed(struct gs_ringbuf *ringbuf);
uint8_t gs_ringbuf_get_packet_count(struct gs_ringbuf *ringbuf);
uint8_t gs_ringbuf_get_availabe_count(void);
uint8_t gs_ringbuf_list_init(uint8_t count,uint8_t size,uint8_t device);
struct gs_ringbuf *gs_ringbuf_get_by_devid(uint8_t devid);
void gs_ringbuf_list_uninit(void);
#endif 

