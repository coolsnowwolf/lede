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
	cmm_single_sku.c
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Single_sku.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#include    "txpwr/single_sku.h"

#ifdef RF_LOCKDOWN
#include    "txpwr/SKUTable_1.h"
#include    "txpwr/SKUTable_2.h"
#include    "txpwr/SKUTable_3.h"
#include    "txpwr/SKUTable_4.h"
#include    "txpwr/SKUTable_5.h"
#include    "txpwr/SKUTable_6.h"
#include    "txpwr/SKUTable_7.h"
#include    "txpwr/SKUTable_8.h"
#include    "txpwr/SKUTable_9.h"
#include    "txpwr/SKUTable_10.h"
#include    "txpwr/SKUTable_11.h"
#include    "txpwr/SKUTable_12.h"
#include    "txpwr/SKUTable_13.h"
#include    "txpwr/SKUTable_14.h"
#include    "txpwr/SKUTable_15.h"
#include    "txpwr/SKUTable_16.h"
#include    "txpwr/SKUTable_17.h"
#include    "txpwr/SKUTable_18.h"
#include    "txpwr/SKUTable_19.h"
#include    "txpwr/SKUTable_20.h"
#include    "txpwr/BFBackoffTable_1.h"
#include    "txpwr/BFBackoffTable_2.h"
#include    "txpwr/BFBackoffTable_3.h"
#include    "txpwr/BFBackoffTable_4.h"
#include    "txpwr/BFBackoffTable_5.h"
#include    "txpwr/BFBackoffTable_6.h"
#include    "txpwr/BFBackoffTable_7.h"
#include    "txpwr/BFBackoffTable_8.h"
#include    "txpwr/BFBackoffTable_9.h"
#include    "txpwr/BFBackoffTable_10.h"
#include    "txpwr/BFBackoffTable_11.h"
#include    "txpwr/BFBackoffTable_12.h"
#include    "txpwr/BFBackoffTable_13.h"
#include    "txpwr/BFBackoffTable_14.h"
#include    "txpwr/BFBackoffTable_15.h"
#include    "txpwr/BFBackoffTable_16.h"
#include    "txpwr/BFBackoffTable_17.h"
#include    "txpwr/BFBackoffTable_18.h"
#include    "txpwr/BFBackoffTable_19.h"
#include    "txpwr/BFBackoffTable_20.h"
#endif /* RF_LOCKDOWN */

extern RTMP_STRING *__rstrtok;

/* TODO: shiang-usw, for MT76x0 series, currently cannot use this function! */
#ifdef COMPOS_WIN

CHAR *os_str_pbrk(CHAR *str1, CHAR  *str2)
{
	const CHAR *x;

	for (; *str1; str1++)
		for (x = str2; *x; x++)
			if (*str1 == *x)
				return (CHAR *) str1;

	return NULL;
}

UINT32 os_str_spn(CHAR *str1, CHAR *str2)
{
	return strspn(str1, str2);
}

ULONG
simple_strtol(
	const RTMP_STRING *szProgID,
	INT EndPtr,
	INT Base
)
{
	ULONG val = 0;
	ANSI_STRING    AS;
	UNICODE_STRING    US;
	RtlInitAnsiString(&AS, szProgID);
	RtlAnsiStringToUnicodeString(&US, &AS, TRUE);
	RtlUnicodeStringToInteger(&US, 0, &val);
	RtlFreeUnicodeString(&US);
	return val;
}

LONG os_str_tol(const CHAR *str, CHAR **endptr, INT base)
{
	return simple_strtol(str, (INT)endptr, base);
}

