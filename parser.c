#include "parser.h"
#include "util.h"
#include "data.h"
#include "gfx.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct
{
	int fd;
} SerialThreadParams;

static int parse_int(const char *a, const char *b, const char *v, int *out)
{
	if(strcmp(a, b))
	{
		return 0;
	}

	char *endptr;
	int val = strtol(v, &endptr, 0);
	if(*v && !*endptr)
	{
		*out = val;
		return 1;
	}

	return 0;
}

static void parse_line(char *line, int len)
{
	static VictronData data;

	const char *code = line;
	const char *value = NULL;
	for(int i = 0; i < len; ++i)
	{
		if(line[i] == '\t')
		{
			line[i] = '\0';
			value = line + i + 1;
			break;
		}
	}

	if(!value)
	{
		return;
	}

	if(!strcmp(code, "Checksum"))
	{
		// Checksum is not verified currently
		data_update(&data);
		return;
	}

	if(parse_int(code, "V",    value, &data.BatteryVoltage)) { return; }
	if(parse_int(code, "VPV",  value, &data.PanelVoltage)) { return; }
	if(parse_int(code, "PPV",  value, &data.PanelPower)) { return; }
	if(parse_int(code, "IL",   value, &data.LoadCurrent)) { return; }
	if(parse_int(code, "OR",   value, &data.OffReason)) { return; }
	if(parse_int(code, "H19",  value, &data.YieldTotal)) { return; }
	if(parse_int(code, "H20",  value, &data.YieldToday)) { return; }
	if(parse_int(code, "H21",  value, &data.MaximumPowerToday)) { return; }
	if(parse_int(code, "H22",  value, &data.YieldYesterday)) { return; }
	if(parse_int(code, "H23",  value, &data.MaximumPowerYesterday)) { return; }
	if(parse_int(code, "ERR",  value, &data.ErrorCode)) { return; }
	if(parse_int(code, "CS",   value, &data.StateOfOperation)) { return; }
	if(parse_int(code, "FW",   value, &data.FirmwareVersion)) { return; }
	if(parse_int(code, "HSDS", value, &data.DaySequenceNumber)) { return; }
	if(parse_int(code, "MPPT", value, &data.TrackerOperationMode)) { return; }
	if(parse_int(code, "PID",  value, &data.ProductId)) { return; }

	if(!strcmp(code, "LOAD"))
	{
		if(!strcmp(value, "OFF"))
		{
			data.LoadOn = 0;
		}
		if(!strcmp(value, "ON"))
		{
			data.LoadOn = 1;
		}
		return;
	}

	if(!strcmp(code, "SER#"))
	{
		strncpy(data.SerialNumber, value, sizeof(data.SerialNumber) - 1);
		data.SerialNumber[sizeof(data.SerialNumber) - 1] = '\0';
		return;
	}
}

static void parse_char(int c)
{
	static char buf[64];
	static int i;

	if(c == '\n')
	{
		buf[i] = '\0';
		parse_line(buf, i);
		i = 0;
	}
	else if(c != '\r')
	{
		if(i < (int)sizeof(buf) - 1)
		{
			buf[i++] = c;
		}
	}
}

static void *thread_serial(void *args)
{
	SerialThreadParams *params = args;
	int fd = params->fd;
	for(;;)
	{
		char buf[1024];
		int n = read(fd, buf, sizeof(buf));
		if(n < 0)
		{
			fprintf(stderr, "(read) Error %d: %s\n", errno, strerror(errno));
			break;
		}
		else if(n == 0)
		{
			fprintf(stderr, "(read) Disconnected\n");
			break;
		}

		for(int i = 0; i < n; ++i)
		{
			parse_char(buf[i]);
		}
	}

	gfx_send_quit_event();
	return NULL;
}

int parser_thread_start(int fd)
{
	static SerialThreadParams params;
	params.fd = fd;

	pthread_t thread;
	int ret = pthread_create(&thread, NULL, thread_serial, &params);
	if(ret)
	{
		fprintf(stderr, "(pthread_create) Error %d: %s\n", ret, strerror(ret));
		return 1;
	}

	return 0;
}
