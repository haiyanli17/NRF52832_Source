/*************************************************************************
	> File Name: gs_event.h
	> Author: spchen
	> Mail: spchen@grandstream.cn 
	> Created Time: 2019年03月20日 星期三 15时41分56秒
 ************************************************************************/
#ifndef __GS_EVENT_H__
#define __GS_EVENT_H__

#define GS_GPIO_HIGH 1
#define GS_GPIO_LOW  0
#define GS_SPIS_READY_PIN	6
struct gs_gpios {
	unsigned char  ready_line;
	unsigned char init;
};

void gs_set_ready_line(void);
void gs_clear_ready_line(void);
void gs_gpio_init(struct gs_gpios *);

#endif 

