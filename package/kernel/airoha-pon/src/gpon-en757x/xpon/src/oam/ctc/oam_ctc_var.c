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
	oam_ctc_var.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include <memory.h>
#include "ctc/oam_ctc_var.h"
#include "ctc/oam_sys_api.h"
#include "ctc/oam_ext_mgr.h"
#include "libcompileoption.h"

extern ExtHandler_t gCtcHandler;
extern int isOamNodeOperated;

extern int startloidAuth;

/* For Support Delay To Take Affect */
#if 1//def TCSUPPORT_RESET
LastWorkNode_t gLastWorkTable[MAX_LAST_WORK_NODES];

void procLastWorkTable(Buff_Ptr bfp)
{
	int i = 0;
	u_char ret;
	LastWorkNode_Ptr pr = NULL;
	for (; i < MAX_LAST_WORK_NODES; ++i)
	{
		pr = &gLastWorkTable[i];
		if (pr->flag == TRUE){
			eponOamExtDbg(DBG_OAM_L4, "%s: %d resetFunc\n", __FUNCTION__, i);
			ret = pr->resetFunc(pr->llidIdx, &(pr->obj), pr->length, pr->pData);
			buildOamResponseCodeTlv(bfp, &pr->branchLeaf, ret);
			if (pr->pData != NULL){
				free(pr->pData);
				pr->pData = NULL;
			}
			
			pr->flag = FALSE;
		}
	}
}

#endif


int procVarGetRequest(Buff_Ptr bfp, u_char *data, int length)
{
	u_char	*dp = data;

	OamObject_t object;
	OamObject_Ptr oop          = NULL;
	OamCtcBranchLeaf_Ptr cblfp = NULL;
	OamCtcObjV1_Ptr cobj1p     = NULL;
	OamCtcObjV2_Ptr cobj2p     = NULL;

	if (length < sizeof(OamCtcBranchLeaf_t)){
		return FAIL;
	}

	while (length >= sizeof(OamCtcBranchLeaf_t))
	{
		cblfp = (OamCtcBranchLeaf_Ptr)dp;

		switch(cblfp->branch)
		{
			case OAM_OBJECT_BRANCH_V1:
				if (length < (sizeof(OamCtcObjV1_t)+sizeof(OamCtcBranchLeaf_t))){
					goto OUT1;
				}else{
					length -= sizeof(OamCtcObjV1_t);
					cobj1p = (OamCtcObjV1_Ptr)dp;

					// README: need to test the width of Object is 1 byte ?
					object.branch = cobj1p->branch;
					object.leaf   = ntohs(cobj1p->leaf);
					object.index  = cobj1p->instance;
					oop = &object;

					eponOamExtDbg(DBG_OAM_L2, ">> CTC OBJ V1: Type=%-10s length=0x%02X value=0x%02X\n",\
					 		objTypeName(&object), cobj1p->width, object.index);

					oop = &object;
					PTR_MOVE(dp, sizeof(OamCtcObjV1_t));
				}
				break;
			case OAM_OBJECT_BRANCH_V2:
				#if 0
				if (gCtcHandler.usedVerion[bfp->llidIdx] < 0x21){
					oop = NULL;
					length -= sizeof(OamCtcObjV2_t);
					PTR_MOVE(dp, sizeof(OamCtcObjV2_t));
					continue;
				}
				#endif
				if (length < (sizeof(OamCtcObjV2_t) + sizeof(OamCtcBranchLeaf_t))){
					goto OUT1;
				}else{
					length -= sizeof(OamCtcObjV2_t);
					cobj2p = (OamCtcObjV2_Ptr)dp;

					object.branch = cobj2p->branch;
					object.leaf   = ntohs(cobj2p->leaf);
					object.index  = ntohl(cobj2p->instance);

					eponOamExtDbg(DBG_OAM_L2, ">> CTC OBJ V2: Type=%-10s length=0x%02X value=0x%08X\n",\
							objTypeName(&object), cobj2p->width, object.index);

					oop = &object;
					PTR_MOVE(dp, sizeof(OamCtcObjV2_t));
				}
				break;
			case OAM_STD_ATTR:
			case OAM_EXT_ATTR:
			case OAM_STD_ACT:
			case OAM_EXT_ACT:
				/*isOamNodeOperated = 1; */
				procAttribActionGetTlv(bfp, oop, &dp, &length);
				break;
			case OAM_END_TLV:
				goto OUT1;	
			default :
				eponOamExtDbg(DBG_OAM_L2, ">> CTC Unknown branch=0x%02X leaf=0x%04X\n",\
						cblfp->branch, ntohs(cblfp->leaf));
				goto OUT1;	// if we find one TLV which we don't know, we skip all the data from here to end.
		}//end switch
	}

OUT1:
	// from here the data is pad data, we skip.
	// step 4: send the reply packet to OLT


	return SUCCESS;
}

