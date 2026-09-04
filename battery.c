#include "battery.h"
#include "gfx.h"

#define N_BARS  46
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

static void r_to_g(float percent)
{
	if(percent < 50.0f)
	{
		set_color(255, 255.0f * (percent / 50.0f), 0);
	}
	else
	{
		set_color(255.0f * ((100.0f - percent) / 50.0f), 255, 0);
	}
}

void render_battery_bar(int x, int y, double percent)
{
	corners(x, y, N_BARS * SPACING - (SPACING - BAR_W) + 2 * PADDING,
		BAR_H + 2 * PADDING, 15, 3);

	int count = percent / 100.0 * N_BARS;
	for(int i = 0; i < N_BARS; ++i)
	{
		if(i < count)
		{
			r_to_g(i * 100.0 / N_BARS);
		}
		else
		{
			set_color(60, 60, 60);
		}

		fill_rect(x + PADDING + i * SPACING, y + PADDING, BAR_W, BAR_H);
	}
}
