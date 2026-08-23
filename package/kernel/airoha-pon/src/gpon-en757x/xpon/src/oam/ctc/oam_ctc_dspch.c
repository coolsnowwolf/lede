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
	oam_ctc_dispatch.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <pthread.h>
#include "ctc/oam_ctc_dspch.h"
#include "ctc/oam_ctc_var.h"
#include "ctc/oam_ctc_node.h"
#include "ctc/oam_ctc_event.h"
#include "ctc/oam_ctc_swup.h"
#include "ctc/oam_ctc_auth.h"
#include "ctc/oam_ctc_alarm.h"
#include "ctc/oam_ctc_dba.h"
#include "ctc/oam_ctc_churning.h"
#include "ctc/oam_ext_mgr.h"
#include "ctc/oam_sys_api.h"
#include "epon_oam.h"
#include "api/mgr_api.h"
#ifdef TCSUPPORT_EPON_OAM_CUC
#include "ctc/oam_cuc_init.h"
#endif


#if 1//def TCSUPPORT_RESET
extern LastWorkNode_t gLastWorkTable[MAX_LAST_WORK_NODES];
#endif
extern int eponOamCtcDbgLvl;
extern eponOamCfg_t eponOamCfg;

OamCtcDB_t gCtcDB;
ExtHandler_t gCtcHandler;
u_char gOuiCtc[3] = {0x11, 0x11, 0x11};	//default

void getOuiFromSystem(u_char oui[3])
{
	char buf[10] = {0};
	u_int val = 0;
	if (TCAPI_SUCCESS == tcapi_get("Oam_CTC", "OUI", buf)){
		sscanf(buf, "%x", &val);
		oui[0] = (u_char)(val>>16);
		oui[1] = (u_char)(val>>8);
		oui[2] = (u_char)val;
	}
}

int initCtcHandler(ExtHandler_Ptr pExtHdlr)
{
	char buf[10] = {0};

	if (FAIL == oam_ctc_db_init(&gCtcDB)){
		return FAIL;
	}

	if (FAIL == oam_ctc_db_sync_init()){
		return FAIL;
	}
	getOuiFromSystem(gOuiCtc);
	pExtHdlr->oui[0] = gOuiCtc[0];
	pExtHdlr->oui[1] = gOuiCtc[1];
	pExtHdlr->oui[2] = gOuiCtc[2];
	/* if the OAM alive is dead, the low level oam need to tell the CTC OAM to init the discovery state!! */
	memset(pExtHdlr->dscvryState, DS_PASSIVE_WAIT, EPON_LLID_MAX_NUM);
	memset(pExtHdlr->usedVerion, 0, EPON_LLID_MAX_NUM);

	pExtHdlr->procInfoData = ctcProcInfoData;
	pExtHdlr->buildEventData = ctcBuildEventData;
	pExtHdlr->procExtData = ctcProcExtData;

	#if 1//def TCSUPPORT_RESET
	memset(gLastWorkTable, 0, sizeof(LastWorkNode_t)*MAX_LAST_WORK_NODES);
	gLastWorkTable[FAST_LEAVE].resetFunc = oamResetFastLeaveAdminControl;
	gLastWorkTable[FAST_LEAVE].flag = FALSE;
	gLastWorkTable[FAST_LEAVE].pData = NULL;
	#endif

	return SUCCESS;
}

static u_char eponOamSupportedVers[MAX_SUPPORT_OPERATOR_VER_CNT] = {0};

#define GET_VER_CNT(data, lastest_ver, tolVerCnt)\
        ({\
            u_int idx = 0;\
            for(idx = 0; idx < tolVerCnt; idx++)\
            {\
                if(lastest_ver == data[idx]){\
                    break;\
                }\
            }\
            ++idx;\
        })