// TODO: Maybe build more than one reply packet to contain all response data
int procVarSetRequest(Buff_Ptr bfp, u_char *data, int length)
{
	u_char	*dp = data;

	OamObject_t object;
	OamObject_Ptr oop          = NULL;
	OamCtcBranchLeaf_Ptr cblfp = NULL;
	OamCtcObjV1_Ptr cobj1p     = NULL;
	OamCtcObjV2_Ptr cobj2p     = NULL;

	if (length < sizeof(OamCtcBranchLeaf_t)){
		return FAIL;
	}

	while (length >= sizeof(OamCtcBranchLeaf_t))
	{
		cblfp = (OamCtcBranchLeaf_Ptr)dp;
		switch(cblfp->branch)
		{
			case OAM_OBJECT_BRANCH_V1:
				if (length < (sizeof(OamCtcObjV1_t) + sizeof(OamCtcBranchLeaf_t))){
					goto OUT2;
				}else{
					length -= sizeof(OamCtcObjV1_t);
					cobj1p = (OamCtcObjV1_Ptr)dp;
					
					// README: need to test the width of Object is 1 byte ?
					object.branch = cobj1p->branch;
					object.leaf   = ntohs(cobj1p->leaf);
					object.index  = cobj1p->instance;

					eponOamExtDbg(DBG_OAM_L2, ">> CTC OBJ V1: Type=%-10s length=0x%02X value=0x%02X\n",\
							objTypeName(&object), cobj1p->width, object.index);

					oop = &object;					
					PTR_MOVE(dp, sizeof(OamCtcObjV1_t));
				}
				break;
			case OAM_OBJECT_BRANCH_V2:
				if (gCtcHandler.usedVerion[bfp->llidIdx] < 0x21){
					// TODO: only skip the TLV and it's get request branch&leaf
				}
				if (length < (sizeof(OamCtcObjV2_t) + sizeof(OamCtcBranchLeaf_t))){
					goto OUT2;
				}else{
					length -= sizeof(OamCtcObjV2_t);
					cobj2p = (OamCtcObjV2_Ptr)dp;

					object.branch = cobj2p->branch;
					object.leaf   = ntohs(cobj2p->leaf);
					object.index  = ntohl(cobj2p->instance);
					
					eponOamExtDbg(DBG_OAM_L2, ">> CTC OBJ V2: Type=%-10s length=0x%02X value=0x%08X\n",\
							objTypeName(&object), cobj2p->width, object.index);
					
					oop = &object;
					PTR_MOVE(dp, sizeof(OamCtcObjV2_t));
				}
				break;
			case OAM_STD_ATTR:
			case OAM_EXT_ATTR:
			case OAM_STD_ACT:
			case OAM_EXT_ACT:					
				isOamNodeOperated++;
				if (startloidAuth == 0 && isOamNodeOperated >= 2) {/* if no loid auth, mac auth ok, set traffic up */
					epon_sys_set_link_state(bfp->llidIdx, LINK_UP);
				}
				
				procAttribActionSetTlv(bfp, oop, &dp, &length);
				break;
			case OAM_END_TLV:
				goto OUT2;
			default :
				eponOamExtDbg(DBG_OAM_L2, ">> CTC Unknown branch=0x%02X leaf=0x%04X\n",\
						cblfp->branch, ntohs(cblfp->leaf));
				goto OUT2;	// if we find one TLV which we don't know, we skip all the data from here to end.
		}//end switch
	}

OUT2:
	// from here the data is pad data, we skip.

    #if 1//def TCSUPPORT_RESET
    procLastWorkTable(bfp);
    #endif
    
	return SUCCESS;
}

