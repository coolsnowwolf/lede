//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
#include "QMIThread.h"
#include "query_pcie_mode.h"
int speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300,//
                    B38400, B19200, B9600, B4800, B2400, B1200, B300,
                  };
int name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300,
                  38400, 19200, 9600, 4800, 2400, 1200, 300,
                 };

int get_pcie_mode_debug = 0;

//2021-02-24 willa.liu@fibocom.com changed begin for support eipd SN-20210129001
int get_private_gateway_debug = 1;
//2021-02-24 willa.liu@fibocom.com changed end for support eipd SN-20210129001

//static int xset1(int fd, struct termios *tio, const char *device)
int xset1(int fd, struct termios *tio, const char *device)
{
    int ret = tcsetattr(fd, TCSAFLUSH, tio);

    if (ret) {
        printf("can't tcsetattr for %s", device);
    }
    return ret;
}

// set raw tty mode
//static void xget1(int fd, struct termios *t, struct termios *oldt)
void xget1(int fd, struct termios *t, struct termios *oldt)
{
    tcgetattr(fd, oldt);
    *t = *oldt;
    cfmakeraw(t);
//	t->c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
//	t->c_iflag &= ~(BRKINT|IXON|ICRNL);
//	t->c_oflag &= ~(ONLCR);
//	t->c_cc[VMIN]  = 1;
//	t->c_cc[VTIME] = 0;
}

//static int get_pcie_mode()
int get_pcie_mode()
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
{
        int i;
        int fd;
        int ret;
        char buffer[409600] = {0};
        int rate;
        char* sendbuffer;
        int totallen = 0;
        fd_set readfds;
        struct timeval timeout;
        struct termios tio0, tiosfd, tio;

        int timeoutVal = 5;
    
        char *dev = "/dev/ttyUSB1"; //The port under Linux is operated by opening the device file
        rate = 115200;
        sendbuffer = "at+gtpcie=3";
        printf ( "dev: %s\nrate:%d\nsendbuffer:%s\n", dev,rate,sendbuffer);
        
        fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd < 0)
            goto ERR;
        fcntl(fd, F_SETFL, O_RDWR);
    
        // put device to "raw mode"
        xget1(fd, &tio, &tiosfd);
        // set device speed
        for ( i = 0;i < sizeof ( speed_arr ) / sizeof ( int );i++ )
        {
            if ( rate == name_arr[i] ) //Judge if the pass is equal to the pass
            {
                break;
            }
        }
        
        if(i >= sizeof ( speed_arr ) / sizeof ( int ))
        {
            printf("bound rate set failed\n");
            goto ERR;
        }
    
        cfsetspeed(&tio, speed_arr[i]);
        if (xset1(fd, &tio, dev))
            goto ERR;
    
        tcflush ( fd, TCIOFLUSH );
    
        sprintf(buffer, "%s\r", sendbuffer);
        ret = write ( fd, buffer, strlen(buffer) );
        if(ret < 0)
        {
            printf ( "write failed\n" );
            goto ERR;
        }
        if(get_pcie_mode_debug)printf("write %d\n", ret);
    
        FD_ZERO ( &readfds );
        FD_SET ( fd, &readfds );
        
        while(1)
        {
            timeout.tv_sec = timeoutVal;
            timeout.tv_usec = 0;
    
            ret = select ( fd+1, &readfds, ( fd_set * ) 0, ( fd_set * ) 0, &timeout );
            if(ret > 0)
            {
                ret = read ( fd, buffer+totallen, sizeof(buffer)-totallen-1 );
                if(ret < 0)
                {
                    printf ( "read failed\n" );
                    goto ERR;
                }           
                
                if(ret == 0)
                {
                    goto ERR;
                }           
                        
                totallen += ret; 
                buffer[totallen] = '\0';
                if(get_pcie_mode_debug)printf("read %d(%s)\n", ret, &buffer[totallen-ret]);            
                if(totallen == sizeof(buffer)-1)
                    break;
                    
                if(strstr(buffer,"\nOK") || strstr(buffer,"\nERROR")
                    || strstr(buffer,"\n+CME ERROR:") || strstr(buffer,"\n+CMS ERROR:"))
                {
                    if(get_pcie_mode_debug)printf("match OK/ERROR");
                    if(get_pcie_mode_debug)printf("%s", buffer);
                    break;
                }
    
            }
            else
            {
                printf ( "select timeout\n" );
                goto ERR;
            }
    
        }
    
        tcsetattr(fd, TCSAFLUSH, &tiosfd);
        //printf("buffer:\n %s\n", buffer);
        printf("%s\n", buffer);
        if(strstr(buffer,"\nERROR") || strstr(buffer,"\n+CME ERROR:") || strstr(buffer,"\n+CMS ERROR:"))
            goto ERR;
    
        close ( fd );
        return 0;    
    ERR:
        if(fd > 0)
            close(fd);
        return -1;    
}

