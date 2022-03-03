/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mtusb_io.c
*/
#include	"rt_config.h"

#define MAX_VENDOR_REQ_RETRY_COUNT  10

/*
	========================================================================

	Routine Description: NIC initialization complete

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
void mtusb_cfg_read(RTMP_ADAPTER *ad, UINT32 *value)
{
	RTUSB_VendorRequest(ad, (USBD_TRANSFER_DIRECTION_IN |
            USBD_SHORT_TRANSFER_OK),
            DEVICE_VENDOR_REQUEST_IN,
            0x63, (0x50029018 & 0xffff0000) >> 16,
            (0x50029018 & 0x0000ffff), value, 4);
}

void mtusb_cfg_write(RTMP_ADAPTER *ad, UINT32 value)
{
	RTUSB_VendorRequest(
            ad,
            USBD_TRANSFER_DIRECTION_OUT,
            DEVICE_VENDOR_REQUEST_OUT,
            0x66,
            (0x50029018 & 0xffff0000) >> 16,
            (0x50029018 & 0x0000ffff),
            &value,
            4);
}


NTSTATUS RTUSBVenderReset(RTMP_ADAPTER *pAd)
{
	NTSTATUS Status;
	DBGPRINT_RAW(RT_DEBUG_ERROR, ("-->RTUSBVenderReset\n"));
	Status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x01,
		0x1,
		0,
		NULL,
		0);

	DBGPRINT_RAW(RT_DEBUG_ERROR, ("<--RTUSBVenderReset\n"));
	return Status;
}


static NTSTATUS mtusb_multiwrite(RTMP_ADAPTER *pAd,
							IN UINT32 Offset,
							IN UCHAR *buf,
							IN USHORT len,
							IN USHORT batchLen)
{
	NTSTATUS Status = STATUS_SUCCESS;
	UINT32 index = Offset, actLen = batchLen, leftLen = len;
	UCHAR *pSrc = buf;

	do
	{
		actLen = (actLen > batchLen ? batchLen : actLen);
		Status = RTUSB_VendorRequest(
			pAd,
			USBD_TRANSFER_DIRECTION_OUT,
			DEVICE_VENDOR_REQUEST_OUT,
			0x66,
			(index & 0xffff0000) >> 16,
			(index & 0x0000ffff),
			pSrc,
			actLen);

		if (Status != STATUS_SUCCESS) {
			DBGPRINT(RT_DEBUG_ERROR, ("VendrCmdMultiWrite_nBytes failed!\n"));
			break;
		}

		index += actLen;
		leftLen -= actLen;
		pSrc = pSrc + actLen;
	} while (leftLen > 0);

	return Status;
}


static NTSTATUS mtusb_multiread(RTMP_ADAPTER *pAd, UINT32 addr, UCHAR *buf, UINT32 len)
{
	NTSTATUS Status;
	
	Status =  RTUSB_VendorRequest(pAd, (USBD_TRANSFER_DIRECTION_IN |
										USBD_SHORT_TRANSFER_OK),
										DEVICE_VENDOR_REQUEST_IN,
										0x63, (addr & 0xffff0000) >> 16, 
										(addr & 0x0000ffff), buf, len);

	return Status;
}


NTSTATUS mtusb_read_reg(RTMP_ADAPTER *pAd, UINT32 addr, UINT32 *value)
{
	NTSTATUS Status = 0;
	UINT32 localVal;
	UINT32 global_addr = mt_physical_addr_map(addr);

#if defined(CONFIG_STA_SUPPORT) && defined(STA_LP_PHASE_1_SUPPORT)
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		if (pAd->CountDowntoPsm == 0)
		{
			pAd->CountDowntoPsm = 2;	/* 100 ms; stay awake 200ms at most, average will be 1xx ms */	
		}

		AsicForceWakeup(pAd, TRUE);
	}
