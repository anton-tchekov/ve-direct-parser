#include "export.h"
#include "data.h"
#include <stdio.h>
#include <time.h>

static const char *format_date(char *buf, size_t len, int i)
{
	time_t now = time(NULL);
	time_t past = now - (i * 24 * 60 * 60);
	struct tm *info = localtime(&past);
	strftime(buf, len, "%d.%m.%Y", info);
	return buf;
}

void export_data(void)
{
	VictronData data;
	data_get(&data);
	if(data.ProductId == 0)
	{
		return;
	}

	printf("{");
	printf("\n\t\"product_id\": \"0x%04X\"", data.ProductId);
	printf(",\n\t\"device_name\": \"%s\"", data.DeviceName);
	if(data.IsMPPT)
	{
		printf(",\n\t\"error_id\": %d", data.ErrorCode);
		printf(",\n\t\"error_message\": \"%s\"", data.ErrorMsg);
		printf(",\n\t\"serial_number\": \"%s\"", data.SerialNumber);
		printf(",\n\t\"firmware_version\": \"%s\"", data.FirmwareVersion);
		printf(",\n\t\"battery_voltage\": %.2f", get_battery_volts(&data));
		printf(",\n\t\"load_current\": %.2f", get_load_amps(&data));
		printf(",\n\t\"load_power:\": %.2f", get_load_watts(&data));
		printf(",\n\t\"load_on:\": %s", data.LoadOn ? "true" : "false");
		printf(",\n\t\"load_state:\": \"%s\"", data.LoadState);
		printf(",\n\t\"state_of_operation_id\": %d", data.StateOfOperationId);
		printf(",\n\t\"state_of_operation\": \"%s\"", data.StateOfOperation);
		printf(",\n\t\"pv_voltage\": %.2f", get_pv_volts(&data));
		printf(",\n\t\"pv_current\": %.2f", get_pv_amps(&data));
		printf(",\n\t\"pv_power\": %.2f", get_pv_watts(&data));
		printf(",\n\t\"tracker_operation_mode_id\": %d", data.TrackerOperationModeId);
		printf(",\n\t\"tracker_operation_mode\": \"%s\"", data.TrackerOperationMode);
		printf(",\n\t\"off_reason_id\": %d", data.OffReasonId);
		printf(",\n\t\"off_reason\": \"%s\"", data.OffReason);
		printf(",\n\t\"yield_today\": %d", get_yield_today_wh(&data));
		printf(",\n\t\"maximum_power_today\": %d", data.MaximumPowerToday);
		printf(",\n\t\"yield_yesterday\": %d", get_yield_yesterday_wh(&data));
		printf(",\n\t\"maximum_power_yesterday\": %d", data.MaximumPowerYesterday);
		printf(",\n\t\"day_sequence_number\": %d", data.DaySequenceNumber);
		printf(",\n\t\"yield_total\": %d", get_yield_total_wh(&data));

		HistoryTotalRecord *t = &data.TotalRecord;
		if(t->Available)
		{
			printf(",\n\t\"pv_voltage_maximum\": %.2f", t->PanelVoltageMaximum / 100.0);
			printf(",\n\t\"battery_voltage_maximum\": %.2f", t->BatteryVoltageMaximum / 100.0);
			printf(",\n\t\"battery_voltage_minimum\": %.2f", t->BatteryVoltageMinimum / 100.0);

			printf(",\n\t\"days\":"
					"\n\t[");

			char buf[32];
			for(int i = 0; i <= t->NumberOfDaysAvailable; ++i)
			{
				HistoryDayRecord *record = &data.DailyRecord[i];
				printf("\n\t\t{");
				printf("\n\t\t\t\"date\": \"%s\"", format_date(buf, sizeof(buf), i));
				printf(",\n\t\t\t\"day\": %d", record->DaySequenceNumber);
				printf(",\n\t\t\t\"yield\": %d", record->Yield * 10);
				printf(",\n\t\t\t\"consumed\": %d", record->Consumed * 10);
				printf(",\n\t\t\t\"battery_voltage_maximum\": %.2f", record->BatteryVoltageMaximum / 100.0);
				printf(",\n\t\t\t\"battery_voltage_minimum\": %.2f", record->BatteryVoltageMinimum / 100.0);
				printf(",\n\t\t\t\"battery_current_maximum\": %.2f", record->BatteryCurrentMaximum / 10.0);
				printf(",\n\t\t\t\"pv_voltage_maximum\": %.2f", record->PanelVoltageMaximum / 100.0);
				printf(",\n\t\t\t\"time_power_maximum\": %d", record->PowerMaximum);
				printf(",\n\t\t\t\"time_bulk\": %d", record->TimeBulk);
				printf(",\n\t\t\t\"time_absorption\": %d", record->TimeAbsorbtion);
				printf(",\n\t\t\t\"time_float\": %d", record->TimeFloat);
				printf("\n\t\t}%s", i == t->NumberOfDaysAvailable ? "" : ",");
			}

			printf("\n\t]");
		}
	}

	printf("\n}\n");
}
