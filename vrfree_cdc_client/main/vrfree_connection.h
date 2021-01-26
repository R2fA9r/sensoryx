#ifndef _VRFREE_CONNECTION_H
#define _VRFREE_CONNECTION_H

#include <stdbool.h>
#include <stdint.h>
#include "vrfree_types.h"

void vrfree_on_positionData(USB_position_report_t* report);
void vrfree_on_trackerIMUData(USB_IMU_data_short_t* report);
void vrfree_on_handIMUData(USB_IMU_data_long_t* report);


#define VRFREE_USB_RX_BUFLEN	256
#define VRFREE_USB_TX_BUFLEN	256

typedef enum {
	CMD_TYPE = 0x20,
	DATA_TYPE = 0x21,
} buffer_data_t;

typedef enum {
	POSITION_DATA_REPORT_ID = 0x02,
	SHORT_IMU_DATA_REPORT_ID = 0x03,
	LONG_IMU_DATA_REPORT_ID = 0x04,
} usb_data_type_id;

typedef enum {
	CMD_CDC_DEVICE_CONNECTED = 0x12,
	CMD_ENABLE_TRACKING_DATA = 0xf0,
	CMD_DISABLE_OUTPUT = 0xf1,
	CMD_ENABLE_DEBUG_DATA = 0xf2,
	CMD_DISABLE_DEBUG_DATA = 0xf3
} usb_command_t;

typedef enum {
	VRFREE_RX_IDLE = 0,
	VRFREE_RX_PARTIALLY_ASSEMBLED,
} vrfree_data_rx_state_t;

typedef enum {
	VRFREE_STATE_IDLE,
	VFREEE_STATE_STARTING,
	VRFREE_STATE_RUNNING
} vrfree_state_t;

// needs to be called once at the beginning
void vrfree_connection_init();
// if data is received by CDC call that function with the array address and the length of the elements.
void vrfree_connection_onNewData(uint8_t* buf, int len);
// to send a command:
void vrfree_connection_sendCommand(usb_command_t cmd);
// to send data:
void vrfree_connection_sendData(uint8_t* buf, int len);



#endif