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
	JQ.zhu		2012/7/20	Create
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "libepon.h"
#include "global.h"

#define EPONCMD_LOCK_FILE "/tmp/epon_lockfd"


static int doEponMpcp(int argc, char *argv[], void *p);
static int doEponMpcpDereg(int argc, char *argv[], void *p);
static int doEponMpcpDiscvRgstAck(int argc, char *argv[], void *p);
static int doEponMpcpTimeoutTime(int argc, char *argv[], void *p);
static int doEponGetLlidKey(int argc, char *argv[], void *p);
static int doEponSetLlidKey(int argc, char *argv[], void *p);
static int doEponGetLlidFec(int argc, char *argv[], void *p);
static int doEponSetLlidFec(int argc, char *argv[], void *p);
static int doEponSetLlidMask(int argc, char *argv[], void *p);
static int doEponMacRst(int argc, char *argv[], void *p);
static int doEponHoldoverCfg(int argc, char *argv[], void *p);
static int doEponDumpEthCnt(int argc, char *argv[], void *p);
static int doEponClrEthCnt(int argc, char *argv[], void *p);
static int doEponPhyPowerCtl(int argc, char *argv[], void *p);

#ifdef TCSUPPORT_EPON_OAM
static int doEponOam(int argc, char *argv[], void *p);
static int doEponOamDebugLevel(int argc, char *argv[], void *p);
static int doEponOamInit(int argc, char *argv[], void *p);
static int doEponOamStop(int argc, char *argv[], void *p);
static int doEponOamRestart(int argc, char *argv[], void *p);
static int doEponOamDestory(int argc, char *argv[], void *p);
static int doEponOamShow(int argc, char *argv[], void *p);
static int doEponOamShowStatus(int argc, char *argv[], void *p);
static int doEponOamSndCriticalPdu(int argc, char *argv[], void *p);
static int doEponOamStopPduTm(int argc, char *argv[], void *p);
static int doEponOamSndNormalPdu(int argc, char *argv[], void *p);
static int doEponOamLocalSatisfy(int argc, char *argv[], void *p);
static int doEponOamTmInfo(int argc, char *argv[], void *p);
static int doEponOamSndDygaspPdu(int argc, char *argv[], void *p);
static int doEponOamLostTmout(int argc, char *argv[], void *p);
static int doEponOamOnuType(int argc, char *argv[], void *p);


#ifdef TCSUPPORT_EPON_OAM_CTC
static int doEponOamCtcDbgLevel(int argc, char *argv[], void *p);
static int doEponOamCtcDscvyState(int argc, char *argv[], void *p);
#endif /*TCSUPPORT_EPON_OAM_CTC*/

#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
static int doEponOamLanDbg(int argc, char *argv[], void *p);
#endif /*TCSUPPORT_EPON_OAM_LAN_DBG*/
static int doEponOamTmInfo(int argc, char *argv[], void *p);
static int doEponOamSndDygaspPdu(int argc, char *argv[], void *p);
static int doEponOamLostTmout(int argc, char *argv[], void *p);
static int doEponOamMaxPduNumPerSec(int argc, char *argv[], void *p);
#ifdef TCSUPPORT_EPON_OAM_CUC
static int doEponOamSaveCfg(int argc, char *argv[], void *p);
static int doEponOamLoadCfg(int argc, char *argv[], void *p);
#endif
#endif /*TCSUPPORT_EPON_OAM*/

static cmds_t eponCfgCmds[] = {
	/* ---------epon mac  cmds--------- */
#ifdef TCSUPPORT_EPON_OAM
	{"oam",    		doEponOam,    		0x12, 0, NULL},
#endif /*TCSUPPORT_EPON_OAM*/
	{"setllidmask",		doEponSetLlidMask,   0x10, 0, NULL},
	{"getllidkey",		doEponGetLlidKey,    	0x10, 0, NULL},
	{"setllidkey",    	doEponSetLlidKey,    	0x10, 0, NULL},
	{"getllidfec",	  	doEponGetLlidFec,    	0x10, 0, NULL},
	{"setllidfec",	  	doEponSetLlidFec,    	0x10, 0, NULL},
	{"mpcp",    		doEponMpcp,    		0x12, 0, NULL},
	{"eponmacreset",	doEponMacRst,	  	0x10, 0, NULL},	
	{"hldover",		doEponHoldoverCfg,	0x10, 0, NULL},
	{"dumpethcnt",	doEponDumpEthCnt,	0x10, 0, NULL},
	{"clrethcnt",		doEponClrEthCnt,	0x10, 0, NULL},
	{"phypwr", 		doEponPhyPowerCtl,	0x10, 0, NULL},
	{NULL,               NULL,               0x10, 0, NULL},
	
};

