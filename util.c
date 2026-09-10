#include "util.h"
#include <string.h>

int starts_with(const char *s, const char *prefix)
{
	while(*prefix)
	{
		if(*s != *prefix)
		{
			return 0;
		}

		++prefix;
		++s;
	}

	return !*prefix;
}

char *copy_str(char *out, int len, const char *v)
{
	strncpy(out, v, len - 1);
	out[len - 1] = '\0';
	return out;
}

int nibble_to_ascii(int v)
{
	return "0123456789ABCDEF"[v];
}
