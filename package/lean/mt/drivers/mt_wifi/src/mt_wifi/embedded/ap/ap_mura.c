/*
 *  $Id$
 *  $DateTime$
 *  Pluto Kang
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

#ifdef CFG_SUPPORT_MU_MIMO_RA

static VOID MuraEventDispatcher(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len);

static VOID mura_algorithm_state_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len);

static VOID mura_algorithm_group_state_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len);

static VOID mura_algorithm_hwfb_state_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len);

/** @brief Monitor the MU-RGA PFID statistics callback.
 *
 *  Show PFID statistics.
 *
 *  @param msg: pointer to message
 *  @param rsp_payload: payload array
 *  @param rsp_payload_len: payload length
 *  @return Void.
 */
static VOID mura_algorithm_pfid_stat_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len
);

/*
	==========================================================================
	Description:
		Set Mura Sounding Period, TBD

	Parameters:
		Standard MU SND Paramter

	==========================================================================
 */
INT SetMuraPeriodicSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_PERIODIC_SND;
	CMD_SET_PERIODIC_SND param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


/*
	==========================================================================
	Description:
		Set MU_RGA initialization, for test

	Parameters:
		Standard MU SND Paramter

	==========================================================================
 */
INT SetMuraTestAlgorithmInit(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	PCHAR pIdx = NULL;
	UINT32 index = 0;

	pch = arg;

	if (pch != NULL)
		pIdx = pch;
	else
		return 0;

	index = os_str_tol(pIdx, 0, 10);

	/* 4  E3 RF /BBP  Script */
	/* TBD */

	/* 4 E3 MU-MIMO Script */
	/* TBD */

	/* 4 MU-MIMO Initialization */
	if (index == 0)
		SetMuraTestAlgorithmProc(pAd, "1");

	return TRUE;
}

/*
	==========================================================================
	Description:
		Set MU_RGA test flow, for test

	Parameters:
		Standard MU SND Paramter

	==========================================================================
 */
INT SetMuraTestAlgorithmProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT16 value = 0;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_TEST_ALGORITHM;
	CMD_SET_PERIODIC_SND param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	value = os_str_tol(arg, 0, 10);
	param.u2Reserved = cpu2le16(value);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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

/*
	==========================================================================
	Description:
		Get MU_RGA algorithm status, for Debug

	Parameters:
		Standard MU-RGA  Paramter

	==========================================================================
 */
INT GetMuraMonitorStateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* =============================================================================================== */
	UINT32 index = 0;
	UINT32 index2 = 0;
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_ALGORITHM_STAT;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		index = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	if (index == MURA_STATE) {
		EVENT_SHOW_ALGORITHM_STATE stat_result = {0};

		msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(stat_result));

		if (!msg) {
			Ret = 0;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(stat_result));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &stat_result);
		SET_CMD_ATTR_RSP_HANDLER(attr, MuraEventDispatcher);
		AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		cmd = cpu2le32(cmd);
#endif
		AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
		AndesSendCmdMsg(pAd, msg);
	} else if (index == MURA_GROUP_STAT) {
		EVENT_SHOW_ALGORITHM_GROUP_STATE grouop_stat_result = {0};

		cmd = MURA_ALGORITHM_GROUP_STAT;
		pch = strsep(&arg, "-");

		if (pch == NULL) {
			Ret = 0;
			goto error;
		} else {
			pch = strsep(&arg, "");

			if (pch != NULL) {
				index2 = os_str_tol(pch, 0, 10);

				if (index2 >= MAX_MURA_GRP) {
					Ret = 0;
					goto error;
				}

#ifdef RT_BIG_ENDIAN
				index2 = cpu2le32(index2);
#endif
			} else {
				Ret = 0;
				goto error;
			}
		}

		msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(grouop_stat_result));

		if (!msg) {
			Ret = 0;
			goto error;
		}
#ifdef RT_BIG_ENDIAN
		cmd = cpu2le32(cmd);
