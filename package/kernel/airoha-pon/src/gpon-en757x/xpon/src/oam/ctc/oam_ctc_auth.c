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
	oam_ctc_auth.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include "ctc/oam_ctc_auth.h"
#include "ctc/oam_ext_cmn.h"
#include "ctc/oam_ext_buff.h"
#include "ctc/oam_sys_api.h"
#include "ctc/oam_ctc_dspch.h"
#include "epon_sys_api.h"
#include "libepon.h"


extern u_char gOuiCtc[3];
extern OamCtcDB_t gCtcDB;

#include "libcompileoption.h"

int eponAuthTimeID = 0;
extern int epon_auth_timeout;
int isOamNodeOperated = 0;

int startloidAuth = 0;
int current_llid = 0;

int eponAuthTimeout(u_char *llid)
{
	if(isOamNodeOperated >= 2){
		//tcdbg_printf("\r\neponAuthTimeout(): isOamNodeOperated = %d ,LINK_UP\n", isOamNodeOperated);
		tcapi_set("EPON_LOIDAuth", "AuthStatus", "1");	   //olt is already do Set 2times, but do not set auth success
		epon_sys_set_link_state(*llid, LINK_UP);
	}else if (gCtcDB.llid[0].auth.loid[23] != '\0') {  
		tcapi_set("EPON_LOIDAuth", "AuthStatus", "2");	   // do not care the specific error code now, if needed, use errcode + 1, because we usr 1 as auth success status
		startloidAuth = 0;
	}
	else {
		tcdbg_printf("\r\neponAuthTimeout(): loid is empty");
	}
}

inline void getLoidPassword(IN u_char llidIdx, OUT u_char loid[24], OUT u_char password[12])
{
	memcpy((void*)loid, gCtcDB.llid[llidIdx].auth.loid, 24);
	memcpy((void*)password, gCtcDB.llid[llidIdx].auth.passwd,12);
}

inline void procAuthFailure(IN u_char llid, IN u_char failureType)
{

	gCtcDB.llid[llid].auth.state = FALSE;
	epon_sys_set_link_state(llid, LINK_DOWN);
	
	isOamNodeOperated = 0;

	eponOamExtDbg(DBG_OAM_L1, ">> Auth Failure: type = %d ", failureType);
	switch(failureType){
		case AUTH_FAILURE_LOID_NOEXIST:
			eponOamExtDbg(DBG_OAM_L1, "LOID not exist!\n");
		break;
		case AUTH_FAILURE_PWD_WRONG:
			eponOamExtDbg(DBG_OAM_L1, "Password is wrong!\n");
		break;
		case AUTH_FAILURE_LOID_CONFLICT:
			eponOamExtDbg(DBG_OAM_L1, "LOID conflict!\n");
		break;
		default:
		break;
	}
	
	if (eponAuthTimeID > 0) {
		timerStopS(eponAuthTimeID);
		eponAuthTimeID = 0;
	}
	
	if (gCtcDB.llid[0].auth.loid[23] != 0) {  
	     tcapi_set("EPON_LOIDAuth", "AuthStatus", "2");	   // do not care the specific error code now, if needed, use errcode + 1, because we usr 1 as auth success status
	}
}

inline void procAuthSuccess(IN u_char llid)
{
	gCtcDB.llid[llid].auth.state = TRUE;

	eponOamExtDbg(DBG_OAM_L1, ">> Auth Success!\r\n");
	epon_sys_set_link_state(llid, LINK_UP);

	if (eponAuthTimeID > 0) {
		timerStopS(eponAuthTimeID);
		eponAuthTimeID = 0;
	}
	tcapi_set("EPON_LOIDAuth", "AuthStatus", "1");

}

/* ------------------------------------Begin Ext Auth--------------------------------------*/

/* ----------------------  <-- pData
 * |  1    Auth_Code    |
 * ----------------------
 * |  2    length       |
 * ----------------------
 * |  X    Auth_data    |
 * ----------------------  length = 3+X
 */
