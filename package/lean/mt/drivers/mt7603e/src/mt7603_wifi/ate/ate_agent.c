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
	ate_agent.c
*/

#include "rt_config.h"

/*  CCK Mode */
static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1};

/*  OFDM Mode */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1};

/*  HT Mixed Mode */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 32, -1};

static CHAR TemplateFrame[32] = {0x08, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
					 		0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56, 0x00,
					 		0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x00, 0x00,
					 		0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


INT32 SetTxStop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetRxStop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


VOID ATE_QA_Statistics(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI, RXINFO_STRUC *pRxInfo, PHEADER_802_11 pHeader)
{

}

#ifdef DBG
INT32 SetEERead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetEEWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetBBPRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetBBPWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetRFWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}
#endif /* DBG */


VOID EEReadAll(PRTMP_ADAPTER pAd, UINT16 *Data)
{
	UINT16 Offset = 0;
	UINT16 Value;

	for (Offset = 0; Offset < (EEPROM_SIZE >> 1);)
	{
		RT28xx_EEPROM_READ16(pAd, (Offset << 1), Value);
		Data[Offset] = Value;
		Offset++;
	}
}


INT32 SetATEDa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	//ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Octet;
	RTMP_STRING *Value;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Da = %s\n", __FUNCTION__, Arg));

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */

	if (strlen(Arg) != 17)
		return FALSE;

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_AP_SUPPORT
		AtoH(Value, &ATECtrl->Addr1[Octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("%s: (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__,
		ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3],
		ATECtrl->Addr1[4], ATECtrl->Addr1[5]));

#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}


INT32 SetATESa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	RTMP_STRING *Value;
	INT32 Octet;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Sa = %s\n", __FUNCTION__, Arg));

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(Arg) != 17)
		return FALSE;

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value+1))))
		{
			return FALSE;
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(Value, &ATECtrl->Addr3[Octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("%s: (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__,
		ATECtrl->Addr3[0], ATECtrl->Addr3[1], ATECtrl->Addr3[2], ATECtrl->Addr3[3],
		ATECtrl->Addr3[4], ATECtrl->Addr3[5]));
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}


INT32 SetATEBssid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	RTMP_STRING *Value;
	INT32 Octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(Arg) != 17)
		return FALSE;

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_AP_SUPPORT
		AtoH(Value, &ATECtrl->Addr2[Octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("%s: (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__,
		ATECtrl->Addr2[0], ATECtrl->Addr2[1], ATECtrl->Addr2[2], ATECtrl->Addr2[3],
		ATECtrl->Addr2[4], ATECtrl->Addr2[5]));
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}


