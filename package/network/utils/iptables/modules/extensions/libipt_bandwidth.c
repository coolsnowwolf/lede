/*  bandwidth --	An iptables extension for bandwidth monitoring/control
 *  			Can be used to efficiently monitor bandwidth and/or implement bandwidth quotas
 *  			Can be queried using the iptbwctl userspace library
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
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

/*
 * in iptables 1.4.0 and higher, iptables.h includes xtables.h, which
 * we can use to check whether we need to deal with the new requirements
 * in pre-processor directives below
 */
#include <iptables.h>  
#include <linux/netfilter_ipv4/ipt_bandwidth.h>

#ifdef _XTABLES_H
	#define iptables_rule_match	xtables_rule_match
	#define iptables_match		xtables_match
	#define iptables_target		xtables_target
	#define ipt_tryload		xt_tryload
#endif

/* 
 * XTABLES_VERSION_CODE is only defined in versions 1.4.1 and later, which
 * also require the use of xtables_register_match
 * 
 * Version 1.4.0 uses register_match like previous versions
 */
#ifdef XTABLES_VERSION_CODE 
	#define register_match          xtables_register_match
#endif



int get_minutes_west(void);
void set_kernel_timezone(void);
int parse_sub(char* subnet_string, uint32_t* subnet, uint32_t* subnet_mask);
static unsigned long get_pow(unsigned long base, unsigned long pow);
static void param_problem_exit_error(char* msg);


/* Function which prints out usage message. */
static void help(void)
{
	printf("bandwidth options:\n");
	printf("  --id [unique identifier for querying bandwidth]\n");
	printf("  --type [combined|individual_src|individual_dst|individual_local|individual_remote]\n");
	printf("  --subnet [a.b.c.d/mask] (0 < mask < 32)\n");
	printf("  --greater_than [BYTES]\n");
	printf("  --less_than [BYTES]\n");
	printf("  --current_bandwidth [BYTES]\n");
	printf("  --reset_interval [minute|hour|day|week|month]\n");
	printf("  --reset_time [OFFSET IN SECONDS]\n");
	printf("  --intervals_to_save [NUMBER OF PREVIOS INTERVALS TO STORE IN MEMORY]\n");
	printf("  --last_backup_time [UTC SECONDS SINCE 1970]\n");
	printf("  --bcheck Check another bandwidth rule without incrementing it\n");
	printf("  --bcheck_with_src_dst_swap Check another bandwidth rule without incrementing it, swapping src & dst ips for check\n");
}

static struct option opts[] = 
{
	{ .name = "id", 			.has_arg = 1, .flag = 0, .val = BANDWIDTH_ID },	
	{ .name = "type", 			.has_arg = 1, .flag = 0, .val = BANDWIDTH_TYPE },	
	{ .name = "subnet", 			.has_arg = 1, .flag = 0, .val = BANDWIDTH_SUBNET },	
	{ .name = "greater_than", 		.has_arg = 1, .flag = 0, .val = BANDWIDTH_GT },
	{ .name = "less_than", 			.has_arg = 1, .flag = 0, .val = BANDWIDTH_LT },	
	{ .name = "current_bandwidth",		.has_arg = 1, .flag = 0, .val = BANDWIDTH_CURRENT },	
	{ .name = "reset_interval",		.has_arg = 1, .flag = 0, .val = BANDWIDTH_RESET_INTERVAL },
	{ .name = "reset_time",			.has_arg = 1, .flag = 0, .val = BANDWIDTH_RESET_TIME },
	{ .name = "intervals_to_save",		.has_arg = 1, .flag = 0, .val = BANDWIDTH_NUM_INTERVALS },
	{ .name = "last_backup_time",		.has_arg = 1, .flag = 0, .val = BANDWIDTH_LAST_BACKUP},
	{ .name = "bcheck",	 		.has_arg = 0, .flag = 0, .val = BANDWIDTH_CHECK_NOSWAP },
	{ .name = "bcheck_with_src_dst_swap",	.has_arg = 0, .flag = 0, .val = BANDWIDTH_CHECK_SWAP },
	{ .name = 0 }
};


/* Function which parses command options; returns true if it
   ate an option */
