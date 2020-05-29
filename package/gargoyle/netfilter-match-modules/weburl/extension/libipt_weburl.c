/*  weburl --	An iptables extension to match URLs in HTTP(S) requests
 *  		This module can match using string match or regular expressions
 *  		Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2008-2010 by Eric Bishop <eric@gargoyle-router.com>
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


/*
 * in iptables 1.4.0 and higher, iptables.h includes xtables.h, which
 * we can use to check whether we need to deal with the new requirements
 * in pre-processor directives below
 */
#include <iptables.h>  
#include <linux/netfilter_ipv4/ipt_weburl.h>

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



/* Function which prints out usage message. */
static void help(void)
{
	printf(	"weburl options:\n  --contains [!] [STRING]\n  --contains_regex [!] [REGEX]\n --matches_exactly [!] [STRING]\n --domain_only\n --path_only\n");
}

static struct option opts[] = 
{
	{ .name = "contains", 		.has_arg = 1, .flag = 0, .val = WEBURL_CONTAINS_TYPE },	//string
	{ .name = "contains_regex", 	.has_arg = 1, .flag = 0, .val = WEBURL_REGEX_TYPE },	//regex
	{ .name = "matches_exactly",	.has_arg = 1, .flag = 0, .val = WEBURL_EXACT_TYPE },	//exact string match
	{ .name = "domain_only",	.has_arg = 0, .flag = 0, .val = WEBURL_DOMAIN_PART },	//only match domain portion of url
	{ .name = "path_only",		.has_arg = 0, .flag = 0, .val = WEBURL_PATH_PART },	//only match path portion of url
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
	struct ipt_weburl_info *info = (struct ipt_weburl_info *)(*match)->data;
	int valid_arg = 0;

	if(*flags < 10)
	{
		info->match_part = WEBURL_ALL_PART;
	}

	switch (c)
	{
		case WEBURL_CONTAINS_TYPE:
		case WEBURL_REGEX_TYPE:
		case WEBURL_EXACT_TYPE:
			info->match_type = c;

			//test whether to invert rule
			my_check_inverse(optarg, &invert, &optind, 0);
			info->invert = invert ? 1 : 0;
	
			//test that test string is reasonable length, then to info
			int testlen = strlen(argv[optind-1]);
			if(testlen > 0 && testlen < MAX_TEST_STR)
			{
				strcpy(info->test_str, argv[optind-1]);
			}
			else if(testlen >= MAX_TEST_STR)
			{
				char err[100];
				sprintf(err, "Parameter definition is too long, must be less than %d characters", MAX_TEST_STR);
				param_problem_exit_error(err);
			}
			else
			{
				param_problem_exit_error("Parameter definition is incomplete");
			}

			if(*flags % 10 == 1)
			{
				param_problem_exit_error("You may only specify one string/pattern to match");
			}
			*flags = *flags + 1;
			
			valid_arg = 1;
			break;

		case WEBURL_DOMAIN_PART:
		case WEBURL_PATH_PART:
			info->match_part = c;
			if(*flags >= 10)
			{
				param_problem_exit_error("You may specify at most one part of the url to match:\n\t--domain_only, --path_only or neither (to match full url)\n");
			}
			*flags = *flags+10;
			
			valid_arg = 1;
			break;
	}
	
	return valid_arg;
}


	
static void print_weburl_args(	struct ipt_weburl_info* info )
{
	//invert
	if(info->invert > 0)
	{
		printf("! ");
	}
	//match type
	switch (info->match_type)
	{
		case WEBURL_CONTAINS_TYPE:
			printf("--contains ");
			break;
		case WEBURL_REGEX_TYPE:
			printf("--contains_regex ");
			break;
		case WEBURL_EXACT_TYPE:
			printf("--matches_exactly ");
			break;
	}
	//test string
	printf("%s ", info->test_str);

	//match part
	switch(info->match_part)
	{
		case WEBURL_DOMAIN_PART:
			printf("--domain_only ");
			break;
		case WEBURL_PATH_PART:
			printf("--path_only ");
			break;
		case WEBURL_ALL_PART:
			//print nothing
			break;
	}
	
}

/* Final check; must have specified a test string with either --contains or --contains_regex. */
static void final_check(unsigned int flags)
{
	if (flags %10 == 0)
	{
		param_problem_exit_error("You must specify '--contains' or '--contains_regex' or '--matches_exactly'");
	}
}

/* Prints out the matchinfo. */
#ifdef _XTABLES_H
static void print(const void *ip, const struct xt_entry_match *match, int numeric)
#else	
static void print(const struct ipt_ip *ip, const struct ipt_entry_match *match, int numeric)
#endif
{
	printf("WEBURL ");
	struct ipt_weburl_info *info = (struct ipt_weburl_info *)match->data;

	print_weburl_args(info);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
#ifdef _XTABLES_H
static void save(const void *ip, const struct xt_entry_match *match)
#else
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
#endif
{
	struct ipt_weburl_info *info = (struct ipt_weburl_info *)match->data;
	print_weburl_args(info);
}

static struct iptables_match weburl = 
{ 
	.next		= NULL,
 	.name		= "weburl",
	#ifdef XTABLES_VERSION_CODE
		.version = XTABLES_VERSION,
	#else
		.version = IPTABLES_VERSION,
	#endif
	.size		= XT_ALIGN(sizeof(struct ipt_weburl_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ipt_weburl_info)),
	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

void _init(void)
{
	register_match(&weburl);
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