/*
    Description: Get a Number of which contain all OAM protocol versions supported by
                 SW, this list will be used during OAM extended discovery process to
                 negotiate which protocol to be used by OLT & ONU
                 
    Params:      N/A
    Return Val:
                 on success, return the number of protocol versions supported by SW
                 on failure, return -1
*/
static int getSupportOamVerCnt()
{
    unsigned verCnt  = 0;
    unsigned int idx = 0;
    u_char operatorId = eponOamCfg.operators.id;
    u_char latestVer = eponOamCfg.operators.latestVer;
    u_char size = ARRAY_SIZE(eponOamSupportedVers);
    operVersion_t * allOperVers = &eponOamCfg.operators.vers;
    operVersion_t * oneOperVers  = NULL;

    if(operatorId < OPERATOR_NUM)
    {
        oneOperVers = &allOperVers[operatorId];
        verCnt = GET_VER_CNT(oneOperVers->data, latestVer, oneOperVers->verCnt);
        if(size < verCnt){
            return -1;
        }
    }
    else
    {
        for(idx = 0 ; idx  < OPERATOR_NUM; idx++)
        {
            oneOperVers = &allOperVers[idx];
            verCnt += oneOperVers->verCnt;
            if(size < verCnt){
                return -1;
            }
        }
    }
    return verCnt;
}

/*
Description:
    ONU supports OAM protocol version or not
Params:
    version
        OAM protocol version
Return Val:
    success, return TRUE
    failure, return FALSE
*/
static int isVersionSupported(u_char version)
{
    u_int i = 0;
    int cnt = getSupportOamVerCnt();

    if ( -1 == cnt)
        return FALSE;

    for(i = 0; i<= cnt ; i++)
    {
        if(version == eponOamSupportedVers[i]){
            return TRUE;
        }
    }
	return FALSE;
}

/*
    Description: Get a list which contain all OAM protocol versions supported by
                 SW, this list will be used during OAM extended discovery process to
                 negotiate which protocol to be used by OLT & ONU
                 
    Params:      N/A
    Return Val:
                 on success, return the number of protocol versions supported by SW
                 on failure, return -1
*/
static int getSupportOamVerList()
{
    unsigned verCnt  = getSupportOamVerCnt();
    unsigned int idx = 0;
    u_char operatorId = eponOamCfg.operators.id;
    u_char * pVers = eponOamSupportedVers;
    operVersion_t * allOperVers = &eponOamCfg.operators.vers;
    operVersion_t * oneOperVers  = NULL;

    if (-1 == verCnt)
        return -1;
    
    if(operatorId < OPERATOR_NUM)
    {
        oneOperVers = &allOperVers[operatorId];
        memcpy(pVers, oneOperVers->data, verCnt*sizeof(u_char));
    }
    else
    {
        for(idx = 0 ; idx  < OPERATOR_NUM; idx++)
        {
            oneOperVers = &allOperVers[idx];
            memcpy(pVers, oneOperVers->data, oneOperVers->verCnt*sizeof(u_char));
            pVers += oneOperVers->verCnt;
        }
    }

    return 0;
}

/* if the versionlist is more than 372, the packet can't contain it.
 * how to do under this condition?
 *
 * -------------------  <-- pInData / pOutData
 * | 1   info type   |
 * -------------------
 * | 1   length=7+4X |  <-- inLen / outLen
 * -------------------
 * | 3     OUI       |
 * -------------------
 * | 1   ExtSupport  |
 * -------------------
 * | 1    version    |
 * -------------------
 * | 3     OUI[1]    |
 * -------------------
 * | 1   version[1]  |
 * -------------------
 * |       ...       |
 * -------------------
 * */
