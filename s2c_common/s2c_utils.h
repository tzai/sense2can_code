/*
 * s2c_utils.h
 *
 * Created: 2019-01-27 1:01:34 AM
 *  Author: Tal Zaitsev
 */ 


#ifndef S2C_UTILS_H_
#define S2C_UTILS_H_

// SENSE2CAN board types
enum s2c_board_type {
	/* S2C board mounted near wheels:
	 * - 1 analog input:
	 * --> suspension potentiometer
	 * - 1 I2C slave:
	 * --> brake temperature sensor
	 * 
	 * CAN setup:
	 * - frame 1: 4 bytes
	 * --> bytes 0 & 1: suspension potentiometer
	 * --> bytes 2 & 3: brake temperature
	 */
	S2C_BOARD_WHEEL,
	/* S2C board used for tire temperature bar:
	 * - 3 I2C slaves:
	 * --> outer tire temp
	 * --> middle tire temp
	 * --> inner tire temp
	 * 
	 * CAN setup:
	 * - frame 1: 6 bytes
	 * --> bytes 0 & 1: outer tire temp
	 * --> bytes 2 & 3: middle tire temp
	 * --> bytes 4 & 5: inner tire temp
	 */
	S2C_BOARD_TIRE_TEMP,
	/* S2C board mounted near radiator:
	 * - 2 analog inputs:
	 * --> radiator inlet temperature
	 * -- radiator outlet temperature
	 * 
	 * CAN setup:
	 * - frame 1: 4 bytes
	 * --> bytes 0 & 1: radiator inlet temperature
	 * --> bytes 2 & 3: radiator outlet temperature
	 */
	S2C_BOARD_RADIATOR,
	/* Any other S2C board use */
	S2C_BOARD_OTHER
};

// S2C configuration struct
struct s2c_board_config {
	bool use_adc;			// True if this configuration needs ADC
	uint8_t adc_channels;	// Number of ADC inputs defined for this configuration
	bool use_i2c;			// True if this configuration needs I2C
};

#define S2C_BOARD_WHEEL_CONFIG(x)			{ x.use_adc = true; x.adc_channels = 1; x.use_i2c = true; }
#define S2C_BOARD_TIRE_TEMP_CONFIG(x)		{ x.use_adc = false; x.adc_channels = 0; x.use_i2c = true; }
#define S2C_BOARD_RADIATOR_CONFIG(x)		{ x.use_adc = true; x.adc_channels = 2; x.use_i2c = false; }

/*
 * Returns board type based on the board ID.
 * ID = 0 to 3: S2C_BOARD_WHEEL
 * ID = 4 to 7: S2C_BOARD_TIRE_TEMP
 * ID = 8:		S2C_BOARD_RADIATOR
 * All others:	S2C_BOARD_OTHER
 * 
 * Order follows CAN bus order:
 *           __
 *          /  \
 *   2,6 FL ---- FR 3,7
 *         / CM \ <------- CENTRAL MODULE
 *        |      |
 *        |      | RAD 8
 *        \      /
 *  1,5 RL |----| RR 0,4
 *         |____|
 *      
 */
static inline enum s2c_board_type get_board_type_from_id(uint8_t id) {
	if(id < 4) { // covers 0, 1, 2, 3
		return S2C_BOARD_WHEEL;
	} else if(id < 8) { // 4, 5, 6, 7
		return S2C_BOARD_TIRE_TEMP;
	} else if(id == 8) {
		return S2C_BOARD_RADIATOR;
	} else {
		return S2C_BOARD_OTHER;
	}
}

// CAN stuff
#define CAN_ID_BASE 0x700 // avoids clashing with potential bootloader messages
#define CAN_MSG_ID(id, msg_id)	 CAN_ID_BASE + (id << 4) + msg_id

// ADC stuff
#define ADC_NUM_SAMPLES			4
#define ADC_SAMPLE_DIV			2
#define ADC_NUM_CHANNELS		4

// I2C stuff
#define I2C_BRAKE_TEMP			0
#define I2C_OUTER_TEMP			0
#define I2C_MIDDLE_TEMP			1
#define I2C_INNER_TEMP			2
#define I2C_NUM_TEMP_SENSORS	3

#define I2C_MLX_BASE_ID			0x5A
#define I2C_MLX_WHEEL_ID		I2C_MLX_BASE_ID
#define I2C_MLX_INNER_ID		I2C_MLX_BASE_ID
#define I2C_MLX_MIDDLE_ID		I2C_MLX_BASE_ID + 0x1
#define I2C_MLX_OUTER_ID		I2C_MLX_BASE_ID + 0x2


#endif /* S2C_UTILS_H_ */