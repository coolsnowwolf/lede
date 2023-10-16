#include <stdio.h>    /*标准输入输出定义*/
#include <errno.h>    /*错误号定义*/
#include <sys/stat.h>
#include <fcntl.h>    /*文件控制定义*/
#include <termios.h>  /*POSIX 终端控制定义*/
#include <stdlib.h>   /*标准函数库定义*/
#include <sys/types.h>
#include <unistd.h>   /*UNIX 标准函数定义*/
#include <string.h> 
#include <pthread.h>
#include <time.h>
#include <sys/select.h>
#include <sys/stat.h>



#define MSG_MAX_SIZE  1024
//将串口驱动定义成宏
int serialfd;     //串口的标识符 
int serial_init(int port,int Baudrate,int nBits,int nParity,int nStop)//串口初始化，Baudrate波特率，nBits数据位，nParity奇偶校验位，nStop停止位
{
    struct termios newttys1,oldttys1;
	/*设置端口*  O_RDONLY|O_NONBLOCK   

	O_NONBLOCK和O_NDELAY所产生的结果都是使I/O变成非阻塞模式(non-blocking)，在读取不到数据或是写入缓冲区已满会马上return，而不会阻塞等待。

	它们的差别在于：在读操作时，如果读不到数据，O_NDELAY会使I/O函数马上返回0，但这又衍生出一个问题，因为读取到文件末尾(EOF)时返回的也是0，这样无法区分是哪种情况。因此，O_NONBLOCK就产生出来，它在读取不到数据时会回传-1，并且设置errno为EAGAIN。

	O_NDELAY是在System V的早期版本中引入的，在编码时，还是推荐POSIX规定的O_NONBLOCK，O_NONBLOCK可以在open和fcntl时设置。

*/
	switch( port)  
	{
		case 1:
			serialfd=open("/dev/ttyUSB1",O_RDWR | O_NOCTTY | O_NDELAY);
			break;
		case 2:
			serialfd=open("/dev/ttyUSB2",O_RDWR | O_NOCTTY | O_NDELAY);
			break;
		case 3:
			serialfd=open("/dev/ttyUSB3",O_RDWR | O_NOCTTY | O_NDELAY);
			break;
		case 4:
			serialfd=open("/dev/ttyUSB4",O_RDWR | O_NOCTTY | O_NDELAY);
			break;
		default:
	    		perror("open_port: Unable to open /dev/ttyUSB0 - No 0\n");
	    		return -1;
			break;
	}
    if(serialfd == -1)
    {
        //不能打开串口
        perror("open_port: Unable to open");
        return(serialfd);
    }
    else
    {
        fcntl(serialfd, F_SETFL, 0);
        /*printf("open.....\n");*/
    }
	/*保存原有串口配置*/
	if(tcgetattr(serialfd,&oldttys1)!=0) 
	{
	  perror("Setupserial 1");
	  return -1;
	}
	bzero(&newttys1,sizeof(newttys1));
	newttys1.c_cflag|=(CLOCAL|CREAD ); /*CREAD 开启串行数据接收，保证程序可以从串口中读取数据  CLOCAL打开本地连接模式，保证程序不占用串口*/
	newttys1.c_cflag &=~CSIZE;//字符长度，设置数据位之前一定要屏掉这个位
	/*数据位选择*/   
	switch(nBits)
	{
		case 7:
			newttys1.c_cflag |=CS7;
		break;
		case 8:
			newttys1.c_cflag |=CS8;
		break;
	}
	/*设置奇偶校验位*/
	switch( nParity )
	{
		case 0: /*无奇偶校验*/
			newttys1.c_cflag &= ~PARENB;
			break;
		case 1:  /*奇校验*/
			newttys1.c_cflag |= PARENB;/*开启奇偶校验*/
			newttys1.c_iflag |= (INPCK | ISTRIP);/*INPCK打开输入奇偶校验；ISTRIP去除字符的第八个比特  */
			newttys1.c_cflag |= PARODD;/*启用奇校验(默认为偶校验)*/
			break;
		case 2:/*偶校验*/
			newttys1.c_cflag |= PARENB; /*开启奇偶校验  */
			newttys1.c_iflag |= ( INPCK | ISTRIP);/*打开输入奇偶校验并去除字符第八个比特*/
			newttys1.c_cflag &= ~PARODD;/*启用偶校验*/
			break;
	}
	/*设置波特率*/
	switch( Baudrate )  
	{
		case 2400:
			cfsetispeed(&newttys1, B2400);
			cfsetospeed(&newttys1, B2400);
			break;
		case 4800:
			cfsetispeed(&newttys1, B4800);
			cfsetospeed(&newttys1, B4800);
			break;
		case 9600:
			cfsetispeed(&newttys1, B9600);
			cfsetospeed(&newttys1, B9600);
			break;
		case 115200:
			cfsetispeed(&newttys1, B115200);
			cfsetospeed(&newttys1, B115200);
			break;
		default:
			cfsetispeed(&newttys1, B9600);
			cfsetospeed(&newttys1, B9600);
			break;
	}
	 /*设置停止位*/
	if( nStop == 1)/*设置停止位；若停止位为1，则清除CSTOPB，若停止位为2，则激活CSTOPB*/
	{
		newttys1.c_cflag &= ~CSTOPB;/*默认为一位停止位； */
	}
	else if( nStop == 2)
	{
		newttys1.c_cflag |= CSTOPB;/*CSTOPB表示送两位停止位*/
	}
 
	/*--------------------其他配置-----------------------*/
	/*设置本地模式为原始模式*/
	/*newttys1.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);*/
	newttys1.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG);
	/*
		 *ICANON：允许规范模式进行输入处理
		 *ECHO：允许输入字符的本地回显
		 *ECHOE：在接收EPASE时执行Backspace,Space,Backspace组合
		 *ISIG：允许信号
	*/
	/*设置输出模式为原始输出*/
	newttys1.c_oflag &= ~OPOST;  //OPOST：若设置则按定义的输出处理，否则所有c_oflag失效
	/*发送字符0X0d的时候，往往接收端得到的字符是0X0a，原因是因为在串口设置中c_iflag和c_oflag中存在从NL-CR和CR-NL的映射，即串口能把回车和换行当成同一个字符，可以进行如下设置屏蔽之*/
	newttys1.c_oflag &= ~(ONLCR | OCRNL);
	newttys1.c_iflag &= ~(ICRNL | INLCR);    
	newttys1.c_cflag &= ~CRTSCTS;// 不使用数据流控制
	newttys1.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); /*c_cc数组的VSTART和VSTOP元素被设定成DC1和DC3，代表ASCII标准的XON和XOFF字符，如果在传输这两个字符的时候就传不过去，需要把软件流控制屏蔽*/
 
	/*设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
	newttys1.c_cc[VTIME] = 10;/*非规范模式读取时的超时时间；1*/ 
	newttys1.c_cc[VMIN]  = 0; /*非规范模式读取时的最小字符数 1*/
	tcflush(serialfd ,TCIFLUSH);/*tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */
 
	 /*激活配置使其生效*/
	if((tcsetattr( serialfd, TCSANOW,&newttys1))!=0)
	{
		perror("com set error");
		return -1;
	}
 
	return 0;
}

