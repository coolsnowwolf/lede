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


#include "ipt_bwctl.h"
#define malloc ipt_bwctl_safe_malloc
#define strdup ipt_bwctl_safe_strdup


static int bandwidth_semaphore = -1;

union semun 
{
	int val; // Value for SETVAL 
	struct semid_ds *buf; // Buffer for IPC_STAT, IPC_SET 
	unsigned short *array; // Array for GETALL, SETALL 
	struct seminfo *__buf; // Buffer for IPC_INFO (Linux specific) 
};




/* semaphore functions */
static int get_sem_val(int sid, int member);
static int get_sem(int *sid, key_t key);
static int lock_sem(int sid);
static int unlock_sem(int sid);
static int lock(unsigned long max_wait_milliseconds);
static int unlock(void);


/* needed to calculate history time intervals */
static time_t get_next_node_start_time(		time_t current_start_time, 
						time_t reset_interval, 
						time_t reset_time, 
						unsigned char is_constant_interval
						);

/* functions used to get data from kernel module */
static void parse_returned_ip_data(		void *out_data, 
						uint32_t* out_index, 
						unsigned char* in_buffer, 
						uint32_t* in_index, 
						unsigned char get_history, 
						time_t reset_interval, 
						time_t reset_time, 
						unsigned char is_constant_interval
						);

static int get_bandwidth_data(			char* id, 
						unsigned char get_history, 
						char* ip, 
						unsigned long* num_ips, 
						void** data, 
						unsigned long max_wait_milliseconds
						);


/* functions used to send/restore data to kernel module */
static int set_ip_block(			void* ip_block_data, 
						unsigned char is_history, 
						unsigned char* output_buffer, 
						uint32_t* current_output_index, 
						uint32_t output_buffer_length
						);

static int set_bandwidth_data(			char* id, 
						unsigned char zero_unset, 
						unsigned char set_history, 
						unsigned long num_ips, 
						time_t last_backup, 
						void* data, 
						unsigned long max_wait_milliseconds
						);

/* utility i/o functions when saving/restoring data to/from file */
static unsigned char* read_entire_file(		FILE* in, 
						unsigned long read_block_size, 
						unsigned long *length
						);

static char** split_on_separators(		char* line, 
						char* separators, 
						int num_separators, 
						int max_pieces, 
						int include_remainder_at_max, 
						unsigned long *pieces_read
						);



static int get_sem_val(int sid, int member)
{
        int semval;
        semval = semctl(sid, member, GETVAL, 0);
        return(semval);
}

static int get_sem(int *sid, key_t key)
{
        int cntr;
        union semun semopts;
	int members = 1;

	
	int success = ((*sid = semget(key, members, IPC_CREAT|IPC_EXCL|0777))== -1) ? 0 : 1;
	if(success)
	{
		semopts.val = 1;
        	/* Initialize all members (could be done with SETALL) */        
        	for(cntr=0; cntr<members; cntr++)
		{
			semctl(*sid, cntr, SETVAL, semopts);
		}
	}
	else
	{
		success = ((*sid = semget(key, members, 0777)) == -1) ? 0 : 1;
	}
	return success;
}


static int lock_sem(int sid)
{
	int member = 0;
        struct sembuf sem_lock={ 0, -1, IPC_NOWAIT};
	int success = 0;
	
	//printf("locking sem, member count = %d\n", get_sem_member_count(sid)  );

       	/* Attempt to lock the semaphore set */
	int semval = get_sem_val(sid, member);
	if(semval > 0)
       	{
       		sem_lock.sem_num = member;
       		if((semop(sid, &sem_lock, 1)) != -1)
		{
			success = 1;
		}
	}

	return success;
}

static int unlock_sem(int sid)
{
	int member = 0;
        struct sembuf sem_unlock={ member, 1, IPC_NOWAIT};
	
	/* will fail if we can't can't unlock semaphore for some reason, 
	 * will NOT fail if semaphore is already unlocked
	 */
	int success = 1; 


	/* Is the semaphore set locked? */
	int semval = get_sem_val(sid, member);
	if(semval == 0)
	{
		/* it's locked, unlock it */
		sem_unlock.sem_num = member;
        	success = ((semop(sid, &sem_unlock, 1)) == -1) ? 0 : 1;
	}
	return success;
}



static int lock(unsigned long max_wait_milliseconds)
{
	int locked = 0;
	if(bandwidth_semaphore == -1)
	{
		get_sem(&bandwidth_semaphore, (key_t)(BANDWIDTH_SEMAPHORE_KEY) );
	}
	if(bandwidth_semaphore != -1)
	{
		do
		{
			locked = lock_sem(bandwidth_semaphore);
			if(locked == 0 && max_wait_milliseconds > 25)
			{
				usleep(1000*25);
			}
			max_wait_milliseconds = max_wait_milliseconds > 25 ? max_wait_milliseconds - 25 : 0;
		} while(locked == 0 && max_wait_milliseconds > 0);
	}
	return locked;
}

static int unlock(void)
{
	int unlocked = 0;
	if(bandwidth_semaphore == -1)
	{
		get_sem(&bandwidth_semaphore, (key_t)(BANDWIDTH_SEMAPHORE_KEY) );
	}
	if(bandwidth_semaphore != -1)
	{
		unlocked = unlock_sem(bandwidth_semaphore);
	}
	return unlocked;
	
}