static cmds_t eponMpcpCmds[] = {
	/* ---------epon mac MPCP cmds--------- */
	{"deregister",    doEponMpcpDereg,    0x10, 0, NULL},
	{"discvrgstack",    doEponMpcpDiscvRgstAck,    0x10, 0, NULL},
	{"timeout",    doEponMpcpTimeoutTime,    0x10, 0, NULL},
	{NULL,               NULL,               0x10, 0, NULL},
};

#ifdef TCSUPPORT_EPON_OAM
static cmds_t eponOamCmds[] = {
	/* ---------epon mac OAM cmds--------- */
	{"debuglevel",    doEponOamDebugLevel,    0x10, 0, NULL},
	{"init",    doEponOamInit,    0x10, 0, NULL},
	{"stop",    doEponOamStop,    0x10, 0, NULL},
	{"destory",    doEponOamDestory,    0x10, 0, NULL},
	{"restart",     doEponOamRestart,  0x10, 0, NULL},
	{"stoppdutimer",    doEponOamStopPduTm,    0x10, 0, NULL},
	{"sendnormalpdu",    doEponOamSndNormalPdu,    0x10, 0, NULL},
	{"sendcrtclpdu",    doEponOamSndCriticalPdu,    0x10, 0, NULL},
	{"senddygasp",    doEponOamSndDygaspPdu,    0x10, 0, NULL},
	{"show",    doEponOamShow,    0x12, 0, NULL},
	{"localsatisfy",    doEponOamLocalSatisfy,    0x10, 0, NULL},
	
#ifdef TCSUPPORT_EPON_OAM_CTC
	{"ctcdbg",	doEponOamCtcDbgLevel,	  0x10, 0, NULL},	
	{"ctcdscvy",	doEponOamCtcDscvyState, 0x10, 0, NULL},	
#endif	
#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
	{"landbg", doEponOamLanDbg, 0x10, 0, NULL},
#endif
	{"timerinfo",    doEponOamTmInfo,    0x10, 0, NULL},
	{"losttm",       doEponOamLostTmout,      0x10, 0, NULL},
	{"maxpdunum",doEponOamMaxPduNumPerSec,      0x10, 0, NULL},	
#ifdef TCSUPPORT_EPON_OAM_CUC
	{"savecfg",    doEponOamSaveCfg,  0x10, 0, NULL},
	{"loadcfg",    doEponOamLoadCfg,  0x10, 0, NULL},
#endif
	{"onutype",       doEponOamOnuType,      0x10, 0, NULL},
	{NULL,               NULL,               0x10, 0, NULL},
};

static cmds_t eponOamShowCmds[] = {
	/* ---------epon mac OAM show cmds--------- */
	{"status",    doEponOamShowStatus,    0x10, 0, NULL},
	{NULL,               NULL,               0x10, 0, NULL},
};
#endif /*TCSUPPORT_EPON_OAM*/

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

int main(int argc, char **argv)
{
	void *p = NULL;
	int ret = -1, pidfd;

	init_compileoption_val();
	pidfd = open(EPONCMD_LOCK_FILE, O_RDWR|O_CREAT);
	if (pidfd < 0){
		printf("\r\nOpen EPON cfg lock file error!\n");
		ret = subcmd(eponCfgCmds, argc, argv, p);
	}
	else{
		writew_lock(pidfd, 0, SEEK_SET, 0);
		ret = subcmd(eponCfgCmds, argc, argv, p);
		un_lock(pidfd, 0, SEEK_SET, 0);
	}
	return ret;
}

static int doEponMpcp(int argc, char *argv[], void *p){
	return subcmd(eponMpcpCmds, argc, argv, p);
}

