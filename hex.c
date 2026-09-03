#include "hex.h"

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

static int nibble_to_ascii(int v)
{
	return "0123456789ABCDEF"[v];
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