static time_t get_next_node_start_time(		time_t current_start_time, 
						time_t reset_interval, 
						time_t reset_time, 
						unsigned char is_constant_interval
						)
{
	time_t next = current_start_time;
	if(is_constant_interval)
	{
		next = current_start_time + reset_interval;
	}
	else
	{
		while(next + reset_time <=  current_start_time)
		{
			struct tm* curr = localtime(&current_start_time);
			curr->tm_isdst = -1;
			if(reset_interval == BANDWIDTH_MINUTE)
			{
				curr->tm_sec = 0;
				curr->tm_min = curr->tm_min+1;
				next = mktime(curr);
			}
			else if(reset_interval == BANDWIDTH_HOUR)
			{
				curr->tm_sec  = 0;
				curr->tm_min  = 0;
				curr->tm_hour = curr->tm_hour+1;
				next = mktime(curr);
			}
			else if(reset_interval == BANDWIDTH_DAY)
			{
				curr->tm_sec  = 0;
				curr->tm_min  = 0;
				curr->tm_hour = 0;
				curr->tm_mday = curr->tm_mday+1;
				next = mktime(curr);
			}
			else if(reset_interval == BANDWIDTH_WEEK)
			{
				curr->tm_sec  = 0;
				curr->tm_min  = 0;
				curr->tm_hour = 0;
				curr->tm_mday = curr->tm_mday+1;
				time_t tmp = mktime(curr);
				curr = localtime(&tmp);
				while(curr->tm_wday != 0)
				{
					curr->tm_mday=curr->tm_mday+1;
					tmp = mktime(curr);
					curr = localtime(&tmp);
				}
				next = mktime(curr);
			}
			else if(reset_interval == BANDWIDTH_MONTH)
			{
				curr->tm_sec  = 0;
				curr->tm_min  = 0;
				curr->tm_hour = 0;
				curr->tm_mday = 1;
				curr->tm_mon  = curr->tm_mon+1;
				next = mktime(curr);
			}
		}
		next = next + reset_time; 
	}
	return next;
}

static void parse_returned_ip_data(	void *out_data, 
					uint32_t* out_index, 
					unsigned char* in_buffer, 
					uint32_t* in_index, 
					unsigned char get_history, 
					time_t reset_interval, 
					time_t reset_time, 
					unsigned char is_constant_interval
					)
{
	uint32_t ip = *( (uint32_t*)(in_buffer + *in_index) );
    ip_bw_kernel_data_item* ip_bw_data = (ip_bw_kernel_data*)(in_buffer + *in_index);
	if(get_history == 0)
	{
		(((ip_bw*)out_data)[*out_index]).ip = ip;
		*in_index = *in_index + 4;
		(((ip_bw*)out_data)[*out_index]).bw = *( (uint64_t*)(in_buffer + *in_index) );
		*in_index = *in_index + 8;
	}
	else
	{
			ip_bw_history *history = ((ip_bw_history*)out_data) + *out_index;
			history->reset_interval = reset_interval;
			history->reset_time = reset_time;
			history->is_constant_interval = is_constant_interval;

            history->ip = ip_bw_data->ip;
			history->num_nodes = ip_bw_data->num_nodes;
			history->first_start = ip_bw_data->first_start;
			history->first_end   = ip_bw_data->first_end;
			history->last_end    = ip_bw_data->last_end;

			history->history_bws =  (uint64_t*)malloc( (history->num_nodes+1)*sizeof(uint64_t) );
			
			/* read bws */
			int node_index = 0;
            *in_index += 32;
            for (node_index = 0; node_index < history->num_nodes; node_index++)
			{
                *in_index += 8;
				(history->history_bws)[node_index] =  ip_bw_data->ipbw_data[node_index];
			}


			/* 
			 * We now need to deal with DST
			 *
			 * The problem is that the kernel can't tell the difference
			 * between timezones being switched and entering daylight savings
			 * time.  Whenever the time offset from UTC shifts, the kernel module
			 * shifts values in the bandwidth history to reflect the time
			 * as it would be if the current offset from UTC had always been
			 * in effect.  So, we need to go backwards through the history and
			 * anytime we go from DST to non-DST (or visa-versa) implement a 
			 * shift so that returned times reflect reality.
			 */
			time_t now;
			time(&now);
			int current_minutes_west = get_minutes_west(now);
			history->first_start = history->first_start + (60*(get_minutes_west(history->first_start)-current_minutes_west));
			history->first_end = history->first_end + (60*(get_minutes_west(history->first_end)-current_minutes_west));
			history->last_end = history->last_end + (60*(get_minutes_west(history->last_end)-current_minutes_west));
	}
	*out_index = *out_index + 1;
}


