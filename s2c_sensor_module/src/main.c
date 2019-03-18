/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include <s2c_utils.h>

// Function prototypes
uint8_t get_pinstrap_id(void);
inline float uint16ToC(uint16_t data);

void configure_adc(void);
void configure_can(void);
void configure_i2c(void);

void adc_callback(struct adc_module *const module);
void i2c_callback(struct i2c_master_module *const module);

void loop_adc(void);
void loop_i2c(void);
void loop_can(void);

// Board management variables
uint8_t board_id = 255;
enum s2c_board_type board_type = S2C_BOARD_OTHER;
struct s2c_board_config board_config;

// ASF driver instances
struct adc_module adc_instance;
struct can_module can_instance;
struct i2c_master_module i2c_instance;

// ADC variables
uint16_t adc_sample_buffer[ADC_NUM_SAMPLES] = {0}; // stores single channel conversion samples
uint32_t adc_channel[ADC_NUM_CHANNELS] = {AN0, AN1, AN2, AN3}; // stores ADC input pins in the order that they will be read
uint16_t adc_channel_vals[ADC_NUM_CHANNELS] = {0}; // stores the final averaged value of each channel's conversion
uint8_t adc_channel_index = 0; // index of current channel being read
bool adc_section_done = false; // true when all adc cannels have been read

// I2C variables
struct i2c_master_packet wr_packet, rd_packet;
struct i2c_master_module i2c_master_instance;
uint16_t i2c_temperature_vals[I2C_NUM_TEMP_SENSORS] = {0};
static uint8_t registerAddress = 0x07;
static uint8_t buffer[8];
uint16_t ret;
bool i2c_section_done = false;

// CAN variables
//TODO


/**
 * \brief Gets board ID from pinstrap configuration
 * 
 * \return Board ID
 * 
 */
uint8_t get_pinstrap_id(void) {
	static uint8_t id = 255;
	// If not initialized, initialize
	if(id == 255) {
		int input = port_group_get_input_level(&PORTA, PINSTRAPS);
		id = (input & PINSTRAP_0) | ((input & PINSTRAP_1) << 1) | ((input & PINSTRAP_2) << 2) | ((input & PINSTRAP_3) << 3);
	}
	return id;
}

/**
 * \brief Converts a raw 16-bit integer value into degrees Celsius 
 * 
 * \return floating point temperature value in degrees Celsius
 * 
 */
inline float uint16ToC(uint16_t data) {
	
	return (float)data * 0.02 - 273.15;
}

// Configuration functions

void configure_adc(void) {
	struct adc_config config;
	adc_get_config_defaults(&config);
	
	config.clock_prescaler = ADC_CLOCK_PRESCALER_DIV8;
	config.reference       = ADC_REFERENCE_INTVCC2;
	config.positive_input  = ADC_POSITIVE_INPUT_PIN5;
	config.resolution      = ADC_RESOLUTION_10BIT;
	
	adc_init(&adc_instance, ADC0, &config);
	
	adc_enable(&adc_instance);
	
	adc_register_callback(&adc_instance, adc_callback, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adc_instance, ADC_CALLBACK_READ_BUFFER);
}

void configure_i2c(void) {
	struct i2c_master_config config_i2c;
	i2c_master_get_config_defaults(&config_i2c);
	
	config_i2c.pinmux_pad0 = I2C_SDA_PIN;
	config_i2c.pinmux_pad1 = I2C_SCL_PIN;
	config_i2c.buffer_timeout = 65535;


	/* Initialize and enable device with config */
	while(i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c) != STATUS_OK);

	i2c_master_enable(&i2c_master_instance);
	
	wr_packet.data_length = 1;
	wr_packet.data = &registerAddress;
}