int ctcProcInfoData(u_char llid, u_char *pOutData, int *outLen, u_char *pInData, int inLen)
{
	int len = -1, fstAny = 1;
	int i ;
	char buf[25] = {0};
    int verCnt = 0;

    operatorInfo_t * operInfo = &eponOamCfg.operators;

	OamCtcInfoHdr_Ptr pOutInfoHdr = (OamCtcInfoHdr_Ptr)pOutData;
	OamCtcInfoHdr_Ptr pInInfoHdr = (OamCtcInfoHdr_Ptr)pInData;

	u_char *ptr = pOutData;
	OamCtcOuiVersion_Ptr ovp = NULL;

	if (inLen < sizeof(OamCtcInfoHdr_t)){
		return FAIL;
	}

	#ifdef CONFIG_DEBUG
	if (eponOamCtcDbgLvl >= DBG_OAM_L2){
		eponOamExtDbg(DBG_OAM_L2, ">> CTC Info: extSupport = 0x%02X, version = 0x%02X\r\n", pInInfoHdr->extSupport, pInInfoHdr->version);
		ovp = (OamCtcOuiVersion_Ptr) (pInData + sizeof(OamCtcInfoHdr_t));
		for (i = 0; i < (inLen-sizeof(OamCtcInfoHdr_t))/sizeof(OamCtcOuiVersion_t); i++){
			eponOamExtDbg(DBG_OAM_L2, "\t%d : OUI = 0x%02X%02X%02X, version = 0x%02X\r\n", i, ovp->oui[0], ovp->oui[0], ovp->oui[0], ovp->version);
			ovp += 1;
		}
	}
	#endif

CTC_DSCVY_FSM:
	eponOamExtDbg(DBG_OAM_L2, "CTC LLID %d Discovery State : %d\r\n", llid, gCtcHandler.dscvryState[llid]);
	switch(gCtcHandler.dscvryState[llid]){
		case DS_PASSIVE_WAIT:
		{
            verCnt = getSupportOamVerCnt();

            if( -1 == verCnt )
                return FAIL;
            
            getSupportOamVerList();
            
			if (inLen < sizeof(OamCtcInfoHdr_t) + 4) // need OUI-Version list
				break;

			eponOamExtDbg(DBG_OAM_L2, "Enter CTC Discovery State SEND_EXT_INFO\r\n");
			len = sizeof(OamCtcInfoHdr_t) + verCnt * sizeof(OamCtcOuiVersion_t);

			pOutInfoHdr->type   = pInInfoHdr->type;
			pOutInfoHdr->length = len;
			pOutInfoHdr->oui[0] = pInInfoHdr->oui[0];
			pOutInfoHdr->oui[1] = pInInfoHdr->oui[1];
			pOutInfoHdr->oui[2] = pInInfoHdr->oui[2];

			if (isSameOui(pInInfoHdr->oui, gOuiCtc)){
				pOutInfoHdr->extSupport = 0x01;
			}else{ // README: can't reach here
				pOutInfoHdr->extSupport = 0x00;
			}

			pOutInfoHdr->version = 0x00; // the version is set to be 0x00

			PTR_MOVE(ptr, sizeof(OamCtcInfoHdr_t));
			ovp = (OamCtcOuiVersion_Ptr)ptr;

			for (i = 0; i < verCnt; ++i, ++ovp)
            {
				ovp->oui[0]  = gOuiCtc[0];
				ovp->oui[1]  = gOuiCtc[1];
				ovp->oui[2]  = gOuiCtc[2];
                ovp->version = eponOamSupportedVers[i];
			}

			*outLen = len;
			gCtcHandler.dscvryState[llid] = DS_SEND_EXT_INFO;
			break;
		}
		case DS_SEND_EXT_INFO:
		{
			if (inLen > sizeof(OamCtcInfoHdr_t)){
				gCtcHandler.dscvryState[llid] = DS_PASSIVE_WAIT;
				goto CTC_DSCVY_FSM;
			}

			len            = sizeof(OamCtcInfoHdr_t);// + sizeof(OamCtcOuiVersion_t);
			pOutInfoHdr->type    = pInInfoHdr->type;
			pOutInfoHdr->length  = len;
			pOutInfoHdr->oui[0]  = gOuiCtc[0];
			pOutInfoHdr->oui[1]  = gOuiCtc[1];
			pOutInfoHdr->oui[2]  = gOuiCtc[2];
			pOutInfoHdr->version = pInInfoHdr->version;

			*outLen    = len;

			if ( isVersionSupported(pInInfoHdr->version) && pInInfoHdr->extSupport == 0x01
				/* &&isSameOui(pInInfoHdr->oui, gOuiCtc)*/){
				pOutInfoHdr->extSupport   = 0x01;

				gCtcHandler.usedVerion[llid] = pInInfoHdr->version;
				gCtcHandler.dscvryState[llid] = DS_SEND_ANY;
#ifdef TCSUPPORT_AUTOBENCH
	system("echo epon ctcpass >> /proc/xpon/autobench");
	eponOamExtDbg(DBG_OAM_L0, "EPON CTC Discovery done!\n");
#endif
				eponOamExtDbg(DBG_OAM_L1, "Enter CTC Discovery State SEND_ANY.\n>> CTC Discovery done!\n");

				if (fstAny == 1)
				{
					// only CTC OAM need to reset system params to default before discovery done.
					if (pInInfoHdr->version >= 0x01 && pInInfoHdr->version<=0x30){
						oamResetSystemParam(llid);
						oamClearSelfConfig();
					}

					if (eponOamCfg.isExtOrg){
						if (0 != epon_sys_set_trx_mode(llid, ENUM_EPON_RX_FORWARDING, ENUM_EPON_TX_FORWARDING)){
							eponOamLlidDbg(EPON_DEBUG_LEVEL_ERROR, llid, ">> Set llid %d TxRx Forwarding Mode Fail!\n", llid);
						}
						eponOamLlidDbg(EPON_DEBUG_LEVEL_DEBUG, llid, ">> CTC Set llid %d Tx&Rx Mode = Forwarding!\n", llid);

						epon_sys_set_link_state(llid, LINK_UP);
					}
				}
			}else{
				pOutInfoHdr->extSupport   = 0x00;
				gCtcHandler.usedVerion[llid] = 0;
				gCtcHandler.dscvryState[llid] = DS_PASSIVE_WAIT;
				eponOamExtDbg(DBG_OAM_L2, "Enter CTC Discovery State PASSIVE_WAIT\r\n");
			}
			break;
		}
		case DS_SEND_ANY:
		{
			if (inLen == sizeof(OamCtcInfoHdr_t)){
				gCtcHandler.dscvryState[llid] = DS_SEND_EXT_INFO;
				fstAny = 0;
				goto CTC_DSCVY_FSM;
			}else if (inLen > sizeof(OamCtcInfoHdr_t)){
				gCtcHandler.dscvryState[llid] = DS_PASSIVE_WAIT;
				goto CTC_DSCVY_FSM;
			}
			eponOamExtDbg(DBG_OAM_L1, "We are CTC Discovery State SEND_ANY, not need any ext discovery info.\r\n");
			break;
		}
		default:
			eponOamExtDbg(DBG_OAM_L1, "Enter CTC Discovery Error: unknown state!\r\n");
			break;
	}

	return len;
}

