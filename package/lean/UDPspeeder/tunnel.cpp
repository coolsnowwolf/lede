/*
 * tunnel.cpp
 *
 *  Created on: Oct 26, 2017
 *      Author: root
 */

#include "tunnel.h"


int tunnel_client_event_loop()
{
	//char buf[buf_len];
	int i, j, k;int ret;
	int yes = 1;
	int epoll_fd;
	int remote_fd;
	fd64_t remote_fd64;

    conn_info_t *conn_info_p=new conn_info_t;
    conn_info_t &conn_info=*conn_info_p;  //huge size of conn_info,do not allocate on stack
    //conn_info.conv_manager.reserve();
	//conn_info.fec_encode_manager.re_init(fec_data_num,fec_redundant_num,fec_mtu,fec_pending_num,fec_pending_time,fec_type);


	int local_listen_fd;
	//fd64_t local_listen_fd64;
    new_listen_socket(local_listen_fd,local_ip_uint32,local_port);
    //local_listen_fd64=fd_manager.create(local_listen_fd);

	epoll_fd = epoll_create1(0);
	assert(epoll_fd>0);

	const int max_events = 4096;
	struct epoll_event ev, events[max_events];
	if (epoll_fd < 0) {
		mylog(log_fatal,"epoll return %d\n", epoll_fd);
		myexit(-1);
	}

	ev.events = EPOLLIN;
	ev.data.u64 = local_listen_fd;
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_listen_fd, &ev);
	if (ret!=0) {
		mylog(log_fatal,"add  udp_listen_fd error\n");
		myexit(-1);
	}

	assert(new_connected_socket(remote_fd,remote_ip_uint32,remote_port)==0);
	remote_fd64=fd_manager.create(remote_fd);

	mylog(log_debug,"remote_fd64=%llu\n",remote_fd64);

	ev.events = EPOLLIN;
	ev.data.u64 = remote_fd64;

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, remote_fd, &ev);
	if (ret!= 0) {
		mylog(log_fatal,"add raw_fd error\n");
		myexit(-1);
	}

	ev.events = EPOLLIN;
	ev.data.u64 = delay_manager.get_timer_fd();

	mylog(log_debug,"delay_manager.get_timer_fd()=%d\n",delay_manager.get_timer_fd());
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, delay_manager.get_timer_fd(), &ev);
	if (ret!= 0) {
		mylog(log_fatal,"add delay_manager.get_timer_fd() error\n");
		myexit(-1);
	}

	u64_t tmp_fd64=conn_info.fec_encode_manager.get_timer_fd64();
	ev.events = EPOLLIN;
	ev.data.u64 = tmp_fd64;

	mylog(log_debug,"conn_info.fec_encode_manager.get_timer_fd64()=%llu\n",conn_info.fec_encode_manager.get_timer_fd64());
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_manager.to_fd(tmp_fd64), &ev);
	if (ret!= 0) {
		mylog(log_fatal,"add fec_encode_manager.get_timer_fd64() error\n");
		myexit(-1);
	}

	//my_timer_t timer;
	conn_info.timer.add_fd_to_epoll(epoll_fd);
	conn_info.timer.set_timer_repeat_us(timer_interval*1000);

	mylog(log_debug,"conn_info.timer.get_timer_fd()=%d\n",conn_info.timer.get_timer_fd());



	int fifo_fd=-1;

	if(fifo_file[0]!=0)
	{
		fifo_fd=create_fifo(fifo_file);
		ev.events = EPOLLIN;
		ev.data.u64 = fifo_fd;

		ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fifo_fd, &ev);
		if (ret!= 0) {
			mylog(log_fatal,"add fifo_fd to epoll error %s\n",strerror(errno));
			myexit(-1);
		}
		mylog(log_info,"fifo_file=%s\n",fifo_file);
	}

	while(1)////////////////////////
	{
		if(about_to_exit) myexit(0);

		int nfds = epoll_wait(epoll_fd, events, max_events, 180 * 1000);
		if (nfds < 0) {  //allow zero
			if(errno==EINTR  )
			{
				mylog(log_info,"epoll interrupted by signal continue\n");
				//myexit(0);
			}
			else
			{
				mylog(log_fatal,"epoll_wait return %d,%s\n", nfds,strerror(errno));
				myexit(-1);
			}
		}
		int idx;
		for (idx = 0; idx < nfds; ++idx) {
			if(events[idx].data.u64==(u64_t)conn_info.timer.get_timer_fd())
			{
				uint64_t value;
				read(conn_info.timer.get_timer_fd(), &value, 8);
				conn_info.conv_manager.clear_inactive();
				mylog(log_trace,"events[idx].data.u64==(u64_t)conn_info.timer.get_timer_fd()\n");

				conn_info.stat.report_as_client();

				if(debug_force_flush_fec)
				{
				int  out_n;char **out_arr;int *out_len;my_time_t *out_delay;
				dest_t dest;
				dest.type=type_fd64;
				dest.inner.fd64=remote_fd64;
				dest.cook=1;
				from_normal_to_fec(conn_info,0,0,out_n,out_arr,out_len,out_delay);
				for(int i=0;i<out_n;i++)
				{
					delay_send(out_delay[i],dest,out_arr[i],out_len[i]);
				}
				}
			}
			else if (events[idx].data.u64 == (u64_t)fifo_fd)
			{
				char buf[buf_len];
				int len=read (fifo_fd, buf, sizeof (buf));
				if(len<0)
				{
					mylog(log_warn,"fifo read failed len=%d,errno=%s\n",len,strerror(errno));
					continue;
				}
				buf[len]=0;
				handle_command(buf);
			}
			else if (events[idx].data.u64 == (u64_t)local_listen_fd||events[idx].data.u64 == conn_info.fec_encode_manager.get_timer_fd64())
			{
				char data[buf_len];
				int data_len;
				ip_port_t ip_port;
				u32_t conv;
				int  out_n;char **out_arr;int *out_len;my_time_t *out_delay;
				dest_t dest;
				dest.type=type_fd64;
				dest.inner.fd64=remote_fd64;
				dest.cook=1;

				if(events[idx].data.u64 == conn_info.fec_encode_manager.get_timer_fd64())
				{
					fd64_t fd64=events[idx].data.u64;
					mylog(log_trace,"events[idx].data.u64 == conn_info.fec_encode_manager.get_timer_fd64()\n");

					//mylog(log_info,"timer!!!\n");
					uint64_t value;
					if(!fd_manager.exist(fd64))   //fd64 has been closed
					{
						mylog(log_trace,"!fd_manager.exist(fd64)");
						continue;
					}
					if((ret=read(fd_manager.to_fd(fd64), &value, 8))!=8)
					{
						mylog(log_trace,"(ret=read(fd_manager.to_fd(fd64), &value, 8))!=8,ret=%d\n",ret);
						continue;
					}
					if(value==0)
					{
						mylog(log_debug,"value==0\n");
						continue;
					}
					assert(value==1);
					from_normal_to_fec(conn_info,0,0,out_n,out_arr,out_len,out_delay);
					//from_normal_to_fec(conn_info,0,0,out_n,out_arr,out_len,out_delay);
				}
				else//events[idx].data.u64 == (u64_t)local_listen_fd
				{
					mylog(log_trace,"events[idx].data.u64 == (u64_t)local_listen_fd\n");
					struct sockaddr_in udp_new_addr_in={0};
					socklen_t udp_new_addr_len = sizeof(sockaddr_in);
					if ((data_len = recvfrom(local_listen_fd, data, max_data_len, 0,
							(struct sockaddr *) &udp_new_addr_in, &udp_new_addr_len)) == -1) {
						mylog(log_error,"recv_from error,this shouldnt happen,err=%s,but we can try to continue\n",strerror(errno));
						continue;
						//mylog(log_error,"recv_from error,this shouldnt happen at client\n");
						//myexit(1);
					};

					if(!disable_mtu_warn&&data_len>=mtu_warn)
					{
						mylog(log_warn,"huge packet,data len=%d (>=%d).strongly suggested to set a smaller mtu at upper level,to get rid of this warn\n ",data_len,mtu_warn);
					}
					mylog(log_trace,"Received packet from %s:%d,len: %d\n", inet_ntoa(udp_new_addr_in.sin_addr),
							ntohs(udp_new_addr_in.sin_port),data_len);

					ip_port.ip=udp_new_addr_in.sin_addr.s_addr;
					ip_port.port=ntohs(udp_new_addr_in.sin_port);

					u64_t u64=ip_port.to_u64();

					if(!conn_info.conv_manager.is_u64_used(u64))
					{
						if(conn_info.conv_manager.get_size() >=max_conv_num)
						{
							mylog(log_warn,"ignored new udp connect bc max_conv_num exceed\n");
							continue;
						}
						conv=conn_info.conv_manager.get_new_conv();
						conn_info.conv_manager.insert_conv(conv,u64);
						mylog(log_info,"new packet from %s:%d,conv_id=%x\n",inet_ntoa(udp_new_addr_in.sin_addr),ntohs(udp_new_addr_in.sin_port),conv);
					}
					else
					{
						conv=conn_info.conv_manager.find_conv_by_u64(u64);
						mylog(log_trace,"conv=%d\n",conv);
					}
					conn_info.conv_manager.update_active_time(conv);
					char * new_data;
					int new_len;
					put_conv(conv,data,data_len,new_data,new_len);


					mylog(log_trace,"data_len=%d new_len=%d\n",data_len,new_len);
					//dest.conv=conv;
					from_normal_to_fec(conn_info,new_data,new_len,out_n,out_arr,out_len,out_delay);

				}
				mylog(log_trace,"out_n=%d\n",out_n);
				for(int i=0;i<out_n;i++)
				{
					delay_send(out_delay[i],dest,out_arr[i],out_len[i]);
				}
				//my_send(dest,data,data_len);
			}
		    else if (events[idx].data.u64 == (u64_t)delay_manager.get_timer_fd()) {
				uint64_t value;
				read(delay_manager.get_timer_fd(), &value, 8);
				mylog(log_trace,"events[idx].data.u64 == (u64_t)delay_manager.get_timer_fd()\n");
				//printf("<timerfd_triggered, %d>",delay_mp.size());
				//fflush(stdout);
			}
			else if(events[idx].data.u64>u32_t(-1) )
			{
				char data[buf_len];
				if(!fd_manager.exist(events[idx].data.u64))   //fd64 has been closed
				{
					mylog(log_trace,"!fd_manager.exist(events[idx].data.u64)");
					continue;
				}
				assert(events[idx].data.u64==remote_fd64);
				int fd=fd_manager.to_fd(remote_fd64);
				int data_len =recv(fd,data,max_data_len,0);
				mylog(log_trace, "received data from udp fd %d, len=%d\n", remote_fd,data_len);
				if(data_len<0)
				{
					if(errno==ECONNREFUSED)
					{
						//conn_manager.clear_list.push_back(udp_fd);
						mylog(log_debug, "recv failed %d ,udp_fd%d,errno:%s\n", data_len,remote_fd,strerror(errno));
					}

					mylog(log_warn, "recv failed %d ,udp_fd%d,errno:%s\n", data_len,remote_fd,strerror(errno));
					continue;
				}
				if(!disable_mtu_warn&&data_len>mtu_warn)
				{
					mylog(log_warn,"huge packet,data len=%d (>%d).strongly suggested to set a smaller mtu at upper level,to get rid of this warn\n ",data_len,mtu_warn);
				}

				if(de_cook(data,data_len)!=0)
				{
					mylog(log_debug,"de_cook error");
					continue;
				}

				int  out_n;char **out_arr;int *out_len;my_time_t *out_delay;
				from_fec_to_normal(conn_info,data,data_len,out_n,out_arr,out_len,out_delay);

				mylog(log_trace,"out_n=%d\n",out_n);

				for(int i=0;i<out_n;i++)
				{
					u32_t conv;
					char *new_data;
					int new_len;
					if(get_conv(conv,out_arr[i],out_len[i],new_data,new_len)!=0)
					{
						mylog(log_debug,"get_conv(conv,out_arr[i],out_len[i],new_data,new_len)!=0");
						continue;
					}
					if(!conn_info.conv_manager.is_conv_used(conv))
					{
						mylog(log_trace,"!conn_info.conv_manager.is_conv_used(conv)");
						continue;
					}

					conn_info.conv_manager.update_active_time(conv);

					u64_t u64=conn_info.conv_manager.find_u64_by_conv(conv);
					dest_t dest;
					dest.inner.fd_ip_port.fd=local_listen_fd;
					dest.inner.fd_ip_port.ip_port.from_u64(u64);
					dest.type=type_fd_ip_port;
					//dest.conv=conv;

					delay_send(out_delay[i],dest,new_data,new_len);
				}
				//mylog(log_trace,"[%s] send packet\n",dest.inner.ip_port.to_s());
			}
			else
			{
				mylog(log_fatal,"unknown fd,this should never happen\n");
				myexit(-1);
			}
		}
		delay_manager.check();
	}
	return 0;
}

