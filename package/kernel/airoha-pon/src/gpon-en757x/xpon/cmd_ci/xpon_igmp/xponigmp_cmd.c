/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
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
	xpon_igmp.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name				Date			Modification logs
	lidong.hu		2012/7/28	Create
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../lib/xpon_igmp/lib_xpon_igmp.h"

///////////////////////////////////////////////////////////////////////
#define MAX_CMD_LEN	256
#define NARG		40 
#define FALSE		0
#define TRUE		1

typedef	struct cmds_s
{
	char  *name;		
	int  (*func)(int argc,char *argv[],void *p);
	int  flags;	
	int  argcmin;
	char  *argc_errmsg;	
}cmds_t;

int subcmd(const cmds_t tab[], int argc, char *argv[], void *p)
{
	const cmds_t *cmdp;
	int found = 0;
	int i;

	/* Strip off first token and pass rest of line to subcommand */
	if (argc < 2) {
		if (argc < 1)
			printf("SUBCMD - Don't know what to do?\n");
		else 
			goto print_out_cmds;	
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
		printf("valid subcommands:\n");
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
	
	if(cmdp->func==NULL){
		return 0;
	}
	else{
		return (*cmdp->func)(argc,argv,p);
	}
}
////////////////////////////////////////////////////////////////////////
unsigned char  hex_to_dec(unsigned char ch)
{
	if (ch>='0' && ch<='9')
		return ch-'0';
	if (ch>='a' && ch<='f')
		return ch-'a'+10;
	if (ch>='A' && ch<='F')
		return ch-'A'+10;
	return 0;
}

int str_to_mac(unsigned char* mac,unsigned char* mac_str)
{
	int i;
	unsigned char ch = 0;
	
	if (mac_str == NULL || mac == NULL)
		return 0;
	
	if (strlen(mac_str) != 12)
		return 0;
	
	for(i=0;i<12;i++)
	{
		if (i%2)
			mac[i/2] = (ch << 4) +  hex_to_dec(mac_str[i]);
		else
			
ch = hex_to_dec(mac_str[i]);
	}
	return 0;
}

int mac_to_str(unsigned char* mac_str,unsigned char* mac)
{
	if (mac_str==NULL || mac == NULL)
		return 0;	
	sprintf(mac_str,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	return 0;
}

////////////////////////////////////////////////////////////////////////
static int port_index = 0;
static int doSetPortIndex(int argc, char *argv[], void *p)
{
	FILE* fd;
	port_index = atoi(argv[1]);
	fd = fopen("/etc/xpon_igmp.conf","w");
	if (fd != NULL)
	{
		fprintf(fd,"%d",port_index);
		fclose(fd);
	}
	return 0;
}
static int doSetVersion(int argc, char *argv[], void *p)
{
	int ver = atoi(argv[1]);
	return igmp_set_ver(port_index,ver);
}

static int doSetFunc(int argc, char *argv[], void *p)
{
	int func = atoi(argv[1]);
	return igmp_set_func(port_index,func);
}

static int doSetFastLeave(int argc, char *argv[], void *p)
{
	int fast = atoi(argv[1]);
	return igmp_set_fastleave(port_index,fast);
}

static int doSetUpTCI(int argc, char *argv[], void *p)
{
	int tci = atoi(argv[1]);
	return igmp_set_up_tci(port_index,tci);
}

static int doSetUpTagctl(int argc, char *argv[], void *p)
{
	int tagctl = atoi(argv[1]);
	return igmp_set_up_tagctrl(port_index,tagctl);
}

static int doSetDownTCI(int argc, char *argv[], void *p)
{
	int tci = atoi(argv[1]);
	return igmp_set_down_tci(port_index,tci);
}

static int doSetDownTagctl(int argc, char *argv[], void *p)
{

	int tagctl = atoi(argv[1]);
	return igmp_set_down_tagctrl(port_index,tagctl);
}

static int doSetMaxRate(int argc, char *argv[], void *p)
{
	int rate =  atoi(argv[1]);
	return igmp_set_maxrate(port_index,rate);	
}

static int doSetRobust(int argc, char *argv[], void *p)
{
	int robust =  atoi(argv[1]);
	return igmp_set_robust(port_index,robust);	
}

static int doSetUnauthor(int argc, char *argv[], void *p)
{
	int unauthor =  atoi(argv[1]);
	return igmp_set_unauthor(port_index,unauthor);	
}
static int is_ipv4_str(char* ip)
{
	if (strstr(ip,".") != NULL)
		return 1;
	return 0;
}

static int is_ipv4_addr(char* ip)
{
	int i;

	for(i=4;i<16;i++)
		if (ip[i] != 0)
			return 0;
	return 1;
}


static int doSetQueryIp(int argc, char *argv[], void *p)
{
	char queryip[16];
//	char ip[4];
	memset(queryip,0,16);

	if (is_ipv4_str(argv[1]))
		inet_pton(AF_INET,argv[1],queryip);
	else
		inet_pton(AF_INET6,argv[1],queryip);
	
	return igmp_set_query_ip(port_index,queryip);	
}

static int doSetQueryInterval(int argc, char *argv[], void *p)
{
	int interval =  atoi(argv[1]);
	return igmp_set_query_interval(port_index,interval);	
}

static int doSetLastInterval(int argc, char *argv[], void *p)
{
	int interval =  atoi(argv[1]);
	return igmp_set_last_interval(port_index,interval);	
}

static int doSetQueryMaxResp(int argc, char *argv[], void *p)
{
	int resp =  atoi(argv[1]);
	return igmp_set_query_maxresp(port_index,resp);	
}

static int doSetMaxPlayGroup(int argc, char *argv[], void *p)
{
	int maxgrp =  atoi(argv[1]);
	return igmp_set_max_playgroup(port_index,maxgrp);	
}

static int doSetTagStrip(int argc, char *argv[], void *p)
{
	int strip =  atoi(argv[1]);
	return igmp_set_tagstrip(port_index,strip);	
}

static int doSetPortFlag(int argc, char *argv[], void *p)
{
	int flag =  atoi(argv[1]);
	return igmp_set_port_flag(port_index,flag);	
}

static int doSetMaxBW(int argc, char *argv[], void *p)
{
	int maxbw = atoi(argv[1]);
	return igmp_set_max_bw(port_index,maxbw);	
}

static int doSetBWEnforcement(int argc, char *argv[], void *p)
{
	int bwe = atoi(argv[1]);
	return igmp_set_bwe(port_index,bwe);	
}

static int igmp_get_user_subscribe(int port_index)
{
	int i,cnt;
	int vid,gem,type,band;
	unsigned char srcip[16] = {0}, program_ip[16] = {0};
	unsigned char srcip_str[64] = {0}, programip_str[64] = {0};

	cnt = igmp_get_user_subscribe_cnt(port_index);
	printf("---------------------------------------------------\n");
	printf("User Subscribe Count = %d \n",cnt);
	printf("index\tsrc_ip\tprogram_ip\t\n");
	
	for(i=0;i<cnt;i++)
	{
		if (!igmp_get_user_subscribe_by_index(port_index, i, srcip, program_ip))
		{
    		if (is_ipv4_addr(srcip))
    		{
    			inet_ntop(AF_INET,srcip,srcip_str,64);
    			inet_ntop(AF_INET,program_ip,programip_str,64);
    		}
    		else
    		{
    			inet_ntop(AF_INET6,srcip,srcip_str,64);
    			inet_ntop(AF_INET6,program_ip,programip_str,64);
    		}
    		printf("%d\t%s\t%s\n",i,srcip_str,programip_str);
		}
		else
		    printf("%d\tERROR!\n",i);
	}
	return 0;
}

static int doSetDSBWCtrl(int argc, char *argv[], void *p)
{
	int enable = 0;
	if (argc>1)
	{
        enable = atoi(argv[1]);
      	igmp_set_xpon_ds_bw_ctrl(enable);
	}
	return 0;
}

static int doShowAll(int argc, char *argv[], void *p)
{
	char ip[16],ip_str[64];
// - start - 20140430 - carrylko - modify - //    
//	if (port_index<0 || port_index >8)
   	if ( port_index < 0 || port_index >8 && port_index != 21 ) {
// - end - 20140430 - carrylko - modify - //            
		printf("\n please set port inndex at first!");
		return 0;
	}
	
	igmp_get_query_ip(port_index,ip);
	if (is_ipv4_addr(ip))
		inet_ntop(AF_INET,ip,ip_str,64);
	else
		inet_ntop(AF_INET6,ip,ip_str,64);
	
//	printf("\n IGMP flag = %d",igmp_get_igmp_flag());
	printf("index = %d \n",port_index);
	printf("flag = %d \n",igmp_get_port_flag(port_index));
	printf("version = %d \n",igmp_get_ver(port_index));
	printf("function = %d \n",igmp_get_func(port_index));
	printf("fastleave = %d \n",igmp_get_fastleave(port_index));
	printf("upstreamtci = %d \n",igmp_get_up_tci(port_index));
	printf("upstreamtagctl = %d \n",igmp_get_up_tagctrl(port_index));
	printf("downstreamtci = %d \n",igmp_get_down_tci(port_index));
	printf("downstreamtagctl = %d \n",igmp_get_down_tagctrl(port_index));
	printf("maxrate = %d \n",igmp_get_maxrate(port_index));
	printf("robust = %d \n", igmp_get_robust(port_index));
	printf("unauthor = %d \n",igmp_get_unauthor(port_index));
	printf("queryip = %s \n",ip_str);
	printf("queryinterval = %d \n",igmp_get_query_interval(port_index));
	printf("lastinterval = %d \n", igmp_get_last_interval(port_index));
	printf("querymaxresp = %d \n", igmp_get_query_maxresp(port_index));
	printf("maxplaygroup = %d \n",igmp_get_max_playgroup(port_index));
	printf("tagstrip = %d \n", igmp_get_tagstrip(port_index));
	printf("max bandwidth = %d \n",igmp_get_max_bw(port_index));
	printf("bandwidth enforcement = %d \n",igmp_get_bwe(port_index));   
	printf("---------------------------------------------------\n");
	printf("current mcast bandwidth = %d \n", igmp_get_counter_curr_mcast_bw(port_index));
	printf("join messages counter = %d \n", igmp_get_counter_join_msg(port_index));
	printf("bandwidth exceeded counter = %d \n", igmp_get_counter_bw_exceeded(port_index));
	igmp_get_user_subscribe(port_index);
	return 0;
}

static int doPortConf(int argc, char *argv[], void *p);
static int doFwdTbl(int argc, char *argv[], void *p);
static int doMulVLan(int argc, char *argv[], void *p);

static int doSetIgmpFlag(int argc, char *argv[], void *p)
{
	int flag;
	if (argc==1)
	{
		flag = igmp_get_igmp_flag();
		printf("flag = %x \n",flag);
		return 0;
	}
	flag =  atoi(argv[1]);
	return igmp_set_igmp_flag(flag);
}

static int doSetXponMode(int argc, char *argv[], void *p)
{
	int mode;
	if(argc==1)
	{
		mode = igmp_get_xpon_mode();
		printf("mode = %d \n",mode);
		return;
	}
	mode = atoi(argv[1]);
	return igmp_set_xpon_mode(mode);
}

static int doSetOnuType(int argc, char *argv[], void *p)
{
	int type;
	if (argc==1)
	{
		type = igmp_get_onu_type();
		printf("type = %d \n",type);
		return 0;
	}
	return 0;
}
static int doSetDebug(int argc, char *argv[], void *p)
{
	int flag = 0;
	if (argc>1)
	{
		flag = atoi(argv[1]);
	}
	return igmp_set_xpon_debug(flag);
}

static int doClearHwEntry(int argc, char *argv[], void *p)
{
    igmp_clear_hw_entry();

    return 0;
}

static int doClearHwDropEntry(int argc, char *argv[], void *p)
{
    igmp_clear_hw_drop_entry();

    return 0;
}


static int doSetCareVerDyStalist(int argc, char *argv[], void *p)
{
	int flag = 0;
	if (argc>1)
	{
	
	flag = atoi(argv[1]);
	}
    
	return igmp_set_care_ver_dy_stalist(flag);
}

cmds_t Cmds[] = 
{
	{"flag",doSetIgmpFlag,0x12,0,""},
	{"mode",doSetXponMode,0x12,0,"mode [1|2] 1=GPON 2=EPON"},
	{"type",doSetOnuType,0x12,0,""},
	{"debug",doSetDebug,0x12,0,""},
	{"port",doPortConf,0x12,0,""},
	{"mvlan",doMulVLan,0x12,0,""},
	{"fwdtbl",doFwdTbl,0x12,0,""},
	{"clearhw",doClearHwEntry,0x12,0,""},
	{"clearhwdrop",doClearHwDropEntry,0x12,0,""},
	{"dsbwctrl",doSetDSBWCtrl,0x12,0,"DS MCast Rate Limit [0|1] 0=Disable 1=Enable"},
	{"carever",    doSetCareVerDyStalist,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}
};


static int doWhitelist(int argc, char *argv[], void *p);
static int doStaWhitelist(int argc, char *argv[], void *p);

static int doPortVLan(int argc, char *argv[], void *p);


cmds_t port_cmds[] = 
{
	{"index",doSetPortIndex,0x12,1,"set port index"},
	{"flag",doSetPortFlag,0x12,1,"set port flag"},	
	{"ver",doSetVersion,0x12,1,"setversion"},
	{"func",doSetFunc,0x12,1,"set function"},
	{"fastleave",doSetFastLeave,0x12,1,""},
	{"uptci",doSetUpTCI,0x12,1,""},
	{"uptagctl",doSetUpTagctl,0x12,1,""},
	{"downtci",doSetDownTCI,0x12,1,""},
	{"downtagctl",doSetDownTagctl,0x12,1,""},
	{"maxrate",doSetMaxRate,0x12,1,""},
	{"robust",doSetRobust,0x12,1,""},
	{"unauthor",doSetUnauthor,0x12,1,""},
	{"queryip",doSetQueryIp,0x12,1,""},
	{"queryinterval",doSetQueryInterval,0x12,1,""},
	{"lastinterval",doSetLastInterval,0x12,1,""},
	{"maxresp",doSetQueryMaxResp,0x12,1,""},
	{"maxgrp",doSetMaxPlayGroup,0x12,1,""},
	{"maxbw",doSetMaxBW,0x12,1,""},
	{"bwe",doSetBWEnforcement,0x12,1,""},
	{"tagstrip",doSetTagStrip,0x12,1,""},
	{"show",doShowAll,0x12,0,""},
	{"dynlist",doWhitelist,0x12,0,""},
	{"stalist",doStaWhitelist,0x12,0,""},	
	{"vlan",doPortVLan,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}
};



static int doAddWhiteList(int argc, char *argv[], void *p)
{
	int index,vid,gem,type,band;
	 char srcip[16],startip[16],endip[16];
	
	memset(srcip,0,16);
	memset(startip,0,16);
	memset(endip,0,16);
	index = atoi(argv[1]);
	type =  atoi(argv[2]);
	gem = atoi(argv[3]);
	vid = atoi(argv[4]);
	band =  atoi(argv[5]);

	if (is_ipv4_str(argv[6]))
	{
		inet_pton(AF_INET,argv[6],srcip);
		inet_pton(AF_INET,argv[7],startip);
		inet_pton(AF_INET,argv[8],endip);
	}
	else
	{
		inet_pton(AF_INET6,argv[6],srcip);
		inet_pton(AF_INET6,argv[7],startip);
		inet_pton(AF_INET6,argv[8],endip);
	}
	return igmp_add_dyn_whitelist(port_index,index,type,gem,vid,band,srcip,startip,endip);
}

static int doDelWhiteList(int argc, char *argv[], void *p)
{
	int index;

	index = atoi(argv[1]);
	return igmp_del_dyn_whitelist(port_index,index);
}

static int doClearWhiteList(int argc, char *argv[], void *p)
{
	return igmp_clear_dyn_whitelist(port_index);
}


static int doShowWhiteList(int argc, char *argv[], void *p)
{
	int i,cnt;
	int index,vid,gem,type,band;
	unsigned char  srcip[16],startip[16],endip[16];
	unsigned char  srcip_str[64],startip_str[64],endip_str[64];

	cnt = igmp_get_dyn_whitelist_cnt(port_index);
	
	printf("index = %d \n",port_index);
	printf("idx  index  type  gem  vid  band  srcip  startip   endip \n");
	
	for(i=0;i<cnt;i++)
	{
		igmp_get_dyn_whitelist(port_index,i,&index,&type,&gem,&vid,&band,srcip,startip,endip);
		if (is_ipv4_addr(srcip))
		{
			inet_ntop(AF_INET,srcip,srcip_str,64);
			inet_ntop(AF_INET,startip,startip_str,64);
			inet_ntop(AF_INET,endip,endip_str,64);	
		}
		else
		{
			inet_ntop(AF_INET6,srcip,srcip_str,64);
			inet_ntop(AF_INET6,startip,startip_str,64);
			inet_ntop(AF_INET6,endip,endip_str,64);		
		}
		printf("%d  %d  %d  %d  %d  %d   %s   %s   %s \n",i,index,type,gem,vid,band,srcip_str,startip_str,endip_str);
	}
	return 0;
}

cmds_t whitelist_cmds[] = 
{
	{"add",doAddWhiteList,0x12,8,"xpon_igmp port dynlist add <index> <type> <gem> <vid> <band> <src_ip> <grp_start> <grp_end>"},
	{"del",doDelWhiteList,0x12,1,"xpon_igmp port dynlist del <index>"},
	{"clear",doClearWhiteList,0x12,0,"xpon_igmp port dynlist clear"},
	{"show",doShowWhiteList,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}
};




static int doAddStaWhiteList(int argc, char *argv[], void *p)
{
	int index,vid,gem,type,band;
	unsigned char srcip[16],startip[16],endip[16];
	
	memset(srcip,0,16);
	memset(startip,0,16);
	memset(endip,0,16);
	index = atoi(argv[1]);
	type =  atoi(argv[2]);
	gem = atoi(argv[3]);
	vid = atoi(argv[4]);
	band =  atoi(argv[5]);
	
	if (is_ipv4_str(argv[6]))
	{
		inet_pton(AF_INET,argv[6],srcip);
		inet_pton(AF_INET,argv[7],startip);
		inet_pton(AF_INET,argv[8],endip);
	}
	else
	{
		inet_pton(AF_INET6,argv[6],srcip);
		inet_pton(AF_INET6,argv[7],startip);
		inet_pton(AF_INET6,argv[8],endip);
	}

	
	return igmp_add_sta_whitelist(port_index,index,type,gem,vid,band,srcip,startip,endip);
}

static int doDelStaWhiteList(int argc, char *argv[], void *p)
{
	int index;

	index = atoi(argv[1]);
	return igmp_del_sta_whitelist(port_index,index);
}

static int doClearStaWhiteList(int argc, char *argv[], void *p)
{
	return igmp_clear_sta_whitelist(port_index);
}

static int doShowStaWhiteList(int argc, char *argv[], void *p)
{
	int i,cnt;
	int index,vid,gem,type,band;
	unsigned char  srcip[16],startip[16],endip[16];
	unsigned char  srcip_str[64],startip_str[64],endip_str[64];

	cnt = igmp_get_sta_whitelist_cnt(port_index);
	
	printf("index = %d \n",port_index);
	printf("idx  index  type  gem  vid  band  srcip  startip   endip \n");
	
	for(i=0;i<cnt;i++)
	{
		igmp_get_sta_whitelist(port_index,i,&index,&type,&gem,&vid,&band,srcip,startip,endip);
		if (is_ipv4_addr(srcip))
		{
			inet_ntop(AF_INET,srcip,srcip_str,64);
			inet_ntop(AF_INET,startip,startip_str,64);
			inet_ntop(AF_INET,endip,endip_str,64);	
		}
		else
		{
			inet_ntop(AF_INET6,srcip,srcip_str,64);
			inet_ntop(AF_INET6,startip,startip_str,64);
			inet_ntop(AF_INET6,endip,endip_str,64);		
		}
		printf("%d  %d  %d  %d  %d  %d   %s   %s   %s \n",i,index,type,gem,vid,band,srcip_str,startip_str,endip_str);
	}
	return 0;
}



cmds_t staticlist_cmds[] = 
{
	{"add",doAddStaWhiteList,0x12,8,"xpon_igmp port stalist add <index> <type> <gem> <vid> <band> <src_ip> <grp_start> <grp_end>"},
	{"del",doDelStaWhiteList,0x12,1,"xpon_igmp port stalist del <index>"},
	{"clear",doClearStaWhiteList,0x12,0,"xpon_igmp port stalist clear"},
	{"show",doShowStaWhiteList,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}
};




static int doAddPortVLan(int argc, char *argv[], void *p)
{
	int vid;
	int switchid;

	vid = atoi(argv[1]);
	if (argc ==2)
		return igmp_add_portvlan(port_index,vid);
	
	if (argc==3)
		switchid = atoi(argv[2]);
	
	return igmp_set_portvlan_switchid(port_index,vid,switchid);
}

static int doDelPortVLan(int argc, char *argv[], void *p)
{
	int vid;
	if (argc != 2)
		return 0;
	vid = atoi(argv[1]);
	return igmp_del_portvlan(port_index,vid);
}

static int doClearPortVLan(int argc, char *argv[],void *p)
{
	return igmp_clear_portvlan(port_index);
}

static int doShowPortVLan(int argc, char *argv[], void *p)
{
	int i,cnt;
	int vid, newvid;
	printf("index = %d \n",port_index);
	printf("flag = %d \n",igmp_ge_portvlan_flag(port_index));
	printf("transcnt = %d \n",igmp_get_portvlan_switchcnt(port_index));
	printf("idx   vlan   transvlan \n");
	cnt = igmp_get_portvlan_cnt(port_index);
	for(i=0;i<cnt;i++)
	{
		igmp_get_portvlan_id(port_index,i,&vid,&newvid);
		printf("%d     %d     %d \n",i,vid,newvid);
	}
	return 0;
}

static int doSetPortVLanFlag(int argc, char *argv[], void *p)
{
	int flag;
	if (argc != 2)
		return 0;
	flag = atoi(argv[1]);
	return igmp_set_portvlan_flag(port_index,flag);
}


cmds_t portvlan_cmds[] = 
{
	{"add",doAddPortVLan,0x12,1,"xpon_igmp port vlan add <vid> [switchid]"},
	{"del",doDelPortVLan,0x12,1,"xpon_igmp port vlan del <vid>"},
	{"clear",doClearPortVLan,0x12,0,"xpon_igmp port vlan clear"},
	{"vflag",doSetPortVLanFlag,0x12,1,"xpon_igmp port vflag <0|1>"},
	{"show",doShowPortVLan,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}
};

static int doAddFwdEntry(int argc, char *argv[], void *p)
{
	int type,port,vid;
	unsigned char grp_addr[16], src_ip[16];
	

	memset(grp_addr,0,16);
	memset(src_ip,0,16);

	type = atoi(argv[1]);
	port = atoi(argv[2]);
	vid = atoi(argv[3]);
	
	if (type==0 || type==1 || type==2 || type==5)
	{
		str_to_mac(grp_addr,argv[4]);
		printf("str_to_mac(): %s  %02x:%02x:%02x:%02x:%02x:%02x",argv[4],
			grp_addr[0],grp_addr[1],grp_addr[2],grp_addr[3],grp_addr[4],grp_addr[5]);
		if (type==2)
		{
			inet_pton(AF_INET,argv[5],src_ip);
		}
		if (type==5)
		{
			inet_pton(AF_INET6,argv[5],src_ip);
		}
	}
	else if (type==3)
	{
		inet_pton(AF_INET,argv[4],grp_addr);
	}
	else if (type==4)
	{
		inet_pton(AF_INET6,argv[4],grp_addr);
	
	}
	else
	{
		return 0;
	}

	return igmp_add_fwdentry(type,port,vid,grp_addr,src_ip);
	
}

static int doDelFwdEntry(int argc, char *argv[], void *p)
{
	int type,port,vid,ip;
	unsigned char grp_addr[16], src_ip[16];

	type = atoi(argv[1]);
	port = atoi(argv[2]);
	vid = atoi(argv[3]);
	
	if (type==0 || type==1 || type==2 || type==5)
	{
		str_to_mac(grp_addr,argv[4]);
		if (type==2)
		{
			inet_pton(AF_INET,argv[5],src_ip);
		}
		if (type==5)
		{
			inet_pton(AF_INET6,argv[5],src_ip);
		}
	}
	else if (type==3)
	{
		inet_pton(AF_INET,argv[4],grp_addr);
	}
	else if (type==4)
	{
	
	inet_pton(AF_INET6,argv[4],grp_addr);
	}
	else
	{
		return 0;
	}

	return igmp_del_fwdentry(type,port,vid, grp_addr,src_ip);
}

static int doClearFwdEntry(int argc, char *argv[],void *p)
{
	return igmp_clear_fwdentry();
}

static int doShowFwdEntry(int argc, char *argv[], void *p)
{
	int i,cnt;
	int type,port,vid,ip,flag;
	unsigned char grp_addr[16],src_ip[16],grp_str[64],src_str[64];


	printf("Current Fwd Mode: %d \n",igmp_get_fwdmode());
	cnt = igmp_get_fwdentry_cnt();
	
	printf("idx  type  flag  port    vid     Grp_addr        Src_ip \n");
	
	for(i=0;i<cnt;i++)
	{
		igmp_get_fwdentry_ext(i,&type,&port,&vid,grp_addr,src_ip,&flag);
		
		if (type==0 || type==1 || type==2 || type==5)
		{
			mac_to_str(grp_str,grp_addr);
			if (type==2)
			{
				inet_ntop(AF_INET,src_ip,src_str,64);
			}
			if (type==5)
			{
				inet_ntop(AF_INET6,src_ip,src_str,64);
			}
		}
		else if (type==3 || type==6 )
		{
			inet_ntop(AF_INET,grp_addr,grp_str,64);
			inet_ntop(AF_INET,src_ip,src_str,64);
		}
		else if (type==4 || type==7)
		{
		
			inet_ntop(AF_INET6,grp_addr,grp_str,64);
			inet_ntop(AF_INET6,src_ip,src_str,64);
		}
		else
		{
			continue;
		}
		printf("%d    %d     %4x     %d     %d    %s   %s \n",i,type,flag,port,vid,grp_str,src_str);
	}
	return 0;
}

static int doSetFwdMode(int argc, char *argv[], void *p)
{
	int mode;
	if (argc != 2)
		return 0;
	mode = atoi(argv[1]);
	return igmp_set_fwdmode(mode);
}

cmds_t fwdtbl_cmds[] = 
{
	{"add",doAddFwdEntry,0x12,5,"xpon_igmp fwdtbl add <type> <port> <vid> <grp_ip> <src_ip>"},
	{"del",doDelFwdEntry,0x12,5,"xpon_igmp fwdtbl del <type> <port> <vid> <grp_ip> <src_ip>"},
	{"clear",doClearFwdEntry,0x12,0,"xpon_igmp fwdtbl clear"},
	{"mode",doSetFwdMode,0x12,1,"xpon_imgp mode <0|1>"},
	{"show",doShowFwdEntry,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}	
};



static int doAddMulVLan(int argc, char *argv[], void *p)
{
	int vid;
	if (argc != 2)
		return 0;
	vid = atoi(argv[1]);
	return igmp_add_mulvlan(vid);
}

static int doDelMulVLan(int argc, char *argv[], void *p)
{
	int vid;
	if (argc != 2)
		return 0;
	vid = atoi(argv[1]);
	return igmp_del_mulvlan(vid);
}

static int doClearMulVLan(int argc, char *argv[],void *p)
{
	return igmpONU_ClearMulVLan();
}

static int doShowMulVLan(int argc, char *argv[], void *p)
{
	int i,cnt;
	int vid, flag;
	printf("idx   mvlan_id   flag \n");
	cnt = igmp_get_mulvlan_cnt();
	for(i=0;i<cnt;i++)
	{
		vid = igmp_get_mulvlan_id(i);
		flag = igmp_get_mulvlan_flag(vid);
		printf("%d   %d     %d \n",i,vid,flag);
	}
	
	return 0;
}

static int doSetMulVLanFlag(int argc, char *argv[], void *p)
{
	int flag,vid;
	if (argc != 3)
		return 0;
	vid = atoi(argv[1]);
	flag = atoi(argv[2]);
	return igmp_set_mulvlan_flag(vid,flag);
}



cmds_t mulvlan_cmds[] = 
{
	{"add",doAddMulVLan,0x12,1,"xpon_igmp mvlan add <vid>"},
	{"del",doDelMulVLan,0x12,1,"xpon_igmp mvlan del <vid>"},
	{"clear",doClearMulVLan,0x12,0,"xpon_igmp mvlan clear"},
	{"mflag",doSetMulVLanFlag,0x12,1,"xpon_igmp mvlan mflag <>"},
	{"show",doShowMulVLan,0x12,0,""},
	{NULL,NULL,0x10,0,NULL}
};



static int doPortConf(int argc, char *argv[], void *p)
{
	return subcmd(port_cmds,argc,argv,p);
}

static int doStaWhitelist(int argc, char *argv[], void *p)
{
	return subcmd(staticlist_cmds,argc,argv,p);
}

static int doWhitelist(int argc, char *argv[], void *p)
{
	return subcmd(whitelist_cmds,argc,argv,p);
}

static int doPortVLan(int argc, char *argv[], void *p)
{
	return subcmd(portvlan_cmds,argc,argv,p);
}

static int doFwdTbl(int argc, char *argv[], void *p)
{
	return subcmd(fwdtbl_cmds,argc,argv,p);
}
static int doMulVLan(int argc, char *argv[], void *p)
{
	return subcmd(mulvlan_cmds,argc,argv,p);
}


int main(int argc,char* argv[])
{
	FILE* fd = fopen("/etc/xpon_igmp.conf","r+");
	if (fd != NULL)
	{
		fscanf(fd,"%d",&port_index);
		fclose(fd);
	}
	if(igmp_init()<0)
	{
		printf("\n xPON IGMP:igmpONU_init err");
		return 0;
	}
	subcmd(Cmds, argc, argv, NULL);
	return 0;
}







