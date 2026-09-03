#ifndef __DATA_H__
#define __DATA_H__

typedef struct
{
	int BatteryVoltage;        // mV
	int PanelVoltage;          // mV
	int PanelPower;            // W
	int LoadCurrent;           // mA
	int LoadOn;                // true/false
	int OffReason;             // Index
	int YieldTotal;            // 0.01 kWh
	int YieldToday;            // 0.01 kWh
	int MaximumPowerToday;     // W
	int YieldYesterday;        // 0.01 kWh
	int MaximumPowerYesterday; // W
	int ErrorCode;             // Index
	int StateOfOperation;      // Index
	int FirmwareVersion;       // Id
	int ProductId;             // Id
	int DaySequenceNumber;     // 0-364
	int TrackerOperationMode;  // Index
	char SerialNumber[12];      // LLYYMMSSSSS
} VictronData;

void data_update(VictronData *data);
void data_get(VictronData *data);

double get_battery_volts(VictronData *data);
double get_pv_volts(VictronData *data);
double get_pv_watts(VictronData *data);
double get_pv_amps(VictronData *data);
double get_load_amps(VictronData *data);
double get_load_watts(VictronData *data);
const char *get_load_state(VictronData *data);
const char *get_off_reason(VictronData *data);
int get_yield_total_wh(VictronData *data);
int get_yield_today_wh(VictronData *data);
int get_yield_yesterday_wh(VictronData *data);
const char *get_error_msg(VictronData *data);
const char *get_state_of_operation(VictronData *data);
const char *get_tracker_operation_mode(VictronData *data);
const char *get_device_name(VictronData *data);
bool is_mppt(const char *name);

#endif
