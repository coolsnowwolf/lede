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
	omciMessageHandle.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/

#ifndef _OMCI_MESSAGEHANDLE_H
#define _OMCI_MESSAGEHANDLE_H

#include "omci.h"
//#include "../../lib/libtcapi.h"
/***************************************
*
*	macro definition
*
***************************************/


/*omci message mechanism----------add by jqzhu*/
//#define PROJID 2
//#define MAX_READ_SIZE 48
//#define MAX_SEND_SIZE 48
#define OMCI_RX_HIGH_Q_KEY 2012
#define OMCI_RX_LOW_Q_KEY 2013
#define OMCI_TX_HIGH_Q_KEY 2014
#define OMCI_TX_LOW_Q_KEY 2015
#define OMCI_CMD_Q_KEY 2016
#define OMCI_NOTIFY_Q_KEY 2017
//#define CWMP_MAX_MSG_LEN 10




/***************************************
*
*	function declare
*
***************************************/
//int process_mq(char *message);

//int read_msg(int mqid,long type,char *text,int flag);
int read_msg(int mqid,long type,omciPayload_t *text,int flag);
//int write_msg(int mqid,long type,char *text,int flag);
int write_msg(int mqid,omciPayload_t *text,int flag);
//int init_msg();
int omciInitQ(void);

int readIpcCmdMsg(int mqid,long type,omci_cmd_msg_t *text,int flag);



typedef struct ipc_queue_msg_s{
	long msgType;// terminal cmd; 
	omciPayload_t payload;
}ipc_queue_msg_t;



#endif
