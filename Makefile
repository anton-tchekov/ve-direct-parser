all:
	gcc \
		main.c \
		serial.c \
		gfx.c \
		fields.c \
		util.c \
		parser.c \
		data.c \
		display.c \
		hex.c \
		-o ve-direct-gui -Wall -Wextra -lSDL2 -lSDL2_ttf -g
