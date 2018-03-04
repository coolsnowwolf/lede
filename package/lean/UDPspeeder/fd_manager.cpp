/*
 * fd_manager.cpp
 *
 *  Created on: Sep 25, 2017
 *      Author: root
 */


#include "fd_manager.h"
int fd_manager_t::fd_exist(int fd)
{
	return fd_to_fd64_mp.find(fd)!=fd_to_fd64_mp.end();
}
int fd_manager_t::exist(fd64_t fd64)
{
	return fd64_to_fd_mp.find(fd64)!=fd64_to_fd_mp.end();
}
int fd_manager_t::to_fd(fd64_t fd64)
{
	assert(exist(fd64));
	return fd64_to_fd_mp[fd64];
}
void fd_manager_t::fd64_close(fd64_t fd64)
{
	assert(exist(fd64));
	int fd=fd64_to_fd_mp[fd64];
	fd64_to_fd_mp.erase(fd64);
	fd_to_fd64_mp.erase(fd);
	if(exist_info(fd64))
	{
		fd_info_mp.erase(fd64);
	}
	close(fd);
}
void fd_manager_t::reserve(int n)
{
	fd_to_fd64_mp.reserve(n);
	fd64_to_fd_mp.reserve(n);
	fd_info_mp.reserve(n);
}
u64_t fd_manager_t::create(int fd)
{
	assert(!fd_exist(fd));
	fd64_t fd64=counter++;
	fd_to_fd64_mp[fd]=fd64;
	fd64_to_fd_mp[fd64]=fd;
	return fd64;
}
fd_manager_t::fd_manager_t()
{
	counter=u32_t(-1);
	counter+=100;
	reserve(10007);
}
fd_info_t & fd_manager_t::get_info(fd64_t fd64)
{
	assert(exist(fd64));
	return fd_info_mp[fd64];
}
int fd_manager_t::exist_info(fd64_t fd64)
{
	return fd_info_mp.find(fd64)!=fd_info_mp.end();
}