int handleGetPortAll(Buff_Ptr bfp, OamObject_Ptr oop, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp)
{
	int i, ret = SUCCESS;
	int ports = getPortNum();
	OamObject_t oob = *oop;

	for(i=0; i< ports; i++){
		if (oob.branch == OAM_OBJECT_BRANCH_V1)
			oob.index = i+1;
		else
			oob.index =  (PORT_ETHER<<24) | (i+1);
		ret = handleObjectGetResponse(bfp, &oob, TRUE, cblfp, opnp);
	}
	return ret ;
}

int handleObjectGetResponse(Buff_Ptr bfp, OamObject_Ptr oop,
	u_char addObject, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp)
{
	u_char ret;
	int realUseLen;
	int retLen = 0;
	u_char value[OAM_TLV_MAX_VALUE_LEN] = {0};
	int objectLen = 0;

	if(addObject){
		objectLen = (oop->branch == OAM_OBJECT_BRANCH_V1)?OAM_OBJECT_LENGTH_V1:OAM_OBJECT_LENGTH_V2;
	}
	realUseLen = sizeof(OamBranchLeafLength_t) + objectLen;

	ret = opnp->oamGetFunc(bfp->llidIdx, oop, &retLen, value);
	if (ret == SUCCESS)
		realUseLen += ((OAM_CTC_VAR_TLV_DATA_MAX_LEN+sizeof(OamBranchLeafLength_t))*
					(retLen/OAM_CTC_VAR_TLV_DATA_MAX_LEN)) +
					retLen%OAM_CTC_VAR_TLV_DATA_MAX_LEN;

	if (realUseLen + bfp->usedLen > BUFF_MAX_LEN){
		handleBuffNotEnough(bfp, /*sizeof(OamHdr_t)+*/sizeof(OamOrgHdr_t));
		buildOamObjTlv(bfp, oop);
	}else{
		if (addObject)
			buildOamObjTlv(bfp, oop);
	}

	if (ret == SUCCESS)
		buildOamResponseDataTlv(bfp, cblfp, retLen, value);
	else if (ret == DIRECT_COPY){
		putBuffData(bfp, value, retLen);
		addBuffFlag(bfp, BF_NEED_SEND);
	}else
		buildOamResponseCodeTlv(bfp, cblfp, ret /*retLen*/);

	return SUCCESS;
}

int handleGetResponse(Buff_Ptr bfp, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp)
{
	/* use a big enough buff to store GetData */
	u_char ret;
	int   retLen = 0;
	u_char value[OAM_TLV_MAX_VALUE_LEN] = {0};
	int realUseLen = sizeof(OamBranchLeafLength_t);

	ret = opnp->oamGetFunc(bfp->llidIdx, NULL, &retLen, value);
	if (ret == SUCCESS)
		realUseLen += ((OAM_CTC_VAR_TLV_DATA_MAX_LEN+sizeof(OamBranchLeafLength_t)) *
					(retLen/OAM_CTC_VAR_TLV_DATA_MAX_LEN)) +
					retLen%OAM_CTC_VAR_TLV_DATA_MAX_LEN;
	else if (ret == DIRECT_COPY){
		realUseLen += retLen;
	}

	if (realUseLen + bfp->usedLen > BUFF_MAX_LEN){
		handleBuffNotEnough(bfp, /*sizeof(OamHdr_t)+*/sizeof(OamOrgHdr_t));
	}

	if (ret == SUCCESS){
		buildOamResponseDataTlv(bfp, cblfp, retLen, value);
	}else if (ret == DIRECT_COPY){
		putBuffData(bfp, value, retLen);
		addBuffFlag(bfp, BF_NEED_SEND);
		eponOamExtDbg(DBG_OAM_L3, "<< CTC Build Reply Data: ");
		eponOamDumpHexString(DBG_OAM_L3, value, retLen);
	}else{
		buildOamResponseCodeTlv(bfp, cblfp, ret /*retLen*/);
	}

	return SUCCESS;
}

/* process one Object's Attribute or Action TLVS
 * -----------------------   <---- *tlv_hdr (before call this func)
 * |  1    branch        |
 * -----------------------
 * |  2    leaf          |
 * -----------------------
 * |       ......        |
 * -----------------------
 * |  1   branch         |
 * -----------------------
 * |  2    leaf          |
 * -----------------------   <---- *tlv_hdr (after call this func)
 * */
