#include "parser.h"
#include "util.h"
#include "data.h"
#include "gfx.h"
#include "fields.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>

#define DEBUG 0

#define REGISTER_TOTAL_HISTORY       0x104F
#define REGISTER_DAILY_HISTORY_START 0x1050
#define REGISTER_DAILY_HISTORY_END   0x106E

enum
{
	H_INIT,
	H_SEND,
	H_WAIT_RESPONSE,
	H_END
};

static pthread_t thread;
static int msg_pipe[2];
static int hex_id;
static int hex_state = H_INIT;
static int next_id = REGISTER_TOTAL_HISTORY;

typedef struct
{
	int serial_fd;
	int msg_fd;
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

static int parse_str(const char *a, const char *b,
	const char *v, char *out, int len)
{
	if(!strcmp(a, b))
	{
		copy_str(out, len, v);
		return 1;
	}

	return 0;
}

static void data_process(VictronData *data)
{
	data->DeviceName = map_find(data->ProductId, map_devices);
	data->ErrorMsg = map_find(data->ErrorCode, map_err);
	data->OffReason = map_find(data->OffReasonId, map_or);
	data->IsMPPT = strstr(data->DeviceName, "MPPT") ? true : false;
	data->LoadState = data->LoadOn ? "On" : "Off";
	data->StateOfOperation = map_find(data->StateOfOperationId, map_cs);
	data->TrackerOperationMode = map_find(data->TrackerOperationModeId, map_mppt);
}

static void parse_line(VictronData *data, const char *label, const char *value)
{
	if(parse_int(label, "V",    value, &data->BatteryVoltage)) { return; }
	if(parse_int(label, "VPV",  value, &data->PanelVoltage)) { return; }
	if(parse_int(label, "PPV",  value, &data->PanelPower)) { return; }
	if(parse_int(label, "IL",   value, &data->LoadCurrent)) { return; }
	if(parse_int(label, "OR",   value, &data->OffReasonId)) { return; }
	if(parse_int(label, "H19",  value, &data->YieldTotal)) { return; }
	if(parse_int(label, "H20",  value, &data->YieldToday)) { return; }
	if(parse_int(label, "H21",  value, &data->MaximumPowerToday)) { return; }
	if(parse_int(label, "H22",  value, &data->YieldYesterday)) { return; }
	if(parse_int(label, "H23",  value, &data->MaximumPowerYesterday)) { return; }
	if(parse_int(label, "ERR",  value, &data->ErrorCode)) { return; }
	if(parse_int(label, "CS",   value, &data->StateOfOperationId)) { return; }
	if(parse_int(label, "HSDS", value, &data->DaySequenceNumber)) { return; }
	if(parse_int(label, "MPPT", value, &data->TrackerOperationModeId)) { return; }
	if(parse_int(label, "PID",  value, &data->ProductId)) { return; }

	if(parse_str(label, "FW",   value, data->FirmwareVersion, sizeof(data->FirmwareVersion))) { return; }
	if(parse_str(label, "SER#", value, data->SerialNumber, sizeof(data->SerialNumber))) { return; }

	if(!strcmp(label, "LOAD"))
	{
		if(!strcmp(value, "OFF"))
		{
			data->LoadOn = 0;
		}

		if(!strcmp(value, "ON"))
		{
			data->LoadOn = 1;
		}

		return;
	}
}

enum
{
	S_WAIT_FOR_CR,
	S_WAIT_FOR_LF,
	S_READ_LABEL,
	S_READ_FIELD,
	S_READ_CHECKSUM,
	S_READ_HEX
};

static int hex_to_nibble(int n)
{
	if(n >= 'A' && n <= 'F')
	{
		return n - 'A' + 10;
	}
	else if(n >= '0' && n <= '9')
	{
		return n - '0';
	}

	return -1;
}

static uint16_t read16(const uint8_t *buf, int pos)
{
	return buf[pos] | (buf[pos + 1] << 8);
}

static uint32_t read32(const uint8_t *buf, int pos)
{
	return buf[pos] | (buf[pos + 1] << 8) |
		(buf[pos + 2] << 16) | (buf[pos + 3] << 24);
}

static void process_hex_get_response(VictronData *vd, uint8_t *data, int len)
{
	int get_response_id = read16(data, 1);
	if(get_response_id != hex_id)
	{
		return;
	}

	if(DEBUG)
	{
		printf("Success. Received response for correct ID (0x%04X)\n", get_response_id);
	}

	if(get_response_id == REGISTER_TOTAL_HISTORY)
	{
		if(DEBUG)
		{
			printf("Parsing Total History (len = %d)\n", len);
		}

		if(len != 24 && len != 39)
		{
			fprintf(stderr, "Invalid length for total history\n");
			return;
		}

		HistoryTotalRecord *r = &vd->TotalRecord;

		r->Available = true;
		r->ErrorDatabase = data[5];
		r->Errors[0] = data[6];
		r->Errors[1] = data[7];
		r->Errors[2] = data[8];
		r->Errors[3] = data[9];
		r->TotalYieldUser = read32(data, 10);
		r->TotalYieldSystem = read32(data, 14);
		r->PanelVoltageMaximum = read16(data, 18);
		r->BatteryVoltageMaximum = read16(data, 20);
		r->NumberOfDaysAvailable = data[22];

		if(len == 39)
		{
			r->BatteryVoltageMinimum = read16(data, 23);
		}

		if(r->NumberOfDaysAvailable > 0 && r->NumberOfDaysAvailable <= 30)
		{
			hex_state = H_SEND;
			next_id = REGISTER_DAILY_HISTORY_START;
		}
	}
	else if(get_response_id >= REGISTER_DAILY_HISTORY_START &&
		get_response_id <= REGISTER_DAILY_HISTORY_END)
	{
		if(DEBUG)
		{
			printf("Parsing Daily History\n");
		}

		int day = get_response_id - REGISTER_DAILY_HISTORY_START;
		HistoryDayRecord *r = &vd->DailyRecord[day];

		r->Yield = read32(data, 5);
		r->Consumed = read32(data, 9);
		r->BatteryVoltageMaximum = read16(data, 13);
		r->BatteryVoltageMinimum = read16(data, 15);

		r->ErrorDatabase = data[17];
		r->Errors[0] = data[18];
		r->Errors[1] = data[19];
		r->Errors[2] = data[20];
		r->Errors[3] = data[21];

		r->TimeBulk = read16(data, 22);
		r->TimeAbsorbtion = read16(data, 24);
		r->TimeFloat = read16(data, 26);

		r->PowerMaximum = read32(data, 28);
		r->BatteryCurrentMaximum = read16(data, 32);
		r->PanelVoltageMaximum = read16(data, 34);
		r->DaySequenceNumber = read16(data, 36);

		if(day < vd->TotalRecord.NumberOfDaysAvailable)
		{
			hex_state = H_SEND;
			next_id = get_response_id + 1;
		}
	}
}

static void parse_hex_msg(VictronData *vd, const char *buf, int len)
{
	if(DEBUG)
	{
		printf("Received MSG: %s\n", buf);
	}

	if(len < 3)
	{
		fprintf(stderr, "Hex Message too short\n");
		return;
	}

	if((len & 1) == 0)
	{
		fprintf(stderr, "Number of hex digits must be odd\n");
		return;
	}

	int data_len = len / 2 + 1;
	uint8_t data[128];

	int cmd = hex_to_nibble(buf[0]);
	if(cmd < 0)
	{
		fprintf(stderr, "Invalid hex digit\n");
		return;
	}

	data[0] = cmd;
	for(int i = 0; i < data_len - 1; ++i)
	{
		int hi = hex_to_nibble(buf[1 + 2 * i]);
		int lo = hex_to_nibble(buf[2 + 2 * i]);

		if(hi < 0 || lo < 0)
		{
			fprintf(stderr, "Invalid hex digit\n");
			return;
		}

		int v = (hi << 4) | lo;
		data[i + 1] = v;
	}

	if(DEBUG)
	{
		printf("Parsed MSG:  ");
		for(int i = 0; i < data_len; ++i)
		{
			printf("%02X", data[i]);
		}

		printf("\n");
	}

	int calc_checksum = hex_checksum(data, data_len - 1);
	if(calc_checksum != data[data_len - 1])
	{
		fprintf(stderr, "Invalid checksum. Expected (%02X), Got (%02X)\n", calc_checksum, data[data_len - 1]);
		return;
	}

	if(data[0] == 0x7)
	{
		if(data_len < 4)
		{
			fprintf(stderr, "Get Command Response too short\n");
			return;
		}

		process_hex_get_response(vd, data, data_len);
	}
}

static int write_in_full(int fd, char *buf, int len)
{
	while(len > 0)
	{
		int ret = write(fd, buf, len);
		if(ret < 0)
		{
			return -1;
		}

		buf += ret;
		len -= ret;
	}

	return 0;
}

static void parse_char(int c)
{
	static VictronData data;
	static char label[64];
	static char value[64];
	static char hex_buf[256];
	static int cnt;
	static int i;
	static int state = S_WAIT_FOR_CR;
	static uint8_t bytesum = 0;

	bytesum += c;
	switch(state)
	{
	case S_WAIT_FOR_CR:
		if(c == '\r')
		{
			state = S_WAIT_FOR_LF;
		}
		else if(c == ':')
		{
			state = S_READ_HEX;
			i = 0;
		}
		else
		{
			fprintf(stderr, "Parse error: expected CR (%c)\n", c);
			bytesum = 0;
		}
		break;

	case S_READ_HEX:
		if(c == '\n')
		{
			hex_buf[i] = '\0';
			parse_hex_msg(&data, hex_buf, i);
			state = S_WAIT_FOR_CR;
			bytesum = 0;
		}
		else
		{
			if(i < (int)sizeof(hex_buf) - 1)
			{
				hex_buf[i++] = c;
			}
			else
			{
				fprintf(stderr, "Parse error: hex message too long\n");
				state = S_WAIT_FOR_CR;
				bytesum = 0;
			}
		}
		break;

	case S_WAIT_FOR_LF:
		if(c == '\n')
		{
			state = S_READ_LABEL;
			i = 0;
		}
		else
		{
			fprintf(stderr, "Parse error: expected LF\n");
			state = S_WAIT_FOR_CR;
			bytesum = 0;
		}
		break;

	case S_READ_LABEL:
		if(c == '\t')
		{
			if(i == 0)
			{
				fprintf(stderr, "Parse error: field label must not be empty\n");
				state = S_WAIT_FOR_CR;
				bytesum = 0;
			}
			else
			{
				label[i] = '\0';
				i = 0;
				state = !strcmp(label, "Checksum") ? S_READ_CHECKSUM : S_READ_FIELD;
			}
		}
		else if(isprint(c))
		{
			if(i < (int)sizeof(label) - 1)
			{
				label[i++] = c;
			}
			else
			{
				fprintf(stderr, "Parse error: field label too long\n");
				state = S_WAIT_FOR_CR;
				bytesum = 0;
			}
		}
		else
		{
			fprintf(stderr, "Parse error: invalid character\n");
			state = S_WAIT_FOR_CR;
			bytesum = 0;
		}
		break;

	case S_READ_CHECKSUM:
		if(bytesum == 0)
		{
			data_process(&data);
			data_update(&data);
			if(cnt < 2)
			{
				++cnt;
			}
			else if(data.IsMPPT)
			{
				if(hex_state == H_INIT)
				{
					hex_state = H_SEND;
				}
			}
		}
		else
		{
			fprintf(stderr, "Checksum incorrect\n");
			bytesum = 0;
		}

		state = S_WAIT_FOR_CR;
		break;

	case S_READ_FIELD:
		if(c == '\r')
		{
			state = S_WAIT_FOR_LF;
			if(i == 0)
			{
				fprintf(stderr, "Parse error: field value must not be empty\n");
				bytesum = 0;
			}
			else
			{
				value[i] = '\0';
				parse_line(&data, label, value);
			}
		}
		else if(isprint(c))
		{
			if(i < (int)sizeof(value) - 1)
			{
				value[i++] = c;
			}
			else
			{
				fprintf(stderr, "Parse error: field value too long\n");
				state = S_WAIT_FOR_CR;
				bytesum = 0;
			}
		}
		else
		{
			fprintf(stderr, "Parse error: invalid character\n");
			state = S_WAIT_FOR_CR;
			bytesum = 0;
		}
		break;
	}
}

static void msg_send_get(int fd, int id)
{
	char cmd[64];
	int len = hex_command_get(cmd, id);

	if(DEBUG)
	{
		printf("Sending CMD: %s\n", cmd);
	}

	write_in_full(fd, cmd, len);
	hex_id = id;
	hex_state = H_WAIT_RESPONSE;
}

static void *thread_serial(void *args)
{
	SerialThreadParams *params = args;
	int serial_fd = params->serial_fd;
	int msg_fd = params->msg_fd;

	struct pollfd fds[2] =
	{
		{ .fd = serial_fd, .events = POLLIN },
		{ .fd = msg_fd,    .events = POLLIN }
	};

	uint8_t buf[1024];
	for(;;)
	{
		int ret = 0;
		do
		{
			ret = poll(fds, ARRLEN(fds), -1);
		}
		while(ret < 0 && errno == EINTR);
		if(ret < 0)
		{
			fprintf(stderr, "(poll) Error %d: %s\n", errno, strerror(errno));
			break;
		}

		if(fds[1].revents & POLLIN)
		{
			char byte[1];
			read(msg_fd, byte, 1);
			if(byte[0] == 'Q')
			{
				return NULL;
			}
		}

		if(fds[0].revents & POLLIN)
		{
			int n = read(serial_fd, buf, sizeof(buf));
			if(n < 0 && errno != EINTR)
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

		if(hex_state == H_SEND)
		{
			msg_send_get(serial_fd, next_id);
		}
	}

	gfx_send_quit_event();
	return NULL;
}

int parser_thread_start(int fd)
{
	static SerialThreadParams params;

	if(pipe(msg_pipe) < 0)
	{
		fprintf(stderr, "(pipe) Error %d: %s\n", errno, strerror(errno));
		return 1;
	}

	params.serial_fd = fd;
	params.msg_fd = msg_pipe[0];

	int ret = pthread_create(&thread, NULL, thread_serial, &params);
	if(ret)
	{
		fprintf(stderr, "(pthread_create) Error %d: %s\n", ret, strerror(ret));
		return 1;
	}

	return 0;
}

void parser_thread_quit(void)
{
	char msg[1] = { 'Q' };
	write(msg_pipe[1], msg, 1);

	int ret = pthread_join(thread, NULL);
	if(ret)
	{
		fprintf(stderr, "(pthread_join) Error %d: %s\n", ret, strerror(ret));
	}

	close(msg_pipe[0]);
	close(msg_pipe[1]);
}
