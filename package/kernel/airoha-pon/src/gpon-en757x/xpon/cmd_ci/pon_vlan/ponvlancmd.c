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
#include "ponvlancmd.h"

#define PONVLANCMD_LOCK_FILE "/tmp/ponvlanlockfd"

static int do_enable(int argc, char *argv[], void *p);
static int do_disable(int argc, char *argv[], void *p);
static int do_veip_enable(int argc, char *argv[], void *p);
static int do_veip_disable(int argc, char *argv[], void *p);

static int do_set_downstream_mode(int argc, char *argv[], void *p);
static int do_enable_default_rule(int argc, char *argv[], void *p);
static int do_disable_default_rule(int argc, char *argv[], void *p);
static int do_add_sptpid(int argc, char *argv[], void *p);
static int do_del_sptpid(int argc, char *argv[], void *p);
static int do_set_iotpid(int argc, char *argv[], void *p);
static int do_set_dscp_map(int argc, char *argv[], void *p);
static int do_add_rule(int argc, char *argv[], void *p);
static int do_del_rule(int argc, char *argv[], void *p);
static int do_clean_rule(int argc, char *argv[], void *p);
static int do_clean_all_rule(int argc, char *argv[], void *p);
static int do_clean_hwnat(int argc, char *argv[], void *p);
static int do_show_rule(int argc, char *argv[], void *p);

static int do_enable_igmp_tagging(int argc, char *argv[], void *p);
static int do_disable_igmp_tagging(int argc, char *argv[], void *p);
static int do_set_igmp_mode(int argc, char *argv[], void *p);
static int do_set_igmp_tci(int argc, char *argv[], void *p);

static int do_enable_if_vlan(int argc, char *argv[], void *p);
static int do_disable_if_vlan(int argc, char *argv[], void *p);
static int do_set_if_vlan(int argc, char *argv[], void *p);

static int do_set_pcp_mode(int argc, char *argv[], void *p);

static int do_enable_mac_vlan(int argc, char *argv[], void *p);
static int do_disable_mac_vlan(int argc, char *argv[], void *p);
static int do_set_mac_vlan_time(int argc, char *argv[], void *p);

static int do_clean_class_reg(int argc, char *argv[], void *p);
static int do_show_class_reg(int argc, char *argv[], void *p);

static int do_enable_user_group(int argc, char *argv[], void *p);
static int do_disable_user_group(int argc, char *argv[], void *p);
static int do_set_user_group(int argc, char *argv[], void *p);
static int do_clean_user_group(int argc, char *argv[], void *p);

static int do_set_dbg_level(int argc, char *argv[], void *p);

#ifdef TCSUPPORT_PON_VLAN_FILTER
static int doAddVlanFilterRule(int argc, char *argv[], void *p);
static int doGetVlanFilterRule(int argc, char *argv[], void *p);
static int doDelVlanFilterRule(int argc, char *argv[], void *p);
static int doDispVlanFilterRule(int argc, char *argv[], void *p);
static int doSetVlanFilterDbgLevel(int argc, char *argv[], void *p);
#endif	
#ifdef TCSUPPORT_EPON_OAM_CTC
static int doEponVlan(int argc, char *argv[], void *p);
static int doEponVlanTransparent(int argc, char *argv[], void *p);
static int doEponVlanTag(int argc, char *argv[], void *p);
static int doEponVlanTranslation(int argc, char *argv[], void *p);
static int doEponVlanTrunk(int argc, char *argv[], void *p);
static int doEponVlanN1Translation(int argc, char *argv[], void *p);
static int doEponVlanShow(int argc, char *argv[], void *p);
#endif
#ifdef TCSUPPORT_PON_IP_HOST
#define isPortNum(x) ((x) == 0 || ((x) > 10 && (x) < 15) \
	|| ((x) > 20 && (x) < 25) || (x) == 30  || (x) == 40 || (x) == 60) 
#else
#define isPortNum(x) ((x) == 0 || ((x) > 10 && (x) < 15) \
	|| ((x) > 20 && (x) < 25) || (x) == 30  || (x) == 40)
#endif


#define isVIDValue(x) ((x) >= 0 && (x) <= 4096)

#define isTreatMethod(x) ((x) == 0 || (x) == 1  || (x) == 21 \
	|| (x) == 22 || (x) == 23 || (x) == 31 || (x) == 32 || (x) == 33 || (x) == 40 || (x) == 41  || (x) == 42)

#define isEtherType(x) ((x) >= 0 && (x) <= 0xFFFF)

#define isZeroToTwo(x) ((x) >= 0 && (x) < 3)

#define isZeroToThree(x) ((x) >= 0 && (x) < 4)

#define isZeroToEight(x) ((x) >= 0 && (x) < 9)

#define isZeroToTen(x) ((x) >= 0 && (x) < 11)

