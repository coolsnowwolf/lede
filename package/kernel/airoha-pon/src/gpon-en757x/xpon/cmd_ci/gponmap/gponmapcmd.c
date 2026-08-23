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
	gponmapcmd.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/20	Create
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "libgponmap.h"

#define GPONMAPCMD_LOCK_FILE "/tmp/gponmaplockfd"
typedef struct {
    char *name;
    int (*func)(int argc,char *argv[],void *p);
    int flags;
    int argcmin;
    char *argc_errmsg;
} cmds_t;

#define writewLock(fd,offset,whence,len) \
		lockReg(fd,F_SETLKW,F_WRLCK,offset,whence,len)

#define unLock(fd,offset,whence,len) \
		lockReg(fd,F_SETLK,F_UNLCK,offset,whence,len)
		
static int lockReg(int fd,int cmd,int type,off_t offset,int whence,int len)
{
	struct flock lock;
	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	return (fcntl(fd,cmd,&lock));
}


static int doAddGemPortRule(int argc, char *argv[], void *p);
static int doDelGemPortRule(int argc, char *argv[], void *p);
static int doAddPQRule(int argc, char *argv[], void *p);
static int doDelPQRule(int argc, char *argv[], void *p);
static int doShowGemPortRule(int argc, char *argv[], void *p);
static int doShowQueueRule(int argc, char *argv[], void *p);
#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
static int doEnableDownQueueMap(int argc, char *argv[], void *p);
static int doEnableDownTrtcmMap(int argc, char *argv[], void *p);
static int doDisableDownMap(int argc, char *argv[], void *p);
static int doAddDownRule(int argc, char *argv[], void *p);
static int doDelDownRule(int argc, char *argv[], void *p);
static int doCleanDownRule(int argc, char *argv[], void *p);
static int doShowDownRule(int argc, char *argv[], void *p);
#endif


static cmds_t gponMapCmds[] = {
	/* ---------Gpon mapping cmds--------- */
	{"addGemPortRule",    doAddGemPortRule,    0x10, 0, NULL},
	{"delGemPortRule",    doDelGemPortRule,    0x10, 0, NULL},
	{"addQueueRule",    doAddPQRule,    0x10, 0, NULL},
	{"delQueueRule",    doDelPQRule,    0x10, 0, NULL},
	{"showGemPortRule",    doShowGemPortRule,    0x10, 0, NULL},	
	{"showQueueRule",    doShowQueueRule,    0x10, 0, NULL},	
#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
	{"enableDownQueue",		doEnableDownQueueMap,	0x10, 0, NULL},
	{"enableDownTrtcm",		doEnableDownTrtcmMap,	0x10, 0, NULL},
	{"disableDownMap",		doDisableDownMap,	0x10, 0, NULL},
	{"addDownRule", 		doAddDownRule,		0x10, 0, NULL},
	{"delDownRule", 		doDelDownRule,		0x10, 0, NULL},
	{"cleanDownRule",		doCleanDownRule,	0x10, 0, NULL},
	{"showDownRule",		doShowDownRule, 	0x10, 0, NULL},
#endif

	{NULL,               NULL,               0x10, 0, NULL},
};


int subcmd(const cmds_t tab[], int argc, char *argv[], void *p)
{
	register const cmds_t *cmdp;
	int found = 0;
	int i;

	/* Strip off first token and pass rest of line to subcommand */
	if (argc < 2){
		if (argc < 1)
			printf("SUMCMD - Don't know what to do?\n");
		else{
			argv ++;
			goto print_out_cmds;
		}
		return -1;
	}
	argc --;
	argv ++;
	for(cmdp = tab; cmdp->name != NULL; cmdp++){
		if (strncmp(argv[0], cmdp->name, strlen(argv[0])) == 0){
			found = 1;
			break;
		}
	}
	if (!found){
		char buf[66];
		
		print_out_cmds:
		printf("valid subcommands of %s:\n", (argv-1)[0]);

		memset(buf, ' ', sizeof(buf));
		buf[64] = '\n';
		buf[65] = '\0';

		for (i = 0, cmdp = tab; cmdp->name != NULL; cmdp++){
			strncpy(&buf[i*16], cmdp->name, strlen(cmdp->name));
			if (i == 3){
				printf(buf);
				memset(buf, ' ', sizeof(buf));
				buf[64] = '\n';
				buf[65] = '\0';
			}
			i = (i+1)%4;
		}
		if (i != 0)
			printf(buf);
		return -1;
	}
	if (argc <= cmdp->argcmin){
		if (cmdp->argc_errmsg != NULL)
			printf("Usage: %s\n", cmdp->argc_errmsg);
		return -1;
	}
	return (*cmdp->func)(argc, argv, p);
}

