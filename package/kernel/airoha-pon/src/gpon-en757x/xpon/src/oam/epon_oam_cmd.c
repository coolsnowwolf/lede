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
	epon_oam_cmd.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/7/5	Create
*/


#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <errno.h>
#include "../../../version/tcversion.h"
#include "epon_oam.h"
#include "libepon.h"

#ifdef TCSUPPORT_EPON_OAM_CTC
#include "ctc/oam_ext_mgr.h"
#endif
#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
#include "epon_oam_dbg_srv.h"
#endif

#if !defined __UCLIBC__
#include <pthread.h>
#define MAX_STACK_SIZE 131072
#else
#define MAX_STACK_SIZE 16384
#endif
#include "ctc/oam_ctc_dspch.h"

#ifdef TCSUPPORT_AUTOBENCH
int tcapi_get(char* node, char* attr, char *retval)
{	return (TCAPI_SUCCESS+1);}
int tcapi_set(char *node,char * attr, char *value)
{	return (TCAPI_SUCCESS+1);}
#endif


int eponOamCmdMsgQ = 0;
pthread_t eponOamMsgThread = 0;

int eponOamReadIpcCmdMsg(int mqid,long type,eponOamCmdMsg_t *text,int flag);


int eponOamMsgTask(void){
	int msgLen = 0;
	eponOamCmdMsg_t cmdMsg;
	
	eponOamDbg(EPON_DEBUG_LEVEL_TRACE,"=> eponOamMsgTask loop start!\n");
	while(1){
		memset((u_char*)&cmdMsg, 0, sizeof(eponOamCmdMsg_t));

		msgLen = eponOamReadIpcCmdMsg(eponOamCmdMsgQ , 1 , &cmdMsg, 0);//blocked to wait for cmd
		if (msgLen < 8){
			eponOamDbg(EPON_DEBUG_LEVEL_TRACE,"=> eponOamMsgTask recv small msg from CmdMsgQ!\n");
			continue;
		}
		
		switch(cmdMsg.cmdType){
			case EPONOAM_CMD_TYPE_DEBUGLEVEL:
				eponOamCfg.dbgLvl = (u_char)get32((u_char *)cmdMsg.cmdInfo);
				eponOamCfg.dbgLlidMask = (u_char)get32((u_char*)cmdMsg.cmdInfo+4);
				tcdbg_printf("=>Set eponOamCfg.dbgLvl=%d, eponOamCfg.dbgLlidMask=0x%2X\n", (int)eponOamCfg.dbgLvl, eponOamCfg.dbgLlidMask);	
				break;
			case EPONOAM_CMD_TYPE_INIT:
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG,"=> Init llid=%d\n", cmdMsg.cmdInfo[0]);
				eponOam.llidMask |= (1<<cmdMsg.cmdInfo[0]);
				eponOamLlidInit(cmdMsg.cmdInfo[0]);
				break;
			case EPONOAM_CMD_TYPE_STOP:
				eponOam.llidMask &= ~(1<<cmdMsg.cmdInfo[0]);
				eponOamLlidStop(cmdMsg.cmdInfo[0]);
				break;
			case EPONOAM_CMD_TYPE_DESTORY:
				eponOamExit(0);
				break;
			case EPONOAM_CMD_TYPE_SHOW_STATUS:
				eponOamShowStatus();
				break;
			case EPONOAM_CMD_TYPE_STOPPDUTM:
				eponOamStopPduTimer(cmdMsg.cmdInfo[0]);
				break;
			case EPONOAM_CMD_TYPE_SEND_NORMAL_PDU:
				eponOamSendNormalPdu(cmdMsg.cmdInfo[0], cmdMsg.cmdInfo[1]);
				break;
			case EPONOAM_CMD_TYPE_SEND_CRITICAL_PDU:
				eponOamSendCriticalPdu(cmdMsg.cmdInfo[0], cmdMsg.cmdInfo[1]);
				break;
			case EPONOAM_CMD_TYPE_LOCALSATISFY:
				eponOamSetLocalSatisfy(cmdMsg.cmdInfo[0], cmdMsg.cmdInfo[1]);
				break;
				
			#ifdef TCSUPPORT_EPON_OAM_CTC
			case EPONOAM_CMD_TYPE_CTC_DBG_LVL:
				eponOamCtcSetDbgLevel(cmdMsg.cmdInfo[0]);
				break;
			case EPONOAM_CMD_TYPE_CTC_SET_DSCVY_STATE:
				eponOamSetCtcDiscoveryState(cmdMsg.cmdInfo[0], cmdMsg.cmdInfo[1]);
				break;
			case EPONOAM_CMD_TYPE_OAM_ALARM:
				eponOamProcessAlarm((OamAlarmMsg_Ptr)cmdMsg.cmdInfo);
				break;
			#endif
			
			#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
			case EPONOAM_CMD_TYPE_LAN_DBG:
				eponOamSetLanDbgEnable(cmdMsg.cmdInfo[0]);
				break;
			#endif
			case EPONOAM_CMD_TYPE_TMSENDINFO:
				eponOamCfg.isInfoPduSent = get32((u_char *)cmdMsg.cmdInfo);
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG,"=> Set eponOamCfg.isInfoPduSent=%d\n", eponOamCfg.isInfoPduSent);
				break;
			case EPONOAM_CMD_TYPE_SEND_DYGASP:
				eponOamSendDyGaspPdu(cmdMsg.cmdInfo[0], cmdMsg.cmdInfo[1], cmdMsg.cmdInfo[2]);
				break;
			case EPONOAM_CMD_TYPE_LOSTTMOUT:
				eponOamCfg.lostLinkTime = get32((u_char *)cmdMsg.cmdInfo);
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "=> Set eponOamCfg.lostLinkTime = %u\n", eponOamCfg.lostLinkTime);
				break;
			case EPONOAM_CMD_TYPE_MAXPDUNUMPERSEC:
				eponOamCfg.maxPduPerSec= get32((u_char *)cmdMsg.cmdInfo);
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "=> Set eponOamCfg.maxPduPerSec = %u\n", eponOamCfg.maxPduPerSec);
				break;
			case EPONOAM_CMD_DRIVER_EVENT:
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "=> Recv driver event: %x\n", get32((u_char *)cmdMsg.cmdInfo));
				break;
			case EPONOAM_CMD_TYPE_RESTART:
				eponOamRestart();
				break;
		#ifdef TCSUPPORT_EPON_OAM_CUC	
			case EPONOAM_CMD_TYPE_SAVECFG:
				oam_cuc_cmd_db_save();
				break;
			case EPONOAM_CMD_TYPE_LOADCFG:
				oam_cuc_cmd_db_load();
				break;
		#endif
		#ifdef TCSUPPORT_EPON_OAM_CTC
			case EPONOAM_CMD_TYPE_UPDATACFG:
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "=> Recv TCAPI_SAVE info to update config\n");
				oam_ctc_cmd_update_conf();
				break;
		#endif
			case EPONOAM_CMD_TYPE_ONUTYPE:
				eponOamCfg.isHgu = get32((u_char *)cmdMsg.cmdInfo);
				tcdbg_printf("=>Set eponOamCfg.isHgu=%d\n", eponOamCfg.isHgu);
				break;
		
			default:
				eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "=> Recv unknow eponOam message");
				break;
		}
	}
	return 0;
}

