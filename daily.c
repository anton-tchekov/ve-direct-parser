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

static void render_day(int i)
{
	char buf[64];

	int x = START_X + i * BAR_SPACING;
	int y = START_Y - 16;

	render_str_centered(x, y, format_day(buf, sizeof(buf), i));

	y -= 16;

	render_str_centered(x, y, format_date(buf, sizeof(buf), i));
}

void display_daily(void)
{
	set_color(255, 255, 255);
	set_font(2);

	int num_days = 30;
	for(int i = 0; i < num_days; ++i)
	{
		render_day(i);
	}
}