static int get_bandwidth_data(char* id, unsigned char get_history, char* ip, unsigned long* num_ips, void** data, unsigned long max_wait_milliseconds)
{	
	
	unsigned char buf[BANDWIDTH_QUERY_LENGTH];
	memset(buf, '\0',  BANDWIDTH_QUERY_LENGTH);
	int done = 0;
	ip_bw_kernel_data* ip_bw_data = (ip_bw_kernel_data*)(buf);

	*data = NULL;
	*num_ips = 0;


	int got_lock = lock(max_wait_milliseconds);
	int sockfd = -1;
	if(got_lock)
	{
		sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	}


	uint32_t* request_ip = (uint32_t*)buf;
	uint32_t* request_index = (uint32_t*)(buf + 4);
	unsigned char* request_history =(unsigned char*)(buf + 8);
	char* request_id = (char*)(buf+9);

	if(strcmp(ip, "ALL") == 0)
	{
		*request_ip = 0;
	}
	else
	{
		struct in_addr addr;
		inet_aton(ip, &addr);
		*request_ip = (uint32_t)addr.s_addr;
	}
	*request_index = 0;
	*request_history = get_history;
	sprintf(request_id, "%s", id);

	unsigned char error = 0;
	unsigned char data_initialized = 0;
	uint32_t data_index = 0;
	uint32_t next_request_index = 0;

	while(!done && sockfd >= 0 && got_lock)
	{
		uint32_t size = BANDWIDTH_QUERY_LENGTH;
		getsockopt(sockfd, IPPROTO_IP, BANDWIDTH_GET, buf, &size);
		error = (unsigned char)buf[0];
		if(error != 0)
		{
			done = 1;
		}
		else
		{
			uint32_t total_ips                 = ip_bw_data->ip_total;
			/*uint32_t next_ip_index             = *( (uint32_t*)(buf+5) ); //unused */
			uint32_t response_ips              = ip_bw_data->ip_num;
			time_t reset_interval              = ip_bw_data->reset_interval;
			time_t reset_time                  = ip_bw_data->reset_time;
			unsigned char is_constant_interval = ip_bw_data->reset_is_constant_interval;
			
			if(!data_initialized)
			{
				*num_ips = total_ips;
				if(get_history)
				{
					*data = (void*)malloc(sizeof(ip_bw_history)*(total_ips+1));
					memset(*data, 0, sizeof(ip_bw_history)*(total_ips+1));
				}
				else
				{
					*data = (void*)malloc(sizeof(ip_bw)*(total_ips+1));
					memset(*data, 0, sizeof(ip_bw)*(total_ips+1));
				}
				data_initialized = 1;
			}

			int response_index=0;
			uint32_t buffer_index = 30;
			for(response_index=0; response_index < response_ips; response_index++)
			{
				parse_returned_ip_data(*data, &data_index, buf, &buffer_index, get_history, reset_interval, reset_time, is_constant_interval);
			}
			next_request_index = next_request_index + response_ips;
			done = next_request_index < total_ips ? 0 : 1;
			if(!done)
			{
				memset(buf, '\0',  BANDWIDTH_QUERY_LENGTH);

				if(strcmp(ip, "ALL") == 0)
				{
					*request_ip = 0;
				}
				else
				{
					struct in_addr addr;
					inet_aton(ip, &addr);
					*request_ip = (uint32_t)addr.s_addr;
				}
				*request_index = next_request_index;
				*request_history = get_history;
				sprintf(request_id, "%s", id);
			}
		}
	}
	if( (error != 0) && data_initialized)
	{
		if(get_history)
		{
			free_ip_bw_histories(  (ip_bw_history*)(*data), *num_ips );
		}
		else
		{
			free(*data);
		}
		*data = NULL;
		*num_ips = 0;
	}

	if(sockfd >= 0)
	{
		close(sockfd);
	}
	if(got_lock)
	{
		unlock();
	}

	return got_lock && (error == 0);
}