INT32 SetATEInitChan(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetADCDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATETxPower0(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	CHAR Power;
	INT32 Ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Power0 = %s\n", __FUNCTION__, Arg));

	Power = simple_strtol(Arg, 0, 10);

    ATECtrl->TxPower0 = Power;

	Ret = ATEOp->SetTxPower0(pAd, Power);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxPower1(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	CHAR Power;
	INT32 Ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Power1 = %s\n", __FUNCTION__, Arg));

	Power = simple_strtol(Arg, 0, 10);

	ATECtrl->TxPower1 = Power;

	Ret = ATEOp->SetTxPower1(pAd, Power);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxPowerEvaluation(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATETxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{

	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	CHAR Ant;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Ant = %s\n", __FUNCTION__, Arg));

	Ant = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetTxAntenna(pAd, Ant);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATERxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	CHAR Ant;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Ant = %s\n", __FUNCTION__, Arg));

	Ant = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetRxAntenna(pAd, Ant);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 Default_Set_ATE_TX_FREQ_OFFSET_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATETxFreqOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UINT32 FreqOffset;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	DBGPRINT(RT_DEBUG_OFF, ("%s: FreqOffset = %s\n", __FUNCTION__, Arg));

	FreqOffset = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetTxFreqOffset(pAd, FreqOffset);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 Default_Set_ATE_TX_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATETxLength(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UINT32 TxLength;

	DBGPRINT(RT_DEBUG_OFF, ("%s: TxLength = %s\n", __FUNCTION__, Arg));

	TxLength = simple_strtol(Arg, 0, 10);

	if ((TxLength < 24) || (TxLength > (MAX_FRAME_SIZE - 34/* == 2312 */)))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Out of range (%d), \
						it should be in range of 24~%d.\n", __FUNCTION__, ATECtrl->TxLength,
						(MAX_FRAME_SIZE - 34/* == 2312 */)));
		return FALSE;
	}
	else
	{
		ATECtrl->TxLength = TxLength;

	}

	return TRUE;
}


INT32 SetATETxCount(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	DBGPRINT(RT_DEBUG_OFF, ("%s: TxCount = %s\n", __FUNCTION__, Arg));

	ATECtrl->TxCount = simple_strtol(Arg, 0, 10);

	if (ATECtrl->TxCount == 0)
	{
#ifdef RTMP_PCI_SUPPORT
		ATECtrl->TxCount = 0xFFFFFFFF;
#endif /* RTMP_MAC_PCI */
	}

	return TRUE;
}


INT32 CheckMCSValid(PRTMP_ADAPTER pAd, UCHAR PhyMode, UCHAR Mcs)
{
	int Index;
	PCHAR pRateTab = NULL;

	switch (PhyMode)
	{
		case MODE_CCK:
			pRateTab = CCKRateTable;
			break;
		case MODE_OFDM:
			pRateTab = OFDMRateTable;
			break;
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			pRateTab = HTMIXRateTable;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Unrecognizable Tx Mode %d\n", __FUNCTION__, PhyMode));
			return -1;
			break;
	}

	Index = 0;

	while (pRateTab[Index] != -1)
	{
		if (pRateTab[Index] == Mcs)
			return 0;
		Index++;
	}

	return -1;
}


INT32 SetATETxMcs(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Mcs, PhyMode = 0;
	INT32 Ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Mcs = %s\n", __FUNCTION__, Arg));

	PhyMode = ATECtrl->PhyMode;

	Mcs = simple_strtol(Arg, 0, 10);

	Ret = CheckMCSValid(pAd, PhyMode, Mcs);

	if (Ret != -1)
	{
		ATECtrl->Mcs = Mcs;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Out of range, refer to rate table.\n", __FUNCTION__));
		return FALSE;
	}

	return TRUE;
}


INT32 SetATETxStbc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Stbc;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Stbc = %s\n", __FUNCTION__, Arg));

	Stbc = simple_strtol(Arg, 0, 10);

	if (Stbc > 1)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Out of range (%d)\n", __FUNCTION__, Stbc));
		return FALSE;
	}
	else
	{
		ATECtrl->Stbc = Stbc;
	}

	return TRUE;
}


INT32 SetATETxMode(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR PhyMode;

	DBGPRINT(RT_DEBUG_OFF, ("%s: TxMode = %s\n", __FUNCTION__, Arg));

	PhyMode = simple_strtol(Arg, 0, 10);

	if (PhyMode > MODE_HTGREENFIELD)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::Out of range.\nIt should be in range of 0~3\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("0: CCK, 1: OFDM, 2: HT_MIX, 3: HT_GREEN_FIELD\n"));
		return FALSE;
	}
	else
	{
		ATECtrl->PhyMode = PhyMode;
	}

	return TRUE;
}


INT32 SetATETxGi(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Sgi;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Sgi = %s\n", __FUNCTION__, Arg));

	Sgi = simple_strtol(Arg, 0, 10);

	if (Sgi > 1)
	{
		DBGPRINT(RT_DEBUG_ERROR, (":%s: Out of range (%d)\n", __FUNCTION__, Sgi));
		return FALSE;
	}
	else
	{
		ATECtrl->Sgi = Sgi;
	}

	return TRUE;
}


