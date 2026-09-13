#include "serial.h"
#include <termios.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"

int serial_open(const char *port, int baud)
{
	int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
	if(fd < 0)
	{
		fprintf(stderr, "(open) Error %d: %s\n", errno, strerror(errno));
		return -1;
	}

	struct termios tty;
	if(tcgetattr(fd, &tty))
	{
		fprintf(stderr, "(tcgetattr) Error %d: %s\n", errno, strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, baud);
	cfsetispeed(&tty, baud);
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN]  = 1;
	tty.c_cc[VTIME] = 0;
	tty.c_iflag &= ~(IGNBRK | ICRNL | IXON | IXOFF | IXANY);
	tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
	tty.c_cflag |= (CLOCAL | CREAD | CS8);
	if(tcsetattr(fd, TCSANOW, &tty))
	{
		fprintf(stderr, "(tcsetattr) Error %d: %s\n", errno, strerror(errno));
		return -1;
	}

	return fd;
}

void serial_list(void)
{
	DIR *dp = opendir("/dev/");
	if(!dp)
	{
		fprintf(stderr, "(opendir) Error %d: %s\n", errno, strerror(errno));
		return;
	}

	int cnt = 0;
	struct dirent *ep;
	while((ep = readdir(dp)))
	{
		const char *name = ep->d_name;
		if(starts_with(name, "ttyUSB") || starts_with(name, "ttyACM"))
		{
			++cnt;
			if(cnt == 1)
			{
				printf("\nAvailable ports:\n\n");
			}

			printf("/dev/%s\n", name);
		}
	}

	if(!cnt)
	{
		puts("No USB serial devices found");
	}

	closedir(dp);
}
