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
	epon_oam_var.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/7/5	Create
*/


#include <unistd.h>
#include <sys/stat.h>

#include "epon_oam.h"



u_char getOamId(IN eponOamLlid_t *pOam , OUT u_char *width , OUT oamPacket_t *pTxOamPkt);



eponOamVar_t eponOamVarTable[]={
{EPON_OAM_VAR_BRANCH_ATTR, EPON_VAR_LEAF_OAMID , 4 , getOamId},
};



int getOamVarTableSize(){
	return sizeof(eponOamVarTable)/sizeof(eponOamVar_t);
}

u_char getOamId(IN eponOamLlid_t *pOam , OUT u_char *width , OUT oamPacket_t *pTxOamPkt){
	*width = 4;	
	put32(pTxOamPkt->currentPtr+1, pOam->localOamId);
	pTxOamPkt->currentPtr += 5;
	pTxOamPkt->pduLen += 5;
	return 0;
}


