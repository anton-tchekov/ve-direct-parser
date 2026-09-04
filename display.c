#include "display.h"
#include "gfx.h"
#include "data.h"
#include "voltage.h"
#include "battery.h"
#include <stdarg.h>
#include <stdio.h>

static void printstr(int x, int y, int font, const char *s)
{
	set_font(font ? 0 : 1);
	if(!font)
	{
		set_color(180, 180, 180);
	}

	render_str(x * 300 + 5, y * 80 + font * 28 + 150, s);
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

static void printxy(int x, int y, const char *fmt, ...)
{
	char buf[64];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	render_str(x, y, buf);
	va_end(ap);
}

static void show_charge_level(double voltage)
{
	double percent = voltage_to_percent(voltage);
	printxy(5, 10, "Battery Charge Level: %6.2f %%", percent);
	render_battery_bar(8, 50, percent);
}

static void mppt_specific(VictronData data, bool censor)
{
	if(censor)
	{
		for(int i = 6; i < 11; ++i)
		{
			data.SerialNumber[i] = '*';
		}
	}

	show_charge_level(get_battery_volts(&data));

	if(data.ErrorCode)
	{
		printfmt(0, 1, 0, "Error (%d):", data.ErrorCode);
		set_color(255, 0, 0);
		printfmt(0, 1, 1, " %s", get_error_msg(&data));
		set_color(255, 255, 255);
	}

	printstr(0, 3, 0, "Serial Number:");
	printfmt(0, 3, 1, " %s", data.SerialNumber);
	printstr(0, 4, 0, "Firmware Version:");
	printfmt(0, 4, 1, " %d.%02d", data.FirmwareVersion / 100, data.FirmwareVersion % 100);

	printstr(1, 2, 0, "Battery Voltage:");
	printfmt(1, 2, 1, " %5.2f V", get_battery_volts(&data));
	printstr(1, 3, 0, "Load Current:");
	printfmt(1, 3, 1, " %5.2f A", get_load_amps(&data));
	printstr(1, 4, 0, "Load Power:");
	printfmt(1, 4, 1, " %5.2f W", get_load_watts(&data));
	printstr(1, 5, 0, "Load State:");
	printfmt(1, 5, 1, " %s", get_load_state(&data));
	printstr(1, 6, 0, "State of Operation:");
	printfmt(1, 6, 1, " %s", get_state_of_operation(&data));

	printstr(2, 2, 0, "PV Voltage:");
	printfmt(2, 2, 1, " %5.2f V", get_pv_volts(&data));
	printstr(2, 3, 0, "PV Current:");
	printfmt(2, 3, 1, " %5.2f A", get_pv_amps(&data));
	printstr(2, 4, 0, "PV Power:");
	printfmt(2, 4, 1, " %5.2f W", get_pv_watts(&data));
	printstr(2, 5, 0, "MPPT State:");
	printfmt(2, 5, 1, " %s", get_tracker_operation_mode(&data));
	if(data.OffReason)
	{
		printstr(2, 6, 0, "Off Reason:");
		printfmt(2, 6, 1, " %s", get_off_reason(&data));
	}

	printstr(1, 8, 0, "Yield Today:");
	printfmt(1, 8, 1, " %d Wh", get_yield_today_wh(&data));
	printstr(1, 9, 0, "Maximum Power Today:");
	printfmt(1, 9, 1, " %d W", data.MaximumPowerToday);

	printstr(2, 8, 0, "Yield Yesterday:");
	printfmt(2, 8, 1, " %d Wh", get_yield_yesterday_wh(&data));
	printstr(2, 9, 0, "Maximum Power Yesterday:");
	printfmt(2, 9, 1, " %d W", data.MaximumPowerYesterday);

	printstr(0, 8, 0, "Day Sequence Number:");
	printfmt(0, 8, 1, " %d", data.DaySequenceNumber);
	printstr(0, 9, 0, "Yield Total:");
	printfmt(0, 9, 1, " %d Wh", get_yield_total_wh(&data));
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

	const char *name = get_device_name(&data);

	printstr(0, 0, 0, "Device:");
	printfmt(0, 0, 1, " %s", name);

	printstr(0, 2, 0, "Product ID:");
	printfmt(0, 2, 1, " 0x%04X", data.ProductId);

	if(is_mppt(name))
	{
		mppt_specific(data, censor);
	}
}
