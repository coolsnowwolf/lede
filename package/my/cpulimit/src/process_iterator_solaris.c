/**
 *
 * process_iterator_solaris.c
 * Copyright (C) 2016 by Jim Mason <jmason at ibinx dot com>
 *
 * Adapted from process_iterator_linux.c
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

int init_process_iterator(struct process_iterator *it, struct process_filter *filter) {
	// open a directory stream to /proc directory
	if (!(it->dip = opendir("/proc"))) {
		perror("opendir");
		return -1;
	}
	it->filter = filter;
	return 0;
}

static int read_process_info(pid_t pid, struct process *p) {
	psinfo_t psinfo;  
	char statfile[32];

	p->pid = pid;
	sprintf(statfile, "/proc/%ld/psinfo", (long)pid);
	FILE *fd = fopen(statfile, "r");
	if (!fd) return -1;
	if (!fread(&psinfo, sizeof(psinfo), 1, fd)) {
		fclose(fd);
		return -1;
	}
	fclose(fd);

	p->ppid = psinfo.pr_ppid;
	p->cputime = psinfo.pr_time.tv_sec * 1.0e03 + psinfo.pr_time.tv_nsec / 1.0e06;
	p->starttime = psinfo.pr_start.tv_sec * 1.0e03 + psinfo.pr_start.tv_nsec / 1.0e06;
	strcpy(p->command, psinfo.pr_psargs);

	return 0;
}

static pid_t getppid_of(pid_t pid) {
	psinfo_t psinfo;  
	char statfile[32];

	sprintf(statfile, "/proc/%ld/psinfo", (long)pid);
	FILE *fd = fopen(statfile, "r");
	if (!fd) return -1;
	if (!fread(&psinfo, sizeof(psinfo), 1, fd)) {
		fclose(fd);
		return -1;
	}
	fclose(fd);

	return psinfo.pr_ppid;
}

static int is_child_of(pid_t child_pid, pid_t parent_pid) {
	int ppid = child_pid;
	while(ppid > 1 && ppid != parent_pid)
		ppid = getppid_of(ppid);
	return ppid == parent_pid;
}

int get_next_process(struct process_iterator *it, struct process *p) {
	if (!it->dip) {
		// end of processes
		return -1;
	}

	if (it->filter->pid != 0 && !it->filter->include_children) {
		int ret = read_process_info(it->filter->pid, p);
		closedir(it->dip);
		it->dip = NULL;
		return ret;
	}

	// read in from /proc and seek for process dirs
	struct dirent *dit;
	while ((dit = readdir(it->dip))) {
		p->pid = atoi(dit->d_name);
		if (it->filter->pid != 0 && it->filter->pid != p->pid && !is_child_of(p->pid, it->filter->pid)) continue;
		read_process_info(p->pid, p);
		break;
	}

	if (!dit) {
		// end of processes
		closedir(it->dip);
		it->dip = NULL;
		return -1;
	}

	return 0;
}

int close_process_iterator(struct process_iterator *it) {
	if (it->dip && closedir(it->dip) == -1) {
		perror("closedir");
		return 1;
	}
	it->dip = NULL;
	return 0;
}
