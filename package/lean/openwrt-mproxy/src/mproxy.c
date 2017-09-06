#include <arpa/inet.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <netinet/in.h> 


#include <string.h>

#define BUF_SIZE 8192

#define READ  0
#define WRITE 1

#define DEFAULT_LOCAL_PORT    8080  
#define DEFAULT_REMOTE_PORT   8081 
#define DEFAULT_R_P   1024		/* 定义-r 时默认的转发CONNECT流的端口 */
#define DEFAULT_M_S   Lbxx: 	/* 定义-m 时默认的ML关键字符串(经判断无用，可舍弃) */
#define SERVER_SOCKET_ERROR -1
#define SERVER_SETSOCKOPT_ERROR -2
#define SERVER_BIND_ERROR -3
#define SERVER_LISTEN_ERROR -4
#define CLIENT_SOCKET_ERROR -5
#define CLIENT_RESOLVE_ERROR -6
#define CLIENT_CONNECT_ERROR -7
#define CREATE_PIPE_ERROR -8
#define BROKEN_PIPE_ERROR -9
#define HEADER_BUFFER_FULL -10
#define BAD_HTTP_PROTOCOL -11

#define MAX_HEADER_SIZE 8192


#if defined(OS_ANDROID)
#include <android/log.h>
 
#define LOG(fmt...) __android_log_print(ANDROID_LOG_DEBUG,__FILE__,##fmt)
 
#else
#define LOG(fmt...)  do { fprintf(stderr,"%s %s ",__DATE__,__TIME__); fprintf(stderr, ##fmt); } while(0)
#endif


char remote_host[128]; 
int remote_port; 
int local_port;
char r_h[128]; 
int r_p; 
char m_s[128];

int server_sock; 
int client_sock;
int remote_sock;




char * header_buffer ;


enum 
{
    FLG_NONE = 0,       /* 正常数据流不进行编解码 */
    R_C_DEC = 1,        /* 读取客户端数据仅进行解码 */
    W_S_ENC = 2         /* 发送到服务端进行编码 */
};

static int io_flag; /* 网络io的一些标志位 */
static int r_flag; /* 是否启用-r 匹配免流host直接转发CONNECT流量模式 */
static int m_flag; /* 是否启用-m 关键字头匹配免流模式 */
static int m_pid; /* 保存主进程id */



void server_loop();
void stop_server();
void handle_client(int client_sock, struct sockaddr_in client_addr);
void forward_header(int destination_sock);
void forward_data(int source_sock, int destination_sock);
void rewrite_header();
int send_data(int socket,char * buffer,int len );
int receive_data(int socket, char * buffer, int len);
void hand_mproxy_info_req(int sock,char * header_buffer) ;
void get_info(char * output);
const char * get_work_mode() ;
int create_connection() ;
int _main(int argc, char *argv[]) ;




ssize_t readLine(int fd, void *buffer, size_t n)
{
    ssize_t numRead;                    
    size_t totRead;                     
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;                       

    totRead = 0;
    for (;;) {
        numRead = receive_data(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR)         
                continue;
            else
                return -1;              /* 未知错误 */

        } else if (numRead == 0) {      /* EOF */
            if (totRead == 0)           /* No bytes read; return 0 */
                return 0;
            else                        /* Some bytes read; add '\0' */
                break;

        } else {     
                                      
            if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }

            if (ch == '\n')
                break;
        }
    }

    *buf = '\0';
    return totRead;
}

int read_header(int fd, void * buffer)
{
    // bzero(header_buffer,sizeof(MAX_HEADER_SIZE));
    memset(header_buffer,0,MAX_HEADER_SIZE);
    char line_buffer[2048];
    char * base_ptr = header_buffer;

    for(;;)
    {
        memset(line_buffer,0,2048);

        int total_read = readLine(fd,line_buffer,2048);
        if(total_read <= 0)
        {
            return CLIENT_SOCKET_ERROR;
        }
        //防止header缓冲区蛮越界
        if(base_ptr + total_read - header_buffer <= MAX_HEADER_SIZE)
        {
           strncpy(base_ptr,line_buffer,total_read); 
           base_ptr += total_read;
        } else 
        {
            return HEADER_BUFFER_FULL;
        }

        //读到了空行，http头结束
        if(strcmp(line_buffer,"\r\n") == 0 || strcmp(line_buffer,"\n") == 0)
        {
            break;
        }

    }
    return 0;

}