static cmds_t PonVlanCmds[] = {
	{"enable",				do_enable,					0x10,   	0,  		NULL},
	{"disable",				do_disable,					0x10,   	0,  		NULL},
	{"veipenable",				do_veip_enable,					0x10,   	0,  		NULL},
	{"veipdisable",				do_veip_disable,					0x10,   	0,  		NULL},
	{"setdsmode",				do_set_downstream_mode,		0x10,	0, 		NULL},
	{"enabledefrule",			do_enable_default_rule,			0x10,	0, 		NULL},
	{"disabledefrule",			do_disable_default_rule,			0x10,	0, 		NULL},
	{"addsptpid",				do_add_sptpid,					0x10,	0, 		NULL},
	{"delsptpid",				do_del_sptpid,					0x10,	0, 		NULL},
	{"setiotpid",				do_set_iotpid,					0x10,	0, 		NULL},
	{"setdscpmap",			do_set_dscp_map,    	  		0x10,	0,		NULL},
	/*
	fo:filter outer 		fi:filter inner 		tf:treat first		ts:treat second
	port 
	tagNum
	fotpid fopbit fodei fovid
	fitpid fipbit fidei fivid
	ethertype treatmethod
	tftpid tfpbit tfdei tovid
	tstpid tspbit tidei tivid
	priority
	*/
	{"addrule",					do_add_rule,      				0x10,		0,		NULL},
	{"delrule",					do_del_rule,      				0x10,		0,		NULL},
	{"cleanrule",				do_clean_rule,      			0x10,		0,		NULL},
	{"cleanall",				do_clean_all_rule,				0x10,		0,		NULL},
	{"cleanhwnat",				do_clean_hwnat,      			0x10,		0,		NULL},
	{"showrule",				do_show_rule,      				0x10,		0,		NULL},
	
	{"enableigmptag",			do_enable_igmp_tagging,			0x10,   	0,  	NULL},
	{"disableigmptag",			do_disable_igmp_tagging,		0x10,   	0,  	NULL},
	{"setigmpmode",				do_set_igmp_mode,				0x10,   	0,  	NULL},
	{"setigmptci",				do_set_igmp_tci,				0x10,   	0,  	NULL},

	{"enableifvlan",			do_enable_if_vlan,				0x10,   	0,  	NULL},
	{"disableifvlan",			do_disable_if_vlan,				0x10,   	0,  	NULL},
	{"setifvlan",				do_set_if_vlan,					0x10,   	0,  	NULL},
	
	{"setpcpmode",				do_set_pcp_mode,				0x10,		0,		NULL},

	{"enablemacvlan",			do_enable_mac_vlan,				0x10,		0,		NULL},
	{"disablemacvlan",			do_disable_mac_vlan,			0x10,		0,		NULL},
	{"setmacvlantime",			do_set_mac_vlan_time,			0x10,		0,		NULL},
	
	{"cleanclassreg",			do_clean_class_reg,				0x10,   	0,  	NULL},
	{"showclassreg",			do_show_class_reg,				0x10,   	0,  	NULL},

	{"enableusergp",			do_enable_user_group,			0x10,   	0,  	NULL},
	{"disableusergp",			do_disable_user_group,			0x10,   	0,  	NULL},
	{"setusergroup",			do_set_user_group,				0x10,   	0,  	NULL},
	{"cleanusergroup",			do_clean_user_group,			0x10,   	0,  	NULL},
	
	{"dbglevel",				do_set_dbg_level,				0x10,   	0,  	NULL},
#ifdef TCSUPPORT_PON_VLAN_FILTER
	{"addfilterrule",			doAddVlanFilterRule,			0x10,   	0,  	NULL},
	{"getfilterrule",			doGetVlanFilterRule,			0x10,   	0,  	NULL},
	{"delfilterrule",			doDelVlanFilterRule,			0x10,   	0,  	NULL},
	{"dispfilterrule",			doDispVlanFilterRule,			0x10,   	0,  	NULL},
	{"dbgfilterrule",			doSetVlanFilterDbgLevel,		0x10,   	0,  	NULL},
#endif
#ifdef TCSUPPORT_EPON_OAM_CTC
	{"epon", 					doEponVlan,					0x10, 	0,		NULL},
#endif
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

static void show_port(void)
{
	printf("\r\nport num define");
	printf("\r\n0: CPE");
	printf("\r\n11~14: lan port 1~4");
	printf("\r\n21~24: wlan port 1~4");
	printf("\r\n30: usb");
	printf("\r\n40: default rule");
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

int ponvlan_ioctl(int cmd, void *data, int datealen)
{
	int fp = -1;
	int ret = -1;	
	fp = open("/dev/ponvlan", O_RDONLY);
	if(fp == -1)
		printf("\r\nponvlan open ioctl file fail");
	ret = ioctl(fp, cmd, data);
	if(ret < 0)
	{
		printf("\r\nponvlan ioctl error");
	}
	close(fp);
	return 0;
}

int do_enable(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd enable");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	
	data.vlan_enable = ENABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_disable(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd disable");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	
	data.vlan_enable = DISABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_SWITCH_OPT,&data,sizeof(data));

	return 0;
}


int do_veip_enable(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd veipenable");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	
	data.veip_vlan_enable = ENABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_VEIP_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_veip_disable(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd veipdisable");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	
	data.veip_vlan_enable = DISABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_VEIP_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_set_downstream_mode(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd setdsmode portnum 0(transparent)|1(inverse of upstream)|2(strip outer tag");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	tmp = atoi(argv[2]);
	if(tmp < 0 || tmp > 2)
	{
		printf("\r\ninvalid mode value:%d",tmp);
		return 0;
	}
	data.downstream_mode = tmp;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_DOWNSTREAM_MODE_OPT,&data,sizeof(data));

	return 0;

}

int do_enable_default_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd enabledefrule portnum");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	data.default_vlan_rule_flag = ENABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_DEFAULT_RULE_FLAG_OPT,&data,sizeof(data));

	return 0;
}

int do_disable_default_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd disabledefrule portnum");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	data.default_vlan_rule_flag = DISABLE;

	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_DEFAULT_RULE_FLAG_OPT,&data,sizeof(data));

	return 0;
}

int do_add_sptpid(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd addsptpid tpid(0~0xFFFF)");
		return 0;
	}
	
	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid input TPID value:%d,should be 0~0xFFFF",tmp);
		return 0;
	}
	data.special_tpid= tmp;

	data.tpid_type = 0;
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_TPID_OPT,&data,sizeof(data));

	return 0;
}


int do_del_sptpid(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd delsptpid tpid(0~0xFFFF)");
		return 0;
	}
	
	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid input TPID value:%d",tmp);
		printf("\r\ninvalid input TPID value:%d,should be 0~0xFFFF",tmp);
		return 0;
	}
	data.special_tpid= tmp;

	data.tpid_type = 0;
	data.option_flag = OPT_DEL;
	ponvlan_ioctl(PONVLAN_IOC_TPID_OPT,&data,sizeof(data));

	return 0;
}



int do_set_iotpid(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 4)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd setiotpid portnum intpid(0~0xFFFF) outtpid(0~0xFFFF)");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	tmp = atoi(argv[2]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid input TPID value:%d,should be 0~0xFFFF",tmp);
		return 0;
	}
	data.input_tpid = tmp;

	tmp = atoi(argv[3]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid output TPID value:%d,should be 0~0xFFFF",tmp);
		return 0;
	}
	data.output_tpid = tmp;
	
	data.tpid_type = 1;
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_TPID_OPT,&data,sizeof(data));

	return 0;
}

int do_set_dscp_map(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	__u32 tmp = 0;
	int i = 0;
	
	if (argc != 8)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd setdscpmap portnum map1 map2 map3 map4 map5 map6(0~0xFFFFFFFF)");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	for(i = 0; i <6; i++)
	{
		tmp = atoui(argv[2 + i]);
		data.dscp_map[i] = tmp;
	}
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_DSCP_MAP_OPT,&data,sizeof(data));

	return 0;
}

