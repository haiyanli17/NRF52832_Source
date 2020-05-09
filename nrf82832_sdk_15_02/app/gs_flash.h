#ifndef __GS_FLASH_H__
#define __GS_FLASH_H__

struct gs_flash {
	uint32_t flash_start;
	uint32_t flash_stop;
	void *flash_addr;
};


int gs_flash_init(struct gs_flash *flash);
void wait_for_flash_ready(void);
int fstorage_erase(uint32_t addr, uint32_t pages_cnt);
int fstorage_write( uint32_t addr, void const * p_data,uint32_t len);
int fstorage_read(uint32_t addr, void * data, uint32_t len);

#endif 


