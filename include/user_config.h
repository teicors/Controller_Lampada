#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

	// UART config
	#define SERIAL_BAUD_RATE 115200
/*
	// ESP SDK config
	#define LWIP_OPEN_SRC
	#define USE_US_TIMER

	// Default types
	#define __CORRECT_ISO_CPP_STDLIB_H_PROTO
	#include <limits.h>
	#include <stdint.h>

	// Override c_types.h include and remove buggy espconn
	#define _C_TYPES_H_
	#define _NO_ESPCON_

	// Updated, compatible version of c_types.h
	// Just removed types declared in <stdint.h>
	#include <espinc/c_types_compatible.h>

	// System API declarations
	#include <esp_systemapi.h>

	// C++ Support
	#include <esp_cplusplus.h>
	// Extended string conversion for compatibility
	#include <stringconversion.h>
	// Network base API
	#include <espinc/lwip_includes.h>

 */
	// Beta boards
	#define BOARD_ESP01

#ifdef __cplusplus
}
#endif

#endif


/*
 * Hardware SPI mode:
 * GND      (GND)         GND
 * VCC      (VCC)         3.3v
 * D0       (CLK)         GPIO14   D5
 * D1       (MOSI)        GPIO13   D7
 * RES      (RESET)       GPIO16   D0
 * DC       (DC)          GPIO0    D3
 * CS       (CS)          GPIO2    D4
 */
    
#define TFT_SCLK 	14 /* D5 */
#define TFT_MOSI 	13 /* D7 */
#define TFT_RST  	16 /* D0 */
#define	TFT_DC   	15 /* D8 */ // 15 8  
#define TFT_CS   	4  /* D2 */ //  4 2

#define PIN_BUTTON      5  /* D1 */ //  5 1
#define PIN_PWM         2  /* D4 */ //  2 4
#define PIN_BUZZER      0  /* D3 */ //  0 3