static int set_ip_block(void* ip_block_data, unsigned char is_history, unsigned char* output_buffer, uint32_t* current_output_index, uint32_t output_buffer_length)
{
	if(is_history)
	{
		ip_bw_history* history = (ip_bw_history*)ip_block_data;
		uint32_t block_length = (2*4) + (3*8) + (8*history->num_nodes);
		if(*current_output_index + block_length > output_buffer_length)
		{
			return 1;
		}
	
		*( (uint32_t*)(output_buffer + *current_output_index) ) = history->ip;
		*current_output_index = *current_output_index + 4;

		*( (uint32_t*)(output_buffer + *current_output_index) ) = history->num_nodes;
		*current_output_index = *current_output_index + 4;

		
		/* 
		 * We now need to deal with DST
		 *
		 * The problem is that the kernel module can't tell the difference
		 * between timezones being switched and entering daylight savings
		 * time.  Whenever the time offset from UTC shifts, the kernel module
		 * shifts values in the bandwidth history to reflect the time
		 * as it would be if the current offset from UTC had always been
		 * in effect.  In order to keep all data in the kernel module 
		 * consistent we need to make sure values we send kernel module
		 * are adjusted for current offset from UTC, not the real one, 
		 * where the offset may be inconsistent because of DST.
		 *
		 * Also note we ignore all but the first, second and last values in the
		 * history time list.  These are the only values the kernel needs/uses.
		 * We have the whole list in the structure so that we don't have to make
		 * programs that deal with the library worry about the conversion when
		 * values are returned.  However, when setting data, it's perfectly ok
		 * to just have the three necessary values and set everything else to zero
		 */
		time_t first_start = history->first_start;	
		time_t first_end = history->first_end;
		time_t last_end = history->last_end;

		time_t now;
		time(&now);
		int current_minutes_west = get_minutes_west(now);
		first_start = first_start + (get_minutes_west(first_start)-current_minutes_west);
		first_end = first_end + (get_minutes_west(first_end)-current_minutes_west);
		last_end = last_end + (get_minutes_west(last_end)-current_minutes_west);
		
		
		*( (uint64_t*)(output_buffer + *current_output_index) ) = (uint64_t)first_start;
		*current_output_index = *current_output_index + 8;

		*( (uint64_t*)(output_buffer + *current_output_index) ) = (uint64_t)first_end;
		*current_output_index = *current_output_index + 8;

		*( (uint64_t*)(output_buffer + *current_output_index) ) = (uint64_t)last_end;
		*current_output_index = *current_output_index + 8;

		uint32_t node_num = 0;
		for(node_num=0; node_num < history->num_nodes; node_num++)
		{
			*( (uint64_t*)(output_buffer + *current_output_index) ) = (history->history_bws)[ node_num ];
			*current_output_index = *current_output_index + 8;
		}
	}
	else
	{
		if(*current_output_index + 12 > output_buffer_length)
		{
			return 1;
		}
	

		ip_bw* ib = (ip_bw*)ip_block_data;
		*( (uint32_t*)(output_buffer + *current_output_index) ) = ib->ip;
		*current_output_index = *current_output_index + 4;
		*( (uint64_t*)(output_buffer + *current_output_index) ) = ib->bw;
		
		/*
		struct in_addr addr;
		addr.s_addr = ib->ip;
		printf("setting ip = %s, ip index = %ld\n", inet_ntoa(addr), (*current_output_index)-4);
		printf("setting bw = %lld, bw_index = %ld\n", ib->bw, *current_output_index);
		*/

		*current_output_index = *current_output_index + 8;
	}
	return 0;
}

static int set_bandwidth_data(char* id, unsigned char zero_unset, unsigned char set_history, unsigned long num_ips, time_t last_backup, void* data, unsigned long max_wait_milliseconds)
{
	unsigned char buf[BANDWIDTH_QUERY_LENGTH];
	memset(buf, 0, BANDWIDTH_QUERY_LENGTH);
	int done = 0;

		
	int got_lock = lock(max_wait_milliseconds);
	int sockfd = -1;
	if(got_lock)
	{
		sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	}

	uint32_t* total_ips = (uint32_t*)(buf+0);
	uint32_t* next_ip_index = (uint32_t*)(buf+4);
	uint32_t* num_ips_in_buffer = (uint32_t*)(buf+8);
	unsigned char* history_included = (unsigned char*)(buf+12);
	unsigned char* zero_unset_ips = (unsigned char*)(buf+13);
	uint64_t* last_backup_time = (uint64_t*)(buf+14);
	unsigned char* set_id = (unsigned char*)(buf+22);


	*total_ips = num_ips;
	*next_ip_index = 0;
	*num_ips_in_buffer = 0;
	*history_included = set_history;
	*zero_unset_ips = zero_unset;
	*last_backup_time = (uint64_t)last_backup;
	strncpy(set_id, id, BANDWIDTH_MAX_ID_LENGTH);
	set_id[BANDWIDTH_MAX_ID_LENGTH-1] = '\0';

	while(!done && sockfd >= 0 && got_lock)
	{
		uint32_t buf_index = (3*4) + (2*1) + 8 + BANDWIDTH_MAX_ID_LENGTH;
		uint32_t ip_index = *next_ip_index;
		unsigned char buffer_full = 0;
		memset( (buf + buf_index), 0, (BANDWIDTH_QUERY_LENGTH-buf_index) );
		*num_ips_in_buffer = 0;
		done = (ip_index >= *total_ips);


		while( (!buffer_full) && (!done) )
		{
			void *next_data = set_history ? (void*)(((ip_bw_history*)data) + ip_index) : (void*)(((ip_bw*)data) + ip_index);
			buffer_full = set_ip_block(next_data , set_history, buf, &buf_index, BANDWIDTH_QUERY_LENGTH);
			ip_index = buffer_full ? ip_index : ip_index+1;
			*num_ips_in_buffer = buffer_full ? *num_ips_in_buffer : *num_ips_in_buffer + 1;
			done = (ip_index >= *total_ips);

		}
		setsockopt(sockfd, IPPROTO_IP, BANDWIDTH_SET, buf, BANDWIDTH_QUERY_LENGTH);

		*next_ip_index = ip_index;
	}
	if(sockfd >= 0)
	{
		close(sockfd);
	}
	if(got_lock)
	{
		unlock();
	}
	return got_lock;
}

static unsigned char* read_entire_file(FILE* in, unsigned long read_block_size, unsigned long *length)
{
	int max_read_size = read_block_size;
	unsigned char* read_string = (unsigned char*)malloc(max_read_size+1);
	unsigned long bytes_read = 0;
	int end_found = 0;
	while(end_found == 0)
	{
		int nextch = '?';
		while(nextch != EOF && bytes_read < max_read_size)
		{
			nextch = fgetc(in);
			if(nextch != EOF)
			{
				read_string[bytes_read] = (unsigned char)nextch;
				bytes_read++;
			}
		}
		read_string[bytes_read] = '\0';
		end_found = (nextch == EOF) ? 1 : 0;
		if(end_found == 0)
		{
			unsigned char *new_str;
			max_read_size = max_read_size + read_block_size;
		       	new_str = (unsigned char*)malloc(max_read_size+1);
			memcpy(new_str, read_string, bytes_read);
			free(read_string);
			read_string = new_str;
		}
	}
	*length = bytes_read;
	return read_string;
}

