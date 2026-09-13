#include "display.h"
#include "gfx.h"
#include "data.h"
#include "util.h"
#include "voltage.h"
#include "battery.h"
#include "daily.h"
#include <stdarg.h>
#include <stdio.h>

static void printstr(int x, int y, int font, const char *s)
{
	set_font(font ? 0 : 1);
	if(!font)
	{
		set_color(180, 180, 180);
	}

	render_str(x * 300 + 5, y * 80 + font * 28 + 167, s);
	if(!font)
	{
		set_color(255, 255, 255);
	}
}

static void printfmt(int x, int y, int font, const char *fmt, ...)
{
	char buf[64];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	printstr(x, y, font, buf);
	va_end(ap);
}

static void printat(int x, int y, const char *label, const char *fmt, ...)
{
	printstr(x, y, 0, label);

	char buf[64];
	va_list ap;
	va_start(ap, fmt);
	buf[0] = ' ';
	vsnprintf(buf + 1, sizeof(buf) - 1, fmt, ap);
	printstr(x, y, 1, buf);
	va_end(ap);
}

static void show_charge_level(double voltage)
{
	double percent = voltage_to_percent(voltage);
	printat(0, -2, "Battery Percent:", "%6.2f %%", percent);
	render_battery_bar(8, 87, percent);
}

static void mppt_specific(VictronData *data, bool censor)
{
	if(censor)
	{
		for(int i = 6; i < 11; ++i)
		{
			data->SerialNumber[i] = '*';
		}
	}

	show_charge_level(get_battery_volts(data));

	if(data->ErrorCode)
	{
		printfmt(0, 1, 0, "Error:");
		set_color(255, 0, 0);
		printfmt(0, 1, 1, " %s", data->ErrorMsg);
		set_color(255, 255, 255);
	}

	printat(0, 3, "Serial Number:", "%s", data->SerialNumber);
	printat(0, 4, "Firmware Version:", "%s", format_fw_version(data->FirmwareVersion));

	printat(1, 2, "Battery Voltage:", "%5.2f V", get_battery_volts(data));
	printat(1, 3, "Load Current:", "%5.2f A", get_load_amps(data));
	printat(1, 4, "Load Power:", "%5.2f W", get_load_watts(data));
	printat(1, 5, "Load State:", "%s", data->LoadState);
	printat(1, 6, "State of Operation:", "%s", data->StateOfOperation);

	printat(2, 2, "PV Voltage:", "%5.2f V", get_pv_volts(data));
	printat(2, 3, "PV Current:", "%5.2f A", get_pv_amps(data));
	printat(2, 4, "PV Power:", "%5.2f W", get_pv_watts(data));
	printat(2, 5, "MPPT State:", "%s", data->TrackerOperationMode);
	if(data->OffReasonId)
	{
		printat(2, 6, "Off Reason:", "%s", data->OffReason);
	}

	printat(1, 8, "Yield Today:", "%d Wh", get_yield_today_wh(data));
	printat(1, 9, "Maximum Power Today:", "%d W", data->MaximumPowerToday);

	printat(2, 8, "Yield Yesterday:", "%d Wh", get_yield_yesterday_wh(data));
	printat(2, 9, "Maximum Power Yesterday:", "%d W", data->MaximumPowerYesterday);

	printat(0, 8, "Day Sequence Number:", "%d", data->DaySequenceNumber);
	printat(0, 9, "Yield Total:", "%d Wh", get_yield_total_wh(data));

	// Help
	printat(2, -2, "View Past Data:", "^ Drag Up ^");
	printat(2, -1, "Export Data as JSON:", "Press E");
	printat(2,  0, "Hide Serial Number:", "Press C");

	HistoryTotalRecord *t = &data->TotalRecord;
	if(t->Available)
	{
		printat(4, 2, "PV Voltage Maximum:", "%5.2f V", t->PanelVoltageMaximum / 100.0);
		printat(4, 3, "Battery Voltage Maximum:", "%5.2f V", t->BatteryVoltageMaximum / 100.0);
		printat(4, 4, "Battery Voltage Minimum:", "%5.2f V", t->BatteryVoltageMinimum / 100.0);
		printat(4, 5, "History Available:", "%d Days", t->NumberOfDaysAvailable);
		display_daily(data);
	}
}

void display_data(bool censor)
{
	set_color(255, 255, 255);

	VictronData data;
	data_get(&data);
	if(data.ProductId == 0)
	{
		printstr(0, 0, 1, " Waiting for data ...");
		return;
	}

	printat(0, 0, "Device:", "%s", data.DeviceName);
	printat(0, 2, "Product ID:", "0x%04X", data.ProductId);

	if(data.IsMPPT)
	{
		mppt_specific(&data, censor);
	}
}
