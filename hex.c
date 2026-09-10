#include "hex.h"
#include "util.h"
#include <ctype.h>

static int hex_checksum(const uint8_t *data, int len)
{
	uint8_t sum = 0;
	for(int i = 0; i < len; ++i)
	{
		sum += data[i];
	}

	sum = 0x55 - sum;
	return sum;
}

static void out_byte(char *out, int *p, int v)
{
	out[*p++] = nibble_to_ascii((v >> 4) & 0x0F);
	out[*p++] = nibble_to_ascii(v & 0x0F);
}

int hex_convert(char *out, const uint8_t *data, int len)
{
	int p = 0;
	out[p++] = ':';
	out[p++] = nibble_to_ascii(data[0] & 0x0F); // Command nibble
	for(int i = 1; i < len; ++i)
	{
		out_byte(out, &p, data[i]);
	}

	out_byte(out, &p, hex_checksum(data, len));
	out[p] = '\0';
	return p;
}

static void command_get(char *out, int id)
{
	uint8_t data[4];
	data[0] = 7;
	data[1] = id & 0xFF;
	data[2] = id >> 8;
	data[3] = 0;
	hex_convert(out, data, 4);
}

static int parse_get_response(const char *p)
{
	return 0;
}

static int parse_hex(const char *p)
{
	if(p[0] != ':')
	{
		return 1;
	}

	if(!isxdigit(p[1]))
	{
		return -1;
	}

	if(parse_get_response(p))
	{

	}

	return 0;
}
