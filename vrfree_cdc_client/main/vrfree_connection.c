#include "vrfree_connection.h"
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "tinyusb.h"
#include "vrfree_types.h"

static const char *TAG = "vrfree_connection";

vrfree_data_rx_state_t 	m_rxState;
vrfree_state_t 			m_StatemachineState;
usb_data_type_id		m_lastUSBDataType;

uint8_t m_rxBuffer[VRFREE_USB_RX_BUFLEN];
int m_rxBufferIdx;
uint8_t m_txBuffer[VRFREE_USB_TX_BUFLEN];
int m_txBufferIdx;

void _vrfree_handleNewCommand(uint8_t* buf, int len);
void _vrfree_handleNewData(uint8_t* buf, int len);
void _vrfree_handleMoreData(uint8_t* data, int len);

void vrfree_on_positionData(USB_position_report_t* report) {
	//ESP_LOGI(TAG,"Position data: device 0x%x,\ttimestamp: %d",report->address,report->timestamp);
}

void vrfree_on_trackerIMUData(USB_IMU_data_short_t* report){
	//ESP_LOGI(TAG,"Single IMU data: device 0x%x,\ttimestamp: %d",report->address,report->timestamp);
}

void vrfree_on_handIMUData(USB_IMU_data_long_t* report) {
	//ESP_LOGI(TAG,"Multi-IMU data: device 0x%x,\ttimestamp: %d",report->address,report->timestamp);
}


void vrfree_connection_init() {
	memset(m_rxBuffer,0,VRFREE_USB_RX_BUFLEN);
	memset(m_txBuffer,0,VRFREE_USB_TX_BUFLEN);
	m_rxState 			= VRFREE_RX_IDLE;
	m_StatemachineState = VRFREE_STATE_IDLE;
	m_lastUSBDataType	= 0;
	m_rxBufferIdx		= 0;
	m_txBufferIdx		= 0;
}

// if data is received by CDC call that function with the array address and the length of the elements.
void vrfree_connection_onNewData(uint8_t* buf, int len) {
	ESP_LOGI(TAG,"received %d bytes", len);

	if(len <= 0) {
		return;
	}

	switch(m_rxState) {
		case VRFREE_RX_IDLE:
		// ok, that is the beginning of a new packet. have a look at it:
		switch(buf[0]) {
			case CMD_TYPE:
			_vrfree_handleNewCommand(buf+1,len-1);
			break;

			case DATA_TYPE:
			_vrfree_handleNewData(buf+1,len-1);
			break;

			default:
			ESP_LOGW(TAG,"Received unknown packet type: 0x%x",buf[0]);
			break;
		}
		break;

		case VRFREE_RX_PARTIALLY_ASSEMBLED:
		_vrfree_handleMoreData(buf,len);
		break;

		default:
		break;
	}
}

void vrfree_connection_sendCommand(usb_command_t cmd) {
	ESP_LOGI(TAG,"Sending command 0x%x.",cmd);
	m_txBuffer[0] = CMD_TYPE;
	m_txBuffer[1] = cmd;
	tud_cdc_write(m_txBuffer,2);
}

void vrfree_connection_sendData(uint8_t* buf, int len) {
	ESP_LOGI(TAG,"Sending %d bytes of data.",len);
	m_txBuffer[0] = DATA_TYPE;
	memcpy(m_txBuffer + 1, buf, len);
	tud_cdc_write(m_txBuffer,len + 1);
}


void _vrfree_handleNewCommand(uint8_t* buf, int len) {
	switch(buf[0]) {
		case CMD_CDC_DEVICE_CONNECTED:
		ESP_LOGI(TAG,"received command DEVICE_CONNECTED");
		break;

		case CMD_ENABLE_TRACKING_DATA:
		ESP_LOGI(TAG,"received command ENABLE_TRACKING");
		// should not happen...
		break;

		case CMD_DISABLE_OUTPUT:
		ESP_LOGI(TAG,"received command DISABLE_TRACKING");
		// should not happen...
		break;

		case CMD_ENABLE_DEBUG_DATA:
		ESP_LOGI(TAG,"received command ENABLE_DEBUG");
		// should not happen...
		break;

		case CMD_DISABLE_DEBUG_DATA:
		ESP_LOGI(TAG,"received command DISABLE_DEBUG");
		// should not happen...
		break;

		default:
		ESP_LOGW(TAG,"received unknown command 0x%x", buf[0]);
		// should not happen...
		break;
	}
}