CHAR *os_str_chr(CHAR *str, INT character)
{
	return strchr(str, character);
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
static RTMP_STRING *__rstrtok;
RTMP_STRING *rstrtok(RTMP_STRING *s, const RTMP_STRING *ct)
{
	RTMP_STRING *sbegin, *send;
	sbegin  = s ? s : __rstrtok;

	if (!sbegin)
		return NULL;

	sbegin += os_str_spn((CHAR *)sbegin, (CHAR *)ct);

	if (*sbegin == '\0') {
		__rstrtok = NULL;
		return NULL;
	}

	send = os_str_pbrk((CHAR *)sbegin, (CHAR *)ct);

	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;
	return sbegin;
}
#endif

#ifdef SINGLE_SKU_V2
#if defined(MT7615) || defined(MT7622)
INT MtSingleSkuLoadParam(RTMP_ADAPTER *pAd)
{
	CHAR *buffer;
	CHAR *readline, *token;
#ifdef RF_LOCKDOWN
#else
	RTMP_OS_FD_EXT srcf;
	INT retval = 0;
#endif /* RF_LOCKDOWN */
	CHAR *ptr;
	INT index, i;
	CH_POWER *StartCh = NULL;
	UCHAR band = 0;
	UCHAR channel, *temp;
	CH_POWER *pwr = NULL;
	UCHAR *sku_path = NULL;

	/* Link list Init */
	DlListInit(&pAd->PwrLimitSkuList);
	/* allocate memory for buffer SKU value */
	os_alloc_mem(pAd, (UCHAR **)&buffer, MAX_INI_BUFFER_SIZE);

	if (!buffer)
		return FALSE;

#ifdef RF_LOCKDOWN
	pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0, 6);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: RF_LOCKDOWN Feature ON !!!\n" KNRM, __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: SKU Table index = %d \n" KNRM, __FUNCTION__,
			 pAd->CommonCfg.SKUTableIdx));
	/* card information file exists so reading the card information */
	os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);

	switch (pAd->CommonCfg.SKUTableIdx) {
	case SKUTABLE_1:
		os_move_mem(buffer, SKUvalue_1, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_2:
		os_move_mem(buffer, SKUvalue_2, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_3:
		os_move_mem(buffer, SKUvalue_3, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_4:
		os_move_mem(buffer, SKUvalue_4, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_5:
		os_move_mem(buffer, SKUvalue_5, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_6:
		os_move_mem(buffer, SKUvalue_6, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_7:
		os_move_mem(buffer, SKUvalue_7, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_8:
		os_move_mem(buffer, SKUvalue_8, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_9:
		os_move_mem(buffer, SKUvalue_9, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_10:
		os_move_mem(buffer, SKUvalue_10, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_11:
		os_move_mem(buffer, SKUvalue_11, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_12:
		os_move_mem(buffer, SKUvalue_12, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_13:
		os_move_mem(buffer, SKUvalue_13, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_14:
		os_move_mem(buffer, SKUvalue_14, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_15:
		os_move_mem(buffer, SKUvalue_15, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_16:
		os_move_mem(buffer, SKUvalue_16, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_17:
		os_move_mem(buffer, SKUvalue_17, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_18:
		os_move_mem(buffer, SKUvalue_18, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_19:
		os_move_mem(buffer, SKUvalue_19, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_20:
		os_move_mem(buffer, SKUvalue_20, MAX_INI_BUFFER_SIZE);;
		break;

	default:
		os_move_mem(buffer, SKUvalue_20, MAX_INI_BUFFER_SIZE);
		break;
	}

#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature OFF !!!\n", __FUNCTION__));
	/* open card information file*/
	sku_path = get_single_sku_path(pAd);
	if (sku_path && *sku_path)
		srcf = os_file_open(sku_path, O_RDONLY, 0);
	else
		srcf.Status = 1;

	if (srcf.Status) {
		/* card information file does not exist */
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("--> Error opening %s\n", sku_path));
		goto  free_resource;
	}

	/* card information file exists so reading the card information */
	os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);
	retval = os_file_read(srcf, buffer, MAX_INI_BUFFER_SIZE);

	if (retval < 0) {
		/* read fail */
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KRED "--> Read %s error %d\n" KNRM, sku_path,
				 -retval));
	} else {
#endif /* RF_LOCKDOWN */
#ifdef RF_LOCKDOWN

	for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\t')) != NULL; readline = ptr + 1, index++)
#else
	for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\n')) != NULL; readline = ptr + 1, index++)
#endif /* RF_LOCKDOWN */
	{
		*ptr = '\0';
#ifdef RF_LOCKDOWN

		if (readline[0] == '!')
			continue;

#else

		if (readline[0] == '#')
			continue;

#endif /* RF_LOCKDOWN */

		/* Band Info Parsing */
		if (!strncmp(readline, "Band: ", 6)) {
			token = rstrtok(readline + 6, " ");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			band = (UCHAR)os_str_tol(token, 0, 10);

			if (band == 2)
				band = 0;
			else if (band == 5)
				band = 1;

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("band = %d\n", band));
		}

		/* Rate Info Parsing for each channel */
		if (!strncmp(readline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UCHAR **)&pwr, sizeof(*pwr));
			/* set default value to 0 for parsing structure */
			os_zero_mem(pwr, sizeof(*pwr));
			token = rstrtok(readline + 2, " ");

			/* sanity check for non-Null pointer */
			if (!token) {
				/* free memory buffer before escape this loop */
				os_free_mem(pwr);
				/* escape this loop for Null pointer */
				continue;
			}

			channel = (UCHAR)os_str_tol(token, 0, 10);
			pwr->StartChannel = channel;
			pwr->band = band;

			/* Rate Info Parsing (CCK) */
			if (band == 0) {
				for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++) {
					token = rstrtok(NULL, " ");

					/* sanity check for non-Null pointer */
					if (!token)
						break;

					/* config CCK Power Limit */
					MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitCCK + i, token);
				}
			}

			/* Rate Info Parsing (OFDM) */
			for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config ofdm Power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitOFDM + i, token);
			}

#ifdef DOT11_VHT_AC

			/* Rate Info Parsing (VHT20) */
			for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config vht20 Power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT20 + i, token);
			}

			/* Rate Info Parsing (VHT40) */
			for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config vht40 Power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT40 + i, token);
			}

			/* if (pwr->StartChannel > 14) */
			if (band == 1) {
				/* Rate Info Parsing (VHT80) */
				for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
					token = rstrtok(NULL, " ");

					/* sanity check for non-Null pointer */
					if (!token)
						break;

					/* config vht80 Power Limit */
					MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT80 + i, token);
				}

				/* Rate Info Parsing (VHT160) */
				for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
					token = rstrtok(NULL, " ");

					/* sanity check for non-Null pointer */
					if (!token)
						break;

					/* config vht160 Power Limit */
					MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT160 + i, token);
				}
			}

#endif /* DOT11_VHT_AC */

			/* Tx Stream offset Info Parsing */
			for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* parsing order is 3T, 2T, 1T */
				pwr->u1PwrLimitTxStreamDelta[i] = os_str_tol(token, 0, 10) * 2;
			}

			/* Tx Spatial Stream offset Info Parsing */
			for (i = 0; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* parsing order is 1SS, 2SS, 3SS, 4SS */
				pwr->u1PwrLimitTxNSSDelta[i] = os_str_tol(token, 0, 10) * 2;
			}

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same rate Info, band Info, Tx Stream offset Info, Tx Spatial stream offset Info) */
			if (!StartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this channel */
				StartCh = pwr;
				/* add tail for Link list */
				DlListAddTail(&pAd->PwrLimitSkuList, &pwr->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				/* if (pwr->StartChannel <= 14) */
				if (band == 0) {
					for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++) {
						if (StartCh->u1PwrLimitCCK[i] != pwr->u1PwrLimitCCK[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++) {
						if (StartCh->u1PwrLimitOFDM[i] != pwr->u1PwrLimitOFDM[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT20[i] != pwr->u1PwrLimitVHT20[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT40[i] != pwr->u1PwrLimitVHT40[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT80[i] != pwr->u1PwrLimitVHT80[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT160[i] != pwr->u1PwrLimitVHT160[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++) {
						if (StartCh->u1PwrLimitTxStreamDelta[i] != pwr->u1PwrLimitTxStreamDelta[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; i++) {
						if (StartCh->u1PwrLimitTxNSSDelta[i] != pwr->u1PwrLimitTxNSSDelta[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					if (StartCh->band != pwr->band)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different channel */
				if (fgSameCont)
					os_free_mem(pwr);
				else {
					/* Assign new pointer head to SKU table contents for this channel */
					StartCh = pwr;
					/* add tail for Link list */
					DlListAddTail(&pAd->PwrLimitSkuList, &StartCh->List);
				}
			}

			/* Increment total channel counts for channels with same SKU table contents */
			StartCh->num++;
			/* allocate memory for channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUCHAR *)&temp, StartCh->num);

			/* backup non-empty channel list to temp buffer */
			if (NULL != StartCh->Channel) {
				/* copy channel list to temp buffer */
				os_move_mem(temp, StartCh->Channel, StartCh->num - 1);
				/* free memory for channel list used before assign pointer of temp memory buffer */
				os_free_mem(StartCh->Channel);
			}

			/* assign pointer of temp memory buffer */
			StartCh->Channel = temp;
			/* update latest channel number to channel list */
			StartCh->Channel[StartCh->num - 1] = channel;
		}
	}

#ifdef RF_LOCKDOWN
#else
}

#endif /* RF_LOCKDOWN */

	/* print out Sku table info */
	MtShowSkuTable(pAd, DBG_LVL_INFO);

#ifdef RF_LOCKDOWN
#else
	/* close file*/
	retval = os_file_close(srcf);
free_resource:
#endif /* RF_LOCKDOWN */
	os_free_mem(buffer);
	return TRUE;
}

VOID MtSingleSkuUnloadParam(RTMP_ADAPTER *pAd)
{
	CH_POWER *ch, *ch_temp;
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitSkuList, CH_POWER, List) {
		DlListDel(&ch->List);

		/* free memory for channel list with same table contents */
		os_free_mem(ch->Channel);

		/* free memory for table contents*/
		os_free_mem(ch);
	}
}

INT MtBfBackOffLoadParam(RTMP_ADAPTER *pAd)
{
	CHAR *buffer;
	CHAR *readline, *token;
#ifdef RF_LOCKDOWN
#else
	RTMP_OS_FD_EXT srcf;
	INT retval = 0;
#endif /* RF_LOCKDOWN */
	CHAR *ptr;
	INT index, i;
	BACKOFF_POWER *StartCh = NULL;
	UCHAR band = 0;
	UCHAR channel, *temp;
	BACKOFF_POWER *pwr = NULL;
	BACKOFF_POWER *ch, *ch_temp;
	UCHAR *sku_path = NULL;

	DlListInit(&pAd->PwrLimitBackoffList);
	/* init*/
	os_alloc_mem(pAd, (UCHAR **)&buffer, MAX_INI_BUFFER_SIZE);

	if (buffer == NULL)
		return FALSE;

#ifdef RF_LOCKDOWN
	pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0, 6);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature ON !!!\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BFBackoff Table index = %d \n", __FUNCTION__,
			 pAd->CommonCfg.SKUTableIdx));
	/* card information file exists so reading the card information */
	os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);

	switch (pAd->CommonCfg.SKUTableIdx) {
	case SKUTABLE_1:
		os_move_mem(buffer, BFBackoffvalue_1, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_2:
		os_move_mem(buffer, BFBackoffvalue_2, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_3:
		os_move_mem(buffer, BFBackoffvalue_3, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_4:
		os_move_mem(buffer, BFBackoffvalue_4, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_5:
		os_move_mem(buffer, BFBackoffvalue_5, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_6:
		os_move_mem(buffer, BFBackoffvalue_6, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_7:
		os_move_mem(buffer, BFBackoffvalue_7, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_8:
		os_move_mem(buffer, BFBackoffvalue_8, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_9:
		os_move_mem(buffer, BFBackoffvalue_9, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_10:
		os_move_mem(buffer, BFBackoffvalue_10, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_11:
		os_move_mem(buffer, BFBackoffvalue_11, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_12:
		os_move_mem(buffer, BFBackoffvalue_12, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_13:
		os_move_mem(buffer, BFBackoffvalue_13, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_14:
		os_move_mem(buffer, BFBackoffvalue_14, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_15:
		os_move_mem(buffer, BFBackoffvalue_15, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_16:
		os_move_mem(buffer, BFBackoffvalue_16, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_17:
		os_move_mem(buffer, BFBackoffvalue_17, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_18:
		os_move_mem(buffer, BFBackoffvalue_18, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_19:
		os_move_mem(buffer, BFBackoffvalue_19, MAX_INI_BUFFER_SIZE);
		break;

	case SKUTABLE_20:
		os_move_mem(buffer, BFBackoffvalue_20, MAX_INI_BUFFER_SIZE);
		break;

	default:
		os_move_mem(buffer, SKUvalue_20, MAX_INI_BUFFER_SIZE);
		break;
	}

#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature OFF !!!\n", __FUNCTION__));
	/* open card information file*/
	sku_path = get_single_sku_path(pAd);
	if (sku_path && *sku_path)
		srcf = os_file_open(sku_path, O_RDONLY, 0);
	else
		srcf.Status = 1;

	if (srcf.Status) {
		/* card information file does not exist */
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--> Error opening %s\n", sku_path));
		goto  free_resource;
	}

	/* card information file exists so reading the card information */
	os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);
	retval = os_file_read(srcf, buffer, MAX_INI_BUFFER_SIZE);

	if (retval < 0) {
		/* read fail */
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--> Read %s error %d\n", sku_path, -retval));
	} else {
#endif /* RF_LOCKDOWN */
#ifdef RF_LOCKDOWN

	for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\t')) != NULL; readline = ptr + 1, index++)
#else
	for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\n')) != NULL; readline = ptr + 1, index++)
#endif /* RF_LOCKDOWN */
	{
		*ptr = '\0';
#ifdef RF_LOCKDOWN

		if (readline[0] == '!')
			continue;

#else

		if (readline[0] == '#')
			continue;

#endif /* RF_LOCKDOWN */

		/* Band Info Parsing */
		if (!strncmp(readline, "Band: ", 6)) {
			token = rstrtok(readline + 6, " ");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			band = (UCHAR)os_str_tol(token, 0, 10);

			if (band == 2)
				band = 0;
			else if (band == 5)
				band = 1;

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("band = %d\n", band));
		}

		/* BF Backoff Info Parsing for each channel */
		if (!strncmp(readline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UCHAR **)&pwr, sizeof(*pwr));
			/* set default value to 0 for parsing structure */
			os_zero_mem(pwr, sizeof(*pwr));
			token = rstrtok(readline + 2, " ");

			/* sanity check for non-Null pointer */
			if (!token) {
				/* free memory buffer before escape this loop */
				os_free_mem(pwr);
				/* escape this loop for Null pointer */
				continue;
			}

			channel = (UCHAR)os_str_tol(token, 0, 10);
			pwr->StartChannel = channel;
			pwr->band = band;

			/* BF Backoff Info Parsing */
			for (i = 0; i < 3; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config bf power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->PwrMax + i, token);
			}

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same BF Backoff Info) */
			if (!StartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this channel */
				StartCh = pwr;
				/* add tail for Link list */
				DlListAddTail(&pAd->PwrLimitBackoffList, &pwr->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				if (fgSameCont) {
					for (i = 0; i < 3; i++) {
						if (StartCh->PwrMax[i] != pwr->PwrMax[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					if (StartCh->band != pwr->band)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different channel */
				if (fgSameCont)
					os_free_mem(pwr);
				else {
					/* Assign new pointer head to SKU table contents for this channel */
					StartCh = pwr;
					/* add tail for Link list */
					DlListAddTail(&pAd->PwrLimitBackoffList, &StartCh->List);
				}
			}

			/* Increment total channel counts for channels with same SKU table contents */
			StartCh->num++;
			/* allocate memory for channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUCHAR *)&temp, StartCh->num);

			/* backup non-empty channel list to temp buffer */
			if (StartCh->Channel != NULL) {
				/* copy channel list to temp buffer */
				os_move_mem(temp, StartCh->Channel, StartCh->num - 1);
				/* free memory for channel list used before assign pointer of temp memory buffer */
				os_free_mem(StartCh->Channel);
			}

			/* assign pointer of temp memory buffer */
			StartCh->Channel = temp;
			/* update latest channel number to channel list */
			StartCh->Channel[StartCh->num - 1] = channel;
		}
	}

#ifdef RF_LOCKDOWN
#else
}

#endif /* RF_LOCKDOWN */
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		int i;
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Band: %d \n", ch->band));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Channel: "));

		for (i = 0; i < ch->num; i++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%d ", ch->Channel[i]));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n"));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Max Power: "));

		for (i = 0; i < 3; i++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%d ", ch->PwrMax[i]));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n"));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-----------------------------------------------------------------\n"));
	}
#ifdef RF_LOCKDOWN
#else
	/* close file*/
	retval = os_file_close(srcf);
free_resource:
#endif /* RF_LOCKDOWN */
	os_free_mem(buffer);
	return TRUE;
}

VOID MtBfBackOffUnloadParam(RTMP_ADAPTER *pAd)
{
	BACKOFF_POWER *ch, *ch_temp;
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		DlListDel(&ch->List);

		/* free memory for channel list with same table contents */
		os_free_mem(ch->Channel);

		/* free memory for table contents*/
		os_free_mem(ch);
	}
}

VOID MtFillSkuParam(RTMP_ADAPTER *pAd, UINT8 channel, UCHAR Band, UCHAR TxStream, UINT8 *txPowerSku)
{
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UINT8 i, j;
	UINT8 TxOffset = 0;
	UCHAR band_local = 0;

	/* -----------------------------------------------------------------------------------------------------------------------*/
	/* This part is due to MtCmdChannelSwitch is not ready for 802.11j and variable channel_band is always 0				  */
	/* -----------------------------------------------------------------------------------------------------------------------*/

	if (channel >= 16) /* must be 5G */
		band_local = 1;
	else if ((channel <= 14) && (channel >= 8)) /* depends on "channel_band" in MtCmdChannelSwitch */
		band_local = Band;
	else if (channel <= 8) /* must be 2.4G */
		band_local = 0;

	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitSkuList, CH_POWER, List) {
		start_ch = ch->StartChannel;
		/* if (channel >= start_ch) */
		/* { */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: channel = %d, start_ch = %d , Band = %d\n", __func__, channel,
				 start_ch, Band));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: ch->num = %d\n", __func__, ch->num));

		for (j = 0; j < ch->num; j++) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: In for loop, channel = %d, ch->Channel[%d] = %d\n", __func__,
					 channel, j, ch->Channel[j]));

			if (Band == ch->band) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Band check, ch->Channel[%d] = %d\n", __func__, j,
						 ch->Channel[j]));

				if (channel == ch->Channel[j]) {
					for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CCK[%d]: 0x%x\n", i, ch->u1PwrLimitCCK[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("OFDM[%d]: 0x%x\n", i, ch->u1PwrLimitOFDM[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT20[%d]: 0x%x\n", i, ch->u1PwrLimitVHT20[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT40[%d]: 0x%x\n", i, ch->u1PwrLimitVHT40[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT80[%d]: 0x%x\n", i, ch->u1PwrLimitVHT80[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT160[%d]: 0x%x\n", i, ch->u1PwrLimitVHT160[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxStreamDelta(%dT): 0x%x (ref to 4T)\n", (3 - i),
								 ch->u1PwrLimitTxStreamDelta[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxNSSDelta(%dSS): 0x%x (ref to 4SS)\n", i, ch->u1PwrLimitTxNSSDelta[i]));

					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxStream = %d\n", TxStream));

					/* check the TxStream 1T/2T/3T/4T*/
					if (TxStream == 1) {
						TxOffset = ch->u1PwrLimitTxStreamDelta[2];
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ch->u1PwrLimitTxStreamDelta[2] = %d\n", ch->u1PwrLimitTxStreamDelta[2]));
					} else if (TxStream == 2) {
						TxOffset = ch->u1PwrLimitTxStreamDelta[1];
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ch->u1PwrLimitTxStreamDelta[1] = %d\n", ch->u1PwrLimitTxStreamDelta[1]));
					} else if (TxStream == 3) {
						TxOffset = ch->u1PwrLimitTxStreamDelta[0];
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ch->u1PwrLimitTxStreamDelta[0] = %d\n", ch->u1PwrLimitTxStreamDelta[0]));
					} else if (TxStream == 4)
						TxOffset = 0;
					else
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("The TxStream value is invalid.\n"));

					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxOffset = %d\n", TxOffset));
					/* Fill in the SKU table for destination channel*/
					txPowerSku[SKU_CCK_1_2]	   = ch->u1PwrLimitCCK[0]	?  (ch->u1PwrLimitCCK[0]	+ TxOffset) : 0x3F;
					txPowerSku[SKU_CCK_55_11]	   = ch->u1PwrLimitCCK[1]	?  (ch->u1PwrLimitCCK[1]	+ TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_6_9]	   = ch->u1PwrLimitOFDM[0]   ?  (ch->u1PwrLimitOFDM[0]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_12_18]	 = ch->u1PwrLimitOFDM[1]   ?  (ch->u1PwrLimitOFDM[1]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_24_36]	 = ch->u1PwrLimitOFDM[2]   ?  (ch->u1PwrLimitOFDM[2]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_48]		= ch->u1PwrLimitOFDM[3]   ?  (ch->u1PwrLimitOFDM[3]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_54]		= ch->u1PwrLimitOFDM[4]   ?  (ch->u1PwrLimitOFDM[4]   + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_0_8]	   = ch->u1PwrLimitVHT20[0]  ?  (ch->u1PwrLimitVHT20[0]  + TxOffset) : 0x3F;
					/*MCS32 is a special rate will chose the max power, normally will be OFDM 6M */
					txPowerSku[SKU_HT20_32]	   =  ch->u1PwrLimitOFDM[0]  ?  (ch->u1PwrLimitOFDM[0]   + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_1_2_9_10]  = ch->u1PwrLimitVHT20[1]  ?  (ch->u1PwrLimitVHT20[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_3_4_11_12] = ch->u1PwrLimitVHT20[2]  ?  (ch->u1PwrLimitVHT20[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_5_13]	   = ch->u1PwrLimitVHT20[3]  ?  (ch->u1PwrLimitVHT20[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_6_14]	   = ch->u1PwrLimitVHT20[3]  ?  (ch->u1PwrLimitVHT20[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_7_15]	   = ch->u1PwrLimitVHT20[4]  ?  (ch->u1PwrLimitVHT20[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_0_8]	   = ch->u1PwrLimitVHT40[0]  ?  (ch->u1PwrLimitVHT40[0]  + TxOffset) : 0x3F;
					/*MCS32 is a special rate will chose the max power, normally will be OFDM 6M */
					txPowerSku[SKU_HT40_32]	   =  ch->u1PwrLimitOFDM[0]  ?  (ch->u1PwrLimitOFDM[0]   + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_1_2_9_10]  = ch->u1PwrLimitVHT40[1]  ?  (ch->u1PwrLimitVHT40[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_3_4_11_12] = ch->u1PwrLimitVHT40[2]  ?  (ch->u1PwrLimitVHT40[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_5_13]	   = ch->u1PwrLimitVHT40[3]  ?  (ch->u1PwrLimitVHT40[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_6_14]	   = ch->u1PwrLimitVHT40[3]  ?  (ch->u1PwrLimitVHT40[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_7_15]	   = ch->u1PwrLimitVHT40[4]  ?  (ch->u1PwrLimitVHT40[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_0]	   = ch->u1PwrLimitVHT20[0]  ?  (ch->u1PwrLimitVHT20[0]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_1_2]	   = ch->u1PwrLimitVHT20[1]  ?  (ch->u1PwrLimitVHT20[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_3_4]	   = ch->u1PwrLimitVHT20[2]  ?  (ch->u1PwrLimitVHT20[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_5_6]	   = ch->u1PwrLimitVHT20[3]  ?  (ch->u1PwrLimitVHT20[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_7]	   = ch->u1PwrLimitVHT20[4]  ?  (ch->u1PwrLimitVHT20[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_8]	   = ch->u1PwrLimitVHT20[5]  ?  (ch->u1PwrLimitVHT20[5]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_9]	   = ch->u1PwrLimitVHT20[6]  ?  (ch->u1PwrLimitVHT20[6]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_0]	   = ch->u1PwrLimitVHT40[0]  ?  (ch->u1PwrLimitVHT40[0]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_1_2]	   = ch->u1PwrLimitVHT40[1]  ?  (ch->u1PwrLimitVHT40[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_3_4]	   = ch->u1PwrLimitVHT40[2]  ?  (ch->u1PwrLimitVHT40[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_5_6]	   = ch->u1PwrLimitVHT40[3]  ?  (ch->u1PwrLimitVHT40[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_7]	   = ch->u1PwrLimitVHT40[4]  ?  (ch->u1PwrLimitVHT40[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_8]	   = ch->u1PwrLimitVHT40[5]  ?  (ch->u1PwrLimitVHT40[5]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_9]	   = ch->u1PwrLimitVHT40[6]  ?  (ch->u1PwrLimitVHT40[6]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_0]	   = ch->u1PwrLimitVHT80[0]  ?  (ch->u1PwrLimitVHT80[0]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_1_2]	   = ch->u1PwrLimitVHT80[1]  ?  (ch->u1PwrLimitVHT80[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_3_4]	   = ch->u1PwrLimitVHT80[2]  ?  (ch->u1PwrLimitVHT80[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_5_6]	   = ch->u1PwrLimitVHT80[3]  ?  (ch->u1PwrLimitVHT80[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_7]	   = ch->u1PwrLimitVHT80[4]  ?  (ch->u1PwrLimitVHT80[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_8]	   = ch->u1PwrLimitVHT80[5]  ?  (ch->u1PwrLimitVHT80[5]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_9]	   = ch->u1PwrLimitVHT80[6]  ?  (ch->u1PwrLimitVHT80[6]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_0]	   = ch->u1PwrLimitVHT160[0] ?  (ch->u1PwrLimitVHT160[0] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_1_2]	   = ch->u1PwrLimitVHT160[1] ?  (ch->u1PwrLimitVHT160[1] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_3_4]	   = ch->u1PwrLimitVHT160[2] ?  (ch->u1PwrLimitVHT160[2] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_5_6]	   = ch->u1PwrLimitVHT160[3] ?  (ch->u1PwrLimitVHT160[3] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_7]	   = ch->u1PwrLimitVHT160[4] ?  (ch->u1PwrLimitVHT160[4] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_8]	   = ch->u1PwrLimitVHT160[5] ?  (ch->u1PwrLimitVHT160[5] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_9]	   = ch->u1PwrLimitVHT160[6] ?  (ch->u1PwrLimitVHT160[6] + TxOffset) : 0x3F;
					txPowerSku[SKU_1SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[0] ?  ch->u1PwrLimitTxNSSDelta[0] : 0x0;
					txPowerSku[SKU_2SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[1] ?  ch->u1PwrLimitTxNSSDelta[1] : 0x0;
					txPowerSku[SKU_3SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[2] ?  ch->u1PwrLimitTxNSSDelta[2] : 0x0;
					txPowerSku[SKU_4SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[3] ?  ch->u1PwrLimitTxNSSDelta[3] : 0x0;

					for (i = 0; i < SKU_TOTAL_SIZE; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("txPowerSku[%d]: 0x%x\n", i, txPowerSku[i]));

					break;
				}
			}
		}

		/* } */
	}
}

VOID MtFillBackoffParam(RTMP_ADAPTER *pAd, UINT8 channel, UCHAR Band, UINT8 *BFPowerBackOff)
{
	BACKOFF_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UINT8 i, j;
	UCHAR band_local = 0;

	/* -----------------------------------------------------------------------------------------------------------------------*/
	/* This part is due to MtCmdChannelSwitch is not ready for 802.11j and variable channel_band is always 0				  */
	/* -----------------------------------------------------------------------------------------------------------------------*/

	if (channel >= 16) /* must be 5G */
		band_local = 1;
	else if ((channel <= 14) && (channel >= 8)) /* depends on "channel_band" in MtCmdChannelSwitch */
		band_local = Band;
	else if (channel <= 8) /* must be 2.4G */
		band_local = 0;

	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		start_ch = ch->StartChannel;
		/* if (channel >= start_ch) */
		/* { */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: channel = %d, start_ch = %d , Band = %d\n", __func__, channel,
				 start_ch, Band));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: ch->num = %d\n", __func__, ch->num));

		for (j = 0; j < ch->num; j++) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: In for loop, channel = %d, ch->Channel[%d] = %d\n", __func__,
					 channel, j, ch->Channel[j]));

			if (Band == ch->band) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Band check, ch->Channel[%d] = %d\n", __func__, j,
						 ch->Channel[j]));

				if (channel == ch->Channel[j]) {
					for (i = 0; i < 3; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Max Power[%d]: 0x%x\n", i, ch->PwrMax[i]));

					/* Fill in the SKU table for destination channel*/
					BFPowerBackOff[0] = ch->PwrMax[0] ? (ch->PwrMax[0]) : 0x3F;
					BFPowerBackOff[1] = ch->PwrMax[1] ? (ch->PwrMax[1]) : 0x3F;
					BFPowerBackOff[2] = ch->PwrMax[2] ? (ch->PwrMax[2]) : 0x3F;

					for (i = 0; i < 3; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("BFPowerBackOff[%d]: 0x%x\n", i, BFPowerBackOff[i]));

					break;
				}
			}
		}

		/* } */
	}
}

VOID MtShowSkuTable(RTMP_ADAPTER *pAd, UINT8 u1DebugLevel)
	{
		UINT8 u1ColIdx;
		P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("SKU table index: %d \n", pAd->CommonCfg.SKUTableIdx));

		DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, &pAd->PwrLimitSkuList, CH_POWER, List) {
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("start channel: %d, ChListNum: %d\n", prPwrLimitTbl->StartChannel, prPwrLimitTbl->num));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Band: %d \n", prPwrLimitTbl->band));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Channel: "));
			for (u1ColIdx = 0; u1ColIdx < prPwrLimitTbl->num; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->Channel[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("CCK: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_CCK_LENGTH; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitCCK[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("OFDM: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_OFDM_LENGTH; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitOFDM[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT20: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT20[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT40: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT40[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT80: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT80[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT160: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT160[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("TxStreamDelta: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_TX_OFFSET_NUM; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitTxStreamDelta[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("TxNSSDelta: "));
			for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitTxNSSDelta[u1ColIdx]));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));
		}

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
	}


#else
INT MtPwrLimitLoadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	PCHAR pi1Buffer;
	BOOLEAN fgReadTbl;
	PDL_LIST pList;

	/* get pointer of link list address */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		pList = &pAd->PwrLimitSkuList;
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		pList = &pAd->PwrLimitBackoffList;

	/* Link list Init */
	DlListInit(&pAd->pList);

	/* allocate memory for buffer SKU value */
	os_alloc_mem(pAd, (UCHAR **)&pi1Buffer, MAX_INI_BUFFER_SIZE);

	if (!pi1Buffer)
		return FALSE;

	/* update buffer with sku table content */
	fgReadTbl = MtReadPwrLimitTable(pAd, pi1Buffer, u1Type);

	if (!fgReadTbl)
		goto error;

	/* parsing sku table contents from buffer */
	MtParsePwrLimitTable(pAd, pi1Buffer, u1Type);

	/* print out Sku table info */
	MtShowPwrLimitTable(pAd, u1Type, DBG_LVL_TRACE);

	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return TRUE;

error:
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return FALSE;
}

VOID MtPwrLimitUnloadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;
	PDL_LIST pList;

	/* get pointer of link list address */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		pList = &pAd->PwrLimitSkuList;
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		pList = &pAd->PwrLimitBackoffList;

	/* free allocated memory for power limit table */
	if (pAd->fgPwrLimitRead[u1Type]) {
		DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER, List) {

			/* delete this element link to next element */
			DlListDel(&prPwrLimitTbl->List);

			/* free memory for channel list with same table contents */
			os_free_mem(prPwrLimitTbl->pu1ChList);

			/* free memory for power limit parameters */
			os_free_mem(prPwrLimitTbl->pu1PwrLimit);

			/* free memory for table contents*/
			os_free_mem(prPwrLimitTbl);
		}
	}

	/* disable flag for Read Power limit table pass */
	pAd->fgPwrLimitRead[u1Type] = FALSE;
}

VOID MtParsePwrLimitTable(RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type)
{
	UINT8 u1ChBand;
	PCHAR pcReadline, pcToken, pcptr;
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	UINT8 u1Channel;
	PUINT8 prTempChList;
	UINT8 u1PwrLimitParamNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_PARAM_NUM, BACKOFF_PARAM_NUM};

	P_CH_POWER prTbl = NULL, prStartCh = NULL;

	for (pcReadline = pcptr = pi1Buffer; (pcptr = os_str_chr(pcReadline, '\t')) != NULL; pcReadline = pcptr + 1) {
		*pcptr = '\0';

		/* Skip Phy mode notation cloumn line */
		if (pcReadline[0] == '#')
			continue;

		/* Channel Band Info Parsing */
		if (!strncmp(pcReadline, "Band: ", 6)) {
			pcToken = rstrtok(pcReadline + 6, " ");

			/* sanity check for non-Null pointer */
			if (!pcToken)
				continue;

			u1ChBand = (UCHAR)os_str_tol(pcToken, 0, 10);

			switch (u1ChBand) {
			case 2:
				u1ChBand = G_BAND;
				break;
			case 5:
				u1ChBand = A_BAND;
				break;
			default:
				break;
			}

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ChBand: %s\n", (G_BAND == u1ChBand) ? "G_BAND" : "A_BAND"));
		}

		/* Rate Info Parsing for each u1Channel */
		if (!strncmp(pcReadline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UCHAR **)&prTbl, sizeof(CH_POWER));
			/* set default value to 0 for parsing structure */
			os_zero_mem(prTbl, sizeof(CH_POWER));

			/* Dynamic allocate memory for parsing structure power limit paramters */
			os_alloc_mem(pAd, (UCHAR **)&prTbl->pu1PwrLimit, u1PwrLimitParamNum[u1Type]);
			/* set default value to 0 for parsing structure */
			os_zero_mem(prTbl->pu1PwrLimit, u1PwrLimitParamNum[u1Type]);

			/*  */
			pcToken = rstrtok(pcReadline + 2, " ");

			/* sanity check for non-Null pointer */
			if (!pcToken) {
				/* free memory buffer of power limit parameters before escape this loop */
				os_free_mem(prTbl->pu1PwrLimit);
				/* free total memory buffer before escape this loop */
				os_free_mem(prTbl);
				/* escape this loop for Null pointer */
				continue;
			}

			u1Channel = (UCHAR)os_str_tol(pcToken, 0, 10);
			prTbl->u1StartChannel = u1Channel;
			prTbl->u1ChBand = u1ChBand;

			/* Rate Info Parsing (CCK, OFDM, VHT20/40/80/160) */
			MtPwrLimitParse(prTbl->pu1PwrLimit, u1ChBand, u1Type);

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same Rate Power Limit Info, Band Info) */
			if (!prStartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this u1Channel */
				prStartCh = prTbl;
				/* add tail for Link list */
				DlListAddTail(&pAd->PwrLimitSkuList, &prTbl->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
				fgSameCont = MtPwrLimitSimilarCheck(prStartCh->pu1PwrLimit, prTbl->pu1PwrLimit, u1ChBand, u1Type);

				/* check if different info contents for different channel (channel band) */
				if (fgSameCont) {
					if (prStartCh->u1ChBand != prTbl->u1ChBand)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different u1Channel */
				if (fgSameCont) {
					os_free_mem(prTbl->pu1PwrLimit);
					os_free_mem(prTbl);
				} else {
					/* Assign new pointer head to SKU table contents for this u1Channel */
					prStartCh = prTbl;
					/* add tail for Link list */
					DlListAddTail(&pAd->PwrLimitSkuList, &prStartCh->List);
				}
			}

			/* Increment total u1Channel counts for channels with same SKU table contents */
			prStartCh->u1ChNum++;
			/* allocate memory for u1Channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUCHAR *)&prTempChList, prStartCh->u1ChNum);

			/* backup non-empty u1Channel list to prTempChList buffer */
			if (prStartCh->pu1ChList) {
				/* copy u1Channel list to prTempChList buffer */
				os_move_mem(prTempChList, prStartCh->pu1ChList, prStartCh->u1ChNum - 1);
				/* free memory for u1Channel list used before assign pointer of prTempChList memory buffer */
				os_free_mem(prStartCh->pu1ChList);
			}

			/* assign pointer of prTempChList memory buffer */
			prStartCh->pu1ChList = prTempChList;
			/* update latest u1Channel number to u1Channel list */
			prStartCh->pu1ChList[prStartCh->u1ChNum - 1] = u1Channel;
		}
	}
}

BOOLEAN MtReadPwrLimitTable(RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type)
{
	RTMP_OS_FD_EXT srcfile;
	PUCHAR pcptrSkuTbl[TABLE_SIZE] = {SKUvalue_1,  SKUvalue_2,  SKUvalue_3,  SKUvalue_4,  SKUvalue_5,
									  SKUvalue_6,  SKUvalue_7,  SKUvalue_8,  SKUvalue_9,  SKUvalue_10,
									  SKUvalue_11, SKUvalue_12, SKUvalue_13, SKUvalue_14, SKUvalue_15,
									  SKUvalue_16, SKUvalue_17, SKUvalue_18, SKUvalue_19, SKUvalue_20};

	PUCHAR pcptrBackoffTbl[TABLE_SIZE] = {BFBackoffvalue_1,  BFBackoffvalue_2,  BFBackoffvalue_3,  BFBackoffvalue_4,  BFBackoffvalue_5,
										  BFBackoffvalue_6,  BFBackoffvalue_7,  BFBackoffvalue_8,  BFBackoffvalue_9,  BFBackoffvalue_10,
										  BFBackoffvalue_11, BFBackoffvalue_12, BFBackoffvalue_13, BFBackoffvalue_14, BFBackoffvalue_15,
										  BFBackoffvalue_16, BFBackoffvalue_17, BFBackoffvalue_18, BFBackoffvalue_19, BFBackoffvalue_20};

	UCHAR *sku_path = NULL;

#ifdef RF_LOCKDOWN
	pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0, 6);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: RF_LOCKDOWN Feature ON !!!\n" KNRM, __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: SKU Table index: %d\n" KNRM, __FUNCTION__, pAd->CommonCfg.SKUTableIdx));

	/* init bufer for Sku table */
	os_zero_mem(pi1Buffer, MAX_INI_BUFFER_SIZE);

	/* update buffer with sku table content */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		os_move_mem(pi1Buffer, pcptrSkuTbl[pAd->CommonCfg.SKUTableIdx], MAX_INI_BUFFER_SIZE);
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		os_move_mem(pi1Buffer, pcptrBackoffTbl[pAd->CommonCfg.SKUTableIdx], MAX_INI_BUFFER_SIZE);
#else

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature OFF !!!\n", __FUNCTION__));

	/* open Sku table file */
	if (u1Type == POWER_LIMIT_TABLE_TYPE_SKU)
		sku_path = get_single_sku_path(pAd);
	else if (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF)
		sku_path = get_bf_sku_path(pAd);

	if (sku_path && *sku_path)
		srcfile = os_file_open(sku_path, O_RDONLY, 0);
	else
		srcfile.Status = 1;

	if (srcfile.Status) {
		/* card information file does not exist */
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--> Error opening file <%s>\n", sku_path));
		return FALSE;
	}

	/* Read Sku table file */
	os_zero_mem(pi1Buffer, MAX_INI_BUFFER_SIZE);

	if (os_file_read(srcfile, pi1Buffer, MAX_INI_BUFFER_SIZE) < 0) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KRED "--> Error read <%s>\n" KNRM, sku_path));
		return FALSE;
	}

	/* close Sku table file */
	if (os_file_close(srcfile) < 0) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KRED "--> Error close <%s>\n" KNRM, sku_path));
		return FALSE;
	}
#endif

	return TRUE;
}

VOID MtPwrFillLimitParam(RTMP_ADAPTER *pAd, UINT8 u1Channel, UINT8 ChBand, PINT8 pi1PwrLimitParam, UINT8 u1Type)
{
	UINT8 u1RateIdx, u1FillParamType, u1ParamIdx, u1ParamIdx2, u1ChListIdx;
	PUINT8 pu1FillParamTypeLen = NULL;
	P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;
	UINT8 u1TypeFillNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_NUM, BACKOFF_TYPE_NUM};

	/* get pointer of parameter type length array and Need Parse check */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		pu1FillParamTypeLen = pAd->u1SkuFillParamLen;
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		pu1FillParamTypeLen = pAd->u1BackoffFillParamLen;
	else
		return;

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, &pAd->PwrLimitSkuList, CH_POWER, List) {
		/* search for specific channel */
		for (u1ChListIdx = 0; u1ChListIdx < prPwrLimitTbl->u1ChNum; u1ChListIdx++) {
			/* check Channel Band and Channel */
			if ((ChBand == prPwrLimitTbl->u1ChBand) && (u1Channel == prPwrLimitTbl->pu1ChList[u1ChListIdx])) {

				/* Fill in Sku parameter for CCK, OFDM, HT20/40, VHT20/40/80/160 */
				for (u1FillParamType = 0, u1ParamIdx = 0, u1ParamIdx2 = 0; u1FillParamType < u1TypeFillNum[u1Type]; u1FillParamType++, u1ParamIdx += *(pu1FillParamTypeLen + u1FillParamType)) {
					for (u1RateIdx = 0; u1RateIdx < *(pu1FillParamTypeLen + u1FillParamType); u1RateIdx++) {
						*(pi1PwrLimitParam + u1RateIdx + u1ParamIdx) = (prPwrLimitTbl->pu1PwrLimit + u1RateIdx + u1ParamIdx2) ? : (prPwrLimitTbl->pu1PwrLimit + u1RateIdx + u1ParamIdx2) : (0x3F);
					}

					/* Not increment index for Power limit paramter for HT20/40 */
					if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type) {
						if ((u1FillParamType != SINGLE_SKU_TABLE_HT20) && (u1FillParamType != SINGLE_SKU_TABLE_HT40))
							u1ParamIdx2 += *(pu1FillParamTypeLen + u1FillParamType);
					} else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type) {
						if ((u1FillParamType != BACKOFF_TABLE_BF_OFF_HT20) && (u1FillParamType != BACKOFF_TABLE_BF_ON_HT20) && (u1FillParamType != BACKOFF_TABLE_BF_OFF_HT40) && (u1FillParamType != BACKOFF_TABLE_BF_ON_HT40))
							u1ParamIdx2 += *(pu1FillParamTypeLen + u1FillParamType);
					}
				}

				/* stop channel list search loop */
				break;
			}
		}
	}
}

VOID MtPwrLimitParse(PUINT pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	PINT8 pu1ParamTypeLen, pu1ChBandNeedParse;
	PCHAR pcToken;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_PARSE_NUM, BACKOFF_TYPE_PARSE_NUM};

	/* get pointer of parameter type length array and Need Parse check */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type) {
		pu1ParamTypeLen = pAd->u1SkuParameLen;
		pu1ChBandNeedParse = pAd->u1SkuChBandNeedParse;
	} else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type) {
		pu1ParamTypeLen = pAd->u1BackoffParamLen;
		pu1ChBandNeedParse = pAd->u1BackoffChBandNeedParse;
	}

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++, u1ParamIdx += *(pu1ParamTypeLen + u1ParamType)) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				/* toker update for next character parsing */
				pcToken = rstrtok(NULL, " ");

				if (pcToken)
					*(pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx) = os_str_tol(pcToken, 0, 10);
				else
					break;
			}
		}
	}
}

BOOLEAN MtPwrLimitSimilarCheck(PUINT pi1PwrLimitStartCh, PUINT pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	PINT8 pu1ParamTypeLen, pu1ChBandNeedParse;
	BOOLEAN fgSameContent = TRUE;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_PARSE_NUM, BACKOFF_TYPE_PARSE_NUM};

	/* get pointer of parameter type length array and Need Parse check */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type) {
		pu1ParamTypeLen = pAd->u1SkuParameLen;
		pu1ChBandNeedParse = pAd->u1SkuChBandNeedParse;
	} else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type) {
		pu1ParamTypeLen = pAd->u1BackoffParamLen;
		pu1ChBandNeedParse = pAd->u1BackoffChBandNeedParse;
	}

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++, u1ParamIdx += *(pu1ParamTypeLen + u1ParamType)) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				if (*(pi1PwrLimitStartCh + u1ColIdx + u1ParamIdx) != *(pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx)) {
					fgSameContent = FALSE;
					return fgSameContent;
				}
			}
		}
	}

	return fgSameContent;
}

