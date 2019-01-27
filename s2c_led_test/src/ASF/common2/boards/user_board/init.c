/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

#if defined(__GNUC__)
void board_init(void) WEAK __attribute__((alias("system_board_init")));
#elif defined(__ICCARM__)
void board_init(void);
#  pragma weak board_init=system_board_init
#endif

void system_board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	struct port_config config;
	port_get_config_defaults(&config);
	
	config.direction = PORT_PIN_DIR_OUTPUT;
	config.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(LED_USER_PIN, &config);
	
#if USE_PINSTRAPS
	config.direction = PORT_PIN_DIR_INPUT;
	config.input_pull = PORT_PIN_PULL_UP;
	port_group_set_config(&PORTA, PINSTRAPS, &config);
#endif
}