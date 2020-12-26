/*
 *  $Id$
 *  $DateTime$
 *  Jeffrey Chang
 */
/*
     This file contains IOCTL for MU-MIMO specfic commands
 */
/*******************************************************************************
 * Copyright (c) 2014 MediaTek Inc.
 *
 *  All rights reserved. Copying, compilation, modification, distribution
 *  or any other use whatsoever of this material is strictly prohibited
 *  except in accordance with a Software License Agreement with
 *  MediaTek Inc.
 * ******************************************************************************
 */
/******************************************************************************
 * LEGAL DISCLAIMER
 *
 * BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
 * SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
 * PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
 * DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
 * ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
 * WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
 * SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
 * WARRANTY CLAIM RELATING THERetO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
 * FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
 * CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
 * BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
 * ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
 * WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
 * OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
 * THEREOF AND RELATED THERetO SHALL BE SETTLED BY ARBITRATION IN SAN
 * FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
 * (ICC).
 * ******************************************************************************
 */
#include "rt_config.h"
#ifdef CFG_SUPPORT_MU_MIMO
/* For debugging, Not for ATE */
#define SU 0
#define MU 1
#define MU_PROFILE_NUM  32 /* to be removed */
#define QD_RAW_DATA_LEN 56
static VOID eventDispatcher(struct cmd_msg *msg, char *rsp_payload, UINT16 rsp_payload_len);
INT SetMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pValid = NULL;
	PCHAR pIdx = NULL;
	PCHAR pBaMask = NULL;
	PCHAR pWlanIdx = NULL;
	CMD_MU_SET_MUPROFILE_ENTRY param = {0};
	UINT32 cmd = MU_SET_MUPROFILE_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: arg = %s\n", __func__, arg));
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pValid = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pBaMask = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pWlanIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.valid = (BOOLEAN)os_str_tol(pValid, 0, 10);
	param.index = os_str_tol(pIdx, 0, 10);
	param.baMask = os_str_tol(pBaMask, 0, 16);
	param.wlanIdx = os_str_tol(pWlanIdx, 0, 10);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	struct cmd_msg *msg = NULL;
	INT32 Ret = TRUE;
	PCHAR pIdx = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MUPROFILE_ENTRY result = {0};
	UINT32 cmd = MU_GET_MUPROFILE_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIdx, 0, 10);
	index = cpu2le32(index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* | ucNumUser | ucBW | ucNS0 | ucNS1 | ucNS2 | ucNS3 |
 * | ucPFIDUser0 | ucPFIDUser1 | ucPFIDUser2 | ucPFIDUser3 |
 * | fgIsShortGI | fgIsUsed | fgIsDisable |
 * | ucInitMCSUser0 | ucInitMCSUser1 | ucInitMCSUser2 | ucIitMCSUser3|
 * | ucdMCSUser0 | ucdMCSUser1 | ucdMCSUser2 | ucdMCSUser3|
*/
/* iwpriv ra0 [index] [num_user: 0/1/2/3] [bw:/0/1/2/3] [ns0:0/1] [ns1:0/1] [ns2:0/1] [ns3:0/1]
   [pfid0] [pfid1] [pfid2] [pfid3] [sgi] [used] [dis] [initMcs0] [initMcs1]
   [initMcs2] [initMcs3] [dMcs0] [dMcs1] [dMcs2] [dMcs3]
*/
INT SetGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pNumUser = NULL;
	PCHAR pIndex = NULL;
	PCHAR pBw = NULL;
	PCHAR pNs0 = NULL;
	PCHAR pNs1 = NULL;
	PCHAR pNs2 = NULL;
	PCHAR pNs3 = NULL;
	PCHAR ppfid0 = NULL;
	PCHAR ppfid1 = NULL;
	PCHAR ppfid2 = NULL;
	PCHAR ppfid3 = NULL;
	PCHAR psgi = NULL;
	PCHAR pused = NULL;
	PCHAR pdis = NULL;
	PCHAR pinitMcsUser0 = NULL;
	PCHAR pinitMcsUser1 = NULL;
	PCHAR pinitMcsUser2 = NULL;
	PCHAR pinitMcsUser3 = NULL;
	PCHAR pdMcsUser0 = NULL;
	PCHAR pdMcsUser1 = NULL;
	PCHAR pdMcsUser2 = NULL;
	PCHAR pdMcsUser3 = NULL;
	CMD_MU_SET_GROUP_TBL_ENTRY param = {0};
	UINT32 cmd = MU_SET_GROUP_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if  (pch != NULL)
		pNumUser = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pBw = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		psgi = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pused = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdis = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdMcsUser0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdMcsUser1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdMcsUser2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pdMcsUser3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.index = os_str_tol(pIndex, 0, 10);
	param.numUser = os_str_tol(pNumUser, 0, 10);
	param.BW = os_str_tol(pBw, 0, 10);
	param.PFIDUser0 = os_str_tol(ppfid0, 0, 10);
	param.PFIDUser1 = os_str_tol(ppfid1, 0, 10);
	param.PFIDUser2 = os_str_tol(ppfid2, 0, 10);
	param.PFIDUser3 = os_str_tol(ppfid3, 0, 10);
	param.fgIsShortGI = (BOOLEAN) os_str_tol(psgi, 0, 10);
	param.fgIsUsed = (BOOLEAN) os_str_tol(pused, 0, 10);
	param.fgIsDisable = (BOOLEAN) os_str_tol(pdis, 0, 10);
	param.initMcsUser0 = os_str_tol(pinitMcsUser0, 0, 10);
	param.initMcsUser1 = os_str_tol(pinitMcsUser1, 0, 10);
	param.initMcsUser2 = os_str_tol(pinitMcsUser2, 0, 10);
	param.initMcsUser3 = os_str_tol(pinitMcsUser3, 0, 10);
	param.dMcsUser0 = os_str_tol(pdMcsUser0, 0, 10);
	param.dMcsUser1 = os_str_tol(pdMcsUser1, 0, 10);
	param.dMcsUser2 = os_str_tol(pdMcsUser2, 0, 10);
	param.dMcsUser3 = os_str_tol(pdMcsUser3, 0, 10);
	param.NS0 = os_str_tol(pNs0, 0, 10);
	param.NS1 = os_str_tol(pNs1, 0, 10);
	param.NS2 = os_str_tol(pNs2, 0, 10);
	param.NS3 = os_str_tol(pNs3, 0, 10);
#ifdef RT_BIG_ENDIAN
	param.index = cpu2le32(param.index);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Values %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			  param.index, param.numUser, param.BW,
			  param.NS0, param.NS1, param.NS2, param.NS3,
			  param.PFIDUser0, param.PFIDUser1, param.PFIDUser2, param.PFIDUser3,
			  param.fgIsShortGI, param.fgIsUsed, param.fgIsDisable, param.initMcsUser0,
			  param.initMcsUser1, param.initMcsUser2, param.initMcsUser3,
			  param.dMcsUser0, param.dMcsUser1, param.dMcsUser2, param.dMcsUser3));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MU_SET_GROUP_TBL_ENTRY));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(CMD_MU_SET_GROUP_TBL_ENTRY));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_GROUP_TBL_ENTRY result = {0};
	UINT32 cmd = MU_GET_GROUP_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*<-------------------------------------------------------------------------->*/
/* iwpriv ra0 [index] [lowMemberStatus] [highMemberStatus] [[lowUserPosition0]
	[lowUserPosition1][highUserPosistion0] [highUserPosistion1] - in hex
*/
INT SetClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	PCHAR pLowGidUserMemberStatus = NULL;
	PCHAR pHighGidUserMemberStatus = NULL;
	PCHAR pLowGidUserPosition0 = NULL;
	PCHAR pLowGidUserPosition1 = NULL;
	PCHAR pHighGidUserPosition0 = NULL;
	PCHAR pHighGidUserPosition1 = NULL;
	CMD_MU_SET_CLUSTER_TBL_ENTRY param = {0};
	UINT32 cmd = MU_SET_CLUSTER_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pLowGidUserMemberStatus = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pHighGidUserMemberStatus = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pLowGidUserPosition0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pLowGidUserPosition1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pHighGidUserPosition0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pHighGidUserPosition1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.index = os_str_tol(pIndex, 0, 10);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s\n", __func__));
	/* do we need change edian? */
	param.gidUserMemberStatus[0] = os_str_tol(pLowGidUserMemberStatus, 0, 16);
	param.gidUserMemberStatus[1] = os_str_tol(pHighGidUserMemberStatus, 0, 16);
	param.gidUserPosition[0] = os_str_tol(pLowGidUserPosition0, 0, 16);
	param.gidUserPosition[1] = os_str_tol(pLowGidUserPosition1, 0, 16);
	param.gidUserPosition[2] = os_str_tol(pHighGidUserPosition0, 0, 16);
	param.gidUserPosition[3] = os_str_tol(pHighGidUserPosition1, 0, 16);
#ifdef RT_BIG_ENDIAN
	param.gidUserMemberStatus[0] = cpu2le32(param.gidUserMemberStatus[0]);
	param.gidUserMemberStatus[1] = cpu2le32(param.gidUserMemberStatus[1]);
	param.gidUserPosition[0] = cpu2le32(param.gidUserPosition[0]);
	param.gidUserPosition[1] = cpu2le32(param.gidUserPosition[1]);
	param.gidUserPosition[2] = cpu2le32(param.gidUserPosition[2]);
	param.gidUserPosition[3] = cpu2le32(param.gidUserPosition[3]);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Values: %d %d %d %d %d %d %d\n", param.index,
			  param.gidUserMemberStatus[0], param.gidUserMemberStatus[1],
			  param.gidUserPosition[0], param.gidUserPosition[1],
			  param.gidUserPosition[2], param.gidUserPosition[3]));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(CMD_MU_SET_CLUSTER_TBL_ENTRY));
	AndesSendCmdMsg(pAd, msg);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("msg is sent\n"));
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_MU_GET_CLUSTER_TBL_ENTRY result = {0};
	UINT32 cmd = MU_GET_CLUSTER_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetMuEnableProc
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 * @descriton: enable/1 or disable/0 the MU module
 */
