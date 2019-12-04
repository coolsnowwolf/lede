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
#include "hdev/hdev.h"

/*decision flash api by compiler flag*/
#ifdef CONFIG_PROPRIETARY_DRIVER
/*
* @ used for proprietary driver support, can't read/write mtd on driver
* @ read: mtd flash patrition use request firmware to load
* @ write: write not support, use ated to write to flash
*/
static void flash_bin_read(void *ctrl, UCHAR *p, ULONG a, ULONG b)
{
	struct _RTMP_ADAPTER *ad = ((struct hdev_ctrl *) ctrl)->priv;
	UCHAR *buffer = NULL;
	UINT32 len;
	UCHAR *name = get_dev_eeprom_binary(ad);

	/*load from request firmware*/
	os_load_code_from_bin(ad, &buffer, name, &len);

	if (len > 0 && buffer != NULL) {
		os_move_mem(p, buffer + a, b);
		os_free_mem(buffer);
	}
}

static void flash_bin_write(void *ctrl, UCHAR *p, ULONG a, ULONG b)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		 ("proprietary driver not support flash write, will write on ated.\n"));
}

#define flash_read(_ctrl, _ptr, _offset, _len) flash_bin_read(_ctrl, _ptr, _offset, _len)
#define flash_write(_ctrl, _ptr, _offset, _len) flash_bin_write(_ctrl, _ptr, _offset, _len)

#else

#ifdef CONFIG_RALINK_FLASH_API
/*
* @ The flag "CONFIG_RALINK_FLASH_API" is used for APSoC Linux SDK
*/

int32_t FlashRead(
	uint32_t *dst,
	uint32_t *src,
	uint32_t count);

int32_t FlashWrite(
	uint16_t *source,
	uint16_t *destination,
	uint32_t numBytes);

#define flash_read(_ctrl, _ptr, _offset, _len) FlashRead((uint16_t *)_ptr, (uint16_t *)_offset, (uint32_t)_len)
#define flash_write(_ctrl, _ptr, _offset, _len) FlashWrite(_ptr, _offset, _len)

#else
/*============================================================================*/
#ifdef RA_MTD_RW_BY_NUM

/*
* @ The flag "CONFIG_RALINK_FLASH_API" is used for APSoC Linux SDK
*/

#if defined(CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif
extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);

#define flash_read(_ctrl, _ptr, _offset, _len) ra_mtd_read(MTD_NUM_FACTORY, 0, (size_t)_len, _ptr)
#define flash_write(_ctrl, _ptr, _offset, _len) ra_mtd_write(MTD_NUM_FACTORY, 0, (size_t)_len, _ptr)

#else

#ifdef CONFIG_WIFI_MTD
/*
* @ used mtd mode flash partition from proprietary driver mt_wifi_mtd.c
*/
int mt_mtd_write_nm_wifi(char *name, loff_t to, size_t len, const u_char *buf);
int mt_mtd_read_nm_wifi(char *name, loff_t from, size_t len, u_char *buf);

#define flash_read(_ctrl, _ptr, _offset, _len) mt_mtd_read_nm_wifi("Factory", _offset, (size_t)_len, _ptr)
#define flash_write(_ctrl, _ptr, _offset, _len) mt_mtd_write_nm_wifi("Factory", _offset, (size_t)_len, _ptr)

#else
/*
* @ use sdk export func.
*/

extern int ra_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);

#define flash_read(_ctrl, _ptr, _offset, _len) ra_mtd_read_nm("Factory", _offset, (size_t)_len, _ptr)
#define flash_write(_ctrl, _ptr, _offset, _len) ra_mtd_write_nm("Factory", _offset, (size_t)_len, _ptr)

#endif /*CONFIG_WIFI_MTD*/
#endif /*RA_MTD_RW_BY_NUM*/
#endif /* CONFIG_RALINK_FLASH_API */
#endif /*CONFIG_PROPRIETERY_DRIVER*/


void RtmpFlashRead(
	void *hdev_ctrl,
	UCHAR *p,
	ULONG a,
	ULONG b)
{
	flash_read(hdev_ctrl, p, a, b);
}

void RtmpFlashWrite(
	void *hdev_ctrl,
	UCHAR *p,
	ULONG a,
	ULONG b)
{
	flash_write(hdev_ctrl, p, a, b);
}



static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start);

static BOOLEAN rtmp_ee_init_check(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	return cap->ee_inited;
}

#define IS_EE_INIT(_ad) (rtmp_ee_init_check(_ad))

/*******************************************************************************
  *
  *	Flash-based EEPROM read/write procedures.
  *		some chips use the flash memory instead of internal EEPROM to save the
  *		calibration info, we need these functions to do the read/write.
  *
  ******************************************************************************/
