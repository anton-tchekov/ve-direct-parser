#include "data.h"
#include "gfx.h"
#include "fields.h"
#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
VictronData store;

void data_update(VictronData *data)
{
	pthread_mutex_lock(&mutex);
	store = *data;
	pthread_mutex_unlock(&mutex);
	main_thread_notify();
}

void data_get(VictronData *data)
{
	pthread_mutex_lock(&mutex);
	*data = store;
	pthread_mutex_unlock(&mutex);
}

double get_battery_volts(VictronData *data)
{
	return data->BatteryVoltage / 1000.0;
}

double get_pv_volts(VictronData *data)
{
	return data->PanelVoltage / 1000.0;
}

double get_pv_watts(VictronData *data)
{
	return data->PanelPower;
}

double get_pv_amps(VictronData *data)
{
	return get_pv_watts(data) / get_pv_volts(data);
}

double get_load_amps(VictronData *data)
{
	return data->LoadCurrent / 1000.0;
}

double get_load_watts(VictronData *data)
{
	return get_load_amps(data) * get_battery_volts(data);
}

const char *get_load_state(VictronData *data)
{
	return data->LoadOn ? "On" : "Off";
}

const char *get_off_reason(VictronData *data)
{
	return map_find(data->OffReason, map_or);
}

int get_yield_total_wh(VictronData *data)
{
	return data->YieldTotal * 10;
}

int get_yield_today_wh(VictronData *data)
{
	return data->YieldToday * 10;
}

int get_yield_yesterday_wh(VictronData *data)
{
	return data->YieldYesterday * 10;
}

const char *get_error_msg(VictronData *data)
{
	return map_find(data->ErrorCode, map_err);
}

const char *get_state_of_operation(VictronData *data)
{
	return map_find(data->StateOfOperation, map_cs);
}

const char *get_tracker_operation_mode(VictronData *data)
{
	return map_find(data->TrackerOperationMode, map_mppt);
}

const char *get_device_name(VictronData *data)
{
	return map_find(data->ProductId, map_devices);
}

bool is_mppt(const char *name)
{
	// It's an MPPT if it's got MPPT in the name lol
	return strstr(name, "MPPT") ? true : false;
}