void extract_server_path(const char * header,char * output)
{
    char * p = strstr(header,"GET /");
    if(p) {
        char * p1 = strchr(p+4,' ');
        strncpy(output,p+4,(int)(p1  - p - 4) );
    }
    
}

int extract_host(const char * header)
{
	if(!m_flag)
	{
		if(!r_flag)
		{
			char * _p = strstr(header,"CONNECT");  /* 在 CONNECT 方法中解析 隧道主机名称及端口号 */
    		if(_p)
    		{
    		    char * _p1 = strchr(_p,' ');

    		    char * _p2 = strchr(_p1 + 1,':');
    		    char * _p3 = strchr(_p1 + 1,' ');

    		    if(_p2)
    		    {
    		        char s_port[10];
    		        bzero(s_port,10);

    		        strncpy(remote_host,_p1+1,(int)(_p2  - _p1) - 1);
    		        strncpy(s_port,_p2+1,(int) (_p3 - _p2) -1);
    		        remote_port = atoi(s_port);

    		    } else 
    		    {
    		        strncpy(remote_host,_p1+1,(int)(_p3  - _p1) -1);
    		        remote_port = 80;
    		    }
    		    
    		    
    		    return 0;
    		}


    		char * p = strstr(header,"Host:");
    		if(!p) 
    		{
    		    return BAD_HTTP_PROTOCOL;
    		}
    		char * p1 = strchr(p,'\n');
    		if(!p1) 
    		{
    		    return BAD_HTTP_PROTOCOL; 
    		}

    		char * p2 = strchr(p + 5,':'); /* 5是指'Host:'的长度 */

    		if(p2 && p2 < p1) 
    		{
    		    
    		    int p_len = (int)(p1 - p2 -1);
    		    char s_port[p_len];
    		    strncpy(s_port,p2+1,p_len);
    		    s_port[p_len] = '\0';
    		    remote_port = atoi(s_port);

    		    int h_len = (int)(p2 - p -5 -1 );
    		    strncpy(remote_host,p + 5 + 1  ,h_len); //Host:
    		    //assert h_len < 128;
    		    remote_host[h_len] = '\0';
    		} else 
    		{   
    		    int h_len = (int)(p1 - p - 5 -1 -1); 
    		    strncpy(remote_host,p + 5 + 1,h_len);
    		    //assert h_len < 128;
    		    remote_host[h_len] = '\0';
    		    remote_port = 80;
    		}
		} else
		{
			strncpy(remote_host, r_h, strlen(r_h));
            remote_port = r_p;
		}
	} else
	{
    	char * __p = strstr(header,"CONNECT");  /* 在 CONNECT 方法中解析 隧道主机名称及端口号 */
    	char * _p = strstr(header,m_s);
    	//printf("Show: %s\n",_p);
    	if(_p && __p)
    	{
    	    char * _p1 = strchr(_p,' ');

    	    char * _p2 = strchr(_p1 + 1,':');
    	    char * _p3 = strchr(_p1 + 1,'\r');

    	    if(_p2)
    	    {
    	        char s_port[10];
    	        bzero(s_port,10);

    	        strncpy(remote_host,_p1+1,(int)(_p2  - _p1) - 1);
    	        strncpy(s_port,_p2+1,(int) (_p3 - _p2) -1);
    	        remote_port = atoi(s_port);

    	    } else 
    	    {
    	        strncpy(remote_host,_p1+1,(int)(_p3  - _p1) -1);
    	        remote_port = 80;
    	    }
    	    
    	    
    	    return 0;
    	}


    	char * p = strstr(header,m_s);
    	if(!p) 
    	{
	    	if(r_flag)
	    	{
		    	strncpy(remote_host, r_h, strlen(r_h));
		    	remote_port = r_p;
		    	return 0;
	    	}else 
	    	{
		    	return BAD_HTTP_PROTOCOL;
	    	}
    	}
    	char * p1 = strchr(p,'\n');
    	if(!p1) 
    	{
    	    return BAD_HTTP_PROTOCOL; 
    	}
    	
		int m_l = strlen(m_s);
    	char * p2 = strchr(p + m_l,':'); /* m_l是指类似自定义'Lbxx:'的长度 */

    	if(p2 && p2 < p1) 
    	{
    	    
    	    int p_len = (int)(p1 - p2 -1);
    	    char s_port[p_len];
    	    strncpy(s_port,p2+1,p_len);
    	    s_port[p_len] = '\0';
    	    remote_port = atoi(s_port);

    	    int h_len = (int)(p2 - p - m_l -1 );
    	    strncpy(remote_host,p + m_l + 1  ,h_len); //Host:
    	    //assert h_len < 128;
    	    remote_host[h_len] = '\0';
    	} else 
    	{   
    	    int h_len = (int)(p1 - p - m_l -1 -1); 
    	    strncpy(remote_host,p + m_l + 1,h_len);
    	    //assert h_len < 128;
    	    remote_host[h_len] = '\0';
    	    remote_port = 80;
    	}
    }
    return 0;
}

