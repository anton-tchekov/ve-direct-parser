#include "voltage.h"
#include "util.h"

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

static double vmap_voltage_to_percent(const VMap *vmap, int len, double voltage)
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

double voltage_to_percent(double voltage)
{
	return vmap_voltage_to_percent(vmap_lifepo4, ARRLEN(vmap_lifepo4), voltage);
}
