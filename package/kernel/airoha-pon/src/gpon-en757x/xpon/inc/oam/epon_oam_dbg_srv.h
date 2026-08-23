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
	oam_dbg_srv.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lightman		2012/8/24	Create
*/
#ifndef OAM_DBG_SRV_H_
#define OAM_DBG_SRV_H_
#include "epon_oam.h"

#define EPON_OAM_DBG_PORT   8809
#define EPON_OAM_DBG_MAX_BUF  1550

#define THREAD_STATE_DEAD        0
#define THREAD_STATE_WORKING  1

#define GET_DATA_FROM_OLT              0
#define GET_DATA_FROM_DBG_QUEUE  1
#define GET_DATA_MAX				2


int eponOamAcptClient();
int eponOamTcpSrvStp(unsigned short port);
void eponOamTcpSrvExit();

void eponOamSetLanDbgEnable(int enable);

int eponOamPktRxDbgQ(oamPacket_t *pOamPkt);
int eponOamPktTxDbgQ(int len, char *pkts);

void *eponOamDbgThread(void *data);
void eponOamDbgThreadClean(void *data);



#endif
