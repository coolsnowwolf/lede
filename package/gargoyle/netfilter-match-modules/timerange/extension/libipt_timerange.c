/*  timerange --	An iptables extension to match multiple timeranges within a week
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
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>


/*
 * in iptables 1.4.0 and higher, iptables.h includes xtables.h, which
 * we can use to check whether we need to deal with the new requirements
 * in pre-processor directives below
 */
#include <iptables.h>  
#include <linux/netfilter_ipv4/ipt_timerange.h>

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

/* utility functions necessary for module to work across multiple iptables versions */
static int  my_check_inverse(const char option[], int* invert, int *my_optind, int argc);
static void param_problem_exit_error(char* msg);


long* parse_time_ranges(char* time_ranges, unsigned char is_weekly_range);
void merge_adjacent_time_ranges(long* time_ranges, unsigned char is_weekly_range);
unsigned long parse_time(char* time_str);
long* parse_weekdays(char* wd_str);

char** split_on_separators(char* line, char* separators, int num_separators, int max_pieces, int include_remainder_at_max);
void to_lowercase(char* str);
char* trim_flanking_whitespace(char* str);

void set_kernel_timezone(void);

/* Function which prints out usage message. */
static void help(void)
{
	printf(	"timerange options:\n  --hours [HOURLY RANGES] --weekdays [WEEKDAYS ACTIVE] --weekly_ranges [WEEKLY RANGES]\n");
}

static struct option opts[] = 
{
	{ .name = "hours",        .has_arg = 1, .flag = 0, .val = HOURS },
	{ .name = "weekdays",     .has_arg = 1, .flag = 0, .val = WEEKDAYS },
	{ .name = "weekly_ranges", .has_arg = 1, .flag = 0, .val = WEEKLY_RANGE },
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
	struct ipt_timerange_info *info = (struct ipt_timerange_info *)(*match)->data;
	int valid_arg = 0;
	if(*flags == 0)
	{
		my_check_inverse(optarg, &invert, &optind, 0);
		info->invert = invert ? 1 : 0;
	}

	long* parsed = NULL;
	switch (c)
	{
		case HOURS:
			parsed = parse_time_ranges(argv[optind-1], 0);
			if(parsed != NULL && (*flags & HOURS) == 0 && (*flags & WEEKLY_RANGE) == 0)
			{
				int range_index = 0;
				for(range_index = 0; parsed[range_index] != -1; range_index++)
				{
					if(range_index > 100)
					{
						return 0;
					}
					info->ranges[range_index] = parsed[range_index];
				}
				info->ranges[range_index] = -1;
				free(parsed);


				valid_arg = 1;
				*flags = *flags+ c;
				info->type = *flags;
			}
			break;


		case WEEKDAYS:
			parsed = parse_weekdays(argv[optind-1]);
			if(parsed != NULL && (*flags & WEEKDAYS) == 0 && (*flags & WEEKLY_RANGE) == 0)
			{
				int day_index;
				for(day_index=0; day_index < 7; day_index++)
				{
					info->days[day_index] = parsed[day_index];
				}
				free(parsed);

				valid_arg = 1 ;
				*flags = *flags + c;
				info->type = *flags;
			}
			break;
		case WEEKLY_RANGE:
			parsed = parse_time_ranges(argv[optind-1], 1);
			if(parsed != NULL && (*flags & HOURS) == 0 && (*flags & WEEKDAYS) == 0 && (*flags & WEEKLY_RANGE) == 0 )
			{
				int range_index = 0;
				for(range_index = 0; parsed[range_index] != -1; range_index++)
				{
					if(range_index > 100)
					{
						return 0;
					}
					info->ranges[range_index] = parsed[range_index];
				
				}
				info->ranges[range_index] = -1;
				free(parsed);

				valid_arg = 1;
				*flags = *flags+c;
				info->type = *flags;
			}
			break;
	}

