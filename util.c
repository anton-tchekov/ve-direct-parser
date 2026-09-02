#include "util.h"

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
