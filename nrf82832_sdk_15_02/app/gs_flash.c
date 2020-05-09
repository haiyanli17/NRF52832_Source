/*************************************************************************
  > File Name: gs_flash.c
  > Author: spchen
  > Mail: spchen@grandstream.cn 
  > Created Time: 2019年07月17日 星期三 10时28分30秒
 ************************************************************************/
#include "nrf_log.h"
#include "nrf_fstorage.h"
#include "nrf_strerror.h"
#include "nrf_fstorage_nvmc.h"
#include "sdk_errors.h"
#include "nrf_soc.h"

#include "gs_prx.h"
#include "app_timer.h"
#include "bsp.h"

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
	.evt_handler = fstorage_evt_handler,
	.start_addr = 0x3e000,
	.end_addr   = 0x3ffff,
};

static uint32_t nrf5_flash_end_addr_get()
{
	uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
	uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
	uint32_t const code_sz         = NRF_FICR->CODESIZE;

	return (bootloader_addr != 0xFFFFFFFF ?
			bootloader_addr : (code_sz * page_sz));
}

static void power_manage(void)
{
	__WFE();
}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
	if (p_evt->result != NRF_SUCCESS) {
		NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
		return;
	}

	switch (p_evt->id)
	{
		case NRF_FSTORAGE_EVT_WRITE_RESULT:
			{
				NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
						p_evt->len, p_evt->addr);
			} break;
		case NRF_FSTORAGE_EVT_ERASE_RESULT:
			{
				NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
						p_evt->len, p_evt->addr);
			} break;

		default:
			break;
	}
}

static void print_flash_info(nrf_fstorage_t * p_fstorage)
{
	NRF_LOG_INFO("========| flash info |========");
	NRF_LOG_INFO("erase unit: \t%d bytes",      p_fstorage->p_flash_info->erase_unit);
	NRF_LOG_INFO("program unit: \t%d bytes",    p_fstorage->p_flash_info->program_unit);
	NRF_LOG_INFO("==============================");
}


void wait_for_flash_ready(void)
{
	/* While fstorage is busy, sleep and wait for an event. */
	while (nrf_fstorage_is_busy(&fstorage))
	{
		power_manage();
	}
}

static uint32_t round_up_u32(uint32_t len)
{
	if (len % sizeof(uint32_t)) {
		return (len + sizeof(uint32_t) - (len % sizeof(uint32_t)));
	}

	return len;
}

int fstorage_read(uint32_t addr, void * data ,uint32_t len)
{
	ret_code_t rc;
	len = round_up_u32(len);

	/* Read data. */
	rc = nrf_fstorage_read(&fstorage, addr, data, len);
	if (rc != NRF_SUCCESS) {
		NRF_LOG_INFO("nrf_fstorage_read() returned: %s\r\n",nrf_strerror_get(rc));
		return rc;
	}
	
	return rc;
}

int fstorage_write( uint32_t addr, void const * p_data,uint32_t len)
{
	len = round_up_u32(len);
	ret_code_t rc = nrf_fstorage_write(&fstorage, addr, p_data, len, NULL);
	if (rc != NRF_SUCCESS) {
		NRF_LOG_INFO("nrf_fstorage_write() returned: %s\r\n",nrf_strerror_get(rc));
	}

	return rc;
}

int fstorage_erase(uint32_t addr, uint32_t pages_cnt)
{
	ret_code_t rc = nrf_fstorage_erase(&fstorage, addr, pages_cnt, NULL);
	if (rc != NRF_SUCCESS) {
		NRF_LOG_INFO("nrf_fstorage_erase() returned: %s\r\n",nrf_strerror_get(rc));
	}

	return rc;
}

int gs_flash_init(struct gs_flash *flash)
{
	uint32_t err_code;
	nrf_fstorage_api_t * p_fs_api;
	struct gs_twis_reg *reglist = (struct gs_twis_reg *)flash->flash_addr;
	
	p_fs_api = &nrf_fstorage_nvmc;
	fstorage.start_addr 	= flash->flash_start;
	fstorage.end_addr		= flash->flash_stop;
	err_code = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
	APP_ERROR_CHECK(err_code);

	print_flash_info(&fstorage);
	(void) nrf5_flash_end_addr_get();

	/*read i2c configure from flash*/
	nrf_fstorage_read(&fstorage, flash->flash_start, reglist, round_up_u32(256));

	if (reglist->flash_init != 0x01) {
		memset(reglist,0x00,0xff);
		reglist->flash_init = 0x01;
		fstorage_erase(flash->flash_start,1);
		wait_for_flash_ready();
		fstorage_write(flash->flash_start,reglist, 256);
	}
	else { 
		memset(reglist,0x00,0x60);
		reglist->flash_init = 0x01;
	}
	
	NRF_LOG_INFO("nrf_fstorage_init get reglist %x\n",reglist);

	return 0;
}