int doAddGemPortRule(int argc, char *argv[], void *p)
{
	int tmp = 0;
	int i = 0;
	gemPortMappingIoctl_t data;

	if (argc < 7){
		printf("Usage: addGemPortRule [tagCtl] [tagFlag] [userPort] [vid] [dscp] [pbit] [gemPort] \n");
		return 0;
	}

	printf("argc = %x \n", argc);

	memset(&data, 0, sizeof(data));
	for (i = 1; i < argc; i = i+ 2)
	{
		printf("argv[%d] = %s, argv[%d] = %s\n", i, argv[i], i+1,argv[i+1]);
		if(strcmp(argv[i], "tagCtl") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%x", &tmp);
			data.tagCtl = tmp;
		}
		else if(strcmp(argv[i], "tagFlag") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.tagFlag = tmp;
		}
		else if(strcmp(argv[i], "userPort") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.userPort = tmp;
		}
		else if(strcmp(argv[i], "vid") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.vid = tmp;
		}
		else if(strcmp(argv[i], "dscp") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.dscp = tmp;
		}
		else if(strcmp(argv[i], "pbit") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.pbit = tmp;
		}
		else if(strcmp(argv[i], "gemPort") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.gemPort = tmp;
		}
	}

	
	printf("------------IOCTL----------------------\n");
	printf("addGemPortRule\n");
	printf("tagCtl = %x\n", data.tagCtl);	
	printf("tagFlag = %d\n", data.tagFlag);	
	printf("userPort = %d\n", data.userPort);	
	printf("vid = %d\n", data.vid);	
	printf("dscp = %d\n", data.dscp);	
	printf("pbit = %d\n", data.pbit);	
	printf("gemPort = %d\n", data.gemPort);	
	fflush(stdout);
	sleep(1);

	gponmapIoctl(GEMPORT_MAPPING_ADD_ENTRY, (void *)&data, sizeof(data));

	return 0;
}