#endif /* #ifdef CONFIG_STA_SUPPORT && STA_LP_PHASE_1_SUPPORT */

	Status = mtusb_multiread(pAd, global_addr, (PUCHAR)&localVal, 4);
	
	*value = le2cpu32(localVal);
	
	if (Status != 0)
		*value = 0xffffffff;

	return Status;
}


NTSTATUS mtusb_write_reg(RTMP_ADAPTER *pAd, UINT32 addr, UINT32 value)
{
	NTSTATUS Status;
	UINT32 global_addr = mt_physical_addr_map(addr);

#if defined(CONFIG_STA_SUPPORT) && defined(STA_LP_PHASE_1_SUPPORT)
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{	
		if (pAd->CountDowntoPsm == 0)
		{
			pAd->CountDowntoPsm = 2;	/* 100 ms; stay awake 200ms at most, average will be 1xx ms */	
		}

		AsicForceWakeup(pAd, TRUE);
	}
#endif /* #ifdef CONFIG_STA_SUPPORT && STA_LP_PHASE_1_SUPPORT */

	Status = mtusb_multiwrite(pAd, global_addr, (PUCHAR)&value, 4, 4); 

	return Status;
}

NTSTATUS RTUSBMultiWrite(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN UCHAR *pData,
	IN USHORT length,
	IN BOOLEAN bWriteHigh)
{
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS RTUSBMultiWrite_OneByte(RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *pData)
{
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS RTUSBMultiWrite_nBytes(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN UCHAR *buf,
	IN USHORT len,
	IN USHORT batchLen)
{
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS RTUSBWriteMACRegister(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN UINT32 Value,
	IN BOOLEAN bWriteHigh)
{
	mtusb_multiwrite(pAd, Offset, (PUCHAR)&Value, 4, 4);
	
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS ResetBulkOutHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	DBGPRINT(RT_DEBUG_TRACE, ("CMDTHREAD_RESET_BULK_OUT(ResetPipeid=0x%0x)===>\n", pAd->bulkResetPipeid));

	return NDIS_STATUS_SUCCESS;
}


/* All transfers must be aborted or cancelled before attempting to reset the pipe.*/
static NTSTATUS ResetBulkInHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN === >\n"));

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN <===\n"));
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetAsicWcidHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS DelAsicWcidHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));
        
	if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
		return NDIS_STATUS_FAILURE;
        
        AsicDelWcidTab(pAd, (UCHAR)SetAsicWcid.WCID);

        return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SetWcidSecInfoHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_SEC_INFO pInfo;

	pInfo = (PRT_ASIC_WCID_SEC_INFO)CMDQelmt->buffer;
	RTMPSetWcidSecurityInfo(pAd,
							 pInfo->BssIdx,
							 pInfo->KeyIdx,
							 pInfo->CipherAlg,
							 pInfo->Wcid,
							 pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetAsicWcidIVEIVHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_IVEIV_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_IVEIV_ENTRY)CMDQelmt->buffer;
	AsicUpdateWCIDIVEIV(pAd,
						  pInfo->Wcid,
						  pInfo->Iv,
						  pInfo->Eiv);

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetAsicWcidAttrHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_ATTR_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_ATTR_ENTRY)CMDQelmt->buffer;
	AsicUpdateWcidAttributeEntry(pAd,
								  pInfo->BssIdx,
								  pInfo->KeyIdx,
								  pInfo->CipherAlg,
								  pInfo->Wcid,
								  pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SETAsicSharedKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_SHARED_KEY pInfo;

	pInfo = (PRT_ASIC_SHARED_KEY)CMDQelmt->buffer;
	AsicAddSharedKeyEntry(pAd,
						       pInfo->BssIndex,
							pInfo->KeyIdx,
							&pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SetAsicPairwiseKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PAIRWISE_KEY pInfo;

	pInfo = (PRT_ASIC_PAIRWISE_KEY)CMDQelmt->buffer;
	AsicAddPairwiseKeyEntry(pAd,
							 pInfo->WCID,
							 &pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}



static NTSTATUS RemovePairwiseKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR Wcid = *((PUCHAR)(CMDQelmt->buffer));

	AsicRemovePairwiseKeyEntry(pAd, Wcid);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetClientMACEntryHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_SET_ASIC_WCID pInfo;

	pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;
	AsicUpdateRxWCIDTable(pAd, pInfo->WCID, pInfo->Addr);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS UpdateProtectHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PROTECT_INFO pAsicProtectInfo;

	pAsicProtectInfo = (PRT_ASIC_PROTECT_INFO)CMDQelmt->buffer;
	AsicUpdateProtect(pAd, pAsicProtectInfo->OperationMode, pAsicProtectInfo->SetMask,
							pAsicProtectInfo->bDisableBGProtect, pAsicProtectInfo->bNonGFExist);
	
	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS APUpdateCapabilityAndErpieHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	APUpdateCapabilityAndErpIe(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS _802_11_CounterMeasureHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MAC_TABLE_ENTRY *pEntry;

		pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
		HandleCounterMeasure(pAd, pEntry);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */




#ifdef CONFIG_AP_SUPPORT
static NTSTATUS APEnableTXBurstHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_ENABLE_TX_BURST  \n"));

		AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x20);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APDisableTXBurstHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_DISABLE_TX_BURST  \n"));

		AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x00);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APAdjustEXPAckTimeHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME  \n"));
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APRecoverEXPAckTimeHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME  \n"));
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef LED_CONTROL_SUPPORT
static NTSTATUS SetLEDStatusHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR LEDStatus = *((PUCHAR)(CMDQelmt->buffer));

	RTMPSetLEDStatus(pAd, LEDStatus);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: CMDTHREAD_SET_LED_STATUS (LEDStatus = %d)\n",
								__FUNCTION__, LEDStatus));

	return NDIS_STATUS_SUCCESS;
}
#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10*/
static NTSTATUS LEDWPSMode10Hdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT WPSLedMode10 = *((PUINT)(CMDQelmt->buffer));

	DBGPRINT(RT_DEBUG_INFO, ("WPS LED mode 10::ON or Flash or OFF : %x\n", WPSLedMode10));

	switch(WPSLedMode10)
	{
		case LINK_STATUS_WPS_MODE10_TURN_ON:
			RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_ON);
			break;
		case LINK_STATUS_WPS_MODE10_FLASH:
			RTMPSetLEDStatus(pAd,LED_WPS_MODE10_FLASH);
			break;
		case LINK_STATUS_WPS_MODE10_TURN_OFF:
			RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_OFF);
			break;
		default:
			DBGPRINT(RT_DEBUG_INFO, ("WPS LED mode 10:: No this status %d!!!\n", WPSLedMode10));
			break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS ChannelRescanHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	DBGPRINT(RT_DEBUG_TRACE, ("cmd> Re-scan channel! \n"));

	pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, TRUE);
