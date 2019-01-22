/**
 *
 * cpulimit - a cpu limiter for Linux
 *
 * Copyright (C) 2012, by:  Gang Liu <gangban.lau@gmail.com>
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
 *
 **********************************************************************
 *
 * This is a simple program to limit the cpu usage of a process
 * If you modify this code, send me a copy please
 *
 * Date:    30/8/2008
 * Version: 1.2 beta
 * Get the latest version at: http://cpulimit.sourceforge.net
 *
 * Changelog from 1.1:
 * - reorganization of the code, splitted in more source files
 * - cpu count detection, i.e. if you have 4 cpu, it is possible to limit up to 400%
 * - in order to avoid deadlocks, cpulimit now prevents to limit itself
 * - option --path eliminated, use --exe instead both for absolute path and file name
 * - call setpriority() just once in limit_process()
 * - no more segmentation fault when processes exit
 * - no more memory corruption when processes exit
 * - cpulimit exits if --lazy option is specified and the process terminates
 * - target process can be created on-fly given command line
 * - light and scalable algorithm for subprocesses detection and limitation
 * - mac os support
 * - minor enhancements and bugfixes
 *
 */

#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "process.h"
#include "procutils.h"
#include "list.h"

//some useful macro
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

//control time slot in microseconds
//each slot is splitted in a working slice and a sleeping slice
//TODO: make it adaptive, based on the actual system load
#define TIME_SLOT 100000

#define MAX_PRIORITY -10

/* GLOBAL VARIABLES */

//the "family"
struct process_family pf;
//pid of cpulimit
pid_t cpulimit_pid;
//name of this program (maybe cpulimit...)
char *program_name;

//number of cpu
int NCPU;

/* CONFIGURATION VARIABLES */

//verbose mode
int verbose = 0;
//lazy mode (exits if there is no process)
int lazy = 0;

static void *memrchr(const void *s, int c, size_t n)
{
	const unsigned char *start = (const unsigned char*)s;
	const unsigned char *end = (const unsigned char*)s;

	end+=n-1;

	while(end>=start) {
		if(*end==c)
			return (void *)end;
		else
			end--;
	}

	return NULL;
}

//SIGINT and SIGTERM signal handler
static void quit(int sig)
{
	//let all the processes continue if stopped
	struct list_node *node = NULL;
	for (node=pf.members.first; node!= NULL; node=node->next) {
		struct process *p = (struct process*)(node->data);
		kill(p->pid, SIGCONT);
		process_close(p);
	}
	//free all the memory
	cleanup_process_family(&pf);
	//fix ^C little problem
	printf("\r");
	fflush(stdout);
	exit(0);
}

//return t1-t2 in microseconds (no overflow checks, so better watch out!)
static inline unsigned long timediff(const struct timeval *t1,const struct timeval *t2)
{
	return (t1->tv_sec - t2->tv_sec) * 1000000 + (t1->tv_usec - t2->tv_usec);
}

static void print_usage(FILE *stream, int exit_code)
{
	fprintf(stream, "Usage: %s [OPTIONS...] TARGET\n", program_name);
	fprintf(stream, "   OPTIONS\n");
	fprintf(stream, "      -l, --limit=N          percentage of cpu allowed from 0 to %d (required)\n", 100*NCPU);
	fprintf(stream, "      -v, --verbose          show control statistics\n");
	fprintf(stream, "      -z, --lazy             exit if there is no target process, or if it dies\n");
	fprintf(stream, "      -i, --ignore-children  don't limit children processes\n");
	fprintf(stream, "      -h, --help             display this help and exit\n");
	fprintf(stream, "   TARGET must be exactly one of these:\n");
	fprintf(stream, "      -p, --pid=N            pid of the process (implies -z)\n");
	fprintf(stream, "      -e, --exe=FILE         name of the executable program file or path name\n");
	fprintf(stream, "      COMMAND [ARGS]         run this command and limit it (implies -z)\n");
	fprintf(stream, "\nReport bugs to <marlonx80@hotmail.com>.\n");
	exit(exit_code);
}