int do_add_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0,tagnum = 0;
	
	if (argc != 22)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd addrule portnum tagnum(0~2)");
		printf("\r\nfilter_outer_tpid 0:don't care 1:0x8100 2:intpid");
		printf("\r\nfilter_outer_pri 0~7:pbit 8:don't care");
		printf("\r\nfilter_outer_dei 0~1:dei 2:don't care");
		printf("\r\nfilter_outer_vid 0~4095:vid 4096:don't care");
		printf("\r\nfilter_inner_tpid filter_inner_pri filter_inner_dei filter_inner_vid");
		printf("\r\nfilter_ethertype 0~0xffff");
		printf("\r\ntreatment_method 0:transparent 1:block");
		printf("\r\n2x:add x tags,x=1,2 or 23:add a tag and modify original outer tag");
		printf("\r\n3x:del x tags,x=1,2 or 33:del a tag and modify original inner tag");
		printf("\r\n4x:change tag info.x=0:inner,x=1:outer,x=2:both");
		printf("\r\ntreat_fst_tpid 0:0x8100 1:output tpid 2:copy inner 3:copy outer");
		printf("\r\ntreat_fst_pri 0~7:pbit 8:copy inner 9:copy outer 10:dscp mapping");
		printf("\r\ntreat_fst_dei 0,1:dei 2:copy inner 3:copy outer");
		printf("\r\ntreat_fst_vid 0~4095:vid 4096:copy inner 4097:copy outer");
		printf("\r\ntreat_sec_tpid treat_sec_pri treat_sec_dei treat_sec_vid");
		printf("\r\nrule_priority 0:high 1:low");
		return 0;
	}

	tagnum = atoi(argv[2]);
	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	tmp = atoi(argv[2]);
	if(!isZeroToTwo(tmp))
	{
		printf("\r\ninvalid tagnum value:%d",tmp);
		return 0;
	}
	data.rule.tag_num = tmp;

	//begin of filter outer
	tmp = atoi(argv[3]);
	if(!isZeroToTwo(tmp))
	{
		printf("\r\ninvalid filter outer TPID value:%d",tmp);
		return 0;
	}
	data.rule.filter_outer_tpid = tmp;

	tmp = atoi(argv[4]);
	if(!isZeroToEight(tmp))
	{
		printf("\r\ninvalid filter outer pbit value:%d",tmp);
		return 0;
	}
	data.rule.filter_outer_pri = tmp;

	tmp = atoi(argv[5]);
	if(!isZeroToTwo(tmp))
	{
		printf("\r\ninvalid filter outer DEI value:%d",tmp);
		return 0;
	}
	data.rule.filter_outer_dei = tmp;

	tmp = atoi(argv[6]);
	if(!isVIDValue(tmp))
	{
		printf("\r\ninvalid filter outer VID value:%d",tmp);
		return 0;
	}
	data.rule.filter_outer_vid = tmp;
	//end of filter outer

	//begin of filter inner
	tmp = atoi(argv[7]);
	if(!isZeroToTwo(tmp))
	{
		printf("\r\ninvalid filter inner TPID value:%d",tmp);
		return 0;
	}
	data.rule.filter_inner_tpid= tmp;

	tmp = atoi(argv[8]);
	if(!isZeroToEight(tmp))
	{
		printf("\r\ninvalid filter inner pbit value:%d",tmp);
		return 0;
	}
	data.rule.filter_inner_pri= tmp;

	tmp = atoi(argv[9]);
	if(!isZeroToTwo(tmp))
	{
		printf("\r\ninvalid filter inner DEI value:%d",tmp);
		return 0;
	}
	data.rule.filter_inner_dei= tmp;

	tmp = atoi(argv[10]);
	if(!isVIDValue(tmp))
	{
		printf("\r\ninvalid filter inner VID value:%d",tmp);
		return 0;
	}
	data.rule.filter_inner_vid = tmp;
	//end of filter inner

	tmp = atoi(argv[11]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid EtherType value:%d",tmp);
		return 0;
	}
	data.rule.filter_ethertype = tmp;

	tmp = atoi(argv[12]);
	if(!isTreatMethod(tmp))
	{
		printf("\r\ninvalid treat method value:%d",tmp);
		return 0;
	}
	if(tmp == 23 && tagnum == 0)
	{
		printf("\r\ninvalid treat method value:%d,packet tagnum is 0",tmp);
		return 0;
	}
	if(tmp == 33 && tagnum != 2)
	{
		printf("\r\ninvalid treat method value:%d,packet tagnum is not 2",tmp);
		return 0;
	}
	if(tmp/10 == 2 && tmp%10 > 3)
	{
		printf("\r\ninvalid treat method value:%d,add num larger than 3",tmp);
		return 0;
	}
	if(tmp/10 == 3 && tmp != 33 && tmp%10 > tagnum)
	{
		printf("\r\ninvalid treat method value:%d,del num larger than tag num",tmp);
		return 0;
	}
	if(tmp/10 == 4)
	{
		if(tagnum == 0)
		{
			printf("\r\ninvalid treat method value:%d,tag num is 0.",tmp);
			return 0;
		}
		else if(tagnum == 1)
		{
			if(tmp%10 == 1 || tmp%10 == 2)
			{
				printf("\r\ninvalid treat method value:%d,tag num is 1,no outer tag here",tmp);
				return 0;
			}
		}
	}
	data.rule.treatment_method = tmp;


	//begin of treat first
	tmp = atoi(argv[13]);
	if(!isZeroToThree(tmp))
	{
		printf("\r\ninvalid treat first TPID value:%d",tmp);
		return 0;
	}
	data.rule.add_fst_tpid= tmp;

	tmp = atoi(argv[14]);
	if(!isZeroToTen(tmp))
	{
		printf("\r\ninvalid treat first pbit value:%d",tmp);
		return 0;
	}
	data.rule.add_fst_pri = tmp;

	tmp = atoi(argv[15]);
	if(!isZeroToThree(tmp))
	{
		printf("\r\ninvalid treat first DEI value:%d",tmp);
		return 0;
	}
	data.rule.add_fst_dei = tmp;

	tmp = atoi(argv[16]);
	if(!(isVIDValue(tmp) || tmp == 4097))
	{
		printf("\r\ninvalid treat first VID value:%d",tmp);
		return 0;
	}
	data.rule.add_fst_vid = tmp;
	//end of treat first

	//begin of treat second
	tmp = atoi(argv[17]);
	if(!isZeroToThree(tmp))
	{
		printf("\r\ninvalid treat second TPID value:%d",tmp);
		return 0;
	}
	data.rule.add_sec_tpid = tmp;

	tmp = atoi(argv[18]);
	if(!isZeroToTen(tmp))
	{
		printf("\r\ninvalid treat second pbit value:%d",tmp);
		return 0;
	}
	data.rule.add_sec_pri = tmp;

	tmp = atoi(argv[19]);
	if(!isZeroToThree(tmp))
	{
		printf("\r\ninvalid treat second DEI value:%d",tmp);
		return 0;
	}
	data.rule.add_sec_dei = tmp;

	tmp = atoi(argv[20]);
	if(!(isVIDValue(tmp) || tmp == 4097))
	{
		printf("\r\ninvalid treat second VID value:%d",tmp);
		return 0;
	}
	data.rule.add_sec_vid = tmp;
	//end of treat second
	
	tmp = atoi(argv[21]);
	if(tmp != 0 && tmp != 1)
	{
		printf("\r\ninvalid rule priority value:%d",tmp);
		return 0;
	}
	data.rule.rule_priority = tmp;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}

