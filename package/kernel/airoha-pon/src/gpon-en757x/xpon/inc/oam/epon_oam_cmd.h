/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    epon_oam_cmd.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    JQ.Zhu  2012-06-06      Initial version
*/

#ifndef __EPON_OAM_CMD_
#define __EPON_OAM_CMD_
/*
#define EPONOAM_MQ_FLAG_PATH "/tmp/epon_oam/epon_oam_cmd_queue"
#define EPONOAM_PROJID 			10
#define EPONOAM_CMD_INFO_LEN 		128


//cmd type define
#define EPONOAM_CMD_TYPE_DEBUGLEVEL			1
#define EPONOAM_CMD_TYPE_INIT					2
#define EPONOAM_CMD_TYPE_STOP				3
#define EPONOAM_CMD_TYPE_DESTORY			4
#define EPONOAM_CMD_TYPE_SHOW_STATUS		5
#define EPONOAM_CMD_TYPE_STOPPDUTM			6
#define EPONOAM_CMD_TYPE_SEND_NORMAL_PDU	7
#define EPONOAM_CMD_TYPE_SEND_CRITICAL_PDU	8
#define EPONOAM_CMD_TYPE_LOCALSATISFY		9

#ifdef TCSUPPORT_EPON_OAM_CTC
#define EPONOAM_CMD_TYPE_CTC_DBG_LVL 			10
#define EPONOAM_CMD_TYPE_CTC_SET_DSCVY_STATE 	11
#define EPONOAM_CMD_TYPE_OAM_ALARM   			12
#endif
#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
#define EPONOAM_CMD_TYPE_LAN_DBG		15
#endif
#define EPONOAM_CMD_TYPE_TMSENDINFO		16
#define EPONOAM_CMD_TYPE_SEND_DYGASP	17
#define EPONOAM_CMD_TYPE_LOSTTMOUT    	18


typedef struct eponOamCmdMsg_s{
	u_int cmdType;
	u_int cmdSeq;
	u_char cmdInfo[EPONOAM_CMD_INFO_LEN];
}eponOamCmdMsg_t;

typedef struct eponOamIpcCmdMsg_s{
	long msgType;
	eponOamCmdMsg_t msg;
}eponOamIpcCmdMsg_t;

*/
int eponOamCmdInit(void);

#endif//end of __EPON_OAM_CMD_
