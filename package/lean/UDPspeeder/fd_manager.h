/*
 * fd_manager.h
 *
 *  Created on: Sep 25, 2017
 *      Author: root
 */

#ifndef FD_MANAGER_H_
#define FD_MANAGER_H_

#include "common.h"
#include "packet.h"



struct fd_manager_t   //conver fd to a uniq 64bit number,avoid fd value conflict caused by close and re-create
//this class is not strictly necessary,it just makes epoll fd handling easier
{
	fd_info_t & get_info(fd64_t fd64);
	int exist_info(fd64_t);
	int exist(fd64_t fd64);
	int to_fd(fd64_t);
	void fd64_close(fd64_t fd64);
	void reserve(int n);
	u64_t create(int fd);
	fd_manager_t();
private:
	u64_t counter;
	unordered_map<int,fd64_t> fd_to_fd64_mp;
	unordered_map<fd64_t,int> fd64_to_fd_mp;
	unordered_map<fd64_t,fd_info_t> fd_info_mp;
	int fd_exist(int fd);
	//void remove_fd(int fd);
	//fd64_t fd_to_fd64(int fd);
};

extern fd_manager_t fd_manager;
#endif /* FD_MANAGER_H_ */
