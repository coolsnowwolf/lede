/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	eeprom.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#include "rt_config.h"

struct chip_map{
	UINT32 ChipVersion;
	PSTRING name;
};

struct chip_map RTMP_CHIP_E2P_FILE_TABLE[] = {
#ifdef MT76x2
	{0x7602, "MT7602E_EEPROM.bin"},
	{0x7612, "MT7612E_EEPROM.bin"},
#endif
#ifdef RT6352
	{0x7620, "MT7620_AP_2T2R-4L_V15.BIN"},
#endif
	{0, NULL}
};

#ifdef RTMP_FLASH_SUPPORT
extern USHORT EE_FLASH_ID_LIST[];
#endif /* RTMP_FLASH_SUPPORT */

UCHAR RtmpEepromGetDefault(
	IN RTMP_ADAPTER 	*pAd)
{
	UCHAR e2p_default = E2P_FLASH_MODE;

#if defined (DRIVER_HAS_MULTI_DEV)
	if ( pAd->dev_idx == 0 )
	{
		if ( RTMPEqualMemory("efuse", CONFIG_RT_FIRST_CARD_EEPROM, 5) )
			e2p_default = E2P_EFUSE_MODE;
		if ( RTMPEqualMemory("prom", CONFIG_RT_FIRST_CARD_EEPROM, 4) )
			e2p_default = E2P_EEPROM_MODE;
		if ( RTMPEqualMemory("flash", CONFIG_RT_FIRST_CARD_EEPROM, 5) )
			e2p_default = E2P_FLASH_MODE;
		goto out;
	}

	if ( pAd->dev_idx == 1 )
	{
		if ( RTMPEqualMemory("efuse", CONFIG_RT_SECOND_CARD_EEPROM, 5) )
			e2p_default = E2P_EFUSE_MODE;
		if ( RTMPEqualMemory("prom", CONFIG_RT_SECOND_CARD_EEPROM, 4) )
			e2p_default = E2P_EEPROM_MODE;
		if ( RTMPEqualMemory("flash", CONFIG_RT_SECOND_CARD_EEPROM, 5) )
			e2p_default = E2P_FLASH_MODE;
		goto out;
	}
out:
#endif
	DBGPRINT(RT_DEBUG_OFF, ("%s::e2p_default=%d\n", __FUNCTION__, e2p_default));
	return e2p_default;
}


#if defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT)
static VOID RtmpEepromTypeAdjust(RTMP_ADAPTER *pAd, UCHAR *pE2pType)
{
	UINT EfuseFreeBlock=0;

	eFuseGetFreeBlockCount(pAd, &EfuseFreeBlock);	
	
	if (EfuseFreeBlock >= pAd->chipCap.EFUSE_RESERVED_SIZE)
	{
		DBGPRINT(RT_DEBUG_OFF, ("NVM is efuse and the information is too less to bring up the interface\n"));
		DBGPRINT(RT_DEBUG_OFF, ("Force to use Flash mode\n"));
		*pE2pType = E2P_FLASH_MODE;
	}
	else 
	{
		USHORT eeFlashId = 0;
		int listIdx, num_flash_id;
		BOOLEAN bFound = FALSE;
		
		num_flash_id = rtmp_get_flash_id_num();

		rtmp_ee_efuse_read16(pAd, 0, &eeFlashId);
		DBGPRINT(RT_DEBUG_OFF, ("%s:: eeFlashId = 0x%x.\n", __FUNCTION__, eeFlashId));
		for(listIdx =0 ; listIdx < num_flash_id; listIdx++)
		{
			if (eeFlashId == EE_FLASH_ID_LIST[listIdx])
			{			
				bFound = TRUE;
				break;
			}
		}

		if (bFound == FALSE)
		{
			*pE2pType = E2P_FLASH_MODE;
			pAd->bUseEfuse = FALSE;
		}
	}
}
#endif /* defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT) */


INT RtmpChipOpsEepromHook(
	IN RTMP_ADAPTER 	*pAd,
	IN INT				infType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	UCHAR e2p_type;
	UINT32 val;

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgITxBfBinWrite)
		pAd->E2pAccessMode = E2P_BIN_MODE;
#endif		