/*______________________________________________________________________________
**	eponOamInitMsgThread
**
**	descriptions:
**		
**	parameters:
**		
**	return:
**		no return 
**____________________________________________________________________________
*/
int eponOamInitMsgThread(void){
	int ret;
	pthread_attr_t attr;
	

	/*create thread*/
	ret = pthread_attr_init(&attr);
	if(ret != 0)
	{
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR, "\r\n eponOamInitMsgThread attribute creation fail!");
		return -1;
	}


	ret = pthread_attr_setstacksize(&attr, MAX_STACK_SIZE);
	if(ret != 0)
	{
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR, "\r\nSet stacksize fail!");
		return -1;
	}
	//need to set deached way
	ret = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(ret != 0)
	{
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR, "\r\nSet attribute fail!");
		return -1;
	}

	ret = pthread_create(&eponOamMsgThread , &attr, (void *)eponOamMsgTask, NULL);
	if(ret != 0)
	{
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR, "\r\nCreate thread fail!");
		return -1;
	}

	pthread_attr_destroy(&attr);
	return 0;
}

int eponOamReadIpcCmdMsg(int mqid,long type,eponOamCmdMsg_t *text,int flag)
{
	int ret;
	eponOamIpcCmdMsg_t ipcMsg;

	ret = msgrcv(mqid,&ipcMsg,sizeof(eponOamCmdMsg_t),1,flag);
	if(ret < 0)
	{
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR,"\r\neponReadIpcCmdMsg read message fail!");
		return -1;
	}

	memcpy((void*)text,(void*)&(ipcMsg.msg),sizeof(eponOamCmdMsg_t));
	return ret;
}

int eponOamCmdInit(void){
	int ret;
	int oflag;
	key_t mqkey;

	oflag = 0666 | IPC_CREAT;
	system("rm -rf /tmp/epon_oam");
	system("mkdir /tmp/epon_oam");

	//create msg queue between APPs
	ret = open(EPONOAM_MQ_FLAG_PATH,O_CREAT | O_RDONLY);
	if(ret < 0)
	{
		perror("\r\nopen file error!");
		return -1;
	}
	else
	{
		eponOamDbg(EPON_DEBUG_LEVEL_TRACE,"\r\nOpen file ok!");
	}
	
	//init queue
	mqkey = ftok(EPONOAM_MQ_FLAG_PATH, EPONOAM_PROJID);
	eponOamCmdMsgQ = msgget(mqkey,oflag);   
	
	if(eponOamCmdMsgQ < 0)
	{
		perror("\r\n eponOamCmdMsgQ open message queue fail!");
		return -1;
	}
	eponOamDbg(EPON_DEBUG_LEVEL_DEBUG,"\r\n eponOamCmdMsgQ =%d\n", eponOamCmdMsgQ);

	//start a thread to handle message
	eponOamInitMsgThread();
	
	return 0;
}