/* 响应隧道连接请求  */
int send_tunnel_ok(int client_sock)
{
    char * resp = "HTTP/1.1 200 Connection Established\r\n\r\n";
    int len = strlen(resp);
    char buffer[len+1];
    strcpy(buffer,resp);
    if(send_data(client_sock,buffer,len) < 0)
    {
        perror("Send http tunnel response  failed\n");
        return -1;
    }
    return 0;
}


//返回mproxy的运行基本信息
void hand_mproxy_info_req(int sock, char * header) {
    char server_path[255] ;
    char response[8192];
    extract_server_path(header,server_path);
    
    LOG("server path:%s\n",server_path);
    char info_buf[1024];
    get_info(info_buf);
    sprintf(response,"HTTP/1.0 200 OK\nServer: MProxy/0.1\n\
                    Content-type: text/html; charset=utf-8\n\n\
                     <html><body>\
                     <pre>%s</pre>\
                     </body></html>\n",info_buf);


    write(sock,response,strlen(response));

}

/* 获取运行的基本信息输出到指定的缓冲区 */
void get_info(char * output)
{
    int pos = 0;
    char line_buffer[512];
    sprintf(line_buffer,"======= mproxy (v0.1) ========\n");
    int len = strlen(line_buffer);
    memcpy(output,line_buffer,len);
    pos += len ;

    sprintf(line_buffer,"%s\n",get_work_mode());
    len = strlen(line_buffer);
    memcpy(output + pos,line_buffer,len);
    pos += len;

    if(strlen(remote_host) > 0) 
    {
        sprintf(line_buffer,"start server on %d and next hop is %s:%d\n",local_port,remote_host,remote_port);

    } else 
    {
        sprintf(line_buffer,"start server on %d\n",local_port);
    }
    
    len = strlen(line_buffer);
    memcpy(output+ pos,line_buffer,len);
    pos += len ;
    
    output[pos] = '\0';

}


const char * get_work_mode() 
{
	
    	if(strlen(remote_host) == 0) 
    	{
	    	if(!r_flag && !m_flag)
			{
    	    	if(io_flag == FLG_NONE) 
    	    	{
    	    	    return "start as normal http proxy";
    	    	} else if(io_flag == R_C_DEC)
    	    	{
    	    	   return "start as remote forward proxy and do decode data when recevie data" ;
    	    	}
    	    } else
    		{
	    		return "start as -r or -m mode";
    		}
    	    
    	} else 
    	{
    	    if(io_flag == FLG_NONE) 
    	    {
    	        return "start as remote forward proxy";
    	    } else if(io_flag == W_S_ENC) 
    	    {
    	        return "start as forward proxy and do encode data when send data";
    	    }
    	}
    	return "unknow";
    

}

/* 处理客户端的连接 */
void handle_client(int client_sock, struct sockaddr_in client_addr)
{
    int is_http_tunnel = 0; 
    if(strlen(remote_host) == 0) /* 未指定远端主机名称从http 请求 HOST 字段中获取 */
    {
        
        #ifdef DEBUG
        LOG(" ============ handle new client ============\n");
        LOG(">>>Header:%s\n",header_buffer);
        #endif
        
        if(read_header(client_sock,header_buffer) < 0)
        {
            LOG("Read Http header failed\n");
            return;
        } else 
        {
            char * p = strstr(header_buffer,"CONNECT"); /* 判断是否是http 隧道请求 */
            if(p) 
            {
                LOG("receive CONNECT request\n");
                is_http_tunnel = 1;
            }

            if(strstr(header_buffer,"GET /mproxy") >0 ) 
            {
                LOG("====== hand mproxy info request ====");
                //返回mproxy的运行基本信息
                hand_mproxy_info_req(client_sock,header_buffer);

                return; 
            }

            if(extract_host(header_buffer) < 0) 
            {
                LOG("Cannot extract host field,bad http protrotol");
                return;
            }
            LOG("Host:%s port: %d io_flag:%d\n",remote_host,remote_port,io_flag);

        }
    }

    if ((remote_sock = create_connection()) < 0) {
        LOG("Cannot connect to host [%s:%d]\n",remote_host,remote_port);
        return;
    }

    if (fork() == 0) { // 创建子进程用于从客户端转发数据到远端socket接口

        if(strlen(header_buffer) > 0 && !is_http_tunnel) 
        {
            forward_header(remote_sock); //普通的http请求先转发header
        } 
        
        forward_data(client_sock, remote_sock);
        exit(0);
    }

    if (fork() == 0) { // 创建子进程用于转发从远端socket接口过来的数据到客户端

        if(io_flag == W_S_ENC)
        {
            io_flag = R_C_DEC; //发送请求给服务端进行编码，读取服务端的响应则进行解码
        } else if (io_flag == R_C_DEC)
        {
             io_flag = W_S_ENC; //接收客户端请求进行解码，那么响应客户端请求需要编码
        }

        if(is_http_tunnel)
        {
            send_tunnel_ok(client_sock);
        } 

        forward_data(remote_sock, client_sock);
        exit(0);
    }

    close(remote_sock);
    close(client_sock);
}