BOOLEAN rtmp_ee_flash_read(PRTMP_ADAPTER pAd, UINT16 Offset, UINT16 *pValue)
{
	BOOLEAN IsEmpty = 0;

	if (!IS_EE_INIT(pAd))
		*pValue = 0xffff;
	else
		memcpy(pValue, pAd->EEPROMImage + Offset, 2);

	if ((*pValue == 0xffff) || (*pValue == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


int rtmp_ee_flash_write(PRTMP_ADAPTER pAd, USHORT Offset, USHORT Data)
{
	if (IS_EE_INIT(pAd)) {
		memcpy(pAd->EEPROMImage + Offset, &Data, 2);
		/*rt_nv_commit();*/
		/*rt_cfg_commit();*/
		RtmpFlashWrite(pAd->hdev_ctrl, pAd->EEPROMImage, get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));
	}

	return 0;
}


BOOLEAN rtmp_ee_flash_read_with_range(PRTMP_ADAPTER pAd, UINT16 start, UINT16 Length, UCHAR *pbuf)
{
	BOOLEAN IsEmpty = 0;
	UINT16  u2Loop;
	UCHAR   ucValue = 0;

	if (!IS_EE_INIT(pAd))
		*pbuf = 0xff;
	else
		memcpy(pbuf, pAd->EEPROMImage + start, Length);

	for (u2Loop = 0; u2Loop < Length; u2Loop++)
		ucValue |= pbuf[u2Loop];

	if ((ucValue == 0xff) || (ucValue == 0x00))
		IsEmpty = 1;

	return IsEmpty;
}


int rtmp_ee_flash_write_with_range(PRTMP_ADAPTER pAd, USHORT start, USHORT Length, UCHAR *pbuf)
{
	if (IS_EE_INIT(pAd)) {
		memcpy(pAd->EEPROMImage + start, pbuf, Length);
		/*rt_nv_commit();*/
		/*rt_cfg_commit();*/
		RtmpFlashWrite(pAd->hdev_ctrl, pAd->EEPROMImage, get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));
	}

	return 0;
}


VOID rtmp_ee_flash_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	if (!IS_EE_INIT(pAd))
		return;

	memcpy(Data, pAd->EEPROMImage, get_dev_eeprom_size(pAd));
}


VOID rtmp_ee_flash_write_all(PRTMP_ADAPTER pAd)
{
	if (!IS_EE_INIT(pAd))
		return;

	RtmpFlashWrite(pAd->hdev_ctrl, pAd->EEPROMImage, get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));
}


