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


int main(int argc, char **argv)
{
	char *id = NULL;
	char* out_file_path = NULL;;
	char *address = NULL;

	unsigned long num_ips;
	void *ip_buf;
	unsigned long out_index;
	int query_succeeded;
	int get_history = 0;
	char output_type = 'h';

	int c;
	struct in_addr read_addr;
	while((c = getopt(argc, argv, "i:I:a:A:f:F:tThHmMuU")) != -1)
	{	
		switch(c)
		{
			case 'i':
			case 'I':
				if(strlen(optarg) < BANDWIDTH_MAX_ID_LENGTH && strlen(optarg) > 0)
				{
					id = strdup(optarg);
				}
				else
				{
					fprintf(stderr, "ERROR: ID length is improper length.\n");
					exit(0);
				}
				break;
			case 'a':
			case 'A':
				if(strcmp(optarg, "combined") == 0 || strcmp(optarg, "COMBINED") == 0)
				{
					address = strdup("0.0.0.0");
				}
				else if( inet_aton(optarg, &read_addr) )
				{
					address = strdup(optarg);
				}
				else
				{
					fprintf(stderr, "ERROR: invalid IP address specified\n");
					exit(0);
				}

				break;
			case 'f':
			case 'F':
				out_file_path = strdup(optarg);
				break;
			case 'h':
			case 'H':
				get_history = 1;
				break;
			case 'm':
			case 'M':
				output_type = 'm';
				break;
			case 't':
			case 'T':
				output_type = 't';
				break;
			case 'u':
			case 'U':
			default:
				fprintf(stderr, "USAGE:\n\t%s -i [ID] -a [IP ADDRESS] -f [OUT_FILE_NAME]\n", argv[0]);
				exit(0);
		}
	}


	if(id == NULL)
	{
		fprintf(stderr, "ERROR: you must specify an id to query\n\n");
		exit(0);
	}
	
	set_kernel_timezone();	
	unlock_bandwidth_semaphore_on_exit();
	
	if(get_history == 0)
	{
		if(address == NULL)
		{
			query_succeeded = get_all_bandwidth_usage_for_rule_id(id, &num_ips, (ip_bw**)&ip_buf, 1000);
		}
		else
		{
			num_ips = 1;
			query_succeeded = get_ip_bandwidth_usage_for_rule_id(id, address, (ip_bw**)&ip_buf, 1000);
		}
	}
	else
	{
		if(address == NULL)
		{
			query_succeeded = get_all_bandwidth_history_for_rule_id(id, &num_ips, (ip_bw_history**)&ip_buf, 1000);
		}
		else
		{
			num_ips = 1;
			query_succeeded = get_ip_bandwidth_history_for_rule_id(id, address, (ip_bw_history**)&ip_buf, 1000);
		}
	}
	if(!query_succeeded)
	{
		fprintf(stderr, "ERROR: Bandwidth query failed, make sure rule with specified id exists, and that you are performing only one query at a time.\n\n");
		exit(0);
	}


	if(out_file_path != NULL)
	{
		if(get_history == 0)
		{
			save_usage_to_file( (ip_bw*)ip_buf, num_ips, out_file_path);
		}
		else
		{
			save_history_to_file( (ip_bw_history*)ip_buf, num_ips, out_file_path);
		}
	}
	else
	{
		if(get_history == 0)
		{
			print_usage(stdout, (ip_bw*)ip_buf, num_ips);
		}
		else
		{
			print_histories(stdout, id, (ip_bw_history*)ip_buf, num_ips, output_type );
		}
	}
	if(num_ips == 0)
	{
	       	if(output_type != 't' && output_type != 'm')
		{
			fprintf(stderr, "No data available for id \"%s\"\n", id);
		}
	}
	printf("\n");

	if(out_file_path != NULL)
	{
		free(out_file_path);
	}

	return 0;
}