/*
 * line is the line to be parsed -- it is not modified in any way
 * max_pieces indicates number of pieces to return, if negative this is determined dynamically
 * include_remainder_at_max indicates whether the last piece, when max pieces are reached, 
 * 	should be what it would normally be (0) or the entire remainder of the line (1)
 * 	if max_pieces < 0 this parameter is ignored
 *
 *
 * returns all non-separator pieces in a line
 * result is dynamically allocated, MUST be freed after call-- even if 
 * line is empty (you still get a valid char** pointer to to a NULL char*)
 */
static char** split_on_separators(char* line, char* separators, int num_separators, int max_pieces, int include_remainder_at_max, unsigned long *pieces_read)
{
	char** split;
	*pieces_read = 0;

	if(line != NULL)
	{
		int split_index;
		int non_separator_found;
		char* dup_line;
		char* start;

		if(max_pieces < 0)
		{
			/* count number of separator characters in line -- this count + 1 is an upperbound on number of pieces */
			int separator_count = 0;
			int line_index;
			for(line_index = 0; line[line_index] != '\0'; line_index++)
			{
				int sep_index;
				int found = 0;
				for(sep_index =0; found == 0 && sep_index < num_separators; sep_index++)
				{
					found = separators[sep_index] == line[line_index] ? 1 : 0;
				}
				separator_count = separator_count+ found;
			}
			max_pieces = separator_count + 1;
		}
		split = (char**)malloc((1+max_pieces)*sizeof(char*));
		split_index = 0;
		split[split_index] = NULL;


		dup_line = strdup(line);
		start = dup_line;
		non_separator_found = 0;
		while(non_separator_found == 0)
		{
			int matches = 0;
			int sep_index;
			for(sep_index =0; sep_index < num_separators; sep_index++)
			{
				matches = matches == 1 || separators[sep_index] == start[0] ? 1 : 0;
			}
			non_separator_found = matches==0 || start[0] == '\0' ? 1 : 0;
			if(non_separator_found == 0)
			{
				start++;
			}
		}

		while(start[0] != '\0' && split_index < max_pieces)
		{
			/* find first separator index */
			int first_separator_index = 0;
			int separator_found = 0;
			while(	separator_found == 0 )
			{
				int sep_index;
				for(sep_index =0; separator_found == 0 && sep_index < num_separators; sep_index++)
				{
					separator_found = separators[sep_index] == start[first_separator_index] || start[first_separator_index] == '\0' ? 1 : 0;
				}
				if(separator_found == 0)
				{
					first_separator_index++;
				}
			}
			
			/* copy next piece to split array */
			if(first_separator_index > 0)
			{
				char* next_piece = NULL;
				if(split_index +1 < max_pieces || include_remainder_at_max <= 0)
				{
					next_piece = (char*)malloc((first_separator_index+1)*sizeof(char));
					memcpy(next_piece, start, first_separator_index);
					next_piece[first_separator_index] = '\0';
				}
				else
				{
					next_piece = strdup(start);
				}
				split[split_index] = next_piece;
				split[split_index+1] = NULL;
				split_index++;
				*pieces_read = split_index;
			}


			/* find next non-separator index, indicating start of next piece */
			start = start+ first_separator_index;
			non_separator_found = 0;
			while(non_separator_found == 0)
			{
				int matches = 0;
				int sep_index;
				for(sep_index =0; sep_index < num_separators; sep_index++)
				{
					matches = matches == 1 || separators[sep_index] == start[0] ? 1 : 0;
				}
				non_separator_found = matches==0 || start[0] == '\0' ? 1 : 0;
				if(non_separator_found == 0)
				{
					start++;
				}
			}
		}
		free(dup_line);
		
	}
	else
	{
		split = (char**)malloc((1)*sizeof(char*));
		split[0] = NULL;
	}
	return split;
}


time_t* get_interval_starts_for_history(ip_bw_history history)
{
	time_t *start_times = NULL;
	if(history.num_nodes > 0)
	{
		start_times = (time_t*)malloc(history.num_nodes*sizeof(time_t));
		int node_index =0; 
		time_t next_start = history.first_start;
		time_t next_end = get_next_node_start_time(next_start, history.reset_interval, history.reset_time, history.is_constant_interval);
		for(node_index=0; node_index < history.num_nodes; node_index++)
		{
			start_times[node_index] = next_start;
			next_start = next_end;
			next_end = get_next_node_start_time(next_start, history.reset_interval, history.reset_time, history.is_constant_interval);
		}
	}
	return start_times;
}



void free_ip_bw_histories(ip_bw_history* histories, int num_histories)
{
	if(histories == NULL)
	{
		return;
	}
	int history_index = 0;
	for(history_index=0; history_index < num_histories; history_index++)
	{
		if((histories[history_index]).history_bws != NULL)
		{
			free( (histories[history_index]).history_bws );
		}
	}
	free(histories);
}