INT32 SetATERxFer(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATETempSensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATEReadRF(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = 0;

	Ret = ShowAllRF(pAd);

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetATELoadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	BOOLEAN Ret = FALSE;
	RTMP_STRING *Src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD Srcf;
	INT32 Retval;
	UINT32 u4MaxBufSize = EEPROM_SIZE >> 1;
	USHORT WriteEEPROM[u4MaxBufSize];

	//USHORT WriteEEPROM[(EEPROM_SIZE >> 1)];
	INT32 FileLength = 0;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);
	RTMP_OS_FS_INFO	OsFSInfo;

	DBGPRINT(RT_DEBUG_OFF, ("===> %s (value=%d)\n\n", __FUNCTION__, Value));

	if (Value > 0)
	{
		/* zero the e2p buffer */
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);

		RtmpOSFSInfoChange(&OsFSInfo, TRUE);

		do
		{
			/* open the bin file */
			Srcf = RtmpOSFileOpen(Src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(Srcf))
			{
				DBGPRINT_ERR(("%s - Error opening file %s\n", __FUNCTION__, Src));
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(Srcf, (RTMP_STRING *)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				DBGPRINT_ERR(("%s : error file length (=%d) in e2p.bin\n",
					   __FUNCTION__, FileLength));
				break;
			}
			else
			{
				/* write the content of .bin file to EEPROM */
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)
                {
                    UINT16 Index=0;
                    UINT16 Value=0;
                    INT32 E2pSize = 512;/* == 0x200 for PCI interface */
                    UINT16 TempData=0;

                    for (Index = 0 ; Index < (E2pSize >> 1); Index++)
                    {
                        /* "value" is especially for some compilers... */
                        TempData = le2cpu16(WriteEEPROM[Index]);
                        Value = TempData;
                        RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
                    }
                }
#else

//				rt_ee_write_all(pAd, WriteEEPROM);
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
				Ret = TRUE;
			}
			break;
		} while(TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(Srcf))
		{
			;
		}
		else
		{
			Retval = RtmpOSFileClose(Srcf);

			if (Retval)
			{
				DBGPRINT_ERR(("--> Error %d closing %s\n", -Retval, Src));

			}
		}

		/* restore */
		RtmpOSFSInfoChange(&OsFSInfo, FALSE);
	}

    DBGPRINT(RT_DEBUG_OFF, ("<=== %s (Ret=%d)\n", __FUNCTION__, Ret));

    return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
INT32 SetATELoadE2pFromBuf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	BOOLEAN Ret = FALSE;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("===> %s (Value=%d)\n\n", __FUNCTION__, Value));

	if (Value > 0)
	{
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)
            {
                UINT16 Index = 0;
                UINT16 Value = 0;
                INT32 E2PSize = 512;/* == 0x200 for PCI interface */
                UINT16 TempData = 0;

                for (Index = 0; Index < (E2PSize >> 1); Index++)
                {
                    /* "value" is especially for some compilers... */
		        	TempData = le2cpu16(pAd->EEPROMImage[Index]);
                    Value = TempData;
                    RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
                }
            }
#else

//		rt_ee_write_all(pAd, pAd->EEPROMImage);
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
		Ret = TRUE;

	}

    DBGPRINT(RT_DEBUG_OFF, ("<=== %s (Ret=%d)\n", __FUNCTION__, Ret));

    return Ret;

}
#endif /* RTMP_EFUSE_SUPPORT */


INT32 SetATEReadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT16 Buffer[EEPROM_SIZE >> 1];
	UINT16 *p;
	int i;

	EEReadAll(pAd, (UINT16 *)Buffer);

	p = Buffer;

	for (i = 0; i < (EEPROM_SIZE >> 1); i++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", *p));
		if (((i+1) % 16) == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		p++;
	}

	return TRUE;
}


INT32 SetATEAutoAlc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATEIpg(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATEPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_STRING *Value;

	Value = Arg;

	/* only one octet acceptable */
	if (strlen(Value) != 2)
		return FALSE;

	AtoH(Value, &(ATECtrl->Payload), 1);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", ATECtrl->Payload));

	return TRUE;
}