int procAttribActionGetTlv(Buff_Ptr bfp, OamObject_Ptr pObj, u_char **tlv_hdr, int *length)
{
	u_char	*dp                     = (*tlv_hdr);
	int     len                     = *length;
	u_char   isObjectAdded           = FALSE;
	OamCtcBranchLeaf_Ptr cblfp      = NULL;
	OamParamNode_Ptr opnp = NULL;


	cblfp = (OamCtcBranchLeaf_Ptr)dp;

	while( (len >= sizeof(OamCtcBranchLeaf_t)) &&
		   isOamAttribActBranch(cblfp->branch)
		 )
	{
		cblfp->leaf = ntohs(cblfp->leaf);
		eponOamExtDbg(DBG_OAM_L2, ">> CTC Var_Get_Request branch=0x%02X leaf=0x%04X\n", cblfp->branch, cblfp->leaf);

		opnp = findOamParamNodeByBranchLeaf(cblfp->branch, cblfp->leaf);
		/* Skip the Branch&Leaf we don't know */
		if(opnp && opnp->oamGetFunc){
			eponOamExtDbg(DBG_OAM_L2, ">> Node: %s, Object: %s\n", opnp->name, nodeTypeName(opnp->objectType));

			//to test the object is right
			if (pObj && isObjectBranchLeaf(pObj, opnp)){
				if (//opnp->objectType & OBJ_BIT_PORT &&
					isOamPortAll(pObj) == TRUE)
				{
					handleGetPortAll(bfp, pObj, cblfp, opnp);
				}else{
					// the object TLV is added with the Response TLV
					handleObjectGetResponse(bfp, pObj, !isObjectAdded, cblfp, opnp);
					if (FALSE == isObjectAdded){
						isObjectAdded = TRUE;
					}
				}
			}else{
				pObj = NULL;
				/* If has no Object, we only handle the ONU's B&L we know.
				 * Others will be skiped! */
				if ( FALSE == isNeedObject(opnp)
				){
					handleGetResponse(bfp, cblfp, opnp);
				}else{
					eponOamExtDbg(DBG_OAM_L2, ">> Warning: The node need Object, but no Object!\n");
					//if ctc version is 0x21, must return VAR_BAD_PARAMETERS
					buildOamResponseCodeTlv( bfp, cblfp, VAR_BAD_PARAMETERS);
				}
			}
		}else{
			// buildOamResponseCodeTlv(bfp, cblfp, VAR_BAD_PARAMETERS); //TODO: needed?
			eponOamExtDbg(DBG_OAM_L2, ">> Node: not found or not support get\n");
			buildOamResponseCodeTlv( bfp, cblfp, VAR_BAD_PARAMETERS);
		}
		len -= sizeof(OamCtcBranchLeaf_t);
		PTR_MOVE(dp, sizeof(OamCtcBranchLeaf_t));
		cblfp = (OamCtcBranchLeaf_Ptr)dp;
	}

	*tlv_hdr = dp;
	*length = len;

	return SUCCESS;
}


/* The Attribute/Action Get Response TLV we want build is like this as below.
 * -----------------------   <---- bfp->usedLen   (before call this func)
 * |   1     branch      |
 * -----------------------
 * |   2     leaf        |
 * -----------------------
 * |   1     length      |
 * -----------------------
 * |  X/128  data        |
 * -----------------------   <----bfp->usedLen  (after call this func)
 * */