int get_all_bandwidth_history_for_rule_id(char* id, unsigned long* num_ips, ip_bw_history** data, unsigned long max_wait_milliseconds)
{
	return get_bandwidth_data(id, 1, "ALL", num_ips, (void*)data, max_wait_milliseconds);
}
int get_ip_bandwidth_history_for_rule_id(char* id, char* ip, ip_bw_history** data, unsigned long max_wait_milliseconds)
{
	unsigned long num_ips;
	return get_bandwidth_data(id, 1, ip, &num_ips, (void*)data, max_wait_milliseconds);
}
int get_all_bandwidth_usage_for_rule_id(char* id, unsigned long* num_ips, ip_bw** data, unsigned long max_wait_milliseconds)
{
	return get_bandwidth_data(id, 0, "ALL", num_ips, (void*)data, max_wait_milliseconds);
}
int get_ip_bandwidth_usage_for_rule_id(char* id,  char* ip, ip_bw** data, unsigned long max_wait_milliseconds)
{
	unsigned long num_ips;
	return get_bandwidth_data(id, 0, ip, &num_ips, (void*)data, max_wait_milliseconds);
}


int set_bandwidth_history_for_rule_id(char* id, unsigned char zero_unset, unsigned long num_ips, ip_bw_history* data, unsigned long max_wait_milliseconds)
{
	return set_bandwidth_data(id, zero_unset, 1, num_ips, 0, data, max_wait_milliseconds);
}

int set_bandwidth_usage_for_rule_id(char* id, unsigned char zero_unset, unsigned long num_ips, time_t last_backup, ip_bw* data, unsigned long max_wait_milliseconds)
{
	return set_bandwidth_data(id, zero_unset, 0, num_ips, last_backup, data, max_wait_milliseconds);
}






/* save single id in ascii */
int save_usage_to_file(ip_bw* data, unsigned long num_ips, char* out_file_path)
{
		
	int success = 0;
	FILE* out_file = fopen(out_file_path, "w");
	if(out_file != NULL)
	{
		//dump backup time
		time_t now;
		time(&now);
		fprintf(out_file, "%-15ld\n", now);
		
		//dump ips
		int out_index=0;
		for(out_index=0; out_index < num_ips; out_index++)
		{
			struct in_addr ipaddr;
			ip_bw next = data[out_index];
			ipaddr.s_addr = next.ip;
			fprintf(out_file, "%-15s\t%lld\n", inet_ntoa(ipaddr), (long long int)next.bw);
		}
		fclose(out_file);
		success = 1;
	}
	return success;
}

/* save history (must be for one id only) in binary so it takes up less space */
int save_history_to_file(ip_bw_history* data, unsigned long num_ips, char* out_file_path)
{
	int success = 0;
	FILE* out_file = fopen(out_file_path, "wb");
	if(out_file != NULL)
	{
		//dump number of ips & history interval parameter 
		//note that we assume interval is same for all histories
		//(which will be the case if they all come from the same rule id)
		fwrite((uint32_t*)(&num_ips), 4, 1, out_file);
		if(num_ips > 0)
		{
			ip_bw_history first = data[0];
			uint64_t interval = (uint64_t)(first.reset_interval);
			uint64_t time     = (uint64_t)(first.reset_time);
			unsigned char is_constant = first.is_constant_interval;

			fwrite( &interval, 8, 1, out_file);
			fwrite( &time, 8, 1, out_file);
			fwrite( &is_constant, 1, 1, out_file);
		}

		unsigned char  bw_fits_in_32bits = 1;
		uint32_t out_index=0;
		for(out_index=0; out_index < num_ips && bw_fits_in_32bits; out_index++)
		{
			uint32_t node_index = 0;
			ip_bw_history next = data[out_index];
			for(node_index=0; node_index < next.num_nodes && bw_fits_in_32bits; node_index++)
			{
				uint64_t bw = (next.history_bws)[node_index];
				bw_fits_in_32bits = bw_fits_in_32bits && (bw <  INT32_MAX);
			}
		}

		//dump data for each ip
		for(out_index=0; out_index < num_ips; out_index++)
		{
			ip_bw_history next = data[out_index];
			
			fwrite( &(next.ip), 4, 1, out_file);
			fwrite( &(next.num_nodes), 4, 1, out_file);
			if(next.num_nodes == 0)
			{
				uint64_t dummy = 0;
				unsigned char bw_bits = 32;
				fwrite( &dummy, 8, 1, out_file);
				fwrite( &dummy, 8, 1, out_file);
				fwrite( &dummy, 8, 1, out_file);
				fwrite( &bw_bits,  1, 1, out_file);
			}
			else
			{
				uint32_t node_index = 0;
				uint64_t first_start = (uint64_t)next.first_start;
				uint64_t first_end   = (uint64_t)next.first_end;
				uint64_t last_end    = (uint64_t)next.last_end;
				unsigned char  bw_bits = 32;
				for(node_index=0; node_index < next.num_nodes && bw_bits == 32; node_index++)
				{
					uint64_t bw = (next.history_bws)[node_index];
					bw_bits = bw_bits == 32 && (bw <  INT32_MAX) ? 32 : 64;
				}

				
				fwrite( &first_start, 8, 1, out_file);
				fwrite( &first_end,   8, 1, out_file);
				fwrite( &last_end,    8, 1, out_file);
				fwrite( &bw_bits,     1, 1, out_file);
				for(node_index=0; node_index < next.num_nodes; node_index++)
				{
					if(bw_bits == 32)
					{
						uint32_t bw = (uint32_t)(next.history_bws)[node_index];
						fwrite( &bw, 4, 1, out_file);

					}
					else
					{
						uint64_t bw = (next.history_bws)[node_index];
						fwrite( &bw, 8, 1, out_file);
					}
				}
			}
		}
		fclose(out_file);
		success = 1;
	}
	return success;
}


