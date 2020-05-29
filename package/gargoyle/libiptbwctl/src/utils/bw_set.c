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

#include <ipt_bwctl.h>
#define malloc ipt_bwctl_safe_malloc
#define strdup ipt_bwctl_safe_strdup

static char* read_entire_file(FILE* in, int read_block_size);
static char** split_on_separators(char* line, char* separators, int num_separators, int max_pieces, int include_remainder_at_max, unsigned long *pieces_read);

int main(int argc, char **argv)
{
	char *id = NULL;
	char* in_file_path = NULL;
	FILE* in_file = NULL;
	time_t last_backup = 0;
	int last_backup_from_cl = 0;
	int is_history_file = 0;


	int c;
	while((c = getopt(argc, argv, "i:I:b:B:f:F:UuHh")) != -1)
	{	
		switch(c)
		{
			case 'i':
			case 'I':
				if(strlen(optarg) < BANDWIDTH_MAX_ID_LENGTH)
				{
					id = strdup(optarg);
				}
				else
				{
					fprintf(stderr, "ERROR: ID length is improper length.\n");
					exit(0);
				}

				break;
			case 'b':
			case 'B':
				if(sscanf(optarg, "%ld", &last_backup) == 0)
				{
					fprintf(stderr, "ERROR: invalid backup time specified. Should be unix epoch seconds -- number of seconds since 1970 (UTC)\n");
					exit(0);
				}
				last_backup_from_cl = 1;
				break;
			case 'f':
			case 'F':
				in_file_path = strdup(optarg);
				in_file = fopen(optarg, "rb");
				if(in_file == NULL)
				{
					fprintf(stderr, "ERROR: cannot open specified file for reading\n");
					exit(0);
				}
				fclose(in_file);
				break;
			case 'h':
			case 'H':
				is_history_file = 1;
				break;
			case 'u':
			case 'U':
			default:
				fprintf(stderr, "USAGE:\n\t%s -i [ID] -b [LAST_BACKUP_TIME] -f [IN_FILE_NAME] [ IP BANDWIDTH PAIRS, IF -f NOT SPECIFIED ]\n", argv[0]);
				exit(0);

		}
	}

	if(id == NULL)
	{
		fprintf(stderr, "ERROR: you must specify an id for which to set data\n\n");
		exit(0);
	}
	if(in_file_path == NULL && is_history_file)
	{
		fprintf(stderr, "ERROR: you need to specify file to load history from\n\t\t(history format is too complex to load from command line)\n");
	}


	set_kernel_timezone();
	unlock_bandwidth_semaphore_on_exit();
	int query_succeeded = 0;
	if(in_file_path != NULL)
	{
		if(is_history_file)
		{
			unsigned long num_ips;
			ip_bw_history* history_data = load_history_from_file(in_file_path, &num_ips);
			if(history_data != NULL)
			{
				query_succeeded = set_bandwidth_history_for_rule_id(id, 1, num_ips, history_data, 1000);
			}
		}
		else
		{
			unsigned long num_ips;
			time_t last_backup;
			ip_bw* usage_data = load_usage_from_file(in_file_path, &num_ips, &last_backup);
			if(usage_data != NULL)
			{
				query_succeeded = set_bandwidth_usage_for_rule_id(id, 1, num_ips, last_backup, usage_data, 1000);
			}
		}
	}
	else
	{
		char** data_parts;
		unsigned long num_data_parts;
		data_parts = argv+optind;
		num_data_parts = argc - optind;
	
	
		unsigned long num_ips = num_data_parts/2;
       		ip_bw* buffer = (ip_bw*)malloc(num_ips*sizeof(ip_bw));
		unsigned long data_index = 0;
		unsigned long buffer_index = 0;
		while(data_index < num_data_parts)
		{
			ip_bw next;
			struct in_addr ipaddr;
			int valid = inet_aton(data_parts[data_index], &ipaddr);
			if((!valid) && (!last_backup_from_cl))
			{
				sscanf(data_parts[data_index], "%ld", &last_backup);
			}
			data_index++;
	
			if(valid && data_index < num_data_parts)
			{
				next.ip = ipaddr.s_addr;
				valid = sscanf(data_parts[data_index], "%lld", (long long int*)&(next.bw) );
				data_index++;
			}
			else
			{
				valid = 0;
			}

			if(valid)
			{
				/* printf("ip=%d, bw=%lld\n", next.ip, (long long int)next.bw); */
				buffer[buffer_index] = next;
				buffer_index++;
			}
		}
		num_ips = buffer_index; /* number that were successfully read */
		query_succeeded = set_bandwidth_usage_for_rule_id(id, 1, num_ips, last_backup, buffer, 1000);
	}
	
	if(!query_succeeded)
	{
		fprintf(stderr, "ERROR: Could not set data. Please try again.\n\n");
	}
	else
	{
		fprintf(stderr, "Data set successfully\n\n");
	}

	if(in_file_path != NULL)
	{
		free(in_file_path);
	}

	return 0;
}

