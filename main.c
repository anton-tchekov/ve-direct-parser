#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

#include <SDL2/SDL.h>

#include "fields.h"
#include "gfx.h"
#include "util.h"
#include "serial.h"
#include "parser.h"
#include "display.h"

#define VE_DIRECT_BAUD B19200

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "Usage: ./ve-direct-gui `port`\n");
		serial_list();
		return 1;
	}

	const char *port = argv[1];
	int fd = serial_open(port, VE_DIRECT_BAUD);
	if(fd < 0)
	{
		fprintf(stderr, "Failed to open port: `%s`\n", port);
		return 1;
	}

	if(gfx_init(480, 320, "Victron MPPT VE-Direct"))
	{
		return 1;
	}

	parser_thread_start(fd);

	int running = 1;
	while(running)
	{
		set_color(0, 0, 0);
		gfx_clear();
		display_data();
		gfx_update();

		SDL_Event e;
		if(!SDL_WaitEvent(&e))
		{
			running = 0;
		}

		switch(e.type)
		{
		case SDL_QUIT:
			running = 0;
			break;

		case SDL_KEYDOWN:
			switch(e.key.keysym.scancode)
			{
			case SDL_SCANCODE_ESCAPE:
				running = 0;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	gfx_destroy();
	return 0;
}
