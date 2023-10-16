#ifndef _UART_H_
#define _UART_H_

#include <stdio.h>
#include <assert.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#define MAX_PORTS	4

/*this array hold information about each port we have opened */
struct PortInfo {
    char name[16];
    int  port_fd;
};

int serial_init(int port, int spd, int databits, int parity, \
						int stopbits, int RTSCTS, int need_line_input);
int serial_write(int fd, void *src, int len);
int serial_read(int fd, char *buf, int len);
int serial_recv(int fd,char *rbuf,int rbuf_len, int timeout);

//串口常用初始化接口
#define serial_com_init(port, spd, databits, parity, stopbits)\
		serial_init(port, spd, databits, parity, stopbits, 0, 0)
//串口默认初始化接口
#define serial_def_init(port, spd) serial_init(port, spd, 8, 'n', 1, 0, 1)


#endif


