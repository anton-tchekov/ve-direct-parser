#include "display.h"
#include "gfx.h"
#include "data.h"
#include "util.h"
#include <stdarg.h>
#include <stdio.h>

#define CENSOR 1

int w = 0;

static void printstr(int x, int y, int font, const char *s)
{
	set_font(font ? 0 : 1);
	if(!font)
	{
		set_color(180, 180, 180);
	}

	render_str(x * w / 2 + 5, y * 65 + font * 28 + 150, s);
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

typedef struct
{
	double Percent;
	double Voltage;
} VMap;

static const VMap vmap_lifepo4[] =
{
	{   0.0, 2.50 },
	{  10.0, 3.00 },
	{  20.0, 3.20 },
	{  30.0, 3.22 },
	{  40.0, 3.25 },
	{  50.0, 3.26 },
	{  60.0, 3.27 },
	{  70.0, 3.30 },
	{  80.0, 3.32 },
	{  90.0, 3.35 },
	{ 100.0, 3.40 }
};

static int series_cells(double voltage)
{
	if(voltage < 16.0)
	{
		return 4;
	}

	if(voltage < 32.0)
	{
		return 8;
	}

	return 16;
}

static double voltage_to_percent(const VMap *vmap, int len, double voltage)
{
	int cells = series_cells(voltage);
	for(int i = 0; i < len; ++i)
	{
		double v_cur = vmap[i].Voltage * cells;
		if(voltage < v_cur)
		{
			if(i == 0)
			{
				return vmap[i].Percent;
			}

			double v_prev = vmap[i - 1].Voltage * cells;
			double pos = (voltage - v_prev) / (v_cur - v_prev);
			double p_cur = vmap[i].Percent;
			double p_prev = vmap[i - 1].Percent;
			return p_prev + (p_cur - p_prev) * pos;
		}
	}

	return vmap[len - 1].Percent;
}

#define N_BARS  40
#define PADDING  8
#define SPACING 10
#define BAR_W    6
#define BAR_H   30

static void corners(int x, int y, int w, int h, int l, int t)
{
	fill_rect(x, y, l, t);
	fill_rect(x, y + t, t, l - t);

	fill_rect(x + w - l, y, l, t);
	fill_rect(x + w - t, y + t, t, l - t);

	fill_rect(x, y + h - t, l, t);
	fill_rect(x, y + h - l, t, l - t);

	fill_rect(x + w - l, y + h - t, l, t);
	fill_rect(x + w - t, y + h - l, t, l - t);
}

static void bar(int x, int y, double percent)
{
	corners(x, y, N_BARS * SPACING - (SPACING - BAR_W) + 2 * PADDING,
		BAR_H + 2 * PADDING, 15, 3);

	int count = percent / 100.0 * N_BARS;
	for(int i = 0; i < count; ++i)
	{
		fill_rect(x + PADDING + i * SPACING, y + SPACING, BAR_W, BAR_H);
	}
}

static void charge_level(double voltage)
{
	double percent = voltage_to_percent(vmap_lifepo4, ARRLEN(vmap_lifepo4), voltage);

	printxy(5, 10, "Battery Charge Level: %6.2f %%", percent);
	bar(10, 50, percent);
}

void display_data(void)
{
	set_color(255, 255, 255);

	VictronData data;
	data_get(&data);
	if(data.ProductId == 0)
	{
		printstr(0, 0, 1, " Waiting for data ...");
		return;
	}

	int h;
	SDL_GetWindowSize(window, &w, &h);

	if(CENSOR)
	{
		for(int i = 6; i < 11; ++i)
		{
			data.SerialNumber[i] = '*';
		}
	}

	charge_level(get_battery_volts(&data));

	// Left
	printstr(0, 0, 0, "Device:");
	printfmt(0, 0, 1, " %s", get_device_name(&data));
	if(data.ErrorCode)
	{
		printfmt(0, 1, 0, "Error (%d):", data.ErrorCode);
		set_color(255, 0, 0);
		printfmt(0, 1, 1, " %s", get_error_msg(&data));
		set_color(255, 255, 255);
	}

	printstr(0, 2, 0, "State of Operation:");
	printfmt(0, 2, 1, " %s", get_state_of_operation(&data));
	printstr(0, 3, 0, "Battery Voltage:");
	printfmt(0, 3, 1, " %5.2f V", get_battery_volts(&data));
	printstr(0, 4, 0, "Load Current:");
	printfmt(0, 4, 1, " %5.2f A", get_load_amps(&data));
	printstr(0, 5, 0, "Load Power:");
	printfmt(0, 5, 1, " %5.2f W", get_load_watts(&data));
	printstr(0, 6, 0, "Load State:");
	printfmt(0, 6, 1, " %s", get_load_state(&data));
	printstr(0, 7, 0, "Yield Today:");
	printfmt(0, 7, 1, " %d Wh", get_yield_today_wh(&data));
	printstr(0, 8, 0, "Maximum Power Today:");
	printfmt(0, 8, 1, " %d W", data.MaximumPowerToday);
	printstr(0, 9, 0, "Day Sequence Number:");
	printfmt(0, 9, 1, " %d", data.DaySequenceNumber);

	// Right
	printstr(1, 0, 0, "Product ID:");
	printfmt(1, 0, 1, " 0x%04X", data.ProductId);
	printstr(1, 1, 0, "Serial Number:");
	printfmt(1, 1, 1, " %s", data.SerialNumber);
	printstr(1, 2, 0, "PV Voltage:");
	printfmt(1, 2, 1, " %5.2f V", get_pv_volts(&data));
	printstr(1, 3, 0, "PV Current:");
	printfmt(1, 3, 1, " %5.2f A", get_pv_amps(&data));
	printstr(1, 4, 0, "PV Power:");
	printfmt(1, 4, 1, " %5.2f W", get_pv_watts(&data));
	printstr(1, 5, 0, "MPPT State:");
	printfmt(1, 5, 1, " %s", get_tracker_operation_mode(&data));
	if(data.OffReason)
	{
		printstr(1, 6, 0, "Off Reason:");
		printfmt(1, 6, 1, " %s", get_off_reason(&data));
	}

	printstr(1, 7, 0, "Yield Yesterday:");
	printfmt(1, 7, 1, " %d Wh", get_yield_yesterday_wh(&data));
	printstr(1, 8, 0, "Maximum Power Yesterday:");
	printfmt(1, 8, 1, " %d W", data.MaximumPowerYesterday);
	printstr(1, 9, 0, "Yield Total:");
	printfmt(1, 9, 1, " %d Wh", get_yield_total_wh(&data));
}