ip_bw* load_usage_from_file(char* in_file_path, unsigned long* num_ips, time_t* last_backup)
{
	ip_bw* data = NULL;
	*num_ips = 0;
	*last_backup = 0;
	FILE* in_file = fopen(in_file_path, "r");
	if(in_file != NULL)
	{
		unsigned long num_data_parts = 0;
		char* file_data = read_entire_file(in_file, 4086, &num_data_parts);
		fclose(in_file);
		char whitespace[] =  {'\n', '\r', '\t', ' '};
		char** data_parts = split_on_separators(file_data, whitespace, 4, -1, 0, &num_data_parts);
		free(file_data);

		*num_ips = (num_data_parts/2) + 1;
       		data = (ip_bw*)malloc( (*num_ips) * sizeof(ip_bw) );
		*num_ips = 0;
		unsigned long data_index = 0;
		unsigned long data_part_index = 0;
		while(data_part_index < num_data_parts)
		{
			ip_bw next;
			struct in_addr ipaddr;
			if(data_part_index == 0)
			{
				sscanf(data_parts[data_part_index], "%ld", last_backup);
				//printf("last_backup = %ld\n", *last_backup);
				data_part_index++;
			}
			int valid = inet_aton(data_parts[data_part_index], &ipaddr);
			data_part_index++;

			if(valid && data_index < num_data_parts)
			{
				next.ip = ipaddr.s_addr;
				valid = sscanf(data_parts[data_part_index], "%lld", (long long int*)&(next.bw) );
				data_part_index++;
			}
			else
			{
				valid = 0;
			}

			if(valid)
			{
				//printf("next.bw = %lld\n", next.bw);
				//printf("next.ip = %d\n", next.ip);
				data[data_index] = next;
				data_index++;
				*num_ips = *num_ips + 1;
			}
		}
		
		/* cleanup by freeing data_parts */
		for(data_part_index = 0; data_part_index < num_data_parts; data_part_index++)
		{
			free(data_parts[data_part_index]);
		}
		
		free(data_parts);
	}
	return data;
}


ip_bw_history* load_history_from_file(char* in_file_path, unsigned long* num_ips)
{
	ip_bw_history* data = NULL;
	*num_ips = 0;
	FILE* in_file = fopen(in_file_path, "rb");
	if(in_file != NULL)
	{
		uint64_t reset_interval;
		uint64_t reset_time;
		unsigned char is_constant_interval;
		
		uint32_t nips = 0;
		fread(&nips, 4, 1, in_file);
		*num_ips = (unsigned long)nips;

		if(*num_ips > 0)
		{
			fread(&reset_interval, 8, 1, in_file);
			fread(&reset_time, 8, 1, in_file);
			fread(&is_constant_interval, 1, 1, in_file);
			data = (ip_bw_history*)malloc( (*num_ips) * sizeof(ip_bw_history));
		}

		uint32_t ip_index;
		for(ip_index=0; ip_index < *num_ips; ip_index++)
		{

			uint32_t ip;
			uint32_t num_nodes;
			uint64_t first_start;	
			uint64_t first_end;
			uint64_t last_end;
			unsigned char bw_bits;

			fread(&ip, 4, 1, in_file);
			fread(&num_nodes, 4, 1, in_file);
			fread(&first_start, 8, 1, in_file);
			fread(&first_end, 8, 1, in_file);
			fread(&last_end, 8, 1, in_file);
			fread(&bw_bits, 1, 1, in_file);

			ip_bw_history next;
			next.reset_interval       = (time_t)reset_interval;
			next.reset_time           = (time_t)reset_time;
			next.is_constant_interval = is_constant_interval;
			next.ip                   = ip;
			next.num_nodes            = num_nodes;
			next.first_start          = (time_t)first_start;
			next.first_end            = (time_t)first_end;
			next.last_end             = (time_t)last_end;
			next.history_bws          = NULL;
			if(next.num_nodes > 0)
			{
				next.history_bws = malloc( next.num_nodes * sizeof(uint64_t) );
				uint32_t node_index = 0;
				for(node_index=0; node_index < next.num_nodes; node_index++)
				{
					if(bw_bits == 32)
					{
						uint32_t nextbw = 0;
						fread(&nextbw, 4, 1, in_file);
						(next.history_bws)[node_index] = (uint64_t)nextbw;

					}
					else
					{
						uint64_t nextbw = 0;
						fread(&nextbw, 8, 1, in_file);
						(next.history_bws)[node_index] = nextbw;
					}
				}
			}
			data[ip_index] = next;
		}
		fclose(in_file);
	}
	return data;
}