VOID MtShowPwrLimitTable(RTMP_ADAPTER *pAd, UINT8 u1Type, UINT8 u1DebugLevel)
{
	PDL_LIST pList;
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	PINT8 pu1ParamTypeLen;
	P_CH_POWER prPwrLimitTbl, prTempPwrLimitTbl;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TYPE_PARSE_NUM, BACKOFF_TYPE_PARSE_NUM};
	CHAR cSkuParseTypeName[SINGLE_SKU_TYPE_PARSE_NUM][7] = {"CCK", "OFDM", "VHT20", "VHT40", "VHT80", "VHT160"};
	CHAR cBackoffParseTypeName[BACKOFF_TYPE_PARSE_NUM][14] = {"BFOFF_CCK", "BF_OFF_OFDM", "BF_ON_OFDM", "BF_OFF_VHT20", "BF_ON_VHT20", "BF_OFF_VHT40", "BF_ON_VHT40", "BF_OFF_VHT80", "BF_ON_VHT80", "BF_OFF_VHT160", "BF_ON_VHT160"};

	/* get pointer of parameter type length array */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type) {
		pu1ParamTypeLen = pAd->u1SkuParameLen;
		pList = &pAd->PwrLimitSkuList;
	} else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type) {
		pu1ParamTypeLen = pAd->u1BackoffParamLen;
		pList = &pAd->PwrLimitBackoffList;
	}

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("SKU table index: %d \n", pAd->CommonCfg.SKUTableIdx));

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER, List) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("start channel: %d, ChListNum: %d\n", prPwrLimitTbl->u1StartChannel, prPwrLimitTbl->u1ChNum));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Band: %d \n", prPwrLimitTbl->u1ChBand));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Channel: "));
		for (u1ColIdx = 0; u1ColIdx < prPwrLimitTbl->u1ChNum; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->pu1ChList[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
		for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++, u1ParamIdx += *(pu1ParamTypeLen + u1ParamType)) {

			if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%s: ", cSkuParseTypeName[u1ParamType]));
			else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%s: ", cBackoffParseTypeName[u1ParamType]));

			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", *(prPwrLimitTbl->pu1PwrLimit + u1ColIdx + u1ParamIdx)));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));
		}
	}

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
}
#endif /* defined(MT7615) || defined(MT7622) */

