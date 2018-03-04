/*
 * connection.cpp
 *
 *  Created on: Sep 23, 2017
 *      Author: root
 */

#include "connection.h"

const int disable_conv_clear=0;//a udp connection in the multiplexer is called conversation in this program,conv for short.

const int disable_conn_clear=0;//a raw connection is called conn.

int report_interval=0;

void server_clear_function(u64_t u64)//used in conv_manager in server mode.for server we have to use one udp fd for one conv(udp connection),
//so we have to close the fd when conv expires
{
	fd64_t fd64=u64;
	assert(fd_manager.exist(fd64));
	fd_manager.fd64_close(fd64);
}

////////////////////////////////////////////////////////////////////

conv_manager_t::conv_manager_t()
{
	clear_it=conv_last_active_time.begin();
	long long last_clear_time=0;
	reserve();
}
conv_manager_t::~conv_manager_t()
{
	clear();
}
int conv_manager_t::get_size()
{
	return conv_to_u64.size();
}
void conv_manager_t::reserve()
{
	u64_to_conv.reserve(10007);
	conv_to_u64.reserve(10007);
	conv_last_active_time.reserve(10007);
}
void conv_manager_t::clear()
{
	//if(disable_conv_clear) return ;//////what was the purpose of this code?

	if(client_or_server==server_mode)
	{
		for(auto it=conv_to_u64.begin();it!=conv_to_u64.end();it++)
		{
			//int fd=int((it->second<<32u)>>32u);
			server_clear_function(  it->second);
		}
	}
	u64_to_conv.clear();
	conv_to_u64.clear();
	conv_last_active_time.clear();

	clear_it=conv_last_active_time.begin();

}
u32_t conv_manager_t::get_new_conv()
{
	u32_t conv=get_true_random_number_nz();
	while(conv_to_u64.find(conv)!=conv_to_u64.end())
	{
		conv=get_true_random_number_nz();
	}
	return conv;
}
int conv_manager_t::is_conv_used(u32_t conv)
{
	return conv_to_u64.find(conv)!=conv_to_u64.end();
}
int conv_manager_t::is_u64_used(u64_t u64)
{
	return u64_to_conv.find(u64)!=u64_to_conv.end();
}
u32_t conv_manager_t::find_conv_by_u64(u64_t u64)
{
	assert(is_u64_used(u64));
	return u64_to_conv[u64];
}
u64_t conv_manager_t::find_u64_by_conv(u32_t conv)
{
	assert(is_conv_used(conv));
	return conv_to_u64[conv];
}
int conv_manager_t::update_active_time(u32_t conv)
{
	assert(is_conv_used(conv));
	return conv_last_active_time[conv]=get_current_time();
}
int conv_manager_t::insert_conv(u32_t conv,u64_t u64)//////todo add capacity ///done at upper level
{
	assert(!is_conv_used(conv));
	int bucket_size_before=conv_last_active_time.bucket_count();
	u64_to_conv[u64]=conv;
	conv_to_u64[conv]=u64;
	conv_last_active_time[conv]=get_current_time();
	int bucket_size_after=conv_last_active_time.bucket_count();
	if(bucket_size_after!=bucket_size_before)
		clear_it=conv_last_active_time.begin();
	return 0;
}
int conv_manager_t::erase_conv(u32_t conv)
{
	//if(disable_conv_clear) return 0;
	assert(conv_last_active_time.find(conv)!=conv_last_active_time.end());
	u64_t u64=conv_to_u64[conv];
	if(client_or_server==server_mode)
	{
		server_clear_function(u64);
	}
	assert(conv_to_u64.find(conv)!=conv_to_u64.end());
	conv_to_u64.erase(conv);
	u64_to_conv.erase(u64);
	conv_last_active_time.erase(conv);
	return 0;
}
int conv_manager_t::clear_inactive(char * ip_port)
{
	if(get_current_time()-last_clear_time>conv_clear_interval)
	{
		last_clear_time=get_current_time();
		return clear_inactive0(ip_port);
	}
	return 0;
}
int conv_manager_t::clear_inactive0(char * ip_port)
{
	if(disable_conv_clear) return 0;

	//map<uint32_t,uint64_t>::iterator it;
	int cnt=0;
	auto it=clear_it;
	int size=conv_last_active_time.size();
	int num_to_clean=size/conv_clear_ratio+conv_clear_min;   //clear 1/10 each time,to avoid latency glitch

	num_to_clean=min(num_to_clean,size);

	u64_t current_time=get_current_time();
	for(;;)
	{
		if(cnt>=num_to_clean) break;
		if(conv_last_active_time.begin()==conv_last_active_time.end()) break;

		if(it==conv_last_active_time.end())
		{
			it=conv_last_active_time.begin();
		}

		if( current_time -it->second  >conv_timeout )
		{
			//mylog(log_info,"inactive conv %u cleared \n",it->first);
			//auto old_it=it;
			//it++;
			u32_t conv= it->first;
			it++;
			erase_conv(conv);
			if(ip_port==0)
			{
				mylog(log_info,"conv %x cleared\n",conv);
			}
			else
			{
				mylog(log_info,"[%s]conv %x cleared\n",ip_port,conv);
			}
		}
		else
		{
			it++;
		}
		cnt++;
	}
	clear_it=it;
	return 0;
}