int procExtAuth(OUT Buff_Ptr bfp, IN Buff_Ptr obfp)
{
	OamAuth_Ptr pAuth = NULL;

	if (isNotEnough(obfp, sizeof(OamAuth_t))){
		return FAIL;
	}

	startloidAuth = 1;

	pAuth = (OamAuth_Ptr)(obfp->data+obfp->offset);
	obfp->offset += sizeof(OamAuth_t);
	pAuth->length = ntohs(pAuth->length);

	eponOamExtDbg(DBG_OAM_L2, ">> CTC OAM Auth: "
		"Code = 0x%02x, Data length = 0x%04X\r\n",\
		pAuth->authCode, pAuth->length);

	if (pAuth->authCode == AUTH_CODE_REQUEST) {
		if (eponAuthTimeID == 0) {
			current_llid = obfp->llidIdx;
			eponAuthTimeID = timerStartS(epon_auth_timeout, eponAuthTimeout , &current_llid);
		}
		else {
			timerStopS(eponAuthTimeID);
			current_llid = obfp->llidIdx;
			eponAuthTimeID = timerStartS(epon_auth_timeout, eponAuthTimeout , &current_llid);
		}
	}

	if (isNotEnough(obfp, pAuth->length)
		&& (pAuth->length != 0x0101)) // fibehome AN5516 's bug: length==0x0101
		return FAIL;

	switch(pAuth->authCode){
		case AUTH_CODE_REQUEST:
		{
			isOamNodeOperated = 0;
			if (pAuth->length == 0x01 && isEnough(obfp, 1)){
				u_char authType = *(obfp->data + obfp->offset);

				obfp->offset += 1;
				buildAuthResponse(bfp, authType);
			}else if (pAuth->length == 0x0101){ // fibehome AN5516 's bug: length==0x0101
				buildAuthResponse(bfp, 1);
			}
		}
		break;
		case AUTH_CODE_SUCESS:
		{
			procAuthSuccess(obfp->llidIdx);
			eponApiSetAuthFail(0);
			return FAIL;
		}
		break;
		case AUTH_CODE_FAILURE:
		{
			u_char failureType = *(obfp->data + obfp->offset);

			obfp->offset += 1;
			procAuthFailure(obfp->llidIdx, failureType);
			eponApiSetAuthFail(1);
			return FAIL;
		}
		break;
		default:
		break;
	}
	return SUCCESS;
}

int buildAuthResponse(OUT Buff_Ptr bfp, IN u_char authType)
{
	putBuffU8(bfp, AUTH_CODE_RESPONSE);

	switch(authType){
		case AUTH_TYPE_LOID_PWD:
		{
			// step 2.1: get the LOID + PWD and its length
			u_char loid[24]     = {0};
			u_char password[12] = {0};
			getLoidPassword(bfp->llidIdx, loid, password);

			// build the oam auth response packet
			putBuffU16(bfp, 0x25); // the Auth_Data length = 36
			putBuffU8(bfp, AUTH_TYPE_LOID_PWD);
			putBuffData(bfp, loid, 24);
			putBuffData(bfp, password, 12);

			eponOamExtDbg(DBG_OAM_L3, "<< Auth_Code: 0x02; Length: 0x25; Auth_Type: 0x01; "
				"\r\n   LOID:");
			eponOamDumpHexString(DBG_OAM_L3, loid, 24);
			eponOamExtDbg(DBG_OAM_L3, "Password:");
			eponOamDumpHexString(DBG_OAM_L3, password, 12);
		}
		break;
		default:
		{
			putBuffU16(bfp, 2); // the length of Auth Data
			putBuffU8(bfp, AUTH_TYPE_NAK); // Auth_Type
			putBuffU8(bfp, gCtcDB.llid[bfp->llidIdx].auth.desired_type); // Desired Auth_Type

			eponOamExtDbg(DBG_OAM_L3, "<< Auth_Code: 0x02; Length: 0x02; Auth_Type: Nak=0x02; "
				"Desired Auth_Type: %d\r\n", gCtcDB.llid[bfp->llidIdx].auth.desired_type);
		}
		break;
	}
	addBuffFlag(bfp, BF_NEED_SEND);
	return SUCCESS;
}
/* ------------------------------------End Ext Auth--------------------------------------*/
