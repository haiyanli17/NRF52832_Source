/*************************************************************************
	> File Name: gs_rf.c
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月12日 星期二 11时15分38秒
 ************************************************************************/
#ifdef CONFIG_NRF_GZLL
#include "gs_nrf_gzll.c"
#else
#include "gs_nrf_esb.c"
#endif

void gs_rf_disable()
{
#ifdef CONFIG_NRF_GZLL
	gs_gzll_disable();
#else
	gs_esb_disable();
#endif
	NRF_LOG_INFO("RF Disable!\n");
}

uint32_t gs_rf_enabled()
{
#ifdef CONFIG_NRF_GZLL
	return nrf_gzll_is_enabled();
#else
	return gs_esb_enabled();
#endif
}

uint32_t gs_rf_init()
{
	NRF_LOG_INFO("GrandStream RfAudio Init.");
#ifdef CONFIG_NRF_GZLL
	return gs_gzll_init();
#else
	return gs_esb_init();
#endif

	return -1;
}

uint32_t gs_rf_enable()
{
	NRF_LOG_INFO("RF Enable!\n.");
	gs_rf_init();
#ifdef CONFIG_NRF_GZLL
	return gs_gzll_enable();
#else
	return gs_esb_enable();
#endif
}




