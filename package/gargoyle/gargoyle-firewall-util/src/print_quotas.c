/*  print_quotas --	Used to print quota data from iptables rules that use the "bandwidth" module
 *  			Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2009-2010 by Eric Bishop <eric@gargoyle-router.com>
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <erics_tools.h>
#include <uci.h>
#include <ipt_bwctl.h>
#define malloc safe_malloc
#define strdup safe_strdup

list* get_all_sections_of_type(struct uci_context *ctx, char* package, char* section_type);
void  backup_quota(char* quota_id, char* quota_backup_dir);
char* get_uci_option(struct uci_context* ctx,char* package_name, char* section_name, char* option_name);
char* get_option_value_string(struct uci_option* uopt);


int main(void)
{
	struct uci_context *ctx = uci_alloc_context();
	list* quota_sections = get_all_sections_of_type(ctx, "firewall", "quota");
	unlock_bandwidth_semaphore_on_exit();

	/* for each ip have uint64_t[6], */
	string_map *id_ip_to_bandwidth = initialize_string_map(1);
	string_map *id_ip_to_percents  = initialize_string_map(1);	
	string_map *id_ip_to_limits    = initialize_string_map(1);
	list *id_to_time               = initialize_list();

	while(quota_sections->length > 0)
	{
		char* next_quota = shift_list(quota_sections);
		

		/* base id for quota is the ip associated with it*/
		char *id = get_uci_option(ctx, "firewall", next_quota, "id");
		char* ip = get_uci_option(ctx, "firewall", next_quota, "ip");	
		if(ip == NULL)
		{
			ip = strdup("ALL");
		}
		else if(strcmp(ip, "") == 0)
		{
			free(ip);
			ip = strdup("ALL");
		}
		if(id == NULL)
		{
			id = strdup(ip);
		}
		else if(strcmp(id, "") == 0)
		{
			free(id);
			id = strdup(ip);
		}





		string_map* ip_to_bandwidth = get_string_map_element(id_ip_to_bandwidth, id);
		ip_to_bandwidth = ip_to_bandwidth == NULL ? initialize_string_map(1) : ip_to_bandwidth;
		set_string_map_element(id_ip_to_bandwidth, id, ip_to_bandwidth);

		string_map* ip_to_percents = get_string_map_element(id_ip_to_percents, id);
		ip_to_percents = ip_to_percents == NULL ? initialize_string_map(1) : ip_to_percents;
		set_string_map_element(id_ip_to_percents, id, ip_to_percents);

		string_map* ip_to_limits = get_string_map_element(id_ip_to_limits, id);
		ip_to_limits = ip_to_limits == NULL ? initialize_string_map(1) : ip_to_limits;
		set_string_map_element(id_ip_to_limits, id, ip_to_limits);

		char* offpeak_hours         = get_uci_option(ctx, "firewall", next_quota, "offpeak_hours");
		char* offpeak_weekdays      = get_uci_option(ctx, "firewall", next_quota, "offpeak_weekdays");
		char* offpeak_weekly_ranges = get_uci_option(ctx, "firewall", next_quota, "offpeak_weekly_ranges");
		char* onpeak_hours          = get_uci_option(ctx, "firewall", next_quota, "onpeak_hours");
		char* onpeak_weekdays       = get_uci_option(ctx, "firewall", next_quota, "onpeak_weekdays");
		char* onpeak_weekly_ranges  = get_uci_option(ctx, "firewall", next_quota, "onpeak_weekly_ranges");
		if(offpeak_hours != NULL || offpeak_weekdays != NULL || offpeak_weekly_ranges != NULL || onpeak_hours != NULL || onpeak_weekdays != NULL || onpeak_weekly_ranges != NULL)
		{
			unsigned char is_off_peak = (offpeak_hours != NULL || offpeak_weekdays != NULL || offpeak_weekly_ranges != NULL) ? 1 : 0;
			char* hours_var = is_off_peak ? offpeak_hours : onpeak_hours;
			char* weekdays_var = is_off_peak ? offpeak_weekdays : onpeak_weekdays;
			char* weekly_ranges_var = is_off_peak ? offpeak_weekly_ranges : onpeak_weekly_ranges;
			char* active_var = is_off_peak ? strdup("except") : strdup("only");
			
			if(weekly_ranges_var != NULL)
			{
				if(hours_var    != NULL) { free(hours_var); hours_var=NULL; }
				if(weekdays_var != NULL) { free(weekly_ranges_var); weekly_ranges_var=NULL; }
			}
			hours_var = hours_var == NULL ? strdup("") : hours_var;
			weekdays_var = weekdays_var == NULL ? strdup("") : weekdays_var;
			weekly_ranges_var = weekly_ranges_var == NULL ? strdup("") : weekly_ranges_var;
			push_list(id_to_time, dynamic_strcat(11, "quotaTimes[\"", id, "\"] = [\"", hours_var, "\", \"", weekdays_var, "\", \"", weekly_ranges_var ,"\", \"", active_var, "\"];"));
			
			free(hours_var);
			free(weekdays_var);
			free(weekly_ranges_var);
			free(active_var);
		}
		else
		{
			push_list(id_to_time, dynamic_strcat(3, "quotaTimes[\"", id, "\"] = [\"\", \"\", \"\", \"always\"];"));
		}

		char* types[] = { "combined_limit", "ingress_limit", "egress_limit" };
		char* postfixes[] = { "_combined", "_ingress", "_egress" };
		
		
		int type_index;
		for(type_index=0; type_index < 3; type_index++)
		{
			char* limit = get_uci_option(ctx, "firewall", next_quota, types[type_index]);
			if(limit != NULL)
			{
				char* type_id = dynamic_strcat(2, id, postfixes[type_index]);
				ip_bw* ip_buf;
				unsigned long num_ips = 0;
				int query_succeeded = get_all_bandwidth_usage_for_rule_id(type_id, &num_ips, &ip_buf, 5000);
				if(query_succeeded && num_ips > 0)
				{
					unsigned long ip_index = 0;
					for(ip_index = 0; ip_index < num_ips; ip_index++)
					{
						ip_bw next = ip_buf[ip_index];
						char* next_ip = NULL;
						if(next.ip == 0)
						{
							next_ip = strdup(ip);
						}
						else
						{
							struct in_addr addr;
							addr.s_addr = next.ip;
							next_ip = strdup(inet_ntoa(addr));
						}
						
						
						uint64_t *bw_list = get_string_map_element(ip_to_bandwidth,next_ip);
						if(bw_list == NULL)
						{
							bw_list = (uint64_t*)malloc(sizeof(uint64_t)*6);
							bw_list[0] = 0;
							bw_list[1] = 0;
							bw_list[2] = 0;
							bw_list[3] = 0;
							bw_list[4] = 0;
							bw_list[5] = 0;
							set_string_map_element(ip_to_bandwidth, next_ip, bw_list);
						}
						bw_list[type_index] = 1;
						bw_list[type_index+3] = next.bw;
						
						char bw_str[50];
						sprintf(bw_str, "%lld", next.bw);
						double bw_percent;
						double bw_limit;
						uint64_t bw_limit_64;
						sscanf(bw_str, "%lf", &bw_percent);
						sscanf(limit, "%lf", &bw_limit);
						sscanf(limit, "%lld", &bw_limit_64);
						if(bw_limit > 0)
						{
							bw_percent = (bw_percent*100.0)/bw_limit;
							bw_percent = bw_percent > 100.0 ? 100.0 : bw_percent;
						}
						else
						{
							bw_percent = 100.0;
						}
					
						double* percent_list = get_string_map_element(ip_to_percents, next_ip);
						if(percent_list == NULL)
						{
							percent_list = (double*)malloc(sizeof(double)*3);
							percent_list[0] = -1;
							percent_list[1] = -1;
							percent_list[2] = -1;
							set_string_map_element(ip_to_percents, next_ip, percent_list);
						}
						percent_list[type_index] = bw_percent;

						uint64_t* limit_list = get_string_map_element(ip_to_limits, next_ip);
						if(limit_list == NULL)
						{
							limit_list = (uint64_t*)malloc(sizeof(uint64_t)*3);
							limit_list[0] = -1;
							limit_list[1] = -1;
							limit_list[2] = -1;
							set_string_map_element(ip_to_limits, next_ip, limit_list);
						}
						limit_list[type_index] = bw_limit_64;
					}
				}
				free(type_id);
				free(limit);

			}
		}
		free(id);
		free(ip);
		free(next_quota);
	}
	

	unsigned long num_ids;
	char** id_list = (char**)get_string_map_keys(id_ip_to_bandwidth, &num_ids);
	printf("var quotaIdList = [ ");
	char print_comma[10] = "";
	unsigned long id_index;
	for(id_index=0; id_index < num_ids; id_index++)
	{
		printf("%s\"%s\"", print_comma, id_list[id_index]);
		sprintf(print_comma, ", ");
	}
	printf(" ];\n");

	printf("var quotaIpLists = [];\n");
	for(id_index=0; id_index < num_ids; id_index++)
	{
		string_map* ip_to_bandwidth = get_string_map_element(id_ip_to_bandwidth, id_list[id_index]);
		if(ip_to_bandwidth != NULL)
		{
			unsigned long num_ips = 0;
			unsigned long ip_index = 0;
			char** ip_list = (char**)get_string_map_keys(ip_to_bandwidth, &num_ips);
			printf("quotaIpLists[\"%s\"] = [ ", id_list[id_index]);
			sprintf(print_comma, "");
			for(ip_index=0; ip_index < num_ips; ip_index++)
			{
				printf("%s\"%s\"", print_comma, ip_list[ip_index]);
				sprintf(print_comma, ", ");
			}
			printf("];\n");
		}
	}	

	printf("var quotaTimes    = new Array();\n");
	printf("var quotaUsed     = new Array();\n");
	printf("var quotaLimits   = new Array();\n");
	printf("var quotaPercents = new Array();\n");

	char* next_time = shift_list(id_to_time);
	while(next_time != NULL)
	{
		printf("%s\n", next_time);
		next_time = shift_list(id_to_time);
	}

	for(id_index=0; id_index < num_ids; id_index++)
	{
		char* next_id = id_list[id_index];
		string_map* ip_to_bandwidth = get_string_map_element(id_ip_to_bandwidth, next_id);
		string_map* ip_to_percents = get_string_map_element(id_ip_to_percents, next_id);
		string_map* ip_to_limits = get_string_map_element(id_ip_to_limits, next_id);
		
		printf("quotaUsed[ \"%s\" ] = [];\n", next_id);
		printf("quotaPercents[ \"%s\" ] = [];\n", next_id);
		printf("quotaLimits[ \"%s\" ] = [];\n", next_id);
		
		
		if(ip_to_bandwidth != NULL)
		{
			unsigned long num_ips;
			unsigned long ip_index = 0;
			char** ip_list = (char**)get_string_map_keys(ip_to_bandwidth, &num_ips);
			for(ip_index=0; ip_index < num_ips; ip_index++)
			{
				char* next_ip = ip_list[ip_index];
				uint64_t* used   = (uint64_t*)get_string_map_element(ip_to_bandwidth, next_ip);
				double* percents = (double*)get_string_map_element(ip_to_percents, next_ip);
				uint64_t* limits = (uint64_t*)get_string_map_element(ip_to_limits, next_ip);

				if(used != NULL)
				{
					int type_index;
					printf("quotaUsed[ \"%s\" ][ \"%s\" ] = [ ", next_id, next_ip);
					sprintf(print_comma, "");
					for(type_index=0; type_index < 3; type_index++)
					{
						if(!used[type_index])
						{
							printf("%s-1", print_comma);
						}
						else
						{
							printf("%s%lld", print_comma, (long long int)used[type_index+3]);
						}
						sprintf(print_comma, ", ");
					}
					printf(" ];\n");

					printf("quotaPercents[ \"%s\" ][ \"%s\" ] = [ ", next_id, next_ip);
					sprintf(print_comma, "");
					for(type_index=0; type_index < 3; type_index++)
					{
						printf("%s%6.3lf", print_comma, percents[type_index]);
						sprintf(print_comma, ", ");
					}
					printf(" ];\n");

					printf("quotaLimits[ \"%s\" ][ \"%s\" ] = [ ", next_id, next_ip);
					sprintf(print_comma, "");
					for(type_index=0; type_index < 3; type_index++)
					{
						printf("%s%lld", print_comma,limits[type_index]);
						sprintf(print_comma, ", ");
					}
					printf(" ];\n");
				}
			}
		}
	}


	unsigned long num;
	destroy_list(quota_sections, DESTROY_MODE_FREE_VALUES, &num);
	uci_free_context(ctx);

	return 0;
}

