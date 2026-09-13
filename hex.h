#ifndef __HEX_H__
#define __HEX_H__

#include <stdint.h>
#include <stdbool.h>

// register 0x1050 - 0x106E
// Size 34 bytes
typedef struct
{
	// uint8_t Reserved;               // = 0
	uint32_t Yield;                 // 0.01 kWh
	uint32_t Consumed;              // 0.01 kWh
	uint16_t BatteryVoltageMaximum; // 0.01 V
	uint16_t BatteryVoltageMinimum; // 0.01 V
	uint8_t ErrorDatabase;          // = 0
	uint8_t Errors[4];              // -
	uint16_t TimeBulk;              // min
	uint16_t TimeAbsorbtion;        // min
	uint16_t TimeFloat;             // min
	uint32_t PowerMaximum;          // W
	uint16_t BatteryCurrentMaximum; // 0.1 A
	uint16_t PanelVoltageMaximum;   // 0.01 V
	uint16_t DaySequenceNumber;     // -
} HistoryDayRecord;

// register 0x104F
// Size 19 or 34 bytes
typedef struct
{
	bool Available;

	// uint8_t Reserved;               // Reserved (0 or 1)
	uint8_t ErrorDatabase;          // = 0
	uint8_t Errors[4];              // -
	uint32_t TotalYieldUser;        // 0.01 kWh
	uint32_t TotalYieldSystem;      // 0.01 kWh
	uint16_t PanelVoltageMaximum;   // 0.01 V
	uint16_t BatteryVoltageMaximum; // 0.01 V
	uint8_t NumberOfDaysAvailable;  // -
	uint16_t BatteryVoltageMinimum; // 0.01 V
	// uint8_t Padding[13];            // -
} HistoryTotalRecord;

int hex_checksum(const uint8_t *data, int len);
int hex_convert(char *out, const uint8_t *data, int len);
int hex_command_get(char *out, int id);

#endif