#endif
		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(grouop_stat_result));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &grouop_stat_result);
		SET_CMD_ATTR_RSP_HANDLER(attr, MuraEventDispatcher);
		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
		AndesAppendCmdMsg(msg, (char *)&index2, sizeof(index2));
		AndesSendCmdMsg(pAd, msg);
	} else if (index == MURA_HWFB_STAT) {
		EVENT_SHOW_ALGORITHM_HWFB_STATE hwfb_stat_result = {0};

		cmd = MURA_ALGORITHM_HWFB_STAT;
		msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(hwfb_stat_result));

		if (!msg) {
			Ret = 0;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(hwfb_stat_result));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &hwfb_stat_result);
		SET_CMD_ATTR_RSP_HANDLER(attr, MuraEventDispatcher);
		AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		cmd = cpu2le32(cmd);
#endif
		AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
		AndesSendCmdMsg(pAd, msg);
	} else
		goto error;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT GetMuraPFIDStatProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4Index = 0;
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_PFID_STAT;
	struct _CMD_ATTRIBUTE attr = {0};
	EVENT_SHOW_PFID_STAT stat_result = {0};

	pch = arg;
	if (pch != NULL) {
		u4Index = simple_strtol(pch, 0, 10);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Argument is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(stat_result) + sizeof(u4Index));
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Msg allocation failed\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(stat_result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &stat_result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MuraEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&u4Index, sizeof(u4Index));

	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}

/*
	==========================================================================
	Description:
		Set MU_RGA algorithm with Fixed Rate

	Parameters:
		Standard MU-RGA  Paramter

	==========================================================================
 */
INT SetMuraFixedRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_FIXED_RATE_ALGORITHM;
	UINT_8 param = 0;
	UINT8 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	/* 2: MU-RGA Stop, 1:MU-RGA Fixed Rate, 0:MU-RGA Auto Rate */
	param = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


/*
	==========================================================================
	Description:
		Set MU_RGA algorithm with Fixed Group Entry

	Parameters:
		Standard MU-RGA  Paramter

	==========================================================================
 */