int ctcBuildEventData(Buff_Ptr bfp, u_char *pData, int length)
{
	int len              = sizeof(OamOrgEventTlv_t);
	OamOrgEventTlv_t oet = {0};
	OamAlarmMsg_Ptr amp     = (OamAlarmMsg_Ptr)pData;
	OamAlarmNode_Ptr anp = findAlarmNodeById(amp->trapIndex);

	oet.type    = OAM_EVENT_ORG_SPEC;
	oet.oui[0]  = gOuiCtc[0];
	oet.oui[1]  = gOuiCtc[1];
	oet.oui[2]  = gOuiCtc[2];
	oet.alarmID = amp->trapIndex;
	oet.alarmState = *(amp->trapOtherInfo);
	oet.timeStamp = 0x0000;

	eponOamExtDbg(DBG_OAM_L4, "Enter %s: alarmId=%.4X\n", __FUNCTION__, amp->trapIndex);
	if (anp){
		len += +anp->length;
		eponOamExtDbg(DBG_OAM_L4, "%s Alarm: %s ID:%.4X Length:%d\n", __FUNCTION__, anp->name,anp->alarmId, anp->length);
		switch((anp->alarmId >> 8)){
			case 0x00: // ONU
				oet.objectType = OBJ_LF_ONU;
				oet.instaneNum = 0xFFFFFFFF;
				break;
			case 0x01: // PON IF
				oet.objectType = OBJ_LF_PONIF;
				oet.instaneNum = amp->devIndex_3;
				break;
			case 0x03: // PORT ETHER
				oet.objectType = OBJ_LF_PORT;
				oet.instaneNum = (0x01<<24) | (u_int)amp->devIndex_1;
				break;
			case 0x04: // POST
				oet.objectType = OBJ_LF_PORT;
				oet.instaneNum = (0x02<<24) | (u_int)amp->devIndex_1;
				break;
			case 0x05: // E1
				oet.objectType = OBJ_LF_PORT;
				oet.instaneNum = (0x05<<24) | (u_int)amp->devIndex_1;
				break;
			default:
				return FAIL;
				break;
		}

		eponOamExtDbg(DBG_OAM_L4, "%s ObjectType: %.4X InstanceNum: %.8X\n", __FUNCTION__, oet.objectType, oet.instaneNum);
		// here we need to store the param for every object's alarm/warning
		/*if (anp->state & ALARM_STATE_SEND){
			oet.alarmState = 0x00; // clear alarm
			anp->state &= ~ALARM_STATE_SEND;
			len += anp->state;
		}else{
			len += anp->state;
			oet.alarmState = 0x01; // report alarm
			anp->state |= ALARM_STATE_SEND;
		}*/
	}else{
		eponOamExtDbg(DBG_OAM_L4, "%s This Alarm: %.4X doesn't exist! Return Fail!\n", __FUNCTION__, amp->trapIndex);
		return FAIL;
	}

	/*if (isNotEnough(bfp, len)){
		return FAIL;
	}*/

	oet.objectType = htons(oet.objectType);
	oet.instaneNum = htonl(oet.instaneNum);
	oet.alarmID    = htons(oet.alarmID);
	oet.timeStamp  = htons(oet.timeStamp);
	oet.length = len;
	eponOamExtDbg(DBG_OAM_L4, "%s Alarm Tlv Length:%d\n", __FUNCTION__, len);

	putBuffData(bfp, (u_char*)&oet, sizeof(OamOrgEventTlv_t));
	addBuffFlag(bfp, BF_NEED_SEND);

	/*if (anp->length == 4){
		u_int alarmInfo = get32(amp->trapOtherInfo+1));
		putBuffUint(bfp, alarmInfo);
	}else if (anp->length == 8){
		//unsigned long long alarmInfo = get64(amp->trapOtherInfo+1);
		//putBuffData(bfp,,int length)
		putBuffData(bfp, amp->trapOtherInfo+1, 8);
	}*/
	putBuffData(bfp, amp->trapOtherInfo+1+(8-anp->length), anp->length);
	return len;
}

