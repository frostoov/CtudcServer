#pragma once

/* Macros to provide branch prediction information to compiler */
#define START_DURATION_1							10
#define START_DURATION_2							20
#define I2C_ADDRESS_READ_MASK						0x01
#define I2C_ADDRESS_WRITE_MASK						0xFE
#define SEND_ACK									0x00
#define SEND_NACK									0x80
#define STOP_DURATION_1								10
#define STOP_DURATION_2								10
#define STOP_DURATION_3								10
#define usbBufferSize								65536
#define disableEvent								0
#define disableChar									0
#define writeTimeout								5000
#define interfaceMaskIn								0x00
#define resetInterface								0
#define ENABLE_MPSSE								0x02
#define MID_6MHZ									6000000
#define MID_30MHZ									30000000
#define DISABLE_CLOCK_DIVIDE						0x8A
#define ENABLE_CLOCK_DIVIDE							0x8B
#define MID_MAX_IN_BUF_SIZE							4096
#define MID_ECHO_COMMAND_ONCE						0
#define MID_ECHO_COMMAND_CONTINUOUSLY				1
#define MID_BAD_COMMAND_RESPONSE					0xFA
#define MID_ECHO_CMD_1								0xAA
#define MID_ECHO_CMD_2								0xAB
#define I2C_ENABLE_DRIVE_ONLY_ZERO					0x0002
#define MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO			0x9E
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER			0x00000030
#define I2C_TRANSFER_OPTIONS_START_BIT				0x00000001
#define I2C_TRANSFER_OPTIONS_STOP_BIT				0x00000002
#define I2C_TRANSFER_OPTIONS_BREAK_ON_NACK			0x00000004
#define I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE			0x00000008
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES	0x00000010
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS		0x00000020
#define I2C_TRANSFER_OPTIONS_NO_ADDRESS				0x00000040
#define I2C_CMD_GETDEVICEID_RD							0xF9
#define I2C_CMD_GETDEVICEID_WR							0xF8
#define I2C_GIVE_ACK									1
#define I2C_GIVE_NACK									0
#define I2C_DISABLE_3PHASE_CLOCKING						0x0001

#define MID_COMMENTED_AFTER_REVIEW_COMMENT  			0
#define MID_COMMENTED_AFTER_REVIEW_COMMENT_ENTRY_EXIT 	0

#define MID_NO_CHANNEL_FOUND	0

#define MID_NO_MPSSE					0
#define MID_MPSSE_AVAILABLE				1



#define MID_CMD_NOT_ECHOED				0
#define MID_CMD_ECHOED					1

/*clock*/
#define MID_SET_LOW_BYTE_DATA_BITS_CMD	0x80
#define MID_GET_LOW_BYTE_DATA_BITS_CMD	0x81
#define MID_SET_HIGH_BYTE_DATA_BITS_CMD	0x82
#define MID_GET_HIGH_BYTE_DATA_BITS_CMD	0x83
#define MID_SET_CLOCK_FREQUENCY_CMD		0x86
#define MID_SET_LOW_BYTE_DATA_BITS_DATA 0x13
#define MID_SET_HIGH_BYTE_DATA_BITS_DATA 0x0F

#define MID_LOOPBACK_FALSE				0
#define MID_LOOPBACK_TRUE				1
#define MID_TURN_ON_LOOPBACK_CMD		0x84
#define MID_TURN_OFF_LOOPBACK_CMD		0x85

#define MID_LEN_MAX_ERROR_STRING		500

/******************************************************************************/
/*								Macro defines								  */
/******************************************************************************/
/* Macros to be called before starting and after ending communication over a MPSSE channel.
Implement the lock/unlock only if really required, otherwise keep as placeholders */

#define USB_INPUT_BUFFER_SIZE			65536
#define USB_OUTPUT_BUFFER_SIZE			65536
#define DISABLE_EVENT					0
#define DISABLE_CHAR					0
#define DEVICE_READ_TIMEOUT_INFINITE    0
#define DEVICE_WRITE_TIMEOUT 			5000
#define INTERFACE_MASK_IN				0x00
#define INTERFACE_MASK_OUT				0x01
#define RESET_INTERFACE					0

/*MPSSE Control Commands*/
#define MPSSE_CMD_SET_DATA_BITS_LOWBYTE		0x80
#define MPSSE_CMD_SET_DATA_BITS_HIGHBYTE	0x82
#define MPSSE_CMD_GET_DATA_BITS_LOWBYTE		0x81
#define MPSSE_CMD_GET_DATA_BITS_HIGHBYTE	0x83

#define MPSSE_CMD_SEND_IMMEDIATE			0x87
#define MPSSE_CMD_ENABLE_3PHASE_CLOCKING	0x8C
#define MPSSE_CMD_DISABLE_3PHASE_CLOCKING	0x8D



/*MPSSE Data Commands - bit mode - MSB first */
#define MPSSE_CMD_DATA_OUT_BITS_POS_EDGE	0x12
#define MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE	0x13
#define MPSSE_CMD_DATA_IN_BITS_POS_EDGE		0x22
#define MPSSE_CMD_DATA_IN_BITS_NEG_EDGE		0x26
#define MPSSE_CMD_DATA_BITS_IN_POS_OUT_NEG_EDGE	0x33
#define MPSSE_CMD_DATA_BITS_IN_NEG_OUT_POS_EDGE	0x36


/*MPSSE Data Commands - byte mode - MSB first */
#define MPSSE_CMD_DATA_OUT_BYTES_POS_EDGE	0x10
#define MPSSE_CMD_DATA_OUT_BYTES_NEG_EDGE	0x11
#define MPSSE_CMD_DATA_IN_BYTES_POS_EDGE	0x20
#define MPSSE_CMD_DATA_IN_BYTES_NEG_EDGE	0x24
#define MPSSE_CMD_DATA_BYTES_IN_POS_OUT_NEG_EDGE	0x31
#define MPSSE_CMD_DATA_BYTES_IN_NEG_OUT_POS_EDGE	0x34


/*SCL & SDA directions*/
#define DIRECTION_SCLIN_SDAIN				0x10
#define DIRECTION_SCLOUT_SDAIN				0x11
#define DIRECTION_SCLIN_SDAOUT				0x12
#define DIRECTION_SCLOUT_SDAOUT				0x13

/*SCL & SDA values*/
#define VALUE_SCLLOW_SDALOW					0x00
#define VALUE_SCLHIGH_SDALOW				0x01
#define VALUE_SCLLOW_SDAHIGH				0x02
#define VALUE_SCLHIGH_SDAHIGH				0x03

/*Data size in bits*/
#define DATA_SIZE_8BITS						0x07
#define DATA_SIZE_1BIT						0x00


#define LATENCY_TIMER_MAX	255
#define CLOCKRATE_MAX		30000000
#define SET_LOW_BYTE_DATA_BITS	 0x80
#define GET_LOW_BYTE_DATA_BITS	 0x81
#define SET_HIGH_BYTE_DATA_BITS  0x82
#define GET_HIGH_BYTE_DATA_BITS  0x83
#define SET_CLOCK_FREQUENCY		 0x86
#define TURN_ON_LOOPBACK		 0x84
#define TURN_OFF_LOOPBACK		 0x85
