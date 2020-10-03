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
	ee_flash.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RTMP_FLASH_SUPPORT

#include	"rt_config.h"






USHORT EE_FLASH_ID_LIST[]={

#ifdef RT6352
	0x6352,
	0x7620,
#endif /* RT6352 */



#ifdef MT76x2
#ifdef RTMP_MAC_PCI
	0x7612,
	0x7662,
#endif /* RTMP_MAC_PCI */
#endif /* MT76x0 */

};

static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start);

INT rtmp_get_flash_id_num(VOID)
{
	return (sizeof(EE_FLASH_ID_LIST) / sizeof(USHORT));
}

/*******************************************************************************
  *
  *	Flash-based EEPROM read/write procedures.
  *		some chips use the flash memory instead of internal EEPROM to save the 
  *		calibration info, we need these functions to do the read/write.
  *
  ******************************************************************************/
int rtmp_ee_flash_read(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT Offset,
	OUT USHORT *pValue)
{	
	if (!pAd->chipCap.ee_inited)
	{
		*pValue = 0xffff;
	}
	else
	{
		memcpy(pValue, pAd->eebuf + Offset, 2);
	}
	return (*pValue);
}


int rtmp_ee_flash_write(PRTMP_ADAPTER pAd, USHORT Offset, USHORT Data)
{
	if (pAd->chipCap.ee_inited)
	{
		memcpy(pAd->eebuf + Offset, &Data, 2);
		/*rt_nv_commit();*/
		/*rt_cfg_commit();*/
#ifdef MULTIPLE_CARD_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("rtmp_ee_flash_write:pAd->MC_RowID = %d\n", pAd->MC_RowID));
		DBGPRINT(RT_DEBUG_TRACE, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
		if ((pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x48000) || (pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x40000))
			RtmpFlashWrite(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
		RtmpFlashWrite(pAd->eebuf, pAd->flash_offset, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */
	}
	return 0;
}


VOID rtmp_ee_flash_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{	
	if (!pAd->chipCap.ee_inited)
		return;
		
	memcpy(Data, pAd->eebuf, EEPROM_SIZE);
}


VOID rtmp_ee_flash_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	if (!pAd->chipCap.ee_inited)
		return;
	memcpy(pAd->eebuf, Data, EEPROM_SIZE);
#ifdef MULTIPLE_CARD_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("rtmp_ee_flash_write_all:pAd->MC_RowID = %d\n", pAd->MC_RowID));
	DBGPRINT(RT_DEBUG_TRACE, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
	if ((pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x48000) || (pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x40000))
		RtmpFlashWrite(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
	RtmpFlashWrite(pAd->eebuf, pAd->flash_offset, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */
}


static NDIS_STATUS rtmp_ee_flash_reset(RTMP_ADAPTER *pAd, UCHAR *start)
{
	PUCHAR src;
	RTMP_OS_FS_INFO osFsInfo;
	RTMP_OS_FD srcf;
	INT retval;
	STRING file_path[128];
	PSTRING file_name = NULL;
	UINT32 chip_id = (pAd->ChipID >> 16);

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	STRING	BinFilePath[128];
	PSTRING	pBinFileName = NULL;
	UINT32	ChipVerion = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, ChipVerion, &pBinFileName) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(BinFilePath, "%s%s", EEPROM_2ND_FILE_DIR, pBinFileName);
		else
			sprintf(BinFilePath, "%s%s", EEPROM_1ST_FILE_DIR, pBinFileName);

		src = BinFilePath;
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): src = %s\n", __FUNCTION__, src));
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
	if (rtmp_get_default_bin_file_by_chip(pAd, chip_id, &file_name) == TRUE) {
		sprintf(file_path, "%s%s", EEPROM_FILE_DIR, file_name);
		src = file_path;
		DBGPRINT(RT_DEBUG_OFF, ("%s()::src = %s\n", __FUNCTION__, src));
	}
	else
		src = EEPROM_DEFAULT_FILE_PATH;

	RtmpOSFSInfoChange(&osFsInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> Error opening file %s\n", src));
			return NDIS_STATUS_FAILURE;
		}
		else 
		{
			/* The object must have a read method*/
			NdisZeroMemory(start, EEPROM_SIZE);
			
			retval = RtmpOSFileRead(srcf, start, EEPROM_SIZE);
			if (retval < 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> Read %s error %d\n", src, -retval));
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> rtmp_ee_flash_reset copy %s to eeprom buffer\n", src));
			}

			retval = RtmpOSFileClose(srcf);
			if (retval)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
			}
		}
	}

	RtmpOSFSInfoChange(&osFsInfo, FALSE);

	return NDIS_STATUS_SUCCESS;
}

#ifdef LINUX
/* 0 -- Show ee buffer */
/* 1 -- force reset to default */
/* 2 -- Change ee settings */
int	Set_EECMD_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	USHORT i;
	
	i = simple_strtol(arg, 0, 10);
	switch(i)
	{
		case 0:
			{
				USHORT value, k;
				for (k = 0; k < EEPROM_SIZE; k+=2)
				{
					RT28xx_EEPROM_READ16(pAd, k, value);
					DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", value));
					if (((k+2) % 0x20) == 0)
						DBGPRINT(RT_DEBUG_OFF,("\n"));
				}
				
			}
			break;
		case 1:
			if (pAd->infType == RTMP_DEV_INF_RBUS)
			{
				DBGPRINT(RT_DEBUG_OFF, ("EEPROM reset to default......\n"));
				DBGPRINT(RT_DEBUG_OFF, ("The last byte of MAC address will be re-generated...\n"));
				if (rtmp_ee_flash_reset(pAd, pAd->eebuf) != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Set_EECMD_Proc: rtmp_ee_flash_reset() failed\n"));
					return FALSE;
				}
			
				/* Random number for the last bytes of MAC address*/
				{
					USHORT  Addr45;

					rtmp_ee_flash_read(pAd, 0x08, &Addr45);
					Addr45 = Addr45 & 0xff;
					Addr45 = Addr45 | (RandomByte(pAd)&0xf8) << 8;
					DBGPRINT(RT_DEBUG_OFF, ("Addr45 = %4x\n", Addr45));
					rtmp_ee_flash_write(pAd, 0x08, Addr45);
				}
			
				if ((rtmp_ee_flash_read(pAd, 0, &i) != 0x2880) && (rtmp_ee_flash_read(pAd, 0, &i) != 0x2860))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Set_EECMD_Proc: invalid eeprom\n"));
					return FALSE;
				}
			}
			break;
		case 2:
			{
				USHORT offset, value = 0;
				PUCHAR p;
				
				p = arg+2;
				offset = simple_strtol(p, 0, 10);
				p+=2;
				while (*p != '\0')
				{
					if (*p >= '0' && *p <= '9')
						value = (value << 4) + (*p - 0x30);
					else if (*p >= 'a' && *p <= 'f')
						value = (value << 4) + (*p - 0x57);
					else if (*p >= 'A' && *p <= 'F')
						value = (value << 4) + (*p - 0x37);
					p++;
				}
				RT28xx_EEPROM_WRITE16(pAd, offset, value);
			}
			break;
		default:
			break;
	}

	return TRUE;
}
#endif /* LINUX */


