/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include <conf_board.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup group_common_boards
 * \defgroup user_board_group User board
 *
 * @{
 */

void system_board_init(void);

/** Name string macro */
#define BOARD_NAME                "SENSE2CAN"

// On-board LED
#define LED_USER_PORT			PORT_PA28
#define LED_USER_PIN			PIN_PA28

// ADC inputs
#define AN0						ADC_POSITIVE_INPUT_PIN0
#define AN1						ADC_POSITIVE_INPUT_PIN1
#define AN2						ADC_POSITIVE_INPUT_PIN4
#define AN3						ADC_POSITIVE_INPUT_PIN5

// I2C
#define I2C_MASTER_MODULE		SERCOM2
#define I2C_SDA_PIN				PIN_PA08D_SERCOM2_PAD0
#define I2C_SCL_PIN				PIN_PA09D_SERCOM2_PAD1

// Pinstraps
#define PINSTRAP_0				PORT_PA00
#define PINSTRAP_1				PORT_PA01
#define PINSTRAP_2				PORT_PA22
#define PINSTRAP_3				PORT_PA27
#define PINSTRAPS				PINSTRAP_0 | PINSTRAP_1 | PINSTRAP_2 | PINSTRAP_3

// CAN
#define CAN_STBY_PIN			PIN_PA23
#define CAN_MODULE              CAN0
#define CAN_TX_PIN              PIN_PA24G_CAN0_TX
#define CAN_TX_MUX_SETTING      MUX_PA24G_CAN0_TX
#define CAN_RX_PIN              PIN_PA25G_CAN0_RX
#define CAN_RX_MUX_SETTING      MUX_PA25G_CAN0_RX

/** @} */

#ifdef __cplusplus
}
#endif

#endif // USER_BOARD_H