/* iwpriv ra0 set_mu_enable [1/0] */
INT SetMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	BOOLEAN value = 0;
	UINT32 cmd = MU_SET_ENABLE;
	CMD_MU_SET_ENABLE param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, NULL, 10);
	param.fgIsEnable = value;
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MU_SET_ENABLE));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &value, sizeof(value));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	EVENT_SHOW_MU_ENABLE result = {0};
	UINT32 cmd = MU_GET_ENABLE;
	struct _CMD_ATTRIBUTE attr = {0};
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(EVENT_SHOW_MU_ENABLE));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	PCHAR pch = NULL;
	PCHAR pMinVal = NULL;
	PCHAR pMaxVal = NULL;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_GROUP_USER_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_GROUP_USER_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pMinVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pMaxVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.min = os_str_tol(pMinVal, 0, 10);
	param.max = os_str_tol(pMaxVal, 0, 10);

	if (param.min < 2 || param.max > 4) {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("Min < 2 and Max > 4 is NOT allowed\n"));
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Values %d %d\n", param.min, param.max));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_GROUP_USER_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_GROUP_USER_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetCalculateInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_CALC_INIT_MCS;
	EVENT_STATUS result = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = cpu2le32(index);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &index, sizeof(index));
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetGroupNssProc
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	PCHAR pch = NULL;
	PCHAR pMinVal = NULL;
	PCHAR pMaxVal = NULL;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_GROUP_NSS_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_GROUP_NSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pMinVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pMaxVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.min = os_str_tol(pMinVal, 0, 10);
	param.max = os_str_tol(pMaxVal, 0, 10);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Values %d %d\n", param.min, param.max));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_GROUP_NSS_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_GROUP_NSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetTxReqMinTime
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 */
INT SetTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_TXREQ_MIN_TIME param = {0};
	UINT32 cmd = MU_SET_TXREQ_MIN_TIME;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 16);
	param.value = cpu2le16(value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Values %d\n", param.value));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_TXREQ_MIN_TIME result = {0};
	UINT32 cmd = MU_GET_TXREQ_MIN_TIME;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Nitesh: In the function ShowCalcInitMCSProc
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *it shows the Init MCS value for the corresponding Group Index
 */
INT ShowCalcInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_GROUP_TBL_ENTRY result = {0};
	UINT32 cmd = MU_GET_CALC_INIT_MCS;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function MuSetSuNssCheck
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *it checl the nss of the primary ac owner if su nss= mu nss
 */
INT SetSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_SU_NSS_CHECK;
	CMD_SET_NSS_CHECK param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);

	if (value < 0 || value > 1)
		return FALSE;

	param.fgIsEnable = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function MuSetSuNssCheck
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT ShowSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_GET_SU_NSS_CHECK;
	EVENT_SHOW_NSS_CHECK result = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function send GID management frame
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *it triggers the GID management frame
 */
INT SetTriggerGIDMgmtFrameProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_GID_MGMT_FRAME;
	PCHAR pch = NULL;
	PCHAR pWlanIdx = NULL;
	PCHAR pGid = NULL;
	PCHAR pUp = NULL;
	CMD_SET_TRIGGER_GID_MGMT_FRAME param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pWlanIdx = pch;
	else {
		Ret = FALSE;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pGid = pch;
	else {
		Ret = FALSE;
		goto error;
	}

	pch = strsep(&arg, "");

	if  (pch != NULL)
		pUp = pch;
	else {
		Ret = FALSE;
		goto error;
	}

	param.wlanIndex = os_str_tol(pWlanIdx, 0, 10);
#ifdef RT_BIG_ENDIAN
	param.wlanIndex = cpu2le16(param.wlanIndex);
#endif
	param.gid = os_str_tol(pGid, 0, 10);
	param.up = os_str_tol(pUp, 0, 10);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(value));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetTriggerSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_SND;
	CMD_SET_TRIGGER_SND param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetTriggerBbpProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_BBP;
	CMD_SET_TRIGGER_BBP param = {0};
	UINT16 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);

	if (value > 2048) {
		Ret = FALSE;
		goto error;
	}

	param.u2GroupIndex = cpu2le16(value); /* 0~3: 1 to 4 user grouping */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetTriggerGroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_GROUP;
	CMD_SET_TRIGGER_GROUP param = {0};
	UINT8 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);

	if (value < 2) {
		Ret = FALSE;
		goto error;
	}

	param.ucNum = (value - 1); /* 0~3: 1 to 4 user grouping */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetTriggerDegroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_DEGROUP;
	CMD_SET_TRIGGER_DEGROUP param = {0};
	UINT8 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.ucMuProfileIndex = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT SetTriggerMuTxProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_MU_TX;
	UINT32 index = 0;
	CMD_SET_TRIGGER_MU_TX_FRAME param = {0};
	PCHAR pch = NULL;
	PCHAR pAcIndex = NULL;
	PCHAR pNumOfStas = NULL;
	PCHAR pRound = NULL;
	PCHAR pRandom = NULL;
	PCHAR pWlanIndex = NULL;
	PCHAR pPayloadLength = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pAcIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pRandom = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNumOfStas = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if  (pch != NULL)
		pRound = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.ucAcIndex = os_str_tol(pAcIndex, 0, 10);
	param.fgIsRandomPattern = os_str_tol(pRandom, 0, 10);
	param.u4NumOfSTAs = os_str_tol(pNumOfStas, 0, 10);
	param.u4Round = os_str_tol(pRound, 0, 10);

	for (index = 0; index < param.u4NumOfSTAs; index++) {
		pch = strsep(&arg, "_");

		if (pch != NULL)
			pWlanIndex = pch;
		else {
			Ret = 0;
			goto error;
		}

		param.aucWlanIndexArray[index] = os_str_tol(pWlanIndex, 0, 10);
	}

	for (index = 0; index < param.u4NumOfSTAs; index++) {
		if (index != (param.u4NumOfSTAs - 1)) {
			pch = strsep(&arg, "_");

			if (pch != NULL)
				pPayloadLength = pch;
			else {
				Ret = 0;
				goto error;
			}
		} else {
			pch = strsep(&arg, "");

			if (pch != NULL)
				pPayloadLength = pch;
			else {
				Ret = 0;
				goto error;
			}
		}

		param.au4PayloadLength[index] = os_str_tol(pPayloadLength, 0, 10);
		param.au4PayloadLength[index] = cpu2le32(param.au4PayloadLength[index]);
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(param));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &param);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	param.u4NumOfSTAs = cpu2le32(param.u4NumOfSTAs);
	param.u4Round = cpu2le32(param.u4Round);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetTxopDefault
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_TXOP_DEFAULT param = {0};
	UINT32 cmd = MU_SET_TXOP_DEFAULT;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le32(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_GET_TXOP_DEFAULT result = {0};
	UINT32 cmd = MU_GET_TXOP_DEFAULT;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetSuLossThreshold
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_SU_LOSS_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_SU_LOSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le16(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_SU_LOSS_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_SU_LOSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetMuGainThreshold
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_GAIN_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_MU_GAIN_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le16(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_MU_GAIN_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_MU_GAIN_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetSecondaryAcPolicy
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_SECONDARY_AC_POLICY param = {0};
	UINT32 cmd = MU_SET_SECONDARY_AC_POLICY;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le16(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_SECONDARY_AC_POLICY result = {0};
	UINT32 cmd = MU_GET_SECONDARY_AC_POLICY;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetGroupTblDmcsMask
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_GROUP_TBL_DMCS_MASK param = {0};
	UINT32 cmd = MU_SET_GROUP_TBL_DMCS_MASK;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_GROUP_TBL_DMCS_MASK result = {0};
	UINT32 cmd = MU_GET_GROUP_TBL_DMCS_MASK;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/*
 * @Jeffrey: In the function SetMaxGroupSearchCnt
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_MAX_GROUP_SEARCH_CNT param = {0};
	UINT32 cmd = MU_SET_MAX_GROUP_SEARCH_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le32(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_MAX_GROUP_SEARCH_CNT result = {0};
	UINT32 cmd = MU_GET_MAX_GROUP_SEARCH_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
INT ShowMuProfileTxStsCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT16 index = 0;
	CMD_GET_MU_PFID_TXS_CNT param = {0};
	EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT result = {0};
	UINT32 cmd = MU_GET_MU_PROFILE_TX_STATUS_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	param.u2PfidIndex = cpu2le16(index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static VOID ShowGroupTblEntryCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT8 ucI = 0;
	P_EVENT_SHOW_GROUP_TBL_ENTRY pGentry =
		(P_EVENT_SHOW_GROUP_TBL_ENTRY)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Resp Payload: "));

	for (ucI = 0; ucI < rsp_payload_len; ucI++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %x", rsp_payload[ucI]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("group table index %d\n", pGentry->index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Number of User  %d\n", pGentry->numUser));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("BW %d\n", pGentry->BW));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("NS0 %d\n", pGentry->NS0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("NS1 %d\n", pGentry->NS1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("NS2 %d\n", pGentry->NS2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("NS3 %d\n", pGentry->NS3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("PFIDUser0  %d\n", pGentry->PFIDUser0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("PFIDUser1  %d\n", pGentry->PFIDUser1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("PFIDUser2  %d\n", pGentry->PFIDUser2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("PFIDUser3  %d\n", pGentry->PFIDUser3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("SGI  %d\n", pGentry->fgIsShortGI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("USED  %d\n", pGentry->fgIsUsed));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("DISABLED  %d\n", pGentry->fgIsDisable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("initMCS0  %d\n", pGentry->initMcsUser0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("initMCS1  %d\n", pGentry->initMcsUser1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("initMCS2  %d\n", pGentry->initMcsUser2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("initMCS3  %d\n", pGentry->initMcsUser3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("dMCS0  %d\n", pGentry->dMcsUser0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("dMCS1  %d\n", pGentry->dMcsUser1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("dMCS2  %d\n", pGentry->dMcsUser2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("dMCS3  %d\n", pGentry->dMcsUser3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
/* | Valid |BA | WLAN INDEX| */
/* usage iwpriv ra0 set_mu_profile [valid] [index] [bamask] [wlanIdex] */
static VOID ShowMuProfileEntryCallback(char *rsp_payload,
									   UINT16 rsp_payload_len)
{
	UINT8 ucI = 0;
	P_EVENT_SHOW_MUPROFILE_ENTRY pMentry =
		(P_EVENT_SHOW_MUPROFILE_ENTRY)rsp_payload;
	/* TODO: replaced printk to Driver logger */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("mu table index %d\n", pMentry->index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Valid  %d\n", pMentry->valid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("BAMask %x\n", pMentry->baMask));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("wlanIdx %d\n", pMentry->wlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("rsp payload len %d\n", rsp_payload_len));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Resp Payload: "));

	for (ucI = 0; ucI < rsp_payload_len; ucI++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %x", rsp_payload[ucI]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
/* usage iwpriv ra0 set_mu_profile [valid] [index] [bamask] [wlanIdex] */
static VOID ShowMuEnableCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT8 ucI = 0;
	P_EVENT_SHOW_MU_ENABLE pMentry = (P_EVENT_SHOW_MU_ENABLE)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("MU enable:%x", pMentry->fgIsEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Resp Payload: "));

	for (ucI = 0; ucI < rsp_payload_len; ucI++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %x", rsp_payload[ucI]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowGroupUserThresholdCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_GROUP_USER_THRESHOLD pGentry =
		(P_EVENT_MU_GET_GROUP_USER_THRESHOLD)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Group user Threshold minimum value: %x\n", pGentry->min));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Group user Threshold maximum value: %x\n", pGentry->max));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowSecondaryAcPolicyCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_SECONDARY_AC_POLICY pGentry =
		(P_EVENT_MU_GET_SECONDARY_AC_POLICY)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("secondary ac policy value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowGroupTblDmcsMaskCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_GROUP_TBL_DMCS_MASK pGentry =
		(P_EVENT_MU_GET_GROUP_TBL_DMCS_MASK)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("group table dmcs mask value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowMaxGroupSearchCntCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_MAX_GROUP_SEARCH_CNT pGentry =
		(P_EVENT_MU_GET_MAX_GROUP_SEARCH_CNT)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("max. group search cnt value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowTxReqMinTimeCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_TXREQ_MIN_TIME pGentry =
		(P_EVENT_MU_GET_TXREQ_MIN_TIME)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TxReqMinTime value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowMuProfileTxStsCntCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT pGentry =
		(P_EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT) rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("mu profile index: %x\n", pGentry->pfIndex));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,2) used: %x\n", pGentry->cn2used));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,2) rate down: %x\n", pGentry->cn2rateDown));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,2) delta mcs: %x\n", pGentry->cn2deltaMcs));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,2) tx fail count: %x\n", pGentry->cn2TxFailCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,2) tx succes count: %x\n", pGentry->cn2TxSuccessCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,3) used: %x\n", pGentry->cn3used));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,3) rate down: %x\n", pGentry->cn3rateDown));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,3) delta mcs: %x\n", pGentry->cn3deltaMcs));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,3) tx fail count: %x\n", pGentry->cn3TxFailCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,3) tx succes count: %x\n", pGentry->cn3TxSuccessCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,4) used: %x\n", pGentry->cn4used));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,4) rate down: %x\n", pGentry->cn4rateDown));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,4) delta mcs: %x\n", pGentry->cn4deltaMcs));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,4) tx fail count: %x\n", pGentry->cn4TxFailCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("c(n,4) tx succes count: %x\n", pGentry->cn4TxSuccessCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowSuNssCheckCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_NSS_CHECK ptr = (P_EVENT_SHOW_NSS_CHECK)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("SU NSS Check value: %x\n", ptr->fgIsEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowGroupNssThresholdCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_GROUP_NSS_THRESHOLD pGentry =
		(P_EVENT_MU_GET_GROUP_NSS_THRESHOLD)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Group Nss Threshold minimum value: %x\n", pGentry->min));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Group Nss Threshold maximum value: %x\n", pGentry->max));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowClusterTblEntryCallback(char *rsp_payload,
										UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_CLUSTER_TBL_ENTRY pGentry =
		(P_EVENT_MU_GET_CLUSTER_TBL_ENTRY)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Cluster index %d\n", pGentry->index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("GID User Membership Status [0] = %x\n",
			  pGentry->gidUserMemberStatus[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("GID User Membership Status [1] = %x\n",
			  pGentry->gidUserMemberStatus[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("GID User Position [0] = %x\n", pGentry->gidUserPosition[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("GID User Position [1] = %x\n", pGentry->gidUserPosition[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("GID User Position [2] = %x\n", pGentry->gidUserPosition[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("GID User Position [3] = %x\n", pGentry->gidUserPosition[3]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowTxopDefaultCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_GET_TXOP_DEFAULT pGentry = (P_EVENT_GET_TXOP_DEFAULT)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TXOP default value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowSuLossThresholdCallback(char *rsp_payload,
										UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_SU_LOSS_THRESHOLD pGentry =
		(P_EVENT_MU_GET_SU_LOSS_THRESHOLD)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("su loss threshold value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowMuGainThresholdCallback(char *rsp_payload,
										UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_MU_GAIN_THRESHOLD pGentry =
		(P_EVENT_MU_GET_MU_GAIN_THRESHOLD)rsp_payload;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("mu gain threshold value: %x\n", pGentry->value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowStatusOfCommand(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_STATUS pGentry = (P_EVENT_STATUS)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("status: %x\n", pGentry->status));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
static VOID ShowStatusOfHqaCommand(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_STATUS pGentry = (P_EVENT_HQA_STATUS)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("status: %x\n", pGentry->status));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("END"));
}
bool
hqa_wifi_test_mu_cal_init_mcs(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_CALC_INIT_MCS pParams
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_CALC_INIT_MCS;
	CMD_HQA_SET_INIT_MCS param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	param.num_of_user  = pParams->num_of_user;
	param.bandwidth    = pParams->bandwidth;
	param.nss_of_user0 = pParams->nss_of_user0;
	param.nss_of_user1 = pParams->nss_of_user1;
	param.nss_of_user2 = pParams->nss_of_user2;
	param.nss_of_user3 = pParams->nss_of_user3;
	param.pf_mu_id_of_user0 = pParams->pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pParams->pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pParams->pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pParams->pf_mu_id_of_user3;
	param.spe_index = pParams->spe_index;
	param.num_of_txer = pParams->num_of_txer;
	param.group_index = cpu2le32(pParams->group_index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("spe_index %d\n", param.spe_index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Group Index Values %d\n", pParams->group_index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Txer Values %d\n", pParams->num_of_txer));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static VOID hqa_wifi_test_mu_get_init_mcs_callback(struct cmd_msg *msg,
		char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_INIT_MCS pEntry = (P_EVENT_HQA_INIT_MCS)rsp_payload;

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! event fill null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! buffer not specific by cmd\n", __func__));
	} else {
		P_MU_STRUCT_MU_GROUP_INIT_MCS pOutput =
			(P_MU_STRUCT_MU_GROUP_INIT_MCS)msg->attr.rsp.wb_buf_in_calbk;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: EVENT_HQA_INIT_MCS\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("eventId %u\n", pEntry->u4EventId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("user0InitMCS %u\n", cpu2le32(pEntry->rEntry.user0InitMCS)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("user1InitMCS %u\n", cpu2le32(pEntry->rEntry.user1InitMCS)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("user2InitMCS %u\n", cpu2le32(pEntry->rEntry.user2InitMCS)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("user3InitMCS %u\n", cpu2le32(pEntry->rEntry.user3InitMCS)));
		pOutput->user0InitMCS = cpu2le32(pEntry->rEntry.user0InitMCS);
		pOutput->user1InitMCS = cpu2le32(pEntry->rEntry.user1InitMCS);
		pOutput->user2InitMCS = cpu2le32(pEntry->rEntry.user2InitMCS);
		pOutput->user3InitMCS = cpu2le32(pEntry->rEntry.user3InitMCS);
	}
}
bool
hqa_wifi_test_mu_get_init_mcs(
	PRTMP_ADAPTER pAd,
	UINT32 groupIndex,
	P_MU_STRUCT_MU_GROUP_INIT_MCS poutput
)
{
	INT32 Ret = 0;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_HQA_CALC_GET_INIT_MCS_ENTRY param = {0};
	UINT32 cmd = MU_HQA_GET_CALC_INIT_MCS;
	struct _CMD_ATTRIBUTE attr = {0};

	if (poutput == NULL) {
		Ret = FALSE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Group Index Values %d\n", groupIndex));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("poutput = %p\n", poutput));
	param.groupIndex = cpu2le32(groupIndex);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: param.groupIndex %u\n", __func__, param.groupIndex));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_INIT_MCS));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, poutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_mu_cal_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_CALC_LQ pParams
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_CALC_LQ;
	CMD_HQA_SET_MU_CALC_LQ param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	param.num_of_user = pParams->num_of_user;
	param.bandwidth = pParams->bandwidth;
	param.nss_of_user0 = pParams->nss_of_user0;
	param.nss_of_user1 = pParams->nss_of_user1;
	param.nss_of_user2 = pParams->nss_of_user2;
	param.nss_of_user3 = pParams->nss_of_user3;
	param.pf_mu_id_of_user0 = pParams->pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pParams->pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pParams->pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pParams->pf_mu_id_of_user3;
	param.spe_index = pParams->spe_index;
	param.num_of_txer = pParams->num_of_txer;
	param.group_index = cpu2le32(pParams->group_index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: CMD_HQA_SET_MU_CALC_LQ\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("num_of_user = %u\n", param.num_of_user));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("bandwidth = %u\n", param.bandwidth));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("nss_of_user0 = %u\n", param.nss_of_user0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("nss_of_user1 = %u\n", param.nss_of_user1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("nss_of_user2 = %u\n", param.nss_of_user2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("nss_of_user3 = %u\n", param.nss_of_user3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("pf_mu_id_of_user0 = %u\n", param.pf_mu_id_of_user0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("pf_mu_id_of_user1 = %u\n", param.pf_mu_id_of_user1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("pf_mu_id_of_user2 = %u\n", param.pf_mu_id_of_user2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("pf_mu_id_of_user3 = %u\n", param.pf_mu_id_of_user3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("num_of_txer = %u\n", param.num_of_txer));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("group_index = %u\n", param.group_index));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_su_cal_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_SU_CALC_LQ pParams
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_CALC_SU_LQ;
	CMD_HQA_SET_SU_CALC_LQ param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	param.num_of_user       = pParams->num_of_user;
	param.bandwidth         = pParams->bandwidth;
	param.nss_of_user0      = pParams->nss_of_user0;
	param.pf_mu_id_of_user0 = pParams->pf_mu_id_of_user0;
	param.num_of_txer       = pParams->num_of_txer;
	param.group_index       = cpu2le32(pParams->group_index);
	param.spe_index         = pParams->spe_index;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: CMD_HQA_SET_SU_CALC_LQ\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("num_of_user = %u\n", param.num_of_user));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("bandwidth = %u\n", param.bandwidth));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("nss_of_user0 = %u\n", param.nss_of_user0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("pf_mu_id_of_user0 = %u\n", param.pf_mu_id_of_user0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("num_of_txer = %u\n", param.num_of_txer));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("group_index = %u\n", param.group_index));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static VOID hqa_wifi_test_mu_get_su_lq_callback(struct cmd_msg *msg,
		char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_GET_SU_CALC_LQ pEntry = (P_EVENT_HQA_GET_SU_CALC_LQ)rsp_payload;

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! event fill null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! buffer not specific by cmd\n", __func__));
	} else {
		P_SU_STRUCT_LQ_REPORT pOutput =
			(P_SU_STRUCT_LQ_REPORT)msg->attr.rsp.wb_buf_in_calbk;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:EVENT_HQA_GET_SU_CALC_LQ\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("eventId %u\n", pEntry->u4EventId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0] = 0x%x\n", cpu2le32(pEntry->rEntry.lq_report[0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1] = 0x%x\n", cpu2le32(pEntry->rEntry.lq_report[1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2] = 0x%x\n", cpu2le32(pEntry->rEntry.lq_report[2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3] = 0x%x\n", cpu2le32(pEntry->rEntry.lq_report[3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[4] = 0x%x\n", cpu2le32(pEntry->rEntry.lq_report[4])));
		pOutput->lq_report[0] = cpu2le32(pEntry->rEntry.lq_report[0]);
		pOutput->lq_report[1] = cpu2le32(pEntry->rEntry.lq_report[1]);
		pOutput->lq_report[2] = cpu2le32(pEntry->rEntry.lq_report[2]);
		pOutput->lq_report[3] = cpu2le32(pEntry->rEntry.lq_report[3]);
		pOutput->lq_report[4] = cpu2le32(pEntry->rEntry.lq_report[4]);
	}
}
bool
hqa_wifi_test_su_get_lq(
	PRTMP_ADAPTER pAd,
	P_SU_STRUCT_LQ_REPORT pOutput
)
{
	INT32 Ret = 0;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_GET_CALC_SU_LQ;
	struct _CMD_ATTRIBUTE attr = {0};

	if (pOutput == NULL) {
		Ret = -1;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:intput pOutput = %p\n", __func__, pOutput));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_GET_SU_CALC_LQ));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pOutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static VOID hqa_wifi_test_mu_get_lq_callback(struct cmd_msg *msg,
		char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_GET_MU_CALC_LQ pEntry = (P_EVENT_HQA_GET_MU_CALC_LQ)rsp_payload;

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! event fill null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! buffer not specific by cmd\n", __func__));
	} else {
		P_MU_STRUCT_LQ_REPORT pOutput =
			(P_MU_STRUCT_LQ_REPORT)msg->attr.rsp.wb_buf_in_calbk;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:EVENT_HQA_GET_MU_CALC_LQ\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("eventId %u\n", pEntry->u4EventId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][0] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[0][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][1] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[0][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][2] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[0][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][3] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[0][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][4] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[0][4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][0] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[1][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][1] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[1][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][2] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[1][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][3] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[1][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][4] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[1][4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][0] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[2][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][1] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[2][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][2] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[2][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][3] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[2][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][4] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[2][4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][0] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[3][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][1] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[3][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][2] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[3][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][3] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[3][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][4] = 0x%x\n",
				  cpu2le32(pEntry->rEntry.lq_report[3][4])));
		pOutput->lq_report[0][0] = cpu2le32(pEntry->rEntry.lq_report[0][0]);
		pOutput->lq_report[0][1] = cpu2le32(pEntry->rEntry.lq_report[0][1]);
		pOutput->lq_report[0][2] = cpu2le32(pEntry->rEntry.lq_report[0][2]);
		pOutput->lq_report[0][3] = cpu2le32(pEntry->rEntry.lq_report[0][3]);
		pOutput->lq_report[0][4] = cpu2le32(pEntry->rEntry.lq_report[0][4]);
		pOutput->lq_report[1][0] = cpu2le32(pEntry->rEntry.lq_report[1][0]);
		pOutput->lq_report[1][1] = cpu2le32(pEntry->rEntry.lq_report[1][1]);
		pOutput->lq_report[1][2] = cpu2le32(pEntry->rEntry.lq_report[1][2]);
		pOutput->lq_report[1][3] = cpu2le32(pEntry->rEntry.lq_report[1][3]);
		pOutput->lq_report[1][4] = cpu2le32(pEntry->rEntry.lq_report[1][4]);
		pOutput->lq_report[2][0] = cpu2le32(pEntry->rEntry.lq_report[2][0]);
		pOutput->lq_report[2][1] = cpu2le32(pEntry->rEntry.lq_report[2][1]);
		pOutput->lq_report[2][2] = cpu2le32(pEntry->rEntry.lq_report[2][2]);
		pOutput->lq_report[2][3] = cpu2le32(pEntry->rEntry.lq_report[2][3]);
		pOutput->lq_report[2][4] = cpu2le32(pEntry->rEntry.lq_report[2][4]);
		pOutput->lq_report[3][0] = cpu2le32(pEntry->rEntry.lq_report[3][0]);
		pOutput->lq_report[3][1] = cpu2le32(pEntry->rEntry.lq_report[3][1]);
		pOutput->lq_report[3][2] = cpu2le32(pEntry->rEntry.lq_report[3][2]);
		pOutput->lq_report[3][3] = cpu2le32(pEntry->rEntry.lq_report[3][3]);
		pOutput->lq_report[3][4] = cpu2le32(pEntry->rEntry.lq_report[3][4]);
	}
}
bool
hqa_wifi_test_mu_get_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_LQ_REPORT pOutput
)
{
	INT32 Ret = 0;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_GET_CALC_LQ;
	struct _CMD_ATTRIBUTE attr = {0};

	if (pOutput == NULL) {
		Ret = -1;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:intput pOutput = %p\n", __func__, pOutput));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_GET_MU_CALC_LQ));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pOutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_snr_offset_set(
	PRTMP_ADAPTER pAd,
	char val
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_SNR_OFFSET;
	CMD_HQA_SET_MU_SNR_OFFSET param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	param.offset = val;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s, offset:0x%x\n", __func__, val));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_mu_set_zero_nss(
	PRTMP_ADAPTER pAd,
	unsigned char val
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_SET_MU_NSS_ZERO param = {0};
	UINT32 cmd = MU_HQA_SET_ZERO_NSS;
	struct _CMD_ATTRIBUTE attr = {0};

	param.ucValue = val;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s, val:%u\n", __func__,  val));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_mu_speed_up_lq(
	PRTMP_ADAPTER pAd,
	int val
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_SET_MU_LQ_SPEED_UP param = {0};
	UINT32 cmd = MU_HQA_SET_SPEED_UP_LQ;
	struct _CMD_ATTRIBUTE attr = {0};

	param.ucValue = val;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s, val:%d\n", __func__, val));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_mu_table_set(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_TABLE ptr
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = 0;
	P_CMD_HQA_SET_MU_METRIC_TABLE pMuParam = NULL;
	P_CMD_HQA_SET_SU_METRIC_TABLE pSuParam = NULL;
	UINT32 i = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	if (ptr == NULL) {
		Ret = -1;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: type = %u, length = %u\n",
			  __func__, ptr->type, ptr->length));

	cmd = (ptr->type == SU) ? MU_HQA_SET_SU_TABLE : MU_HQA_SET_MU_TABLE;

	if (ptr->type == SU) {
		os_alloc_mem(pAd, (UCHAR **)&pSuParam, sizeof(*pSuParam));
		NdisZeroMemory(pSuParam, sizeof(*pSuParam));
	} else if (ptr->type == MU) {
		os_alloc_mem(pAd, (UCHAR **)&pMuParam, sizeof(*pMuParam));
		NdisZeroMemory(pMuParam, sizeof(*pMuParam));
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("error type!\n"));
		Ret = -1;
		goto error;
	}

	if (ptr->type == SU) {
		if (ptr->length <= sizeof(pSuParam->metric_table))
			NdisCopyMemory(pSuParam->metric_table, ptr->prTable, ptr->length);
		else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("length(%u) error!!should < su_metric_tbl(%zu)\n",
					  ptr->length, sizeof(pSuParam->metric_table)));
			Ret = -1;
			goto error;
		}
	} else {
		if (ptr->length <= sizeof(pMuParam->metric_table))
			NdisCopyMemory(pMuParam->metric_table, ptr->prTable, ptr->length);
		else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("length(%u) error!!should < mu_metric_tbl(%zu)\n",
					  ptr->length, sizeof(pSuParam->metric_table)));
			Ret = -1;
			goto error;
		}
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s\n", ptr->type == SU ? "su_metric_table:":"mu_metric_table:"));

	if (ptr->type == MU) {
		for (i = 0; i < ptr->length; i += 8) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
					  pMuParam->metric_table[i + 0],
					  pMuParam->metric_table[i + 1],
					  pMuParam->metric_table[i + 2],
					  pMuParam->metric_table[i + 3],
					  pMuParam->metric_table[i + 4],
					  pMuParam->metric_table[i + 5],
					  pMuParam->metric_table[i + 6],
					  pMuParam->metric_table[i + 7]));
		}
	} else {
		for (i = 0; i < ptr->length; i += 3) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("0x%x 0x%x 0x%x\n",
					  pSuParam->metric_table[i + 0],
					  pSuParam->metric_table[i + 1],
					  pSuParam->metric_table[i + 2]));
		}
	}

	msg = AndesAllocCmdMsg(pAd, (sizeof(cmd) + ((ptr->type == SU) ?
								 (sizeof(*pSuParam)) : (sizeof(*pMuParam)))));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (ptr->type == SU) ? (char *)pSuParam : (char *)pMuParam,
					  (ptr->type == SU) ? (sizeof(*pSuParam)) : (sizeof(*pMuParam)));
	AndesSendCmdMsg(pAd, msg);
error:

	if (ptr && (ptr->type == SU)) {
		if (pSuParam)
			os_free_mem(pSuParam);
	} else {
		if (pMuParam)
			os_free_mem(pMuParam);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static void dump_set_mu_group_parms(P_MU_STRUCT_MU_GROUP mu_group)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: MU_STRUCT_MU_GROUP Content\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("groupIndex = %u\n", mu_group->groupIndex));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("numOfUser       = %u\n", mu_group->numOfUser));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("fgUser0Ldpc     = %u\n", mu_group->user0Ldpc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("fgUser1Ldpc     = %u\n", mu_group->user1Ldpc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("fgUser20Ldpc    = %u\n", mu_group->user2Ldpc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("fgUser3Ldpc     = %u\n", mu_group->user3Ldpc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("shortGI = %u\n", mu_group->shortGI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("bw = %u\n", mu_group->bw));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user0Nss = %u\n", mu_group->user0Nss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user1Nss = %u\n", mu_group->user1Nss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user2Nss = %u\n", mu_group->user2Nss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user3Nss = %u\n", mu_group->user3Nss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("groupId = %u\n", mu_group->groupId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user0UP = %u\n", mu_group->user0UP));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user1UP = %u\n", mu_group->user1UP));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user2UP = %u\n", mu_group->user2UP));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user3UP = %u\n", mu_group->user3UP));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user0MuPfId     = %u\n", mu_group->user0MuPfId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user1MuPfId     = %u\n", mu_group->user1MuPfId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user2MuPfId     = %u\n", mu_group->user2MuPfId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user3MuPfId     = %u\n", mu_group->user3MuPfId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user0InitMCS = %u\n", mu_group->user0InitMCS));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user1InitMCS    = %u\n", mu_group->user1InitMCS));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user2InitMCS    = %u\n", mu_group->user2InitMCS));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("user3InitMCS    = %u\n", mu_group->user3InitMCS));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("aucUser0MacAddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(mu_group->aucUser0MacAddr)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("aucUser1MacAddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(mu_group->aucUser1MacAddr)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("aucUser2MacAddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(mu_group->aucUser2MacAddr)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("aucUser3MacAddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(mu_group->aucUser3MacAddr)));
}
bool
hqa_wifi_test_mu_group_set(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_GROUP mu_group
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_GROUP;
	CMD_HQA_SET_MU_GROUP param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	param.groupIndex = cpu2le32(mu_group->groupIndex);
	param.numOfUser         = mu_group->numOfUser;
	param.fgUser0Ldpc       = mu_group->user0Ldpc;
	param.fgUser1Ldpc       = mu_group->user1Ldpc;
	param.fgUser2Ldpc       = mu_group->user2Ldpc;
	param.fgUser3Ldpc       = mu_group->user3Ldpc;
	param.bandwidth         = mu_group->bw;
	param.fgIsShortGI       = mu_group->shortGI;
	param.user0Nss          = mu_group->user0Nss;
	param.user1Nss          = mu_group->user1Nss;
	param.user2Nss          = mu_group->user2Nss;
	param.user3Nss          = mu_group->user3Nss;
	param.groupId           = mu_group->groupId;
	param.user0InitMCS      = mu_group->user0InitMCS;
	param.user1InitMCS      = mu_group->user1InitMCS;
	param.user2InitMCS      = mu_group->user2InitMCS;
	param.user3InitMCS      = mu_group->user3InitMCS;
	param.user0MuPfId       = mu_group->user0MuPfId;
	param.user1MuPfId       = mu_group->user1MuPfId;
	param.user2MuPfId       = mu_group->user2MuPfId;
	param.user3MuPfId       = mu_group->user3MuPfId;
	/* config_mu_vht_ldpc_cap_in_wtbl(pAd, mu_group); */
	dump_set_mu_group_parms(mu_group);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static VOID hqa_wifi_test_mu_get_qd_callback(struct cmd_msg *msg,
		char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_MU_QD pEntry = (P_EVENT_HQA_MU_QD)rsp_payload;

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! event fill null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: error !! buffer not specific by cmd\n", __func__));
	} else {
		P_MU_STRUCT_MU_QD pOutput =
			(P_MU_STRUCT_MU_QD)msg->attr.rsp.wb_buf_in_calbk;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:EVENT_HQA_MU_QD\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("eventId %u\n", pEntry->u4EventId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[0] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[1] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[2] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[3] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[4] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[5] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[5])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[6] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[6])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[7] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[7])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[8] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[8])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[9] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[9])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[10] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[10])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[11] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[11])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[12] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[12])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[13] = 0x%x\n", cpu2le32(pEntry->rEntry.qd_report[13])));
		pOutput->qd_report[0] = cpu2le32(pEntry->rEntry.qd_report[0]);
		pOutput->qd_report[1] = cpu2le32(pEntry->rEntry.qd_report[1]);
		pOutput->qd_report[2] = cpu2le32(pEntry->rEntry.qd_report[2]);
		pOutput->qd_report[3] = cpu2le32(pEntry->rEntry.qd_report[3]);
		pOutput->qd_report[4] = cpu2le32(pEntry->rEntry.qd_report[4]);
		pOutput->qd_report[5] = cpu2le32(pEntry->rEntry.qd_report[5]);
		pOutput->qd_report[6] = cpu2le32(pEntry->rEntry.qd_report[6]);
		pOutput->qd_report[7] = cpu2le32(pEntry->rEntry.qd_report[7]);
		pOutput->qd_report[8] = cpu2le32(pEntry->rEntry.qd_report[8]);
		pOutput->qd_report[9] = cpu2le32(pEntry->rEntry.qd_report[9]);
		pOutput->qd_report[10] = cpu2le32(pEntry->rEntry.qd_report[10]);
		pOutput->qd_report[11] = cpu2le32(pEntry->rEntry.qd_report[11]);
		pOutput->qd_report[12] = cpu2le32(pEntry->rEntry.qd_report[12]);
		pOutput->qd_report[13] = cpu2le32(pEntry->rEntry.qd_report[13]);
	}
}
bool
hqa_wifi_test_mu_get_qd(
	PRTMP_ADAPTER pAd,
	INT8 subcarrierIndex,
	P_MU_STRUCT_MU_QD pOutput
)
{
	INT32 Ret = 0;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_HQA_GET_QD param = {0};
	UINT32 cmd = MU_HQA_GET_QD;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!pOutput)
		return -1;

	param.scIdx = subcarrierIndex;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(param.scIdx = %d_\n", __func__, param.scIdx));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_MU_QD));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pOutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
bool
hqa_wifi_test_mu_set_enable(
	PRTMP_ADAPTER pAd,
	unsigned char val
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_SET_MU_ENABLE param = {0};
	UINT32 cmd = MU_HQA_SET_ENABLE;
	struct _CMD_ATTRIBUTE attr = {0};

	param.fgIsEnable = val;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(param.fgIsEnable = %d_\n",
			  __func__, param.fgIsEnable));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
/* New API for Mu TX */
bool
hqa_wifi_test_mu_trigger_mu_tx(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM pParam
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_SET_TRIGGER_MU_TX_FRAME param = {0};
	UINT32 cmd = MU_SET_TRIGGER_MU_TX;
	struct _CMD_ATTRIBUTE attr = {0};

	param.ucAcIndex = 1;
	param.fgIsRandomPattern = pParam->fgIsRandomPattern;
	param.u4NumOfSTAs   = cpu2le32(pParam->u4NumOfSTAs + 1);
	param.u4Round = cpu2le32(pParam->u4MuPacketCount);
	param.au4PayloadLength[0] = cpu2le32(pParam->msduPayloadLength0);
	param.au4PayloadLength[1] = cpu2le32(pParam->msduPayloadLength1);
	param.au4PayloadLength[2] = cpu2le32(pParam->msduPayloadLength2);
	param.au4PayloadLength[3] = cpu2le32(pParam->msduPayloadLength3);
	param.aucWlanIndexArray[0] = 1;
	param.aucWlanIndexArray[1] = 2;
	param.aucWlanIndexArray[2] = 3;
	param.aucWlanIndexArray[3] = 4;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:MD_SET_TRIGGER_MU_TX_FRAME\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ucAcIndex = %u\n", param.ucAcIndex));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("fgIsRandomPattern = %u\n", param.fgIsRandomPattern));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("u4NumOfSTAs = %u\n", param.u4NumOfSTAs));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("u4Round = %u\n",  param.u4Round));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("au4PayloadLength[0] = %u\n", param.au4PayloadLength[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("au4PayloadLength[1] = %u\n", param.au4PayloadLength[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("au4PayloadLength[2] = %u\n", param.au4PayloadLength[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("au4PayloadLength[3] = %u\n", param.au4PayloadLength[3]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("aucWlanIndexArray[0] = %u\n", param.aucWlanIndexArray[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("aucWlanIndexArray[1] = %u\n", param.aucWlanIndexArray[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("aucWlanIndexArray[2] = %u\n", param.aucWlanIndexArray[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("aucWlanIndexArray[3] = %u\n", param.aucWlanIndexArray[3]));
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}
static VOID eventDispatcher(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *)rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, len));
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

	switch (u4EventId) {
	case MU_EVENT_MU_ENABLE:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_MU_ENABLE\n", __func__));
		ShowMuEnableCallback(pData, len);
		break;

	case MU_EVENT_MUPROFILE_ENTRY:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_MUPROFILE_ENTRY\n", __func__));
		ShowMuProfileEntryCallback(pData, len);
		break;

	case MU_EVENT_GROUP_TBL_ENTRY:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_GROUP_TBL_ENTRY\n", __func__));
		ShowGroupTblEntryCallback(pData, len);
		break;

	case MU_EVENT_CALC_INIT_MCS:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_CALC_INIT_MCS\n", __func__));
		ShowGroupTblEntryCallback(pData, len);
		break;

	case MU_EVENT_GROUP_NSS_THRESHOLD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_GROUP_NSS_THRESHOLD\n", __func__));
		ShowGroupNssThresholdCallback(pData, len);
		break;

	case MU_EVENT_TXREQ_MIN_TIME:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_TXREQ_MIN_TIME\n", __func__));
		ShowTxReqMinTimeCallback(pData, len);
		break;

	case MU_EVENT_GROUP_USER_THRESHOLD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_GROUP_USER_THRESHOLD\n", __func__));
		ShowGroupUserThresholdCallback(pData, len);
		break;

	case MU_EVENT_CLUSTER_TBL_ENTRY:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_CLUSTER_TBL_ENTRY\n", __func__));
		ShowClusterTblEntryCallback(pData, len);
		break;

	case MU_EVENT_SU_NSS_CHECK:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_SU_NSS_CHECK\n", __func__));
		ShowSuNssCheckCallback(pData, len);
		break;

	case MU_EVENT_TXOP_DEFAULT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_TXOP_DEFAULT\n", __func__));
		ShowTxopDefaultCallback(pData, len);
		break;

	case MU_EVENT_SU_LOSS_THRESHOLD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_SU_LOSS_THRESHOLD\n", __func__));
		ShowSuLossThresholdCallback(pData, len);
		break;

	case MU_EVENT_MU_GAIN_THRESHOLD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_MU_GAIN_THRESHOLD\n", __func__));
		ShowMuGainThresholdCallback(pData, len);
		break;

	case MU_EVENT_SECONDARY_AC_POLICY:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_SECONDARY_AC_POLICY\n", __func__));
		ShowSecondaryAcPolicyCallback(pData, len);
		break;

	case MU_EVENT_GROUP_TBL_DMCS_MASK:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_GROUP_TBL_DMCS_MASK\n", __func__));
		ShowGroupTblDmcsMaskCallback(pData, len);
		break;

	case MU_EVENT_MAX_GROUP_SEARCH_CNT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_MAX_GROUP_SEARCH_CNT\n", __func__));
		ShowMaxGroupSearchCntCallback(pData, len);
		break;

	case MU_EVENT_MUPROFILE_TX_STS_CNT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_MUPROFILE_TX_STS_CNT\n", __func__));
		ShowMuProfileTxStsCntCallback(pData, len);
		break;

	case MU_EVENT_STATUS:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_STATUS\n", __func__));
		ShowStatusOfCommand(pData, len);
		break;

	case MU_EVENT_HQA_STATUS:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_HQA_STATUS\n", __func__));
		ShowStatusOfHqaCommand(pData, len);
		break;

	case MU_EVENT_HQA_GET_INIT_MCS:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_HQA_GET_INIT_MCS\n", __func__));
		hqa_wifi_test_mu_get_init_mcs_callback(msg, pData, len);
		break;

	case MU_EVENT_HQA_GET_QD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_HQA_GET_QD\n", __func__));
		hqa_wifi_test_mu_get_qd_callback(msg, pData, len);
		break;

	case MU_EVENT_HQA_GET_LQ:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_HQA_GET_LQ\n", __func__));
		hqa_wifi_test_mu_get_lq_callback(msg, pData, len);
		break;

	case MU_EVENT_HQA_GET_SU_LQ:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MU_EVENT_HQA_GET_SU_LQ\n", __func__));
		hqa_wifi_test_mu_get_su_lq_callback(msg, pData, len);
		break;

	default:
		break;
	}
}
/* MU UT API test code */
INT32 hqa_mu_get_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	UINT32 gid = 0;
	MU_STRUCT_MU_GROUP_INIT_MCS init_mcs;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	gid = os_str_toul(arg, 0, 10);
	os_zero_mem(&init_mcs, sizeof(init_mcs));
	Ret = hqa_wifi_test_mu_get_init_mcs(pAd, gid, &init_mcs);

	if (Ret == 0) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s, gid:%u\n", __func__, gid));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s, init_mcs.user1InitMCS:%u\n",
				  __func__, init_mcs.user0InitMCS));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s, init_mcs.user1InitMCS:%u\n",
				  __func__, init_mcs.user1InitMCS));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s, init_mcs.user1InitMCS:%u\n",
				  __func__, init_mcs.user2InitMCS));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s, init_mcs.user1InitMCS:%u\n",
				  __func__, init_mcs.user3InitMCS));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_cal_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	MU_STRUCT_SET_CALC_INIT_MCS param = {0};
	UINT32 num_of_user = 0;
	UINT32 bandwidth = 0;
	UINT32 nss_of_user0 = 0;
	UINT32 nss_of_user1 = 0;
	UINT32 nss_of_user2 = 0;
	UINT32 nss_of_user3 = 0;
	UINT32 pf_mu_id_of_user0 = 0;
	UINT32 pf_mu_id_of_user1 = 0;
	UINT32 pf_mu_id_of_user2 = 0;
	UINT32 pf_mu_id_of_user3 = 0;
	UINT32 num_of_txer = 0;
	UINT32 group_index = 0;
	PCHAR pnum_of_user = NULL;
	PCHAR pbandwidth = NULL;
	PCHAR pnss_of_user0 = NULL;
	PCHAR pnss_of_user1 = NULL;
	PCHAR pnss_of_user2 = NULL;
	PCHAR pnss_of_user3 = NULL;
	PCHAR ppf_mu_id_of_user0 = NULL;
	PCHAR ppf_mu_id_of_user1 = NULL;
	PCHAR ppf_mu_id_of_user2 = NULL;
	PCHAR ppf_mu_id_of_user3 = NULL;
	PCHAR pnum_of_txer = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	pnum_of_user = strsep(&arg, ":");
	pbandwidth = strsep(&arg, ":");
	pnss_of_user0 = strsep(&arg, ":");
	pnss_of_user1 = strsep(&arg, ":");
	pnss_of_user2 = strsep(&arg, ":");
	pnss_of_user3 = strsep(&arg, ":");
	ppf_mu_id_of_user0 = strsep(&arg, ":");
	ppf_mu_id_of_user1 = strsep(&arg, ":");
	ppf_mu_id_of_user2 = strsep(&arg, ":");
	ppf_mu_id_of_user3 = strsep(&arg, ":");
	pnum_of_txer = strsep(&arg, ":");

	if (pnum_of_user        == NULL ||
		pbandwidth          == NULL ||
		pnss_of_user0       == NULL ||
		pnss_of_user1       == NULL ||
		pnss_of_user2       == NULL ||
		pnss_of_user3       == NULL ||
		ppf_mu_id_of_user0  == NULL ||
		ppf_mu_id_of_user1  == NULL ||
		ppf_mu_id_of_user2  == NULL ||
		ppf_mu_id_of_user3  == NULL ||
		pnum_of_txer        == NULL ||
		arg                 == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Invalid parameters\n", __func__));
		return -1;
	}

	num_of_user = os_str_toul(pnum_of_user, 0, 10);
	bandwidth = os_str_toul(pbandwidth, 0, 10);
	nss_of_user0 = os_str_toul(pnss_of_user0, 0, 10);
	nss_of_user1 = os_str_toul(pnss_of_user1, 0, 10);
	nss_of_user2 = os_str_toul(pnss_of_user2, 0, 10);
	nss_of_user3 = os_str_toul(pnss_of_user3, 0, 10);
	pf_mu_id_of_user0 = os_str_toul(ppf_mu_id_of_user0, 0, 10);
	pf_mu_id_of_user1 = os_str_toul(ppf_mu_id_of_user1, 0, 10);
	pf_mu_id_of_user2 = os_str_toul(ppf_mu_id_of_user2, 0, 10);
	pf_mu_id_of_user3 = os_str_toul(ppf_mu_id_of_user3, 0, 10);
	num_of_txer = os_str_toul(pnum_of_txer, 0, 10);
	group_index = os_str_toul(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: gid = %u, num_txer = %u\n",
			  __func__, group_index, num_of_txer));
	os_zero_mem(&param, sizeof(param));
	param.num_of_user     = num_of_user;
	param.bandwidth     = bandwidth;
	param.nss_of_user0 = nss_of_user0;
	param.nss_of_user1 = nss_of_user1;
	param.nss_of_user2 = nss_of_user2;
	param.nss_of_user3 = nss_of_user3;
	param.pf_mu_id_of_user0 = pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pf_mu_id_of_user3;
	param.num_of_txer = num_of_txer;
	param.group_index    = group_index;
	Ret = hqa_wifi_test_mu_cal_init_mcs(pAd, &param);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	UINT32 num_of_user = 0;
	UINT32 bandwidth = 0;
	UINT32 nss_of_user0 = 0;
	UINT32 nss_of_user1 = 0;
	UINT32 nss_of_user2 = 0;
	UINT32 nss_of_user3 = 0;
	UINT32 pf_mu_id_of_user0 = 0;
	UINT32 pf_mu_id_of_user1 = 0;
	UINT32 pf_mu_id_of_user2 = 0;
	UINT32 pf_mu_id_of_user3 = 0;
	UINT32 num_of_txer = 0;
	UINT32 group_index = 0;
	PCHAR pnum_of_user = NULL;
	PCHAR pbandwidth = NULL;
	PCHAR pnss_of_user0 = NULL;
	PCHAR pnss_of_user1 = NULL;
	PCHAR pnss_of_user2 = NULL;
	PCHAR pnss_of_user3 = NULL;
	PCHAR ppf_mu_id_of_user0 = NULL;
	PCHAR ppf_mu_id_of_user1 = NULL;
	PCHAR ppf_mu_id_of_user2 = NULL;
	PCHAR ppf_mu_id_of_user3 = NULL;
	PCHAR pnum_of_txer = NULL;
	CMD_HQA_SET_MU_CALC_LQ param = {0};

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	pnum_of_user = strsep(&arg, ":");
	pbandwidth = strsep(&arg, ":");
	pnss_of_user0 = strsep(&arg, ":");
	pnss_of_user1 = strsep(&arg, ":");
	pnss_of_user2 = strsep(&arg, ":");
	pnss_of_user3 = strsep(&arg, ":");
	ppf_mu_id_of_user0 = strsep(&arg, ":");
	ppf_mu_id_of_user1 = strsep(&arg, ":");
	ppf_mu_id_of_user2 = strsep(&arg, ":");
	ppf_mu_id_of_user3 = strsep(&arg, ":");
	pnum_of_txer = strsep(&arg, ":");

	if (pnum_of_user        == NULL ||
		pbandwidth          == NULL ||
		pnss_of_user0       == NULL ||
		pnss_of_user1       == NULL ||
		pnss_of_user2       == NULL ||
		pnss_of_user3       == NULL ||
		ppf_mu_id_of_user0  == NULL ||
		ppf_mu_id_of_user1  == NULL ||
		ppf_mu_id_of_user2  == NULL ||
		ppf_mu_id_of_user3  == NULL ||
		pnum_of_txer        == NULL ||
		arg                 == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Invalid parameters\n", __func__));
		return -1;
	}

	num_of_user = os_str_toul(pnum_of_user, 0, 10);
	bandwidth = os_str_toul(pbandwidth, 0, 10);
	nss_of_user0 = os_str_toul(pnss_of_user0, 0, 10);
	nss_of_user1 = os_str_toul(pnss_of_user1, 0, 10);
	nss_of_user2 = os_str_toul(pnss_of_user2, 0, 10);
	nss_of_user3 = os_str_toul(pnss_of_user3, 0, 10);
	pf_mu_id_of_user0 = os_str_toul(ppf_mu_id_of_user0, 0, 10);
	pf_mu_id_of_user1 = os_str_toul(ppf_mu_id_of_user1, 0, 10);
	pf_mu_id_of_user2 = os_str_toul(ppf_mu_id_of_user2, 0, 10);
	pf_mu_id_of_user3 = os_str_toul(ppf_mu_id_of_user3, 0, 10);
	num_of_txer = os_str_toul(pnum_of_txer, 0, 10);
	group_index = os_str_toul(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: gid = %u, num_txer = %u\n",
			  __func__, group_index, num_of_txer));
	os_zero_mem(&param, sizeof(param));
	param.num_of_user  = num_of_user;
	param.bandwidth    = bandwidth;
	param.nss_of_user0 = nss_of_user0;
	param.nss_of_user1 = nss_of_user1;
	param.nss_of_user2 = nss_of_user2;
	param.nss_of_user3 = nss_of_user3;
	param.pf_mu_id_of_user0 = pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pf_mu_id_of_user3;
	param.num_of_txer = num_of_txer;
	param.group_index = group_index;
	Ret = hqa_wifi_test_mu_cal_lq(pAd, (P_MU_STRUCT_SET_CALC_LQ)&param);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	MU_STRUCT_LQ_REPORT lq;

	os_zero_mem(&lq, sizeof(lq));
	Ret = hqa_wifi_test_mu_get_lq(pAd, (P_MU_STRUCT_LQ_REPORT)&lq);

	if (Ret == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:MU_STRUCT_LQ_REPORT\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][0] = 0x%x\n", cpu2le32(lq.lq_report[0][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][1] = 0x%x\n", cpu2le32(lq.lq_report[0][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][2] = 0x%x\n", cpu2le32(lq.lq_report[0][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][3] = 0x%x\n", cpu2le32(lq.lq_report[0][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0][4] = 0x%x\n", cpu2le32(lq.lq_report[0][4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][0] = 0x%x\n", cpu2le32(lq.lq_report[1][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][1] = 0x%x\n", cpu2le32(lq.lq_report[1][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][2] = 0x%x\n", cpu2le32(lq.lq_report[1][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][3] = 0x%x\n", cpu2le32(lq.lq_report[1][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1][4] = 0x%x\n", cpu2le32(lq.lq_report[1][4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][0] = 0x%x\n", cpu2le32(lq.lq_report[2][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][1] = 0x%x\n", cpu2le32(lq.lq_report[2][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][2] = 0x%x\n", cpu2le32(lq.lq_report[2][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][3] = 0x%x\n", cpu2le32(lq.lq_report[2][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2][4] = 0x%x\n", cpu2le32(lq.lq_report[2][4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][0] = 0x%x\n", cpu2le32(lq.lq_report[3][0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][1] = 0x%x\n", cpu2le32(lq.lq_report[3][1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][2] = 0x%x\n", cpu2le32(lq.lq_report[3][2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][3] = 0x%x\n", cpu2le32(lq.lq_report[3][3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3][4] = 0x%x\n", cpu2le32(lq.lq_report[3][4])));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_su_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	UINT32 num_of_user = 0;
	UINT32 bandwidth = 0;
	UINT32 nss_of_user0 = 0;
	UINT32 pf_mu_id_of_user0 = 0;
	UINT32 num_of_txer = 0;
	UINT32 group_index = 0;
	PCHAR pnum_of_user = NULL;
	PCHAR pbandwidth = NULL;
	PCHAR pnss_of_user0 = NULL;
	PCHAR ppf_mu_id_of_user0 = NULL;
	PCHAR pnum_of_txer = NULL;
	CMD_HQA_SET_SU_CALC_LQ param = {0};

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	pnum_of_user = strsep(&arg, ":");
	pbandwidth = strsep(&arg, ":");
	pnss_of_user0 = strsep(&arg, ":");
	ppf_mu_id_of_user0 = strsep(&arg, ":");
	pnum_of_txer = strsep(&arg, ":");

	if (pnum_of_user        == NULL ||
		pbandwidth          == NULL ||
		pnss_of_user0       == NULL ||
		ppf_mu_id_of_user0  == NULL ||
		pnum_of_txer        == NULL ||
		arg                 == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Invalid parameters\n", __func__));
		return -1;
	}

	num_of_user = os_str_toul(pnum_of_user, 0, 10);
	bandwidth = os_str_toul(pbandwidth, 0, 10);
	nss_of_user0 = os_str_toul(pnss_of_user0, 0, 10);
	pf_mu_id_of_user0 = os_str_toul(ppf_mu_id_of_user0, 0, 10);
	num_of_txer = os_str_toul(pnum_of_txer, 0, 10);
	group_index = os_str_toul(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: gid = %u, num_txer = %u\n",
			  __func__, group_index, num_of_txer));
	os_zero_mem(&param, sizeof(param));
	param.num_of_user     = num_of_user;
	param.bandwidth     = bandwidth;
	param.nss_of_user0 = nss_of_user0;
	param.pf_mu_id_of_user0 = pf_mu_id_of_user0;
	param.num_of_txer = num_of_txer;
	param.group_index    = group_index;
	Ret = hqa_wifi_test_su_cal_lq(pAd, (P_MU_STRUCT_SET_SU_CALC_LQ)&param);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_su_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	SU_STRUCT_LQ_REPORT lq;

	os_zero_mem(&lq, sizeof(lq));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:\n", __func__));
	Ret = hqa_wifi_test_su_get_lq(pAd, (P_SU_STRUCT_LQ_REPORT)&lq);

	if (Ret == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:SU_STRUCT_LQ_REPORT\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[0] = 0x%x\n", cpu2le32(lq.lq_report[0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[1] = 0x%x\n", cpu2le32(lq.lq_report[1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[2] = 0x%x\n", cpu2le32(lq.lq_report[2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[3] = 0x%x\n", cpu2le32(lq.lq_report[3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lq_report[4] = 0x%x\n", cpu2le32(lq.lq_report[4])));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_snr_offset(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	INT32 offset = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	offset = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s, offset:0x%x\n", __func__, offset));
	Ret = hqa_wifi_test_snr_offset_set(pAd, offset);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_zero_nss(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	UINT32 zero_nss = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	zero_nss = os_str_toul(arg, 0, 10);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s, zero_nss:0x%x\n", __func__,  zero_nss));
	Ret = hqa_wifi_test_mu_set_zero_nss(pAd, (UCHAR)zero_nss);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_speedup_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	INT32 spdup_lq = 0;

	spdup_lq = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: spdup_lq:%x\n", __func__, spdup_lq));
	Ret = hqa_wifi_test_mu_speed_up_lq(pAd, spdup_lq);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
/* 1024:*/
INT32 hqa_mu_set_mu_table(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	MU_STRUCT_MU_TABLE info;
	UINT32 type = 0;
	UCHAR *type_ptr = NULL;
	UINT32 specific_metric_content = 0;
	UINT32 length = 0;
	UINT32 i = 0;
	struct _CMD_HQA_SET_MU_METRIC_TABLE mu_metric_table;
	struct _CMD_HQA_SET_SU_METRIC_TABLE su_metric_table;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Invalid parameters\n", __func__));
		return -1;
	}

	type_ptr = strsep(&arg, ":");
	if (type_ptr == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Invalid parameters\n", __func__));
		return -1;
	}

	type = os_str_toul(type_ptr, 0, 10);
	specific_metric_content = os_str_toul(arg, 0, 10);
	specific_metric_content = cpu2le32(specific_metric_content);

	if (type == MU) {
		length = sizeof(struct _CMD_HQA_SET_MU_METRIC_TABLE);
		NdisFillMemory(&mu_metric_table, length, specific_metric_content);
	} else if (type == SU) {
		length = sizeof(struct _CMD_HQA_SET_SU_METRIC_TABLE);
		NdisFillMemory(&su_metric_table, length, specific_metric_content);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Type error(%u)!!! neither MU nor SU\n",
				  __func__, type));
		return -1;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: type:%u, length:%u, data = %zu\n",
			  __func__, type, length, type == MU ?
			  sizeof(mu_metric_table) : sizeof(su_metric_table)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s\n", type == MU ? "mu_metric_table:":"su_metric_table:"));

	if (type == MU) {
		for (i = 0; i < length; i += 8) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
					  mu_metric_table.metric_table[i + 0],
					  mu_metric_table.metric_table[i + 1],
					  mu_metric_table.metric_table[i + 2],
					  mu_metric_table.metric_table[i + 3],
					  mu_metric_table.metric_table[i + 4],
					  mu_metric_table.metric_table[i + 5],
					  mu_metric_table.metric_table[i + 6],
					  mu_metric_table.metric_table[i + 7]));
		}
	} else {
		for (i = 0; i < length; i += 3) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("0x%x 0x%x 0x%x\n",
					  su_metric_table.metric_table[i + 0],
					  su_metric_table.metric_table[i + 1],
					  su_metric_table.metric_table[i + 2]));
		}
	}

	info.type = type;
	info.length = length;
	info.prTable = (type == MU) ? (char *)&mu_metric_table : (char *)&su_metric_table;
	Ret = hqa_wifi_test_mu_table_set(pAd, (P_MU_STRUCT_MU_TABLE)&info);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_group(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	MU_STRUCT_MU_GROUP mu_group;
	UINT8 aucUser0MacAddr[6] = {0x00, 0x0c, 0x40, 0x12, 0x34, 0x56};
	UINT8 aucUser1MacAddr[6] = {0x00, 0x0c, 0x40, 0x12, 0x34, 0x57};
	UINT8 aucUser2MacAddr[6] = {0x00, 0x0c, 0x40, 0x12, 0x34, 0x58};
	UINT8 aucUser3MacAddr[6] = {0x00, 0x0c, 0x40, 0x12, 0x34, 0x59};
	mu_group.groupIndex   = 0;
	mu_group.numOfUser    = 1;
	mu_group.user0Ldpc    = 1;
	mu_group.user1Ldpc    = 1;
	mu_group.user2Ldpc    = 1;
	mu_group.user3Ldpc    = 1;
	mu_group.shortGI      = 0;
	mu_group.bw = 0;
	mu_group.user0Nss     = 0;
	mu_group.user1Nss     = 0;
	mu_group.user2Nss     = 0;
	mu_group.user3Nss     = 0;
	mu_group.groupId        = 10;
	mu_group.user0UP        = 0;
	mu_group.user1UP        = 0;
	mu_group.user2UP        = 0;
	mu_group.user3UP        = 0;
	mu_group.user0MuPfId    = 0;
	mu_group.user1MuPfId    = 1;
	mu_group.user2MuPfId    = 2;
	mu_group.user3MuPfId    = 3;
	mu_group.user0InitMCS   = 0;
	mu_group.user1InitMCS   = 0;
	mu_group.user2InitMCS   = 0;
	mu_group.user3InitMCS   = 0;
	NdisMoveMemory(mu_group.aucUser0MacAddr, aucUser0MacAddr, MAC_ADDR_LEN);
	NdisMoveMemory(mu_group.aucUser1MacAddr, aucUser1MacAddr, MAC_ADDR_LEN);
	NdisMoveMemory(mu_group.aucUser2MacAddr, aucUser2MacAddr, MAC_ADDR_LEN);
	NdisMoveMemory(mu_group.aucUser3MacAddr, aucUser3MacAddr, MAC_ADDR_LEN);
	Ret = hqa_wifi_test_mu_group_set(pAd, (P_MU_STRUCT_MU_GROUP)&mu_group);
	/* error: */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_get_qd(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	INT32 subcarrier_idx = 0;
	MU_STRUCT_MU_QD qd;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: arg = %s\n", __func__, arg));
	subcarrier_idx = os_str_toul(arg, 0, 10);
	Ret = hqa_wifi_test_mu_get_qd(pAd, subcarrier_idx, (P_MU_STRUCT_MU_QD)&qd);

	if (Ret == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:MU_STRUCT_MU_QD\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[0] = 0x%x\n", cpu2le32(qd.qd_report[0])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[1] = 0x%x\n", cpu2le32(qd.qd_report[1])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[2] = 0x%x\n", cpu2le32(qd.qd_report[2])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[3] = 0x%x\n", cpu2le32(qd.qd_report[3])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[4] = 0x%x\n", cpu2le32(qd.qd_report[4])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[5] = 0x%x\n", cpu2le32(qd.qd_report[5])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[6] = 0x%x\n", cpu2le32(qd.qd_report[6])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[7] = 0x%x\n", cpu2le32(qd.qd_report[7])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[8] = 0x%x\n", cpu2le32(qd.qd_report[8])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[9] = 0x%x\n", cpu2le32(qd.qd_report[9])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[10] = 0x%x\n", cpu2le32(qd.qd_report[10])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[11] = 0x%x\n", cpu2le32(qd.qd_report[11])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[12] = 0x%x\n", cpu2le32(qd.qd_report[12])));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("qd_report[13] = 0x%x\n", cpu2le32(qd.qd_report[13])));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == TRUE ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	UINT32 is_enable = 0;

	is_enable = os_str_toul(arg, 0, 10);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s,MU is %s\n", __func__,
			  is_enable == 1 ? "Enable":"Disable"));
	Ret = hqa_wifi_test_mu_set_enable(pAd, is_enable);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__,
			  Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_gid_up(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	MU_STRUCT_MU_STA_PARAM param;
	UINT32 gid_0 = 0;
	UINT32 gid_1 = 0;
	UINT32 up_0  = 0;
	UINT32 up_1  = 0;
	UINT32 up_2  = 0;
	UINT32 up_3  = 0;
	PCHAR temp_ptr_use_to_check = NULL;
	PCHAR gid_0_ptr = NULL;
	PCHAR gid_1_ptr = NULL;
	PCHAR up_0_ptr  = NULL;
	PCHAR up_1_ptr  = NULL;
	PCHAR up_2_ptr  = NULL;
	PCHAR up_3_ptr  = NULL;

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		gid_0_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		gid_1_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		up_0_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		up_1_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		up_2_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = arg;

	if (temp_ptr_use_to_check != NULL)
		up_3_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	gid_0 = os_str_tol(gid_0_ptr, 0, 10);
	gid_1 = os_str_tol(gid_1_ptr, 0, 10);
	up_0 = os_str_tol(up_0_ptr, 0, 10);
	up_1 = os_str_tol(up_1_ptr, 0, 10);
	up_2 = os_str_tol(up_2_ptr, 0, 10);
	up_3 = os_str_tol(up_3_ptr, 0, 10);
	param.gid[0] = gid_0;
	param.gid[1] = gid_1;
	param.up[0] = up_0;
	param.up[1] = up_1;
	param.up[2] = up_2;
	param.up[3] = up_3;
	Ret = hqa_wifi_test_mu_set_sta_gid_and_up(pAd, &param);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
INT32 hqa_mu_set_trigger_mu_tx(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM param = {0};
	UINT8  fgIsRandomPattern        = 0;    /* is random pattern or not */
	UINT32 msduPayloadLength0       = 0;    /* payload length of the MSDU for user 0 */
	UINT32 msduPayloadLength1       = 0;    /* payload length of the MSDU for user 1 */
	UINT32 msduPayloadLength2       = 0;    /* payload length of the MSDU for user 2 */
	UINT32 msduPayloadLength3       = 0;    /* payload length of the MSDU for user 3 */
	UINT32 u4MuPacketCount          = 0;    /* MU TX count */
	UINT32 u4NumOfSTAs              = 0;    /* number of user in the MU TX */
	PCHAR temp_ptr_use_to_check     = NULL;
	PCHAR fgIsRandomPattern_ptr     = NULL;
	PCHAR msduPayloadLength0_ptr    = NULL;
	PCHAR msduPayloadLength1_ptr    = NULL;
	PCHAR msduPayloadLength2_ptr    = NULL;
	PCHAR msduPayloadLength3_ptr    = NULL;
	PCHAR u4MuPacketCount_ptr       = NULL;
	PCHAR u4NumOfSTAs_ptr           = NULL;

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		fgIsRandomPattern_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		msduPayloadLength0_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		msduPayloadLength1_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		msduPayloadLength2_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		msduPayloadLength3_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");

	if (temp_ptr_use_to_check != NULL)
		u4MuPacketCount_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	temp_ptr_use_to_check = arg;

	if (temp_ptr_use_to_check != NULL)
		u4NumOfSTAs_ptr = temp_ptr_use_to_check;
	else {
		Ret = -1;
		goto error;
	}

	fgIsRandomPattern   = os_str_tol(fgIsRandomPattern_ptr, 0, 10);
	msduPayloadLength0  = os_str_tol(msduPayloadLength0_ptr, 0, 10);
	msduPayloadLength1  = os_str_tol(msduPayloadLength1_ptr, 0, 10);
	msduPayloadLength2  = os_str_tol(msduPayloadLength2_ptr, 0, 10);
	msduPayloadLength3  = os_str_tol(msduPayloadLength3_ptr, 0, 10);
	u4MuPacketCount     = os_str_tol(u4MuPacketCount_ptr, 0, 10);
	u4NumOfSTAs         = os_str_tol(u4NumOfSTAs_ptr, 0, 10);
	param.fgIsRandomPattern     = fgIsRandomPattern;
	param.msduPayloadLength0    = msduPayloadLength0;
	param.msduPayloadLength1    = msduPayloadLength1;
	param.msduPayloadLength2    = msduPayloadLength2;
	param.msduPayloadLength3    = msduPayloadLength3;
	param.u4MuPacketCount       = u4MuPacketCount;
	param.u4NumOfSTAs           = u4NumOfSTAs;
	param.macAddrs[0][0]        = 0x11;
	param.macAddrs[0][1]        = 0x22;
	param.macAddrs[0][2]        = 0x33;
	param.macAddrs[0][3]        = 0x44;
	param.macAddrs[0][4]        = 0x55;
	param.macAddrs[0][5]        = 0x66;

	if (u4NumOfSTAs >= 1) {
		param.macAddrs[1][0]        = 0x11;
		param.macAddrs[1][1]        = 0x22;
		param.macAddrs[1][2]        = 0x33;
		param.macAddrs[1][3]        = 0x44;
		param.macAddrs[1][4]        = 0x55;
		param.macAddrs[1][5]        = 0x66;
	}

	if (u4NumOfSTAs >= 2) {
		param.macAddrs[2][0]        = 0x11;
		param.macAddrs[2][1]        = 0x22;
		param.macAddrs[2][2]        = 0x33;
		param.macAddrs[2][3]        = 0x44;
		param.macAddrs[2][4]        = 0x55;
		param.macAddrs[2][5]        = 0x66;
	}

	if (u4NumOfSTAs >= 2) {
		param.macAddrs[3][0]        = 0x11;
		param.macAddrs[3][1]        = 0x22;
		param.macAddrs[3][2]        = 0x33;
		param.macAddrs[3][3]        = 0x44;
		param.macAddrs[3][4]        = 0x55;
		param.macAddrs[3][5]        = 0x66;
	}

	Ret = hqa_wifi_test_mu_trigger_mu_tx(pAd, &param);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:CMD %s\n", __func__, Ret == 0 ? "Success":"Fail"));
	return Ret;
}
#endif /* CFG_SUPPORT_MU_MIMO */
