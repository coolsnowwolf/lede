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
	epon_map_cmd.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "libeponmap.h"
#include "global.h"


//#define PON_QOS_DBG

#define EPONMAPCMD_LOCK_FILE "/tmp/ponqoslockfd"

static int doEponmapReset(int argc, char *argv[], void *p);
static int doEponmapEnable(int argc, char *argv[], void *p);
static int doEponmapDisable(int argc, char *argv[], void *p);
static int doEponmapSetDbgLevel(int argc, char *argv[], void *p);
static int doEponmapAddMapRule(int argc, char *argv[], void *p);
static int doEponmapDelMapRule(int argc, char *argv[], void *p);
static int doEponmapClrMapRules(int argc, char *argv[], void *p);
static int doEponmapShowMapRules(int argc, char *argv[], void *p);
static int doEponmapSetLlidQueueMap(int argc, char *argv[], void *p);
static int doEponmapClearLlidQueueMap(int argc, char *argv[], void *p);
static int doEponmapShowLlidQueueMap(int argc, char *argv[], void *p);

//#define ALARM_TEST_DBG
#ifdef ALARM_TEST_DBG
static int doTestEponOamAlarm(int argc, char *argv[], void *p);
#endif

static cmds_t eponmapCmds[] = {
	/* ---------Qos Classification&Marking cmds--------- */
	{"reset",    	doEponmapReset,       		0x10, 0, NULL},
	{"enable",     	doEponmapEnable,        		0x10, 0, NULL},
	{"disable",    	doEponmapDisable,       		0x10, 0, NULL},
	{"dbglvl",    	doEponmapSetDbgLevel, 		0x10, 0, NULL},
	{"addrule",    	doEponmapAddMapRule, 		0x10, 0, NULL},
	{"delrule",    	doEponmapDelMapRule, 		0x10, 0, NULL},
	{"clearrules", 	doEponmapClrMapRules,		0x10, 0, NULL},
	{"showrules",  doEponmapShowMapRules,  	0x10, 0, NULL},

	/* ---------------LLIDQueueConfig cmds--------------- */
	{"setqueuemap",  	doEponmapSetLlidQueueMap,  	0x10, 0, NULL},
	{"clearqueuemap",	doEponmapClearLlidQueueMap,	0x10, 0, NULL},
	{"showqueuemap", doEponmapShowLlidQueueMap,	0x10, 0, NULL},
#ifdef ALARM_TEST_DBG
	{"alarm", doTestEponOamAlarm, 0x10, 0, NULL},
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

int doEponmapReset(int argc, char *argv[], void *p)
{
	eponmapReset();
	return 0;
}

int doEponmapEnable(int argc, char *argv[], void *p)
{
	int port=0;
	if (argc < 2){
		printf("Usage: eponmapcmd enable portId[0~3]\n");
		return 0;
	}
	port = atoi(argv[1]);
	eponmapSetClsfyEnable(port, EPONMAP_ENABLE);
	return 0;
}

int doEponmapDisable(int argc, char *argv[], void *p)
{
	int port=0;
	if (argc < 2){
		printf("Usage: eponmapcmd disable portId[0~3]\n");
		return 0;
	}
	port = atoi(argv[1]);
	eponmapSetClsfyEnable(port, EPONMAP_DISABLE);
	return 0;
}

int doEponmapSetDbgLevel(int argc, char *argv[], void *p)
{
	u_char debuglevel = 0;

	if (argc != 2){
		printf("Usage: eponmapcmd dbglvl [0-4]\n");
		return 0;
	}
	
	debuglevel = atoi(argv[1]);
	eponmapSetDbgLevel(debuglevel);
	return 0;
}

u_char name2Op(char *op)
{
	int len = (strlen(op)>2) ? strlen(op):2;
	
	if (0 == strncmp(op, "never", len) || 0 == strncmp(op, "False", len)){
		return OP_NEVER_MATCH;
	}else if (0 == strncmp(op, "equal", len) || 0 == strcmp(op, "==")){
		return OP_EQUAL;
	}else if (0 == strncmp(op, "notequal", len) || 0 == strncmp(op, "!equal", len) || 0 == strcmp(op, "!=")){
		return OP_NOT_EQUAL;
	}else if (0 == strncmp(op, "less", len) || 0 == strcmp(op, "<=")){
		return OP_LESS_THAN;
	}else if (0 == strncmp(op, "greater", len) || 0 == strcmp(op, ">=")){
		return OP_GREATER_THAN;
	}else if (0 == strncmp(op, "exists", len)){
		return OP_EXISTS;
	}else if (0 == strncmp(op, "notexist", len) || 0 == strncmp(op, "!exist", len)){
		return OP_NOT_EXIST;
	}else if (0 == strncmp(op, "always", len) || 0 == strncmp(op, "True", len)){
		return OP_ALWAYS_MATCH;
	}
	printf("Error operator, must be:\n"
		  "    never(F), equal(==), notequal(!=), less(<=), greater(>=),\n"
		  "    exist, notexist(!exist), always(T)\n");
	return 0xFF;
}

int name2Match(QosMatchRule_Ptr pRule, char *feild, char *value, char *op)
{
	u_char tmp, v8;
	unsigned int mac[6];
	u_short v16;
	u_int v32;

	#ifdef PON_QOS_DBG
	printf("name2Match: feild=%s; value=%s; op=%s\n", feild, value, op);
	#endif

	memset(pRule, 0, sizeof(QosMatchRule_t));

	if (strcmp(feild, "dmac") == 0){		
		pRule->field = FIELD_DMAC;
		if (6 != sscanf(value, "%02x:%02x:%02x:%02x:%02x:%02x", &(mac[0]), &(mac[1]),\
		 &(mac[2]), &(mac[3]), &(mac[4]), &(mac[5]))){
			printf("Error dmac, must be like 03:e2:f4:05:73:90.\n");
			return -1;
		}
		pRule->mac[0] = (u_char)mac[0];
		pRule->mac[1] = (u_char)mac[1];
		pRule->mac[2] = (u_char)mac[2];
		pRule->mac[3] = (u_char)mac[3];
		pRule->mac[4] = (u_char)mac[4];
		pRule->mac[5] = (u_char)mac[5];
		//memcpy(pRule->mac, mac, ETH_ALEN);
	}else if (strcmp(feild, "smac") == 0){
		pRule->field = FIELD_SMAC;
		if (6 != sscanf(value, "%02x:%02x:%02x:%02x:%02x:%02x", &(mac[0]), &(mac[1]),\
		 &(mac[2]), &(mac[3]), &(mac[4]), &(mac[5]))){
			printf("Error smac, must be like 03:e2:f4:05:73:90.\n");
			return -1;
		}
		pRule->mac[0] = (u_char)mac[0];
		pRule->mac[1] = (u_char)mac[1];
		pRule->mac[2] = (u_char)mac[2];
		pRule->mac[3] = (u_char)mac[3];
		pRule->mac[4] = (u_char)mac[4];
		pRule->mac[5] = (u_char)mac[5];
		//memcpy(pRule->mac, mac, ETH_ALEN);
	}else if (strcmp(feild, "pbit") == 0){
		pRule->field = FIELD_PBIT;
		v8 = atoi(value);
		pRule->v8 = v8;
	}else if (strcmp(feild, "vid") == 0){
		pRule->field = FIELD_VLANID;
		v16 = atoi(value);
		if (v16 > 4095){
			printf("Error vid, must be 0~4095!\n");
			return -1;
		}
		pRule->v16 = v16;
		printf("name2Match: vid=%d\n",pRule->v16);
	}else if (strcmp(feild, "ethtype") == 0){
		pRule->field = FIELD_ETHTYPE;
		if (1 != sscanf(value, "%x", &v16)){
			printf("Error ethtype, must be 0x0001 ~ 0xFFFF\n");
			return -1;
		}
		pRule->v16 = v16;
	}else if (strcmp(feild, "dip") == 0){
		pRule->field = FIELD_DIP4;

		if (0 >= inet_pton(AF_INET, value, (void*)&pRule->ip4)){
			printf("Error dip, must be like: 192.168.2.25.\n");
			return -1;
		}
	}else if (strcmp(feild, "sip") == 0){
		pRule->field = FIELD_SIP4;

		if (0 >= inet_pton(AF_INET, value, (void*)&pRule->ip4)){
			printf("Error sip, must be like: 192.168.2.25.\n");
			return -1;
		}
	}else if (strcmp(feild, "ipp4") == 0){
		pRule->field = FIELD_IPPROTO4;
		v8 = atoi(value); // need to use tcp udp icmp igmp to instead value.
		pRule->v8 = v8;
	}else if (strcmp(feild, "dscp4") == 0){
		pRule->field = FIELD_IPDSCP4;
		v8 = atoi(value);
		if (v8 >= 64) {
			printf("Error DSCP4, must be 0~63!\n");
			return -1;
		}
		pRule->v8 = v8;		
	}else if (strcmp(feild, "dscp6") == 0){
		pRule->field = FIELD_IPDSCP6;
		v8 = atoi(value);
		pRule->v8 = v8;		
	}else if (strcmp(feild, "sport") == 0){
		pRule->field = FIELD_SPORT;
		v16 = atoi(value);
		pRule->v16 = v16;
	}else if (strcmp(feild, "dport") == 0){
		pRule->field = FIELD_DPORT;
		v16 = atoi(value);
		pRule->v16 = v16;
	}else if (strcmp(feild, "ipver") == 0){
		pRule->field = FIELD_IPVER;
		v8 = atoi(value);
		if (4 != v8 && 6 != v8){
			printf("Error ipver, must be 4 or 6!\n");
			return -1;
		}
		pRule->v8 = v8;
	}else if (strcmp(feild, "dip6") == 0){
		pRule->field = FIELD_DIP6;

		if (0 >= inet_pton(AF_INET6, value, (void*)&pRule->ip6)){
			printf("Error dip6, must be ipv6 addr.\n");
			return -1;
		}
	}else if (strcmp(feild, "sip6") == 0){
		pRule->field = FIELD_SIP6;

		if (0 >= inet_pton(AF_INET6, value, (void*)&pRule->ip6)){
			printf("Error sip6, must be ipv6 addr.\n");
			return -1;
		}
	}else if (strcmp(feild, "dip6prex") == 0){
		pRule->field = FIELD_SIP6PREX;

		if (0 >= inet_pton(AF_INET6, value, (void*)&pRule->ip6)){
			printf("Error dip6prex, must be ipv6 addr, like: FEC0::40 => FEC0::/64.\n");
			return -1;
		}
	}else if (strcmp(feild, "sip6prex") == 0){
		pRule->field = FIELD_SIP6PREX;

		if (0 >= inet_pton(AF_INET6, value, (void*)&pRule->ip6)){
			printf("Error sip6prex, must be ipv6 addr, like: FEC0::40 => FEC0::/64.\n");
			return -1;
		}
	}else if (strcmp(feild, "flowlbl") == 0){
		pRule->field = FIELD_FLOWLABEL6;
		v32 = atoi(value);
		pRule->ip4 = v32;
	}else if (strcmp(feild, "ipp6") == 0){
		pRule->field = FIELD_IPPROTO6;
		v8 = atoi(value);
		pRule->v8 = v8;
	}else{
		printf("Error field, must be:\n"
			  "    dmac, smac, pbit, vid, ethtype, dip, sip, ipp4, dscp4, dscp6,\n"
			  "    sport, dport, ipver, dip6, sip6, dip6prex, sip6prex, ipp6, flowlbl.\n");
		return -1;		
	}
	
	tmp = name2Op(op);
	if (tmp >= OP_NOT_SET){
		return -1;
	}
	pRule->op = tmp;

	return 0;
}

int doEponmapAddMapRule(int argc, char *argv[], void *p)
{
	u_char tmp, i = 0, matchNum;
	u_char val[16];
	QosIOCtl_t data;
	// addRule [pbit] [queue] [[priority] [field] [value] [operator] (1-12)]
	if ((argc < 7) || ((argc-5)%3 != 0)){
		printf("Usage: addrule [port] [precedence] [pbit] [queue] [[field] [value] [op] ...]\n"
			"    port:  0~3;  precedence:  1~255;  pbit:  0~7;  queue:  0~63; \n"
			"    field:  [dmac|smac|pbit|vid|ethtype|dip|sip|dip6|sip6|dip6prex|sip6prex|\n"
			"              ipp4|ipp6|dscp4|dscp6|flowlbl|ipver|sport|dport]\n"
			"    op:    [never(F)|equal(==)|notequa(!=)|less(<=)|greater(>=)|\n"
			"              exist|notexist(!exist)|always(T)]\n");
		return 0;
	}

	#ifdef PON_QOS_DBG
	for (i = 0; i < argc; i++){
		printf("Param: %d = %s\n", i, argv[i]);
	}
	#endif

	tmp = atoi(argv[1]);
	if (tmp >= PORT_NUM){
		printf("Error PortId: %d, must be 0~%d!\n", tmp, PORT_NUM-1);
		return 0;
	}
	data.portId = tmp;

	tmp = atoi(argv[2]);
	data.result.precedence = tmp;

	tmp = atoi(argv[3]);
	if (tmp > 7 && tmp != 255){
		printf("Error pbit: %d, must be 0~7 or 255!\n", tmp);
		return 0;
	}
	data.result.priority = tmp;

	tmp = atoi(argv[4]);
	data.result.queueMapped = tmp;

	matchNum = (argc - 5) / 3;
	data.matchNum = matchNum;

	for (i = 0; i < matchNum; i++){
		if (-1 == name2Match(&data.matchs[i], argv[5+i*3], argv[6+i*3], argv[7+i*3])){
			return 0;
		}
	}

	#ifdef PON_QOS_DBG
	for (i = 0; i < matchNum; i++){
		printf("matchs %d: field=%d, op=%d\n", i, data.matchs[i].field, data.matchs[i].op);
	}
	fflush(stdout);
	sleep(1);
	#endif

	eponmapIoctl(EPONMAP_IOC_ADD, &data, sizeof(QosIOCtl_t));

	return 0;
}

int doEponmapDelMapRule(int argc, char *argv[], void *p)
{	
	u_char tmp, i = 0, matchNum;
	u_char val[16];
	QosIOCtl_t data;
	// delRule [priority] [pbit] [queue] [[field] [value] [operator] (1-12)]
	if ((argc < 8) || ((argc-5)%3 != 0)){
		printf("Usage: delrule [port] [precedence] [pbit] [queue] [[field] [value] [op] ...]\n"
			"    port:  0~3;  precedence:  1~255;  pbit:  0~7;  queue:  0~63; \n"
			"    field:  [dmac|smac|pbit|vid|ethtype|dip|sip|dip6|sip6|dip6prex|sip6prex|\n"
			"              ipp4|ipp6|dscp4|dscp6|flowlbl|ipver|sport|dport]\n"
			"    op:    [never(F)|equal(==)|notequa(!=)|less(<=)|greater(>=)|\n"
			"              exist|notexist(!exist)|always(T)]\n");
		return 0;
	}

	tmp = atoi(argv[1]);
	if (tmp >= PORT_NUM){
		printf("Error PortId: %d, must be 0~%d!\n", tmp, PORT_NUM-1);
		return 0;
	}
	data.portId = tmp;

	tmp = atoi(argv[2]);
	data.result.precedence = tmp;

	tmp = atoi(argv[3]);
	if (tmp > 7){
		printf("Error pbit: %d, must be 0~7!\n", tmp);
		return 0;
	}
	data.result.priority = tmp;

	tmp = atoi(argv[4]);
	data.result.queueMapped = tmp;

	matchNum = (argc - 5) / 3;
	data.matchNum = matchNum;

	for (i = 0; i < matchNum; i++){
		if (-1 == name2Match(&data.matchs[i], argv[5+i*3], argv[6+i*3], argv[7+i*3])){
			return 0;
		}
	}

	#ifdef PON_QOS_DBG
	for (i = 0; i < matchNum; i++){
		printf("matchs %d: field=%d, op=%d\n", i, data.matchs[i].field, data.matchs[i].op);
	}
	fflush(stdout);
	sleep(1);
	#endif

	eponmapIoctl(EPONMAP_IOC_DELETE, &data, sizeof(data));

	return 0;
}

int doEponmapClrMapRules(int argc, char *argv[], void *p)
{
	u_char portid;

	if (argc != 2){
		printf("Usage: clearrules [portid:0-%d]\n", PORT_NUM-1);
		return 0;
	}
	
	portid = atoi(argv[1]);
	eponmapClearClsfyRules(portid);
	return 0;
}

int doEponmapShowMapRules(int argc, char *argv[], void *p)
{
	u_char portid;

	if (argc != 2){
		printf("Usage: showrules [portid:0-%d]\n", PORT_NUM-1);
		return 0;
	}
	
	portid = atoi(argv[1]);
	eponmapShowClsfyRules(portid);
	return 0;
}

int doEponmapClearLlidQueueMap(int argc, char *argv[], void *p)
{
	u_char llid;
	if (argc < 2){
		printf("Usage: clearqueuemap [llid]; if llid > %d, it will clear all mapping!\n", LLID_NUM_MAX-1);
		return 0;
	}

	llid = atoi(argv[1]);

	eponmapClearLlidQueueMapping(llid);

	return 0;
}

int doEponmapSetLlidQueueMap(int argc, char *argv[], void *p)
{
	u_char tmp, llid, queueNum, i, wsum = 0;
	QueueWeight_t weight[LLID_QUEUE_NUM_MAX];

	// setqueuemap [llid] [[queueid] [weight] ...]
	if (argc < 4){
		printf("Usage: setqueuemap [llid] [[queueid] [weight] ...]\n");
		return 0;
	}

	llid = atoi(argv[1]);
	if (llid >= LLID_NUM_MAX){
		printf("Error llid, must be 0~%d!\n", LLID_NUM_MAX-1);
		return 0;
	}

	queueNum = (argc - 2) / 2;
	if (queueNum < 1 && queueNum > LLID_QUEUE_NUM_MAX){
		printf("Error, has no queueid & weight, or has more than %d!\n", LLID_QUEUE_NUM_MAX);
		return 0;
	}

	for (i = 0; i < queueNum; ++i){
		tmp = atoi(argv[2+i*2]);
		if (tmp >= PRIORITY_QUEUE_NUM_MAX){
			printf("Error queueid, must be 0~%d!\n", PRIORITY_QUEUE_NUM_MAX);
			return 0;
		}
		weight[i].priQueue = tmp;

		tmp = atoi(argv[3+i*2]);
		wsum += tmp;
		if (wsum > 100){
			printf("Error weights, the sum of weights more than 100!\n");
			return 0;
		}
		weight[i].weight = tmp;
	}

/*	if (wsum != 100){
		printf("Error weights, the sum of weights must be 100!\n");
		return 0;
	}
*/
	eponmapSetLlidQueueMapping(llid, queueNum, weight);
	return 0;
}

int doEponmapShowLlidQueueMap(int argc, char *argv[], void *p)
{
	u_char llid;
	if (argc < 2){
		printf("Usage: showqueuemap [llid]\n");
		return 0;
	}

	llid = atoi(argv[1]);
	if (llid >= LLID_NUM_MAX){
		printf("Error llid, must be 0~%d!\n", LLID_NUM_MAX-1);
		return 0;
	}

	eponmapShowLlidQueueMapping(llid);

	return 0;
}

#ifdef ALARM_TEST_DBG
#include "libtcapi.h"
int doTestEponOamAlarm(int argc, char *argv[], void *p)
{
	int key = 0;
	OamAlarmMsg_t amp;
	unsigned char *dp = NULL;
	
	if (argc < 2){
		return 0;
	}

	key = atoi(argv[1]);
	switch (key){
		case 0:
			amp.trapIndex = 0x0001; //equipment alarm
			amp.devIndex_1 = 1;
			amp.trapOtherInfo[0] = 0; // set alarm

			dp = amp.trapOtherInfo + 1;
			*((u_int*)dp) = htonl(1);// reset failed
			break;
		case 1:
			amp.trapIndex = 0x0101; // RxPowerHigh Alarm
			amp.devIndex_3 = 0;  //PON IF
			amp.trapOtherInfo[0] = 0; // set alarm
			dp = amp.trapOtherInfo +1;
			*((u_int*)dp) = htonl(512);// reset failed
			break;
		case 2:			
			amp.trapIndex = 0x0302; // EthPortLOS Alarm
			amp.devIndex_1 = 3;  //EthPort
			amp.trapOtherInfo[0] = 0; // set alarm
			break;
		case 3:
			amp.trapIndex = 0x0306; // EthPort DownstreamDropEvents Alarm
			amp.devIndex_1 = 2;  //EthPort
			amp.trapOtherInfo[0] = 0; // set alarm
			dp = amp.trapOtherInfo +1;
			*((u_int*)dp) = htonl(0);// reset failed
			dp = amp.trapOtherInfo +1+4;
			*((u_int*)dp) = htonl(65000);// reset failed			
			break;		
				
	}
	sendAlarm2EponOam(&amp);
	printf("%s sendAlarm2EponOam\n", __FUNCTION__);
	return 0;
}
#endif

int main(int argc, char **argv)
{
	void *p;
	int ret = -1, pidfd;

	init_compileoption_val();
	pidfd = open(EPONMAPCMD_LOCK_FILE, O_RDWR|O_CREAT);
	if (pidfd < 0){
		printf("\r\nOpen Qos Classification&Marking lock file error!\n");
		ret = subcmd(eponmapCmds, argc, argv, p);
	}
	else{
		writew_lock(pidfd, 0, SEEK_SET, 0);
		ret = subcmd(eponmapCmds, argc, argv, p);
		un_lock(pidfd, 0, SEEK_SET, 0);
	}
	return ret;
}