static NDIS_STATUS rtmp_ee_flash_reset(RTMP_ADAPTER *pAd, UCHAR *start)
{
	PUCHAR src;
	RTMP_OS_FS_INFO osFsInfo;
	RTMP_OS_FD srcf;
	INT retval;
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	RTMP_STRING BinFilePath[128];
	RTMP_STRING *pBinFileName = NULL;
	UINT32	ChipVerion = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, ChipVerion, &pBinFileName) == TRUE) {
		if (pAd->MC_RowID > 0)
			sprintf(BinFilePath, "%s%s", EEPROM_2ND_FILE_DIR, pBinFileName);
		else
			sprintf(BinFilePath, "%s%s", EEPROM_1ST_FILE_DIR, pBinFileName);

		src = BinFilePath;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): src = %s\n", __func__, src));
	} else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
	{
#if defined(MT_MAC) && defined(MT7603)

		if (IS_MT7603(pAd))
			src = EEPROM_DEFAULT_7603_FILE_PATH;
		else
#endif /* MT_MAC && MT7603 */
		{
			src = EEPROM_DEFAULT_FILE_PATH;
		}
	}

	pE2pCtrl->e2pSource = E2P_SRC_FROM_BIN;
	RtmpOSFSInfoChange(&osFsInfo, TRUE);

	if (src && *src) {
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("--> Error opening file %s\n", src));

			if (cap->EEPROM_DEFAULT_BIN != NULL) {
				NdisMoveMemory(start, cap->EEPROM_DEFAULT_BIN,
							   cap->EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE ? MAX_EEPROM_BUFFER_SIZE : cap->EEPROM_DEFAULT_BIN_SIZE);
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Load EEPROM Buffer from default BIN.\n"));
				pE2pCtrl->BinSource = "Default bin";
				return NDIS_STATUS_SUCCESS;
			} else
				return NDIS_STATUS_FAILURE;
		} else {
			/* The object must have a read method*/
			NdisZeroMemory(start, EEPROM_SIZE);
			retval = RtmpOSFileRead(srcf, start, EEPROM_SIZE);

			if (retval < 0) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("--> Read %s error %d\n", src, -retval));
			} else {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("--> rtmp_ee_flash_reset copy %s to eeprom buffer\n", src));
				pE2pCtrl->BinSource = src;
			}

			retval = RtmpOSFileClose(srcf);

			if (retval) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("--> Error %d closing %s\n", -retval, src));
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
	i = os_str_tol(arg, 0, 10);

	switch (i) {
	case 0: {
		USHORT value, k;

		for (k = 0; k < EEPROM_SIZE; k += 2) {
			RT28xx_EEPROM_READ16(pAd, k, value);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%4.4x ", value));

			if (((k + 2) % 0x20) == 0)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}
	}
	break;

	case 1:
		if (pAd->infType == RTMP_DEV_INF_RBUS) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EEPROM reset to default......\n"));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("The last byte of MAC address will be re-generated...\n"));

			if (rtmp_ee_flash_reset(pAd, pAd->EEPROMImage) != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_EECMD_Proc: rtmp_ee_flash_reset() failed\n"));
				return FALSE;
			}

			/* Random number for the last bytes of MAC address*/
			{
				USHORT  Addr45;
				rtmp_ee_flash_read(pAd, 0x08, &Addr45);
				Addr45 = Addr45 & 0xff;
				Addr45 = Addr45 | (RandomByte(pAd) & 0xf8) << 8;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr45 = %4x\n", Addr45));
				rtmp_ee_flash_write(pAd, 0x08, Addr45);
			}
			rtmp_ee_flash_read(pAd, 0, &i);

			if ((i != 0x2880) && (i != 0x2860)) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_EECMD_Proc: invalid eeprom\n"));
				return FALSE;
			}
		}

		break;

	case 2: {
		USHORT offset, value = 0;
		PUCHAR p;
		p = arg + 2;
		offset = os_str_tol(p, 0, 10);
		p += 2;

		while (*p != '\0') {
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
	rtmp_ee_flash_read(pAd, 0, &eeFlashId);

	if ((pAd->ChipID & 0x0000ffff) == eeFlashId)
		return TRUE;

	return FALSE;
}


static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start)
{
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN bCalFree = 0;
#endif /* CAL_FREE_IC_SUPPORT */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	cap->ee_inited = 1;

	if (validFlashEepromID(pAd) == FALSE) {
		if (rtmp_ee_flash_reset(pAd, start) != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("rtmp_ee_init(): rtmp_ee_flash_init() failed\n"));
			return NDIS_STATUS_FAILURE;
		}

#ifdef CAL_FREE_IC_SUPPORT
		RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

		if (bCalFree) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Cal Free IC!!\n"));
			RTMP_CAL_FREE_DATA_GET(pAd);
		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Non Cal Free IC!!\n"));

#endif /* CAL_FREE_IC_SUPPORT */
		/* Random number for the last bytes of MAC address*/
		{
			USHORT  Addr45;
			rtmp_ee_flash_read(pAd, 0x08, &Addr45);
			Addr45 = Addr45 & 0xff;
			Addr45 = Addr45 | (RandomByte(pAd) & 0xf8) << 8;
			*(UINT16 *)(&pAd->EEPROMImage[0x08]) = le2cpu16(Addr45);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The EEPROM in Flash is wrong, use default\n"));
		}
		/*write back  all to flash*/
		rtmp_ee_flash_write_all(pAd);

		if (validFlashEepromID(pAd) == FALSE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("rtmp_ee_flash_init(): invalid eeprom\n"));
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
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> rtmp_nv_init\n"));
	/*
		pAd->EEPROMImage:
		+----------------------------------------------------------------------------------+
		| 1K (calibration) | 9K (DCOC) | 9K (DPDA Part1) | 9K (DPDA Part2G) | 16K (PreCal) |
		+----------------------------------------------------------------------------------+
	 */
	if (pAd->EEPROMImage)
		os_free_mem(pAd->EEPROMImage);

	os_alloc_mem(pAd, &pAd->EEPROMImage, get_dev_eeprom_size(pAd));
	if (pAd->EEPROMImage) {
		NdisZeroMemory(pAd->EEPROMImage, get_dev_eeprom_size(pAd));
		RtmpFlashRead(pAd->hdev_ctrl, pAd->EEPROMImage, get_dev_eeprom_offset(pAd), get_dev_eeprom_size(pAd));

#ifdef PRE_CAL_MT7622_SUPPORT
		if (IS_MT7622(pAd)) {
			pAd->CalTXLPFGImage = pAd->EEPROMImage + TXLPFG_FLASH_OFFSET;
			pAd->CalTXDCIQImage = pAd->EEPROMImage + TXDCIQ_FLASH_OFFSET;
			pAd->CalTXDPDImage = pAd->EEPROMImage + TXDPD_FLASH_OFFSET;
		}
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_TRX_SET1_SUPPORT
		if (IS_MT7615(pAd)) {
			pAd->CalDCOCImage = pAd->EEPROMImage + DCOC_OFFSET;
			pAd->CalDPDAPart1Image = pAd->EEPROMImage + DPDPART1_OFFSET;
			pAd->CalDPDAPart2Image = pAd->EEPROMImage + DPDPART2_OFFSET;
			pAd->bDPDReloaded = TRUE;
		}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
		pE2pCtrl->e2pCurMode = E2P_FLASH_MODE;
		pE2pCtrl->e2pSource = E2P_SRC_FROM_FLASH;
		return rtmp_ee_flash_init(pAd, pAd->EEPROMImage);
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Allocate %d-byte-of EEPROMImage failed.\n", get_dev_eeprom_size(pAd)));
		return NDIS_STATUS_FAILURE;
	}
}

#endif /* RTMP_FLASH_SUPPORT */