#ifdef DOT11_N_SUPPORT
	/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
	N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("cmd> Switch to %d! \n", pAd->CommonCfg.Channel));
	APStop(pAd);
	APStartUp(pAd);

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadAlarmResume(pAd);
#endif /* AP_QLOAD_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT*/


#ifdef LINUX
#endif /* LINUX */



#ifdef STREAM_MODE_SUPPORT
static NTSTATUS UpdateTXChainAddress(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	AsicUpdateTxChainAddress(pAd, CMDQelmt->buffer);
	return NDIS_STATUS_SUCCESS;
}
#endif /* STREAM_MODE_SUPPORT */


#ifdef MT_MAC
static NTSTATUS AddRemoveKeyHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	PMT_ASIC_SEC_INFO pInfo;

	pInfo = (PMT_ASIC_SEC_INFO)CMDQelmt->buffer;
							
	CmdProcAddRemoveKey(pAd, pInfo->AddRemove, pInfo->BssIdx, pInfo->KeyIdx, pInfo->Wcid, pInfo->KeyTabFlag, &pInfo->CipherKey, pInfo->Addr);
	return NDIS_STATUS_SUCCESS;
}
#endif /* MT_MAC */

typedef NTSTATUS (*CMDHdlr)(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt);

CMDHdlr CMDHdlrTable[] = {
	ResetBulkOutHdlr,				/* CMDTHREAD_RESET_BULK_OUT*/
	ResetBulkInHdlr,					/* CMDTHREAD_RESET_BULK_IN*/
	NULL, //CheckGPIOHdlr,					/* CMDTHREAD_CHECK_GPIO	*/
	SetAsicWcidHdlr,					/* CMDTHREAD_SET_ASIC_WCID*/
	DelAsicWcidHdlr,					/* CMDTHREAD_DEL_ASIC_WCID*/
	SetClientMACEntryHdlr,			/* CMDTHREAD_SET_CLIENT_MAC_ENTRY*/

	NULL,
	NULL,
	NULL,
	NULL,

#ifdef CONFIG_AP_SUPPORT
	APUpdateCapabilityAndErpieHdlr,	/* CMDTHREAD_AP_UPDATE_CAPABILITY_AND_ERPIE*/
	APEnableTXBurstHdlr,			/* CMDTHREAD_AP_ENABLE_TX_BURST*/
	APDisableTXBurstHdlr,			/* CMDTHREAD_AP_DISABLE_TX_BURST*/
	APAdjustEXPAckTimeHdlr,		/* CMDTHREAD_AP_ADJUST_EXP_ACK_TIME*/
	APRecoverEXPAckTimeHdlr,		/* CMDTHREAD_AP_RECOVER_EXP_ACK_TIME*/
	ChannelRescanHdlr,				/* CMDTHREAD_CHAN_RESCAN*/
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* CONFIG_AP_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	SetLEDStatusHdlr,			/* CMDTHREAD_SET_LED_STATUS*/
#else
    NULL,
#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	LEDWPSMode10Hdlr,				/* CMDTHREAD_LED_WPS_MODE10*/
#else
	NULL,
#endif /* WSC_LED_SUPPORT */

#else
	NULL,
#endif /* WSC_INCLUDED */

	/* Security related */
	SetWcidSecInfoHdlr,				/* CMDTHREAD_SET_WCID_SEC_INFO*/
	SetAsicWcidIVEIVHdlr,			/* CMDTHREAD_SET_ASIC_WCID_IVEIV*/
	SetAsicWcidAttrHdlr,				/* CMDTHREAD_SET_ASIC_WCID_ATTR*/
	SETAsicSharedKeyHdlr,			/* CMDTHREAD_SET_ASIC_SHARED_KEY*/
	SetAsicPairwiseKeyHdlr,			/* CMDTHREAD_SET_ASIC_PAIRWISE_KEY*/
	RemovePairwiseKeyHdlr,			/* CMDTHREAD_REMOVE_PAIRWISE_KEY*/

	NULL,

#ifdef CONFIG_AP_SUPPORT
	_802_11_CounterMeasureHdlr,	/* CMDTHREAD_802_11_COUNTER_MEASURE*/
#else
	NULL,
#endif /* CONFIG_AP_SUPPORT */

	UpdateProtectHdlr,				/* CMDTHREAD_UPDATE_PROTECT*/


#ifdef LINUX
	NULL,
	NULL,
	NULL,
	NULL,

#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* LINUX */

	NULL,

	NULL,

#ifdef STREAM_MODE_SUPPORT
	UpdateTXChainAddress, /* CMDTHREAD_UPDATE_TX_CHAIN_ADDRESS */
#else
	NULL,
#endif

	NULL,
#ifdef MT_MAC
	AddRemoveKeyHdlr, /* CMDTHREAD_ADDREMOVE_ASIC_KEY */
#else
	NULL,
#endif /* MT_MAC */	
};