int buildOamResponseDataTlv(OUT Buff_Ptr bfp, 
	IN OamCtcBranchLeaf_Ptr cblfp, IN int length, IN u_char *value )
{
	int count  = length/OAM_CTC_VAR_TLV_DATA_MAX_LEN;
	u_char lastLen = length % OAM_CTC_VAR_TLV_DATA_MAX_LEN;
	u_char *dp = value;
	int i;

	for (i = 0; i< count; i++){
		putBuffU8(bfp, cblfp->branch);
		putBuffU16(bfp, cblfp->leaf);
		putBuffU8(bfp, 0x00);// if the real_len == 128 , the real is 0x00?
		putBuffData(bfp, dp, OAM_CTC_VAR_TLV_DATA_MAX_LEN);

		PTR_MOVE(dp, OAM_CTC_VAR_TLV_DATA_MAX_LEN);
	}

	if (lastLen > 0){
		putBuffU8(bfp, cblfp->branch);
		putBuffU16(bfp, cblfp->leaf);
		putBuffU8(bfp, lastLen);
		putBuffData(bfp, dp, lastLen);
	}

	if(length > 0){
		addBuffFlag(bfp, BF_NEED_SEND);

		eponOamExtDbg(DBG_OAM_L3, "<< CTC Build GetResponse TLV: branch:0x%02X leaf:0x%04X length:%d\n<< data:",\
		cblfp->branch, cblfp->leaf, length);
		eponOamDumpHexString(DBG_OAM_L3, value, length);
	}
	eponOamExtDbg(DBG_OAM_L4, "<< CTC Build SUCCESS, and return.\n");
	return SUCCESS;
}



/* The Object TLV we want build is like this as below.
 * -----------------------   <---- bfp->usedLen  (before call this func)
 * |   1    branch       |
 * -----------------------
 * |   2    leaf         |
 * -----------------------
 * |   1    width        |
 * -----------------------
 * |  1/4   index        |
 * -----------------------   <----bfp->usedLen  (after call this func)
 * */
int buildOamObjTlv(OUT Buff_Ptr bfp, IN OamObject_Ptr pObj)
{
	u_char index;
	u_char width;

	#if 0
	/* after the object TLV, need to bring response TLV */
	if ((bfp->usedLen + 8 > BUFF_MAX_LEN)){
		return FAIL;
	}
	#endif
	putBuffU8(bfp, pObj->branch);
	putBuffU16(bfp, pObj->leaf);

	if (pObj->branch == OAM_OBJECT_BRANCH_V1){
		width = OAM_OBJECT_LEAF_WIDTH_V1;
		putBuffU8(bfp, OAM_OBJECT_LEAF_WIDTH_V1);

		index = (u_char)pObj->index;
		putBuffU8(bfp, index);
	}else if (pObj->branch == OAM_OBJECT_BRANCH_V2){
		width = OAM_OBJECT_LEAF_WIDTH_V2;
		putBuffU8(bfp, OAM_OBJECT_LEAF_WIDTH_V2);

		putBuffUint(bfp, pObj->index);
	}else{
		/* will never pass to here */
		eponOamExtDbg(DBG_OAM_L2, "<< CTC Build Object TLV: wrong Object, return error!\n");
		bfp->usedLen -= 3;
		return FAIL;
	}

	eponOamExtDbg(DBG_OAM_L3, "<< CTC Build Object TLV: branch:0x%02X leaf:0x%04X width:%d, value:0x%04X\n", \
		pObj->branch, pObj->leaf, width , pObj->index);

	return SUCCESS;
}
/*-------------Variable Get Request and Response End---------------*/


/*------------Variable Set Request and Response Begin--------------*/
int handleSetPortAll(Buff_Ptr bfp, OamObject_Ptr oop, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp, int length, u_char *value)
{
	//2 reply once for all or once for one ??
	int i;
	int ports = getPortNum();
	OamObject_t oob = *oop;

	for(i=0; i< ports; i++){
		if (oob.branch == OAM_OBJECT_BRANCH_V1)
			oob.index = i+1;
		else
			oob.index =  (PORT_ETHER<<24) | (i+1);
		handleObjectSetResponse(bfp, &oob, TRUE, cblfp, opnp, length, value);
	}
	return SUCCESS ;
}

int handleObjectSetResponse(Buff_Ptr bfp, OamObject_Ptr oop,
	u_char addObject, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp, int length, u_char *value)
{
	u_char ret;
	int realUseLen;
	int objectLen = 0;

	if (opnp->length != 0xFF && opnp->length > length) {
		ret = VAR_BAD_PARAMETERS;
	}else{
		ret = opnp->oamSetFunc(bfp->llidIdx, oop, length, value);
	}
	
	if(addObject){
		objectLen = (oop->branch == OAM_OBJECT_BRANCH_V1)?OAM_OBJECT_LENGTH_V1:OAM_OBJECT_LENGTH_V2;
	}
	
	realUseLen = sizeof(OamBranchLeafLength_t) + objectLen;
	if (realUseLen + bfp->usedLen > BUFF_MAX_LEN){
		handleBuffNotEnough(bfp, /*sizeof(OamHdr_t)+*/sizeof(OamOrgHdr_t));
		buildOamObjTlv(bfp, oop);
	}else{
		if (addObject)
			buildOamObjTlv(bfp, oop);
	}
	
	if (ret != RET_NOT_REPLY)
		buildOamResponseCodeTlv(bfp, (OamCtcBranchLeaf_Ptr)cblfp, ret);
	
	return SUCCESS;
}


