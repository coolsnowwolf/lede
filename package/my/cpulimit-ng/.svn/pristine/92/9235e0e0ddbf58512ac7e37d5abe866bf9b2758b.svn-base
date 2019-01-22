/**
 *
 * cpulimit - a cpu limiter for Linux
 *
 * Copyright (C) 2005-2008, by:  Angelo Marletta <marlonx80@hotmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __PROCESS_H

#define __PROCESS_H

#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

//USER_HZ detection, from openssl code
#ifndef HZ
# if defined(_SC_CLK_TCK) \
     && (!defined(OPENSSL_SYS_VMS) || __CTRL_VER >= 70000000)
#  define HZ ((double)sysconf(_SC_CLK_TCK))
# else
#  ifndef CLK_TCK
#   ifndef _BSD_CLK_TCK_ /* FreeBSD hack */
#    define HZ  100.0
#   else /* _BSD_CLK_TCK_ */
#    define HZ ((double)_BSD_CLK_TCK_)
#   endif
#  else /* CLK_TCK */
#   define HZ ((double)CLK_TCK)
#  endif
# endif
#endif

// process descriptor
struct process {
	//pid of the process
	pid_t pid;
	//pid of the process
	pid_t ppid;
	//start time
	int starttime;
	//is member of the family?
	int member; //TODO: delete this field
	//total number of jiffies used by the process at time last_sample
	int last_jiffies;
	//timestamp when last_jiffies and cpu_usage was calculated
	struct timeval last_sample;
	//actual cpu usage estimation (value in range 0-1)
	double cpu_usage;
	//1 if the process is zombie
	int is_zombie;
	//absolute path of the executable file
	char command[PATH_MAX+1];

	//system-dependent members
//TODO: delete these members for the sake of portability?
#ifdef __linux__
	//preallocate buffers for performance
	//name of /proc/PID/stat file
	char stat_file[20];
	//read buffer for /proc filesystem
	char buffer[1024];
#endif	
};

int get_proc_info(struct process *p, pid_t pid);

int process_init(struct process *proc, pid_t pid);

int process_monitor(struct process *proc);

int process_close(struct process *proc);

#endif