INT32 SetATEFixedPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_STRING *Value;

	Value = Arg;

	Value = (RTMP_STRING *)simple_strtol(Arg, 0, 10);

	if (Value == 0)
		ATECtrl->bFixedPayload = FALSE;
	else
		ATECtrl->bFixedPayload = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: (Fixed Payload  = %d)\n", __FUNCTION__,
						ATECtrl->bFixedPayload));

	return TRUE;
}


INT32 SetATETtr(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATEShow(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_STRING *Mode_String = NULL;
	RTMP_STRING *TxMode_String = NULL;

	switch (ATECtrl->Mode)
	{
		case (fATE_IDLE):
			Mode_String = "ATESTART";
			break;
		case (fATE_EXIT):
			Mode_String = "ATESTOP";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE)):
			Mode_String = "TXCONT";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE)):
			Mode_String = "TXCARR";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE)):
			Mode_String = "TXCARS";
			break;
		case (fATE_TX_ENABLE):
			Mode_String = "TXFRAME";
			break;
		case (fATE_RX_ENABLE):
			Mode_String = "RXFRAME";
			break;
		default:
		{
			Mode_String = "Unknown ATE mode";
			DBGPRINT(RT_DEBUG_OFF, ("ERROR! Unknown ATE mode!\n"));
			break;
		}
	}
	DBGPRINT(RT_DEBUG_OFF, ("ATE Mode = %s\n", Mode_String));
	DBGPRINT(RT_DEBUG_OFF, ("TxPower0 = %d\n", ATECtrl->TxPower0));
	DBGPRINT(RT_DEBUG_OFF, ("TxPower1 = %d\n", ATECtrl->TxPower1));
	DBGPRINT(RT_DEBUG_OFF, ("TxAntennaSel = %d\n", ATECtrl->TxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("RxAntennaSel = %d\n", ATECtrl->RxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("BBPCurrentBW = %u\n", ATECtrl->BW));
	DBGPRINT(RT_DEBUG_OFF, ("GI = %u\n", ATECtrl->Sgi));
	DBGPRINT(RT_DEBUG_OFF, ("MCS = %u\n", ATECtrl->Mcs));

	switch (ATECtrl->PhyMode)
	{
		case 0:
			TxMode_String = "CCK";
			break;
		case 1:
			TxMode_String = "OFDM";
			break;
		case 2:
			TxMode_String = "HT-Mix";
			break;
		case 3:
			TxMode_String = "GreenField";
			break;
		default:
		{
			TxMode_String = "Unknown TxMode";
			DBGPRINT(RT_DEBUG_OFF, ("ERROR! Unknown TxMode!\n"));
			break;
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("TxMode = %s\n", TxMode_String));
	DBGPRINT(RT_DEBUG_OFF, ("Addr1 = %02x:%02x:%02x:%02x:%02x:%02x\n",
		ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr2 = %02x:%02x:%02x:%02x:%02x:%02x\n",
		ATECtrl->Addr2[0], ATECtrl->Addr2[1], ATECtrl->Addr2[2], ATECtrl->Addr2[3], ATECtrl->Addr2[4], ATECtrl->Addr2[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr3 = %02x:%02x:%02x:%02x:%02x:%02x\n",
		ATECtrl->Addr3[0], ATECtrl->Addr3[1], ATECtrl->Addr3[2], ATECtrl->Addr3[3], ATECtrl->Addr3[4], ATECtrl->Addr3[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Channel = %u\n", ATECtrl->Channel));
	DBGPRINT(RT_DEBUG_OFF, ("TxLength = %u\n", ATECtrl->TxLength));
	DBGPRINT(RT_DEBUG_OFF, ("TxCount = %u\n", ATECtrl->TxCount));
	DBGPRINT(RT_DEBUG_OFF, ("RFFreqOffset = %u\n", ATECtrl->RFFreqOffset));
	DBGPRINT(RT_DEBUG_OFF, ("IPG=%u\n", ATECtrl->IPG));
	DBGPRINT(RT_DEBUG_OFF, ("Payload=0x%02x\n", ATECtrl->Payload));

	return TRUE;
}


INT32 SetATEHelp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	DBGPRINT(RT_DEBUG_OFF, ("ATE=ATESTART, ATESTOP, TXCONT, TXCARR, TXCARS, TXFRAME, RXFRAME\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEDA\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATESA\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEBSSID\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATECHANNEL, range:0~14\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW0, set power level of antenna 1.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW1, set power level of antenna 2.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERXANT, set RX antenna.0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXCNT, set how many frame going to transmit.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXMCS, set MCS, reference to rate table.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, 4:VHT, reference to rate table.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXGI, set GI interval, 0:Long, 1:Short\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERXFER, 0:disable Rx Frame error rate. 1:enable Rx Frame error rate.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERRF, show all RF registers.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATELDE2P, load EEPROM from .bin file.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERE2P, display all EEPROM content.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEAUTOALC, enable ATE auto Tx alc (Tx auto level control).\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEIPG, set ATE Tx frame IPG.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEPAYLOAD, set ATE payload pattern for TxFrame.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATESHOW, display all parameters of ATE.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEHELP, online help.\n"));

	return TRUE;
}


VOID ATESampleRssi(PRTMP_ADAPTER pAd, RXWI_STRUC *pRxWI)
{
}


#ifdef RTMP_PCI_SUPPORT
PNDIS_PACKET ATEPayloadInit(RTMP_ADAPTER *pAd, UINT32 TxIdx)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
	PNDIS_PACKET pPacket = pTxRing->Cell[TxIdx].pNdisPacket;
	PUCHAR pDest = (PUCHAR)ATECtrl->AteAllocVa[TxIdx];
	UINT32 Pos = 0;

	GET_OS_PKT_LEN(pPacket) = ATECtrl->TxLength - ATECtrl->HLen;

	DBGPRINT(RT_DEBUG_TRACE, ("%s, %d, %d, %d\n", __FUNCTION__, GET_OS_PKT_LEN(pPacket),
						ATECtrl->TxLength, ATECtrl->HLen));

	/* prepare frame payload */
	for (Pos = 0; Pos < GET_OS_PKT_LEN(pPacket); Pos++)
	{
		if (ATECtrl->bFixedPayload)
		{
			/* default payload is 0xA5 */
			pDest[Pos] = ATECtrl->Payload;
		}
		else
		{
			pDest[Pos] = RandomByte(pAd);
		}
	}

	return pPacket;
}
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_PCI_SUPPORT
INT32 ATEPayloadAlloc(PRTMP_ADAPTER pAd, UINT32 Index)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	NDIS_PHYSICAL_ADDRESS AllocPa;

	ATECtrl->pAtePacket[Index] = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			ATECtrl->TxLength, FALSE, &ATECtrl->AteAllocVa[Index], &AllocPa);

	if (ATECtrl->pAtePacket[Index] == NULL)
	{
		ATECtrl->TxCount = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("%s : fail to alloc packet space.\n",
										 __FUNCTION__));
		return (NDIS_STATUS_RESOURCES);
	}

	return (NDIS_STATUS_SUCCESS);
}
#endif /* RTMP_MAC_PCI */

//#ifdef RTMP_PCI_SUPPORT

INT32 ATEInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	NdisZeroMemory(ATECtrl, sizeof(*ATECtrl));

	ATECtrl->Mode = ATE_STOP;

	ATECtrl->TxCount = 0xFFFFFFFF;
	ATECtrl->Payload = 0xAA;
	ATECtrl->bFixedPayload = 1;
	ATECtrl->IPG = 200;/* 200 : sync with QA */
	ATECtrl->TxLength = 1058;/* 1058 : sync with QA */
	ATECtrl->BW = BW_20;
	ATECtrl->PhyMode = MODE_OFDM;
	ATECtrl->Mcs = 7;
	ATECtrl->Sgi = 0;/* LONG GI : 800 ns*/
	ATECtrl->Channel = 1;
	ATECtrl->TxAntennaSel = 1;
	ATECtrl->RxAntennaSel = 0;

	ATECtrl->QID = QID_AC_BE;
	ATECtrl->Addr1[0] = 0x00;
	ATECtrl->Addr1[1] = 0x11;
	ATECtrl->Addr1[2] = 0x22;
	ATECtrl->Addr1[3] = 0xAA;
	ATECtrl->Addr1[4] = 0xBB;
	ATECtrl->Addr1[5] = 0xCC;

	NdisMoveMemory(ATECtrl->Addr2, ATECtrl->Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(ATECtrl->Addr3, ATECtrl->Addr1, MAC_ADDR_LEN);
	ATECtrl->bQAEnabled = FALSE;
	ATECtrl->bQATxStart = FALSE;
	ATECtrl->bQARxStart = FALSE;
	ATECtrl->TxDoneCount = 0;


	ATECtrl->TemplateFrame = TemplateFrame;

#ifdef MT_MAC
	Ret = MT_ATEInit(pAd);
#endif

#ifdef CONFIG_QA
	ATECtrl->TxStatus = 0;
#endif

	return Ret;
}


INT32 ATEExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	//ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	return Ret;
}