static int doEponMpcpDiscvRgstAck(int argc, char *argv[], void *p){
	eponMacIoctl_t data;
	
	if (argc < 3){
		printf("Usage: discvrgstack <llid Index> <1-ack | 0-nack>\n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data.param0 = atoi(argv[1]);
	data.info[0] = atoi(argv[2]);
	
	fflush(stdout);
	sleep(1);

	return eponMacIoctl(EPON_IOCTL_LLID_DISCV_RGSTACK, (void *)&data, sizeof(data));

}

static int doEponMpcpDereg(int argc, char *argv[], void *p){
	eponMacIoctl_t data;

	
	if (argc < 2){
		printf("Usage: deregister <llid Index> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data.llidIdx= atoi(argv[1]);
	
	fflush(stdout);
	sleep(1);

	return eponMacIoctl(EPON_IOCTL_LLID_DEREGISTER, (void *)&data, sizeof(data));
}

static int doEponMpcpTimeoutTime(int argc, char *argv[], void *p){
	u_short time = 0;
	
	if (argc != 2){
		eponApiGetOnuMpcpTime(&time);
		printf("timeout =  %d ms\n", time);
		printf("Usage: timeout < ms > \n");
		return 0;
	}

	time= atoi(argv[1]);

	fflush(stdout);

	eponApiSetOnuMpcpTime(time);
	return 0;
}

static int doEponMacRst(int argc, char *argv[], void *p)
{
	u_int data = 0;
	printf("epon mac reset\n");

	fflush(stdout);
	sleep(1);

	return eponMacIoctl(EPON_IOCTL_MAC_RST, &data, sizeof(data));

}

static int doEponHoldoverCfg(int argc, char *argv[], void *p){
	u_short time = 0;
	u_char enable = 0;
	
	if (argc != 3){
		eponApiGetOnuHoldoverCfg(&enable , &time);
		printf("enable =  %d , time = %d ms\n", enable, time);
		printf("Usage: hldover < 0-disable |1-enbale >< ms > \n");
		return 0;
	}

	enable = atoi(argv[1]);
	time= atoi(argv[2]);

	fflush(stdout);

	eponApiSetOnuHoldoverCfg(enable  , time);
	return 0;
}

static int doEponDumpEthCnt(int argc, char *argv[], void *p){
	eponTxCnt_t txCnt;
	eponRxCnt_t rxCnt;
	
	if(eponApiGetOnuTxCnt(&txCnt) <0){
		printf("\r\ndoEponDumpEthCnt eponApiGetOnuTxCnt error");
		return -1;
	}
	if(eponApiGetOnuRxCnt(&rxCnt) <0){
		printf("\r\ndoEponDumpEthCnt eponApiGetOnuRxCnt error");
		return -1;
	}

	printf("\r\n txFrameCnt = %d", txCnt.txFrameCnt);
	printf("\r\n txFrameLen = %d", txCnt.txFrameLen);
	printf("\r\n txDropCnt = %d", txCnt.txDropCnt);
	printf("\r\n txBroadcastCnt = %d", txCnt.txBroadcastCnt);
	printf("\r\n txMulticastCnt = %d", txCnt.txMulticastCnt);
	printf("\r\n tx_less64_Cnt = %d", txCnt.tx_less64_Cnt);
	printf("\r\n tx_more1518_Cnt = %d", txCnt.tx_more1518_Cnt);
	printf("\r\n tx_64_Cnt = %d", txCnt.tx_64_Cnt);
	printf("\r\n tx_65_to_127_Cnt = %d", txCnt.tx_65_to_127_Cnt);
	printf("\r\n tx_128_to_255_Cnt = %d", txCnt.tx_128_to_255_Cnt);
	printf("\r\n tx_256_to_511_Cnt = %d", txCnt.tx_256_to_511_Cnt);
	printf("\r\n tx_512_to_1023_Cnt = %d", txCnt.tx_512_to_1023_Cnt);
	printf("\r\n tx_1024_to_1518_Cnt = %d", txCnt.tx_1024_to_1518_Cnt);


	printf("\r\n rxFrameCnt = %d", rxCnt.rxFrameCnt);
	printf("\r\n rxFrameLen = %d", rxCnt.rxFrameLen);
	printf("\r\n rxDropCnt = %d", rxCnt.rxDropCnt);
	printf("\r\n rxBroadcastCnt = %d", rxCnt.rxBroadcastCnt);
	printf("\r\n rxMulticastCnt = %d", rxCnt.rxMulticastCnt);
	printf("\r\n rxCrcCnt = %d", rxCnt.rxCrcCnt);
	printf("\r\n rxFragFameCnt = %d", rxCnt.rxFragFameCnt);
	printf("\r\n rxJabberFameCnt = %d", rxCnt.rxJabberFameCnt);
	printf("\r\n rx_less64_Cnt = %d", rxCnt.rx_less64_Cnt);
	printf("\r\n rx_more1518_Cnt = %d", rxCnt.rx_more1518_Cnt);
	printf("\r\n rx_64_Cnt = %d", rxCnt.rx_64_Cnt);
	printf("\r\n rx_65_to_127_Cnt = %d", rxCnt.rx_65_to_127_Cnt);
	printf("\r\n rx_128_to_255_Cnt = %d", rxCnt.rx_128_to_255_Cnt);
	printf("\r\n rx_256_to_511_Cnt = %d", rxCnt.rx_256_to_511_Cnt);
	printf("\r\n rx_512_to_1023_Cnt = %d", rxCnt.rx_512_to_1023_Cnt);
	printf("\r\n rx_1024_to_1518_Cnt = %d", rxCnt.rx_1024_to_1518_Cnt);
	
	return 0;
}


static int doEponClrEthCnt(int argc, char *argv[], void *p){
	u_char clearTx = 0;
	u_char clearRx = 0;
	
	if (argc != 3){
		printf("\r\n usage: clrethcnt <1- clear TX MIB> <1-clear RX MIB>\r\n");
		return 0;
	}

	clearTx = atoi(argv[1]);
	clearRx = atoi(argv[2]);

	fflush(stdout);

	eponApiClearOnuTxRxCnt(clearTx, clearRx );
	return 0;
}

static int doEponSetLlidMask(int argc, char *argv[], void *p){
	eponMacIoctl_t data;

	if (argc < 2){
		printf("Usage: setllidmask \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data.param2= atoi(argv[1]);

	fflush(stdout);
	return eponMacIoctl(EPON_IOCTL_SET_LLID_ENABLE_MASK, &data, sizeof(data));
}

static int doEponSetLlidKey(int argc, char *argv[], void *p){
	u_char llidIdx = 0;
	u_char keyIndex = 0;
	u_char key[3] = {0};
	int n;
	if (argc != 6){
		printf("Usage: setllidkey <llid index> <key index> <byte0> <byte1> <byte2>\n");
		return -1;
	}

	llidIdx = atoi(argv[1]);
	keyIndex = atoi(argv[2]);
	 sscanf(argv[3] , "%02x", &n);
	 key[0] = n;
	 sscanf(argv[4] , "%02x", &n);
	 key[1] = n;
	 sscanf(argv[5] , "%02x", &n);
	 
	 key[2] = n;
	eponApiSetLlidkey(llidIdx , keyIndex, key);
	return 0;
}

static int doEponGetLlidKey(int argc, char *argv[], void *p){
	u_char llidIdx = 0;
	u_char keyIndex = 0;
	u_char key[3] = {0};
	if (argc != 3){
		printf("Usage: getllidkey <llid index> <key index>\n");
		return -1;
	}

	llidIdx = atoi(argv[1]);
	keyIndex = atoi(argv[2]);

	eponApiGetLlidkey(llidIdx , keyIndex, key);
	printf("\r\nllid-%d key-%d=0x%02x 0x%02x 0x%02x\n", llidIdx, keyIndex, key[0], key[1], key[2]);
	return 0;
}

static int doEponGetLlidFec(int argc, char *argv[], void *p){
	u_char llidIdx = 0;
	int fecFlag = 0;

	if (argc != 2){
		printf("Usage: getllidfec <llid index>\n");
		return -1;
	}

	llidIdx = atoi(argv[1]);

	fecFlag = eponApiGetLlidFec(llidIdx);
	switch (fecFlag){
	case 0:
		printf("\r\nllid-%d FEC Disabled\n", llidIdx);
	break;
	case 1:		
		printf("\r\nllid-%d FEC Enabled\n", llidIdx);
	break;
	default:		
		printf("\r\nllid-%d FEC %d\n", llidIdx, fecFlag);
	}
	return 0;
}

static int doEponSetLlidFec(int argc, char *argv[], void *p){
	u_char llidIdx = 0;
	u_char fecFlag = 0;

	if (argc != 3){
		printf("Usage: setllidfec <llid index> <1 | 0>\n");
		return -1;
	}

	llidIdx = atoi(argv[1]);
	fecFlag = atoi(argv[2]);


	eponApiSetLlidFec(llidIdx , fecFlag);
	return 0;
}

static int doEponPhyPowerCtl(int argc, char *argv[], void *p){
	u_int time;

	if (argc != 2){
		printf("Usage: phypwr time<0~65535>");
		return -1;
	}
	
	time = atoi(argv[1]);
	eponApiSetPhyPower(time);
	return 0;
}

#ifdef TCSUPPORT_EPON_OAM
static int doEponOam(int argc, char *argv[], void *p){
	return subcmd(eponOamCmds, argc, argv, p);
}

static int doEponOamDebugLevel(int argc, char *argv[], void *p){
	u_int data[2] = {0};
	u_int cmdSeq = 0;
		
	if (argc < 3){
		printf("Usage: oam debuglevel <0/1/2/3> llidmask\n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data[0] = atoi(argv[1]);
	data[1] = atoi(argv[2]); 
	printf("-------send message--------debug level %d llidmask %X-\n", data[0], data[1]);	

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_DEBUGLEVEL, (void *)&data, 2*sizeof(u_int), &cmdSeq);
	return  0;
}


static int doEponOamTmInfo(int argc, char *argv[], void *p){
	u_int data = 0;
	u_int cmdSeq = 0;
		
	if (argc < 2){
		printf("Usage: oam timerinfo <0/1> \n");
		return 0;
	}
	memset(&data, 0, sizeof(data));
	data = atoi(argv[1]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_TMSENDINFO, (void *)&data, sizeof(data), &cmdSeq);
	return  0;
}




static int doEponOamInit(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc < 2){
		printf("Usage: oam init <LLID index> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data = atoi(argv[1]);

	if(data >= EPON_LLID_MAX_NUM){
		return -1;
	}

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_INIT, (void *)&data, sizeof(data), &cmdSeq);
	return  0;
}


static int doEponOamStop(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc < 2){
		printf("Usage: oam stop <LLID index> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data = atoi(argv[1]);

	if(data >= EPON_LLID_MAX_NUM){
		return -1;
	}

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_STOP, (void *)&data, sizeof(data), &cmdSeq);
	return  0;
}


static int doEponOamDestory(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc != 1){
		printf("Usage: oam destory \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_DESTORY, (void *)&data, sizeof(data), &cmdSeq);
	return  0;
}
static int doEponOamRestart(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc != 1){
		printf("Usage: oam restart \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_RESTART, (void *)&data, sizeof(data), &cmdSeq);
	return  0;

}



static int doEponOamStopPduTm(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc != 2){
		printf("Usage: oam stoppdutimer <llid index> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_STOPPDUTM, (void *)&data, sizeof(data), &cmdSeq);
	return  0;
}

static int doEponOamSndNormalPdu(int argc, char *argv[], void *p){
	u_char data[EPONOAM_CMD_INFO_LEN] = {0};
	u_int cmdSeq = 0;
		
	if (argc != 3){
		printf("Usage: oam sendnormalpdu <llid index> <count>\n");
		return 0;
	}

	data[0] = atoi(argv[1]);
	data[1] = atoi(argv[2]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_SEND_NORMAL_PDU, data, EPONOAM_CMD_INFO_LEN, &cmdSeq);
	return  0;
}


static int doEponOamSndCriticalPdu(int argc, char *argv[], void *p){
	u_char data[EPONOAM_CMD_INFO_LEN] = {0};
	u_int cmdSeq = 0;
		
	if (argc != 3){
		printf("Usage: oam sendcrtclpdu <llid index> <count>\n");
		return 0;
	}

	data[0] = atoi(argv[1]);
	data[1] = atoi(argv[2]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_SEND_CRITICAL_PDU, data, EPONOAM_CMD_INFO_LEN, &cmdSeq);
	return  0;
}


static int doEponOamSndDygaspPdu(int argc, char *argv[], void *p){
	u_char data[EPONOAM_CMD_INFO_LEN] = {0};
	u_int cmdSeq = 0;
		
	if (argc != 4){
		printf("Usage: oam senddygasp <llid index> <type 0/1/2/3> <count>\n");
		return 0;
	}

	data[0] = atoi(argv[1]);
	data[1] = atoi(argv[2]);
	data[2] = atoi(argv[3]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_SEND_DYGASP, data, EPONOAM_CMD_INFO_LEN, &cmdSeq);
	return  0;
}


static int doEponOamLocalSatisfy(int argc, char *argv[], void *p){
	u_char data[EPONOAM_CMD_INFO_LEN] = {0};
	u_int cmdSeq = 0;
		
	if (argc != 3){
		printf("Usage: oam localsatisfy <llid index> <0- not satisfy | 1- satisfy>\n");
		return 0;
	}

	data[0] = atoi(argv[1]);
	data[1] = atoi(argv[2]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_LOCALSATISFY, data, EPONOAM_CMD_INFO_LEN, &cmdSeq);
	return  0;
}

#ifdef TCSUPPORT_EPON_OAM_CTC
static int doEponOamCtcDscvyState(int argc, char *argv[], void *p)
{
	u_char data[EPONOAM_CMD_INFO_LEN] = {0};
	u_int cmdSeq = 0;
	
	if (argc != 3){
		goto USAGE;
	}

	data[0] = atoi(argv[1]);
	data[1] = atoi(argv[2]);
	
	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_CTC_SET_DSCVY_STATE, (void *)&data, EPONOAM_CMD_INFO_LEN, &cmdSeq);
	
	return 0;
USAGE:
	printf("Usage: oam ctcdscvy <llid index> <0- passive_wait| 1- send_ext_info | 2- send_any>\n");
	return 0;
}

static int doEponOamCtcDbgLevel(int argc, char *argv[], void *p)
{
	u_char data = 0;
	u_int cmdSeq = 0;
	
	if (argc != 2){
		goto USAGE;
	}

	data = atoi(argv[1]);
	
	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_CTC_DBG_LVL, (void *)&data, sizeof(data), &cmdSeq);
	
	return 0;
USAGE:
	printf("Usage: oam ctcdbg <0~5>\n");
	return 0;	
}
#endif


#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
static int doEponOamLanDbg(int argc, char *argv[], void *p)
{
	int l = 0;
	u_char data = 0;
	u_int cmdSeq = 0;
	
	if (argc != 2){
		goto USAGE;
	}
	
	memset(&data, 0, sizeof(data));
	
	l = strlen(argv[1]);
	if (0 == strncmp(argv[1], "enable", l)){
		data = 1;
	}else if (0 == strncmp(argv[1], "disable", l)){
		data = 0;
	}else{
		goto USAGE;
	}
	
	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_LAN_DBG, (void *)&data, sizeof(data), &cmdSeq);
	return 0;
USAGE:
	printf("Usage: oam landbg <enable | disable>\n");
	return 0;	
}
#endif

static int doEponOamLostTmout(int argc, char *argv[], void *p){
	u_int data = 0;
	u_int cmdSeq = 0;
		
	if (argc < 2){
		printf("Usage: oam losttm <599~10000> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data = atoi(argv[1]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_LOSTTMOUT, (void *)&data, sizeof(data), &cmdSeq);
	return  0;	
}

static int doEponOamOnuType(int argc, char *argv[], void *p){
	u_int data = 0;
	u_int cmdSeq = 0;
		
	if (argc < 2){
		printf("Usage: oam onutype <0 | 1> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data = atoi(argv[1]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_ONUTYPE, (void *)&data, sizeof(data), &cmdSeq);
	return  0;	
}

static int doEponOamMaxPduNumPerSec(int argc, char *argv[], void *p){
	u_int data = 0;
	u_int cmdSeq = 0;
		
	if (argc < 2){
		printf("Usage: oam maxpdunum <10~10000> \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));
	data = atoi(argv[1]);

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_MAXPDUNUMPERSEC, (void *)&data, sizeof(data), &cmdSeq);
	return  0;	
}


static int doEponOamShow(int argc, char *argv[], void *p){
	return subcmd(eponOamShowCmds, argc, argv, p);
}


static int doEponOamShowStatus(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc != 1){
		printf("Usage: show status \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_SHOW_STATUS, (void *)&data, sizeof(data), &cmdSeq);
	return  0;
}

#ifdef TCSUPPORT_EPON_OAM_CUC
static int doEponOamSaveCfg(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc != 1){
		printf("Usage: savecfg\n");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_SAVECFG, (void *)&data, sizeof(data), &cmdSeq);
	return  0;	
}

static int doEponOamLoadCfg(int argc, char *argv[], void *p){
	u_char data = 0;
	u_int cmdSeq = 0;
		
	if (argc != 1){
		printf("Usage: loadcfg \n");
		return 0;
	}

	memset(&data, 0, sizeof(data));

	sendEponOamCmdMsg(EPONOAM_CMD_TYPE_LOADCFG, (void *)&data, sizeof(data), &cmdSeq);
	return  0;	
}

#endif

#endif /*TCSUPPORT_EPON_OAM*/


