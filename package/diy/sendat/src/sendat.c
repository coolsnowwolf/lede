#include "uart.h"
#include <stdarg.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

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

#include <iostream>
#include <fstream>
using namespace std;
#include <stdio.h>
#include <io.h>


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


/*
** File: uart.c
**
** Description:
** Provides an RS-232 interface that is very similar to the CVI provided
** interface library
*/

#include "uart.h"

/*this array hold information about each port we have opened */
struct PortInfo ports[13] = 
{
	{"/dev/ttyUSB0", 0},
	{"/dev/ttyUSB1", 0},
	{"/dev/ttyUSB2", 0}, 
	{"/dev/ttyUSB3", 0},
	{"/dev/ttyUSB4", 0},
	{"/dev/ttyUSB5", 0},
	{"/dev/ttyUSB6", 0},
	{"/dev/ttyUSB7", 0},
	{"/dev/ttyUSB8", 0},
	{"/dev/ttyUSB9", 0},
	{"/dev/ttyUSB10", 0},
	{"/dev/ttyUSB11", 0},
	{"/dev/ttyUSB12", 0},
};

int spd_arr[] = \
{B2000000, B1500000, B576000, B500000, B460800, B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400};

int name_arr[] = \
{ 2000000, 1500000,  576000,  500000,  460800,  230400,  115200,  57600,  38400,  19200,  9600,  4800,  2400 };
/////////////////////////////////////////////////////////////////////////////////////////
/**
*@brief  设置串口通信速率
*@param  fd    类型 int 打开串口的文件句柄
*@param  speed 类型 int 串口速度
*@return  void
*/
int set_speed(int fd, int speed)
{
	int   i; 
	int   status; 
	struct termios Opt;
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(spd_arr) / sizeof(int);  i++) { 
		if(speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, spd_arr[i]);  
			cfsetospeed(&Opt, spd_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0) {        
				printf("tcsetattr failed");  
				return 1;     
			}
			tcflush(fd,TCIOFLUSH);   
		}
	}
	// printf("set_speed\n");
	return 0;
}

/**
*@brief  设置串口数据位，停止位和效验位
*@param  fd       类型  int 打开的串口文件句柄
*@param  databits 类型  int 数据位   取值为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity   类型  int 效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd, int databits, int parity, int stopbits, int RTSCTS)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*设置数据位数*/
	{   
	case 7:		
		options.c_cflag |= CS7; 
		break;
	case 8:     
		options.c_cflag |= CS8;
		break;   
	default:    
		fprintf(stderr,"Unsupported data size\n"); 
        return -1;  
	}

    options.c_iflag |= INPCK;
    cfmakeraw(&options);
    //options.c_lflag |= (ICANON | ECHO | ECHOE);
    //options.c_lflag &= ~(ICANON | ECHO | ECHOE);
    //options.c_iflag &= ~(IXON | IXOFF);
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;     /* Enable parity */    
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
			break;
		case 'S': 
		case 's':  /*as no parity*/   
		    options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
	        break;  
		default:   
			fprintf(stderr,"Unsupported parity\n");    
			return -1;  
	}  

	/* 设置停止位*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
		   break;
		default:    
			 fprintf(stderr,"Unsupported stop bits\n");  
			 return -1; 
	} 

	/* Set rts/cts */ 
	if (RTSCTS)
	{
	    printf("Set rts/cts");
		options.c_cflag |= CRTSCTS;
	}

	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		printf("SetupSerial failed");   
		return -1;  
	} 
	// printf("set_Parity\n");
	return 0;  
}

//接收消息结尾加换行符
int serial_set_line_input(int fd)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}

    options.c_lflag |= ICANON;

    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
    	perror("SetupSerial 3");   
    	return -1;  
    } 
    return 0;  
}

