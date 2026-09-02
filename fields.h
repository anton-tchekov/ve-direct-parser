#ifndef __FIELDS_H__
#define __FIELDS_H__

#include <stdint.h>

typedef struct
{
	uint32_t code;
	const char *name;
} MapInt;

const char *map_find(uint32_t code, const MapInt *map);
int map_idx(uint32_t code, const MapInt *map);
const char *map_value(int idx, const MapInt *map);

extern const MapInt map_devices[];
extern const MapInt map_cs[];
extern const MapInt map_err[];
extern const MapInt map_mppt[];
extern const MapInt map_or[];

#endif
