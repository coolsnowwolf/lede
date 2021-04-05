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


enum {
	/* debug commands */
	MU_SET_STA_PARAM = 50,
	MU_GET_STA_PARAM,
	/* HQA STA commands */
	MU_HQA_SET_STA_PARAM = 60,
	MU_HQA_GET_STA_PARAM,
};

enum {
	MU_EVENT_MU_STA_PARAM = 50,
	MU_EVENT_HQA_GET_MU_STA_PARAM = 60,
};
typedef struct _CMD_MU_SET_STA_PARAM {
	UINT32 gid[2];
	UINT32 up[4];
} CMD_MU_SET_STA_PARAM, *P_CMD_MU_SET_STA_PARAM;

typedef struct _CMD_HQA_SET_MU_STA_PARAM {
	MU_STRUCT_MU_STA_PARAM param;
} CMD_HQA_SET_MU_STA_PARAM, *P_CMD_HQA_SET_MU_STA_PARAM;

INT SetMuStaParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pLowGidUserMemberStatus = NULL;
	PCHAR pHighGidUserMemberStatus = NULL;
	PCHAR pLowGidUserPosition0 = NULL;
	PCHAR pLowGidUserPosition1 = NULL;
	PCHAR pHighGidUserPosition0 = NULL;
	PCHAR pHighGidUserPosition1 = NULL;
	PUINT8 pch = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	CMD_MU_SET_STA_PARAM param = { {0}, {0} };
	UINT32 cmd = MU_SET_STA_PARAM;
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

	/* To we need change edian? */
	param.gid[0] = os_str_tol(pLowGidUserMemberStatus, 0, 16);
	param.gid[1] = os_str_tol(pHighGidUserMemberStatus, 0, 16);
	param.up[0] = os_str_tol(pLowGidUserPosition0, 0, 16);
	param.up[1] = os_str_tol(pLowGidUserPosition1, 0, 16);
	param.up[2] = os_str_tol(pHighGidUserPosition0, 0, 16);
	param.up[3] = os_str_tol(pHighGidUserPosition1, 0, 16);
	param.gid[0] = cpu2le32(param.gid[0]);
	param.gid[1] = cpu2le32(param.gid[1]);
	param.up[0] = cpu2le32(param.up[0]);
	param.up[1] = cpu2le32(param.up[1]);
	param.up[2] = cpu2le32(param.up[2]);
	param.up[3] = cpu2le32(param.up[3]);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MU_SET_STA_PARAM));

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
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(CMD_MU_SET_STA_PARAM));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

bool
hqa_wifi_test_mu_set_sta_gid_and_up(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_STA_PARAM ptr
)
{
	INT Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_STA_PARAM;
	CMD_HQA_SET_MU_STA_PARAM param;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = -1;
		goto error;
	}

	NdisCopyMemory(&param.param, ptr, sizeof(MU_STRUCT_MU_STA_PARAM));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: MU_STRUCT_MU_STA_PARAM\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("gid[0] = %u\n", param.param.gid[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("gid[1] = %u\n", param.param.gid[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("up[0] = %u\n", param.param.up[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("up[1] = %u\n", param.param.up[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("up[2] = %u\n", param.param.up[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("up[3] = %u\n", param.param.up[3]));
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
	RTMPEndianChange((UCHAR *)&param.param, sizeof(MU_STRUCT_MU_STA_PARAM));
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

#endif