int do_del_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd delrule portnum index(0~31)");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;
	
	if(argc == 3)
	{
		tmp = atoi(argv[2]);
		if(tmp < 0 || tmp > 31)
		{
			printf("\r\ninvalid rule index value:%d",tmp);
			return 0;
		}
		data.rule_index = tmp;
	}
	else
	{
		
	}
	
	data.option_flag = OPT_DEL;
	ponvlan_ioctl(PONVLAN_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}

int do_clean_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd cleanrule portnum");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;
	
	data.option_flag = OPT_CLEAN;
	ponvlan_ioctl(PONVLAN_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}

int do_clean_all_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd cleanall");
		return 0;
	}
	memset(&data, 0, sizeof(data));
	
	data.option_flag = OPT_CLEAN_ALL;
	ponvlan_ioctl(PONVLAN_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}


int do_clean_hwnat(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd cleanhwnat");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data.port = 50;
	
	data.option_flag = OPT_CLEAN;
	ponvlan_ioctl(PONVLAN_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}


int do_show_rule(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd showrule portnum");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	data.option_flag = OPT_SHOW;
	ponvlan_ioctl(PONVLAN_IOC_RULE_OPT,&data,sizeof(data));

	return 0;
}

int do_enable_igmp_tagging(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	data.igmp_enable = ENABLE;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IGMP_VLAN_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_disable_igmp_tagging(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	data.igmp_enable = DISABLE;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IGMP_VLAN_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_set_igmp_mode(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		return 0;
	}
	data.port = tmp;
	
	tmp = atoi(argv[2]);
	if(!isZeroToThree(tmp))
	{
		printf("\r\ninvalid mode value:%d",tmp);
		return 0;
	}
	data.igmp_mode = tmp;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IGMP_VLAN_MODE_OPT,&data,sizeof(data));

	return 0;
}

int do_set_igmp_tci(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		return 0;
	}
	data.port = tmp;
	
	tmp = atoi(argv[2]);
	if(!isEtherType(tmp))
	{
		printf("\r\ninvalid tci value:%d",tmp);
		return 0;
	}
	data.igmp_tci = tmp;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IGMP_VLAN_TCI_OPT,&data,sizeof(data));

	return 0;
}

int do_enable_if_vlan(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	data.if_vlan_switch = ENABLE;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IF_VLAN_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_disable_if_vlan(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	data.if_vlan_switch = DISABLE;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IF_VLAN_SWITCH_OPT,&data,sizeof(data));

	return 0;
}


int do_set_if_vlan(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = (argv[1][3] - '0')*10 + (argv[1][4] - '0');
	if(strncmp(argv[1],"nas",3) != 0)
	{
		printf("\r\ninvalid interface:%s",argv[1]);
		return 0;
	}
	if(tmp < 13 || tmp > 20)
	{
		printf("\r\ninvalid interface:%s",argv[1]);
		return 0;
	}
	tmp = (argv[1][6] - '0');
	if(tmp < 0 || tmp > 7)
	{
		printf("\r\ninvalid interface:%s",argv[1]);
		return 0;
	}
	strcpy(data.dev_name,argv[1]);
	
	tmp = atoi(argv[2]);
	if(tmp < 0 || tmp > 4097)
	{
		printf("\r\ninvalid vid value:%d",tmp);
		return 0;
	}
	data.dev_vid = tmp;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_IF_VLAN_RULE_OPT,&data,sizeof(data));

	return 0;
}

int do_set_pcp_mode(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd setpcpmode mode[0~3]");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(tmp < 0 || tmp > 3)
	{
		printf("\r\ninvalid mode value:%d",tmp);
		return 0;
	}
	data.pcp_mode = tmp;
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_PCP_MODE_OPT,&data,sizeof(data));

	return 0;
}


int do_enable_mac_vlan(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd enablemacvlan portnum");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	data.mac_bind_vid_enable = ENABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_MAC_BIND_VID_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

static int do_disable_mac_vlan(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd disablemacvlan portnum");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	data.mac_bind_vid_enable = DISABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_MAC_BIND_VID_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

static int do_set_mac_vlan_time(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd setmacvlantime time[0~3600(s)]");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(tmp < 0 || tmp > 3600)
	{
		printf("\r\ninvalid time value:%d,should be 0~3600",tmp);
	}
	
	data.mac_vlan_time = tmp;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_MAC_BIND_VID_TIME_OPT,&data,sizeof(data));

	return 0;
}


int do_clean_class_reg(int argc, char *argv[], void *p)
{
	__u32 addr = 0xbfb50200;
	int i = 0;
	char cmd[50];
	
	for(i = 0; i < 30; i++)
	{
		sprintf(cmd,"sys memwl %x 0",addr);
		system(cmd);
		addr += 4;
	}
	return 0;
}

int do_show_class_reg(int argc, char *argv[], void *p)
{
	__u32 addr = 0xbfb50200;
	int i = 0;
	char cmd[50];
	
	for(i = 0; i < 30; i++)
	{
		sprintf(cmd,"sys memrl %x",addr);
		system(cmd);
		addr += 4;
	}
	return 0;
}

int do_enable_user_group(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd enableusergp");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	data.user_group_enable = ENABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_USER_GROUP_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_disable_user_group(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd enableusergp");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	data.user_group_enable = DISABLE;
		
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_USER_GROUP_SWITCH_OPT,&data,sizeof(data));

	return 0;
}

