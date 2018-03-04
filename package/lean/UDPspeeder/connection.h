/*
 * connection.h
 *
 *  Created on: Sep 23, 2017
 *      Author: root
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

extern int disable_anti_replay;

#include "connection.h"
#include "common.h"
#include "log.h"
#include "delay_manager.h"
#include "fd_manager.h"
#include "fec_manager.h"

extern int report_interval;

struct conv_manager_t  // manage the udp connections
{
	//typedef hash_map map;
	unordered_map<u64_t,u32_t> u64_to_conv;  //conv and u64 are both supposed to be uniq
	unordered_map<u32_t,u64_t> conv_to_u64;
	unordered_map<u32_t,u64_t> conv_last_active_time;

	unordered_map<u32_t,u64_t>::iterator clear_it;

	//void (*clear_function)(uint64_t u64) ;

	long long last_clear_time;

	conv_manager_t();
	conv_manager_t(const conv_manager_t &b)
	{
		assert(0==1);
	}
	~conv_manager_t();
	int get_size();
	void reserve();
	void clear();
	u32_t get_new_conv();
	int is_conv_used(u32_t conv);
	int is_u64_used(u64_t u64);
	u32_t find_conv_by_u64(u64_t u64);
	u64_t find_u64_by_conv(u32_t conv);
	int update_active_time(u32_t conv);
	int insert_conv(u32_t conv,u64_t u64);
	int erase_conv(u32_t conv);
	int clear_inactive(char * ip_port=0);
	int clear_inactive0(char * ip_port);
};


struct inner_stat_t
{
	u64_t input_packet_num;
	u64_t input_packet_size;
	u64_t output_packet_num;
	u64_t output_packet_size;
};
struct stat_t
{
	u64_t last_report_time;
	inner_stat_t normal_to_fec;
	inner_stat_t fec_to_normal;
	stat_t()
	{
		memset(this,0,sizeof(stat_t));
	}
	void report_as_client()
	{
		if(report_interval!=0 &&get_current_time()-last_report_time>u64_t(report_interval)*1000)
		{
			last_report_time=get_current_time();
			inner_stat_t &a=normal_to_fec;
			inner_stat_t &b=fec_to_normal;
			mylog(log_info,"[report]client-->server:(original:%llu pkt;%llu byte) (fec:%llu pkt,%llu byte)  server-->client:(original:%llu pkt;%llu byte) (fec:%llu pkt;%llu byte)\n",
					a.input_packet_num,a.input_packet_size,a.output_packet_num,a.output_packet_size,
					b.output_packet_num,b.output_packet_size,b.input_packet_num,b.input_packet_size
					);
		}
	}
	void report_as_server(ip_port_t &ip_port)
	{
		if(report_interval!=0 &&get_current_time()-last_report_time>u64_t(report_interval)*1000)
		{
			last_report_time=get_current_time();
			inner_stat_t &a=fec_to_normal;
			inner_stat_t &b=normal_to_fec;
			mylog(log_info,"[report][%s]client-->server:(original:%llu pkt;%llu byte) (fec:%llu pkt;%llu byte)  server-->client:(original:%llu pkt;%llu byte) (fec:%llu pkt;%llu byte)\n",
					ip_port.to_s(),
					a.output_packet_num,a.output_packet_size,a.input_packet_num,a.input_packet_size,
					b.input_packet_num,b.input_packet_size,b.output_packet_num,b.output_packet_size
					);
		}
	}
};


struct conn_info_t     //stores info for a raw connection.for client ,there is only one connection,for server there can be thousand of connection since server can
//handle multiple clients
{
	conv_manager_t conv_manager;
	fec_encode_manager_t fec_encode_manager;
	fec_decode_manager_t fec_decode_manager;
	my_timer_t timer;
	//ip_port_t ip_port;
	u64_t last_active_time;
	stat_t stat;
	conn_info_t()
	{
	}
	void update_active_time()
	{
		last_active_time=get_current_time();
	}
	conn_info_t(const conn_info_t &b)
	{
		assert(0==1);
	}
};

struct conn_manager_t  //manager for connections. for client,we dont need conn_manager since there is only one connection.for server we use one conn_manager for all connections
{

	unordered_map<u64_t,conn_info_t*> mp;//<ip,port> to conn_info_t;
	unordered_map<u64_t,conn_info_t*>::iterator clear_it;
	long long last_clear_time;

	conn_manager_t();
	conn_manager_t(const conn_info_t &b)
	{
		assert(0==1);
	}
	int exist(ip_port_t);
	conn_info_t *& find_p(ip_port_t);  //be aware,the adress may change after rehash
	conn_info_t & find(ip_port_t) ; //be aware,the adress may change after rehash
	int insert(ip_port_t);

	int erase(unordered_map<u64_t,conn_info_t*>::iterator erase_it);
	int clear_inactive();
	int clear_inactive0();

};

extern conn_manager_t conn_manager;


#endif /* CONNECTION_H_ */