void forward_header(int destination_sock)
{
    rewrite_header();
    #ifdef DEBUG
        LOG("================ The Forward HEAD =================");
        LOG("%s\n",header_buffer);
    #endif
   
    int len = strlen(header_buffer);
    send_data(destination_sock,header_buffer,len) ;
}

int send_data(int socket,char * buffer,int len)
{

    if(io_flag == W_S_ENC)
    {
        int i;
        for(i = 0; i < len ; i++)
        {
            buffer[i] ^= 1;
           
        }
    }

    return send(socket,buffer,len,0);
}

int receive_data(int socket, char * buffer, int len)
{
    int n = recv(socket, buffer, len, 0);
    if(io_flag == R_C_DEC && n > 0)
    {
        int i; 
        for(i = 0; i< n; i++ )
        {
            buffer[i] ^= 1;
            // printf("%d => %d\n",c,buffer[i]);
        }
    }

    return n;
}



/* 代理中的完整URL转发前需改成 path 的形式 */
void rewrite_header()
{
    char * p = strstr(header_buffer,"http://");
    char * p0 = strchr(p,'\0');
    char * p5 = strstr(header_buffer,"HTTP/"); /* "HTTP/" 是协议标识 如 "HTTP/1.1" */
    int len = strlen(header_buffer);
    if(p)
    {
        char * p1 = strchr(p + 7,'/');
        if(p1 && (p5 > p1)) 
        {
            //转换url到 path
            memcpy(p,p1,(int)(p0 -p1));
            int l = len - (p1 - p) ;
            header_buffer[l] = '\0';


        } else 
        {
            char * p2 = strchr(p,' ');  //GET http://3g.sina.com.cn HTTP/1.1

            // printf("%s\n",p2);
            memcpy(p + 1,p2,(int)(p0-p2));
            *p = '/';  //url 没有路径使用根
            int l  = len - (p2  - p ) + 1;
            header_buffer[l] = '\0';

        }
    }
    
    
    if(m_flag)
    {
    	char * p6 = strstr(header_buffer,"Host:");   //...\r\nHost: miguvod.lovev.com:8080\r\nLbxx: tiny.cc\r\n....
    	char * p00 = strchr(p6,'\0');					//	  p6	p7						  p8	p9				p00
    	char * p7 = strchr(p6,' ');	
    	char * p8 = strstr(header_buffer,m_s);				
    	char * p9 = strchr(p8,' ');
		if(p6)
		{
    		if(p8 && (p8 > p6))
    		{
		    	memcpy(p7,p9,(int)(p00 -p9));  //以Lbxx的值覆盖Host的值，这要求Lbxx在模式中要紧跟在Host后面(仅对http)
    	    	int l = len - (p9 - p7) ;
    	    	header_buffer[l] = '\0';
    		}
		}
	}
}


void forward_data(int source_sock, int destination_sock) {
    char buffer[BUF_SIZE];
    int n;

    while ((n = receive_data(source_sock, buffer, BUF_SIZE)) > 0) 
    { 

        send_data(destination_sock, buffer, n); 
    }

    shutdown(destination_sock, SHUT_RDWR); 

    shutdown(source_sock, SHUT_RDWR); 
}



int create_connection() {
    struct sockaddr_in server_addr;
    struct hostent *server;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return CLIENT_SOCKET_ERROR;
    }

    if ((server = gethostbyname(remote_host)) == NULL) {
        errno = EFAULT;
        return CLIENT_RESOLVE_ERROR;
    }
    LOG("======= forward request to remote host:%s port:%d ======= \n",remote_host,remote_port);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(remote_port);

    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        return CLIENT_CONNECT_ERROR;
    }

    return sock;
}