void configure_can(void) {
	/* Set up the CAN TX/RX pins */
	struct system_pinmux_config pin_config;
	system_pinmux_get_config_defaults(&pin_config);
	pin_config.mux_position = CAN_TX_MUX_SETTING;
	system_pinmux_pin_set_config(CAN_TX_PIN, &pin_config);
	pin_config.mux_position = CAN_RX_MUX_SETTING;
	system_pinmux_pin_set_config(CAN_RX_PIN, &pin_config);

	/* Initialize the module. */
	struct can_config config_can;
	can_get_config_defaults(&config_can);
	can_init(&can_instance, CAN_MODULE, &config_can);

	can_start(&can_instance);

	/* Enable interrupts for this CAN module */
	system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_CAN0);
	can_enable_interrupt(&can_instance, CAN_PROTOCOL_ERROR_ARBITRATION
	| CAN_PROTOCOL_ERROR_DATA);
	
	/* Set standby pin LOW on transceiver */
	struct port_config config_port;
	port_get_config_defaults(&config_port);
	config_port.direction = PORT_PIN_DIR_OUTPUT;
	config_port.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(CAN_STBY_PIN, &config_port);
	port_pin_set_output_level(CAN_STBY_PIN, false);
}

// Callback functions

void adc_callback(struct adc_module *const module) {
	// Average all samples and store value in channel's buffer
	adc_channel_vals[adc_channel_index] = 0;
	for(int i = 0; i < ADC_NUM_SAMPLES; i++) {
		adc_channel_vals[adc_channel_index] += adc_sample_buffer[i];	
	}
	adc_channel_vals[adc_channel_index] >>= ADC_SAMPLE_DIV;
	
	// If there are still more channels to process, then set up next channel and start the sampling
	if(adc_channel_index < board_config.adc_channels - 1) {
		++adc_channel_index;
		adc_set_positive_input(&adc_instance, adc_channel[adc_channel_index]);
		adc_read_buffer_job(&adc_instance, adc_sample_buffer, ADC_NUM_SAMPLES);
		
	} else {
		adc_section_done = true;
	}
}

// Loop functions

void loop_adc(void) {
	// Make sure this is the start of a sequence, and not in the middle of one
	if(adc_channel_index == 0) {
		adc_set_positive_input(&adc_instance, adc_channel[adc_channel_index]);
		adc_read_buffer_job(&adc_instance, adc_sample_buffer, ADC_NUM_SAMPLES);
	}
	//TODO: anything else? is there even a point to this function?
}

void loop_i2c(void) {
	//TODO: Dion to add stuff
	switch(board_type) {
	case S2C_BOARD_WHEEL:
		wr_packet.address = I2C_MLX_WHEEL_ID;
		//rd_packet.address = SENSOR1_ADDRESS;
		while(i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &wr_packet) == STATUS_BUSY);
		while(i2c_master_read_packet_wait(&i2c_master_instance, &rd_packet) == STATUS_BUSY);
		i2c_temperature_vals[I2C_INNER_TEMP] = uint16ToC(buffer[0] | buffer[1] << 8);
		break;
			
	case S2C_BOARD_TIRE_TEMP:
		// read sensor 1 (0x5A) (INNER)
		wr_packet.address = I2C_MLX_INNER_ID;
		//rd_packet.address = SENSOR1_ADDRESS;
		while(i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &wr_packet) == STATUS_BUSY);
		while(i2c_master_read_packet_wait(&i2c_master_instance, &rd_packet) == STATUS_BUSY);
		i2c_temperature_vals[I2C_INNER_TEMP] = buffer[0] | buffer[1] << 8;
		//printf("Inner Band Temperature: %.2f\n", temp);
			
		// read sensor 2 (0x5B) (MIDDLE)
		wr_packet.address = I2C_MLX_MIDDLE_ID;
		//rd_packet.address = SENSOR2_ADDRESS;
		while(i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &wr_packet) == STATUS_BUSY);
		while(i2c_master_read_packet_wait(&i2c_master_instance, &rd_packet) == STATUS_BUSY);
		i2c_temperature_vals[I2C_MIDDLE_TEMP] = buffer[0] | buffer[1] << 8;
		//printf("Middle Band Temperature: %.2f\n", temp);
			
		// read sensor 3 (0x5C) (OUTER)
		wr_packet.address = I2C_MLX_OUTER_ID;
		//rd_packet.address = SENSOR3_ADDRESS;
		while(i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &wr_packet) == STATUS_BUSY);
		while(i2c_master_read_packet_wait(&i2c_master_instance, &rd_packet) == STATUS_BUSY);
		i2c_temperature_vals[I2C_OUTER_TEMP] = buffer[0] | buffer[1] << 8;
		break;
	case S2C_BOARD_RADIATOR:
	case S2C_BOARD_OTHER:
	default:
		// do nothing
		break;
	}
	i2c_section_done = true;
}

