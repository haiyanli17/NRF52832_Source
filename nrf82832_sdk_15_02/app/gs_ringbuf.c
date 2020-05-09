/*************************************************************************
	> File Name: gs_ringbuf.c
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月12日 星期二 14时32分26秒
 ************************************************************************/
#include<stdint.h>
#include<stdlib.h>
#include "nrf_log.h"
//#include "gs_event.h"
#include "gs_ringbuf.h"

static struct gs_ringbuf gs_ringbuf[GS_DEFAULT_DEVICE_NUMBER];
struct gs_ringbuf *gs_ringbuf_list = gs_ringbuf;
static uint8_t g_gs_ringbuf_availabe_count = 0;

static int8_t spis_packet_init(struct gs_ringbuf *ringbuf)
{
	uint8_t index = 0;
	uint8_t count = ringbuf->spis_packet_count;
	struct spis_packet *packet;
	struct spis_packet *packetlist = malloc(sizeof(struct spis_packet) * count);
	if(!packetlist){
		NRF_LOG_ERROR("get spis packet error!");
		return -1;
	}

	packet = packetlist;
	for(index = 0; index < count; index++,packet++){
		packet->index = index;
		packet->status = BUF_FREE;
		packet->addr = ringbuf->buff + ringbuf->spis_packet_size * index;
		packet->start = (uint8_t *)packet->addr;
		packet->type = (uint8_t *)packet->addr + 1;
		packet->devid = (uint8_t *)packet->addr + 2;
		//NRF_LOG_ERROR("packet %d/%d addr %p/%p!",index,count,ringbuf->buff,packet->addr);
	}

	ringbuf->packetlist = packetlist;

	return 0;
}

void spis_packet_set_status(struct spis_packet *packet,enum ringbuf_status status)
{
	if(!packet)
		return;

	packet->status = status;
}

void spis_packet_set_head(struct spis_packet *packet,uint8_t type,uint8_t devid)
{
	if(!packet)
		return;
	*packet->type = type;
	*packet->devid = devid;
	*packet->start = GS_SPI_HEAD_START;
}

static int8_t gs_ringbuf_init(struct gs_ringbuf *ringbuf,uint8_t count,uint8_t size)
{
	uint8_t *pbuf = NULL;
	uint32_t length = 0;
	uint8_t err_code = 0;

	length = count * size;
	pbuf = malloc(sizeof(uint8_t) * length);
	if (!pbuf){
		NRF_LOG_ERROR("get ringbuf error!");
		return -1;
	}

	ringbuf->buff = pbuf;
	ringbuf->tx_index = 0;
	ringbuf->rx_index = 0;
	ringbuf->spis_packet_count = count;
	ringbuf->spis_packet_size = size;

	/*clean buff*/
	memset(pbuf,0x00,length);
	err_code = spis_packet_init(ringbuf);
	if(err_code){
		NRF_LOG_ERROR("init spis packet error!");
		return -2;
	}

	return 0;
}

static void gs_ringbuf_exit(struct gs_ringbuf *ringbuf)
{
	if(!ringbuf)
		return;

	if(ringbuf->packetlist)
		free(ringbuf->packetlist);
	
	if(ringbuf->buff) {
		free(ringbuf->buff);
	}
	ringbuf->buff = NULL;
	ringbuf->packetlist = NULL;
	memset(ringbuf,0x00,sizeof(struct gs_ringbuf));
}


uint8_t gs_ringbuf_get_packet_count(struct gs_ringbuf *ringbuf)
{
	if(!ringbuf)
		return 0;

	return ringbuf->spis_packet_count;
}

uint8_t gs_ringbuf_get_packet_size(struct gs_ringbuf *ringbuf)
{
	if(!ringbuf)
		return 0;

	return ringbuf->spis_packet_size;
}

uint8_t gs_ringbuf_get_packet_availed(struct gs_ringbuf *ringbuf)
{
	struct spis_packet *packet = ringbuf->current;
	if(!packet)
		return 0;


	NRF_LOG_INFO("packet index %d availed %d",packet->index ,packet->availed);
	return packet->availed;

}

int8_t gs_ringbuf_get_txindex(struct gs_ringbuf *ringbuf)
{
	if(!ringbuf)
		return -1;

	return ringbuf->tx_index;
}

int8_t gs_ringbuf_get_rxindex(struct gs_ringbuf *ringbuf)
{
	if(!ringbuf)
		return -1;

	return ringbuf->rx_index;
}


struct spis_packet *gs_ringbuf_get_rxpacket(struct gs_ringbuf *ringbuf)
{
	uint32_t offset = 0;
	struct spis_packet *packet = NULL;

	if(!ringbuf){
		NRF_LOG_DEBUG("ringbuf empty!");
		return NULL;
	}