	return valid_arg;
}


	
static void print_timerange_args(	struct ipt_timerange_info* info )
{
	int i;
	
	if(info->invert == 1)
	{
		printf(" ! ");
	}

	switch(info->type)
	{
		case DAYS_HOURS:
		case HOURS:
			printf(" --hours ");
			for(i=0; info->ranges[i] != -1; i++)
			{
				printf("%ld", info->ranges[i]);
				if(info->ranges[i+1] != -1)
				{
					if(i % 2 == 0){ printf("-"); }
					else { printf(","); }
				}
			}
			if(info->type == HOURS) { break; }
		case WEEKDAYS:
			printf(" --weekdays ");
			for(i=0; i<7; i++)
			{
				printf("%d", info->days[i]);
				if(i != 6){ printf(","); }
			}
			break;
		case WEEKLY_RANGE:
			printf(" --weekly_ranges ");
			for(i=0; info->ranges[i] != -1; i++)
			{
				printf("%ld", info->ranges[i]);
				if(info->ranges[i+1] != -1)
				{
					if(i % 2 == 0){ printf("-"); }
					else { printf(","); }
				}
			}
			break;
	}
	printf(" ");
	
}

/* Final check; must have specified a test string with either --contains or --contains_regex. */
static void final_check(unsigned int flags)
{
	if(flags ==0)
	{
		param_problem_exit_error("Invalid arguments to time_range");
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
	printf("timerange ");
	struct ipt_timerange_info *info = (struct ipt_timerange_info *)match->data;

	print_timerange_args(info);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
#ifdef _XTABLES_H
static void save(const void *ip, const struct xt_entry_match *match)
#else
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
#endif
{
	struct ipt_timerange_info *info = (struct ipt_timerange_info *)match->data;
	print_timerange_args(info);
}

static struct iptables_match timerange = 
{ 
	.next		= NULL,
 	.name		= "timerange",
	#ifdef XTABLES_VERSION_CODE
		.version = XTABLES_VERSION, 
	#else
		.version = IPTABLES_VERSION,
	#endif
	.size		= XT_ALIGN(sizeof(struct ipt_timerange_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ipt_timerange_info)),
	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

void _init(void)
{
	register_match(&timerange);
}

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static int  my_check_inverse(const char option[], int* invert, int *my_optind, int argc)
{
	if (option && strcmp(option, "!") == 0)
	{
		if (*invert)
		{
			param_problem_exit_error("Multiple `!' flags not allowed");
		}
		*invert = TRUE;
		if (my_optind != NULL)
		{
			++*my_optind;
			if (argc && *my_optind > argc)
			{
				param_problem_exit_error("no argument following `!'");
			}
		}
		return TRUE;
	}
	return FALSE;
}
static void param_problem_exit_error(char* msg)
{
	#ifdef xtables_error
		xtables_error(PARAMETER_PROBLEM, "%s", msg);
	#else
		exit_error(PARAMETER_PROBLEM, msg);
	#endif
}

/* takes a string of days e.g. "Monday, Tuesday, Friday", and turns into an array of 7 longs
 * each 0 or 1, one for each weekday starting with sunday, e.g. [0,1,1,0,0,1,0] for our example 
 */
long* parse_weekdays(char* wd_str)
{
	long* weekdays = (long*)malloc(7*sizeof(long));
	weekdays[0] = weekdays[1] = weekdays[2] = weekdays[3] = weekdays[4] = weekdays[5] = weekdays[6] = 0;

	char** days = split_on_separators(wd_str, ",", 1, -1, 0);
	int day_index;
	int found = 0;
	for(day_index=0; days[day_index] != NULL; day_index++)
	{
		char day[4];
		trim_flanking_whitespace(days[day_index]);
		memcpy(day, days[day_index], 3);
		free(days[day_index]);
		day[3] = '\0';
		to_lowercase(day);
		if(strcmp(day, "sun") == 0)
		{
			weekdays[0] = 1;
			found = 1;
		}
		else if(strcmp(day, "mon") ==0)
		{
			weekdays[1] = 1;
			found = 1;
		}
		else if(strcmp(day, "tue") ==0)
		{
			weekdays[2] = 1;
			found = 1;
		}
		else if(strcmp(day, "wed") ==0)
		{
			weekdays[3] = 1;
			found = 1;
		}	
		else if(strcmp(day, "thu") ==0)
		{
			weekdays[4] = 1;
			found = 1;
		}
		else if(strcmp(day, "fri") ==0)
		{
			weekdays[5] = 1;
			found = 1;
		}
		else if(strcmp(day, "sat") ==0)
		{
			weekdays[6] = 1;
			found = 1;
		}
		else if(strcmp(day, "all") ==0)
		{
			weekdays[0] = weekdays[1] = weekdays[2] = weekdays[3] = weekdays[4] = weekdays[5] = weekdays[6] = 1;
			found = 1;
		}
	}
	free(days);
	if(found == 0)
	{
		free(weekdays);
		weekdays = NULL;
	}
	return weekdays;	
}


/* is_weekly_range indicates whether we're parsing hours within a single day or a range over a whole week */
long* parse_time_ranges(char* time_ranges, unsigned char is_weekly_range)
{
	char** pieces = split_on_separators(time_ranges, ",", 1, -1, 0);
	int num_pieces = 0;
	for(num_pieces = 0; pieces[num_pieces] != NULL; num_pieces++) {};
	long *parsed = (long*)malloc( (1+(num_pieces*2)) * sizeof(long));


	
	int piece_index = 0;
	for(piece_index = 0; pieces[piece_index] != NULL; piece_index++)
	{
		trim_flanking_whitespace(pieces[piece_index]);
		char** times=split_on_separators(pieces[piece_index], "-", 1, 2, 0);
		int time_count = 0;
		for(time_count = 0; times[time_count] != 0 ; time_count++){}
		if( time_count == 2 )
		{
			unsigned long  start = parse_time(trim_flanking_whitespace(times[0]));
			unsigned long end = parse_time(trim_flanking_whitespace(times[1]));
			parsed[ piece_index*2 ] = (long)start;
			parsed[ (piece_index*2)+1 ] = (long)end;

			free( times[1] );
		}
		if( time_count > 0) { free(times[0]); }

		free(times);
		free(pieces[piece_index]);
	}
	free(pieces);
	parsed[ (num_pieces*2) ] = -1; // terminated with -1 


	// make sure there is no overlap -- this will invalidate ranges 
	int range_index = 0;
	char overlap_found = 0;
	for(range_index = 0; range_index < num_pieces; range_index++)
	{
		// now test for overlap 
		long start1 = parsed[ (range_index*2) ];
		long end1 = parsed[ (range_index*2)+1 ];
		end1= end1 < start1 ? end1 + (is_weekly_range ? 7*24*60*60 : 24*60*60) : end1;
		
		int range_index2 = 0;
		for(range_index2 = 0; range_index2 < num_pieces; range_index2++)
		{
			if(range_index2 != range_index)
			{
				long start2 = parsed[ (range_index2*2) ];
				long end2 = parsed[ (range_index2*2)+1 ];
				end2= end2 < start2 ? end2 + (is_weekly_range ? 7*24*60*60 : 24*60*60) : end2;
				overlap_found = overlap_found || (start1 < end2 && end1 > start2 );
			}
		}
	}

	if(!overlap_found)
	{
		// sort ranges 
		int sorted_index = 0;
		while(parsed[sorted_index] != -1)
		{
			int next_start=-1;
			int next_start_index=-1;
			int test_index;
			long tmp1;
			long tmp2;
			for(test_index=sorted_index; parsed[test_index] != -1; test_index=test_index+2)
			{
				next_start_index = next_start < 0 || next_start > parsed[test_index] ? test_index : next_start_index;
				next_start = next_start < 0 || next_start > parsed[test_index] ? parsed[test_index] : next_start;
			}
			tmp1 = parsed[next_start_index];
			tmp2 = parsed[next_start_index+1];
			parsed[next_start_index] = parsed[sorted_index];
			parsed[next_start_index+1] = parsed[sorted_index+1];
			parsed[sorted_index] = 	tmp1;
			parsed[sorted_index+1] = tmp2;
			sorted_index = sorted_index + 2;
		}
	}
	else
	{
		// de-allocate parsed, set to NULL 
		free(parsed);
		parsed = NULL;
	}

	// merge time ranges where end of first = start of second 
	merge_adjacent_time_ranges(parsed, is_weekly_range);


	// if always active, free & return NULL 
	int max_multiple = is_weekly_range ? 7 : 1;
	if(parsed[0] == 0 && parsed[1] == max_multiple*24*60*60)
	{
		free(parsed);
		parsed = NULL;
	}


	//adjust so any range that crosses end of range is split in two
	int num_range_indices=0;
	for(num_range_indices=0; parsed[num_range_indices] != -1; num_range_indices++){}

	long* adjusted_range = (long*)malloc((3+num_range_indices)*sizeof(long));
	int ar_index = 0;
	int old_index = 0;
	if(parsed[num_range_indices-1] < parsed[0])
	{
		adjusted_range[0] = 0;
		adjusted_range[1] = parsed[num_range_indices-1];
		ar_index = ar_index + 2;
		parsed[num_range_indices-1] = -1;
	}
	for(old_index=0; parsed[old_index] != -1; old_index++)
	{
		adjusted_range[ar_index] = parsed[old_index];
		ar_index++;
	}

	if(ar_index % 2 == 1 )
	{
		adjusted_range[ar_index] = is_weekly_range ? 7*24*60*60 : 24*60*60;
		ar_index++;
	}
	adjusted_range[ar_index] = -1;
	free(parsed);
	
	return adjusted_range;
}



void merge_adjacent_time_ranges(long* time_ranges, unsigned char is_weekly_range)
{
	int range_length = 0;
	while(time_ranges[range_length] != -1){ range_length++; }
	int* merged_indices = (int*)malloc((range_length+1)*sizeof(int));
	
	int merged_index=0;
	int next_index;
	for(next_index=0; time_ranges[next_index] != -1; next_index++)
	{
		if(next_index == 0)
		{
			merged_indices[merged_index] = next_index;
			merged_index++;
		}
		else if( time_ranges[next_index+1] == -1 )
		{
			merged_indices[merged_index] = next_index;
			merged_index++;
		}
		else if( time_ranges[next_index] != time_ranges[next_index-1] && time_ranges[next_index] != time_ranges[next_index+1] )
		{
			merged_indices[merged_index] = next_index;
			merged_index++;
		}
	}
	merged_indices[merged_index] = -1;
	
	for(next_index=0; merged_indices[next_index] != -1; next_index++)
	{
		time_ranges[next_index] = time_ranges[ merged_indices[next_index] ];
	}
	time_ranges[next_index] = -1;
	free(merged_indices);

}




/* 
 * assumes 24hr time, not am/pm, in format:
 * (Day of week) hours:minutes:seconds
 * if day of week is present, returns seconds since midnight on Sunday
 * otherwise, seconds since midnight
 */
unsigned long parse_time(char* time_str)
{
	while((*time_str == ' ' || *time_str == '\t') && *time_str != '\0') { time_str++; }
	
	int weekday = -1;
	if(strlen(time_str) > 3)
	{
		char wday_test[4];
		memcpy(wday_test, time_str, 3);
		wday_test[3] = '\0';
		to_lowercase(wday_test);
		if(strcmp(wday_test, "sun") == 0)
		{
			weekday = 0;
		}
		else if(strcmp(wday_test, "mon") == 0)
		{
			weekday = 1;
		}
		else if(strcmp(wday_test, "tue") == 0)
		{
			weekday = 2;
		}
		else if(strcmp(wday_test, "wed") == 0)
		{
			weekday = 3;
		}
		else if(strcmp(wday_test, "thu") == 0)
		{
			weekday = 4;
		}
		else if(strcmp(wday_test, "fri") == 0)
		{
			weekday = 5;
		}
		else if(strcmp(wday_test, "sat") == 0)
		{
			weekday = 6;
		}
	}

	if(weekday >= 0)
	{
		time_str = time_str + 3;
		while( (*time_str < 48 || *time_str > 57) && *time_str != '\0') { time_str++; }
	}

	char** time_parts=split_on_separators(time_str, ":", 1, -1, 0);
	unsigned long seconds = weekday < 0 ? 0 : ( ((unsigned long)(weekday))*60*60*24 );
	unsigned long tmp;
	unsigned long multiple = 60*60;

	int tp_index = 0;
	for(tp_index=0; time_parts[tp_index] != NULL; tp_index++)
	{
		sscanf(time_parts[tp_index], "%ld", &tmp);
		seconds = seconds + (tmp*multiple);
		multiple = (unsigned long)(multiple/60);
		free(time_parts[tp_index]);
	}
	free(time_parts);

	return seconds;
}

void to_lowercase(char* str)
{
	int i;
	for(i = 0; str[i] != '\0'; i++)
	{
		str[i] = tolower(str[i]);
	}
}

/*
 * line_str is the line to be parsed -- it is not modified in any way
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
char** split_on_separators(char* line_str, char* separators, int num_separators, int max_pieces, int include_remainder_at_max)
{
	char** split;

	if(line_str != NULL)
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
			for(line_index = 0; line_str[line_index] != '\0'; line_index++)
			{
				int sep_index;
				int found = 0;
				for(sep_index =0; found == 0 && sep_index < num_separators; sep_index++)
				{
					found = separators[sep_index] == line_str[line_index] ? 1 : 0;
				}
				separator_count = separator_count+ found;
			}
			max_pieces = separator_count + 1;
		}
		split = (char**)malloc((1+max_pieces)*sizeof(char*));
		split_index = 0;
		split[split_index] = NULL;


		dup_line = strdup(line_str);
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


char* trim_flanking_whitespace(char* str)
{
	int new_start = 0;
	int new_length = 0;

	char whitespace[5] = { ' ', '\t', '\n', '\r', '\0' };
	int num_whitespace_chars = 4;
	
	
	int str_index = 0;
	int is_whitespace = 1;
	int test;
	while( (test = str[str_index]) != '\0' && is_whitespace == 1)
	{
		int whitespace_index;
		is_whitespace = 0;
		for(whitespace_index = 0; whitespace_index < num_whitespace_chars && is_whitespace == 0; whitespace_index++)
		{
			is_whitespace = test == whitespace[whitespace_index] ? 1 : 0;
		}
		str_index = is_whitespace == 1 ? str_index+1 : str_index;
	}
	new_start = str_index;


	str_index = strlen(str) - 1;
	is_whitespace = 1;
	while( str_index >= new_start && is_whitespace == 1)
	{
		int whitespace_index;
		is_whitespace = 0;
		for(whitespace_index = 0; whitespace_index < num_whitespace_chars && is_whitespace == 0; whitespace_index++)
		{
			is_whitespace = str[str_index] == whitespace[whitespace_index] ? 1 : 0;
		}
		str_index = is_whitespace == 1 ? str_index-1 : str_index;
	}
	new_length = str[new_start] == '\0' ? 0 : str_index + 1 - new_start;
	

	if(new_start > 0)
	{
		for(str_index = 0; str_index < new_length; str_index++)
		{
			str[str_index] = str[str_index+new_start];
		}
	}
	str[new_length] = 0;
	return str;
}

void set_kernel_timezone(void)
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

	struct timeval tv;
	struct timezone old_tz;
	struct timezone new_tz;

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
	new_tz.tz_minuteswest = minuteswest;
	new_tz.tz_dsttime = 0;

	/* Get tv to pass to settimeofday(2) to be sure we avoid hour-sized warp */
	/* (see gettimeofday(2) man page, or /usr/src/linux/kernel/time.c) */
	gettimeofday(&tv, &old_tz);

	/* set timezone */
	settimeofday(&tv, &new_tz);

}
