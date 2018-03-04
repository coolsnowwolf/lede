/*
 * delay_manager.h
 *
 *  Created on: Sep 15, 2017
 *      Author: root
 */

#ifndef DELAY_MANAGER_H_
#define DELAY_MANAGER_H_

#include "common.h"
#include "packet.h"
#include "log.h"

//enum delay_type_t {none=0,enum_sendto_u64,enum_send_fd,client_to_local,client_to_remote,server_to_local,server_to_remote};

/*
struct fd_ip_port_t
{
	int fd;
	u32_t ip;
	u32_t port;
};
union dest_t
{
	fd_ip_port_t fd_ip_port;
	int fd;
	u64_t u64;
};
*/

struct my_timer_t
{
	int timer_fd;
	fd64_t timer_fd64;
	my_timer_t()
	{
		if ((timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)) < 0)
		{
			mylog(log_fatal,"timer_fd create error");
			myexit(1);
		}
		timer_fd64=fd_manager.create(timer_fd);
	}
	my_timer_t(const my_timer_t &b)
	{
		assert(0==1);
	}
	~my_timer_t()
	{
		fd_manager.fd64_close(timer_fd64);
	}
	int add_fd_to_epoll(int epoll_fd)
	{
		epoll_event ev;;
		ev.events = EPOLLIN;
		ev.data.u64 = timer_fd;
		int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev);
		if (ret!= 0) {
			mylog(log_fatal,"add delay_manager.get_timer_fd() error\n");
			myexit(-1);
		}
		return 0;
	}
	int add_fd64_to_epoll(int epoll_fd)
	{
		epoll_event ev;;
		ev.events = EPOLLIN;
		ev.data.u64 = timer_fd64;
		int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev);
		if (ret!= 0) {
			mylog(log_fatal,"add delay_manager.get_timer_fd() error\n");
			myexit(-1);
		}
		return 0;
	}
	int get_timer_fd()
	{
		return timer_fd;
	}
	fd64_t get_timer_fd64()
	{
		return timer_fd64;
	}
	int set_timer_repeat_us(my_time_t my_time)
	{
		itimerspec its;
		memset(&its,0,sizeof(its));
		its.it_interval.tv_sec=my_time/1000000llu;
		its.it_interval.tv_nsec=my_time%1000000llu*1000llu;
		its.it_value.tv_nsec=1; //imidiately
		timerfd_settime(timer_fd,0,&its,0);
		return 0;
	}
	int set_timer_abs_us(my_time_t my_time)
	{
		itimerspec its;
		memset(&its,0,sizeof(its));
		its.it_value.tv_sec=my_time/1000000llu;
		its.it_value.tv_nsec=my_time%1000000llu*1000llu;
		timerfd_settime(timer_fd,TFD_TIMER_ABSTIME,&its,0);
		return 0;
	}
};
struct delay_data_t
{
	dest_t dest;
	//int left_time;//
	char * data;
	int len;
	int handle();
};

struct delay_manager_t
{
	int timer_fd;
	int capacity;
	multimap<my_time_t,delay_data_t> delay_mp;  //unit us,1 us=0.001ms
	delay_manager_t();
	delay_manager_t(delay_manager_t &b)
	{
		assert(0==1);
	}
	int set_capacity(int a){capacity=a;return 0;}
	~delay_manager_t();
	int get_timer_fd();
	int check();
	int add(my_time_t delay,const dest_t &dest,char *data,int len);
};

#endif /* DELAY_MANAGER_H_ */
