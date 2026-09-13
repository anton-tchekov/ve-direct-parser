#include "hex.h"
#include "util.h"
#include <ctype.h>

int hex_checksum(const uint8_t *data, int len)
{
	uint8_t sum = 0;
	for(int i = 0; i < len; ++i)
	{
		sum += data[i];
	}

	sum = 0x55 - sum;
	return sum;
}

int hex_convert(char *out, const uint8_t *data, int len)
{
	int p = 0;
	out[p++] = ':';
	out[p++] = nibble_to_ascii(data[0] & 0x0F); // Command nibble
	for(int i = 1; i < len; ++i)
	{
		int v = data[i];
		out[p++] = nibble_to_ascii((v >> 4) & 0x0F);
		out[p++] = nibble_to_ascii(v & 0x0F);
	}

	int checksum = hex_checksum(data, len);
	out[p++] = nibble_to_ascii((checksum >> 4) & 0x0F);
	out[p++] = nibble_to_ascii(checksum & 0x0F);
	out[p++] = '\n';
	out[p] = '\0';
	return p;
}

int hex_command_get(char *out, int id)
{
	uint8_t data[4];
	data[0] = 7;
	data[1] = id & 0xFF;
	data[2] = id >> 8;
	data[3] = 0;
	return hex_convert(out, data, 4);
}