static BOOLEAN  validFlashEepromID(RTMP_ADAPTER *pAd)
{
	USHORT eeFlashId;
	int listIdx, num_flash_id;

	num_flash_id = rtmp_get_flash_id_num();

	rtmp_ee_flash_read(pAd, 0, &eeFlashId);
	for(listIdx =0 ; listIdx < num_flash_id; listIdx++)
	{
		if (eeFlashId == EE_FLASH_ID_LIST[listIdx])
			return TRUE;
	}
	return FALSE;
}


static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start)
{
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN bCalFree=0;
#endif /* CAL_FREE_IC_SUPPORT */

	pAd->chipCap.ee_inited = 1;

	if (validFlashEepromID(pAd) == FALSE)
	{
		if (rtmp_ee_flash_reset(pAd, start) != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rtmp_ee_init(): rtmp_ee_flash_init() failed\n"));
			return NDIS_STATUS_FAILURE;
		}

		/* Random number for the last bytes of MAC address*/
		{
			USHORT  Addr45;
			
			rtmp_ee_flash_read(pAd, 0x08, &Addr45);
			Addr45 = Addr45 & 0xff;
			Addr45 = Addr45 | (RandomByte(pAd)&0xf8) << 8;

#ifdef CAL_FREE_IC_SUPPORT
			RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

			if ( bCalFree == TRUE ) {
				RTMP_CAL_FREE_DATA_GET(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("Load Cal Free data from e-fuse.\n"));
			}
#endif /* CAL_FREE_IC_SUPPORT */

			rtmp_ee_flash_write(pAd, 0x08, Addr45);
			DBGPRINT(RT_DEBUG_ERROR, ("The EEPROM in Flash is wrong, use default\n"));
		}

		if (validFlashEepromID(pAd) == FALSE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rtmp_ee_flash_init(): invalid eeprom\n"));
			return NDIS_STATUS_FAILURE;
		}
	}
	
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_nv_init(RTMP_ADAPTER *pAd)
{
#ifdef MULTIPLE_CARD_SUPPORT
	UCHAR *eepromBuf;
#endif /* MULTIPLE_CARD_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("--> rtmp_nv_init\n"));


/*
	if (pAd->chipCap.EEPROMImage == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pAd->chipCap.EEPROMImage == NULL!!!\n"));
		return NDIS_STATUS_FAILURE;
	}
*/
	
/*	ASSERT((pAd->eebuf == NULL)); */
	pAd->eebuf = pAd->EEPROMImage;
#ifdef MULTIPLE_CARD_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("rtmp_nv_init:pAd->MC_RowID = %d\n", pAd->MC_RowID));
	os_alloc_mem(pAd, &eepromBuf, EEPROM_SIZE);
	if (eepromBuf)
	{	
		pAd->eebuf = eepromBuf;
		NdisMoveMemory(pAd->eebuf, pAd->chipCap.EEPROM_DEFAULT_BIN, EEPROM_SIZE);
		}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("rtmp_nv_init:Alloc memory for pAd->MC_RowID[%d] failed! used default one!\n", pAd->MC_RowID));
	}
	DBGPRINT(RT_DEBUG_OFF, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
	RtmpFlashRead(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
	RtmpFlashRead(pAd->eebuf, pAd->flash_offset, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */

	return rtmp_ee_flash_init(pAd, pAd->eebuf);
}

INT Set_LoadEepromBufferFromFlash_Proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	UINT free_blk = 0;

	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM buffer from flash, and change to BIN buffer mode\n"));

		NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
		RtmpFlashRead(pAd->EEPROMImage, pAd->flash_offset, MAX_EEPROM_BUFFER_SIZE);
																

		/* Change to BIN eeprom buffer mode */
		pAd->E2pAccessMode = E2P_BIN_MODE;
		RtmpChipOpsEepromHook(pAd, pAd->infType);
		return TRUE;
	}
}
#endif /* RTMP_FLASH_SUPPORT */

