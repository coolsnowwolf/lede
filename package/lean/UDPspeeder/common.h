/*
 * common.h
 *
 *  Created on: Jul 29, 2017
 *      Author: wangyu
 */

#ifndef COMMON_H_
#define COMMON_H_
//#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<getopt.h>

#include<unistd.h>
#include<errno.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/socket.h>    //for socket ofcourse
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/udp.h>
#include <netinet/ip.h>    //Provides declarations for ip header
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <sys/time.h>
#include <time.h>
#include <sys/timerfd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <assert.h>
#include <linux/if_packet.h>
#include <linux/if_tun.h>

#include<unordered_map>
#include<unordered_set>
#include<map>
#include<list>
using  namespace std;


typedef unsigned long long u64_t;   //this works on most platform,avoid using the PRId64
typedef long long i64_t;

typedef unsigned int u32_t;
typedef int i32_t;

typedef unsigned short u16_t;
typedef short i16_t;

typedef u64_t my_time_t;

const int max_data_len=2200;
const int buf_len=max_data_len+200;


//const u32_t timer_interval=400;
////const u32_t conv_timeout=180000;
//const u32_t conv_timeout=40000;//for test
const u32_t conv_timeout=180000;
const int max_conv_num=10000;
const int max_conn_num=200;

/*
const u32_t max_handshake_conn_num=10000;
const u32_t max_ready_conn_num=1000;
//const u32_t anti_replay_window_size=1000;


const u32_t client_handshake_timeout=5000;
const u32_t client_retry_interval=1000;

const u32_t server_handshake_timeout=10000;// this should be much longer than clients. client retry initially ,server retry passtively*/

const int conv_clear_ratio=30;  //conv grabage collecter check 1/30 of all conv one time
const int conn_clear_ratio=50;
const int conv_clear_min=1;
const int conn_clear_min=1;

const u32_t conv_clear_interval=1000;
const u32_t conn_clear_interval=1000;


const i32_t max_fail_time=0;//disable

const u32_t heartbeat_interval=1000;

const u32_t timer_interval=400;//this should be smaller than heartbeat_interval and retry interval;

//const uint32_t conv_timeout=120000; //120 second
//const u32_t conv_timeout=120000; //for test

const u32_t client_conn_timeout=10000;
const u32_t client_conn_uplink_timeout=client_conn_timeout+2000;

//const uint32_t server_conn_timeout=conv_timeout+60000;//this should be 60s+ longer than conv_timeout,so that conv_manager can destruct convs gradually,to avoid latency glicth
const u32_t server_conn_timeout=conv_timeout+20000;//for test


extern int about_to_exit;

enum raw_mode_t{mode_faketcp=0,mode_udp,mode_icmp,mode_end};
extern raw_mode_t raw_mode;
enum program_mode_t {unset_mode=0,client_mode,server_mode};
extern program_mode_t client_or_server;
extern unordered_map<int, const char*> raw_mode_tostring ;

enum working_mode_t {unset_working_mode=0,tunnel_mode,tun_dev_mode};
extern working_mode_t working_mode;

extern int socket_buf_size;


typedef u32_t id_t;

typedef u64_t iv_t;

typedef u64_t padding_t;

typedef u64_t anti_replay_seq_t;

typedef u64_t fd64_t;

//enum dest_type{none=0,type_fd64_ip_port,type_fd64,type_fd64_ip_port_conv,type_fd64_conv/*,type_fd*/};
enum dest_type{none=0,type_fd64_ip_port,type_fd64,type_fd,type_write_fd,type_fd_ip_port/*,type_fd*/};

struct ip_port_t
{
	u32_t ip;
	int port;
	void from_u64(u64_t u64);
	u64_t to_u64();
	char * to_s();
};

struct fd64_ip_port_t
{
	fd64_t fd64;
	ip_port_t ip_port;
};
struct fd_ip_port_t
{
	int fd;
	ip_port_t ip_port;
};
union inner_t
{
	fd64_t fd64;
	int fd;
	fd64_ip_port_t fd64_ip_port;
	fd_ip_port_t fd_ip_port;
};
struct dest_t
{
	dest_type type;
	inner_t inner;
	u32_t conv;
	int cook=0;
};

struct fd_info_t
{
	ip_port_t ip_port;
};

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

u64_t get_current_time();
u64_t get_current_time_us();
u64_t pack_u64(u32_t a,u32_t b);

u32_t get_u64_h(u64_t a);

u32_t get_u64_l(u64_t a);

void write_u16(char *,u16_t a);
u16_t read_u16(char *);

void write_u32(char *,u32_t a);
u32_t read_u32(char *);

void write_u64(char *,u64_t a);
u64_t read_uu64(char *);

char * my_ntoa(u32_t ip);

void myexit(int a);
void init_random_number_fd();
u64_t get_true_random_number_64();
u32_t get_true_random_number();
u32_t get_true_random_number_nz();
u64_t ntoh64(u64_t a);
u64_t hton64(u64_t a);
bool larger_than_u16(uint16_t a,uint16_t b);
bool larger_than_u32(u32_t a,u32_t b);
void setnonblocking(int sock);
int set_buf_size(int fd,int socket_buf_size,int force_socket_buf=0);

unsigned short csum(const unsigned short *ptr,int nbytes);
unsigned short tcp_csum(const pseudo_header & ph,const unsigned short *ptr,int nbytes);

void  signal_handler(int sig);
int numbers_to_char(id_t id1,id_t id2,id_t id3,char * &data,int &len);
int char_to_numbers(const char * data,int len,id_t &id1,id_t &id2,id_t &id3);

void myexit(int a);

int add_iptables_rule(char *);

int clear_iptables_rule();
void get_true_random_chars(char * s,int len);
int random_between(u32_t a,u32_t b);

int set_timer_ms(int epollfd,int &timer_fd,u32_t timer_interval);

int round_up_div(int a,int b);

int create_fifo(char * file);
/*
int create_new_udp(int &new_udp_fd,int remote_address_uint32,int remote_port);
*/

int new_listen_socket(int &fd,u32_t ip,int port);

int new_connected_socket(int &fd,u32_t ip,int port);

#endif /* COMMON_H_ */
