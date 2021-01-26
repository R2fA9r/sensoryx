#ifndef _VRFREE_TYPES_H_
#define _VRFREE_TYPES_H_
#include <stdint.h>
#include "vrfree_macros.h"

#define VRFREE_NUM_HAND_IMUS	13

typedef struct {
	float x;
	float y;
	float z;
} _ATTR_PACKED axis_data_f_t;

typedef struct {
	union {
		int16_t s;
		uint16_t u;
	} x;

	union {
		int16_t s;
		uint16_t u;
	} y;

	union {
		int16_t s;
		uint16_t u;
	}z;
}_ATTR_PACKED axis_data_t;

typedef struct {
	int16_t q0;
	int16_t q1;
	int16_t q2;
	int16_t q3;
} _ATTR_PACKED quat_t;

typedef uint32_t irq_timestamp_t;
typedef uint8_t RF_ADDR_t;


typedef struct {
	axis_data_f_t 		position;
	axis_data_t 		acceleration;
	irq_timestamp_t 	timestamp_mSec;
	RF_ADDR_t 			address;
} _ATTR_PACKED USB_position_report_t;

typedef struct {
	quat_t			quat;
	uint16_t		_reserved;
	uint8_t			fnButtonStatus;
	irq_timestamp_t timestamp_mSec;
	RF_ADDR_t 		address;
} _ATTR_PACKED USB_IMU_data_short_t;

typedef struct {
	quat_t			quat[VRFREE_NUM_HAND_IMUS];
	uint16_t		_reserved;
	uint8_t			pinchStatus;
	uint8_t			fnButtonStatus;
	irq_timestamp_t timestamp_mSec;
	RF_ADDR_t 		address;
}_ATTR_PACKED USB_IMU_data_long_t;

#endif