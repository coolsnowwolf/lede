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
 *
 * Author: Simon Sigurdhsson
 *
 */

#include <errno.h>
#include <stdio.h>
#include <libproc.h>

int unique_nonzero_ints(int *arr_in, int len_in, int *arr_out) {
    int *source = arr_in;
    if (arr_out == NULL) {
        return -1;
    }
    if (arr_in == arr_out) {
        source = malloc(sizeof(int)*len_in);
        memcpy(source, arr_in, sizeof(int)*len_in);
        memset(arr_out, -1, sizeof(int)*len_in);
    }
    int len_out = 0;
    int i, j;
    for (i=0; i<len_in; i++) {
        int found = 0;
        if (source[i] == 0) {
            continue;
        }
        for (j=0; !found && j<len_out; j++) {
            found = (source[i] == arr_out[j]) ? 1 : 0;
        }
        if (!found) {
            arr_out[len_out++] = source[i];
        }
    }
    if (arr_in == arr_out) {
        free(source);
    }
    return len_out-1;
}

int init_process_iterator(struct process_iterator *it, struct process_filter *filter) {
    it->i = 0;
    /* Find out how much to allocate for it->pidlist */
    if ((it->count = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0)) <= 0) {
        fprintf(stderr, "proc_listpids: %s\n", strerror(errno));
        return -1;
    }
    /* Allocate and populate it->pidlist */
    if ((it->pidlist = (int *)malloc((it->count)*sizeof(int))) == NULL) {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
    }
    if ((it->count = proc_listpids(PROC_ALL_PIDS, 0, it->pidlist, it->count)) <= 0) {
        fprintf(stderr, "proc_listpids: %s\n", strerror(errno));
        return -1;
    }
    it->count = unique_nonzero_ints(it->pidlist, it->count, it->pidlist);
    it->filter = filter;
    return 0;
}

static int pti2proc(struct proc_taskallinfo *ti, struct process *process) {
    int bytes;
    process->pid = ti->pbsd.pbi_pid;
    process->ppid = ti->pbsd.pbi_ppid;
    process->starttime = ti->pbsd.pbi_start_tvsec;
    process->cputime = (ti->ptinfo.pti_total_user + ti->ptinfo.pti_total_system) / 1000000;
    bytes = strlen(ti->pbsd.pbi_comm);
    memcpy(process->command, ti->pbsd.pbi_comm, (bytes < PATH_MAX ? bytes : PATH_MAX) + 1);
    return 0;
}

static int get_process_pti(pid_t pid, struct proc_taskallinfo *ti) {
    int bytes;
    bytes = proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, ti, sizeof(*ti));
    if (bytes <= 0) {
        if (!(errno & (EPERM | ESRCH))) {
            fprintf(stderr, "proc_pidinfo: %s\n", strerror(errno));
        }
        return -1;
    } else if (bytes < sizeof(ti)) {
        fprintf(stderr, "proc_pidinfo: too few bytes; expected %ld, got %d\n", sizeof(ti), bytes);
        return -1;
    }
    return 0;
}

int get_next_process(struct process_iterator *it, struct process *p) {
    if (it->i == it->count) {
        return -1;
    }
    if (it->filter->pid != 0 && !it->filter->include_children) {
        struct proc_taskallinfo ti;
        if (get_process_pti(it->filter->pid, &ti) != 0) {
            it->i = it->count = 0;
            return -1;
        }
        it->i = it->count = 1;
        return pti2proc(&ti, p);
    }
    while (it->i < it->count) {
        struct proc_taskallinfo ti;
        if (get_process_pti(it->pidlist[it->i], &ti) != 0) {
            it->i++;
            continue;
        }
        if (ti.pbsd.pbi_flags & PROC_FLAG_SYSTEM) {
            it->i++;
            continue;
        }
        if (it->filter->pid != 0 && it->filter->include_children) {
            pti2proc(&ti, p);
            it->i++;
            if (p->pid != it->pidlist[it->i - 1]) { // I don't know why this can happen
                continue;
            }
            if (p->pid != it->filter->pid && p->ppid != it->filter->pid) {
                continue;
            }
            return 0;
        } else if (it->filter->pid == 0) {
            pti2proc(&ti, p);
            it->i++;
            return 0;
        }
    }
    return -1;
}

int close_process_iterator(struct process_iterator *it) {
    free(it->pidlist);
    it->pidlist = NULL;
    it->filter = NULL;
    it->count = 0;
    it->i = 0;
    return 0;
}