static void increase_priority() {
	//find the best available nice value
    errno = 0; // clear errno first
	int old_priority = getpriority(PRIO_PROCESS, 0);
    if (errno)
    {
        perror("getpriority failed");
        return;
    }
	int priority = old_priority;
	while (priority > MAX_PRIORITY && setpriority(PRIO_PROCESS, 0, priority-1) == 0) {
		priority--;	
	}
	if (priority != old_priority) {
		if (verbose) printf("Priority changed to %d\n", priority);
	}
	else {
		if (verbose) printf("Warning: Cannot change priority. Run as root or renice for best results.\n");
	}
}

/* Get the number of CPUs */
static int get_ncpu() {
	int ncpu = -1;
#ifdef _SC_NPROCESSORS_ONLN
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
	int mib[2] = {CTL_HW, HW_NCPU};
	size_t len = sizeof(ncpu);
	sysctl(mib, 2, &ncpu, &len, NULL, 0);
#endif
	return ncpu;
}

#ifdef __linux__

#include <sys/vfs.h>

static int check_proc()
{
	struct statfs mnt;
	if (statfs("/proc", &mnt) < 0)
		return 0;
	if (mnt.f_type!=0x9fa0)
		return 0;
	return 1;
}
#endif

void limit_process(pid_t pid, double limit, int ignore_children)
{
	//slice of the slot in which the process is allowed to run
	struct timespec twork;
	//slice of the slot in which the process is stopped
	struct timespec tsleep;
	//when the last twork has started
	struct timeval startwork;
	//when the last twork has finished
	struct timeval endwork;
	//initialization
	memset(&twork, 0, sizeof(struct timespec));
	memset(&tsleep, 0, sizeof(struct timespec));
	memset(&startwork, 0, sizeof(struct timeval));
	memset(&endwork, 0, sizeof(struct timeval));	
	//last working time in microseconds
	unsigned long workingtime = 0;
	//generic list item
	struct list_node *node;
	//counter
	int c = 0;

	//get a better priority
	//increase_priority();
	
	//build the family
	if (create_process_family(&pf, pid) == -1)
    {
        printf("create process family failed");
        return ;
    }
	if (ignore_children) {
		//delete any process with a different pid than the father
        node = pf.members.first;
		while (node!=NULL) {
			struct process *proc = (struct process*)(node->data);
			if (proc->pid != pid)
            {
				remove_process_from_family(&pf, proc->pid);
                node = pf.members.first;
                continue;
            }
            node = node->next;
		}
	}
	
	if (!ignore_children && verbose) printf("Members in the family owned by %d: %d\n", pf.father, pf.members.count);

	//rate at which we are keeping active the processes (range 0-1)
	//1 means that the process are using all the twork slice
	double workingrate = -1;
	while(1) {
		if (!ignore_children && c%10==0) {
			//update the process family (checks only for new members)
			int new_children = update_process_family(&pf);
			if (verbose && new_children) {
				printf("%d new children processes detected (", new_children);
				int j;
				node = pf.members.last;
				for (j=0; j<new_children; j++) {
					printf("%d", ((struct process*)(node->data))->pid);
					if (j<new_children-1) printf(" ");
					node = node->previous;
				}
				printf(")\n");
			}
		}

		if (pf.members.count==0) {
			if (verbose) printf("No more processes.\n");
			break;
		}
		
		//total cpu actual usage (range 0-1)
		//1 means that the processes are using 100% cpu
		double pcpu = -1;

		//estimate how much the controlled processes are using the cpu in the working interval
        node = pf.members.first;
		while (node!=NULL) {
			struct process *proc = (struct process*)(node->data);
			if (proc->is_zombie) {
				//process is zombie, remove it from family
				fprintf(stderr,"Process %d is zombie!\n", proc->pid);
				remove_process_from_family(&pf, proc->pid);
                node = pf.members.first;
				continue;
			}
			if (process_monitor(proc) != 0) {
				//process is dead, remove it from family
				if (verbose) fprintf(stderr,"Process %d dead!\n", proc->pid);
				remove_process_from_family(&pf, proc->pid);
                node = pf.members.first;
				continue;
			}
            node = node->next;
			if (proc->cpu_usage<0) {
				continue;
			}
			if (pcpu<0) pcpu = 0;
			pcpu += proc->cpu_usage;
		}

		//adjust work and sleep time slices
		if (pcpu < 0) {
			//it's the 1st cycle, initialize workingrate
			pcpu = limit;
			workingrate = limit;
			twork.tv_nsec = TIME_SLOT*limit*1000;
		}
		else {
			//adjust workingrate
			workingrate = MIN(workingrate / pcpu * limit, 1);
			twork.tv_nsec = TIME_SLOT*1000*workingrate;
		}
		tsleep.tv_nsec = TIME_SLOT*1000-twork.tv_nsec;

		if (verbose) {
			if (c%200==0)
				printf("\n%%CPU\twork quantum\tsleep quantum\tactive rate\n");
			if (c%10==0 && c>0)
				printf("%0.2lf%%\t%6ld us\t%6ld us\t%0.2lf%%\n",pcpu*100,twork.tv_nsec/1000,tsleep.tv_nsec/1000,workingrate*100);
		}

		//resume processes
        node = pf.members.first;
		while (node!=NULL) {
			struct process *proc = (struct process*)(node->data);
			if (kill(proc->pid,SIGCONT)!=0) {
				//process is dead, remove it from family
				if (verbose) fprintf(stderr,"Process %d dead!\n", proc->pid);
				remove_process_from_family(&pf, proc->pid);
                node = pf.members.first;
                continue;
			}
            node=node->next;
		}

		//now processes are free to run (same working slice for all)
		gettimeofday(&startwork, NULL);
		nanosleep(&twork,NULL);
		gettimeofday(&endwork, NULL);
		workingtime = timediff(&endwork,&startwork);
		
		long delay = workingtime-twork.tv_nsec/1000;
		if (c>0 && delay>10000) {
			//delay is too much! signal to user?
			//fprintf(stderr, "%d %ld us\n", c, delay);
		}

		if (tsleep.tv_nsec>0) {
			//stop only if tsleep>0, instead it's useless
            node=pf.members.first;
			while (node!=NULL) {
				struct process *proc = (struct process*)(node->data);
				if (kill(proc->pid,SIGSTOP)!=0) {
					//process is dead, remove it from family
					if (verbose) fprintf(stderr,"Process %d dead!\n", proc->pid);
					remove_process_from_family(&pf, proc->pid);
                    node=pf.members.first;
                    continue;
				}
                node=node->next;
			}
			//now the processes are sleeping
			nanosleep(&tsleep,NULL);
		}
		c++;
	}
	cleanup_process_family(&pf);
}

