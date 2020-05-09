#ifndef GSBOARD_H
#define GSBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

#define LEDS_NUMBER    0

#define LED_0          6
#define LED_1          7
#define LED_2          27

#define LEDS_ACTIVE_STATE 0
#define LEDS_INV_MASK  LEDS_MASK
#define LEDS_LIST { LED_0, LED_1, LED_2 }

#define GS_UI_LED_0      0
#define GS_UI_LED_1      1
#define GS_UI_LED_2      2

//spis define
#define GS_SPIS_SCK_PIN		15
#define GS_SPIS_MOSI_PIN	17
#define GS_SPIS_MISO_PIN	16
#define GS_SPIS_CSN_PIN		19
#define GS_SPIS_READY_PIN	6

#define GS_SPIS_TASK_PIN	18
#define GS_SPIS_EVENT_PIN	20

#define GS_SPIS_DEFAULT_DEF_CHARACTER 0x55
#define GS_SPIS_DEFAULT_ORC_CHARACTER 0xAA
#define GS_SPIS_IRQ_PRIORITY_LEVEL	2

//i2c slave 
#define APP_TWIS_SDA		
#define APP_TWIS_SCL	

#if 0
//default resouse
#define GS_DEFAULT_CHANNEL_TABLE		{4,11,16,20,28,33,44,56,62,71}
#define GS_DEFAULT_DEVICE_TABLE			{0,1}
#define GS_DEFAULT_DEVICE_NUMBER		sizeof(GS_DEFAULT_DEVICE_NUMBER)
#define GS_DEFAULT_GZLL_PAYLOAD_SIZE	32
#define GS_DEFAULT_SPIS_PACKET_COUNT	7   //max
#define GS_DEFAULT_PACKET_HEAD_SIZE		2	
#define GS_DEFAULT_BASE_ADDRESS_00		(('G' << 24) + ('S' << 16) + ('R' << 8) + ('F' << 0))
#define GS_DEFAULT_BASE_ADDRESS_01		(('A' << 24) + ('B' << 16) + ('C' << 8) + ('D' << 0))
#define GS_DEFAULT_ADDR_PREFIX_PIPE		{0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8}
#endif 

// serialization APPLICATION board - temp. setup for running serialized MEMU tests
#define SER_APP_RX_PIN              23    // UART RX pin number.
#define SER_APP_TX_PIN              24    // UART TX pin number.
#define SER_APP_CTS_PIN             2     // UART Clear To Send pin number.
#define SER_APP_RTS_PIN             25    // UART Request To Send pin number.

#define SER_CON_SPIS_SCK_PIN        27    // SPI SCK signal.
#define SER_CON_SPIS_MOSI_PIN       2     // SPI MOSI signal.
#define SER_CON_SPIS_MISO_PIN       26    // SPI MISO signal.
#define SER_CON_SPIS_CSN_PIN        23    // SPI CSN signal.
#define SER_CON_SPIS_RDY_PIN        25    // SPI READY GPIO pin number.
#define SER_CON_SPIS_REQ_PIN        24    // SPI REQUEST GPIO pin number.

#define BUTTONS_NUMBER	0
#define BUTTONS_ACTIVE_STATE 0

#ifdef __cplusplus
}
#endif

#endif