void print_usage(FILE* out, ip_bw* usage, unsigned long num_ips)
{
	unsigned long usage_index;
	for(usage_index =0; usage_index < num_ips; usage_index++)
	{
		ip_bw next = usage[usage_index];
		if(next.ip != 0)
		{
			struct in_addr ipaddr;
			ipaddr.s_addr = next.ip;
			fprintf(out, "%-15s\t%lld\n", inet_ntoa(ipaddr), (long long int)next.bw);
		}
		else
		{
			fprintf(out, "%-15s\t%lld\n", "COMBINED", (long long int)next.bw);
		}
	}
	fprintf(out, "\n");
}

void print_histories(FILE* out, char* id, ip_bw_history* histories, unsigned long num_histories, char output_type)
{
	unsigned long history_index = 0;
	for(history_index=0; history_index < num_histories; history_index++)
	{
		ip_bw_history history = histories[history_index];
		
		int history_initialized = 1;
		if( history.first_start == 0 && history.first_end == 0 && history.last_end == 0)
		{
			history_initialized = 0;
		}

		if(history_initialized)
		{
			char *ip_str = NULL;
			time_t *times = NULL;


			if(history.ip != 0)
			{
				struct in_addr ipaddr;
				ipaddr.s_addr = history.ip;
				ip_str = strdup(inet_ntoa(ipaddr));
			}
			else
			{
				ip_str = strdup("COMBINED");
			}
		
		
			if(output_type == 'm' || output_type == 'h')
			{
				fprintf(out, "%s %-15s\n", id, ip_str);
			}

			if(output_type == 'm')
			{
				printf("%ld\n", history.first_start);
				printf("%ld\n", history.first_end);
				printf("%ld\n", history.last_end);
			}
			else
			{
				times = get_interval_starts_for_history(history);
			}

			int hindex = 0;
			for(hindex=0; hindex < history.num_nodes; hindex++)
			{
				uint64_t bw = (history.history_bws)[hindex];
				if(output_type == 'm')
				{
					if(hindex != 0) { printf(","); };
					printf("%lld", (unsigned long long int)bw);
				}
				else if(times != NULL)
				{
					time_t start = times[hindex];
					time_t end = hindex+1 < history.num_nodes ? times[hindex+1] : 0 ;
	
					char* start_str = strdup(asctime(localtime(&start)));
					char* end_str = end == 0 ? strdup("(Now)") : strdup(asctime(localtime(&end)));
					char* nl = strchr(start_str, '\n');
					if(nl != NULL)
					{
						*nl = '\0';
					}
					nl = strchr(end_str, '\n');
					if(nl != NULL)
					{
						*nl = '\0';
					}
	
					if(output_type == 'h')
					{
						fprintf(out, "%lld\t%s\t%s\n", (unsigned long long int)bw, start_str, end_str);
					}
					else
					{
						fprintf(out, "%s,%s,%ld,%ld,%lld\n", id, ip_str, start, end, (unsigned long long int)bw );
					}
				
	
					free(start_str);
					free(end_str);
				}
			}
			fprintf(out, "\n");
			if(times != NULL) { free(times); };
			if(ip_str != NULL) { free(ip_str); };
		}
	}
}




void unlock_bandwidth_semaphore(void)
{
	unlock();
}

void signal_handler(int sig)
{
	if(sig == SIGTERM || sig == SIGINT )
	{
		unlock_bandwidth_semaphore();
		exit(0);
	}
}

void unlock_bandwidth_semaphore_on_exit(void)
{
	signal(SIGTERM,signal_handler);
	signal(SIGINT, signal_handler);
}


int get_minutes_west(time_t now)
{
	struct tm* utc_info;
	struct tm* tz_info;
	int utc_day;
	int utc_hour;
	int utc_minute;
	int tz_day;
	int tz_hour;
	int tz_minute;
	int minuteswest;

	utc_info = gmtime(&now);
	utc_day = utc_info->tm_mday;
	utc_hour = utc_info->tm_hour;
	utc_minute = utc_info->tm_min;
	tz_info = localtime(&now);
	tz_day = tz_info->tm_mday;
	tz_hour = tz_info->tm_hour;
	tz_minute = tz_info->tm_min;

	utc_day = utc_day < tz_day  - 1 ? tz_day  + 1 : utc_day;
	tz_day =  tz_day  < utc_day - 1 ? utc_day + 1 : tz_day;
	
	minuteswest = (24*60*utc_day + 60*utc_hour + utc_minute) - (24*60*tz_day + 60*tz_hour + tz_minute) ;

	return minuteswest;
}


void set_kernel_timezone(void)
{
	time_t now;
	struct timeval tv;
	struct timezone old_tz;
	struct timezone new_tz;

	time(&now);
	new_tz.tz_minuteswest = get_minutes_west(now);
	new_tz.tz_dsttime = 0;

	/* Get tv to pass to settimeofday(2) to be sure we avoid hour-sized warp */
	/* (see gettimeofday(2) man page, or /usr/src/linux/kernel/time.c) */
	syscall(SYS_gettimeofday, &tv, &old_tz);

	/* set timezone */
	syscall(SYS_settimeofday, &tv, &new_tz);
}