list* get_all_sections_of_type(struct uci_context *ctx, char* package, char* section_type)
{

	struct uci_package *p = NULL;
	struct uci_element *e = NULL;

	list* sections_of_type = initialize_list();
	if(uci_load(ctx, package, &p) == UCI_OK)
	{
		uci_foreach_element( &p->sections, e)
		{
			struct uci_section *section = uci_to_section(e);
			if(safe_strcmp(section->type, section_type) == 0)
			{
				push_list(sections_of_type, strdup(section->e.name));
			}
		}
	}
	return sections_of_type;
}


char* get_uci_option(struct uci_context* ctx, char* package_name, char* section_name, char* option_name)
{
	char* option_value = NULL;
	struct uci_ptr ptr;
	char* lookup_str = dynamic_strcat(5, package_name, ".", section_name, ".", option_name);
	int ret_value = uci_lookup_ptr(ctx, &ptr, lookup_str, 1);
	if(ret_value == UCI_OK)
	{
		if( !(ptr.flags & UCI_LOOKUP_COMPLETE))
		{
			ret_value = UCI_ERR_NOTFOUND;
		}
		else
		{
			struct uci_element *e = (struct uci_element*)ptr.o;
			option_value = get_option_value_string(uci_to_option(e));
		}
	}
	free(lookup_str);

	return option_value;
}




// this function dynamically allocates memory for
// the option string, but since this program exits
// almost immediately (after printing variable info)
// the massive memory leak we're opening up shouldn't
// cause any problems.  This is your reminder/warning
// that this might be an issue if you use this code to
// do anything fancy.
char* get_option_value_string(struct uci_option* uopt)
{
	char* opt_str = NULL;
	if(uopt->type == UCI_TYPE_STRING)
	{
		opt_str = strdup(uopt->v.string);
	}
	if(uopt->type == UCI_TYPE_LIST)
	{
		struct uci_element* e;
		uci_foreach_element(&uopt->v.list, e)
		{
			if(opt_str == NULL)
			{
				opt_str = strdup(e->name);
			}
			else
			{
				char* tmp;
				tmp = dynamic_strcat(3, opt_str, " ", e->name);
				free(opt_str);
				opt_str = tmp;
			}
		}
	}

	return opt_str;
}