/**
*串口接收数据
*要求启动后，在pc端发送ascii文件
*/
int uart_recv(char *data, int datalen)
{
    int len=0, ret = 0;
    fd_set fs_read;
    struct timeval tv_timeout;
 
    FD_ZERO(&fs_read);
    FD_SET(serialfd, &fs_read);
    tv_timeout.tv_sec  =2;     // (10*20/115200+5);
    tv_timeout.tv_usec = 0;
 
    while(FD_ISSET(serialfd,&fs_read))
    {
 
        FD_ZERO(&fs_read);
        FD_SET(serialfd,&fs_read);
        ret = select(serialfd+1, &fs_read, NULL, NULL, &tv_timeout);
        printf("ret = %d\n", ret);
        //如果返回0，代表在描述符状态改变前已超过timeout时间,错误返回-1
 
        if(FD_ISSET(serialfd, &fs_read)) {
            len = read(serialfd, data, datalen);
            printf("len = %d\n", len);
            if(-1==len) {
                return -1;
            }
        } else {
            perror("select");
        }
    }
    return 0;
}

/** 
*串口发送数据 
*@fd:串口描述符 
*@data:待发送数据 
*@datalen:数据长度
*/
int uart_send(char *data, int datalen)
{
    int len = 0;
    len = write(serialfd, data, datalen);//实际写入的长度
    if(len == datalen) {
        return len;
    } else {
        tcflush(serialfd, TCOFLUSH);//TCOFLUSH刷新写入的数据但不传送
        return -1;
    }
 
    return 0;
}


int main(int argc, char *argv[])
{
	int port= 0;
	sscanf(argv[1], "%d", &port);
	char *message= argv[2];
	char *rn= "\r\n";
	char *send= strcat(message, rn);
	//初始化串口
    int serialret = serial_init(port,115200,8,0,1);
	if(serialret == -1)
	{
	     printf("serial init failed ...\r\n");
	     return -1;
	}
	write(serialfd,send,strlen(send));
 	fd_set set;
  	struct timeval timeout;
  	int rv;
	FD_ZERO(&set); /* clear the set */
  	FD_SET(serialfd, &set); /* add our file descriptor to the set */
  	timeout.tv_sec = 0;
  	timeout.tv_usec = 10000;
	rv = select(serialfd + 1, &set, NULL, NULL, &timeout);
  	if(rv == -1)
	{
 		perror("select\r\n"); /* an error accured */
	}
    else if(rv == 0)
	{
		printf("timeout\r\n"); /* a timeout occured */
	}
    else
	{
		char buf[MSG_MAX_SIZE]={0};
	    int len = 0;
		memset(buf,0,sizeof(buf));
	    len=read(serialfd,buf,MSG_MAX_SIZE);



	    printf("%s\r\n",buf);
	}
	close(serialfd);
	pthread_exit(0);
	return 0;
}