int tunnel_server_event_loop()
{
	//char buf[buf_len];
	int i, j, k;int ret;
	int yes = 1;
	int epoll_fd;
	int remote_fd;

//    conn_info_t conn_info;
	int local_listen_fd;
//	fd64_t local_listen_fd64;
    new_listen_socket(local_listen_fd,local_ip_uint32,local_port);
   // local_listen_fd64=fd_manager.create(local_listen_fd);

	epoll_fd = epoll_create1(0);
	assert(epoll_fd>0);

	const int max_events = 4096;
	struct epoll_event ev, events[max_events];
	if (epoll_fd < 0) {
		mylog(log_fatal,"epoll return %d\n", epoll_fd);
		myexit(-1);
	}

	ev.events = EPOLLIN;
	ev.data.u64 = local_listen_fd;
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_listen_fd, &ev);
	if (ret!=0) {
		mylog(log_fatal,"add  udp_listen_fd error\n");
		myexit(-1);
	}

	ev.events = EPOLLIN;
	ev.data.u64 = delay_manager.get_timer_fd();
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, delay_manager.get_timer_fd(), &ev);
	if (ret!= 0) {
		mylog(log_fatal,"add delay_manager.get_timer_fd() error\n");
		myexit(-1);
	}

	mylog(log_debug," delay_manager.get_timer_fd() =%d\n", delay_manager.get_timer_fd());

	mylog(log_info,"now listening at %s:%d\n",my_ntoa(local_ip_uint32),local_port);

	my_timer_t timer;
	timer.add_fd_to_epoll(epoll_fd);
	timer.set_timer_repeat_us(timer_interval*1000);

	mylog(log_debug," timer.get_timer_fd() =%d\n",timer.get_timer_fd());

	int fifo_fd=-1;

	if(fifo_file[0]!=0)
	{
		fifo_fd=create_fifo(fifo_file);
		ev.events = EPOLLIN;
		ev.data.u64 = fifo_fd;

		ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fifo_fd, &ev);
		if (ret!= 0) {
			mylog(log_fatal,"add fifo_fd to epoll error %s\n",strerror(errno));
			myexit(-1);
		}
		mylog(log_info,"fifo_file=%s\n",fifo_file);
	}

	while(1)////////////////////////
	{

		if(about_to_exit) myexit(0);

		int nfds = epoll_wait(epoll_fd, events, max_events, 180 * 1000);
		if (nfds < 0) {  //allow zero
			if(errno==EINTR  )
			{
				mylog(log_info,"epoll interrupted by signal,continue\n");
				//myexit(0);
			}
			else
			{
				mylog(log_fatal,"epoll_wait return %d,%s\n", nfds,strerror(errno));
				myexit(-1);
			}
		}
		int idx;
		for (idx = 0; idx < nfds; ++idx)
		{
			/*
			if ((events[idx].data.u64 ) == (u64_t)timer_fd)
			{
				conn_manager.clear_inactive();
				u64_t dummy;
				read(timer_fd, &dummy, 8);
				//current_time_rough=get_current_time();
			}
			else */
			if(events[idx].data.u64==(u64_t)timer.get_timer_fd())
			{
				uint64_t value;
				read(timer.get_timer_fd(), &value, 8);
				conn_manager.clear_inactive();
				mylog(log_trace,"events[idx].data.u64==(u64_t)timer.get_timer_fd()\n");
				//conn_info.conv_manager.clear_inactive();
			}

			else if (events[idx].data.u64 == (u64_t)fifo_fd)
			{
				char buf[buf_len];
				int len=read (fifo_fd, buf, sizeof (buf));
				if(len<0)
				{
					mylog(log_warn,"fifo read failed len=%d,errno=%s\n",len,strerror(errno));
					continue;
				}
				buf[len]=0;
				handle_command(buf);
			}

			else if (events[idx].data.u64 == (u64_t)local_listen_fd)
			{

				mylog(log_trace,"events[idx].data.u64 == (u64_t)local_listen_fd\n");
				//int recv_len;
				char data[buf_len];
				int data_len;
				struct sockaddr_in udp_new_addr_in={0};
				socklen_t udp_new_addr_len = sizeof(sockaddr_in);
				if ((data_len = recvfrom(local_listen_fd, data, max_data_len, 0,
						(struct sockaddr *) &udp_new_addr_in, &udp_new_addr_len)) == -1) {
					mylog(log_error,"recv_from error,this shouldnt happen,err=%s,but we can try to continue\n",strerror(errno));
					continue;
					//myexit(1);
				};
				mylog(log_trace,"Received packet from %s:%d,len: %d\n", inet_ntoa(udp_new_addr_in.sin_addr),
						ntohs(udp_new_addr_in.sin_port),data_len);

				if(!disable_mtu_warn&&data_len>=mtu_warn)///////////////////////delete this for type 0 in furture
				{
					mylog(log_warn,"huge packet,data len=%d (>=%d).strongly suggested to set a smaller mtu at upper level,to get rid of this warn\n ",data_len,mtu_warn);
				}


				if(de_cook(data,data_len)!=0)
				{
					mylog(log_debug,"de_cook error");
					continue;
				}


				ip_port_t ip_port;
				ip_port.ip=udp_new_addr_in.sin_addr.s_addr;
				ip_port.port=ntohs(udp_new_addr_in.sin_port);
				mylog(log_trace,"ip_port= %s\n",ip_port.to_s());
				if(!conn_manager.exist(ip_port))
				{
					if(conn_manager.mp.size() >=max_conn_num)
					{
						mylog(log_warn,"new connection %s ignored bc max_conn_num exceed\n",ip_port.to_s());
						continue;
					}

					conn_manager.insert(ip_port);
					conn_info_t &conn_info=conn_manager.find(ip_port);
					//conn_info.fec_encode_manager.re_init(fec_data_num,fec_redundant_num,fec_mtu,fec_pending_num,fec_pending_time,fec_type);
					//conn_info.conv_manager.reserve();  //already reserved in constructor

					u64_t fec_fd64=conn_info.fec_encode_manager.get_timer_fd64();
					mylog(log_debug,"fec_fd64=%llu\n",fec_fd64);
					ev.events = EPOLLIN;
					ev.data.u64 = fec_fd64;
					ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_manager.to_fd(fec_fd64), &ev);

					fd_manager.get_info(fec_fd64).ip_port=ip_port;

					conn_info.timer.add_fd64_to_epoll(epoll_fd);
					conn_info.timer.set_timer_repeat_us(timer_interval*1000);

					mylog(log_debug,"conn_info.timer.get_timer_fd64()=%llu\n",conn_info.timer.get_timer_fd64());

					u64_t timer_fd64=conn_info.timer.get_timer_fd64();
					fd_manager.get_info(timer_fd64).ip_port=ip_port;

					mylog(log_info,"new connection from %s\n",ip_port.to_s());

				}
				conn_info_t &conn_info=conn_manager.find(ip_port);

				conn_info.update_active_time();
				int  out_n;char **out_arr;int *out_len;my_time_t *out_delay;
				from_fec_to_normal(conn_info,data,data_len,out_n,out_arr,out_len,out_delay);

				mylog(log_trace,"out_n= %d\n",out_n);
				for(int i=0;i<out_n;i++)
				{
					u32_t conv;
					char *new_data;
					int new_len;
					if(get_conv(conv,out_arr[i],out_len[i],new_data,new_len)!=0)
					{
						mylog(log_debug,"get_conv failed");
						continue;
					}

					/*
					id_t tmp_conv_id;
					memcpy(&tmp_conv_id,&data_[0],sizeof(tmp_conv_id));
					tmp_conv_id=ntohl(tmp_conv_id);*/

					if (!conn_info.conv_manager.is_conv_used(conv))
					{
						if(conn_info.conv_manager.get_size() >=max_conv_num)
						{
							mylog(log_warn,"ignored new udp connect bc max_conv_num exceed\n");
							continue;
						}

						int new_udp_fd;
						ret=new_connected_socket(new_udp_fd,remote_ip_uint32,remote_port);

						if (ret != 0) {
							mylog(log_warn, "[%s]new_connected_socket failed\n",ip_port.to_s());
							continue;
						}

						fd64_t fd64 = fd_manager.create(new_udp_fd);
						ev.events = EPOLLIN;
						ev.data.u64 = fd64;
						ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_udp_fd, &ev);

						conn_info.conv_manager.insert_conv(conv, fd64);
						fd_manager.get_info(fd64).ip_port=ip_port;


						mylog(log_info,"[%s]new conv %x,fd %d created,fd64=%llu\n",ip_port.to_s(),conv,new_udp_fd,fd64);
						//assert(!conn_manager.exist_fd64(fd64));

						//conn_manager.insert_fd64(fd64,ip_port);
					}
					conn_info.conv_manager.update_active_time(conv);
					fd64_t fd64= conn_info.conv_manager.find_u64_by_conv(conv);
					//int fd=fd_manager.fd64_to_fd(fd64);
					dest_t dest;
					dest.type=type_fd64;
					dest.inner.fd64=fd64;
					//dest.conv=conv;
					delay_send(out_delay[i],dest,new_data,new_len);
				}
			}
		    else if (events[idx].data.u64 == (u64_t)delay_manager.get_timer_fd()) {
				uint64_t value;
				read(delay_manager.get_timer_fd(), &value, 8);
				mylog(log_trace,"events[idx].data.u64 == (u64_t)delay_manager.get_timer_fd()\n");
			}
			else if (events[idx].data.u64 >u32_t(-1))
			{
				char data[buf_len];
				int data_len;
				u32_t conv;
				fd64_t fd64=events[idx].data.u64;
				mylog(log_trace,"events[idx].data.u64 >u32_t(-1),%llu\n",(u64_t)events[idx].data.u64);
				if(!fd_manager.exist(fd64))   //fd64 has been closed
				{
					mylog(log_trace,"!fd_manager.exist(fd64)\n");
					continue;
				}

				assert(fd_manager.exist_info(fd64));
				ip_port_t ip_port=fd_manager.get_info(fd64).ip_port;
				assert(conn_manager.exist(ip_port));

				conn_info_t &conn_info=conn_manager.find(ip_port);
				//conn_info.update_active_time(); //cant put it here

				int  out_n=-2;char **out_arr;int *out_len;my_time_t *out_delay;

				dest_t dest;
				dest.inner.fd_ip_port.fd=local_listen_fd;
				dest.inner.fd_ip_port.ip_port=ip_port;
				dest.type=type_fd_ip_port;
				dest.cook=1;

				if(fd64==conn_info.fec_encode_manager.get_timer_fd64())
				{
					//mylog(log_infol,"timer!!!\n");
					uint64_t value;
					if((ret=read(fd_manager.to_fd(fd64), &value, 8))!=8)
					{
						mylog(log_trace,"fd_manager.to_fd(fd64), &value, 8)!=8 ,%d\n",ret);
						continue;
					}
					if(value==0)
					{
						mylog(log_trace,"value==0\n");
						continue;
					}
					assert(value==1);
					from_normal_to_fec(conn_info,0,0,out_n,out_arr,out_len,out_delay);
				}
				else if(fd64==conn_info.timer.get_timer_fd64())
				{
					uint64_t value;
					read(conn_info.timer.get_timer_fd(), &value, 8);
					conn_info.conv_manager.clear_inactive();
					if(debug_force_flush_fec)
					{
					from_normal_to_fec(conn_info,0,0,out_n,out_arr,out_len,out_delay);
					}

					conn_info.stat.report_as_server(ip_port);
					continue;
				}
				else
				{
					assert(conn_info.conv_manager.is_u64_used(fd64));

					conv=conn_info.conv_manager.find_conv_by_u64(fd64);
					conn_info.conv_manager.update_active_time(conv);
					conn_info.update_active_time();

					int fd=fd_manager.to_fd(fd64);
					data_len=recv(fd,data,max_data_len,0);

					mylog(log_trace,"received a packet from udp_fd,len:%d,conv=%d\n",data_len,conv);

					if(data_len<0)
					{
						mylog(log_debug,"udp fd,recv_len<0 continue,%s\n",strerror(errno));

						continue;
					}

					if(!disable_mtu_warn&&data_len>=mtu_warn)
					{
						mylog(log_warn,"huge packet,data len=%d (>=%d).strongly suggested to set a smaller mtu at upper level,to get rid of this warn\n ",data_len,mtu_warn);
					}

					char * new_data;
					int new_len;
					put_conv(conv,data,data_len,new_data,new_len);

					from_normal_to_fec(conn_info,new_data,new_len,out_n,out_arr,out_len,out_delay);
				}

				mylog(log_trace,"out_n=%d\n",out_n);
				for(int i=0;i<out_n;i++)
				{
					delay_send(out_delay[i],dest,out_arr[i],out_len[i]);
				}
				//mylog(log_trace,"[%s] send packet\n",ip_port.to_s());

			}
			else
			{
				mylog(log_fatal,"unknown fd,this should never happen\n");
				myexit(-1);
			}
		}
		delay_manager.check();
	}
	return 0;
}
