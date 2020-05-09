/*************************************************************************
	> File Name: gs_event.c
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月20日 星期三 14时52分46秒
 ************************************************************************/
#include "nrf_log.h"
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "gs_gpio.h"

struct gs_gpios *g_gpio;
void gs_set_ready_line(void)
{
	//set low
	//nrf_gpio_cfg_output(g_gpio->ready_line);
	nrf_gpio_pin_write(g_gpio->ready_line,0);
}

void gs_clear_ready_line(void)
{
	//set high
	//nrf_gpio_cfg_output(g_gpio->ready_line);
	nrf_gpio_pin_write(g_gpio->ready_line,1);
}

void gs_gpio_init(struct gs_gpios *gpio)
{
	//init ready line
	g_gpio = gpio;
	nrf_gpio_cfg_output(g_gpio->ready_line);
	nrf_gpio_pin_write(g_gpio->ready_line,g_gpio->init);
}
