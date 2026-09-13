#include "daily.h"
#include "gfx.h"
#include <time.h>
#include <locale.h>

#define START_X       0
#define START_Y     -50

#define BAR_W        60
#define BAR_SPACING 100

static const char *format_day(char *buf, size_t len, int i)
{
	if(i == 0)
	{
		snprintf(buf, len, "today");
	}
	else if(i == 1)
	{
		snprintf(buf, len, "yesterday");
	}
	else
	{
		snprintf(buf, len, "%d days ago", i);
	}

	return buf;
}

static const char *format_date(char *buf, size_t len, int i)
{
	time_t now = time(NULL);
	time_t past = now - (i * 24 * 60 * 60);
	struct tm *info = localtime(&past);

	setlocale(LC_TIME, "");
	strftime(buf, len, "%x", info);
	//strftime(buf, len, "%d.%m.%Y", info);

	return buf;
}

static int prints(int x, int y, const char *fmt, ...)
{
	char buf[64];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	render_str_centered(x, y, buf);
	va_end(ap);
	return y - 16;
}

static int printd(int x, int y, const char *s, const char *fmt, ...)
{
	char buf[64];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	render_str_centered(x, y, buf);
	y -= 16;
	set_color(128, 128, 128);
	render_str_centered(x, y, s);
	set_color(255, 255, 255);
	y -= 20;
	va_end(ap);
	return y;
}

static const char *format_time(int minutes)
{
	static char buf[32];
	if(minutes <= 59)
	{
		snprintf(buf, sizeof(buf), "%d min", minutes);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%d:%02d h", minutes / 60, minutes % 60);
	}

	return buf;
}

#define BAR_H 800
#define BAR_W  60
#define SEG_H   3
#define SEG_GAP 6

#define BAR_MAX  (BAR_H / SEG_GAP)

static void bar_color(int v)
{
	double n = v / (double)BAR_MAX;
	int g = 60.0 + n * (255.0 - 60.0);
	set_color(255, g, 0);
}

static void render_day(int idx, HistoryDayRecord *record, uint32_t maxyield)
{
	char buf[64];

	int x = START_X + idx * BAR_SPACING;
	int y = START_Y - 16;

	render_str_centered(x, y, format_day(buf, sizeof(buf), idx));
	y -= 16;
	render_str_centered(x, y, format_date(buf, sizeof(buf), idx));
	y -= 16;
	y = prints(x, y, "Day %d", record->DaySequenceNumber);
	y -= 12;
	y = printd(x, y, "Yield",    "%d Wh", record->Yield * 10);
	y = printd(x, y, "Consumed", "%d Wh", record->Consumed * 10);
	y = printd(x, y, "BAT Vmax", "%5.2f V", record->BatteryVoltageMaximum / 100.0);
	y = printd(x, y, "BAT Vmin", "%5.2f V", record->BatteryVoltageMinimum / 100.0);
	y = printd(x, y, "BAT Imax", "%5.2f A", record->BatteryCurrentMaximum / 10.0);
	y = printd(x, y, "PV Vmax",  "%5.2f V", record->PanelVoltageMaximum / 100.0);
	y = printd(x, y, "Pmax",     "%d W", record->PowerMaximum);
	y -= 12;
	y = printd(x, y, "Bulk",       "%s", format_time(record->TimeBulk));
	y = printd(x, y, "Absorption", "%s", format_time(record->TimeAbsorbtion));
	y = printd(x, y, "Float",      "%s", format_time(record->TimeFloat));
	y -= 12;

	double h = (double)record->Yield / (double)maxyield * (double)BAR_H;

	int seg = h / SEG_GAP;

	for(int i = 0; i < seg; ++i)
	{
		bar_color(i);
		fill_rect(x - BAR_W / 2, y - i * SEG_GAP, BAR_W, SEG_H);
	}

	set_color(255, 255, 255);
}

void display_daily(VictronData *data)
{
	set_color(255, 255, 255);
	set_font(2);

	int num_days = data->TotalRecord.NumberOfDaysAvailable;

	uint32_t maxyield = 0;
	for(int i = 0; i <= num_days; ++i)
	{
		HistoryDayRecord *record = &data->DailyRecord[i];
		if(record->Yield > maxyield)
		{
			maxyield = record->Yield;
		}
	}

	for(int i = 0; i <= num_days; ++i)
	{
		render_day(i, &data->DailyRecord[i], maxyield);
	}
}
