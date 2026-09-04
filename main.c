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

	bool censor = false;
	int running = 1;
	int x = 0, y = 0;
	while(running)
	{
		set_color(0, 0, 0);
		gfx_clear();
		display_data(censor);
		gfx_update();

		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
			case SDL_QUIT:
				running = 0;
				break;

			case SDL_MOUSEBUTTONDOWN:
				if(e.button.button == SDL_BUTTON_LEFT)
				{
					x = e.button.x;
					y = e.button.y;
				}
				break;

			case SDL_MOUSEMOTION:
				if(e.button.button == SDL_BUTTON_LEFT)
				{
					gfx_origin_move(e.button.x - x, e.button.y - y);
					x = e.button.x;
					y = e.button.y;
				}
				break;

			case SDL_KEYDOWN:
				switch(e.key.keysym.scancode)
				{
				case SDL_SCANCODE_ESCAPE:
					running = 0;
					break;

				case SDL_SCANCODE_C:
					censor = !censor;
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}
	}

	parser_thread_quit();
	gfx_destroy();
	return 0;
}