/* We can't open file here beacuse it could get file open error. */

	e2p_type = pAd->E2pAccessMode;

	DBGPRINT(RT_DEBUG_OFF, ("%s::e2p_type=%d, inf_Type=%d\n", __FUNCTION__, e2p_type, infType));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
		return -1;

	/* If e2p_type is out of range, get the default mode */
	e2p_type = ((e2p_type != 0) && (e2p_type < NUM_OF_E2P_MODE)) ? e2p_type : RtmpEepromGetDefault(pAd);

	if (infType == RTMP_DEV_INF_RBUS)
	{
		e2p_type = E2P_FLASH_MODE;
		pChipOps->loadFirmware = NULL;
	}
#if defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT)
	else if (pAd->E2pAccessMode == E2P_NONE)
	{
		/*
			User doesn't set E2pAccessMode in profile, adjust access mode automatically here.
		*/
		efuse_probe(pAd);
		if (pAd->bUseEfuse)
			RtmpEepromTypeAdjust(pAd, &e2p_type);
	}
#endif /* defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT) */

	pAd->E2pAccessMode = e2p_type;

	switch (e2p_type)
	{
		case E2P_EEPROM_MODE:
			break;
		case E2P_BIN_MODE:
		{
			pChipOps->eeinit = rtmp_ee_load_from_bin;
			pChipOps->eeread = rtmp_ee_bin_read16;
			pChipOps->eewrite = rtmp_ee_bin_write16;
			DBGPRINT(RT_DEBUG_OFF, ("NVM is BIN mode\n"));	
			return 0;
		}

#ifdef RTMP_FLASH_SUPPORT
		case E2P_FLASH_MODE:
		{
			pChipOps->eeinit = rtmp_nv_init;
			pChipOps->eeread = rtmp_ee_flash_read;
			pChipOps->eewrite = rtmp_ee_flash_write;
			pAd->flash_offset = DEFAULT_RF_OFFSET;
#if defined (DRIVER_HAS_MULTI_DEV)
			if ( pAd->dev_idx == 0 )
				pAd->flash_offset = CONFIG_RT_FIRST_IF_RF_OFFSET;
			if ( pAd->dev_idx == 1 )
				pAd->flash_offset = CONFIG_RT_SECOND_IF_RF_OFFSET;
#endif
			DBGPRINT(RT_DEBUG_OFF, ("NVM is FLASH mode\n"));
			return 0;
		}
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
		default:
		{
			efuse_probe(pAd);
			if (pAd->bUseEfuse)
			{
				pChipOps->eeinit = eFuse_init;
				pChipOps->eeread = rtmp_ee_efuse_read16;
				pChipOps->eewrite = rtmp_ee_efuse_write16;
				DBGPRINT(RT_DEBUG_OFF, ("NVM is EFUSE mode\n"));	
				return 0;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::hook efuse mode failed\n", __FUNCTION__));
				break;
			}
		}
#endif /* RTMP_EFUSE_SUPPORT */
	}

	/* Hook functions based on interface types for EEPROM */
	switch (infType) 
	{
#ifdef RTMP_PCI_SUPPORT
		case RTMP_DEV_INF_PCI:
		case RTMP_DEV_INF_PCIE:
			RTMP_IO_READ32(pAd, E2PROM_CSR, &val);
			if (((val & 0x30) == 0) && (!IS_RT3290(pAd)))
				pAd->EEPROMAddressNum = 6; /* 93C46 */
			else
				pAd->EEPROMAddressNum = 8; /* 93C66 or 93C86 */

			pChipOps->eeinit = NULL;
			pChipOps->eeread = rtmp_ee_prom_read16;
			pChipOps->eewrite = rtmp_ee_prom_write16;
			break;
#endif /* RTMP_PCI_SUPPORT */

		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::hook failed\n", __FUNCTION__));
			break;
	}

	DBGPRINT(RT_DEBUG_OFF, ("NVM is EEPROM mode\n"));
	return 0;
}


BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 	ChipVersion,
	OUT PSTRING *pBinFileName)
{
	BOOLEAN found = FALSE;
	INT i;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s::chip version=0x%04x\n", __FUNCTION__, ChipVersion));
	
	for (i = 0; RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion != 0; i++ )
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			ChipVersion = 0x7620;
		}