static int parse(	int c, 
			char **argv,
			int invert,
			unsigned int *flags,
#ifdef _XTABLES_H
			const void *entry,
#else
			const struct ipt_entry *entry,
			unsigned int *nfcache,
#endif			
			struct ipt_entry_match **match
			)
{
	struct ipt_bandwidth_info *info = (struct ipt_bandwidth_info *)(*match)->data;
	int valid_arg = 0;
	long int num_read;
	uint64_t read_64;
	time_t read_time;

	/* set defaults first time we get here */
	if(*flags == 0)
	{
		/* generate random id */
		srand ( time(NULL) );
		unsigned long id_num = rand();
		sprintf(info->id, "%lu", id_num);

		info->type = BANDWIDTH_COMBINED;
		info->check_type = BANDWIDTH_CHECK_NOSWAP;
		info->local_subnet = 0;
		info->local_subnet_mask = 0;
		info->cmp = BANDWIDTH_MONITOR; /* don't test greater/less than, just monitor bandwidth */
		info->current_bandwidth = 0;
		info->reset_is_constant_interval = 0;
		info->reset_interval = BANDWIDTH_NEVER;
		info->reset_time=0;
		info->last_backup_time = 0;
		info->next_reset = 0;
		
		info->num_intervals_to_save=0;

		info->non_const_self = NULL;
		info->ref_count = NULL;

		*flags = *flags + BANDWIDTH_INITIALIZED;
	}

	switch (c)
	{
		case BANDWIDTH_ID:
			if(strlen(optarg) < BANDWIDTH_MAX_ID_LENGTH)
			{
				sprintf(info->id, "%s", optarg);
				valid_arg = 1;
			}
			c=0;
			break;
		case BANDWIDTH_TYPE:
			valid_arg = 1;
			if(strcmp(optarg, "combined") == 0)
			{
				info->type = BANDWIDTH_COMBINED;
			}
			else if(strcmp(optarg, "individual_src") == 0)
			{
				info->type = BANDWIDTH_INDIVIDUAL_SRC;
			}
			else if(strcmp(optarg, "individual_dst") == 0)
			{
				info->type = BANDWIDTH_INDIVIDUAL_DST;
			}
			else if(strcmp(optarg, "individual_local") == 0)
			{
				info->type = BANDWIDTH_INDIVIDUAL_LOCAL;
				*flags = *flags + BANDWIDTH_REQUIRES_SUBNET;
			}
			else if(strcmp(optarg, "individual_remote") == 0)
			{
				info->type = BANDWIDTH_INDIVIDUAL_REMOTE;
				*flags = *flags + BANDWIDTH_REQUIRES_SUBNET;
			}
			else
			{
				valid_arg = 0;
			}

			c=0;
			break;

		case BANDWIDTH_SUBNET:
			valid_arg =  parse_sub(optarg, &(info->local_subnet), &(info->local_subnet_mask));
			break;
		case BANDWIDTH_LT:
			num_read = sscanf(argv[optind-1], "%lld", &read_64);
			if(num_read > 0 && (*flags & BANDWIDTH_CMP) == 0)
			{
				info->cmp = BANDWIDTH_LT;
				info->bandwidth_cutoff = read_64;
				valid_arg = 1;
			}
			c = BANDWIDTH_CMP; //only need one flag for less_than/greater_than
			break;
		case BANDWIDTH_GT:
			num_read = sscanf(argv[optind-1], "%lld", &read_64);
			if(num_read > 0  && (*flags & BANDWIDTH_CMP) == 0)
			{
				info->cmp = BANDWIDTH_GT;
				info->bandwidth_cutoff = read_64;
				valid_arg = 1;
			}
			c = BANDWIDTH_CMP; //only need one flag for less_than/greater_than
			break;
		case BANDWIDTH_CHECK_NOSWAP:
			if(  (*flags & BANDWIDTH_CMP) == 0 )
			{
				info->cmp = BANDWIDTH_CHECK;
				info->check_type = BANDWIDTH_CHECK_NOSWAP;
				valid_arg = 1;
			}
			c = BANDWIDTH_CMP;
			break;
		case BANDWIDTH_CHECK_SWAP:
			if(  (*flags & BANDWIDTH_CMP) == 0 )
			{
				info->cmp = BANDWIDTH_CHECK;
				info->check_type = BANDWIDTH_CHECK_SWAP;
				valid_arg = 1;
			}
			c = BANDWIDTH_CMP;
			break;
		case BANDWIDTH_CURRENT:
			num_read = sscanf(argv[optind-1], "%lld", &read_64);
			if(num_read > 0 )
			{
				info->current_bandwidth = read_64;
				valid_arg = 1;
			}
			break;
		case BANDWIDTH_RESET_INTERVAL:
			valid_arg = 1;
			if(strcmp(argv[optind-1],"minute") ==0)
			{
				info->reset_interval = BANDWIDTH_MINUTE;
				info->reset_is_constant_interval = 0;
			}
			else if(strcmp(argv[optind-1],"hour") ==0)
			{
				info->reset_interval = BANDWIDTH_HOUR;
				info->reset_is_constant_interval = 0;
			}
			else if(strcmp(argv[optind-1],"day") ==0)
			{
				info->reset_interval = BANDWIDTH_DAY;
				info->reset_is_constant_interval = 0;
			}
			else if(strcmp(argv[optind-1],"week") ==0)
			{
				info->reset_interval = BANDWIDTH_WEEK;
				info->reset_is_constant_interval = 0;
			}
			else if(strcmp(argv[optind-1],"month") ==0)
			{
				info->reset_interval = BANDWIDTH_MONTH;
				info->reset_is_constant_interval = 0;
			}
			else if(strcmp(argv[optind-1],"never") ==0)
			{
				info->reset_interval = BANDWIDTH_NEVER;
			}
			else if(sscanf(argv[optind-1], "%ld", &read_time) > 0)
			{
				info->reset_interval = read_time;
				info->reset_is_constant_interval = 1;
			}
			else
			{
				valid_arg = 0;
			}
			break;
		case BANDWIDTH_NUM_INTERVALS:
			if( sscanf(argv[optind-1], "%ld", &num_read) > 0)
			{
				info->num_intervals_to_save = num_read;
				valid_arg=1;
			}
			c=0;
			break;
		case BANDWIDTH_RESET_TIME:
			num_read = sscanf(argv[optind-1], "%ld", &read_time);
			if(num_read > 0 )
			{
				info->reset_time = read_time;
				valid_arg = 1;
			}	
			break;
		case BANDWIDTH_LAST_BACKUP:
			num_read = sscanf(argv[optind-1], "%ld", &read_time);
			if(num_read > 0 )
			{
				info->last_backup_time = read_time;
				valid_arg = 1;
			}
			break;
	}
	*flags = *flags + (unsigned int)c;


	//if we have both reset_interval & reset_time, check reset_time is in valid range
	if((*flags & BANDWIDTH_RESET_TIME) == BANDWIDTH_RESET_TIME && (*flags & BANDWIDTH_RESET_INTERVAL) == BANDWIDTH_RESET_INTERVAL)
	{
		if(	(info->reset_interval == BANDWIDTH_NEVER) ||
			(info->reset_interval == BANDWIDTH_MONTH && info->reset_time >= 60*60*24*28) ||
			(info->reset_interval == BANDWIDTH_WEEK && info->reset_time >= 60*60*24*7) ||
			(info->reset_interval == BANDWIDTH_DAY && info->reset_time >= 60*60*24) ||
			(info->reset_interval == BANDWIDTH_HOUR && info->reset_time >= 60*60) ||
			(info->reset_interval == BANDWIDTH_MINUTE && info->reset_time >= 60) 
		  )
		{
			valid_arg = 0;
			param_problem_exit_error("Parameter for '--reset_time' is not in valid range");
		}
	}
	if(info->type != BANDWIDTH_COMBINED && (*flags & BANDWIDTH_CURRENT) == BANDWIDTH_CURRENT)
	{
		valid_arg = 0;
		param_problem_exit_error("You may only specify current bandwidth for combined type\n  Use user-space library for setting bandwidth for individual types");
	}

	return valid_arg;
}