NDIS_STATUS MtPowerLimitFormatTrans(RTMP_ADAPTER *pAd, PUINT8 pu1Value, PCHAR pcRawData)
{
	CHAR *cBuffer = NULL;
	CHAR *cToken = NULL;
	UINT8 u1NonInteValue = 0;

	/* sanity check for null pointer */
	if (!pu1Value)
		goto error1;

	/* sanity check for null poitner */
	if (!pcRawData)
		goto error2;

	/* neglect multiple spaces for content parsing */
	pcRawData += strspn(pcRawData, " ");

	/* decimal point existence check */
	if (!strchr(pcRawData, '.'))
		*pu1Value = (UINT8)os_str_tol(pcRawData, 0, 10) * 2;
	else {
		/* backup pointer to string of parser function */
		cBuffer = __rstrtok;

		/* parse integer part */
		cToken = rstrtok(pcRawData, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error3;

		/* transform integer part unit to (0.5) */
		*pu1Value = (UINT8)os_str_tol(cToken, 0, 10) * 2;

		/* parse non-integer part */
		cToken = rstrtok(NULL, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error4;

		/* get non-integer part */
		u1NonInteValue = (UINT8)os_str_tol(cToken, 0, 10);

		/* increment for non-zero non-integer part */
		if (u1NonInteValue >= 5)
			(*pu1Value) += 1;

		/* backup pointer to string of parser function */
		__rstrtok = cBuffer;
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for buffer to update transform result !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for raw data buffer !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for integer value parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for decimal value parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

CHAR SKUTxPwrOffsetGet(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucBW, UINT8 ucPhymode, UINT8 ucMCS, UINT8 ucNss, BOOLEAN fgSE)
{
	CHAR   cPowerOffset = 0;
	UINT8  ucRateOffset = 0;
	UINT8  ucNSS = 0;
	UINT8  BW_OFFSET[4] = {VHT20_OFFSET, VHT40_OFFSET, VHT80_OFFSET, VHT160C_OFFSET};
#ifdef CONFIG_ATE
	struct	_ATE_CTRL	*ATECtrl = &(pAd->ATECtrl);
#endif

	/* Compute MCS rate and Nss for HT mode */
	if ((ucPhymode == MODE_HTMIX) || (ucPhymode == MODE_HTGREENFIELD)) {
		ucNss = (ucMCS >> 3) + 1;
		ucMCS &= 0x7;
	}

	switch (ucPhymode) {
	case MODE_CCK:
		ucRateOffset = SKU_CCK_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_CCK_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_CCK_RATE_M23;
			break;

		default:
			break;
		}

		break;

	case MODE_OFDM:
		ucRateOffset = SKU_OFDM_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_OFDM_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_OFDM_RATE_M23;
			break;

		case MCS_4:
		case MCS_5:
			ucRateOffset = SKU_OFDM_RATE_M45;
			break;

		case MCS_6:
			ucRateOffset = SKU_OFDM_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset = SKU_OFDM_RATE_M7;
			break;

		default:
			break;
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		ucRateOffset = SKU_HT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_HT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_HT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_HT_RATE_M34;
			break;

		case MCS_5:
			ucRateOffset += SKU_HT_RATE_M5;
			break;

		case MCS_6:
			ucRateOffset += SKU_HT_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset += SKU_HT_RATE_M7;
			break;
		}

		break;

	case MODE_VHT:
		ucRateOffset = SKU_VHT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_VHT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_VHT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_VHT_RATE_M34;
			break;

		case MCS_5:
		case MCS_6:
			ucRateOffset += SKU_VHT_RATE_M56;
			break;

		case MCS_7:
			ucRateOffset += SKU_VHT_RATE_M7;
			break;

		case MCS_8:
			ucRateOffset += SKU_VHT_RATE_M8;
			break;

		case MCS_9:
			ucRateOffset += SKU_VHT_RATE_M9;
			break;

		default:
			break;
		}

		break;
	}

	/* Update Power offset by look up Tx Power Compensation Table */
	cPowerOffset = (fgSE) ? (pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][ucNSS - 1]) : (pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][3]);

	/* Debug log for SKU Power offset to compensate */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (KBLU "%s: ucBW: %d, ucPhymode: %d, ucMCS: %d, ucNss: %d, fgSPE: %d !!!\n" KNRM, __func__, ucBW, ucPhymode, ucMCS,
			  ucNss, fgSE));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (KBLU "%s: cPowerOffset: 0x%x (%d) !!!\n" KNRM, __func__,
			 cPowerOffset, cPowerOffset));

#ifdef CONFIG_ATE
	/* Check if Single SKU is disabled */
	if (!ATECtrl->fgTxPowerSKUEn)
		cPowerOffset = 0;
#endif
	return cPowerOffset;
}

#endif /* SINGLE_SKU_V2 */