int do_set_user_group(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 3)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd setusergroup portnum group");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	tmp = atoi(argv[1]);
	if(!isPortNum(tmp))
	{
		printf("\r\ninvalid port num value:%d",tmp);
		show_port();
		return 0;
	}
	data.port = tmp;

	if(strcmp(argv[2],"none") == 0)
	{
		data.user_group = 0xffffffff;
	}
	else
	{
		tmp = atoi(argv[2]);
		if(tmp < 0 || tmp > 0xffff)
			return 0;
		data.user_group = tmp;
	}

	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_USER_GROUP_RULE_OPT,&data,sizeof(data));

	return 0;
}


int do_clean_user_group(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 1)
	{
		printf("\r\nparamater number error");
		printf("\r\nUsage: ponvlancmd cleanusergroup");
		return 0;
	}

	memset(&data, 0, sizeof(data));
		
	data.option_flag = OPT_CLEAN_ALL;
	ponvlan_ioctl(PONVLAN_IOC_USER_GROUP_RULE_OPT,&data,sizeof(data));

	return 0;
}



int do_set_dbg_level(int argc, char *argv[], void *p)
{
	pon_vlan_ioctl data;
	int tmp = 0;
	
	if (argc != 2)
	{
		printf("\r\nparamater number error");
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
	
	data.option_flag = OPT_SET;
	ponvlan_ioctl(PONVLAN_IOC_DBG_LEVEL_OPT,&data,sizeof(data));

	return 0;
}

#ifdef TCSUPPORT_PON_VLAN_FILTER
/*******************************************************************************************
**function name
	updateOmci63365HwNatRule
**description:
	update 63365 hwnat rule, make hwnat restart study.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
	value:	return value
	meInstantPtr :	instance pointer
  	omciAttribute :	ME attribute pointer
  	flag: 0:get history data, 1:get current data
********************************************************************************************/
int updateOmci63365HwNatRule(void){
	int ret = -1;
#ifdef TCSUPPORT_RA_HWNAT
	system("/userfs/bin/hw_nat -U 0 0 0");
	sleep(1);
	system("/userfs/bin/hw_nat -U 15 15 15");
#endif
	ret = 0;
end:
	return ret;
	
}
int dumpVlanFilterRule(gponVlanFilterIoctl_ptr rule_ptr){
	int i = 0;
	
	if(rule_ptr == NULL){
		printf("\r\n dumpVlanFilterRule-->rule_ptr == NULL \n");
		return 0;
	}
	printf("\r\n dumpVlanFilterRule-->\n");
	printf("port = 0x%02x\n", rule_ptr->port);
	printf("portType = 0x%02x\n", rule_ptr->portType);
	printf("type = 0x%02x\n", rule_ptr->type);
	printf("untaggedAction = 0x%02x\n", rule_ptr->untaggedAction);
	printf("taggedAction = 0x%02x\n", rule_ptr->taggedAction);
	printf("maxValidVlanListNum = 0x%02x\n", rule_ptr->maxValidVlanListNum);
	printf("cleanFlag = 0x%02x\n", rule_ptr->cleanFlag);
	for(i=0; i<MAX_GPON_VLAN_FILTER_LIST; i++){
		printf("vlanList[%d] = 0x%02x\n", i, rule_ptr->vlanList[i]);
	}
	fflush(stdout);
	sleep(1);
	return 0;
}
static int doAddVlanFilterRule(int argc, char *argv[], void *p){
	gponVlanFilterIoctl_t rule;
	gponVlanFilterIoctl_ptr rule_ptr;
	int tmp = 0;
	int i = 0;
	int ret = 0;

	if((argc<8) || (argc > 19)){ //no include reserved column, min:1 vlanlist, max:12 vlanlist
		printf("\r\nparamater number error");
		printf("\r\n porttype(0:LAN, 1:ANI)  port(Lan:1~4 ANI:0~31)  type (1:set untagged rule, 2:set tagged rule, 3:set untagged and tagged rule) ");
		printf("\r\n untagged action(0:bridge, 1:discard) tagged action maxValidVlanListNum vlanlist");
		return 0;
	}

	rule_ptr = &rule;
	memset(rule_ptr, 0, sizeof(gponVlanFilterIoctl_t));

	tmp = atoi(argv[1]); //port type: LAN 0, ANI 1
	if((tmp != GPON_VLAN_FILTR_PORT_TYPE_LAN ) && (tmp != GPON_VLAN_FILTR_PORT_TYPE_ANI)){
		printf("\r\n invalid port type:0x%02x",tmp);
		return 0;
	}
	rule_ptr->portType = tmp;

	tmp = atoi(argv[2]); //port
	rule_ptr->port = tmp;
	
	if(rule_ptr->portType == GPON_VLAN_FILTR_PORT_TYPE_LAN){
		if(rule_ptr->port < OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC || rule_ptr->port > 4){
			printf("\r\n invalid port :0x%02x",rule_ptr->port);
			return 0;
		}
		rule_ptr->port -= OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC;
	}else if(rule_ptr->portType == GPON_VLAN_FILTR_PORT_TYPE_ANI){
		if(rule_ptr->port < 0 || rule_ptr->port > 31){
			printf("\r\n invalid port :0x%02x",rule_ptr->port);
			return 0;
		}
	}

	tmp = atoi(argv[3]); //type, 1:set untagged rule, 2:set tagged rule, 3:set untagged and tagged rule
	if(tmp > 3){
		printf("\r\n invalid type:0x%02x",tmp);
		return 0;
	}
	rule_ptr->type= tmp;

	tmp = atoi(argv[4]); //untagged action, 0:bridge, 1:discard
	if((tmp != GPON_VLAN_FILTER_ACTION_BRIDGE ) && (tmp != GPON_VLAN_FILTER_ACTION_DISCARD)){
		printf("\r\n invalid untagged action:0x%02x",tmp);
		return 0;
	}
	rule_ptr->untaggedAction= tmp;

	/*tagged action, 
		0:bridge, 
		1:discard, 		
		21: when egress frame match vid in vlan list, then filter, others is forward.(Action g)
		22: when egress frame match pbit in vlan list, then filter, others is forward.(Action g)
		23: when egress frame match TCI in vlan list, then filter, others is forward.(Action g)		
		31: when ingress frame match vid in vlan list, then bridge, others is filter.(Action h)
		     when egress frame match vid in vlan list, then bridge, others is filter.
		32:when ingress frame match pbit in vlan list, then bridge, others is filter.(Action h)
		     when egress frame match pbit in vlan list, then bridge, others is filter.
		33:when ingress frame match TCI in vlan list, then bridge, others is filter.(Action h)
		     when egress frame match TCI in vlan list, then bridge, others is filter.
		41:when egress frame match vid in vlan list, then forward, others is filter.(Action j)
		42:when egress frame match pbit in vlan list, then forward, others is filter.(Action j)
		43:when egress frame match TCI in vlan list, then forward, others is filter.(Action j)
	*/
	tmp = atoi(argv[5]);
	switch(tmp){
		case GPON_VLAN_FILTER_ACTION_BRIDGE:
		case GPON_VLAN_FILTER_ACTION_DISCARD:
		case GPON_VLAN_FILTER_ACTION_G_VID:
		case GPON_VLAN_FILTER_ACTION_G_PBIT:
		case GPON_VLAN_FILTER_ACTION_G_TCI:
		case GPON_VLAN_FILTER_ACTION_H_VID:
		case GPON_VLAN_FILTER_ACTION_H_PBIT:
		case GPON_VLAN_FILTER_ACTION_H_TCI:
		case GPON_VLAN_FILTER_ACTION_J_VID:
		case GPON_VLAN_FILTER_ACTION_J_PBIT:
		case GPON_VLAN_FILTER_ACTION_J_TCI:
			rule_ptr->taggedAction = tmp;
			break;
		default:
			printf("\r\n invalid tagged action:0x%02x",tmp);
			return 0;
	}

	tmp = atoi(argv[6]); //maxValidVlanListNum
	if(tmp > MAX_GPON_VLAN_FILTER_LIST || tmp < 1){
		printf("\r\n invalid maxValidVlanListNum:0x%02x",tmp);
		return 0;
	}
	rule_ptr->maxValidVlanListNum = tmp;

	/*12 vlan list*/
	for(i =7; i<argc; i++){
		tmp = atoi(argv[i]); //reserved
		rule_ptr->vlanList[i-7] = tmp;
	}

//	printf("\r\n ------------IOCTL----------------------\n");
//	printf("doAddVlanFilterRule\n");
//	dumpVlanFilterRule(rule_ptr);

	ret = ponvlan_ioctl(PONVLAN_IOC_ADD_VLAN_FILTER_ENTRY, (void *)rule_ptr, sizeof(gponVlanFilterIoctl_t));
//	printf("doAddVlanFilterRule-->ioctl ret =0x%02x\n",ret);
	updateOmci63365HwNatRule();
	return 0;
}
static int doGetVlanFilterRule(int argc, char *argv[], void *p){
	gponVlanFilterIoctl_t rule;
	gponVlanFilterIoctl_ptr rule_ptr;
	int tmp = 0;
	int i = 0;
	int ret = 0;

	if(argc != 3){ //port, port type
		printf("\r\nparamater number error");
		printf("\r\nporttype(0:LAN, 1:ANI) port(Lan:1~4 ANI:0~31) ");
		return 0;
	}

	rule_ptr = &rule;
	memset(rule_ptr, 0, sizeof(gponVlanFilterIoctl_t));
	
	tmp = atoi(argv[1]); //port type: LAN 0, ANI 1
	if((tmp != GPON_VLAN_FILTR_PORT_TYPE_LAN ) && (tmp != GPON_VLAN_FILTR_PORT_TYPE_ANI)){
		printf("\r\n invalid port type:0x%02x",tmp);
		return 0;
	}
	rule_ptr->portType = tmp;
	
	tmp = atoi(argv[2]); //port
	rule_ptr->port = tmp;
	
	if(rule_ptr->portType == GPON_VLAN_FILTR_PORT_TYPE_LAN){
		if(rule_ptr->port < OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC){
			printf("\r\n invalid port :0x%02x",rule_ptr->port);
			return 0;
		}
		rule_ptr->port -= OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC;
	}else if(rule_ptr->portType == GPON_VLAN_FILTR_PORT_TYPE_ANI){
		if(rule_ptr->port < 0 || rule_ptr->port > 31){
			printf("\r\n invalid port :0x%02x",rule_ptr->port);
			return 0;
		}
	}

//	printf("\r\n ------------IOCTL----------------------\n");
//	printf("doGetVlanFilterRule-->ioctl before\n");
//	dumpVlanFilterRule(rule_ptr);
	ret = ponvlan_ioctl(PONVLAN_IOC_GET_VLAN_FILTER_ENTRY, (void *)rule_ptr, sizeof(gponVlanFilterIoctl_t));
	printf("doGetVlanFilterRule-->ioctl after\n");
	dumpVlanFilterRule(rule_ptr);
	printf("doGetVlanFilterRule-->ioctl ret =0x%02x\n",ret);
	return 0;

}
static int doDelVlanFilterRule(int argc, char *argv[], void *p){
	gponVlanFilterIoctl_t rule;
	gponVlanFilterIoctl_ptr rule_ptr;
	int tmp = 0;
	int i = 0;
	int ret = 0;

	if(argc != 3){ //port, port type
		printf("\r\nparamater number error");
		printf("\r\n port(1~4) porttype(0:LAN, 1:ANI)");
		return 0;
	}

	rule_ptr = &rule;
	memset(rule_ptr, 0, sizeof(gponVlanFilterIoctl_t));
	
	tmp = atoi(argv[1]); //port type: LAN 0, ANI 1
	if((tmp != GPON_VLAN_FILTR_PORT_TYPE_LAN ) && (tmp != GPON_VLAN_FILTR_PORT_TYPE_ANI)){
		printf("\r\n invalid port type:0x%02x",tmp);
		return 0;
	}
	rule_ptr->portType = tmp;

	tmp = atoi(argv[2]); //port
	rule_ptr->port = tmp;
	
	if(rule_ptr->portType == GPON_VLAN_FILTR_PORT_TYPE_LAN){
		if(rule_ptr->port < OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC){
			printf("\r\n invalid port :0x%02x",rule_ptr->port);
			return 0;
		}
		rule_ptr->port -= OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC;
	}else if(rule_ptr->portType == GPON_VLAN_FILTR_PORT_TYPE_ANI){
		if(rule_ptr->port < 0 || rule_ptr->port > 31){
			printf("\r\n invalid port :0x%02x",rule_ptr->port);
			return 0;
		}
	}
	
//	printf("\r\n ------------IOCTL----------------------\n");
//	printf("doDelVlanFilterRule-->ioctl before\n");
//	dumpVlanFilterRule(rule_ptr);
	ret = ponvlan_ioctl(PONVLAN_IOC_DEL_VLAN_FILTER_ENTRY, (void *)rule_ptr, sizeof(gponVlanFilterIoctl_t));
//	printf("doDispVlanFilterRule-->ioctl ret =0x%02x\n",ret);
	updateOmci63365HwNatRule();
	return 0;
}
static int doDispVlanFilterRule(int argc, char *argv[], void *p){
	gponVlanFilterIoctl_t rule;
	gponVlanFilterIoctl_ptr rule_ptr;
	int ret = 0;

	rule_ptr = &rule;
	memset(rule_ptr, 0, sizeof(gponVlanFilterIoctl_t));
	
//	printf("\r\n ------------IOCTL----------------------\n");
//	printf("doDispVlanFilterRule-->ioctl before\n");
	ret = ponvlan_ioctl(PONVLAN_IOC_DISP_VLAN_FILTER_ENTRY, (void *)rule_ptr, sizeof(gponVlanFilterIoctl_t));
//	printf("doDispVlanFilterRule-->ioctl ret =0x%02x\n",ret);
	return 0;
}
static int doSetVlanFilterDbgLevel(int argc, char *argv[], void *p){
	__u8 dbgLevel = 0;
	int ret = 0;
	
	if(argc != 2){ 
		printf("\r\nparamater number error");
		printf("\r\n debuglever(0~2)");
		return 0;
	}
	dbgLevel = atoi(argv[1]); 
//	printf("\r\n ------------IOCTL----------------------\n");
//	printf("doSetVlanFilterDbgLevel-->ioctl before\n");
	ret  = ponvlan_ioctl(PONVLAN_IOC_VLAN_FILTER_DBG_LEVEL, (void *)&dbgLevel, 1);
//	printf("doSetVlanFilterDbgLevel-->ioctl ret =0x%02x\n",ret);
	return 0;
}
#endif	
#ifdef TCSUPPORT_EPON_OAM_CTC
static cmds_t eponVlanCmds[] = {
	{"transparent",	doEponVlanTransparent,		0x10,	0,  	NULL},
	{"tag", 			doEponVlanTag,				0x10,	0,	NULL},
	{"translation", 	doEponVlanTranslation,		0x10,	0,	NULL},
	{"n1", 			doEponVlanN1Translation,		0x10,	0,	NULL},
	{"trunk", 			doEponVlanTrunk,			0x10,	0,	NULL},
	{"show",			doEponVlanShow,			0x10,	0,	NULL},
	{NULL,			NULL,			0x10,	0,	NULL},
};

int doEponVlan(int argc, char *argv[], void *p)
{
	return subcmd(eponVlanCmds, argc, argv, p);
}
int doEponVlanTransparent(int argc, char *argv[], void *p)
{
	unsigned int port = 0;
	char buf[5] = {0}, pvbuf[25] = {0};

	if (argc != 2) goto help;

	port = atoi(argv[1]);
	if (port > 4) goto help;
	
	sprintf(buf, "%d", 0); // transparent
	sprintf(pvbuf, "PORT_VLAN_MODE%d", port);
	tcapi_set("EPON_ONU", pvbuf, buf);
	
	eponClearVlanRules(port);
	pon_disable_mac_vlan(port+11);
	return eponSetVlanTransparent(port);
	
help:
	printf("Usage: transparent port<0~3>\n");
	return -1;		
}
int doEponVlanTag(int argc, char *argv[], void *p)
{
	unsigned int port = 0, defTag;
	char buf[5] = {0}, pvbuf[25] = {0};
	
	if (argc != 3) goto help;

	port = atoi(argv[1]);
	if (port > 4) goto help;
	
	sprintf(buf, "%d", 1); // tag
	sprintf(pvbuf, "PORT_VLAN_MODE%d", port);
	tcapi_set("EPON_ONU", pvbuf, buf);
	
	defTag = atoi(argv[2]);
	if (defTag > 4095) goto help;
	
	eponClearVlanRules(port);
	eponBlockAllVlanTag(port);
	pon_disable_mac_vlan(port+11);
	return eponSetDefVlanTag(port, defTag);
	
help:
	printf("Usage: tag port<0~3> defTag<0~4095>\n");
	return -1;
}
int doEponVlanTranslation(int argc, char *argv[], void *p)
{
	unsigned int port, defTag, num, oldTag, newTag, i;
	char buf[5] = {0}, pvbuf[25] = {0};
	
   	if (argc < 4) goto help;
	
	port = atoi(argv[1]);
	if (port > 4) goto help;
	
	sprintf(buf, "%d", 2); // translation
	sprintf(pvbuf, "PORT_VLAN_MODE%d", port);
	tcapi_set("EPON_ONU", pvbuf, buf);
	
	defTag = atoi(argv[2]);
	if (defTag > 4095) goto help;
	
	eponClearVlanRules(port);
	eponSetDefVlanTag(port,  defTag);

	pon_disable_mac_vlan(port +11);
	for (i = 0; i < (argc-3); i++){
		sscanf(argv[3+i],"%d:%d", &oldTag, &newTag);
		if (oldTag > 4095 || newTag > 4095) goto help;
		
		eponAddTranslateVlanTag(port, oldTag, newTag);
	}
	return 0;
	
help:
	printf("Usage: translation port<0~3> defTag<0~4095> oldTag:newTag[...]\n");
	return -1;
}
int doEponVlanTrunk(int argc, char *argv[], void *p)
{
	unsigned int port, defTag, num, tag, i;
	char buf[5] = {0}, pvbuf[25] = {0};
	
	if (argc < 4) goto help;
	
	port = atoi(argv[1]);
	if (port > 4) goto help;
	
	sprintf(buf, "%d", 4); // trunk
	sprintf(pvbuf, "PORT_VLAN_MODE%d", port);
	tcapi_set("EPON_ONU", pvbuf, buf);
	
	defTag = atoi(argv[2]);
	if (defTag > 4095) goto help;
	
	eponClearVlanRules(port);
	eponSetDefVlanTag(port,  defTag);

	pon_disable_mac_vlan(port+11);
	for (i = 0; i < argc-3; i++){
		tag = atoi(argv[3+i]);
		if (tag > 4095) goto help;
		
		eponAddTrunkVlanTag(port, tag);
	}
	return 0;
	
help:
	printf("Usage: translation port<0~3> defTag<0~4095> tag[...]\n");
	return -1;	
}
// get oldTags & newTag for string: oldTag1,oldTag2,...:newTag
int getEponVlanN1Tags(char *vlanString, unsigned int *oldTags, unsigned int *newTag){
	unsigned int i= 0;
	char *ps = vlanString, *p = vlanString;

	for (; *p != '\0'; p++){
		if (*p == ',' ){
			*p = '\0';			
			oldTags[i++] = atoi(ps);
			ps = p+1;
		}else if (*p == ':'){
			*p = '\0';			
			oldTags[i++] = atoi(ps);

			ps = p+1;
			*newTag = atoi(ps);
			break;
		}
	}
	return i;
}
int doEponVlanN1Translation(int argc, char *argv[], void *p)
{
	unsigned int port = 0, mode, defTag, oldTags[16], newTag, num, i, j, ret;
	char buf[4] = {0}, pvbuf[25] = {0};

	if (argc < 4) goto help;

	port = atoi(argv[1]);
	if (port > 4) goto help;
	
	sprintf(buf, "%d", 3); // N:1
	sprintf(pvbuf, "PORT_VLAN_MODE%d", port);
	tcapi_set("EPON_ONU", pvbuf, buf);
	
	defTag = atoi(argv[2]);
	if (defTag > 4095) goto help;
	
	eponClearVlanRules(port);
	eponSetDefVlanTag(port,  defTag);

	pon_enable_mac_vlan(port +11);
	for (i = 3; i < argc; i++){
		ret = getEponVlanN1Tags(argv[i], oldTags, &newTag);
		if (ret > 0)
			eponAddN1TranslateVlanTag( port, ret, oldTags, newTag);
	}
	
	return 0;
help:
	printf("Usage: N1 port<0~3> defTag<0~4095> oldTag11,oldTag12,...:newTag1 oldTag21,oldTag22,...:newTag2 ...\n");
	return -1;
}
static int doEponVlanShow(int argc, char *argv[], void *p)
{
	unsigned int port = 0, mode, defTag, newTags[16], oldTags[128], transNum, i, j, oldNums[8], newNum=8;
	char buf[4] = {0}, pvbuf[25] = {0};

	if (argc != 2) goto help;

	port = atoi(argv[1]);
	if (port > 4) goto help;
	
	sprintf(pvbuf, "PORT_VLAN_MODE%d", port);
	if (0 != tcapi_get("EPON_ONU", pvbuf, buf)){
		printf("Error: can't get port%d VLAN mode!\n", port);
		return -1;
	}
       
       mode = atoi(buf);

	switch (mode){
		case 0: //transparent
			printf("Port%d: VLAN mode Transparent!\n", port);
		break;
		case 1: //tag
		 	if (0 == eponGetDefVlanTag(port, &defTag))
				printf("Port%d: VLAN mode Tag!\n\tDefaultTag: %d\n", port, defTag&0xfff);
			else
				printf("Port%d: VLAN mode Tag!\n\tDefaultTag: Unkown!\n", port);				
		break;
		case 2: //translation
		 	if (0 == eponGetDefVlanTag(port, &defTag))
				printf("Port%d: VLAN mode Translation!\n\tDefaultTag: %d\n", port,  defTag&0xfff);
			else
				printf("Port%d: VLAN mode Translation!\n\tDefaultTag: Unkown!\n", port);	
			
			if ((transNum = eponGetTranslateVlanTag(port, 16, oldTags, newTags)) == -1){
				printf("\tTranslation vlan list is null!\n");
			}else{	
				printf("\tTranslation vlan list is:\n");
				for (i = 0; i < transNum; i++){
					printf("\t%d: %d --> %d\n", i, oldTags[i]&0xfff, newTags[i]&0xfff);
				}
			}		
		break;
		case 3: //N:!
		 	if (0 == eponGetDefVlanTag(port, &defTag))
				printf("Port%d: VLAN mode N:1 Aggr!\n\tDefaultTag: %d\n", port,  defTag&0xfff);
			else
				printf("Port%d: VLAN mode N:1 Aggr!\n\tDefaultTag: Unkown!\n", port);	

			for (i = 0; i < 8; i ++){
				oldNums[i] = 8;
			}
			if (0 == eponGetN1TranslateVlanTag(port, oldNums, oldTags, &newNum, newTags)){
				int k = 0;
				printf("\tAggr list:\n");
				for (i = 0; i < newNum; i++){
					printf("\t %d:   ", i);
					for (j = 0; j < oldNums[i]; j++){
						printf("%d ", oldTags[k++]&0xfff);
					}
					printf("--> %d\n", newTags[i]&0xfff);
				}
			}else{
				printf("\tAggr list is Null!\n");
			}
			//printf("Port%d: VLAN mode N:1 Aggr we don't support!\n");
		break;
		case 4: //trunk
		 	if (0 == eponGetDefVlanTag(port, &defTag))
				printf("Port%d: VLAN mode Trunk!\n\tDefaultTag: %d\n", port, defTag&0xfff);
			else
				printf("Port%d: VLAN mode Trunk!\n\tDefaultTag: Unkown!\n", port);	
			
			if ((transNum = eponGetTrunkVlanTag(port, 16, oldTags)) == -1){
				printf("\tTrunk vlan list is null!\n");
			}else{	
				printf("\tTrunk vlan list is:\n\t");
				for (i = 0; i < transNum; i++){
					printf("%d, ", oldTags[i]&0xfff);
				}
			}		
		break;
		default:
			printf("Warning: Unkown vlan mode on port%d\n", port);
	}

	return 0;
	
help:	
	printf("Usage: show port<0~3>\n");
	return -1;
}

#endif
int main(int argc, char **argv) 
{

	void *p;
	int ret = -1;
	int pidfd;

	init_compileoption_val();
	pidfd =open(PONVLANCMD_LOCK_FILE, O_RDWR | O_CREAT);
	if(pidfd < 0)
	{
		printf("\r\nopen lock file error!");
		ret = subcmd(PonVlanCmds, argc, argv, p);
	}
	else
	{
		writew_lock(pidfd,0,SEEK_SET,0);
		ret = subcmd(PonVlanCmds, argc, argv, p);
		un_lock(pidfd,0,SEEK_SET,0);
		//close(pidfd);
	}

	return ret;
}

