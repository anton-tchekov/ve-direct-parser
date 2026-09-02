#ifndef __SERIAL_H__
#define __SERIAL_H__

int serial_open(const char *port, int baud);
void serial_list(void);

#endif