static void print_bandwidth_args( struct ipt_bandwidth_info* info )
{
	if(info->cmp == BANDWIDTH_CHECK)
	{
		if(info->check_type == BANDWIDTH_CHECK_NOSWAP)
		{
			printf("--bcheck ");
		}
		else
		{
			printf("--bcheck_with_src_dst_swap ");
		}
	}
	printf("--id %s ", info->id);



	if(info->cmp != BANDWIDTH_CHECK)
	{
		/* determine current time in seconds since epoch, with offset for current timezone */
		int minuteswest = get_minutes_west();
		time_t now;
		time(&now);
		now = now - (minuteswest*60);

		if(info->type == BANDWIDTH_COMBINED)
		{
			printf("--type combined ");
		}
		if(info->type == BANDWIDTH_INDIVIDUAL_SRC)
		{
			printf("--type individual_src ");
		}
		if(info->type == BANDWIDTH_INDIVIDUAL_DST)
		{
			printf("--type individual_dst ");
		}
		if(info->type == BANDWIDTH_INDIVIDUAL_LOCAL)
		{
			printf("--type individual_local ");
		}
		if(info->type == BANDWIDTH_INDIVIDUAL_REMOTE)
		{
			printf("--type individual_remote ");
		}


		if(info->local_subnet != 0)
		{
			unsigned char* sub = (unsigned char*)(&(info->local_subnet));
			int msk_bits=0;
			int pow=0;
			for(pow=0; pow<32; pow++)
			{
				uint32_t test = get_pow(2, pow);
				msk_bits = ( (info->local_subnet_mask & test) == test) ? msk_bits+1 : msk_bits;
			}
			printf("--subnet %u.%u.%u.%u/%u ", (unsigned char)sub[0], (unsigned char)sub[1], (unsigned char)sub[2], (unsigned char)sub[3], msk_bits); 
		}
		if(info->cmp == BANDWIDTH_GT)
		{
			printf("--greater_than %lld ", info->bandwidth_cutoff);
		}
		if(info->cmp == BANDWIDTH_LT)
		{
			printf("--less_than %lld ", info->bandwidth_cutoff);
		}
		if (info->type == BANDWIDTH_COMBINED) /* too much data to print for multi types, have to use socket to get/set data */
		{
			if( info->reset_interval != BANDWIDTH_NEVER && info->next_reset != 0 && info->next_reset < now)
			{
				/* 
				 * current bandwidth only gets reset when first packet after reset interval arrives, so output
				 * zero if we're already past interval, but no packets have arrived 
				 */
				printf("--current_bandwidth 0 ");
			}
			else 
			{
				printf("--current_bandwidth %lld ", info->current_bandwidth);
			}
		}
		if(info->reset_is_constant_interval)
		{
			printf("--reset_interval %ld ", info->reset_interval);
		}
		else
		{
			if(info->reset_interval == BANDWIDTH_MINUTE)
			{
				printf("--reset_interval minute ");
			}
			else if(info->reset_interval == BANDWIDTH_HOUR)
			{
				printf("--reset_interval hour ");
			}
			else if(info->reset_interval == BANDWIDTH_DAY)
			{
				printf("--reset_interval day ");
			}
			else if(info->reset_interval == BANDWIDTH_WEEK)
			{
				printf("--reset_interval week ");
			}
			else if(info->reset_interval == BANDWIDTH_MONTH)
			{
				printf("--reset_interval month ");
			}
		}
		if(info->reset_time > 0)
		{
			printf("--reset_time %ld ", info->reset_time);
		}
		if(info->num_intervals_to_save > 0)
		{
			printf("--intervals_to_save %d ", info->num_intervals_to_save);
		}
	}
}

