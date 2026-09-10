#include "data.h"
#include "gfx.h"
#include "fields.h"
#include "util.h"
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

const char *format_fw_version(const char *in)
{
	static char out[64];

	if(isupper(in[0]) && isdigit(in[1]) && isdigit(in[2]) &&
		isdigit(in[3]) && in[4] == '\0')
	{
		snprintf(out, sizeof(out), "Candidate %c for %c.%c%c\n",
			in[0], in[1], in[2], in[3]);
	}
	else if(isdigit(in[0]) && isdigit(in[1]) &&
		isdigit(in[2]) && in[3] == '\0')
	{
		snprintf(out, sizeof(out), "%c.%c%c\n",
			in[0], in[1], in[2]);
	}
	else
	{
		copy_str(out, sizeof(out), in);
	}

	return out;
}