INT SetMuraFixedGroupRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	UINT_8 ucNNS_MCS = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_FIXED_GROUP_RATE_ALGORITHM;
	CMD_MURGA_SET_GROUP_TBL_ENTRY param = {0};

	pch = strsep(&arg, "-");

	if (pch != NULL)
		param.numUser = os_str_tol(pch, 0, 10) - 1;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL)
		param.BW = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	if (param.numUser >= 1) {
		pch = strsep(&arg, "-");

		if (pch != NULL)
			param.WLIDUser0 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.NS0 = (ucNNS_MCS > 10);
			param.initMcsUser0 = (ucNNS_MCS > 10) ? (ucNNS_MCS - 10) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL)
			param.WLIDUser1 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		if (param.numUser == 1)
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.NS1 = (ucNNS_MCS > 10);
			param.initMcsUser1 = (ucNNS_MCS > 10) ? (ucNNS_MCS - 10) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}
	}

	if (param.numUser >= 2) {
		pch = strsep(&arg, "-");

		if (pch != NULL)
			param.WLIDUser2 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		if (param.numUser == 2)
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.NS2 = (ucNNS_MCS > 10);
			param.initMcsUser2 = (ucNNS_MCS > 10) ? (ucNNS_MCS - 10) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}
	}

	if (param.numUser >= 3) {
		pch = strsep(&arg, "-");

		if (pch != NULL)
			param.WLIDUser3 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.NS3 = (ucNNS_MCS > 10);
			param.initMcsUser3 = (ucNNS_MCS > 10) ? (ucNNS_MCS - 10) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

/*
==========================================================================
Description:
	Set MU_RGA algorithm with Fixed Group Entry

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraFixedSndParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_SOUNDING_PERIOD;
	CMD_SET_SND_PARAMS param = {0};

	pch = strsep(&arg, "-");

	if (pch != NULL) {
		param.ucWLIDUser = os_str_tol(pch, 0, 10);

		if (param.ucWLIDUser == 0)
			param.ucAllMuUser = 1;
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL)
		param.ucMaxSoundingPeriod = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL)
		param.ucMinSoundingPeriod = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		param.ucSoundingPeriodStep = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

/*
==========================================================================
Description:
	Set MU_RGA algorithm with Fixed Group Entry

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraPlatformTypeProc(RTMP_ADAPTER *pAd)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_PLATFORM_TYPE;
	CMD_SET_PLATFORM_TYPE param = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

#if defined(CONFIG_RALINK_MT7621)
	param.ucPlatformType = 1;  /*MT7621*/
#endif
#if defined(CONFIG_ARCH_MT7623)
	param.ucPlatformType = 2;  /*MT7623*/
#endif
#if defined(CONFIG_ARCH_MT7622)
	param.ucPlatformType = 2;  /*MT7622*/
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Detect On Off

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilityCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_CTRL;
	CMD_MURGA_SET_MOBILITY_TYPE param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	BOOLEAN fgMobilityDetectEn = 0;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			fgMobilityDetectEn = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): fgMobilityDetectEn: %d\n", __func__,
																fgMobilityDetectEn));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.fgMobilityType = fgMobilityDetectEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	iwpriv <interface> set mura_mobility_en=O\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	param1: Mobility Enable (0, 1)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Detect Interval Ctrl

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilityIntervalCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_INTERVAL_CTRL;
	CMD_MURGA_SET_MOBILITY_INTERVAL param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	UINT16 u2MobilityInteral = 0;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u2MobilityInteral = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): u2MobilityInteral: %d\n", __func__,
																u2MobilityInteral));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.u2MobilityInteral = u2MobilityInteral;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   iwpriv <interface> set mura_mobility_interval_ctrl=O\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param1: Mobility Computation Profile Time Interval (180, 360)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Detect SNR Ctrl

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilitySNRCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_SNR_CTRL;
	CMD_MURGA_SET_MOBILITY_SNR param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	UINT8 ucMobilitySNR = 0;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			ucMobilitySNR = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ucMobilitySNR: %d\n", __func__,
																ucMobilitySNR));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.ucMobilitySNR = ucMobilitySNR;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   iwpriv <interface> set mura_mobility_snr_ctrl=O\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param1: Mobility Computation SNR value (0~63)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Detect Threshold Ctrl

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilityThresholdCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_THRESHOLD_CTRL;
	CMD_MURGA_SET_MOBILITY_THRESHOLD param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	UINT8 ucWlanId = 0, ucMobilityThreshold = 0;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 6) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 3 symbol representation */
			ucWlanId = simple_strtol(cstr, 0, 10);
			break;
		case 1:
			/* 2 symbol representation */
			ucMobilityThreshold = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ucWlanId: %d, ucMobilityThreshold: %d\n", __func__,
																ucWlanId, ucMobilityThreshold));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.ucWlanId = ucWlanId;
	param.ucMobilityThreshold = ucMobilityThreshold;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	iwpriv <interface> set mura_mobility_threshold_ctrl=OOO:OO\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	param1: WlanId (0~127)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	param2: Mobility LQ Threshold (0~63)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Sounding counter info

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilitySndCountProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_SOUNDING_INTERVAL_COUNT;
	CMD_MURGA_GET_MOBILITY_SND_INTERVAL param = {0};

	PCHAR pbuffer = NULL;
	PCHAR pflag = NULL;
	UINT8 fgMobilitySndIntvalCnt = 0;

	pbuffer = arg;
	if (pbuffer != NULL) {
		pflag = pbuffer;
	} else {
		return 0;
	}

	fgMobilitySndIntvalCnt = simple_strtol(pflag, 0, 10);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	param.fgMobilitySndIntvalCnt = fgMobilitySndIntvalCnt;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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

	return Ret;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d_\n", __FUNCTION__, Ret));

	return Ret;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Mode Ctrl

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilityModeCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_MODE_CTRL;
	CMD_MURGA_SET_MOBILITY_MODE param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	BOOLEAN fgMULQPingPongEn = TRUE;
	BOOLEAN fgMULQTriggerCalEn = TRUE;
	BOOLEAN fgMobilityFlagForceEn = FALSE;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 5) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			fgMULQPingPongEn = simple_strtol(cstr, 0, 10);
			break;
		case 1:
			/* 1 symbol representation */
			fgMULQTriggerCalEn = simple_strtol(cstr, 0, 10);
			break;
		case 2:
			/* 1 symbol representation */
			fgMobilityFlagForceEn = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): fgMULQPingPongEn: %d, fgMULQTriggerCalEn: %d, fgMobilityFlagForceEn: %d\n", __func__,
																fgMULQPingPongEn, fgMULQTriggerCalEn, fgMobilityFlagForceEn));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.fgMULQPingPongEn = fgMULQPingPongEn;
	param.fgMULQTriggerCalEn = fgMULQTriggerCalEn;
	param.fgMobilityFlagForceEn = fgMobilityFlagForceEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   iwpriv <interface> set mura_mobility_mode_ctrl=O:O:O\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param1: Mobility PingPong Mechanism Enable (0, 1)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param2: Mobility Trigger Computation (0, 1)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param3: Mobility Flag Enable Forced Mode (0, 1)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Log Ctrl

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilityLogCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_LOG_CTRL;
	CMD_MURGA_SET_MOBILITY_LOG param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	BOOLEAN fgMobilityLogEn = TRUE;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 3 symbol representation */
			fgMobilityLogEn = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): fgMobilityLogEn: %d\n", __func__,
																fgMobilityLogEn));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.fgMobilityLogEn = fgMobilityLogEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   iwpriv <interface> set mura_mobility_log_ctrl=O\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param1: Mobility Log Enable (0, 1)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Mobility Test Ctrl

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraMobilityTestCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_MOBILITY_TEST_CTRL;
	CMD_MURGA_SET_MOBILITY_TEST param = {0};

	/* declair parsing function related parameter */
	UINT8 u1ParamIdx;
	CHAR *cstr;
	BOOLEAN fgMobilityTestEn = TRUE;

	/* sanity check for input parameter existence */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!!\n", __func__));
		goto error0;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wrong parameter format!!\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, cstr = rstrtok(arg, ":"); cstr; cstr = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 3 symbol representation */
			fgMobilityTestEn = simple_strtol(cstr, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			goto error0;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): fgMobilityTestEn: %d\n", __func__,
																fgMobilityTestEn));

	/* memory allocate for command message */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	/* sanity check for command message memory allocate */
	if (!msg)
		goto error1;

	/* command message contents config */
	param.fgMobilityTestEn = fgMobilityTestEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
	chip_cmd_tx(pAd, msg);
	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Expected format is as below:\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   iwpriv <interface> set mura_mobility_test_ctrl=O\n\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   param1: Mobility Test Force each MU User as Mobility Candidate (0, 1)\n"));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): command message memory allocated fail!!\n", __func__));
	return FALSE;
}