VOID  ATEPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext,
						PVOID SystemSpecific2, PVOID SystemSpecific3)
{

}


INT32 SetATE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Arg = %s\n", __FUNCTION__, Arg));

	if (!strcmp(Arg, "ATESTART") && (ATECtrl->Mode != ATE_START)) /* support restart w/o ATESTOP */
	{
		Ret = ATEOp->ATEStart(pAd);
	}
	else if (!strcmp(Arg, "ATESTOP") && (ATECtrl->Mode & ATE_START))
	{
		Ret = ATEOp->ATEStop(pAd);
	}
	else if (!strcmp(Arg, "APSTOP") && (ATECtrl->Mode == ATE_STOP))
	{
		Ret = ATEOp->ATEStart(pAd);
	}
	else if (!strcmp(Arg, "APSTART") && (ATECtrl->Mode & ATE_START))
	{
		Ret = ATEOp->ATEStop(pAd);
	}
	else if (!strcmp(Arg, "TXFRAME") && (ATECtrl->Mode & ATE_START))
	{
		Ret = ATEOp->StartTx(pAd);
	}
	else if (!strcmp(Arg, "RXFRAME") && (ATECtrl->Mode & ATE_START))
	{
		Ret = ATEOp->StartRx(pAd);
	}
	else if (!strcmp(Arg, "TXSTOP") && (ATECtrl->Mode & ATE_START))
	{
		Ret = ATEOp->StopTx(pAd, ATECtrl->Mode);
	}
	else if (!strcmp(Arg, "RXSTOP") && (ATECtrl->Mode & ATE_START))
	{
		Ret = ATEOp->StopRx(pAd);
	}
	else if (!strcmp(Arg, "TXCONT") && (ATECtrl->Mode & ATE_START))
	{
    	/* 0: All 1:TX0 2:TX1 */
		switch(ATECtrl->TxAntennaSel){
		case 0:
			Ret = ATEOp->StartContinousTx(pAd, 2);
			break;
		case 1:
			Ret = ATEOp->StartContinousTx(pAd, 0);
			break;
		case 2:
			Ret = ATEOp->StartContinousTx(pAd, 1);
			break;
		}
		ATECtrl->Mode |= ATE_TXCONT;
	}
	else if (!strcmp(Arg, "TXCARRSUPP") && (ATECtrl->Mode & ATE_START))
	{
    	INT32 pwr1 = 0xf;
		INT32 pwr2 = 0;
		/* 0: All 1:TX0 2:TX1 */
		switch(ATECtrl->TxAntennaSel){
		case 0:
			DBGPRINT(RT_DEBUG_TRACE, ("%s: not support two 2 TXCARRSUPP\n",__FUNCTION__));
			break;
		case 1:
			if(ATECtrl->TxPower0>30)
				pwr2 = (ATECtrl->TxPower0 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower0 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower0 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF0_TX_TWO_TONE_5M);
			DBGPRINT(RT_DEBUG_TRACE, ("%s: TXCARRSUPP, TxPower0:0x%x, pwr1:0x%x, pwr2:0x%x\n",__FUNCTION__,ATECtrl->TxPower0, pwr1, pwr2));
			break;
		case 2:
			if(ATECtrl->TxPower1>30)
				pwr2 = (ATECtrl->TxPower1 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower1 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower1 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF1_TX_TWO_TONE_5M);
			DBGPRINT(RT_DEBUG_TRACE, ("%s: TXCARRSUPP, TxPower1:0x%x, pwr1:0x%x, pwr2:0x%x\n",__FUNCTION__,ATECtrl->TxPower1, pwr1, pwr2));
			break;
		}
		ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
		ATECtrl->Mode |= ATE_TXCARRSUPP;
	}
	else if (!strcmp(Arg, "TXTONEPWR") && (ATECtrl->Mode & ATE_START))
	{
		ATEOp->SetTxTonePower(pAd, ATECtrl->TxPower0, ATECtrl->TxPower1);
	}
	else if (!strcmp(Arg, "TXCARR") && (ATECtrl->Mode & ATE_START))
	{
		INT32 pwr1 = 0xf;
		INT32 pwr2 = 0;
		switch(ATECtrl->TxAntennaSel){
		case 0:
			DBGPRINT(RT_DEBUG_OFF, ("%s: not support two 2 TXCARR\n",__FUNCTION__));
			break;
		case 1:
			if(ATECtrl->TxPower0>30)
				pwr2 = (ATECtrl->TxPower0 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower0 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower0 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF0_TX_ONE_TONE_DC);
			DBGPRINT(RT_DEBUG_TRACE, ("%s: TXCARR, TxPower0:0x%x, pwr1:0x%x, pwr2:0x%x\n",__FUNCTION__,ATECtrl->TxPower0, pwr1, pwr2));
			break;
		case 2:
			if(ATECtrl->TxPower1>30)
				pwr2 = (ATECtrl->TxPower1 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower1 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower1 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF1_TX_ONE_TONE_DC);
			DBGPRINT(RT_DEBUG_TRACE, ("%s: TXCARR, TxPower1:0x%x, pwr1:0x%x, pwr2:0x%x\n",__FUNCTION__,ATECtrl->TxPower1, pwr1, pwr2));			
			break;
		}
		ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
		ATECtrl->Mode |= ATE_TXCARR;
	}
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: do nothing(param = (%s), mode = (%d))\n",
										__FUNCTION__, Arg, ATECtrl->Mode));
	}

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEChannel(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT16 Channel;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Channel = %s\n", __FUNCTION__, Arg));

	Channel = simple_strtol(Arg, 0, 10);

	ATECtrl->Channel = Channel;

	if (ATECtrl->BW == BW_20)
		ATECtrl->ControlChl = ATECtrl->Channel;
	else if (ATECtrl->BW == BW_40)
		ATECtrl->ControlChl = ATECtrl->Channel - 2;

	Ret = ATEOp->SetChannel(pAd, Channel);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxBw(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT16 BW;

	DBGPRINT(RT_DEBUG_OFF, ("%s: Bw = %s\n", __FUNCTION__, Arg));

	BW = simple_strtol(Arg, 0, 10);

	ATECtrl->BW = BW;

	if (ATECtrl->BW == BW_20)
		ATECtrl->ControlChl = ATECtrl->Channel;
	else if (ATECtrl->BW == BW_40){
                if(ATECtrl->Channel <= 2){
                        ATECtrl->ControlChl = ATECtrl->Channel;
                        ATECtrl->Channel += 2;
                }else if(ATECtrl->Channel > 12){
                        ATECtrl->ControlChl = ATECtrl->Channel;
                        ATECtrl->Channel -= 2;
                }else{
                        ATECtrl->ControlChl = ATECtrl->Channel - 2;
                }
        }

	Ret = ATEOp->SetBW(pAd, BW);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}
