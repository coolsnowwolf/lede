/*  libiptbwctl --	A userspace library for querying the bandwidth iptables module
 *  			Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2009 by Eric Bishop <eric@gargoyle-router.com>
 *
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/sem.h> 
#include <sys/time.h>
#include <sys/syscall.h>
#define BANDWIDTH_QUERY_LENGTH		16384

/* socket id parameters (for userspace i/o) */
#define BANDWIDTH_SET 			2048
#define BANDWIDTH_GET 			2049


/* max id length */
#define BANDWIDTH_MAX_ID_LENGTH		  50

/* pick something rather random... let's make it end in 666 to
 * freak out the crazy fundies out there ;-) */
#define BANDWIDTH_SEMAPHORE_KEY 12699666

/* possible reset intervals */
#define BANDWIDTH_MINUTE		  80
#define BANDWIDTH_HOUR			  81
#define BANDWIDTH_DAY			  82
#define BANDWIDTH_WEEK			  83
#define BANDWIDTH_MONTH			  84
#define BANDWIDTH_NEVER			  85


#pragma pack(push, 1)
typedef struct ip_bw_struct
{
	uint32_t ip;
	uint64_t bw;
}ip_bw;

/*
* format of response:
* byte 1 : error code (0 for ok)
* bytes 2-5 : total_num_ips found in query (further gets may be necessary to retrieve them)
* bytes 6-9 : start_index, index (in a list of total_num_ips) of first ip in response
* bytes 10-13 : num_ips_in_response, number of ips in this response
* bytes 14-21 : reset_interval (helps deal with DST shifts in userspace)
* bytes 22-29 : reset_time (helps deal with DST shifts in userspace)
* byte  30    : reset_is_constant_interval (helps deal with DST shifts in userspace)
* remaining bytes contain blocks of ip data
* format is dependent on whether history was queried
*/
typedef struct ip_bw_kernel_data_item_struct
{
    uint32_t ip;
    uint32_t num_nodes;
    uint64_t first_start;
    uint64_t first_end;
    uint64_t last_end;
    uint64_t ipbw_data[0];
}ip_bw_kernel_data_item;

typedef struct 
{
    uint8_t error;
    uint32_t ip_total;
    uint32_t index_start;
    uint32_t ip_num;
    uint64_t reset_interval;
    uint64_t reset_time;
    uint8_t reset_is_constant_interval;
    /*payload for history ip bw data*/
    ip_bw_kernel_data_item data_item[0];
}ip_bw_kernel_data;

typedef struct history_struct
{
	uint32_t ip;
	uint32_t num_nodes;

	time_t reset_interval;
	time_t reset_time;
	unsigned char is_constant_interval;

	time_t first_start;
	time_t first_end;
	time_t last_end;

	uint64_t* history_bws;
} ip_bw_history;
#pragma pack(pop)

time_t* get_interval_starts_for_history(ip_bw_history history);

extern void free_ip_bw_histories(ip_bw_history* histories, int num_histories);

extern int get_all_bandwidth_history_for_rule_id(char* id, unsigned long* num_ips, ip_bw_history** data, unsigned long max_wait_milliseconds);
extern int get_ip_bandwidth_history_for_rule_id(char* id, char* ip, ip_bw_history** data, unsigned long max_wait_milliseconds);
extern int get_all_bandwidth_usage_for_rule_id(char* id, unsigned long* num_ips, ip_bw** data, unsigned long max_wait_milliseconds);
extern int get_ip_bandwidth_usage_for_rule_id(char* id,  char* ip, ip_bw** data, unsigned long max_wait_milliseconds);



extern int set_bandwidth_history_for_rule_id(char* id, unsigned char zero_unset, unsigned long num_ips, ip_bw_history* data, unsigned long max_wait_milliseconds);
extern int set_bandwidth_usage_for_rule_id(char* id, unsigned char zero_unset, unsigned long num_ips, time_t last_backup, ip_bw* data, unsigned long max_wait_milliseconds);



extern int save_usage_to_file(ip_bw* data, unsigned long num_ips, char* out_file_path);
extern int save_history_to_file(ip_bw_history* data, unsigned long num_ips, char* out_file_path);



extern ip_bw* load_usage_from_file(char* in_file_path, unsigned long* num_ips, time_t* last_backup);
extern ip_bw_history* load_history_from_file(char* in_file_path, unsigned long* num_ips);

extern void print_usage(FILE* out, ip_bw* usage, unsigned long num_ips);
extern void print_histories(FILE* out, char* id, ip_bw_history* histories, unsigned long num_histories, char output_type);




extern void unlock_bandwidth_semaphore(void);
extern void unlock_bandwidth_semaphore_on_exit(void);


/* sets kernel timezone minuteswest to match user timezone */
extern int get_minutes_west(time_t now);
extern void set_kernel_timezone(void);


/* safe malloc & strdup functions used to handle malloc errors cleanly */
extern void* ipt_bwctl_safe_malloc(size_t size);
extern char* ipt_bwctl_safe_strdup(const char* str);