int create_server_socket(int port) {
    int server_sock, optval;
    struct sockaddr_in server_addr;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return SERVER_SOCKET_ERROR;
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        return SERVER_SETSOCKOPT_ERROR;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        return SERVER_BIND_ERROR;
    }

    if (listen(server_sock, 20) < 0) {
        return SERVER_LISTEN_ERROR;
    }

    return server_sock;
}

/* 处理僵尸进程 */
void sigchld_handler(int signal) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void server_loop() {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addrlen);
        
        if (fork() == 0) { // 创建子进程处理客户端连接请求
            close(server_sock);
            handle_client(client_sock, client_addr);
            exit(0);
        }
        close(client_sock);
    }

}

void stop_server()
{
    kill(m_pid, SIGKILL);        
}

void usage(void)
{
    printf("Usage:\n");
    printf(" -l <port number>  specifyed local listen port \n");
    printf(" -h <remote server and port> specifyed next hop server name to forward all trafic unhandled, prior to -m & -r\n");
    printf(" -r <server and port> specifyed server name to forward 'HTTP'&'CONNECT' to, no matter what host is\n");
    printf(" -m <ml key words>  specifyed key words replaced & recognized as 'Host:' function, prior to -r\n");
    printf(" -d <remote server and port> run as daemon\n");
    printf(" -E encode data when forwarding data\n");
    printf(" -D decode data when receiving data\n");
    printf(" Notice:-h -r -m can not be used together.\n");
    exit (8);
}

void start_server(int daemon)
{
    //初始化全局变量
    header_buffer = (char *) malloc(MAX_HEADER_SIZE);

    signal(SIGCHLD, sigchld_handler); // 防止子进程变成僵尸进程

    if ((server_sock = create_server_socket(local_port)) < 0) 
    { // start server
        LOG("Cannot run server on %d\n",local_port);
        exit(server_sock);
    }
   
    if(daemon)
    {
        pid_t pid;
        if((pid = fork()) == 0)
        {
            server_loop();
        } else if (pid > 0 ) 
        {
            m_pid = pid;
            LOG("mporxy pid is: [%d]\n",pid);
            close(server_sock);
        } else 
        {
            LOG("Cannot daemonize\n");
            exit(pid);
        }

    } else 
    {
        server_loop();
    }

}

int main(int argc, char *argv[])
{
    return _main(argc,argv);
}

int _main(int argc, char *argv[]) 
{
    local_port = DEFAULT_LOCAL_PORT;
    io_flag = FLG_NONE;
    r_flag = 0;
    m_flag = 0;
    //m_s = "DEFAULT_M_S";
    int daemon = 0; 

    char info_buf[2048];
	
	int opt;
	char optstrs[] = ":l:h:r:m:dED";
	char *p = NULL;
	while(-1 != (opt = getopt(argc, argv, optstrs)))
	{
		switch(opt)
		{
			case 'l':
				local_port = atoi(optarg);
				break;
			case 'h':
				p = strchr(optarg, ':');
				if(p)
				{
					strncpy(remote_host, optarg, p - optarg);
					remote_port = atoi(p+1);
				}
				else
				{
					strncpy(remote_host, optarg, strlen(optarg));
					remote_port = DEFAULT_REMOTE_PORT;
				}
				break;
			case 'r':
				p = strchr(optarg, ':');
				if(p)
				{
					strncpy(r_h, optarg, p - optarg);
					r_p = atoi(p+1);
				}
				else
				{
					strncpy(r_h, optarg, strlen(optarg));
					r_p = DEFAULT_R_P;
				}
				printf("Your forward server is: %s:%d\n",r_h,r_p);
				r_flag = 1;
				break;
			case 'm':
				strncpy(m_s, optarg, strlen(optarg));
				printf("Your sting is: %s\n",m_s);
				m_flag = 1;
				break;
			case 'd':
				daemon = 1;
				break;
			case 'E':
				io_flag = W_S_ENC;
				break;
			case 'D':
				io_flag = R_C_DEC;
				break;
			case ':':
				printf("\nMissing argument after: -%c\n", optopt);
				usage();
			case '?':
				printf("\nInvalid argument: %c\n", optopt);
			default:
				usage();
		}
    }

    get_info(info_buf);
    LOG("%s\n",info_buf);
    start_server(daemon);
    return 0;

}