/* process one Object's Attribute or Action TLVS
 * In this function, we need not to care about reply buf not enough.
 * because the reply packet's length is always less than recvied.
 * -----------------------   <---- *tlv_hdr (before call this func)
 * |	1	branch       |
 * -----------------------
 * |	2	leaf         |
 * -----------------------
 * |    1   length       |
 * -----------------------
 * |  X/128 data         | <--value
 * -----------------------
 * |		.....        |
 * -----------------------
 * |	1	branch	     |
 * -----------------------
 * |	2	leaf         |
 * -----------------------
 * |    1   length       |
 * -----------------------
 * |  X/128 data         | <--value
 * -----------------------   <---- *tlv_hdr (after call this func)
 * */
int procAttribActionSetTlv(Buff_Ptr bfp, OamObject_Ptr pObj, u_char **tlv_hdr, int *length)
{
	u_char  ret;
	u_char  *dp                    = (*tlv_hdr);
	int    len                    = *length;
	int    isObjectAdded          = FALSE;
	OamParamNode_Ptr        opnp  = NULL;
	OamBranchLeafLength_Ptr blflp = NULL;

	blflp = (OamBranchLeafLength_Ptr)dp;
	while ( (len >= sizeof(OamCtcBranchLeaf_t)) &&
			isOamAttribActBranch(blflp->branch) )
	{
		opnp = findOamParamNodeByBranchLeaf(blflp->branch, ntohs(blflp->leaf));
		eponOamExtDbg(DBG_OAM_L2, ">> CTC Var_Set_Request branch=0x%02X "
				"leaf=0x%04X,", blflp->branch, ntohs(blflp->leaf));

		if (opnp){
			/* we use a big enough buff to store
			 * more than one TLV's data  */
			int		totalLen = 0;
			u_char	value[OAM_TLV_MAX_VALUE_LEN] = {0};
			int     procLen = 0;

                   // to process the TLV which has no length and vlaue.
			if (opnp->length == 0x00){
				PTR_MOVE(dp, sizeof(OamCtcBranchLeaf_t));
				len -= sizeof(OamCtcBranchLeaf_t);
			}else{
			       // to process split tlvs the length bigger than 128. 
				procLen = getValueLength(value, &totalLen, blflp, dp, len);
				PTR_MOVE(dp, procLen);
				len -= procLen;
			}

			blflp->leaf = ntohs(blflp->leaf);
			eponOamExtDbg(DBG_OAM_L2, "width=0x%02X\n>> Node: %s Object: %s\n>> Data =", \
					totalLen, opnp->name, nodeTypeName(opnp->objectType));
			eponOamDumpHexString(DBG_OAM_L2, value, totalLen);

			if (opnp->oamSetFunc){
				if (pObj && isObjectBranchLeaf(pObj, opnp)){
					if (isOamPortAll(pObj) == TRUE){
						handleSetPortAll(bfp, pObj, (OamCtcBranchLeaf_Ptr)blflp, opnp, totalLen, value);
					}else{					
						handleObjectSetResponse(bfp, pObj, !isObjectAdded, (OamCtcBranchLeaf_Ptr)blflp, opnp, totalLen, value);
						if (FALSE == isObjectAdded){
							isObjectAdded = TRUE;
						}
					}
				}else{
					pObj = NULL;
					if (FALSE == isNeedObject(opnp)){
						if (opnp->length != 0xFF && opnp->length > totalLen) {
							ret = VAR_BAD_PARAMETERS;
						}else{
							ret = opnp->oamSetFunc(bfp->llidIdx, pObj, totalLen, value);
						}
						if (ret != RET_NOT_REPLY)
							buildOamResponseCodeTlv(bfp, (OamCtcBranchLeaf_Ptr)blflp, ret);
					}else{
						eponOamExtDbg(DBG_OAM_L2, ">> Warning: The node need Object, but no Object!\n");
						// if ctc version is 0x21, must return VAR_BAD_PARAMETERS
					 	buildOamResponseCodeTlv(bfp, (OamCtcBranchLeaf_Ptr)blflp, VAR_BAD_PARAMETERS);
					 }
				}
			}else{
				// buildOamResponseCodeTlv(bfp, (OamCtcBranchLeaf_Ptr)blflp, VAR_BAD_PARAMETERS);
				eponOamExtDbg(DBG_OAM_L2, ">> Warning: The node doesn't support set!\n");
			}
		}else{
			// we only skip the branch_leaf that we don't know
			eponOamExtDbg(DBG_OAM_L2, "width=0x%02X\n>> Warning Node: not found\n", blflp->width);

			PTR_MOVE(dp, (sizeof(OamBranchLeafLength_t) + blflp->width));
			len -= sizeof(OamBranchLeafLength_t) + blflp->width;
			buildOamResponseCodeTlv(bfp, (OamCtcBranchLeaf_Ptr)blflp, VAR_BAD_PARAMETERS);
		}

		
		blflp = (OamBranchLeafLength_Ptr)dp;
	}

	*tlv_hdr = dp;
	*length = len;
	return SUCCESS;
}