int main(int argc, char **argv) {
	//argument variables
	const char *exe = NULL;
	int perclimit = 0;
	int exe_ok = 0;
	int pid_ok = 0;
	int limit_ok = 0;
	pid_t pid = 0;
	int ignore_children = 0;

	//get program name
	char *p=(char*)memrchr(argv[0],(unsigned int)'/',strlen(argv[0]));
	program_name = p==NULL?argv[0]:(p+1);
	//get current pid
	cpulimit_pid = getpid();
	//get cpu count
	NCPU = get_ncpu();

	//parse arguments
	int next_option;
    int option_index = 0;
	//A string listing valid short options letters
	const char* short_options = "+p:e:l:vzih";
	//An array describing valid long options
	const struct option long_options[] = {
		{ "pid",        required_argument, NULL, 'p' },
		{ "exe",        required_argument, NULL, 'e' },
		{ "limit",      required_argument, NULL, 'l' },
		{ "verbose",    no_argument,       NULL, 'v' },
		{ "lazy",       no_argument,       NULL, 'z' },
		{ "ignore-children", no_argument,  NULL, 'i' },
		{ "help",       no_argument,       NULL, 'h' },
		{ 0,            0,                 0,     0  }
	};

	do {
		next_option = getopt_long(argc, argv, short_options,long_options, &option_index);
		switch(next_option) {
			case 'p':
				pid = atoi(optarg);
				pid_ok = 1;
				break;
			case 'e':
				exe = optarg;
				exe_ok = 1;
				break;
			case 'l':
				perclimit = atoi(optarg);
				limit_ok = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'z':
				lazy = 1;
				break;
			case 'i':
				ignore_children = 1;
				break;
			case 'h':
				print_usage(stdout, 1);
				break;
			case '?':
				print_usage(stderr, 1);
				break;
			case -1:
				break;
			default:
				abort();
		}
	} while(next_option != -1);

	if (pid_ok && (pid<=1 || pid>=65536)) {
		fprintf(stderr,"Error: Invalid value for argument PID\n");
		print_usage(stderr, 1);
		exit(1);
	}
	if (pid!=0) {
		lazy = 1;
	}

	if (!limit_ok) {
		fprintf(stderr,"Error: You must specify a cpu limit percentage\n");
		print_usage(stderr, 1);
		exit(1);
	}
	double limit = perclimit/100.0;
	if (limit<0 || limit >NCPU) {
		fprintf(stderr,"Error: limit must be in the range 0-%d00\n", NCPU);
		print_usage(stderr, 1);
		exit(1);
	}

	int command_mode = optind<argc;
	if (exe_ok + pid_ok + command_mode == 0) {
		fprintf(stderr,"Error: You must specify one target process, either by name, pid, or command line\n");
		print_usage(stderr, 1);
		exit(1);
	}
	
	if (exe_ok + pid_ok + command_mode > 1) {
		fprintf(stderr,"Error: You must specify exactly one target process, either by name, pid, or command line\n");
		print_usage(stderr, 1);
		exit(1);
	}

	//all arguments are ok!
	signal(SIGINT, quit);
	signal(SIGTERM, quit);

	//print the number of available cpu
	if (verbose) printf("%d cpu detected\n", NCPU);

#ifdef __linux__
	if (!check_proc()) {
		fprintf(stderr, "procfs is not mounted!\nAborting\n");
		exit(-2);
	}
#endif

	if (command_mode) {
		int i;
		//executable file
		const char *cmd = argv[optind];
		//command line arguments
		char **cmd_args = (char**)malloc((argc-optind+1)*sizeof(char*));
		if (cmd_args==NULL) exit(2);
		for (i=0; i<argc-optind; i++) {
			cmd_args[i] = argv[i+optind];
		}
		cmd_args[i] = NULL;

		if (verbose) {
			printf("Running command: '%s", cmd);
			for (i=1; i<argc-optind; i++) {
				printf(" %s", cmd_args[i]);
			}
			printf("'\n");
		}
		
		int child = fork();
		if (child < 0) {
			exit(EXIT_FAILURE);
		}
		else if (child > 0) {
			//parent code
			int limiter = fork();
			if (limiter < 0) {
				exit(EXIT_FAILURE);
			}
			else if (limiter > 0) {
				//parent
				int status_process;
				int status_limiter;
				waitpid(child, &status_process, 0);
				waitpid(limiter, &status_limiter, 0);
				if (WIFEXITED(status_process)) {
					if (verbose) printf("Process %d terminated with exit status %d\n", child, (int)WEXITSTATUS(status_process));
					exit(WEXITSTATUS(status_process));
				}
				printf("Process %d terminated abnormally\n", child);
				exit(status_process);
			}
			else {
				//limiter code
				if (verbose) printf("Limiting process %d\n",child);
				limit_process(child, limit, ignore_children);
				exit(0);
			}
		}
		else {
			//target process code
			int ret = execvp(cmd, cmd_args);
			//if we are here there was an error, show it
			perror("Error");
			exit(ret);
		}
	}

	while(1) {
		//look for the target process..or wait for it
		pid_t ret = 0;
		if (pid_ok) {
			//search by pid
			ret = look_for_process_by_pid(pid);
			if (ret == 0) {
				printf("No process found\n");
			}
			else if (ret < 0) {
				printf("Process found but you aren't allowed to control it\n");
			}
		}
		else {
			//search by file or path name
			ret = look_for_process_by_name(exe);
			if (ret == 0) {
				printf("No process found\n");
			}
			else if (ret < 0) {
				printf("Process found but you aren't allowed to control it\n");
			}
			else {
				pid = ret;
			}
		}
		if (ret > 0) {
			if (ret == cpulimit_pid) {
				printf("Process %d is cpulimit itself! Aborting to avoid deadlock\n", ret);
				exit(1);
			}
			printf("Process %d found\n", pid);
			//control
			limit_process(pid, limit, ignore_children);
		}
		if (lazy) break;
		sleep(2);
	};
	
	exit(0);
}