	offset = ringbuf->rx_index;
	if(offset >= ringbuf->spis_packet_count)
	{
		offset = 0;
	}

	packet = ringbuf->packetlist + offset;
	if(packet->status == BUF_FREE){
		ringbuf->rx_index = offset + 1;
		packet->status = BUF_IN_RF;
		NRF_LOG_ERROR("get rxpacket index %d!",packet->index);
		return packet;
	}

	NRF_LOG_DEBUG("current spis packet busying!");
	return NULL;
}

struct spis_packet *gs_ringbuf_get_txpacket(struct gs_ringbuf *ringbuf)
{
	uint32_t offset = 0;
	struct spis_packet *packet = NULL;

	if(!ringbuf)
		return NULL;

	offset = ringbuf->tx_index;
	if(offset >= ringbuf->spis_packet_count)
	{
		offset = 0;
	}

	//if this packet fill full by rf,it can be used in spis.
	packet = ringbuf->packetlist + offset;
	if(packet->status == BUF_OUT_RF){
		ringbuf->tx_index = offset + 1;
		packet->status = BUF_IN_SPI;
		//NRF_LOG_ERROR("spis get txpacket index %d!",packet->index);
		return packet;
	}

	//NRF_LOG_ERROR("current spis packet [index%d/status%d] busying!",packet->index,packet->status);
	return NULL;
}

static void ringbuf_dump(struct gs_ringbuf *ringbuf)
{
}

uint8_t *rf_request_payload_addr(struct gs_ringbuf *ringbuf,uint8_t size)
{
	struct spis_packet *packet = ringbuf->current;
	uint8_t remind , *rf_payload_addr = NULL;

	if(packet == NULL){
		uint8_t offset = 0;
		offset = ringbuf->rx_index;
		if(offset >= ringbuf->spis_packet_count)
			offset = 0;

		packet = ringbuf->packetlist + offset;
		if(packet->status == BUF_FREE){
			packet->status = BUF_IN_RF;
			ringbuf->rx_index = offset + 1;
			ringbuf->current = packet;
			packet->remind = ringbuf->spis_packet_size - GS_SPI_HEAD_LENGTH ; //2Byte head
			//NRF_LOG_ERROR("rf get rxpacket index %d",packet->index);
		}
		else{
			//NRF_LOG_ERROR("rf get rxpacket index %d error",packet->index);
			ringbuf_dump(ringbuf);
			//NRF_LOG_ERROR("rf get payload from ringbuf error");
			return NULL;
		}
	}

	remind = packet->remind;
	if((remind - size) >= 0){
		rf_payload_addr = (packet->addr + GS_SPI_HEAD_LENGTH ) +\
						  ((ringbuf->spis_packet_size - GS_SPI_HEAD_LENGTH ) - remind);
		remind -= size;
	}
	else{
		ringbuf->current = NULL;
		packet->status = BUF_OUT_RF;
		packet->availed = ringbuf->spis_packet_size - remind;
		return rf_request_payload_addr(ringbuf,size);
	}

	packet->remind = remind;
	if(!rf_payload_addr)
	{
		NRF_LOG_INFO("rf get payload addr from packet:%d offset:%d remind %d",
				packet->index,(remind + size) / size,remind );
	}

	return rf_payload_addr;
}


struct gs_ringbuf *gs_ringbuf_get_by_devid(uint8_t devid)
{
	if(devid > g_gs_ringbuf_availabe_count)
	{
		NRF_LOG_ERROR("devid is big than availabe ringbuf count");
		return NULL;
	}

	return  &gs_ringbuf[devid];
}

uint8_t  gs_ringbuf_list_init(uint8_t count,uint8_t size,uint8_t device)
{
	uint8_t index = 0;
	int8_t err_code = 0;

	g_gs_ringbuf_availabe_count = 0;
	for(index = 0 ; index < device; index++)
	{
		err_code = gs_ringbuf_init(&gs_ringbuf[index],count,size);
		if(err_code != 0)
		{
			NRF_LOG_INFO("gs ringbuf request %d buff error!",index);
			return g_gs_ringbuf_availabe_count;
		}
		else {
			g_gs_ringbuf_availabe_count++;
		}
	}

	return g_gs_ringbuf_availabe_count;
}

uint8_t gs_ringbuf_get_availabe_count(void)
{
	return g_gs_ringbuf_availabe_count;
}

void gs_ringbuf_list_uninit(void)
{
	uint8_t index = 0;

	for(index = 0; index < g_gs_ringbuf_availabe_count; index++)
	{
		struct gs_ringbuf *ringbuf = &gs_ringbuf[index];
		gs_ringbuf_exit(ringbuf);
	}
}
