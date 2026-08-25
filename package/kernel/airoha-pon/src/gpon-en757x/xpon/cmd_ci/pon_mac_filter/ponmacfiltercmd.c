/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ponvlancmd.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Reid.Ma		2013/2/4	Create
*/

#include <asm/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "global.h"
#include "ponmacfiltercmd.h"

#define PONMACFILTERCMD_LOCK_FILE "/tmp/ponmacfilterlockfd"

static int do_enable(int argc, char *argv[], void *p);
static int do_disable(int argc, char *argv[], void *p);
static int do_add_rule(int argc, char *argv[], void *p);
static int do_del_rule(int argc, char *argv[], void *p);
static int do_clean_rule(int argc, char *argv[], void *p);
static int do_show_rule(int argc, char *argv[], void *p);
static int do_clean_all(int argc, char *argv[], void *p);
static int do_set_dbg_level(int argc, char *argv[], void *p);

static cmds_t ponMacFilterCmd[] = {
	{"enable",				do_enable,					0x10,   	0,  		NULL},
	{"disable",				do_disable,					0x10,   	0,  		NULL},
	{"addrule",				do_add_rule,				0x10,   	0,  		NULL},
	{"delrule",				do_del_rule,				0x10,   	0,  		NULL},
	{"cleanrule",			do_clean_rule,				0x10,   	0,  		NULL},
	{"showrule",			do_show_rule,				0x10,   	0,  		NULL},
	{"cleanall",			do_clean_all,				0x10,   	0,  		NULL},
	{"dbglevel",			do_set_dbg_level,			0x10,   	0,  		NULL},
	{NULL,					NULL,              				0x10,	0,		NULL},
};

#define isdigit(x)	((x)>='0'&&(x)<='9')

static inline __u32 atoui(char *s)
{
        __u32 i=0;
        while (isdigit(*s)) {
                i = i*10 + *(s++) - '0';
        }
        return i;
}

int convert_mac_to_array(char * format_mac,unsigned char * des)
{
	char * mac = format_mac;
	unsigned char * desmac = des;
	int i = 0;
	unsigned char tmp = 0;

	for(i = 0; i < 12; i++)
	{
		if(isdigit(*mac))
			tmp = (*mac - '0');
		else if(*mac >= 'a' && *mac <= 'f')
			tmp = (*mac - 'a' + 10);
		else if(*mac >= 'A' && *mac <= 'F')
			tmp = (*mac - 'A' + 10);
		else
			return -1;

		if(i%2 == 0)
			*desmac += tmp << 4;
		else
		{
			*desmac += tmp;
			desmac++;
		}
		mac += 1;
	}
	return 0;
}

static void show_port(void)
{
	printf("\r\nup port num define");
	printf("\r\n0~31 : ani port index");
	printf("\r\ndown port num define");
	printf("\r\n11~14: lan port 1~4");
	printf("\r\n21~24: wlan port 1~4");
	printf("\r\n30: usb");
	return;
}

int subcmd  (
    const cmds_t tab[], int argc, char *argv[], void *p
)
{
    register const cmds_t *cmdp;
    int found = 0;
    int i;

    /* Strip off first token and pass rest of line to subcommand */
    if (argc < 2) {
        if (argc < 1)
            printf("SUBCMD - Don't know what to do?\n");
    	else{
            argv++;
            goto print_out_cmds;
    	}
    	return -1;
    }
	argc--;
	argv++;
	for(cmdp = tab;cmdp->name != NULL;cmdp++){
	    if(strncmp(argv[0],cmdp->name,strlen(argv[0])) == 0){
	       found = 1;
	       break;
		}
	}
	if(!found){
        char buf[66];

print_out_cmds:
        printf("valid subcommands of %s:\n", (argv-1)[0]);
        memset(buf,' ',sizeof(buf));
        buf[64] = '\n';
        buf[65] = '\0';
        for(i=0,cmdp = tab;cmdp->name != NULL;cmdp++){
            strncpy(&buf[i*16],cmdp->name,strlen(cmdp->name));
            if(i == 3){
		       printf(buf);
		       memset(buf,' ',sizeof(buf));
		       buf[64] = '\n';
		       buf[65] = '\0';
		    }
		    i = (i+1)%4;
		}
		if(i != 0)
		   printf(buf);
		return -1;
	}
	if(argc <= cmdp->argcmin){
	    if(cmdp->argc_errmsg != NULL)
	       printf("Usage: %s\n",cmdp->argc_errmsg);
	    return -1;
	}
	return (*cmdp->func)(argc,argv,p);
}