/*
==========================================================================
Description:
	Set MU_RGA Disable CN3, CN4 Group Entry

Parameters:
	Standard MU-RGA  Paramter

==========================================================================
 */
INT SetMuraDisableCN3CN4Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURA_DISABLE_CN3_CN4;
	CMD_SET_DISABLE_CN3_CN4 param = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	param.ucDisableCn3Cn4 = simple_strtol(arg, 0, 10);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO_RA);
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
				("%s:(Ret = %d_\n", __FUNCTION__, Ret));

	return Ret;

}

/*
	==========================================================================
	Description:
		MU_RGA Event Handler

	Parameters:
		Standard MU-RGA  Paramter

	==========================================================================
 */
static VOID MuraEventDispatcher(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *) rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, len));

	switch (u4EventId) {
	case MURA_EVENT_ALGORITHM_STAT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURA_EVENT_GET_ALGORITHM_STATE\n", __func__));
		mura_algorithm_state_callback(msg, pData, len);
		break;

	case MURA_EVENT_ALGORITHM_GROUP_STAT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURA_EVENT_GET_ALGORITHM_GROUP_STATE\n", __func__));
		mura_algorithm_group_state_callback(msg, pData, len);
		break;

	case MURA_EVENT_ALGORITHM_HWFB_STAT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURA_EVENT_GET_ALGORITHM_HWFB_STATE\n", __func__));
		mura_algorithm_hwfb_state_callback(msg, pData, len);
		break;

	case MURA_EVENT_PFID_STAT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURA_EVENT_PFID_STAT\n", __func__));
		mura_algorithm_pfid_stat_callback(msg, pData, len);
		break;

	default:
		break;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief [CMD] Monitor the MU-RGA State
*
* @param pucParam
*
* @return status
*/
/*----------------------------------------------------------------------------*/
static VOID mura_algorithm_state_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_ALGORITHM_STATE pEntry = NULL;
	UINT_8 ucPFIDList_Idx = 0, ucGroupEntryList_Idx = 0; /* , ucNumofGroupEntry = 0; */

	/* P_MURA_CN_ENTRY_INFO_T pGroupEntry; */
	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: error !! rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_SHOW_ALGORITHM_STATE) rsp_payload;

	/* if (msg->rsp_payload == NULL) */
	/* { */
	/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, */
	/* ("%s: error !! buffer not specific by cmd\n", __FUNCTION__)); */
	/* } */
	/* else */
	{
		/* Current MUser in the system */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (MURA_COLOR_RESET "\n Number of Totally MUser : %2d\n", pEntry->ucMaxMuarNum));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n Number of Sounding MUser : %2d\n", pEntry->ucSoundingNum));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n Number of Waiting MUser : %2d\n", pEntry->ucWaitingNum));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n Number of SU Current Space : %2d\n", pEntry->ucMaxSndingCap));
		/* Show MU-RGA Sounding Candidate List */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Sounding Candidate WLAN ID List :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->ucMURAWlanIdList[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Group Candidate List */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Group Entry Candidate List :"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			if (pEntry->ucMURAPfidList[ucPFIDList_Idx] == 0x1F)
				continue;

			for (ucGroupEntryList_Idx = 0; ucGroupEntryList_Idx < MU_2U_NUM; ucGroupEntryList_Idx++) {
				if ((ucGroupEntryList_Idx % 20) == 0)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n %2d: ", ucGroupEntryList_Idx));

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%1d, ", pEntry->au4PfidGroupTableMap[ucPFIDList_Idx][ucGroupEntryList_Idx]));
			}

			for (ucGroupEntryList_Idx = MU_2U_NUM; ucGroupEntryList_Idx < (MU_2U_NUM + MU_3U_NUM); ucGroupEntryList_Idx++) {
				if (((ucGroupEntryList_Idx - MU_2U_NUM) % 20) == 0)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n %2d: ", ucGroupEntryList_Idx));

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 (MURA_COLOR_RED "%1d" MURA_COLOR_RESET ", ", pEntry->au4PfidGroupTableMap[ucPFIDList_Idx][ucGroupEntryList_Idx]));
			}

			for (ucGroupEntryList_Idx = (MU_2U_NUM + MU_3U_NUM); ucGroupEntryList_Idx < MAX_MURA_GRP; ucGroupEntryList_Idx++) {
				if (((ucGroupEntryList_Idx - (MU_2U_NUM + MU_3U_NUM)) % 20) == 0)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n %2d: ", ucGroupEntryList_Idx));

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 (MURA_COLOR_GREEN "%1d" MURA_COLOR_RESET ", ", pEntry->au4PfidGroupTableMap[ucPFIDList_Idx][ucGroupEntryList_Idx]));
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		}

		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\n")); */
		/* Show MU-RGA Tx Success Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn2 Tx Succ Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->u4TxSuccCnt[ucPFIDList_Idx] = le2cpu32(pEntry->u4TxSuccCnt[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->u4TxSuccCnt[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx Fail Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn2 Tx Fail Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->u4TxFailCnt[ucPFIDList_Idx] = le2cpu32(pEntry->u4TxFailCnt[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->u4TxFailCnt[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx Success Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn3 Tx Succ Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
		pEntry->u4TxCn3SuccCnt[ucPFIDList_Idx] = le2cpu32(pEntry->u4TxCn3SuccCnt[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->u4TxCn3SuccCnt[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx Fail Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn3 Tx Fail Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->u4TxCn3FailCnt[ucPFIDList_Idx] = le2cpu32(pEntry->u4TxCn3FailCnt[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->u4TxCn3FailCnt[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx Success Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn4 Tx Succ Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->u4TxCn4SuccCnt[ucPFIDList_Idx] = le2cpu32(pEntry->u4TxCn4SuccCnt[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->u4TxCn4SuccCnt[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx Fail Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn4 Tx Fail Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->u4TxCn4FailCnt[ucPFIDList_Idx] = le2cpu32(pEntry->u4TxCn4FailCnt[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->u4TxCn4FailCnt[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx PER Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn2 Total Tx PER :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->ucTxPER[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx PER Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn3 Total Tx PER :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->ucTxCn3PER[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Tx PER Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Cn4 Total Tx PER :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->ucTxCn4PER[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Mobility */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Mobility :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->fgMobility[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU-RGA Delta MCS */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Delta MCS Rate :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->aucMURADeltaMCS[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU Succ Sounding Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA MU Succ Sounding Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->au2SuccSounding[ucPFIDList_Idx] = le2cpu16(pEntry->au2SuccSounding[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->au2SuccSounding[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU Fail Sounding Counter */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA MU Fail Sounding Counter :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
#ifdef RT_BIG_ENDIAN
			pEntry->au2FailSounding[ucPFIDList_Idx] = le2cpu16(pEntry->au2FailSounding[ucPFIDList_Idx]);
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->au2FailSounding[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		/* Show MU SND PER */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA MU Sounding Fail Rate :\n"));

		for (ucPFIDList_Idx = 0; ucPFIDList_Idx < MAX_MURA_NUM; ucPFIDList_Idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %2d,", pEntry->aucSoundingFailRate[ucPFIDList_Idx]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n\n"));
#ifdef RT_BIG_ENDIAN
		pEntry->u4CalculateSoundingEnd = le2cpu32(pEntry->u4CalculateSoundingEnd);
		pEntry->u4CalculateSoundingStart = le2cpu32(pEntry->u4CalculateSoundingStart);
		pEntry->u4CalculateGroupMcsRateEnd = le2cpu32(pEntry->u4CalculateGroupMcsRateEnd);
		pEntry->u4CalculateGroupMcsRateStart = le2cpu32(pEntry->u4CalculateGroupMcsRateStart);
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Sounding Period : %d ms\n",
				 pEntry->u4CalculateSoundingEnd - pEntry->u4CalculateSoundingStart));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Group Period : %d ms\n",
				 pEntry->u4CalculateGroupMcsRateEnd - pEntry->u4CalculateGroupMcsRateStart));
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief [CMD] Monitor the MU-RGA Group State
*
* @param pucParam
*
* @return status
*/
/*----------------------------------------------------------------------------*/
static VOID mura_algorithm_group_state_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_ALGORITHM_GROUP_STATE pEntry = NULL;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: error !! rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_SHOW_ALGORITHM_GROUP_STATE) rsp_payload;

	/* if (msg->rsp_payload == NULL) */
	/* { */
	/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, */
	/* ("%s: error !! buffer not specific by cmd\n", __FUNCTION__)); */
	/* } */
	/* else */
	{
		/* Show MU-RGA Group Entry Information */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Group Entry Information :"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n GPID MNUM\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %4d %4d\n",
				  (pEntry->rMuraGroupEntry).u4GroupID,
				  (pEntry->rMuraGroupEntry).ucUserNum));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" MU0_ID MU1_ID MU2_ID MU3_ID\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %6d %6d %6d %6d\n",
				  (pEntry->rMuraGroupEntry).u4U0PFID,
				  (pEntry->rMuraGroupEntry).u4U1PFID,
				  (pEntry->rMuraGroupEntry).u4U2PFID,
				  (pEntry->rMuraGroupEntry).u4U3PFID));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" RATE0 RATE1 RATE2 RATE3\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %5d %5d %5d %5d\n",
				  (pEntry->rMuraGroupEntry).u2U0Rate +
				  ((pEntry->rMuraGroupEntry).ucU0SSN * 10),
				  (pEntry->rMuraGroupEntry).u2U1Rate +
				  ((pEntry->rMuraGroupEntry).ucU1SSN * 10),
				  (pEntry->rMuraGroupEntry).u2U2Rate +
				  ((pEntry->rMuraGroupEntry).ucU2SSN * 10),
				  (pEntry->rMuraGroupEntry).u2U3Rate +
				  ((pEntry->rMuraGroupEntry).ucU3SSN * 10)));

		if ((pEntry->rMuraGroupEntry).u4GroupValid)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n No Valid Group Entry\n\n"));
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief [CMD] Monitor the MU-RGA Hardware fallback State
*
* @param pucParam
*
* @return status
*/
/*----------------------------------------------------------------------------*/
static VOID mura_algorithm_hwfb_state_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_ALGORITHM_HWFB_STATE pEntry = NULL;
	UINT_8 ucMCSRate = 0;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: error !! rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_SHOW_ALGORITHM_HWFB_STATE) rsp_payload;

	/* if (msg->rsp_payload == NULL) */
	/* { */
	/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, */
	/* ("%s: error !! buffer not specific by cmd\n", __FUNCTION__)); */
	/* } */
	/* else */
	{
		/* Show MU-RGA Group Entry Information */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MU-RGA Hardware fallback Information :"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n MCS DnEn DnThres DnValue UpEn UpThres UpValue 20BSamp 40BSamp 80BSamp\n"));

		for (ucMCSRate = 0; ucMCSRate < 10; ucMCSRate++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (" %3d %4d %7d %7d %4d %7d %7d %7d %7d %7d\n",
					  ucMCSRate,
					  pEntry->fgDownOneStep[ucMCSRate],
					  pEntry->ucDownThreshold[ucMCSRate],
					  pEntry->ucDownMCS[ucMCSRate],
					  pEntry->fgUpOneStep[ucMCSRate],
					  pEntry->ucUpThreshold[ucMCSRate],
					  pEntry->ucUpMCS[ucMCSRate],
					  pEntry->uc20BWSample[ucMCSRate] * pEntry->uc20BWSampleFactor,
					  pEntry->uc40BWSample[ucMCSRate] * pEntry->uc40BWSampleFactor,
					  pEntry->uc80BWSample[ucMCSRate] * pEntry->uc80BWSampleFactor
					 ));
		}
	}
}