#endif
		if (RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion == ChipVersion)
		{
			*pBinFileName = RTMP_CHIP_E2P_FILE_TABLE[i].name;
			DBGPRINT(RT_DEBUG_OFF, 
						("%s(): Found E2P bin file name:%s\n",
						__FUNCTION__, *pBinFileName));
			found = TRUE;
			break;
		}
	}

	if (found == TRUE)
		DBGPRINT(RT_DEBUG_OFF, ("%s::Found E2P bin file name=%s\n", __FUNCTION__, *pBinFileName));
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s::E2P bin file name not found\n", __FUNCTION__));
	
	return found;	
}


INT rtmp_ee_bin_read16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset,
	OUT USHORT 			*pValue)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s::Read from EEPROM buffer\n", __FUNCTION__));
	NdisMoveMemory(pValue, &(pAd->EEPROMImage[Offset]), 2);
	*pValue = le2cpu16(*pValue);

	return (*pValue);
}


INT rtmp_ee_bin_write16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			data)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s::Write to EEPROM buffer\n", __FUNCTION__));
	data = le2cpu16(data);
	NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &data, 2);

	return 0;
}


INT rtmp_ee_load_from_bin(
	IN PRTMP_ADAPTER 	pAd)
{
	PSTRING src = NULL;
	INT ret_val;			
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	STRING bin_file_path[128];
	PSTRING bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
		src = BIN_FILE_PATH;
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else 
		{
			NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			ret_val = RtmpOSFileRead(srcf, (PSTRING)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			
			if (ret_val > 0)
				ret_val = NDIS_STATUS_SUCCESS;
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s::Read file \"%s\" failed(errCode=%d)!\n", __FUNCTION__, src, ret_val));
      		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);
			
	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;	
}


INT rtmp_ee_write_to_bin(
	IN PRTMP_ADAPTER 	pAd)
{
	PSTRING src = NULL;
	INT ret_val;			
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	STRING bin_file_path[128];
	PSTRING bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
		src = BIN_FILE_PATH;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_WRONLY|O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else 
			RtmpOSFileWrite(srcf, (PSTRING)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);
			
	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));
	
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;	
}


INT Set_LoadEepromBufferFromBin_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	INT result;

#ifdef CAL_FREE_IC_SUPPORT
		BOOLEAN bCalFree=0;
#endif /* CAL_FREE_IC_SUPPORT */

	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM buffer from BIN, and change to BIN buffer mode\n"));	
		result = rtmp_ee_load_from_bin(pAd);

		if ( result == FALSE )
		{
			if ( pAd->chipCap.EEPROM_DEFAULT_BIN != NULL )
			{
				NdisMoveMemory(pAd->EEPROMImage, pAd->chipCap.EEPROM_DEFAULT_BIN,
					pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE?MAX_EEPROM_BUFFER_SIZE:pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE);
				DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM Buffer from default BIN.\n"));
			}

		}

		/* Change to BIN eeprom buffer mode */
		pAd->E2pAccessMode = E2P_BIN_MODE;
		RtmpChipOpsEepromHook(pAd, pAd->infType);

#ifdef CAL_FREE_IC_SUPPORT
		RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

		if ( bCalFree == TRUE ) {
			RTMP_CAL_FREE_DATA_GET(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("Load Cal Free data from e-fuse.\n"));
		}
#endif /* CAL_FREE_IC_SUPPORT */
		
		return TRUE;
	}
}


INT Set_EepromBufferWriteBack_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg)
{
	UINT e2p_mode = simple_strtol(arg, 0, 10);

	if (e2p_mode >= NUM_OF_E2P_MODE)
		return FALSE;

	switch (e2p_mode)
	{		
#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to eFuse\n"));
			rtmp_ee_write_to_efuse(pAd);
			break;
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RTMP_FLASH_SUPPORT			
		case E2P_FLASH_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to Flash\n"));
			rtmp_ee_flash_write_all(pAd, (PUSHORT)pAd->EEPROMImage);
			break;
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RT65xx
		case E2P_EEPROM_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to EEPROM\n"));
			rtmp_ee_write_to_prom(pAd);
			break;
#endif /* RT65xx */

		case E2P_BIN_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to BIN\n"));	
			rtmp_ee_write_to_bin(pAd);
			break;
			
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::do not support this EEPROM access mode\n", __FUNCTION__));
			return FALSE;
	}
	
	return TRUE;
}