////////////////////////////////////////////////////////////////////


conn_manager_t::conn_manager_t()
{
	//ready_num=0;
	mp.reserve(10007);
	//fd64_mp.reserve(100007);
	clear_it=mp.begin();
	last_clear_time=0;
}
int conn_manager_t::exist(ip_port_t ip_port)
{
	u64_t u64=ip_port.to_u64();
	if(mp.find(u64)!=mp.end())
	{
	 return 1;
	}
	return 0;
}
 conn_info_t *& conn_manager_t::find_p(ip_port_t ip_port) //todo capacity   ///done at upper level
 //be aware,the adress may change after rehash
 {
	 assert(exist(ip_port));
	 u64_t u64=ip_port.to_u64();
	 return mp[u64];
 }
 conn_info_t & conn_manager_t::find(ip_port_t ip_port)  //be aware,the adress may change after rehash
 {
	 assert(exist(ip_port));
	 u64_t u64=ip_port.to_u64();
	 return *mp[u64];
 }
 int conn_manager_t::insert(ip_port_t ip_port)
 {
	assert(!exist(ip_port));
	int bucket_size_before=mp.bucket_count();
	mp[ip_port.to_u64()]=new conn_info_t;
	int bucket_size_after=mp.bucket_count();
	if(bucket_size_after!=bucket_size_before)
		clear_it=mp.begin();
	return 0;
 }
 int conn_manager_t::erase(unordered_map<u64_t,conn_info_t*>::iterator erase_it)
 {
	 ////////todo  close and erase timer_fd ,check fd64 empty   ///dont need
		delete(erase_it->second);
		mp.erase(erase_it->first);
		return 0;
 }
int conn_manager_t::clear_inactive()
{
	if(get_current_time()-last_clear_time>conn_clear_interval)
	{
		last_clear_time=get_current_time();
		return clear_inactive0();
	}
	return 0;
}
int conn_manager_t::clear_inactive0()
{
//mylog(log_info,"called\n");
	 unordered_map<u64_t,conn_info_t*>::iterator it;
	 unordered_map<u64_t,conn_info_t*>::iterator old_it;

	if(disable_conn_clear) return 0;

	//map<uint32_t,uint64_t>::iterator it;
	int cnt=0;
	it=clear_it;//TODO,write it back
	int size=mp.size();
	int num_to_clean=size/conn_clear_ratio+conn_clear_min;   //clear 1/10 each time,to avoid latency glitch

	//mylog(log_trace,"mp.size() %d\n", size);

	num_to_clean=min(num_to_clean,(int)mp.size());
	u64_t current_time=get_current_time();

	//mylog(log_info,"here size=%d\n",(int)mp.size());
	for(;;)
	{
		if(cnt>=num_to_clean) break;
		if(mp.begin()==mp.end()) break;
		if(it==mp.end())
		{
			it=mp.begin();
		}

		if(it->second->conv_manager.get_size() >0)
		{
			//mylog(log_info,"[%s:%d]size %d \n",my_ntoa(get_u64_h(it->first)),get_u64_l(it->first),(int)it->second->conv_manager.get_size());
			it++;
		}
		else if(current_time<it->second->last_active_time+server_conn_timeout)
		{
			it++;
		}
		else
		{
			mylog(log_info,"[%s:%d]inactive conn cleared \n",my_ntoa(get_u64_h(it->first)),get_u64_l(it->first));
			old_it=it;
			it++;
			erase(old_it);
		}
		cnt++;
	}
	clear_it=it;
	return 0;
}

