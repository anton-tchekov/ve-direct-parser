#ifndef __DATA_H__
#define __DATA_H__

#include <stdbool.h>
#include "hex.h"

typedef struct
{
	int BatteryVoltage;         // mV
	int PanelVoltage;           // mV
	int PanelPower;             // W
	int LoadCurrent;            // mA
	int LoadOn;                 // true/false
	int OffReasonId;            // Index
	int YieldTotal;             // 0.01 kWh
	int YieldToday;             // 0.01 kWh
	int MaximumPowerToday;      // W
	int YieldYesterday;         // 0.01 kWh
	int MaximumPowerYesterday;  // W
	int ErrorCode;              // Index
	int StateOfOperationId;     // Index
	char FirmwareVersion[8];    // Firmware Version
	int ProductId;              // Id
	int DaySequenceNumber;      // 0-364
	int TrackerOperationModeId; // Index
	char SerialNumber[12];      // LLYYMMSSSSS

	// Processed Info
	const char *DeviceName;
	const char *ErrorMsg;
	const char *OffReason;
	const char *LoadState;
	const char *StateOfOperation;
	const char *TrackerOperationMode;
	bool IsMPPT;

	HistoryTotalRecord TotalRecord;
	HistoryDayRecord DailyRecord[31];
} VictronData;

void data_update(VictronData *data);
void data_get(VictronData *data);

double get_battery_volts(VictronData *data);
double get_pv_volts(VictronData *data);
double get_pv_watts(VictronData *data);
double get_pv_amps(VictronData *data);
double get_load_amps(VictronData *data);
double get_load_watts(VictronData *data);
int get_yield_total_wh(VictronData *data);
int get_yield_today_wh(VictronData *data);
int get_yield_yesterday_wh(VictronData *data);
const char *format_fw_version(const char *in);

#endif
