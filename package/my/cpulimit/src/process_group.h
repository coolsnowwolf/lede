/**
 *
 * cpulimit - a CPU limiter for Linux
 *
 * Copyright (C) 2005-2012, by:  Angelo Marletta <angelo dot marletta at gmail dot com>
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

#ifndef __PROCESS_GROUP_H

#define __PROCESS_GROUP_H

#include "process_iterator.h"

#include "list.h"

#define PIDHASH_SZ 1024
#define pid_hashfn(x) ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

struct process_group {
    //hashtable with all the processes (array of struct list of struct process)
    struct list *proctable[PIDHASH_SZ];
    struct list *proclist;
    pid_t target_pid;
    int include_children;
    struct timeval last_update;
};

int init_process_group(struct process_group *pgroup, int target_pid, int include_children);

void update_process_group(struct process_group *pgroup);

int close_process_group(struct process_group *pgroup);

int find_process_by_pid(pid_t pid);

int find_process_by_name(const char *process_name);

int remove_process(struct process_group *pgroup, int pid);

#endif