/*---------------------------
 * |  3       oui            |
 * ---------------------------   <----  pData
 * |  1      opCode          |
 * ---------------------------
 * |  X       data           |   X+1 = length
 * ---------------------------
 */
int ctcProcExtData(OUT Buff_Ptr bfp, IN Buff_Ptr obfp)
{
	int ret ;
	u_char opCode = *(obfp->data + obfp->offset);

	obfp->offset += 1;
	ret = rcvOamExtVar(bfp, opCode, obfp);
	return ret;
}

/* process the CTC OAM variable GET/SET request
 * ---------------------------
 * |  3       oui            |
 * ---------------------------
 * |  1      opCode          |
 * ---------------------------   <----  dp
 * |  X       data           |   X = length
 * ---------------------------
 * */
int rcvOamExtVar(OUT Buff_Ptr bfp, IN u_char opCode, IN Buff_Ptr obfp)
{
	u_char	*dp = obfp->data + obfp->offset; //getBuffPtr(u_char, obfp);
	int     length = obfp->usedLen - obfp->offset;
	int		ret = SUCCESS;

	if (isNotEnough(obfp, 1))
		return FAIL;

	eponOamExtDbg(DBG_OAM_L2, "\n>> LLID %d: Ext.opCode = 0x%02X\n", bfp->llidIdx, opCode);

	putBuffU24(bfp, gOuiCtc);

	switch(opCode){
	case EXT_OPCODE_GET_REQUEST:
		putBuffU8(bfp, EXT_OPCODE_GET_RESPONSE);
		ret = procVarGetRequest(bfp, dp, length);
		break;
	case EXT_OPCODE_SET_REQUEST:
		putBuffU8(bfp, EXT_OPCODE_SET_RESPONSE);
		ret = procVarSetRequest(bfp, dp, length);
		sem_post(&gCtcDB.db_sync_sem);
		break;
	case EXT_OPCODE_ONU_AUTHENTICATION:
		putBuffU8(bfp, EXT_OPCODE_ONU_AUTHENTICATION);
		ret = procExtAuth(bfp, obfp);
		break;
	case EXT_OPCODE_CHURNING:
		putBuffU8(bfp, EXT_OPCODE_CHURNING);
		ret = procExtChurning(bfp, dp, length);
		break;
	case EXT_OPCODE_DBA:
		putBuffU8(bfp, EXT_OPCODE_DBA);
		ret = procExtDBA(bfp, dp, length);
		break;
	case EXT_OPCODE_SOFTWARE_DOWNLOAD:
		putBuffU8(bfp, EXT_OPCODE_SOFTWARE_DOWNLOAD);
		ret = procExtSoftwareUpgrade(bfp, dp, length);
		break;
	case EXT_OPCODE_EXT_EVENT:
		putBuffU8(bfp, EXT_OPCODE_EXT_EVENT);
		ret = procExtEvent(bfp, dp, length);
		break;
	default:
		break;
	}

	return ret;
}

void postDBSetSyncMsg(u_int msg)
{
    gCtcDB.db_sync_sflag |= msg;
    sem_post(&gCtcDB.db_sync_sem);
}