int pon_mac_filter_ioctl(int cmd, void *data, int datealen)
{
	int fp = -1;
	int ret = -1;	
	fp = open("/dev/ponmacfilter", O_RDONLY);
	if(fp == -1)
		printf("\r\nponvlan mac filter ioctl file fail");
	ret = ioctl(fp, cmd, data);
	if(ret < 0)
	{
		printf("\r\npon mac filter ioctl error");
	}
	close(fp);
	return 0;
}

int do_enable(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd enable");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	
	data.enable_flag = ENABLE;
		
	data.option_flag = OPT_SET;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_disable(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd disable");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	
	data.enable_flag = DISABLE;
		
	data.option_flag = OPT_SET;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_add_rule(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	int tmp = 0,i = 0;
	
	if (argc != 9)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd addrule up/down port ruleType(0:unicast 1:multicast)");
		printf("\r\nruleOpt(0:forward 1:discard macType(0:des mac 1:src mac) ethertype(dec) macStart macEnd");
		printf("\r\nMAC format:12345678abcd");
		show_port();
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[2]);
	if(strcmp(argv[1],"up") == 0)
	{
		if(!isAniPort(tmp))
		{
			printf("\r\nani port error,should be 0~31");
			return -1;
		}
		data.ani_index = tmp;
		data.direction = UPSTREAM;
	}
	else if(strcmp(argv[1],"down") == 0)
	{
		if(!isUniPort(tmp))
		{
			printf("\r\nuni port error,should be 11~14,21~24,30");
			return -1;
		}
		data.uni_index = tmp;
		data.direction = DOWNSTREAM;
	}
	else
	{
		printf("\r\ndirecction error");
		return -1;
	}

	tmp = atoi(argv[3]);
	if(tmp < 0 || tmp > 1)
	{
		printf("\r\nfilter type error,should be 0~1");
		return -1;
	}
	data.rule_type = tmp;
	
	tmp = atoi(argv[4]);
	if(tmp < 0 || tmp > 1)
	{
		printf("\r\nfilter operation error,should be 0~1");
		return -1;
	}
	data.rule.filter_type = tmp;

	tmp = atoi(argv[5]);
	if(tmp < 0 || tmp > 1)
	{
		printf("\r\nmac type error,should be 0~1");
		return -1;
	}
	data.rule.mac_type = tmp;
	
	tmp = atoi(argv[6]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid EtherType value:%d",tmp);
		return -1;
	}
	data.rule.ethertype = tmp;

	if(strlen(argv[7]) != 12 || strlen(argv[8]) != 12)
	{
		printf("\r\nmac addr length error,should be 12");
		return -1;
	}
	
	for(i = 0; i < 12; i++)
	{
		if(argv[7][i] == argv[8][i])
			continue;
		
		if(argv[7][i] < argv[8][i])
			break;

		if(argv[7][i] > argv[8][i])
		{
			printf("\r\nstart mac should be <= end mac");
			return -1;
		}
	}
	if(i == 12 && argv[7][11] != argv[8][11])
	{
		printf("\r\nstart mac should be <= end mac");
		return -1;
	}
		
	convert_mac_to_array(argv[7],data.rule.start_mac);
	convert_mac_to_array(argv[8],data.rule.end_mac);

	data.option_flag = OPT_SET;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}	

int do_del_rule(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	int tmp = 0;
	
	if (argc != 5)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd delrule up/down port ruleType(0:unicast 1:multicast) index");
		show_port();
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[2]);
	if(strcmp(argv[1],"up") == 0)
	{
		if(!isAniPort(tmp))
		{
			printf("\r\nani port error,should be 0~31");
			return -1;
		}
		data.ani_index = tmp;
		data.direction = UPSTREAM;
	}
	else if(strcmp(argv[1],"down") == 0)
	{
		if(!isUniPort(tmp))
		{
			printf("\r\nuni port error,should be 11~14,21~24,30");
			return -1;
		}
		data.uni_index = tmp;
		data.direction = DOWNSTREAM;
	}
	else
	{
		printf("\r\ndirecction error");
		return -1;
	}

	tmp = atoi(argv[3]);
	if(tmp < 0 || tmp > 1)
	{
		printf("\r\nfilter type error,should be 0~1");
		return -1;
	}
	data.rule_type = tmp;
	
	tmp = atoi(argv[4]);
	if(!isZeroToFifteen(tmp))
	{
		printf("\r\nrule index error,should be 0~15");
		return -1;
	}
	data.rule_index = tmp;
	
	data.option_flag = OPT_DEL;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}