/**
*@brief  
*@param  port 串口端口号
*@param  spd  串口速度
*@param  databits,parity,stopbits,RTSCTS,分别为数据位,校验位,停止位,rtscts位
*@param  need_line_input接收数据结尾是否加换行符?
*/
int serial_init(int port, int spd, int databits, int parity, 
				int stopbits, int RTSCTS, int need_line_input)
{
    int fd;

	if(port < 13)
	{
   		// printf("open port:%d\n", port);
	}
	else {
		printf("error: port:%d > MAX_PORTS\n", port);
		return -1;
	}
												 //	   	                	
    fd = open(ports[port].name, O_RDWR|O_NOCTTY);//O_NONBLOCK 非阻塞, O_WRONLY 只读写, O_RDONLY 只读, O_RDWR 读写,O_NOCTTY 阻塞

	if (-1 == fd) { 
        printf("init %s failed\n", ports[port].name);
        return -1;
    }

    set_speed(fd, spd);
	
    set_Parity(fd, databits, parity, stopbits, RTSCTS);
	
    if (need_line_input) {
		serial_set_line_input(fd);
    }

	ports[port].port_fd = fd;
	
    return fd;
}

/**
*@brief  
*@param  fd   串口端口号文件描述符
*@param  src  需要通过串口发送的数据
*@param  len  需要发送的数据长度
*@param  成功返回0, 否则返回-1
*/
int serial_write(int fd, void *src, int len)
{
    int ret = write(fd, src, len);
    if (len != ret) {
		perror("oh, write serial failed!");
		return -1;
    }
    return 0;
}

/**
*@brief  
*@param  fd   串口端口号文件描述符
*@param  src  串口接收数据的指针
*@param  len  需要接收的数据长度
*@param  成功返回0, 否则返回-1
*/
int serial_read(int fd, char *buf, int len)
{
    int ret = read(fd, buf, len-1);
    if (-1 == ret) {
		perror("oh, read serial failed!");
		return -1;
    }
    buf[ret] = '\0';
    return ret;
}

int serial_recv(int fd,char *rbuf,int rbuf_len, int timeout)
{      
    int retval;
    fd_set  rset;
    struct timeval time_out;
    if(!rbuf || rbuf_len <= 0)
    {  
        // printf("serial_recv Invalid parameter.\n");
        return -1;
    }  
    if(timeout) //指定延时等待
    {    
        time_out.tv_sec = (time_t)(timeout / 1000);
        time_out.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(fd,&rset);

        retval = select(fd,&rset,NULL,NULL,&time_out);
        if(retval < 0)
        {
            // printf("%s,Select failed:%s\n",strerror(errno));
            return -2;
        }
        else if(0 == retval)
        {
            // printf("Time Out.\n");
            return 0;
        }

    }
    // usleep(1000);
    retval = read(fd, rbuf, rbuf_len);
    if( retval <= 0)
    {
        // printf("Read failed:%s\n",strerror(errno));
        return -3;
    }
    return retval;

} 


int FileExist(const char* filename)
{
    if (filename && access(filename, F_OK) == 0) {
        return 1;
    }
    return 0;
}


int main(int argc, char **argv)
{
	int port= 0;
	sscanf(argv[1], "%d", &port);

	if(FileExist(ports[port].name)==0)
	{
		printf("AT ERROR absent.\n");
   		return 0;
	}

	char *message= argv[2];
	char *nty= "\r";
    char *send= strcat(message,nty);
	char buff[512];
	//打开串口0，波特率为1500000
	int fd = serial_def_init(port, 1500000);
	if(fd < 0) return 0;

	serial_write(fd,send, strlen(send));

 	int retval;
	fd_set  rset;
    struct timeval time_out;
	time_out.tv_sec = (time_t)(5 / 1000);
	time_out.tv_usec = 0;
	FD_ZERO(&rset);
	FD_SET(fd,&rset);
	while(1) 
	{
		int read = serial_read(fd, buff, sizeof(buff));
		printf("%s", buff);
		retval = select(fd+1,&rset,NULL,NULL,&time_out);
        if(retval < 0)
        {
            return -2;
        }
        else if(0 == retval)
        {
            return 0;
        }
	}


	close(fd);//关闭串口
	return 0;
}