/*
	OUT value  output the data set
	OUT outLen   output the length of data set 
	RETURN       the buff's length we had processed.
*/
int getValueLength(OUT u_char *value, OUT int *outLen, IN OamBranchLeafLength_Ptr bllp, IN u_char *pData, IN int length)
{
	int retLen = 0, realLen;
	u_char *odp = value, *dp = pData;
	u_char selBranch = bllp->branch;
	u_short selLeaf = bllp->leaf;

	*outLen = 0;
	/* MODIFY: need to see the length == 128??
	 * CTC EPON 6.5.9: if continuous TLVs contain
	 * same B&L, should be conside one TLV. */
	while ( (length > sizeof(OamBranchLeafLength_t)) &&
			(selBranch == bllp->branch) &&
			(selLeaf == bllp->leaf) )
	{
		length -= sizeof(OamBranchLeafLength_t);
		retLen += sizeof(OamBranchLeafLength_t);
		PTR_MOVE(dp, sizeof(OamBranchLeafLength_t));
		/* 802.3ah-2004 57.6.2.1 skip the B&L&L
		 * when bit7 = 1, there is no variable value field */
		if (bllp->width >= 128){
			bllp = (OamBranchLeafLength_Ptr)dp;
			continue;
		}

		realLen = (bllp->width == 0x00)? 128: bllp->width;// if the length is 0x00, it is real mean 128.
		if (length < realLen){ // if the buf length less than need length, skip the all pdu from here
			retLen += length;
			break;
		}

		memcpy(odp, dp, realLen);
		PTR_MOVE(odp, realLen);
		PTR_MOVE(dp, realLen);

		*outLen += realLen;
		length  -= realLen;
		retLen  += realLen;

		bllp = (OamBranchLeafLength_Ptr)dp;

		if (selBranch == OAM_EXT_ACT && ntohs(selLeaf) == OAM_LF_SIPDIGITMAP)
			break; // special process
	}
	return retLen;
}

/* The Attribute/Action Set Response TLV
 * we want build is like this as below.
 * -----------------------
 * |    1    branch      |
 * -----------------------
 * |    2    leaf        |
 * -----------------------
 * |    1    retCode     | <--length
 * -----------------------
 * */
int buildOamResponseCodeTlv( Buff_Ptr bfp, OamCtcBranchLeaf_Ptr cblfp, u_char retCode )
{
	eponOamExtDbg(DBG_OAM_L3, "<< CTC Build SetResponse TLV: branch:0x%02X leaf:0x%04X retCode:0x%02X\n",\
			cblfp->branch, cblfp->leaf, retCode);

	putBuffU8(bfp, cblfp->branch);
	putBuffU16(bfp, cblfp->leaf);
	putBuffU8(bfp, retCode);
	addBuffFlag(bfp, BF_NEED_SEND);
	return SUCCESS;
}
/*------------Variable Set Request and Response End--------------*/