void _vrfree_handleNewData(uint8_t* buf, int len) {
	uint8_t data_t_len;

	switch(buf[0]) {
		case POSITION_DATA_REPORT_ID:
		ESP_LOGI(TAG,"Received position data.");
		data_t_len = sizeof(USB_position_report_t);
		if(data_t_len == len-1) {
			m_lastUSBDataType = 0;
			vrfree_on_positionData((USB_position_report_t*)(buf+1));
		} else {
			m_lastUSBDataType = POSITION_DATA_REPORT_ID;
			m_rxState = VRFREE_RX_PARTIALLY_ASSEMBLED;
		}
		break;

		case SHORT_IMU_DATA_REPORT_ID:
		ESP_LOGI(TAG,"Received single IMU data.");
		data_t_len = sizeof(USB_IMU_data_short_t);
		if(data_t_len == len-1) {
			m_lastUSBDataType = 0;
			vrfree_on_trackerIMUData((USB_IMU_data_short_t*)(buf+1));
		} else {
			m_lastUSBDataType = SHORT_IMU_DATA_REPORT_ID;
			m_rxState = VRFREE_RX_PARTIALLY_ASSEMBLED;
		}
		break;
		
		case LONG_IMU_DATA_REPORT_ID:
		ESP_LOGI(TAG,"Received long IMU data.");
		data_t_len = sizeof(USB_IMU_data_long_t);
		if(data_t_len == len-1) {
			m_lastUSBDataType = 0;
			vrfree_on_handIMUData((USB_IMU_data_long_t*)(buf+1));
		} else {
			m_lastUSBDataType = LONG_IMU_DATA_REPORT_ID;
			m_rxState = VRFREE_RX_PARTIALLY_ASSEMBLED;
		}
		break;
		
		default:
		ESP_LOGW(TAG,"received unknown data type 0x%x", buf[0]);
		break;
	}

}

void _vrfree_handleMoreData(uint8_t* data, int len) {
	int total_length;
	bool data_complete = false;

	if(m_rxBufferIdx + len < VRFREE_USB_RX_BUFLEN) {
		memcpy(m_rxBuffer + m_rxBufferIdx,data,len);
		m_rxBufferIdx	 += len;
	} else {
		ESP_LOGW(TAG,"rxBuffer overflow error, dropping packet.");
		m_rxState 			= VRFREE_RX_IDLE;
		m_rxBufferIdx 		= 0;
		return;
	}

	switch(m_lastUSBDataType) {
		case POSITION_DATA_REPORT_ID:
		total_length = sizeof(USB_position_report_t);
		if(total_length == m_rxBufferIdx) {
			vrfree_on_positionData((USB_position_report_t*)(m_rxBuffer));
			data_complete = true;
		}
		break;

		case SHORT_IMU_DATA_REPORT_ID:
		total_length = sizeof(USB_IMU_data_short_t);
		if(total_length == m_rxBufferIdx) {
			vrfree_on_trackerIMUData((USB_IMU_data_short_t*)(m_rxBuffer));
			data_complete = true;
		}
		break;

		case LONG_IMU_DATA_REPORT_ID:
		total_length = sizeof(USB_IMU_data_long_t);
		if(total_length == m_rxBufferIdx) {
			vrfree_on_handIMUData((USB_IMU_data_long_t*)(m_rxBuffer));
			data_complete = true;
		}
		break;

		default:
		m_rxBufferIdx 		= 0;
		m_rxState 			= VRFREE_RX_IDLE;
		ESP_LOGW(TAG,"Unknown partial datatype!!! 0x%x",m_lastUSBDataType);
		return;
		break;
	}

	if(data_complete) {
		m_rxBufferIdx 		= 0;
		m_rxState 			= VRFREE_RX_IDLE;
		ESP_LOGI(TAG,"data completed");
	}

}