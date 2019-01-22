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

#ifndef __PROCUTILS_H

#define __PROCUTILS_H

#include <sys/types.h>
#include <dirent.h>

#include "list.h"
#include "process.h"

#define PIDHASH_SZ 1024
#define pid_hashfn(x) ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

// a hierarchy of processes
struct process_family {
	//the (god)father of the process family
	pid_t father;
	//process list (father process is the first element)
	//elements are struct process
	struct list members;
	//non-members list
	//hashtable with all the processes (array of struct list of struct process)
	struct list *proctable[PIDHASH_SZ];
	//total process count
	int count;
};

//TODO: use this object in proctable and delete member in struct process
struct table_item {
	struct process *proc;
	//1 if the process is a member of the family
	int member;
};

// object to enumerate running processes
struct process_iterator {
#ifdef __linux__
	DIR *dip;
	struct dirent *dit;
#elif defined __APPLE__
	struct kinfo_proc *procList;
	int count;
	int c;
#elif defined __hpux
	int count;
#endif
	struct process *current;
};

// searches for all the processes derived from father and stores them
// in the process family struct
int create_process_family(struct process_family *f, pid_t father);

// checks if there are new processes born in the specified family
// if any they are added to the members list
// the number of new born processes is returned
int update_process_family(struct process_family *f);

// removes a process from the family by its pid
void remove_process_from_family(struct process_family *f, pid_t pid);

// free the heap memory used by a process family
void cleanup_process_family(struct process_family *f);

// searches a process given the name of the executable file, or its absolute path
// returns the pid, or 0 if it's not found
int look_for_process_by_name(const char *process_name);

// searches a process given its pid
// returns the pid, or 0 if it's not found
int look_for_process_by_pid(pid_t pid);

#endif
