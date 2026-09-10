#include "export.h"
#include "data.h"
#include <stdio.h>

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
	}

	printf("\n}\n");
}