static VOID mura_algorithm_pfid_stat_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_PFID_STAT pEntry = (P_EVENT_SHOW_PFID_STAT) rsp_payload;

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Event is NULL!!\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Value of PFID stat are :-\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("WlanIdx = %d\n", pEntry->u1WlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Mobility = %d\n", pEntry->fgMobility));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn2 SuccCnt = %u\n", pEntry->u4TxSuccCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn2 FailCnt = %u\n", pEntry->u4TxFailCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn3 SuccCnt = %u\n", pEntry->u4TxCn3SuccCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn3 FailCnt = %u\n", pEntry->u4TxCn3FailCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn4 SuccCnt = %u\n", pEntry->u4TxCn4SuccCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn4 FailCnt = %u\n", pEntry->u4TxCn4FailCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn2 TxPER = %u\n", pEntry->u1TxPER));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn3 TxPER = %u\n", pEntry->u1TxCn3PER));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Cn4 TxPER = %u\n", pEntry->u1TxCn4PER));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Sounding Period = %u\n", pEntry->u1SoundingPeriod));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Min Sounding Period = %u\n", pEntry->u1MinSoundingPeriod));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Max Sounding Period = %u\n", pEntry->u1MaxSoundingPeriod));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Sounding Period Step = %u\n", pEntry->u1SoundingPeriodStep));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Succ Sounding = %u\n", pEntry->u2SuccSounding));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Fail Sounding = %u\n", pEntry->u2FailSounding));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Sounding Fail Rate = %u\n", pEntry->u1SoundingFailRate));
}

#endif /* CFG_SUPPORT_MU_MIMO_RA */

