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
	oam_ctc_var.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#ifndef OAM_CTC_VAR_H
#define OAM_CTC_VAR_H
#include "ctc/oam_ext_cmn.h"
#include "oam_ctc_node.h"

#define OAM_CTC_VAR_TLV_DATA_MAX_LEN  128

/* OAM CTC OBJECT LEAF DEFINE */
#define OBJECT_LEAF_PORT  0x0001
#define OBJECT_LEAF_LLID  0x0003
#define OBJECT_LEAF_PONIF 0x0004
#define OBJECT_LEAF_ONU   0xFFFF

#pragma pack(push, 1)

typedef struct oamBranchLeafLength
{
	u_char  branch;
	u_short leaf;
	u_char  width;
}OamBranchLeafLength_t, *OamBranchLeafLength_Ptr;

/* OAM Object TLV Version V2.0 and before */
typedef struct oamCtcObjV1
{
	u_char  branch;
	u_short leaf;
	u_char  width;
	u_char  instance;
}OamCtcObjV1_t, *OamCtcObjV1_Ptr;

/* OAM Object TLV Version V2.1 and after */
typedef struct oamCtcObjV2
{
	u_char  branch;
	u_short leaf;
	u_char  width;
	u_int   instance;
}OamCtcObjV2_t, *OamCtcObjV2_Ptr;



#pragma pack(pop)

int procVarGetRequest(Buff_Ptr bfp, u_char *data, int length);
int procVarSetRequest(Buff_Ptr bfp, u_char *data, int length);
	
int procOamObjTlv(u_char **obj_hdr_ptr, u_char obj_branch, u_short *obj_leaf, u_char *obj_width, u_int *obj_index);
int procAttribActionGetTlv(Buff_Ptr bfp, OamObject_Ptr pObj, u_char **tlv_hdr, int *length);
int procAttribActionSetTlv(Buff_Ptr bfp, OamObject_Ptr pObj, u_char **tlv_hdr, int *length);
int getValueLength(OUT u_char *value, OUT int *outLen, IN OamBranchLeafLength_Ptr bllp, IN u_char *pData, IN int length);

int handleGetPortAll(Buff_Ptr bfp, OamObject_Ptr oop, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp);
int handleObjectGetResponse(Buff_Ptr bfp, OamObject_Ptr oop, u_char addObject, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp);
int handleGetResponse(Buff_Ptr bfp, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp);

int handleSetPortAll(Buff_Ptr bfp, OamObject_Ptr oop, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp, int length, u_char *value);
int handleObjectSetResponse(Buff_Ptr bfp, OamObject_Ptr oop,	u_char addObject, OamCtcBranchLeaf_Ptr cblfp, OamParamNode_Ptr opnp, int length, u_char *value);


int buildOamResponseDataTlv(Buff_Ptr bfp, OamCtcBranchLeaf_Ptr cblfp, int length, u_char *value );
int buildOamResponseCodeTlv(Buff_Ptr bfp, OamCtcBranchLeaf_Ptr cblfp, u_char retCode);
int buildOamObjTlv(Buff_Ptr bfp, OamObject_Ptr oop);


#endif