int doDelGemPortRule(int argc, char *argv[], void *p)
{
	int tmp = 0;
	int i = 0;
	gemPortMappingIoctl_t data;


	if (argc < 7){
		printf("Usage: addGemPortRule [tagCtl] [tagFlag] [userPort] [vid] [dscp] [pbit] [gemPort] \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	for (i = 1; i < argc; i = i+ 2)
	{
		printf("argv[%d] = %s, argv[%d] = %s\n", i, argv[i], i+1,argv[i+1]);
		if(strcmp(argv[i], "tagCtl") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%x", &tmp);
			data.tagCtl = tmp;
		}
		else if(strcmp(argv[i], "tagFlag") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.tagFlag = tmp;
		}
		else if(strcmp(argv[i], "userPort") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.userPort = tmp;
		}
		else if(strcmp(argv[i], "vid") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.vid = tmp;
		}
		else if(strcmp(argv[i], "dscp") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.dscp = tmp;
		}
		else if(strcmp(argv[i], "pbit") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.pbit = tmp;
		}
		else if(strcmp(argv[i], "gemPort") == 0)
		{
			tmp = 0;
			sscanf(argv[i+1], "%d", &tmp);
			data.gemPort = tmp;
		}
	}

	

	printf("------------IOCTL----------------------\n");
	fflush(stdout);
	sleep(1);

	gponmapIoctl(GEMPORT_MAPPING_DEL_ENTRY, (void *)&data, sizeof(data));

	return 0;
}
int doAddPQRule(int argc, char *argv[], void *p)
{
	int tmp = 0;
	gponQueueMappingIoctl_t data;


	//addpqrule [gemPort] [mode][tcont] [queue]
	if (argc < 8){
		printf("Usage: addpqrule [gemPort][gemPortType] [pqMode][allocId] [queue][tsEnable][tsChannelId]\n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	data.gemPort = tmp;

	tmp = atoi(argv[2]);
	if (tmp > 2){
		printf("Error gemPortType : %d, must be 0~1!\n", tmp);
		return 0;
	}
	data.gemType = tmp;


	tmp = atoi(argv[3]);
	if (tmp > 2){
		printf("Error pqMode : %d, must be 0~1!\n", tmp);
		return 0;
	}
	data.pqMode= tmp;	
	
	tmp = atoi(argv[4]);
	data.allocId = tmp;
	
	tmp = atoi(argv[5]);
	if (tmp > 7){
		printf("Error queue id: %d, must be 0~7!\n", tmp);
		return 0;
	}
	data.queue = tmp;

	tmp = atoi(argv[6]);
	if (tmp > 2){
		printf("Error tsEnable : %d, must be 0~1!\n", tmp);
		return 0;
	}
	data.tsEnable= tmp;

	tmp = atoi(argv[7]);
	if (tmp > 31){
		printf("Error ts channel id: %d, must be 0~31!\n", tmp);
		return 0;
	}
	data.tsChannelId = tmp;

	printf("------------IOCTL----------------------\n");
	fflush(stdout);
	sleep(1);

	gponmapIoctl(QUEUE_MAPPING_ADD_ENTRY, (void *)&data, sizeof(data));

	return 0;
}

int doDelPQRule(int argc, char *argv[], void *p)
{	
	int tmp = 0;
	gponQueueMappingIoctl_t data;


	//delrule [gemPort]
	if (argc != 2){
		printf("Usage: delQueueRule [gemPort]\n");
		return 0;
	}


	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	data.gemPort = tmp;

	printf("------------IOCTL----------------------\n");
	fflush(stdout);
	sleep(1);

	gponmapIoctl(QUEUE_MAPPING_DEL_ENTRY, (void *)&data, sizeof(data));

	return 0;
}

int doShowGemPortRule(int argc, char *argv[], void *p)
{
	return displayAllGemPortMappingRule();
}
int doShowQueueRule(int argc, char *argv[], void *p)
{
	return displayAllQueueMappingRule();
}

#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
int doEnableDownQueueMap(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	if (argc > 1)
	{
		printf("\r\n don't need paramater");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	data.downstream_mapping_switch = ENABLE;
	data.option_flag = OPT_SET;
	gponmapIoctl(DOWNSTREAM_SWITCH_OPT, (void *)&data, sizeof(data));
}

int doEnableDownTrtcmMap(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	if (argc > 1)
	{
		printf("\r\n don't need paramater");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	data.downstream_mapping_switch = DS_TRTCM_ENABLE;
	data.option_flag = OPT_SET;
	gponmapIoctl(DOWNSTREAM_SWITCH_OPT, (void *)&data, sizeof(data));
}


int doDisableDownMap(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	if (argc > 1)
	{
		printf("\r\n don't need paramater");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	data.downstream_mapping_switch = DISABLE;
	data.option_flag = OPT_SET;
	gponmapIoctl(DOWNSTREAM_SWITCH_OPT, (void *)&data, sizeof(data));
}

int doAddDownRule(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	int tmp;
	char tmpc;
	
	if (argc != 7)
	{
		printf("Usage: addDownRule [Gem_Port] [if_mask(0x0f)] [Queue_Index] [Trtcm_Id] [QueueEnabke(0/1)] [GemPort_Enable(0/1)]\n");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	tmp = atoi(argv[1]);
	if(tmp < 0 || tmp > 4095)
	{
		printf("\r\nGem port num error:0~4095");
		return 0;
	}
	data.gem_port_num = tmp;

	if(argv[2][0] != '0' || argv[2][1] != 'x')
		return -1;
	tmpc = argv[2][2];
	if(tmpc >= '0' && tmpc <= '9')
		tmp = (tmpc - '0') << 4;
	else if(tmpc >= 'a' && tmpc <= 'f')
		tmp = (tmpc - 'a' + 10) << 4;
	else if(tmpc >= 'A' && tmpc <= 'F')
		tmp = (tmpc - 'A' + 10) << 4;
	else
		return -1;
	tmpc = argv[2][3];
	if(tmpc >= '0' && tmpc <= '9')
		tmp += (tmpc - '0');
	else if(tmpc >= 'a' && tmpc <= 'f')
		tmp += (tmpc - 'a' + 10);
	else if(tmpc >= 'A' && tmpc <= 'F')
		tmp += (tmpc - 'A' + 10);
	else
		return -1;

	data.if_mask = tmp;
	
	tmp = atoi(argv[3]);
	if(tmp < 0 || tmp > 7)
	{
		printf("\r\nQueue index error:0~7");
		return 0;
	}
	data.queue = tmp;

	tmp = atoi(argv[4]);
	if(tmp < 0 || tmp > 31)
	{
		printf("\r\nTrtcm index error:0~31");
		return 0;
	}
	data.trtcmId = tmp;

	tmp = atoi(argv[5]);
	if(tmp == 1)
	    data.ds_pq_enable = 1;
	else
	    data.ds_pq_enable = 0;

	tmp = atoi(argv[6]);
	if(tmp == 1)
	    data.ds_trtcm_enable = 1;
	else
	    data.ds_trtcm_enable = 0;

	data.option_flag = OPT_SET;
	gponmapIoctl(DOWNSTREAM_MAPPING_RULE_OPT, (void *)&data, sizeof(data));
}

int doDelDownRule(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	int tmp;
	
	if (argc != 2)
	{
		printf("Usage: delDownRule [GemPort]\n");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	tmp = atoi(argv[1]);
	if(tmp < 0 || tmp > 4095)
	{
		printf("\r\nGem port num error:0~4095");
		return 0;
	}
	data.gem_port_num = tmp;

	data.option_flag = OPT_DEL;
	gponmapIoctl(DOWNSTREAM_MAPPING_RULE_OPT, (void *)&data, sizeof(data));
}

int doCleanDownRule(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	
	if (argc != 1)
	{
		printf("\r\n don't need paramater");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	data.option_flag = OPT_CLEAN;
	gponmapIoctl(DOWNSTREAM_MAPPING_RULE_OPT, (void *)&data, sizeof(data));
}

int doShowDownRule(int argc, char *argv[], void *p)
{
	gpon_downstream_mapping_ioctl data;
	
	if (argc != 1)
	{
		printf("\r\n don't need paramater");
		return 0;
	}
	memset(&data, 0, sizeof(data));

	data.option_flag = OPT_SHOW;
	gponmapIoctl(DOWNSTREAM_MAPPING_RULE_OPT, (void *)&data, sizeof(data));
}
#endif


int main(int argc, char **argv)
{
	void *p = NULL;
	int ret = -1, pidfd;

	init_compileoption_val();
	pidfd = open(GPONMAPCMD_LOCK_FILE, O_RDWR|O_CREAT);
	if (pidfd < 0){
		printf("\r\nOpen GPON mapping lock file error!\n");
		ret = subcmd(gponMapCmds, argc, argv, p);
	}
	else{
		writewLock(pidfd, 0, SEEK_SET, 0);
		ret = subcmd(gponMapCmds, argc, argv, p);
		unLock(pidfd, 0, SEEK_SET, 0);
	}
	return ret;
}