void loop_can(void) {
	struct can_tx_element tx_elem;
	can_get_tx_buffer_element_defaults(&tx_elem);
	tx_elem.T0.bit.XTD = 1;
	
	switch(board_type) {
	case S2C_BOARD_WHEEL:
		tx_elem.T1.bit.DLC = 4;
		convert_16_bit_to_byte_array(adc_channel_vals[0], tx_elem.data);
		convert_16_bit_to_byte_array(i2c_temperature_vals[I2C_BRAKE_TEMP], tx_elem.data + 2);
		break;
		
	case S2C_BOARD_TIRE_TEMP:
		tx_elem.T1.bit.DLC = 6;
		convert_16_bit_to_byte_array(i2c_temperature_vals[I2C_OUTER_TEMP], tx_elem.data);
		convert_16_bit_to_byte_array(i2c_temperature_vals[I2C_MIDDLE_TEMP], tx_elem.data + 2);
		convert_16_bit_to_byte_array(i2c_temperature_vals[I2C_INNER_TEMP], tx_elem.data + 4);
		break;
		
	case S2C_BOARD_RADIATOR:
		tx_elem.T1.bit.DLC = 4;
		convert_16_bit_to_byte_array(adc_channel_vals[0], tx_elem.data);
		convert_16_bit_to_byte_array(adc_channel_vals[1], tx_elem.data + 2);
		
		/*//Dummy values
		tx_elem.data[0] = 0x80 & 0xFF;
		tx_elem.data[1] = 0x30 & 0xFF;
		can_set_tx_buffer_element(&can_instance, &tx_elem, 0);
		can_tx_transfer_request(&can_instance, 1);*/
		break;
	}
}


int main (void)
{
	system_init();

	// If code is configured to use pinstraps, do so. If not, leave at default
	if(USE_PINSTRAPS) {
		board_id = get_pinstrap_id();
	}
	board_type = get_board_type_from_id(board_id);
	
	switch(board_type) {
	case S2C_BOARD_WHEEL:
		S2C_BOARD_WHEEL_CONFIG(board_config);
		break;
	
	case S2C_BOARD_TIRE_TEMP:
		S2C_BOARD_TIRE_TEMP_CONFIG(board_config);
		break;
		
	case S2C_BOARD_RADIATOR:
		S2C_BOARD_RADIATOR_CONFIG(board_config);
		break;
	//default: do anything?
	}
	// Confirm that there is no violation that could lead to the adc channel index being greater than the sample array
	Assert(board_config.adc_channels <= ADC_NUM_CHANNELS);
	
	// Configure ADC and I2C depending on board configuration
	if(board_config.use_adc) {
		configure_adc();
	}
	if(board_config.use_i2c) {
		configure_i2c();
	}
	configure_can(); // this is always configured. any use cases where it shouldn't be?
	
	system_interrupt_enable_global();
	
	while (1) {
		// 1. read ADC, if needed
		// 2. read I2C, if needed
		// 3. send data over CAN
		// 
		
		if(board_config.use_adc) loop_adc();
		if(board_config.use_i2c) loop_i2c();
		
		// Send data over CAN once it is all available. Would it be more efficient to send it as it's partially available?
		if(adc_section_done && i2c_section_done) {
			loop_can();
			adc_section_done = i2c_section_done = false;
		}
	}
}

void CAN0_Handler(void)
{
	volatile uint32_t status;
	status = can_read_interrupt_status(&can_instance);
	
	if ((status & CAN_PROTOCOL_ERROR_ARBITRATION)
	|| (status & CAN_PROTOCOL_ERROR_DATA)) {
		can_clear_interrupt_status(&can_instance, CAN_PROTOCOL_ERROR_ARBITRATION
		| CAN_PROTOCOL_ERROR_DATA);
		//printf("Protocol error, please double check the clock in two boards. \r\n\r\n");
	}
}