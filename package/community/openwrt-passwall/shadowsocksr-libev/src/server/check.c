#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include<arpa/inet.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>

//#define __DEBUG__  
#ifdef __DEBUG__  
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#else  
#define DEBUG(format,...)  
#endif  

static sigjmp_buf jmpbuf;
static void alarm_func()
{
    siglongjmp(jmpbuf, 1);
}

static struct hostent *timeGethostbyname(const char *domain, int timeout)
{
    struct hostent *ipHostent = NULL;
    signal(SIGALRM, alarm_func);
    if(sigsetjmp(jmpbuf, 1) != 0)
    {
        alarm(0);//timout
        signal(SIGALRM, SIG_IGN);
        return NULL;
    }
    alarm(timeout);//setting alarm
    ipHostent = gethostbyname(domain);
    signal(SIGALRM, SIG_IGN);
    return ipHostent;
}


#define MY_HTTP_DEFAULT_PORT 80
#define BUFFER_SIZE 1024
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s"
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"

static int http_parse_url(const char *url,char *host,char *file,int *port)
{
    char *ptr1,*ptr2;
    int len = 0;
    if(!url || !host || !file || !port){
        return 1;
    }

    ptr1 = (char *)url;

    if(!strncmp(ptr1,"http://",strlen("http://"))){
        ptr1 += strlen("http://");
    }else{
        return 1;
    }

    ptr2 = strchr(ptr1,'/');
    if(ptr2){
        len = strlen(ptr1) - strlen(ptr2);
        memcpy(host,ptr1,len);
        host[len] = '\0';
        if(*(ptr2 + 1)){
            memcpy(file,ptr2 + 1,strlen(ptr2) - 1 );
            file[strlen(ptr2) - 1] = '\0';
        }
    }else{
        memcpy(host,ptr1,strlen(ptr1));
        host[strlen(ptr1)] = '\0';
    }
    //get host and ip
    ptr1 = strchr(host,':');
    if(ptr1){
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    }else{
        *port = MY_HTTP_DEFAULT_PORT;
    }

    return 0;
}


static int http_tcpclient_recv(int socket,char *lpbuff){
    int recvnum = 0;

    recvnum = recv(socket, lpbuff,BUFFER_SIZE*4,0);

    return recvnum;
}

static int http_tcpclient_send(int socket,char *buff,int size){
    int sent=0,tmpres=0;

    while(sent < size){
        tmpres = send(socket,buff+sent,size-sent,0);
        if(tmpres == -1){
            return 1;
        }
        sent += tmpres;
    }
    return sent;
}





int  http_get(const char *url,int socket_fd)
{
    char lpbuf[BUFFER_SIZE*4] = {'\0'};

    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;


    if(!url){
       DEBUG("      failed!\n");
        return 1;
    }

    if(http_parse_url(url,host_addr,file,&port)){
       DEBUG("http_parse_url failed!\n");
        return 1;
    }
    DEBUG("url:  %s\thost_addr : %s\tfile:%s\t,%d\n",url,host_addr,file,port);


    if(socket_fd < 0){
       DEBUG("http_tcpclient_create failed\n");
        return 1;
    }

    sprintf(lpbuf,HTTP_GET,file,host_addr,port);

    if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf)) < 0){
       DEBUG("http_tcpclient_send failed..\n");
        return 1;
    }
	  DEBUG("request:\n%s\n",lpbuf);

    if(http_tcpclient_recv(socket_fd,lpbuf) <= 0){
       DEBUG("http_tcpclient_recv failed\n");
        close(socket_fd);
        return 1;
    }
    DEBUG("rec:\n%s\n",lpbuf);
    close(socket_fd);

    //return http_parse_result(lpbuf);
return 0;
}



int main(int argc, char *argv[])
{
        int fd,http_flag=0,http_ret=1;
        struct sockaddr_in addr;
        struct hostent *host;
        struct timeval timeo = {3, 0};
        socklen_t len = sizeof(timeo);
        
        char http_url[100]="http://";

   

        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (argc >= 4)
                 timeo.tv_sec = atoi(argv[3]);
        if (argc>=5)
         http_flag=1;
         
        if((host=timeGethostbyname(argv[1],timeo.tv_sec)) == NULL) {
        DEBUG("gethostbyname err\n");
        return 1;
    }
  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {

      DEBUG("setsockopt send err\n");
       return 1;
    }
            
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {

      DEBUG("setsockopt recv err\n");
       return 1;
    }
  
         addr.sin_family = AF_INET;
         addr.sin_addr = *((struct in_addr *)host->h_addr);
         //addr.sin_addr.s_addr = inet_addr(argv[1]);
         addr.sin_port = htons(atoi(argv[2]));
if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {  
      if (errno == EINPROGRESS) 
       {
         DEBUG("timeout err\n");
          return 1;
        }  
       DEBUG("connect err\n");
        return 1;
    }
if(http_flag==0)
{
 close(fd);
 return 0;
}
strcat(http_url,argv[1]);
http_ret=http_get(http_url,fd);
if(http_ret==1)
{
DEBUG("recv err");
 return 1;
}
else
{
DEBUG("recv ok");

 return 0;
}

}