/* 
 * Final check, we can't have reset_time without reset_interval
 */
static void final_check(unsigned int flags)
{
	if (flags == 0)
	{
		param_problem_exit_error("You must specify at least one argument. ");
	}
	if( (flags & BANDWIDTH_RESET_INTERVAL) == 0 && (flags & BANDWIDTH_RESET_TIME) != 0)
	{
		param_problem_exit_error("You may not specify '--reset_time' without '--reset_interval' ");
	}
	if( (flags & BANDWIDTH_REQUIRES_SUBNET) == BANDWIDTH_REQUIRES_SUBNET && (flags & BANDWIDTH_SUBNET) == 0 )
	{
		param_problem_exit_error("You must specify a local subnet (--subnet a.b.c.d/mask) to match individual local/remote IPs ");
	}

	/* update timezone minutes_west in kernel to match userspace*/
	set_kernel_timezone();
}

/* Prints out the matchinfo. */
#ifdef _XTABLES_H
static void print(const void *ip, const struct xt_entry_match *match, int numeric)
#else	
static void print(const struct ipt_ip *ip, const struct ipt_entry_match *match, int numeric)
#endif
{
	printf("bandwidth ");
	struct ipt_bandwidth_info *info = (struct ipt_bandwidth_info *)match->data;

	print_bandwidth_args(info);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
#ifdef _XTABLES_H
static void save(const void *ip, const struct xt_entry_match *match)
#else
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
#endif
{
	struct ipt_bandwidth_info *info = (struct ipt_bandwidth_info *)match->data;
	time_t now;
	
	print_bandwidth_args(info);
	
	time(&now);
	printf("--last_backup-time %ld ", now);
}

static struct iptables_match bandwidth = 
{ 
	.next		= NULL,
 	.name		= "bandwidth",
	#ifdef XTABLES_VERSION_CODE
		.version = XTABLES_VERSION, 
	#else
		.version = IPTABLES_VERSION,
	#endif
	.size		= XT_ALIGN(sizeof(struct ipt_bandwidth_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ipt_bandwidth_info)),
	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

void _init(void)
{
	register_match(&bandwidth);
}

static void param_problem_exit_error(char* msg)
{
	#ifdef xtables_error
		xtables_error(PARAMETER_PROBLEM, "%s", msg);
	#else
		exit_error(PARAMETER_PROBLEM, msg);
	#endif
}

/* 
 * implement a simple function to get positive powers of positive integers so we don't have to mess with math.h 
 * all we really need are powers of 2 for calculating netmask
 * This is only called a couple of times, so speed isn't an issue either
 */
static unsigned long get_pow(unsigned long base, unsigned long pow)
{
	unsigned long ret = pow == 0 ? 1 : base*get_pow(base, pow-1);
	return ret;
}


int parse_sub(char* subnet_string, uint32_t* subnet, uint32_t* subnet_mask)
{

	int valid = 0;
	unsigned int A,B,C,D,E,F,G,H;
	int read_int = sscanf(subnet_string, "%u.%u.%u.%u/%u.%u.%u.%u", &A, &B, &C, &D, &E, &F, &G, &H);
	if(read_int >= 5)
	{
		if( A <= 255 && B <= 255 && C <= 255 && D <= 255)
		{
			unsigned char* sub = (unsigned char*)(subnet);
			unsigned char* msk = (unsigned char*)(subnet_mask);
			
			*( sub ) = (unsigned char)A;
			*( sub + 1 ) = (unsigned char)B;
			*( sub + 2 ) = (unsigned char)C;
			*( sub + 3 ) = (unsigned char)D;

			if(read_int == 5)
			{
				unsigned int mask = E;
				if(mask <= 32)
				{
					int msk_index;
					for(msk_index=0; msk_index*8 < mask; msk_index++)
					{
						int bit_index;
						msk[msk_index] = 0;
						for(bit_index=0; msk_index*8 + bit_index < mask && bit_index < 8; bit_index++)
						{
							msk[msk_index] = msk[msk_index] + get_pow(2, 7-bit_index);
						}
					}
				}
				valid = 1;
			}
			if(read_int == 8)
			{
				if( E <= 255 && F <= 255 && G <= 255 && H <= 255)
				*( msk ) = (unsigned char)E;
				*( msk + 1 ) = (unsigned char)F;
				*( msk + 2 ) = (unsigned char)G;
				*( msk + 3 ) = (unsigned char)H;
				valid = 1;
			}
		}
	}
	if(valid)
	{
		*subnet = (*subnet & *subnet_mask );
	}
	return valid;
}



int get_minutes_west(void)
{
	time_t now;
	struct tm* utc_info;
	struct tm* tz_info;
	int utc_day;
	int utc_hour;
	int utc_minute;
	int tz_day;
	int tz_hour;
	int tz_minute;
	int minuteswest;

	time(&now);
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
	struct timeval tv;
	struct timezone old_tz;
	struct timezone new_tz;

	new_tz.tz_minuteswest = get_minutes_west();;
	new_tz.tz_dsttime = 0;

	/* Get tv to pass to settimeofday(2) to be sure we avoid hour-sized warp */
	/* (see gettimeofday(2) man page, or /usr/src/linux/kernel/time.c) */
	gettimeofday(&tv, &old_tz);

	/* set timezone */
	settimeofday(&tv, &new_tz);
}