int do_clean_rule(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	int tmp = 0;
	
	if (argc != 4)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd cleanrule up/down port ruleType(0:unicast 1:multicast)");
		show_port();
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[2]);
	if(strcmp(argv[1],"up") == 0)
	{
		if(!isAniPort(tmp))
		{
			printf("\r\nani port error,should be 0~31");
			return -1;
		}
		data.ani_index = tmp;
		data.direction = UPSTREAM;
	}
	else if(strcmp(argv[1],"down") == 0)
	{
		if(!isUniPort(tmp))
		{
			printf("\r\nuni port error,should be 11~14,21~24,30");
			return -1;
		}
		data.uni_index = tmp;
		data.direction = DOWNSTREAM;
	}
	else
	{
		printf("\r\ndirecction error");
		return -1;
	}

	tmp = atoi(argv[3]);
	if(tmp < 0 || tmp > 1)
	{
		printf("\r\nfilter type error,should be 0~1");
		return -1;
	}
	data.rule_type = tmp;
	
	data.option_flag = OPT_CLEAN;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}


int do_show_rule(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd showrule up/down port");
		show_port();
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[2]);
	if(strcmp(argv[1],"up") == 0)
	{
		if(!isAniPort(tmp))
		{
			printf("\r\nani port error,should be 0~31");
			return -1;
		}
		data.ani_index = tmp;
		data.direction = UPSTREAM;
	}
	else if(strcmp(argv[1],"down") == 0)
	{
		if(!isUniPort(tmp))
		{
			printf("\r\nuni port error,should be 11~14,21~24,30");
			return -1;
		}
		data.uni_index = tmp;
		data.direction = DOWNSTREAM;
	}
	else
	{
		printf("\r\ndirecction error");
		return -1;
	}

	data.option_flag = OPT_SHOW;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}

int do_clean_all(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd cleanall");
		return 0;
	}
	memset(&data, 0, sizeof(data));
	
	data.option_flag = OPT_CLEAN_ALL;
	pon_mac_filter_ioctl(PONMACFILTER_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}


int do_set_dbg_level(int argc, char *argv[], void *p)
{
	pon_mac_filter_ioctl_data data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponmacfiltercmd dbglevel level(0~2)");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isZeroToThree(tmp))
	{
		printf("\r\ninvalid dbg value value:%d",tmp);
		return 0;
	}
	data.dbg_level = tmp;
	
	pon_mac_filter_ioctl(PONMACFILTER_IOC_DBG_LEVEL_OPT,&data,sizeof(data));

	return 0;
}

int main(int argc, char **argv) 
{

	void *p;
	int ret = -1;
	int pidfd;

	init_compileoption_val();
	pidfd =open(PONMACFILTERCMD_LOCK_FILE, O_RDWR | O_CREAT);
	if(pidfd < 0)
	{
		printf("\r\nopen lock file error!");
		ret = subcmd(ponMacFilterCmd, argc, argv, p);
	}
	else
	{
		writew_lock(pidfd,0,SEEK_SET,0);
		ret = subcmd(ponMacFilterCmd, argc, argv, p);
		un_lock(pidfd,0,SEEK_SET,0);
		//close(pidfd);
	}

	return ret;
}

