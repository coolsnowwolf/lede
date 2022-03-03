/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"

#ifdef MULTI_PROFILE
#ifdef DSCP_QOS_MAP_SUPPORT
INT multi_profile_get_bss_num(struct _RTMP_ADAPTER *ad, UINT8 profile_num);
#endif
#endif

#define ETH_MAC_ADDR_STR_LEN 17  /* in format of xx:xx:xx:xx:xx:xx*/

/* We assume the s1 is a sting, s2 is a memory space with 6 bytes. and content of s1 will be changed.*/
BOOLEAN rtstrmactohex(RTMP_STRING *s1, RTMP_STRING *s2)
{
	int i = 0;
	RTMP_STRING *ptokS = s1, *ptokE = s1;

	if (strlen(s1) != ETH_MAC_ADDR_STR_LEN)
		return FALSE;

	while ((*ptokS) != '\0') {
		ptokE = strchr(ptokS, ':');

		if (ptokE != NULL)
			*ptokE++ = '\0';

		if ((strlen(ptokS) != 2) || (!isxdigit(*ptokS)) || (!isxdigit(*(ptokS + 1))))
			break; /* fail*/

		AtoH(ptokS, (PUCHAR)&s2[i++], 1);
		ptokS = ptokE;

		if (ptokS == NULL)
			break;

		if (i == 6)
			break; /* parsing finished*/
	}

	return (i == 6 ? TRUE : FALSE);
}


#define ASC_LOWER(_x)	((((_x) >= 0x41) && ((_x) <= 0x5a)) ? (_x) + 0x20 : (_x))
/* we assume the s1 and s2 both are strings.*/
BOOLEAN rtstrcasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	RTMP_STRING *p1 = s1, *p2 = s2;
	CHAR c1, c2;

	if (strlen(s1) != strlen(s2))
		return FALSE;

	while (*p1 != '\0') {
		c1 = ASC_LOWER(*p1);
		c2 = ASC_LOWER(*p2);

		if (c1 != c2)
			return FALSE;

		p1++;
		p2++;
	}

	return TRUE;
}


/* we assume the s1 (buffer) and s2 (key) both are strings.*/
RTMP_STRING *rtstrstruncasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	INT l1, l2, i;
	char temp1, temp2;
	l2 = strlen(s2);

	if (!l2)
		return (char *) s1;

	l1 = strlen(s1);

	while (l1 >= l2) {
		l1--;

		for (i = 0; i < l2; i++) {
			temp1 = *(s1 + i);
			temp2 = *(s2 + i);

			if (('a' <= temp1) && (temp1 <= 'z'))
				temp1 = 'A' + (temp1 - 'a');

			if (('a' <= temp2) && (temp2 <= 'z'))
				temp2 = 'A' + (temp2 - 'a');

			if (temp1 != temp2)
				break;
		}

		if (i == l2)
			return (char *) s1;

		s1++;
	}

	return NULL; /* not found*/
}


/**
 * strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
RTMP_STRING *rtstrstr(const RTMP_STRING *s1, const RTMP_STRING *s2)
{
	INT l1, l2;
	l2 = strlen(s2);

	if (!l2)
		return (RTMP_STRING *)s1;

	l1 = strlen(s1);

	while (l1 >= l2) {
		l1--;

		if (!memcmp(s1, s2, l2))
			return (RTMP_STRING *)s1;

		s1++;
	}

	return NULL;
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
RTMP_STRING *__rstrtok;
RTMP_STRING *rstrtok(RTMP_STRING *s, const RTMP_STRING *ct)
{
	RTMP_STRING *sbegin, *send;
	sbegin  = s ? s : __rstrtok;

	if (!sbegin)
		return NULL;

	sbegin += strspn(sbegin, ct);

	if (*sbegin == '\0') {
		__rstrtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, ct);

	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;
	return sbegin;
}

/**
 * delimitcnt - return the count of a given delimiter in a given string.
 * @s: The string to be searched.
 * @ct: The delimiter to search for.
 * Notice : We suppose the delimiter is a single-char string(for example : ";").
 */
INT delimitcnt(RTMP_STRING *s, RTMP_STRING *ct)
{
	INT count = 0;
	/* point to the beginning of the line */
	RTMP_STRING *token = s;

	for (;; ) {
		token = strpbrk(token, ct); /* search for delimiters */

		if (token == NULL) {
			/* advanced to the terminating null character */
			break;
		}

		/* skip the delimiter */
		++token;
		/*
		 * Print the found text: use len with %.*s to specify field width.
		 */
		/* accumulate delimiter count */
		++count;
	}

	return count;
}

/*
  * converts the Internet host address from the standard numbers-and-dots notation
  * into binary data.
  * returns nonzero if the address is valid, zero if not.
  */
int rtinet_aton(const RTMP_STRING *cp, unsigned int *addr)
{
	unsigned int	val;
	int	base, n;
	RTMP_STRING c;
	unsigned int    parts[4] = {0};
	unsigned int    *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 *	0x=hex, 0=octal, other=decimal.
		 */
		val = 0;
		base = 10;

		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}

		while ((c = *cp) != '\0') {
			if (isdigit((unsigned char) c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}

			if (base == 16 && isxdigit((unsigned char) c)) {
				val = (val << 4) +
					  (c + 10 - (islower((unsigned char) c) ? 'a' : 'A'));
				cp++;
				continue;
			}

			break;
		}

		if (*cp == '.') {
			/*
			 * Internet format: a.b.c.d a.b.c   (with c treated as 16-bits)
			 * a.b     (with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return 0;

			*pp++ = val, cp++;
		} else
			break;
	}

	/*
	 * Check for trailing junk.
	 */
	while (*cp)
		if (!isspace((unsigned char) *cp++))
			return 0;

	/*
	 * Concoct the address according to the number of parts specified.
	 */
	n = pp - parts + 1;

	switch (n) {
	case 1:         /* a -- 32 bits */
		break;

	case 2:         /* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return 0;

		val |= parts[0] << 24;
		break;

	case 3:         /* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return 0;

		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:         /* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return 0;

		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}

	*addr = OS_HTONL(val);
	return 1;
}

#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
static UCHAR GetDefaultChannel(UCHAR PhyMode)
{
	/*priority must the same as Default PhyMode*/
	if (WMODE_CAP_2G(PhyMode))
		return 1;
	else if (WMODE_CAP_5G(PhyMode))
		return 36;

	return 1;
}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT) */


static VOID RTMPChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR Channel;
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		Channel = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			wdev->channel = Channel;

#ifdef BW_VENDOR10_CUSTOM_FEATURE
			SET_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, FALSE);
#endif
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS%d Channel=%d\n", i, wdev->channel));

		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	/*Can not assign default channel to wdev-> channel when channel = 0 */
	/*Just by reason of channel = 0 is one of the indicators of auto-channel selection */
	if (!pAd->ApCfg.bAutoChannelAtBootup) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pAd->ApCfg.MBSSID[0].wdev.channel == 0)
				pAd->ApCfg.MBSSID[0].wdev.channel = GetDefaultChannel(pAd->ApCfg.MBSSID[0].wdev.PhyMode);

			/*Check if any wdev not configure a channel, apply MSSID 0 channel to it.*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;

				if (wdev->channel == 0)
					wdev->channel = pAd->ApCfg.MBSSID[0].wdev.channel;
			}
		}
	}
#endif/*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/

}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
static VOID RTMPOldChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR BackupChannel;

	if ((Buffer == NULL) || (IS_SUPPORT_V10_DFS(pAd) == FALSE))
		return;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
#ifdef CONFIG_AP_SUPPORT
		/* V10 -- AP Mode Only */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			/* V10 -- Only 5G Mode */
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if ((wdev == NULL) || (!WMODE_CAP_5G(wdev->PhyMode)))
				continue;

			/* Extract Channel */
			BackupChannel = os_str_tol(macptr, 0, 10);

			/* Disallow Zero or Invalid Values */
			if ((!BackupChannel) || (DfsV10CheckChnlGrp(BackupChannel) == NA_GRP)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Backup Channel=%d\n",
					__func__, BackupChannel));
				continue;
			}

			/* Valid Old Channel Processing */
			if (BackupChannel && (!IS_V10_OLD_CHNL_VALID(wdev)))
				SET_V10_OLD_CHNL_VALID(wdev, TRUE);

			/* Update Channel */
			wdev->channel = BackupChannel;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] BSS%d Channel=%d\n",
				__func__, i, wdev->channel));
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}
#endif

#ifdef WHNAT_SUPPORT
static VOID RTMPWHNATCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	pAd->CommonCfg.whnat_en = os_str_tol(Buffer, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WHNAT=%d\n", pAd->CommonCfg.whnat_en));
}
#endif /*WHNAT_SUPPORT*/

static VOID RTMPWirelessModeCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i;
	UCHAR cfg_mode, *macptr;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		cfg_mode = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_AP_SUPPORT

		if (i >= pAd->ApCfg.BssidNum)
			break;

		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS%d PhyMode=%d\n", i, wdev->PhyMode));
#ifdef MBSS_SUPPORT

		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++)
				pAd->ApCfg.MBSSID[IdBss].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
		} else
			RT_CfgSetMbssWirelessMode(pAd, macptr);

#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		if (i == 0) {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
			UCHAR idx;

			/* for first time, update all phy mode is same as ra0 */
			for (idx = 0; idx < MAX_APCLI_NUM; idx++)
				pAd->ApCfg.ApCliTab[idx].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;

#endif /*APCLI_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
			RT_CfgSetWirelessMode(pAd, macptr, wdev);
		}
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT

	/*Check if any wdev not configure a wireless mode, apply MSSID value to it.*/
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;

		if (wdev->PhyMode == WMODE_INVALID)
			wdev->PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
	}

#endif/*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/

	if (wdev)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PhyMode=%d\n", wdev->PhyMode));
}


/*
    ========================================================================

    Routine Description:
	Find key section for Get key parameter.

    Arguments:
	buffer                      Pointer to the buffer to start find the key section
	section                     the key of the secion to be find

    Return Value:
	NULL                        Fail
	Others                      Success
    ========================================================================
*/
RTMP_STRING *RTMPFindSection(RTMP_STRING *buffer)
{
	RTMP_STRING temp_buf[32];
	RTMP_STRING *ptr, *ret = NULL;
	strcpy(temp_buf, "Default");
	ptr = rtstrstr(buffer, temp_buf);

	if (ptr != NULL) {
		ret = ptr + strlen("\n");
		return ret;
	} else
		return NULL;
}

/*
    ========================================================================

    Routine Description:
	Get key parameter.

    Arguments:
	key			Pointer to key string
	dest			Pointer to destination
	destsize		The datasize of the destination
	buffer		Pointer to the buffer to start find the key
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPGetKeyParameter(
	IN RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace)
{
	RTMP_STRING *pMemBuf, *temp_buf1 = NULL, *temp_buf2 = NULL;
	RTMP_STRING *start_ptr, *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = NULL;
	INT  len, keyLen;
	keyLen = strlen(key);
	os_alloc_mem(NULL, (PUCHAR *)&pMemBuf, MAX_PARAM_BUFFER_SIZE  * 2);

	if (pMemBuf == NULL)
		return FALSE;

	memset(pMemBuf, 0, MAX_PARAM_BUFFER_SIZE * 2);
	temp_buf1 = pMemBuf;
	temp_buf2 = (RTMP_STRING *)(pMemBuf + MAX_PARAM_BUFFER_SIZE);
	/*find section*/
	offset = RTMPFindSection(buffer);

	if (offset == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	strcpy(temp_buf1, "\n");
	strcat(temp_buf1, key);
	strcat(temp_buf1, "=");
	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	start_ptr += strlen("\n");
	end_ptr = rtstrstr(start_ptr, "\n");

	if (end_ptr == NULL)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';
	start_ptr = rtstrstr(temp_buf2, "=");

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	ptr = (start_ptr + 1);

	/*trim special characters, i.e.,  TAB or space*/
	while (*start_ptr != 0x00) {
		if (((*ptr == ' ') && bTrimSpace) || (*ptr == '\t'))
			ptr++;
		else
			break;
	}

	len = strlen(start_ptr);
	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, ((len >= destsize) ? destsize : len));
	os_free_mem((PUCHAR)pMemBuf);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Add key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPAddKeyParameter(
	IN RTMP_STRING *key,
	IN CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer)
{
	UINT len = strlen(buffer);
	CHAR *ptr = buffer + len;
	snprintf(ptr, 300, "%s=%s\n", key, value);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Set key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPSetKeyParameter(
	IN RTMP_STRING *key,
	OUT CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace)
{
	RTMP_STRING buf[512] = "", *temp_buf1 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *offset = NULL;
	INT keyLen;
	INT start_len;
	INT end_len;
	INT len;
	keyLen = strlen(key);
	temp_buf1 = buf;

	/*find section*/
	offset = RTMPFindSection(buffer);
	if (offset == NULL)
		return (FALSE);

	strncpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));

	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);
	if (start_ptr == NULL) {
		/*can't searched, add directly*/
		RTMPAddKeyParameter(key, value, destsize, buffer);
		return (TRUE);
	}

	/*remove original*/
	start_ptr += strlen("\n");
	start_len = strlen(start_ptr);

	end_ptr = rtstrstr(start_ptr, "\n");
	if (end_ptr == NULL)
		end_ptr = start_ptr + start_len;

	if (end_ptr < start_ptr)
		return (FALSE);

	/*clear original setting*/
	end_ptr += strlen("\n");
	end_len = strlen(end_ptr);
	os_move_mem(start_ptr, end_ptr, end_len);
	start_ptr += end_len;
	len = start_len - end_len;
	os_zero_mem(start_ptr, len);
	/*fill new field & value*/
	RTMPAddKeyParameter(key, value, destsize, buffer);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
	Get multiple key parameter.

    Arguments:
	key                         Pointer to key string
	dest                        Pointer to destination
	destsize                    The datasize of the destination
	buffer                      Pointer to the buffer to start find the key

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
    ========================================================================
*/
INT RTMPGetKeyParameterWithOffset(
	IN  RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	OUT	USHORT *end_offset,
	IN  INT     destsize,
	IN  RTMP_STRING *buffer,
	IN	BOOLEAN	bTrimSpace)
{
	RTMP_STRING *temp_buf1 = NULL;
	RTMP_STRING *temp_buf2 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = 0;
	INT  len;

	if (*end_offset >= MAX_INI_BUFFER_SIZE)
		return FALSE;

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf1, MAX_PARAM_BUFFER_SIZE);

	if (temp_buf1 == NULL)
		return FALSE;

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf2, MAX_PARAM_BUFFER_SIZE);

	if (temp_buf2 == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		return FALSE;
	}

	/*find section		*/
	if (*end_offset == 0) {
		offset = RTMPFindSection(buffer);

		if (offset == NULL) {
			os_free_mem((PUCHAR)temp_buf1);
			os_free_mem((PUCHAR)temp_buf2);
			return FALSE;
		}
	} else
		offset = buffer + (*end_offset);

	strncpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));
	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	start_ptr += strlen("\n");
	end_ptr = rtstrstr(start_ptr, "\n");

	if (end_ptr == NULL)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	*end_offset = end_ptr - buffer;
	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';
	len = strlen(temp_buf2);
	strcpy(temp_buf1, temp_buf2);
	start_ptr = rtstrstr(temp_buf1, "=");

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	strcpy(temp_buf2, start_ptr + 1);
	ptr = temp_buf2;

	/*trim space or tab*/
	while (*ptr != 0x00) {
		if ((bTrimSpace && (*ptr == ' ')) || (*ptr == '\t'))
			ptr++;
		else
			break;
	}

	len = strlen(ptr);
	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, len >= destsize ?  destsize : len);
	os_free_mem((PUCHAR)temp_buf1);
	os_free_mem((PUCHAR)temp_buf2);
	return TRUE;
}

#ifdef MIN_PHY_RATE_SUPPORT
VOID RTMPMinPhyDataRateCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i, *macptr;
	UCHAR MinPhyDataRate;
	BSS_STRUCT *Mbss = NULL;
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		MinPhyDataRate = (UCHAR)simple_strtol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		Mbss = &pAd->ApCfg.MBSSID[i];
		Mbss->wdev.rate.MinPhyDataRate = MinPhyDataRate;
		Mbss->wdev.rate.MinPhyDataRateTransmit = MinPhyRate_2_HtTransmit(Mbss->wdev.rate.MinPhyDataRate);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS%d MinPhyDataRate=%d\n", i, Mbss->wdev.rate.MinPhyDataRate));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s MinPhyDataRateTransmit = 0x%x\n"
					, __func__, Mbss->wdev.rate.MinPhyDataRateTransmit.word));
#ifdef MBSS_SUPPORT
		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyDataRate = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyDataRate;
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyDataRateTransmit = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyDataRateTransmit;
			}
		}
#endif /* MBSS_SUPPORT */
	}
}

VOID RTMPMinPhyBeaconRateCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i, *macptr;
	UCHAR MinPhyBeaconRate;
	BSS_STRUCT *Mbss = NULL;
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		MinPhyBeaconRate = (UCHAR)simple_strtol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		Mbss = &pAd->ApCfg.MBSSID[i];
		Mbss->wdev.rate.MinPhyBeaconRate = MinPhyBeaconRate;
		Mbss->wdev.rate.MinPhyBeaconRateTransmit = MinPhyRate_2_HtTransmit(Mbss->wdev.rate.MinPhyBeaconRate);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS%d MinPhyBeaconRate=%d\n", i, Mbss->wdev.rate.MinPhyBeaconRate));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s MinPhyBeaconRateTransmit = 0x%x\n"
					, __func__, Mbss->wdev.rate.MinPhyBeaconRateTransmit.word));
#ifdef MBSS_SUPPORT
		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyBeaconRate = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyBeaconRate;
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyBeaconRateTransmit = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyBeaconRateTransmit;
			}
		}
#endif /* MBSS_SUPPORT */
	}
}

VOID RTMPMinPhyMgmtRateCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i, *macptr;
	UCHAR MinPhyMgmtRate;
	BSS_STRUCT *Mbss = NULL;
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		MinPhyMgmtRate = (UCHAR)simple_strtol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		Mbss = &pAd->ApCfg.MBSSID[i];
		Mbss->wdev.rate.MinPhyMgmtRate = MinPhyMgmtRate;
		Mbss->wdev.rate.MinPhyMgmtRateTransmit = MinPhyRate_2_HtTransmit(Mbss->wdev.rate.MinPhyMgmtRate);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS%d MinPhyMgmtRate=%d\n", i, Mbss->wdev.rate.MinPhyMgmtRate));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s MinPhyMgmtRateTransmit = 0x%x\n"
					, __func__, Mbss->wdev.rate.MinPhyMgmtRateTransmit.word));
#ifdef MBSS_SUPPORT
		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyMgmtRate = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyMgmtRate;
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyMgmtRateTransmit = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyMgmtRateTransmit;
			}
		}
#endif /* MBSS_SUPPORT */
	}
}

VOID RTMPMinPhyBcMcRateCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i, *macptr;
	UCHAR MinPhyBcMcRate;
	BSS_STRUCT *Mbss = NULL;
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		MinPhyBcMcRate = (UCHAR)simple_strtol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		Mbss = &pAd->ApCfg.MBSSID[i];
		Mbss->wdev.rate.MinPhyBcMcRate = MinPhyBcMcRate;
		Mbss->wdev.rate.MinPhyBcMcRateTransmit = MinPhyRate_2_HtTransmit(Mbss->wdev.rate.MinPhyBcMcRate);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS%d MinPhyBcMcRate=%d\n", i, Mbss->wdev.rate.MinPhyBcMcRate));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s MinPhyBcMcRateTransmit = 0x%x\n"
					, __func__, Mbss->wdev.rate.MinPhyBcMcRateTransmit.word));
#ifdef MBSS_SUPPORT
		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyBcMcRate = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyBcMcRate;
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.MinPhyBcMcRateTransmit = pAd->ApCfg.MBSSID[0].wdev.rate.MinPhyBcMcRateTransmit;
			}
		}
#endif /* MBSS_SUPPORT */
	}
}

VOID RTMPLimitClientSupportRateCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i, *macptr;
	BOOL  LimitClientSupportRate;
	BSS_STRUCT *Mbss = NULL;
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		LimitClientSupportRate = (BOOL)simple_strtol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		Mbss = &pAd->ApCfg.MBSSID[i];
		Mbss->wdev.rate.LimitClientSupportRate = LimitClientSupportRate;
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS%d LimitClientSupportRate=%d\n", i, Mbss->wdev.rate.LimitClientSupportRate));
#ifdef MBSS_SUPPORT
		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.LimitClientSupportRate = pAd->ApCfg.MBSSID[0].wdev.rate.LimitClientSupportRate;
			}
		}
#endif /* MBSS_SUPPORT */
	}
}

VOID RTMPDisableCCKRateCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i, *macptr;
	BOOL  DisableCCKRate;
	BSS_STRUCT *Mbss = NULL;
#ifdef MBSS_SUPPORT
	UCHAR IdBss = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		DisableCCKRate = (BOOL)simple_strtol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		Mbss = &pAd->ApCfg.MBSSID[i];
		Mbss->wdev.rate.DisableCCKRate = DisableCCKRate;
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS%d DisableCCKRate=%d\n", i, Mbss->wdev.rate.DisableCCKRate));
#ifdef MBSS_SUPPORT
		if (i == 0) {
			/* for first time, update all phy mode is same as ra0 */
			for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pAd->ApCfg.MBSSID[IdBss].wdev.rate.DisableCCKRate = pAd->ApCfg.MBSSID[0].wdev.rate.DisableCCKRate;
			}
		}
#endif /* MBSS_SUPPORT */
	}
}

#endif /* MIN_PHY_RATE_SUPPORT */


#ifdef CONFIG_AP_SUPPORT

#ifdef APCLI_SUPPORT
static void rtmp_read_ap_client_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *buffer)
{
	RTMP_STRING *macptr = NULL;
	INT			i = 0, j = 0;
	UCHAR		macAddress[MAC_ADDR_LEN];
	PAPCLI_STRUCT   pApCliEntry = NULL;
	struct wifi_dev *wdev;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	UINT_8 idx = 0;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
		pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
		pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
	}

#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONVERTER_MODE_SWITCH_SUPPORT

		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Initially Start any BSS or AP with Default settings\n"));

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
		pAd->ApCfg.MBSSID[idx].APStartPseduState = AP_STATE_ALWAYS_START_AP_DEFAULT;
	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
		pApCliEntry->ApCliMode = APCLI_MODE_ALWAYS_START_AP_DEFAULT;
		}

#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
	/*ApCliEnable*/
	if (RTMPGetKeyParameter("ApCliEnable", tmpbuf, 128, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			if ((strncmp(macptr, "0", 1) == 0))
				pApCliEntry->Enable = FALSE;
			else if ((strncmp(macptr, "1", 1) == 0))
				pApCliEntry->Enable = TRUE;
			else {
				pApCliEntry->Enable = FALSE;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
				if (strncmp(macptr, "2", 1) == 0) {
					pApCliEntry->Enable = TRUE;
					pApCliEntry->ApCliMode =
							 APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION;
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("Start any BSS or AP, after ApcliConnection with RootAP\n"));
				} else if (strncmp(macptr, "3", 1) == 0) {
					pApCliEntry->Enable = TRUE;
					pApCliEntry->ApCliMode = APCLI_MODE_NEVER_START_AP;
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("Do Not start any BSS or AP\n"));
				}
				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					/* When ApCliEnable=2: Start AP, when Apcli connect to RootAP */
					/* When ApCliEnable=3: Never start any(BSS) AP */
					if (strncmp(macptr, "2", 1) == 0) {
						pAd->ApCfg.MBSSID[idx].APStartPseduState =
								 AP_STATE_START_AFTER_APCLI_CONNECTION;
					} else if (strncmp(macptr, "3", 1) == 0) {
						pAd->ApCfg.MBSSID[idx].APStartPseduState = AP_STATE_NEVER_START_AP;
					}
				}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].Enable=%d\n", i, pApCliEntry->Enable));
		}
	}

	/*ApCliSsid*/
	if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE)) {
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			/*Ssid acceptable strlen must be less than 32 and bigger than 0.*/
			pApCliEntry->CfgSsidLen = (UCHAR)strlen(macptr);

			if (pApCliEntry->CfgSsidLen > 32) {
				pApCliEntry->CfgSsidLen = 0;
				continue;
			}

			if (pApCliEntry->CfgSsidLen > 0) {
				memcpy(&pApCliEntry->CfgSsid, macptr, pApCliEntry->CfgSsidLen);
				pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
			} else {
				NdisZeroMemory(&(pApCliEntry->CfgSsid), MAX_LEN_OF_SSID);
				continue;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].CfgSsidLen=%d, CfgSsid=%s\n", i,
					 pApCliEntry->CfgSsidLen, pApCliEntry->CfgSsid));
		}
	}

#ifdef DBDC_MODE

	/*ApCliWirelessMode*/
	if (RTMPGetKeyParameter("ApCliWirelessMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			UCHAR cfg_mode;
			cfg_mode = os_str_tol(macptr, 0, 10);
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			pApCliEntry->wdev.PhyMode = cfgmode_2_wmode(cfg_mode);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].wdev.PhyMode=%d\n", i,
					 pApCliEntry->wdev.PhyMode));
		}
	}

#ifdef MULTI_PROFILE
	{
		UINT8 mbss_idx, max_phy;
		UCHAR start_idx[2] = {0}, end_idx[2] = {0};
		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			UINT8 pf1_num, pf2_num;

			pf1_num = multi_profile_get_pf1_num(pAd);
			pf2_num = multi_profile_get_pf2_num(pAd);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] pf1=%d pf2=%d\n",
				__func__, pf1_num, pf2_num));
			start_idx[0] = 0;
			start_idx[1] = pf1_num;
			end_idx[0] = pf1_num;
			end_idx[1] = pf1_num + pf2_num;
		} else {
			start_idx[0] = 0;
			start_idx[1] = 0;
			end_idx[0] = pAd->ApCfg.BssidNum;
			end_idx[1] = pAd->ApCfg.BssidNum;
		}

		for (i = 0; i < MAX_APCLI_NUM; i++) {
			max_phy = 0;
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			for (mbss_idx = start_idx[i]; mbss_idx < end_idx[i]; mbss_idx++) {
				BSS_STRUCT *pMbss = NULL;
				struct wifi_dev	 *mbss_wdev = NULL;

				pMbss = &pAd->ApCfg.MBSSID[mbss_idx];
				mbss_wdev = &pMbss->wdev;
				if (pApCliEntry->wdev.PhyMode == pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode) {
					max_phy = pApCliEntry->wdev.PhyMode;
					break;
				} else
					max_phy = (max_phy < mbss_wdev->PhyMode) ? mbss_wdev->PhyMode : max_phy;
			}
			pApCliEntry->wdev.PhyMode = max_phy;
		}
	}
#endif /*MULTI_PROFILE*/
#endif
	/*ApCliBssid*/
	if (RTMPGetKeyParameter("ApCliBssid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			if (strlen(macptr) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
				continue;

			if (strcmp(macptr, "00:00:00:00:00:00") == 0)
				continue;

			for (j = 0; j < MAC_ADDR_LEN; j++) {
				AtoH(macptr, &macAddress[j], 1);
				macptr = macptr + 3;
			}

			memcpy(pApCliEntry->CfgApCliBssid, &macAddress, MAC_ADDR_LEN);
			pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
		}
	}

	/* ApCliTxMode*/
	if (RTMPGetKeyParameter("ApCliTxMode", tmpbuf, 25, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			wdev = &pAd->ApCfg.ApCliTab[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Tx Mode = %d\n", i,
					 wdev->DesiredTransmitSetting.field.FixedTxMode));
		}
	}

	/* ApCliTxMcs*/
	if (RTMPGetKeyParameter("ApCliTxMcs", tmpbuf, 50, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			wdev = &pAd->ApCfg.ApCliTab[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Tx MCS = %s(%d)\n", i,
					 (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : ""),
					 wdev->DesiredTransmitSetting.field.MCS));
		}
	}

#ifdef WSC_AP_SUPPORT

	/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
	if (RTMPGetKeyParameter("ApCli_Wsc4digitPinCode", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			if (os_str_tol(macptr, 0, 10) != 0)
				pAd->ApCfg.ApCliTab[i].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
			else /* Disable */
				pAd->ApCfg.ApCliTab[i].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) ApCli_Wsc4digitPinCode=%d\n", i,
					 pAd->ApCfg.ApCliTab[i].wdev.WscControl.WscEnrollee4digitPinCode));
		}
	}

#ifdef APCLI_SUPPORT
	/* ApCliWscScanMode */
	if (RTMPGetKeyParameter("ApCliWscScanMode", tmpbuf, 32, buffer, TRUE)) {
		UCHAR Mode;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			Mode = simple_strtol(macptr, 0, 10);
			if (Mode != TRIGGER_PARTIAL_SCAN)
				Mode = TRIGGER_FULL_SCAN;

			pAd->ApCfg.ApCliTab[i].WscControl.WscApCliScanMode = Mode;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("I/F(apcli%d) WscApCliScanMode=%d\n", i, Mode));
		}
	}
#endif /* APCLI_SUPPORT */

#endif /* WSC_AP_SUPPORT */
#ifdef UAPSD_SUPPORT

	/*APSDCapable*/
	if (RTMPGetKeyParameter("ApCliAPSDCapable", tmpbuf, 10, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = TRUE;

		for (i = 0, macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			pApCliEntry->wdev.UapsdInfo.bAPSDCapable = \
					(UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliAPSDCapable[%d]=%d\n", i,
					 pApCliEntry->wdev.UapsdInfo.bAPSDCapable));
		}
	}

#endif /* UAPSD_SUPPORT */

	/* ApCliNum */
	if (RTMPGetKeyParameter("ApCliNum", tmpbuf, 10, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) <= MAX_APCLI_NUM)
			pAd->ApCfg.ApCliNum = os_str_tol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli) ApCliNum=%d\n", pAd->ApCfg.ApCliNum));
	}

#if defined(DBDC_MODE) && defined(MT7615)

	if (pAd->CommonCfg.dbdc_mode == TRUE)
		pAd->ApCfg.ApCliNum = 2;
	else
		pAd->ApCfg.ApCliNum = 1;

#else
	pAd->ApCfg.ApCliNum = MAX_APCLI_NUM_DEFAULT;
#endif
#ifdef APCLI_CONNECTION_TRIAL
	pAd->ApCfg.ApCliNum++;

	/* ApCliTrialCh */
	if (RTMPGetKeyParameter("ApCliTrialCh", tmpbuf, 128, buffer, TRUE)) {
		/* last IF is for apcli connection trial */
		pApCliEntry = &pAd->ApCfg.ApCliTab[pAd->ApCfg.ApCliNum - 1];
		pApCliEntry->TrialCh = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TrialChannel=%d\n", pApCliEntry->TrialCh));
	}

#endif /* APCLI_CONNECTION_TRIAL */
#ifdef DOT11W_PMF_SUPPORT

	/* Protection Management Frame Capable */
	if (RTMPGetKeyParameter("ApCliPMFMFPC", tmpbuf, 32, buffer, TRUE)) {

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {

			PMF_CFG *pPmfCfg = NULL;
			struct wifi_dev *wdev = NULL;

			pPmfCfg = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg;
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			if (!pPmfCfg || !wdev) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: pPmfCfg=%p, wdev=%p\n",
						 __func__, pPmfCfg, wdev));
				return;
			}

			if (os_str_tol(macptr, 0, 10))
				pPmfCfg->Desired_MFPC = TRUE;
			else {
				pPmfCfg->Desired_MFPC = FALSE;
				pPmfCfg->MFPC = FALSE;
				pPmfCfg->MFPR = FALSE;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Desired MFPC=%d\n",
					 __func__, pPmfCfg->Desired_MFPC));

			if ((IS_AKM_WPA2_Entry(wdev) || IS_AKM_WPA2PSK_Entry(wdev)
#ifdef APCLI_SAE_SUPPORT
				|| IS_AKM_WPA3PSK_Entry(wdev)
#endif
#ifdef APCLI_OWE_SUPPORT
						|| IS_AKM_OWE_Entry(wdev)
#endif
		)
				&& IS_CIPHER_AES_Entry(wdev)) {
				pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

				if (pPmfCfg->Desired_MFPC) {
					pPmfCfg->MFPC = TRUE;
					pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

					if (pPmfCfg->MFPR)
						pPmfCfg->PMFSHA256 = TRUE;
				}
			} else if (pPmfCfg->Desired_MFPC)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK/WPA3PSK AES\n", __func__));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
					 __func__,
					 pPmfCfg->MFPC,
					 pPmfCfg->MFPR,
					 pPmfCfg->PMFSHA256));

		}
	}

	/* Protection Management Frame Required */
	if (RTMPGetKeyParameter("ApCliPMFMFPR", tmpbuf, 32, buffer, TRUE)) {

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			PMF_CFG *pPmfCfg = NULL;
			struct wifi_dev *wdev = NULL;

			pPmfCfg = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg;
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			if (!pPmfCfg || !wdev) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: pPmfCfg=%p, wdev=%p\n",
						 __func__, pPmfCfg, wdev));
				return;
			}

			if (os_str_tol(macptr, 0, 10))
				pPmfCfg->Desired_MFPR = TRUE;
			else {
				pPmfCfg->Desired_MFPR = FALSE;
				/* only close the MFPR */
				pPmfCfg->MFPR = FALSE;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Desired MFPR=%d\n",
					 __func__, pPmfCfg->Desired_MFPR));

			if ((IS_AKM_WPA2_Entry(wdev) || IS_AKM_WPA2PSK_Entry(wdev)
#ifdef APCLI_SAE_SUPPORT
		|| IS_AKM_WPA3PSK_Entry(wdev)
#endif
#ifdef APCLI_OWE_SUPPORT
					|| IS_AKM_OWE_Entry(wdev)
#endif
		)
				&& IS_CIPHER_AES_Entry(wdev)) {
				pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

				if (pPmfCfg->Desired_MFPC) {
					pPmfCfg->MFPC = TRUE;
					pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

					if (pPmfCfg->MFPR)
						pPmfCfg->PMFSHA256 = TRUE;
				}
			} else if (pPmfCfg->Desired_MFPC)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK/WPA3PSK AES\n", __func__));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
					 __func__, pPmfCfg->MFPC,
					 pPmfCfg->MFPR,
					 pPmfCfg->PMFSHA256));

		}
	}

	if (RTMPGetKeyParameter("ApCliPMFSHA256", tmpbuf, 32, buffer, TRUE)) {

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {

			PMF_CFG *pPmfCfg = NULL;

			pPmfCfg = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg;

			if (!pPmfCfg) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: pPmfCfg=%p\n",
						 __func__, pPmfCfg));
				return;
			}

			if (os_str_tol(macptr, 0, 10))
				pPmfCfg->Desired_PMFSHA256 = TRUE;
			else
				pPmfCfg->Desired_PMFSHA256 = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Desired PMFSHA256=%d\n",
					 __func__, pPmfCfg->Desired_PMFSHA256));

		}
	}

#endif /* DOT11W_PMF_SUPPORT */
}
#endif /* APCLI_SUPPORT */


static void rtmp_read_acl_parms_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING tok_str[32], *macptr;
	INT			i = 0, j = 0, idx;
	UCHAR		macAddress[MAC_ADDR_LEN];
	BOOLEAN		isDuplicate = FALSE;
	memset(macAddress, 0, MAC_ADDR_LEN);

	for (idx = 0; idx < MAX_MBSSID_NUM(pAd); idx++) {
		memset(&pAd->ApCfg.MBSSID[idx].AccessControlList, 0, sizeof(RT_802_11_ACL));
		/* AccessPolicyX*/
		snprintf(tok_str, sizeof(tok_str), "AccessPolicy%d", idx);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 10, buffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /* Allow All, and the AccessControlList is positive now.*/
				pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 1;
				break;

			case 2: /* Reject All, and the AccessControlList is negative now.*/
				pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 2;
				break;

			case 0: /* Disable, don't care the AccessControlList.*/
			default:
				pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 0;
				break;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s=%ld\n", tok_str,
					 pAd->ApCfg.MBSSID[idx].AccessControlList.Policy));
		}

		/* AccessControlListX*/
		snprintf(tok_str, sizeof(tok_str), "AccessControlList%d", idx);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
					continue;

				ASSERT(pAd->ApCfg.MBSSID[idx].AccessControlList.Num <= MAX_NUM_OF_ACL_LIST);

				for (j = 0; j < MAC_ADDR_LEN; j++) {
					AtoH(macptr, &macAddress[j], 1);
					macptr = macptr + 3;
				}

				if (pAd->ApCfg.MBSSID[idx].AccessControlList.Num == MAX_NUM_OF_ACL_LIST) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 ("The AccessControlList is full, and no more entry can join the list!\n"));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The last entry of ACL is %02x:%02x:%02x:%02x:%02x:%02x\n",
							 macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]));
					break;
				}

				isDuplicate = FALSE;
				for (j = 0; j < pAd->ApCfg.MBSSID[idx].AccessControlList.Num; j++) {
					if (memcmp(pAd->ApCfg.MBSSID[idx].AccessControlList.Entry[j].Addr, &macAddress, 6) == 0) {
						isDuplicate = TRUE;
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("You have added an entry before :\n"));
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("The duplicate entry is %02x:%02x:%02x:%02x:%02x:%02x\n",
							macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]));
					}
				}

				if (!isDuplicate) {
					pAd->ApCfg.MBSSID[idx].AccessControlList.Num++;
					NdisMoveMemory(pAd->ApCfg.MBSSID[idx].AccessControlList.Entry[(pAd->ApCfg.MBSSID[idx].AccessControlList.Num - 1)].Addr,
								   macAddress, MAC_ADDR_LEN);
				}
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s=Get %ld Mac Address\n", tok_str,
					 pAd->ApCfg.MBSSID[idx].AccessControlList.Num));
		}
	}
}

/*
    ========================================================================

    Routine Description:
	In kernel mode read parameters from file

    Arguments:
	src                     the location of the file.
	dest                        put the parameters to the destination.
	Length                  size to read.

    Return Value:
	None

    Note:

    ========================================================================
*/

static void rtmp_read_ap_edca_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr, *edcaptr, tok_str[16];
	INT	i = 0, j = 0;
	EDCA_PARM *pEdca;
	RTMP_STRING *ptmpStr[6];
	struct wifi_dev *wdev = NULL;
	UCHAR ack_policy[WMM_NUM_OF_AC];

	for (j = 0; j < WMM_NUM; j++) {
		snprintf(tok_str, sizeof(tok_str), "APEdca%d", j);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			pEdca = &pAd->CommonCfg.APEdcaParm[j];

			for (i = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr; edcaptr = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = edcaptr;

			if (i != 6) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Input parameter incorrect\n"));
				return;
			}

			/*APValid*/
			edcaptr = ptmpStr[0];

			if (edcaptr) {
				pEdca->bValid = (UCHAR) os_str_tol(edcaptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Valid=%d\n", pEdca->bValid));
			}

			/*APAifsn*/
			edcaptr = ptmpStr[1];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Aifsn[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("APAifsn[%d]=%d\n", i, pEdca->Aifsn[i]));
				}
			}

			/*APCwmin*/
			edcaptr = ptmpStr[2];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Cwmin[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmin[%d]=%d\n", i, pEdca->Cwmin[i]));
				}
			}

			/*APCwmax*/
			edcaptr = ptmpStr[3];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Cwmax[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmax[%d]=%d\n", i, pEdca->Cwmax[i]));
				}
			}

			/*APTxop*/
			edcaptr = ptmpStr[4];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Txop[i] = (USHORT) os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APTxop[%d]=%d\n", i, pEdca->Txop[i]));
				}
			}

			/*APACM*/
			edcaptr = ptmpStr[5];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->bACM[i] = (BOOLEAN) os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APACM[%d]=%d\n", i, pEdca->bACM[i]));
				}
			}
		}
	}

	/*AckPolicy*/
	for (i = 0 ; i < pAd->ApCfg.BssidNum; i++) {
		snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", i);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			wdev = &pAd->ApCfg.MBSSID[i].wdev;

			os_zero_mem(ack_policy, WMM_NUM_OF_AC);
			for (j = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr; edcaptr = rstrtok(NULL, ";"), j++)
				ack_policy[j] = (USHORT) simple_strtol(edcaptr, 0, 10);

			wlan_config_set_ack_policy(wdev, ack_policy);
		}
	}
}

static void rtmp_read_bss_edca_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr, *edcaptr, tok_str[16];
	INT	i = 0, j = 0;
	RTMP_STRING *ptmpStr[6];
	struct _EDCA_PARM *pBssEdca = NULL;

	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		snprintf(tok_str, sizeof(tok_str), "BSSEdca%d", j);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[j].wdev);

			if (!pBssEdca) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS[%d]: Invalid pBssEdca\n", j));
				return;
			}

			for (i = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr; edcaptr = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = edcaptr;

			if (i != 5) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Input parameter incorrect\n"));
				return;
			}

			/*BSSAifsn*/
			edcaptr = ptmpStr[0];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]));
				}
			}

			/*BSSCwmin*/
			edcaptr = ptmpStr[1];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]));
				}
			}

			/*BSSCwmax*/
			edcaptr = ptmpStr[2];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]));
				}
			}

			/*BSSTxop*/
			edcaptr = ptmpStr[3];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]));
				}
			}

			/*BSSACM*/
			edcaptr = ptmpStr[4];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]));
				}
			}
		}
	}
}

static void rtmp_read_ap_wmm_parms_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT	i = 0, j = 0;
	struct _EDCA_PARM *pBssEdca = NULL;

	/*WmmCapable*/
	if (RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE)) {
		BOOLEAN bEnableWmm = FALSE;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (os_str_tol(macptr, 0, 10) != 0) {
				pAd->ApCfg.MBSSID[i].wdev.bWmmCapable = TRUE;
				bEnableWmm = TRUE;
			} else
				pAd->ApCfg.MBSSID[i].wdev.bWmmCapable = FALSE;

			if (bEnableWmm) {
				pAd->CommonCfg.APEdcaParm[0].bValid = TRUE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf, TRUE);
				else
					wlan_config_set_edca_valid(&pAd->ApCfg.MBSSID[i].wdev, TRUE);
			} else {
				pAd->CommonCfg.APEdcaParm[0].bValid = FALSE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf, FALSE);
				else
					wlan_config_set_edca_valid(&pAd->ApCfg.MBSSID[i].wdev, FALSE);
			}

			pAd->ApCfg.MBSSID[i].bWmmCapableOrg = \
												  pAd->ApCfg.MBSSID[i].wdev.bWmmCapable;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WmmCapable=%d\n", i,
					 pAd->ApCfg.MBSSID[i].wdev.bWmmCapable));
		}
	}

	/*New WMM Parameter*/
	rtmp_read_ap_edca_from_file(pAd, tmpbuf, buffer);

	/*DLSCapable*/
	if (RTMPGetKeyParameter("DLSCapable", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
				pAd->ApCfg.MBSSID[i].bDLSCapable = TRUE;
			else /*Disable*/
				pAd->ApCfg.MBSSID[i].bDLSCapable = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) DLSCapable=%d\n", i,
					 pAd->ApCfg.MBSSID[i].bDLSCapable));
		}
	}

	/*APAifsn*/
	if (RTMPGetKeyParameter("APAifsn", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Aifsn[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Aifsn[i]));
		}
	}

	/*APCwmin*/
	if (RTMPGetKeyParameter("APCwmin", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Cwmin[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmin[i]));
		}
	}

	/*APCwmax*/
	if (RTMPGetKeyParameter("APCwmax", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Cwmax[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmax[i]));
		}
	}

	/*APTxop*/
	if (RTMPGetKeyParameter("APTxop", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Txop[i] = (USHORT) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Txop[i]));
		}
	}

	/*APACM*/
	if (RTMPGetKeyParameter("APACM", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].bACM[i] = (BOOLEAN) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APACM[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].bACM[i]));
		}
	}

	/* Apply default (BSS) WMM Parameter */
	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS[%d]:\n", j));
		pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[j].wdev);

		if (!pBssEdca)
			continue;

		/*BSSAifsn*/
		if (RTMPGetKeyParameter("BSSAifsn", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]));
			}
		}

		/*BSSCwmin*/
		if (RTMPGetKeyParameter("BSSCwmin", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]));
			}
		}

		/*BSSCwmax*/
		if (RTMPGetKeyParameter("BSSCwmax", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]));
			}
		}

		/*BSSTxop*/
		if (RTMPGetKeyParameter("BSSTxop", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]));
			}
		}

		/*BSSACM*/
		if (RTMPGetKeyParameter("BSSACM", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]));
			}
		}
	}

	/*Apply new (BSS) WMM Parameter*/
	rtmp_read_bss_edca_from_file(pAd, tmpbuf, buffer);

	/*AckPolicy*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.AckPolicy[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]));
		}

		wlan_config_set_ack_policy_all(&pAd->wpf, pAd->CommonCfg.AckPolicy);
	}

#ifdef UAPSD_SUPPORT

	/*APSDCapable*/
	if (RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE)) {

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i < HW_BEACON_MAX_NUM) {
				pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable = \
						(UCHAR) os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APSDCapable[%d]=%d\n", i,
						 pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable));
			}
		}

		if (i == 1) {
			/*
				Old format in UAPSD settings: only 1 parameter
				i.e. UAPSD for all BSS is enabled or disabled.
			*/
			for (i = 1; i < HW_BEACON_MAX_NUM; i++) {
				pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable =
					pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APSDCapable[%d]=%d\n", i,
						 pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable));
			}
		}

#ifdef APCLI_SUPPORT

		if (pAd->ApCfg.FlgApCliIsUapsdInfoUpdated == FALSE) {
			/*
				Backward:
				All UAPSD for AP Client interface is same as MBSS0
				when we can not find "ApCliAPSDCapable".
				When we find "ApCliAPSDCapable" hereafter, we will over-write.
			*/
			for (i = 0; i < MAX_APCLI_NUM; i++) {
				pAd->ApCfg.ApCliTab[i].wdev.UapsdInfo.bAPSDCapable = \
						pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("default ApCliAPSDCapable[%d]=%d\n",
						 i, pAd->ApCfg.ApCliTab[i].wdev.UapsdInfo.bAPSDCapable));
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* UAPSD_SUPPORT */
}
#ifdef DOT11U_INTERWORKING_IE_SUPPORT
static BOOLEAN  rtmp_read_one_ap_InterWorkingIE_param(IN PRTMP_ADAPTER pAd,
		INTERWORKING_IE *pIe, char *buffer)
{
	UCHAR	*CurP = NULL;
	UCHAR	value;
	INT Idx = 0;
	BOOLEAN bValid = FALSE;

	if ((buffer[0] == '\0') || (buffer[0] == '\n'))
		return FALSE;

	while ((CurP = strsep((char **)&buffer, "-")) != NULL) {
		value = (UCHAR) os_str_tol(CurP, 0, 10);
		if (Idx == 0) {
			bValid = TRUE;
			if ((value > 15) || ((value > 5) && (value < 14)))
				bValid = FALSE;
			else
				pIe->AccessNwType = value;
		} else if ((Idx >= 1) && (Idx <= 4)) {
			if (value > 1)
				bValid = FALSE;
			else {
				switch (Idx) {
				case 1:
					pIe->Internet = value;
					break;
				case 2:
					pIe->ASRA = value;
					break;
				case 3:
					pIe->ESR = value;
					break;
				case 4:
					pIe->UESA = value;
					break;
				}
			}
		} else
			bValid = FALSE;

		Idx++;
		if (bValid == FALSE)
			break;
	}
	return bValid;
}
static void rtmp_read_ap_InterWorkingIE_param_from_file(IN  PRTMP_ADAPTER pAd,
		char *tmpbuf, char *buffer)
{
	INT	i;
	RTMP_STRING tok_str[32];
	INTERWORKING_IE	InterworkingIe;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		NdisMoveMemory(&InterworkingIe, &pAd->ApCfg.MBSSID[i].InterWorkingIe, sizeof(InterworkingIe));
		pAd->ApCfg.MBSSID[i].bEnableInterworkingIe = FALSE;
		snprintf(tok_str, sizeof(tok_str), "InterWorkingIe%d", i + 1);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, buffer, TRUE)) {
			if (rtmp_read_one_ap_InterWorkingIE_param(pAd, &InterworkingIe, tmpbuf) == TRUE) {
				pAd->ApCfg.MBSSID[i].bEnableInterworkingIe = TRUE;
				NdisMoveMemory(&pAd->ApCfg.MBSSID[i].InterWorkingIe, &InterworkingIe,
				 sizeof(InterworkingIe));
			}
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF %d - InterworkingIE %s\n",
		 i, pAd->ApCfg.MBSSID[i].bEnableInterworkingIe ? "ON" : "OFF"));
	}
}
#endif /* DOT11U_INTERWORKING_IE_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */

INT rtmp_band_index_get_by_order(struct _RTMP_ADAPTER *pAd, UCHAR order)
{
	INT ret = DBDC_BAND0;
#ifdef MULTI_PROFILE

	if (is_multi_profile_enable(pAd)) {
#ifdef DEFAULT_5G_PROFILE

		if (order == 0)
			ret = DBDC_BAND1;

		if (order == 1)
			ret = DBDC_BAND0;

#else /*DEFAULT_5G_PROFILE*/

		if (order == 0)
			ret = DBDC_BAND0;

		if (order == 1)
			ret = DBDC_BAND1;

#endif /*DEFAULT_5G_PROFILE*/
	} else {
		if (order == 0)
			ret = DBDC_BAND0;

		if (order == 1)
			ret = DBDC_BAND1;
	}

#else /*MULTI_PROFILE*/

	if (order == 0)
		ret = DBDC_BAND0;

	if (order == 1)
		ret = DBDC_BAND1;

#endif /*MULTI_PROFILE*/
	return ret;
}


static UCHAR band_order_check(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR order)
{
	UCHAR ret = 0;

	if (pAd->CommonCfg.dbdc_mode) {
#ifdef MULTI_PROFILE

		if (is_multi_profile_enable(pAd)) {
#ifdef DEFAULT_5G_PROFILE

			if (((order == 0) && WMODE_CAP_5G(wdev->PhyMode))
				|| ((order == 1) && WMODE_CAP_2G(wdev->PhyMode)))
				ret = 1;

#else /*DEFAULT_5G_PROFILE*/

			if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
				|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
				ret = 1;

#endif /*DEFAULT_5G_PROFILE*/
		} else {
			if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
				|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
				ret = 1;
		}

#else /*MULTI_PROFILE*/

		if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
			|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
			ret = 1;

#endif /*MULTI_PROFILE*/
	} else
		ret = 1;

	return ret;
}

static struct wifi_dev *get_curr_wdev(struct _RTMP_ADAPTER *pAd, UCHAR idx)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (idx < MAX_MBSSID_NUM(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSID[%d]\n", idx));
			return &pAd->ApCfg.MBSSID[idx].wdev;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return NULL;
}

static void read_frag_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 frag_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("FragThreshold", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			frag_thld = os_str_tol(macptr, 0, 10);

			if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
				frag_thld = MAX_FRAG_THRESHOLD;
			else if (frag_thld % 2 == 1)
				frag_thld -= 1;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("FragThreshold[%d]=%d\n", i, frag_thld));

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_frag_thld(wdev, frag_thld);
		}
	}
}

static VOID read_rts_pkt_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UCHAR rts_pkt_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("RTSPktThreshold", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			rts_pkt_thld = (UCHAR)os_str_tol(macptr, 0, 10);

			if ((rts_pkt_thld < 1) || (rts_pkt_thld > MAX_RTS_PKT_THRESHOLD))
				rts_pkt_thld = MAX_RTS_PKT_THRESHOLD;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("profile: RTSPktThreshold[%d]=%d\n", i, rts_pkt_thld));

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_rts_pkt_thld(wdev, rts_pkt_thld);
		}
	}
}

static VOID read_rts_len_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 rts_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;


	if (RTMPGetKeyParameter("RTSThreshold", tmpbuf, 128, buf, FALSE)) {
		if (pAd->CommonCfg.dbdc_mode) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				rts_thld = (UINT32)os_str_tol(macptr, 0, 10);
				if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
					rts_thld = MAX_RTS_THRESHOLD;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("profile: RTSThreshold[%d]=%d\n", i, rts_thld));
				wdev = get_curr_wdev(pAd, i);
				if (wdev)
					wlan_config_set_rts_len_thld(wdev, rts_thld);
			}
		} else {
#ifdef CONFIG_AP_SUPPORT
			UCHAR mbss_idx = 0;
#endif
			rts_thld = (UINT32)os_str_tol(tmpbuf, 0, 10);
			if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
				rts_thld = MAX_RTS_THRESHOLD;
#ifdef CONFIG_AP_SUPPORT
			for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
				struct wifi_dev *mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
				wlan_config_set_rts_len_thld(mbss_wdev, rts_thld);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("mbss[idx=%d], RTSThreshold[%d]=%d\n", mbss_idx, mbss_idx,
						 rts_thld));
			}
#endif
		}
	}
}

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
static void rtmp_read_rdd_threshold_parms_from_file(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	RTMP_STRING	tok_str[32];
	UINT8 ucRegionIdx = 0, ucRTIdx = 0, ucMaxNoOfRegion = 0;
	INT32 i4Recv = 0;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;
	PDFS_PULSE_THRESHOLD_PARAM prPulseThresholdParam = NULL;
	PSW_RADAR_TYPE_T prRadarType = NULL;
	DFS_PULSE_THRESHOLD_PARAM rPulseThresholdParam = {0};
	SW_RADAR_TYPE_T rRadarType = {0};
	RTMP_STRING *pRDRegionStr[] = {
					"CE",
					"FCC",
					"JAP",
					"ALL"
					};

	ucMaxNoOfRegion = sizeof(pRDRegionStr)/sizeof(RTMP_STRING *);

	for (ucRegionIdx = 0; ucRegionIdx < ucMaxNoOfRegion; ucRegionIdx++) {
		for (ucRTIdx = 0; ucRTIdx < RT_NUM; ucRTIdx++) {
			snprintf(tok_str, sizeof(tok_str), "RTParam%s%d", pRDRegionStr[ucRegionIdx], ucRTIdx);
			if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
				RTMPZeroMemory((VOID *)&rRadarType, sizeof(SW_RADAR_TYPE_T));
				prRadarThresholdParam = &g_arRadarThresholdParam[ucRegionIdx];
				i4Recv = sscanf(tmpbuf, "%hhu-%hhu-%hhu-%hhu-%hhu-%hhu-%hhu-%u-%u-%hhu-%hhu-%hhu-%hhu-%hhu",
									&(rRadarType.ucRT_ENB), &(rRadarType.ucRT_STGR),
									&(rRadarType.ucRT_CRPN_MIN), &(rRadarType.ucRT_CRPN_MAX),
									&(rRadarType.ucRT_CRPR_MIN), &(rRadarType.ucRT_PW_MIN),
									&(rRadarType.ucRT_PW_MAX), &(rRadarType.u4RT_PRI_MIN),
									&(rRadarType.u4RT_PRI_MAX), &(rRadarType.ucRT_CRBN_MIN),
									&(rRadarType.ucRT_CRBN_MAX), &(rRadarType.ucRT_STGPN_MIN),
									&(rRadarType.ucRT_STGPN_MAX), &(rRadarType.ucRT_STGPR_MIN));

				if (i4Recv != 14) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s():%s Format Error! Please enter in the following format\n"
						"RT_ENB-RT_STGR-RT_CRPN_MIN-RT_CRPN_MAX-RT_CRPR_MIN-RT_PW_MIN-RT_PW_MAX-"
						"RT_PRI_MIN-RT_PRI_MAX-RT_CRBN_MIN-RT_CRBN_MAX-RT_STGPN_MIN-RT_STGPN_MAX-RT_STGPR_MIN\n",
						__func__, tok_str));
					continue;
				}
				prRadarThresholdParam->afgSupportedRT[ucRTIdx] = 1;
				prRadarType = &prRadarThresholdParam->arRadarType[ucRTIdx];
				NdisCopyMemory(prRadarType, &rRadarType, sizeof(SW_RADAR_TYPE_T));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("%s():ucRTIdx = %d\n RT_ENB = %d\n RT_STGR = %d\n "
							"RT_CRPN_MIN = %d\n RT_CRPN_MAX = %d\n RT_CRPR_MIN = %d\n "
							"RT_PW_MIN = %d\n RT_PW_MAX =%d\n RT_PRI_MIN = %d\n "
							"RT_PRI_MAX = %d\n RT_CRBN_MIN = %d\n RT_CRBN_MAX = %d\n"
							"RT_STGPN_MIN = %d\n RT_STGPN_MAX = %d\n RT_STGPR_MIN = %d\n ",
							__func__, ucRTIdx, prRadarType->ucRT_ENB, prRadarType->ucRT_STGR,
							prRadarType->ucRT_CRPN_MIN, prRadarType->ucRT_CRPN_MAX,
							prRadarType->ucRT_CRPR_MIN, prRadarType->ucRT_PW_MIN,
							prRadarType->ucRT_PW_MAX, prRadarType->u4RT_PRI_MIN,
							prRadarType->u4RT_PRI_MAX, prRadarType->ucRT_CRBN_MIN,
							prRadarType->ucRT_CRBN_MAX, prRadarType->ucRT_STGPN_MIN,
							prRadarType->ucRT_STGPN_MAX, prRadarType->ucRT_STGPR_MIN));
			}
		}
	}

	if (RTMPGetKeyParameter("RadarPulseThresholdParam", tmpbuf, 128, buffer, TRUE)) {
		i4Recv = sscanf(tmpbuf, "%u-%u-%u-%u-%u-%u-%u",
					&(rPulseThresholdParam.u4PulseWidthMax), &(rPulseThresholdParam.i4PulsePwrMax),
					&(rPulseThresholdParam.i4PulsePwrMin), &(rPulseThresholdParam.u4PRI_MIN_STGR),
					&(rPulseThresholdParam.u4PRI_MAX_STGR), &(rPulseThresholdParam.u4PRI_MIN_CR),
					&(rPulseThresholdParam.u4PRI_MAX_CR));
		if (i4Recv != 7) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s():RadarPulseThresholdParam Format Error! Please enter in the following format\n"
					"MaxPulseWidth-MaxPulsePower-MinPulsePower-"
					"MinPRISTGR-MaxPRISTGR-MinPRICR-MaxPRICR\n", __func__));
			return;
		}
		for (ucRegionIdx = 0; ucRegionIdx < ucMaxNoOfRegion; ucRegionIdx++) {
			prPulseThresholdParam = &g_arRadarThresholdParam[ucRegionIdx].rPulseThresholdParam;
			NdisCopyMemory(prPulseThresholdParam, &rPulseThresholdParam, sizeof(DFS_PULSE_THRESHOLD_PARAM));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s():RegionIdx = %d\nMaxPulseWidth = %d\nMaxPulsePower = %d\nMinPulsePower = %d\n"
				"MinPRISTGR = %d\nMaxPRISTGR = %d\nMinPRICR = %d\nMaxPRICR = %d\n",
				__func__, ucRegionIdx,
				prPulseThresholdParam->u4PulseWidthMax,
				prPulseThresholdParam->i4PulsePwrMax,
				prPulseThresholdParam->i4PulsePwrMin,
				prPulseThresholdParam->u4PRI_MIN_STGR,
				prPulseThresholdParam->u4PRI_MAX_STGR,
				prPulseThresholdParam->u4PRI_MIN_CR,
				prPulseThresholdParam->u4PRI_MAX_CR));
		}
	}
}
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */


#ifdef DOT11_VHT_AC
static VOID read_vht_sgi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_SGI", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_sgi(wdev, val);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("VHT: Short GI for 80Mhz/160Mhz  = %s\n",
						 (val == GI_800) ? "Disabled" : "Enable"));
			}
		}
	}
}

static VOID read_vht_stbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_STBC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_stbc(wdev, val);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("VHT: STBC supported = %d\n", val));
			}
		}
	}
}

static VOID read_vht_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_LDPC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_ldpc(wdev, val);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("VHT: LDPC supported = %d\n", val));
			}
		}
	}
}

static VOID read_vht_bw_sig(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	UCHAR *bwsig_str[] = {"NONE", "STATIC", "DYNAMIC"};

	if (RTMPGetKeyParameter("VHT_BW_SIGNAL", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_bw_sig(wdev, val);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("VHT: BW SIGNALING = %s\n", bwsig_str[val]));
			}
		}
	}
}

static VOID read_vht_param_from_file(struct _RTMP_ADAPTER *pAd,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	long Value;
	UCHAR vht_bw;
	UCHAR cen_ch_2;

	/* Channel Width */
	if (RTMPGetKeyParameter("VHT_BW", tmpbuf, 25, buf, TRUE)) {
		if (pAd->CommonCfg.dbdc_mode) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				Value = os_str_tol(macptr, 0, 10);

				if (Value <= VHT_BW_8080)
					vht_bw = Value;
				else
					vht_bw = VHT_BW_2040;

				if (vht_bw > VHT_BW_80)
					vht_bw = VHT_BW_80;

				wdev = get_curr_wdev(pAd, i);
				if (wdev)
					wlan_config_set_vht_bw(wdev, vht_bw);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
				if (IS_SUPPORT_V10_DFS(pAd) && vht_bw == VHT_BW_2040) {
					/* Boot Time HT BW Update when VHT BW if VHT2040 */
					wlan_config_set_ht_bw(wdev, HT_BW_20);
#ifdef MCAST_RATE_SPECIFIC
					pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
					pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#ifdef MCAST_BCAST_RATE_SET_SUPPORT
					pAd->CommonCfg.BCastPhyMode.field.BW = pAd->CommonCfg.MCastPhyMode.field.BW;
					pAd->CommonCfg.BCastPhyMode_5G.field.BW = pAd->CommonCfg.MCastPhyMode_5G.field.BW;
#endif /* MCAST_BCAST_RATE_SET_SUPPORT */
#endif /* MCAST_RATE_SPECIFIC */
				}
#endif
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								("wdev[%d] VHT: Channel Width = %s MHz\n", i,
								 VhtBw2Str(vht_bw)));
			}
		} else {
#ifdef CONFIG_AP_SUPPORT
			UCHAR mbss_idx = 0;
#endif
			Value = os_str_tol(tmpbuf, 0, 10);
			if (Value <= VHT_BW_8080)
				vht_bw = Value;
			else
				vht_bw = VHT_BW_2040;
#ifdef CONFIG_AP_SUPPORT
			/* Set for all MBSS */
			for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
				struct wifi_dev *mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;

				wlan_config_set_vht_bw(mbss_wdev, vht_bw);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("mbss[%d] VHT: Channel Width = %s MHz\n", mbss_idx,
						 VhtBw2Str(vht_bw)));
			}
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if (IS_SUPPORT_V10_DFS(pAd) && vht_bw == VHT_BW_2040) {
				/* Boot Time HT BW Update when VHT BW if VHT2040 */
				wlan_config_set_ht_bw(wdev, HT_BW_20);
#ifdef MCAST_RATE_SPECIFIC
				pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
				pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#ifdef MCAST_BCAST_RATE_SET_SUPPORT
				pAd->CommonCfg.BCastPhyMode.field.BW = pAd->CommonCfg.MCastPhyMode.field.BW;
				pAd->CommonCfg.BCastPhyMode_5G.field.BW = pAd->CommonCfg.MCastPhyMode_5G.field.BW;
#endif /* MCAST_BCAST_RATE_SET_SUPPORT */
#endif /* MCAST_RATE_SPECIFIC */
			}
#endif
		}
	}

	/* VHT_SGI */
	read_vht_sgi(pAd, tmpbuf, buf);
	/* VHT_STBC */
	read_vht_stbc(pAd, tmpbuf, buf);
	/* VHT_LDPC */
	read_vht_ldpc(pAd, tmpbuf, buf);
	/* VHT_BW_SIGNAL */
	read_vht_bw_sig(pAd, tmpbuf, buf);

	/* Disallow non-VHT connection */
	if (RTMPGetKeyParameter("VHT_DisallowNonVHT", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.bNonVhtDisallow = FALSE;
		else
			pAd->CommonCfg.bNonVhtDisallow = TRUE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: VHT_DisallowNonVHT = %d\n",
				 pAd->CommonCfg.bNonVhtDisallow));
	}

	/* VHT Secondary80 */
	if (RTMPGetKeyParameter("VHT_Sec80_Channel", tmpbuf, 25, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(tmpbuf, 0, 10);
			cen_ch_2 = vht_cent_ch_freq((UCHAR)Value, VHT_BW_80);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_cen_ch_2(wdev, cen_ch_2);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("wdev[%d] VHT: Secondary80 = %ld, Center = %d\n", i,
					  Value, cen_ch_2));
			}
	}

	/* 2.4G 256QAM */
	if (RTMPGetKeyParameter("G_BAND_256QAM", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.g_band_256_qam = (Value) ? TRUE : FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("VHT: G_BAND_256QAM = %ld\n", Value));
	}

#ifdef WFA_VHT_PF
	/* VHT highest Tx Rate with LGI */
	if (RTMPGetKeyParameter("VHT_TX_HRATE", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_tx_hrate = Value;
		else
			pAd->CommonCfg.vht_tx_hrate = 0;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: TX HighestRate = %d\n", pAd->CommonCfg.vht_tx_hrate));
	}

	if (RTMPGetKeyParameter("VHT_RX_HRATE", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_rx_hrate = Value;
		else
			pAd->CommonCfg.vht_rx_hrate = 0;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: RX HighestRate = %d\n", pAd->CommonCfg.vht_rx_hrate));
	}

	if (RTMPGetKeyParameter("VHT_MCS_CAP", tmpbuf, 25, buf, TRUE))
		set_vht_nss_mcs_cap(pAd, tmpbuf);

#endif /* WFA_VHT_PF */
}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
static VOID read_min_mpdu_start_space(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i = 0;
	UCHAR mpdu_density = 0;

	if (RTMPGetKeyParameter("HT_MpduDensity", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mpdu_density = os_str_tol(tmpbuf, 0, 10);
			if (mpdu_density > 7)
				mpdu_density = 4;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("HT: MPDU Density = %d\n", (INT) mpdu_density));

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_min_mpdu_start_space(wdev, mpdu_density);
		}
	}
}

static VOID read_ht_protect(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_protect_en = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_PROTECT", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_protect_en = os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("HT_PROTECT=%s\n", (ht_protect_en) ? "Enable" : "Disable"));
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_protect_en(wdev, ht_protect_en);
		}
	}
}

static VOID read_ht_gi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_gi = GI_400;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_GI", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_gi = os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("HT_GI = %s\n", (ht_gi == GI_400) ? "GI_400" : "GI_800"));
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_gi(wdev, ht_gi);
		}
	}
}

static VOID read_40M_intolerant(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht40_intolerant = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_40MHZ_INTOLERANT", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht40_intolerant = os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("HT: 40MHZ INTOLERANT = %d\n", ht40_intolerant));
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_40M_intolerant(wdev, ht40_intolerant);
		}
	}
}

static VOID read_ht_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_ldpc = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_LDPC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_ldpc = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_ldpc(wdev, ht_ldpc);
		}
	}
}

static VOID read_ht_stbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_stbc = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_STBC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_stbc = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_stbc(wdev, ht_stbc);
		}
	}
}

static VOID read_ht_mode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_mode = 0;
	INT i = 0;

	/* HT Operation Mode : Mixed Mode , Green Field*/
	if (RTMPGetKeyParameter("HT_OpMode", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_mode = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_mode(wdev, ht_mode);
		}
	}
}

static VOID read_txrx_stream_num(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	RTMP_STRING *macptr = NULL;
	UCHAR tx_stream = 0, rx_stream = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_TxStream", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			tx_stream = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_tx_stream(wdev, min(tx_stream, cap->max_nss));
		}
#ifdef CONFIG_AP_SUPPORT
		if ((pAd->ApCfg.BssidNum > 1) && (i < pAd->ApCfg.BssidNum)) {
			for (; i < pAd->ApCfg.BssidNum; i++) {
				wdev = get_curr_wdev(pAd, i);
				if (wdev)
					wlan_config_set_tx_stream(wdev, min(tx_stream, cap->max_nss));
			}
		}
#endif
	}

	if (RTMPGetKeyParameter("HT_RxStream", tmpbuf, 10, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			rx_stream = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_rx_stream(wdev, min(rx_stream, cap->max_nss));
		}
#ifdef CONFIG_AP_SUPPORT
		if ((pAd->ApCfg.BssidNum > 1) && (i < pAd->ApCfg.BssidNum)) {
			for (; i < pAd->ApCfg.BssidNum; i++) {
				wdev = get_curr_wdev(pAd, i);
				if (wdev)
					wlan_config_set_rx_stream(wdev, min(rx_stream, cap->max_nss));
			}
		}
#endif
	}
}

static VOID read_amsdu_enable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR amsdu_enable = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_AMSDU", tmpbuf, 25, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			amsdu_enable = os_str_tol(tmpbuf, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_amsdu_en(wdev, amsdu_enable);
		}
	}
#ifdef WFA_VHT_PF
	if (RTMPGetKeyParameter("FORCE_AMSDU", tmpbuf, 25, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > DBDC_BAND_NUM)
				break;

			amsdu_enable |= os_str_tol(tmpbuf, 0, 10);
		}
		pAd->force_amsdu = amsdu_enable;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("HT: FORCE A-MSDU = %s\n", (amsdu_enable) ? "Enable" : "Disable"));
	}
#endif /* WFA_VHT_PF */
}

static VOID read_mmps(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mmps = 0;
	INT i = 0, idx = 0;

	if (RTMPGetKeyParameter("HT_MIMOPSMode", tmpbuf, 25, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > DBDC_BAND_NUM)
				break;

			mmps = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: MIMOPS Mode  = %d\n", mmps));
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					wdev = &pAd->ApCfg.MBSSID[idx].wdev;
					if (band_order_check(pAd, wdev, i))
						wlan_config_set_mmps(wdev, mmps);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}
	}
}

static VOID read_ht_bw(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	UCHAR ht_bw;

	if (RTMPGetKeyParameter("HT_BW", tmpbuf, 25, buf, TRUE)) {
		ht_bw = os_str_tol(tmpbuf, 0, 10);
		wlan_config_set_ht_bw_all(&pAd->wpf, ht_bw);
#ifdef MCAST_RATE_SPECIFIC
		pAd->CommonCfg.MCastPhyMode.field.BW = ht_bw;
		pAd->CommonCfg.MCastPhyMode_5G.field.BW = ht_bw;
#ifdef MCAST_BCAST_RATE_SET_SUPPORT
		pAd->CommonCfg.BCastPhyMode.field.BW = pAd->CommonCfg.MCastPhyMode.field.BW;
		pAd->CommonCfg.BCastPhyMode_5G.field.BW = pAd->CommonCfg.MCastPhyMode_5G.field.BW;
#endif /* MCAST_BCAST_RATE_SET_SUPPORT */
#endif /* MCAST_RATE_SPECIFIC */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Channel Width = %s\n",
				 (ht_bw == HT_BW_40) ? "40 MHz" : "20 MHz"));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			RTMP_STRING *Bufptr;
			struct wifi_dev *wdev;
			INT i;

			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				ht_bw = os_str_tol(Bufptr, 0, 10);
				wlan_config_set_ht_bw(wdev, ht_bw);
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
static VOID RTMPOldBWCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer, BOOLEAN isVHT)
{
	UCHAR bw = 0;
	RTMP_STRING *Bufptr = NULL;
	struct wifi_dev *wdev = NULL;
	INT i = 0;

	if ((Buffer == NULL) || (IS_SUPPORT_V10_DFS(pAd) == FALSE))
		return;

#ifdef CONFIG_AP_SUPPORT
	/* V10 -- AP Mode Only */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		bw = os_str_tol(Buffer, 0, 10);

		/* Disallow Invalid Values */
		if ((!isVHT && bw > BW_40) || (isVHT && bw > BW_80)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Incorrect BW=%d\n",
				__func__, bw));
			return;
		}

		for (i = 0, Bufptr = rstrtok(Buffer, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
			Bufptr = rstrtok(NULL, ";"), i++) {
			bw = os_str_tol(Bufptr, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (!wdev || !IS_V10_OLD_CHNL_VALID(wdev))
				continue;

			if (isVHT) {
				wlan_config_set_vht_bw(wdev, bw);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("[%s] wdev[%d] VHT: Channel Width = %s MHz\n", __func__, i, VhtBw2Str(bw)));
			} else {
				wlan_config_set_ht_bw(wdev, bw);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Channel Width = %s\n",
					(bw == HT_BW_40) ? "40 MHz" : "20 MHz"));
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}
#endif


static VOID read_ht_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT Value = 0;
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	RTMP_STRING *Bufptr;
#endif /* CONFIG_AP_SUPPORT */

	/* HT_BW */
	read_ht_bw(pAd, tmpbuf, buf);
	/* Tx/Rx Stream */
	read_txrx_stream_num(pAd, tmpbuf, buf);
	/* HT_OpMode */
	read_ht_mode(pAd, tmpbuf, buf);
	/* HT_PROTECT */
	read_ht_protect(pAd, tmpbuf, buf);
	/* HT_GI */
	read_ht_gi(pAd, tmpbuf, buf);
	/* HT_LDPC */
	read_ht_ldpc(pAd, tmpbuf, buf);
	/* HT_STBC */
	read_ht_stbc(pAd, tmpbuf, buf);
	/* MPDU Density*/
	read_min_mpdu_start_space(pAd, tmpbuf, buf);
	/* 40_Mhz_Intolerant*/
	read_40M_intolerant(pAd, tmpbuf, buf);
	/* Tx A-MSUD */
	read_amsdu_enable(pAd, tmpbuf, buf);
	/* MMPS */
	read_mmps(pAd, tmpbuf, buf);

	if (RTMPGetKeyParameter("HT_BADecline", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.bBADecline = FALSE;
		else
			pAd->CommonCfg.bBADecline = TRUE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: BA Decline  = %s\n", (Value == 0) ? "Disable" : "Enable"));
	}


	if (RTMPGetKeyParameter("HT_AutoBA", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0) {
			pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
			pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
		} else {
			pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
			pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
		}

		pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
		pAd->CommonCfg.REGBACapability.field.Policy = pAd->CommonCfg.BACapability.field.Policy;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Auto BA  = %s\n", (Value == 0) ? "Disable" : "Enable"));
	}


	/* Reverse Direction Mechanism*/
	if (RTMPGetKeyParameter("HT_RDG", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value != 0 && IS_ASIC_CAP(pAd, fASIC_CAP_RDG))
			pAd->CommonCfg.bRdg = TRUE;
		else
			pAd->CommonCfg.bRdg = FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("HT: RDG = %s\n", (Value == 0) ? "Disable" : "Enable(+HTC)"));
	}


	/* Max Rx BA Window Size*/
	if (RTMPGetKeyParameter("HT_BAWinSize", tmpbuf, 25, buf, TRUE)) {
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value >= 1 && Value <= 64) {
			pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = min((UINT8)Value, pChipCap->RxBAWinSize);
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = min((UINT8)Value, pChipCap->RxBAWinSize);
#ifdef COEX_SUPPORT
			pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = min((UINT8)Value, pChipCap->TxBAWinSize);
			pAd->CommonCfg.BACapability.field.TxBAWinLimit = min((UINT8)Value, pChipCap->TxBAWinSize);
#endif /* COEX_SUPPORT */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: BA Windw Size = %d\n", min((UINT8)Value,
					 pChipCap->RxBAWinSize)));
		} else {
			pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = min((UINT8)64, pChipCap->RxBAWinSize);
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = min((UINT8)64, pChipCap->RxBAWinSize);
#ifdef COEX_SUPPORT
			pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = min((UINT8)64, pChipCap->TxBAWinSize);
			pAd->CommonCfg.BACapability.field.TxBAWinLimit = min((UINT8)64, pChipCap->TxBAWinSize);
#endif /* COEX_SUPPORT */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: BA Windw Size = %d\n", min((UINT8)64,
					 pChipCap->RxBAWinSize)));
		}
	}

	/* Fixed Tx mode : CCK, OFDM*/
	if (RTMPGetKeyParameter("FixedTxMode", tmpbuf, 25, buf, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && i < MAX_MBSSID_NUM(pAd));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				pAd->ApCfg.MBSSID[i].wdev.DesiredTransmitSetting.field.FixedTxMode =
					RT_CfgSetFixedTxPhyMode(Bufptr);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) Fixed Tx Mode = %d\n", i,
						 pAd->ApCfg.MBSSID[i].wdev.DesiredTransmitSetting.field.FixedTxMode));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}


	if (RTMPGetKeyParameter("HT_EXTCHA", tmpbuf, 25, buf, TRUE)) {
		struct wifi_dev *wdev;
		UCHAR ext_cha;
#ifdef CONFIG_AP_SUPPORT

		for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
			 Bufptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(Bufptr, 0, 10);
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
			}

			if (Value == 0)
				ext_cha = EXTCHA_BELOW;
			else
				ext_cha = EXTCHA_ABOVE;

			wlan_config_set_ext_cha(wdev, ext_cha);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT: WDEV[%x] Ext Channel = %s\n", i,
					 (Value == 0) ? "BELOW" : "ABOVE"));
		}

		ext_cha = wlan_config_get_ext_cha(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[i].wdev;

			if (wlan_config_get_ext_cha(wdev) == EXTCHA_NOASSIGN) {
				wlan_config_set_ext_cha(wdev, ext_cha);
			}
		}

#endif /*CONFIG_AP_SUPPORT*/
	}

	/* MSC*/
	if (RTMPGetKeyParameter("HT_MCS", tmpbuf, 50, buf, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && i < MAX_MBSSID_NUM(pAd));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;
				Value = os_str_tol(Bufptr, 0, 10);

				if (Value >= MCS_0 && Value <= MCS_32)
					wdev->DesiredTransmitSetting.field.MCS = Value;
				else
					wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) HT: MCS = %s(%d)\n",
						 i, (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : "Fixed"),
						 wdev->DesiredTransmitSetting.field.MCS));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}


#ifdef GREENAP_SUPPORT

	/*Green AP*/
	if (RTMPGetKeyParameter("GreenAP", tmpbuf, 10, buf, TRUE)) {
		struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			greenap_set_capability(greenap, FALSE);
		else
			greenap_set_capability(greenap, TRUE);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT: greenap_cap = %d\n", greenap_get_capability(greenap)));
	}

#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT

	/* PcieAspm */
	if (RTMPGetKeyParameter("PcieAspm", tmpbuf, 10, buf, TRUE)) {

		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
		else
			set_pcie_aspm_dym_ctrl_cap(pAd, TRUE);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ChipI=%x, Value=%d, pcie_aspm in profile=%d\n",
			pAd->ChipID,
			Value,
			get_pcie_aspm_dym_ctrl_cap(pAd)));
	}

#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

	/* HT_DisallowTKIP*/
	if (RTMPGetKeyParameter("HT_DisallowTKIP", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 1)
			pAd->CommonCfg.HT_DisallowTKIP = TRUE;
		else
			pAd->CommonCfg.HT_DisallowTKIP = FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Disallow TKIP mode = %s\n",
				 (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "ON" : "OFF"));
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (RTMPGetKeyParameter("OBSSScanParam", tmpbuf, 32, buf, TRUE)) {
		int ObssScanValue, idx;
		RTMP_STRING *macptr;

		for (idx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), idx++) {
			ObssScanValue = os_str_tol(macptr, 0, 10);

			switch (idx) {
			case 0:
				if (ObssScanValue < 5 || ObssScanValue > 1000)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue));
				else {
					pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000*/
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n",
							 ObssScanValue));
				}

				break;

			case 1:
				if (ObssScanValue < 10 || ObssScanValue > 1000)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue));
				else {
					pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000*/
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanActiveDwell=%d\n",
							 ObssScanValue));
				}

				break;

			case 2:
				pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second*/
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n",
						 ObssScanValue));
				break;

			case 3:
				if (ObssScanValue < 200 || ObssScanValue > 10000)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue));
				else {
					pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000*/
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n",
							 ObssScanValue));
				}

				break;

			case 4:
				if (ObssScanValue < 20 || ObssScanValue > 10000)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue));
				else {
					pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000*/
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n",
							 ObssScanValue));
				}

				break;

			case 5:
				pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
						 ObssScanValue));
				break;

			case 6:
				pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage*/
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
						 ObssScanValue));
				break;
			}
		}

		if (idx != 7) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Wrong OBSSScanParamtetrs format in dat file!!!!! Use default value.\n"));
			pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
			pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
			pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
			pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
			pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
			pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
		}

		pAd->CommonCfg.Dot11BssWidthChanTranDelay = ((UINT32)pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
				(UINT32)pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n",
				 pAd->CommonCfg.Dot11BssWidthChanTranDelay));
	}

	if (RTMPGetKeyParameter("HT_BSSCoexistence", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bBssCoexEnable = ((Value == 1) ? TRUE : FALSE);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: 20/40 BssCoexSupport = %s\n",
				 (pAd->CommonCfg.bBssCoexEnable == TRUE) ? "ON" : "OFF"));
	}

	if (RTMPGetKeyParameter("HT_BSSCoexApCntThr", tmpbuf, 25, buf, TRUE)) {
		pAd->CommonCfg.BssCoexApCntThr = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: 20/40 BssCoexApCntThr = %d\n",
				 pAd->CommonCfg.BssCoexApCntThr));
	}

#endif /* DOT11N_DRAFT3 */

	if (RTMPGetKeyParameter("BurstMode", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bRalinkBurstMode = ((Value == 1) ? 1 : 0);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: RaBurstMode= %d\n", pAd->CommonCfg.bRalinkBurstMode));
	}

#endif /* DOT11_N_SUPPORT */

	if (RTMPGetKeyParameter("TXRX_RXV_ON", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bTXRX_RXV_ON = Value;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TXRX_RXV_ON = %s\n", (Value == 1) ? "ON" : "OFF"));
	}
}
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
static VOID read_etxbf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ETxBfEnCond = SUBF_OFF;
	INT i = 0;

	if (RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ETxBfEnCond = os_str_tol(macptr, 0, 10);
			pAd->CommonCfg.ETxBfEnCond |= ETxBfEnCond;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: ETxBfEnCond = %d\n", __func__, ETxBfEnCond));
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BSSID[%d]\n", __func__, i));
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
			if (wdev) {
				wlan_config_set_etxbf(wdev, ETxBfEnCond);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s: MBSS[%d] ETxBfEnCond = %d\n", __func__, i, ETxBfEnCond));
			}
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: More BSSID[%d]\n", __func__, i));
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				if (wdev) {
					wlan_config_set_etxbf(wdev, ETxBfEnCond);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("%s: More MBSS[%d] ETxBfEnCond = %d\n", __func__, i, ETxBfEnCond));
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}

#ifdef DSCP_PRI_SUPPORT
static VOID read_dscp_pri_param(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	RTMP_STRING tok_str[32];
	UCHAR i = 0, bss_idx = 0;

	if (RTMPGetKeyParameter("DscpPriMapEnable", tmpbuf, 10, pBuffer, TRUE)) {
		if (simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
			pAd->ApCfg.DscpPriMapSupport = 1;
		else
			pAd->ApCfg.DscpPriMapSupport = 0;
	}

	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		snprintf(tok_str, sizeof(tok_str), "DscpPriMapBss%d", bss_idx);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 512, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				RTMP_STRING	*this_char;
				UINT8	dscpValue;
				INT8 pri;

				if (i > 63)
					break;

				this_char = strsep((char **)&macptr, ":");
				if (this_char == NULL) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s value not defined for Dscp and Priority\n", __func__));
					break;
				}

				dscpValue = simple_strtol(this_char, 0, 10);
				if ((dscpValue < 0) || (dscpValue > 63)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s Invalid Dscp Value Valid Value between 0 to 63\n", __func__));
					break;
				}
				if (macptr == NULL) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s Priority not defined for Dscp %d\n", __func__, dscpValue));
					break;
				}
				pri = simple_strtol(macptr, 0, 10);
				if (pri < -1  || pri > 7) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s Invalid Priority value Valid value between 0 to 7\n", __func__));
					break;
				}

				if (pri == 0)
					pri = 3;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s Setting Pri %d for Dscp=%d\n", __func__, pri, dscpValue));
				pAd->ApCfg.MBSSID[bss_idx].dscp_pri_map[dscpValue] = pri;
			}
		}
	}
}
#endif

static VOID read_txbf_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
    /* ETxBfEnCond */
    read_etxbf(pAd, tmpbuf, buf);
}
#endif /* TXBF_SUPPORT */



void RTMPSetCountryCode(RTMP_ADAPTER *pAd, RTMP_STRING *CountryCode)
{
	NdisMoveMemory(pAd->CommonCfg.CountryCode, CountryCode, 2);
	pAd->CommonCfg.CountryCode[2] = ' ';

	if (strlen((RTMP_STRING *) pAd->CommonCfg.CountryCode) != 0)
		pAd->CommonCfg.bCountryFlag = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode=%s\n", pAd->CommonCfg.CountryCode));
}

NDIS_STATUS	RTMPSetPreProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;
	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	/*WHNAT*/
#ifdef WHNAT_SUPPORT

	if (RTMPGetKeyParameter("WHNAT", tmpbuf, 10, pBuffer, TRUE))
		RTMPWHNATCfg(pAd, tmpbuf);

#endif /*WHNAT_SUPPORT*/
	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;
	RTMP_STRING *macptr = NULL;
	INT		i = 0, retval;
	CHAR    *value = 0;
#ifdef CONFIG_AP_SUPPORT
	RTMP_STRING tok_str[16];
	UCHAR BssidCountSupposed = 0;
	BOOLEAN bSSIDxIsUsed = FALSE;
#endif
#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;
	UCHAR ucRDDurRegion;
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

	struct _RTMP_CHIP_CAP *cap;

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	/*get chip cap for usage*/
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	/* If profile parameter is set, channel lists of HW bands need to be reset*/
	hc_init_ChCtrl(pAd);

#ifdef CONFIG_AP_SUPPORT
	/* If profile parameter is set, ACS parameters of HW bands need to be reset*/
	hc_init_ACSChCtrl(pAd);
#endif

	do {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MBSS_SUPPORT

			/*BSSIDNum; This must read first of other multiSSID field, so list this field first in configuration file*/
			if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, pBuffer, TRUE)) {
				pAd->ApCfg.BssidNum = (UCHAR) os_str_tol(tmpbuf, 0, 10);

				if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd)) {
					pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("BssidNum=%d(MAX_MBSSID_NUM is %d)\n",
							  pAd->ApCfg.BssidNum, MAX_MBSSID_NUM(pAd)));
				} else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BssidNum=%d\n", pAd->ApCfg.BssidNum));
			}

#else
			pAd->ApCfg.BssidNum = 1;
#endif /* MBSS_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		/* set file parameter to portcfg*/
		if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetMacAddress(pAd, tmpbuf, 0);

			if (retval)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacAddress = %02x:%02x:%02x:%02x:%02x:%02x\n",
						 PRINT_MAC(pAd->CurrentAddress)));
		}

#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MBSS_SUPPORT

			if (IS_MT7615(pAd) || (IS_MT7637(pAd)) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd)) {
				/* for MT7615, we could assign extend BSSID mac address by ourself. */
				/* extend index starts from 1.*/
				for (i = 1; i < pAd->ApCfg.BssidNum; i++) {
					snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);

					if (RTMPGetKeyParameter(tok_str, tmpbuf, 25, pBuffer, TRUE)) {
						retval = RT_CfgSetMacAddress(pAd, tmpbuf, i);

						if (retval)
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MacAddress%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
									 i, PRINT_MAC(pAd->ExtendMBssAddr[i])));
					}
				}
			}

#endif /* MBSS_SUPPORT */
		}
#endif /*CONFIG_AP_SUPPORT*/
#endif /*MT_MAC*/

		/*CountryRegion*/
		if (RTMPGetKeyParameter("CountryRegion", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryRegion=%d\n", pAd->CommonCfg.CountryRegion));
		}

		/*CountryRegionABand*/
		if (RTMPGetKeyParameter("CountryRegionABand", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryRegionABand=%d\n", pAd->CommonCfg.CountryRegionForABand));
		}

#ifdef BB_SOC
#ifdef RTMP_EFUSE_SUPPORT

		/*EfuseBufferMode*/
		if (RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, 25, pBuffer, TRUE)) {
			pAd->E2pAccessMode = ((UCHAR) os_str_tol(tmpbuf, 0, 10) == 1) ? 4 : (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EfuseBufferMode=%d\n", pAd->E2pAccessMode));
		}

#endif /* RTMP_EFUSE_SUPPORT */
#endif /* BB_SOC */

		/* E2pAccessMode */
		if (RTMPGetKeyParameter("E2pAccessMode", tmpbuf, 25, pBuffer, TRUE)) {
			pAd->E2pAccessMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("E2pAccessMode=%d\n", pAd->E2pAccessMode));
		}

#ifdef CAL_FREE_IC_SUPPORT

		/* DisableCalFree */
		if (RTMPGetKeyParameter("DisableCalFree", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR DisableCalFree = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

			if (DisableCalFree)
				ops->is_cal_free_ic = NULL;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DisableCalFree=%d\n", DisableCalFree));
		}

#endif /* DisableCalFree */

		/*CountryCode*/
		if (pAd->CommonCfg.bCountryFlag == 0) {
			if (RTMPGetKeyParameter("CountryCode", tmpbuf, 25, pBuffer, TRUE))
				RTMPSetCountryCode(pAd, tmpbuf);
		}

#ifdef EXT_BUILD_CHANNEL_LIST

		/*ChannelGeography*/
		if (RTMPGetKeyParameter("ChannelGeography", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR Geography = (UCHAR) os_str_tol(tmpbuf, 0, 10);

			if (Geography <= BOTH) {
				pAd->CommonCfg.Geography = Geography;
				pAd->CommonCfg.CountryCode[2] =
					(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ChannelGeography=%d\n", pAd->CommonCfg.Geography));
			}
		} else {
			pAd->CommonCfg.Geography = BOTH;
			pAd->CommonCfg.CountryCode[2] = ' ';
		}

#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* SSID*/
			if (TRUE) {
				/* PRINT(DBG_LVL_TRACE, ("pAd->ApCfg.BssidNum=%d\n", pAd->ApCfg.BssidNum)); */
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					snprintf(tok_str, sizeof(tok_str), "SSID%d", i + 1);

					if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[i].Ssid, tmpbuf, strlen(tmpbuf));
						pAd->ApCfg.MBSSID[i].Ssid[strlen(tmpbuf)] = '\0';
						pAd->ApCfg.MBSSID[i].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[i].Ssid);

						if (bSSIDxIsUsed == FALSE)
							bSSIDxIsUsed = TRUE;

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SSID[%d]=%s, EdcaIdx=%d\n", i, pAd->ApCfg.MBSSID[i].Ssid,
								 pAd->ApCfg.MBSSID[i].wdev.EdcaIdx));
					}
				}

				if (bSSIDxIsUsed == FALSE) {
					if (RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE)) {
						BssidCountSupposed = delimitcnt(tmpbuf, ";") + 1;

						if (pAd->ApCfg.BssidNum != BssidCountSupposed)
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Your no. of SSIDs( = %d) does not match your BssidNum( = %d)!\n",
									 BssidCountSupposed, pAd->ApCfg.BssidNum));

						if (pAd->ApCfg.BssidNum > 1) {
							/* Anyway, we still do the legacy dissection of the whole SSID string.*/
							for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
								int apidx = 0;

								if (i < pAd->ApCfg.BssidNum)
									apidx = i;
								else
									break;

								NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].Ssid, macptr, strlen(macptr));
								pAd->ApCfg.MBSSID[apidx].Ssid[strlen(macptr)] = '\0';
								pAd->ApCfg.MBSSID[apidx].SsidLen = strlen((RTMP_STRING *)pAd->ApCfg.MBSSID[apidx].Ssid);
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[apidx].Ssid));
							}
						} else {
							if ((strlen(tmpbuf) > 0) && (strlen(tmpbuf) <= 32)) {
								NdisMoveMemory(pAd->ApCfg.MBSSID[BSS0].Ssid, tmpbuf, strlen(tmpbuf));
								pAd->ApCfg.MBSSID[BSS0].Ssid[strlen(tmpbuf)] = '\0';
								pAd->ApCfg.MBSSID[BSS0].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[BSS0].Ssid);
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SSID=%s\n", pAd->ApCfg.MBSSID[BSS0].Ssid));
							}
						}
					}
				}

				if (RTMPGetKeyParameter("EdcaIdx", tmpbuf, 256, pBuffer, FALSE)) {
					UCHAR edca_idx = 0;

					for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
						if (i < pAd->ApCfg.BssidNum) {
							edca_idx = os_str_tol(macptr, 0, 10);
							pAd->ApCfg.MBSSID[i].wdev.EdcaIdx = edca_idx;
						}
					}
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef DBDC_MODE

		/*Note: must be put before WirelessMode/Channel for check phy mode*/
		if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG dbdc_mode = os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.dbdc_mode = dbdc_mode > 0 ? TRUE : FALSE;
			pAd->CommonCfg.eDBDC_mode = dbdc_mode;

			/*
				TODO
				For DBDC mode, currently cannot use this wf_fwd function!
			*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("DBDC Mode=%d, eDBDC_mode = %d\n", pAd->CommonCfg.dbdc_mode, pAd->CommonCfg.eDBDC_mode));
		}

#endif /* DBDC_MODE */
#ifdef MT_DFS_SUPPORT

		if (RTMPGetKeyParameter("DfsCalibration", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 DisableDfsCal = os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.DfsParameter.DisableDfsCal = DisableDfsCal;
		}

		if (RTMPGetKeyParameter("DfsEnable", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 DfsEnable = os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.DfsParameter.bDfsEnable = DfsEnable;
		}

#ifdef CONFIG_RCSA_SUPPORT
		if (RTMPGetKeyParameter("DfsUseCsaCfg", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_8 UseCsaCfg = os_str_tol(tmpbuf, 0, 10);

			if (UseCsaCfg > 0)
				pAd->CommonCfg.DfsParameter.fUseCsaCfg = TRUE;
			else
				pAd->CommonCfg.DfsParameter.fUseCsaCfg = FALSE;
		}
		if (RTMPGetKeyParameter("DfsRCSAEn", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_8 RCSAEn = os_str_tol(tmpbuf, 0, 10);

			if (RCSAEn > 0)
				pAd->CommonCfg.DfsParameter.bRCSAEn = TRUE;
			else
				pAd->CommonCfg.DfsParameter.bRCSAEn = FALSE;
		}
#endif
#endif

		/*WirelessMode*/
		/*Note: BssidNum must be put before WirelessMode in dat file*/
		if (RTMPGetKeyParameter("WirelessMode", tmpbuf, 100, pBuffer, TRUE))
			RTMPWirelessModeCfg(pAd, tmpbuf);

#ifdef CONFIG_AP_SUPPORT
		/*AutoChannelSelect*/
		if (RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0) { /*Enable*/
				ChannelSel_Alg SelAlg = (ChannelSel_Alg)os_str_tol(tmpbuf, 0, 10);

				if (SelAlg > 3 || SelAlg < 0)
					pAd->ApCfg.bAutoChannelAtBootup = FALSE;
				else { /*Enable*/
					pAd->ApCfg.bAutoChannelAtBootup = TRUE;
					pAd->ApCfg.AutoChannelAlg = SelAlg;
				}
			} else /*Disable*/
				pAd->ApCfg.bAutoChannelAtBootup = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AutoChannelAtBootup=%d\n", pAd->ApCfg.bAutoChannelAtBootup));
		}
#endif/* CONFIG_AP_SUPPORT */

		/* Channel Group */
		if (RTMPGetKeyParameter("ChannelGrp", tmpbuf, 25, pBuffer, TRUE))
			MTSetChGrp(pAd, tmpbuf);

		/*Channel*/
		/*Note: AutoChannelSelect must be put before Channel in dat file*/
		if (RTMPGetKeyParameter("Channel", tmpbuf, 100, pBuffer, TRUE)
#ifdef CONFIG_AP_SUPPORT
			&& !pAd->ApCfg.bAutoChannelAtBootup
#endif
			) {
			RTMPChannelCfg(pAd, tmpbuf);
		}

		/* EtherTrafficBand */
		if (RTMPGetKeyParameter("EtherTrafficBand", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.EtherTrafficBand = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EtherTrafficBand=%d\n", pAd->CommonCfg.EtherTrafficBand));

			if (pAd->CommonCfg.EtherTrafficBand > EtherTrafficBand5G)
				pAd->CommonCfg.EtherTrafficBand = EtherTrafficBand5G;
		}

		/* Wf_fwd_ */
		if (RTMPGetKeyParameter("WfFwdDisabled", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.WfFwdDisabled = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WfFwdDisabled=%d\n", pAd->CommonCfg.WfFwdDisabled));
		}

		/*BasicRate*/
		if (RTMPGetKeyParameter("BasicRate", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.BasicRateBitmap = (ULONG) os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.BasicRateBitmapOld = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BasicRate=%ld\n", pAd->CommonCfg.BasicRateBitmap));
		}

		/*BeaconPeriod*/
		if (RTMPGetKeyParameter("BeaconPeriod", tmpbuf, 10, pBuffer, TRUE)) {
			USHORT bcn_val = (USHORT) os_str_tol(tmpbuf, 0, 10);

			/* The acceptable is 20~1000 ms. Refer to WiFi test plan. */
			if (bcn_val >= 20 && bcn_val <= 1000)
				pAd->CommonCfg.BeaconPeriod = bcn_val;
			else
				pAd->CommonCfg.BeaconPeriod = 100;	/* Default value*/

#ifdef APCLI_CONNECTION_TRIAL
			pAd->CommonCfg.BeaconPeriod = 200;
#endif /* APCLI_CONNECTION_TRIAL */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BeaconPeriod=%d\n", pAd->CommonCfg.BeaconPeriod));
		}

#ifdef RTMP_RBUS_SUPPORT

		/*FreqOffsetDelta*/
		if (pAd->infType == RTMP_DEV_INF_RBUS) {
			if (RTMPGetKeyParameter("FreqDelta", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->RfFreqDelta = (USHORT) os_str_tol(tmpbuf, 0, 10);

				if (pAd->RfFreqDelta > 0x20)
					pAd->RfFreqDelta = 0;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("FreqDelta=%d\n", pAd->RfFreqDelta));
			}
		}

#endif /* RTMP_RBUS_SUPPORT */
#ifdef CONFIG_INIT_RADIO_ONOFF
		if(RTMPGetKeyParameter("RadioOn", tmpbuf, 10, pBuffer, TRUE))
       	{
			if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->ApCfg.bRadioOn = TRUE;
           	else
         		pAd->ApCfg.bRadioOn = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RadioOn=%d\n", pAd->ApCfg.bRadioOn));
       }
#endif
#if defined (DOT11V_WNM_SUPPORT) || defined (CONFIG_DOT11V_WNM)
		WNM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT11V_WNM_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*DtimPeriod*/
#ifdef MBSS_DTIM_SUPPORT
		if (RTMPGetKeyParameter("DtimPeriod", tmpbuf, 128, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				pAd->ApCfg.MBSSID[i].DtimPeriod = (UCHAR) os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MBSS id = %d\tDtimPeriod = %d\n", i, pAd->ApCfg.MBSSID[i].DtimPeriod));
			}
		}
#else
			if (RTMPGetKeyParameter("DtimPeriod", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->ApCfg.DtimPeriod = (UCHAR) os_str_tol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DtimPeriod=%d\n", pAd->ApCfg.DtimPeriod));
			}
#endif

#ifdef BAND_STEERING
			/* Read BandSteering profile parameters */
			BndStrgSetProfileParam(pAd, tmpbuf, pBuffer);
#endif /* BAND_STEERING */
#ifdef MBSS_AS_WDS_AP_SUPPORT
			if(RTMPGetKeyParameter("WDSEnable", tmpbuf, 50, pBuffer, TRUE)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF, ("WDS=%s\n", tmpbuf));
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++) {
					pAd->ApCfg.MBSSID[i].wdev.wds_enable = simple_strtoul(macptr,0,10);
				}
			}

			if(RTMPGetKeyParameter("WdsMac", tmpbuf, 50, pBuffer, TRUE)) {
				/*Mac address acceptable format 01:02:03:04:05:06 length 17 */
				if (strlen(tmpbuf) != 17) {
					for (i=0, value = rstrtok(tmpbuf,":"); value; value = rstrtok(NULL,":")) {
						if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
							return FALSE;  /*Invalid */
						AtoH(value, (UCHAR *)&pAd->ApCfg.wds_mac[i++], 1);
					}
				}
			}
#endif

		}
#endif /* CONFIG_AP_SUPPORT */

		/* TxPower */
		if (RTMPGetKeyParameter("TxPower", tmpbuf, 10, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.ucTxPowerPercentage[BAND1] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TxPower] BAND0: %d, BAND1: %d \n",
						 pAd->CommonCfg.ucTxPowerPercentage[BAND0], pAd->CommonCfg.ucTxPowerPercentage[BAND1]));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TxPower] BAND0: %d \n",
						 pAd->CommonCfg.ucTxPowerPercentage[BAND0]));

#else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TxPower] BAND0: %d \n",
					 pAd->CommonCfg.ucTxPowerPercentage[BAND0]));
#endif /* DBDC_MODE */
#if defined(CONFIG_STA_SUPPORT) || defined(CONFIG_AP_SUPPORT)
			if ((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_AP)) {
				pAd->CommonCfg.ucTxPowerDefault[BAND0] = pAd->CommonCfg.ucTxPowerPercentage[BAND0];
#ifdef DBDC_MODE
				pAd->CommonCfg.ucTxPowerDefault[BAND1] = pAd->CommonCfg.ucTxPowerPercentage[BAND1];
#endif /* DBDC_MODE */
			}
#endif /* CONFIG_STA_SUPPORT */
		}


/* Power Boost Feature */

	/* Power Boost Enable */
	if (RTMPGetKeyParameter("PowerUpenable", tmpbuf, 32, pBuffer, TRUE)) {
		/* parameter parsing */
		for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode) {
			switch (i) {
			case 0:
				pAd->CommonCfg.PowerUpenable[BAND1] = simple_strtol(value, 0, 10);
				break;

			case 1:
				pAd->CommonCfg.PowerUpenable[BAND0] = simple_strtol(value, 0, 10);
				break;

			default:
				break;
			}
		} else {
			switch (i) {
			case 0:
				pAd->CommonCfg.PowerUpenable[BAND0] = simple_strtol(value, 0, 10);
				break;

			default:
				break;
			}
		}
#else
		switch (i) {
		case 0:
				pAd->CommonCfg.PowerUpenable[BAND0] = simple_strtol(value, 0, 10);
				break;

		default:
			break;
		}
#endif /* DBDC_MODE */
		}
#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpenable] BAND0: %d, BAND1: %d\n",
					 pAd->CommonCfg.PowerUpenable[BAND0], pAd->CommonCfg.PowerUpenable[BAND1]));
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpenable] BAND0: %d\n", pAd->CommonCfg.PowerUpenable[BAND0]));

#else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpenable] BAND0: %d\n", pAd->CommonCfg.PowerUpenable[BAND0]));
#endif /* DBDC_MODE */
	}


#ifdef TX_POWER_CONTROL_SUPPORT

		/* Power Boost (CCK, OFDM) */
		if (RTMPGetKeyParameter("PowerUpCckOfdm", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (CCK, OFDM): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value;
						value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_TRACE,
						("[PowerUpCckOfdm] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}

			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
				if (pAd->CommonCfg.PowerUpenable[BAND0]) {
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
						= simple_strtol(value, 0, 10);
				} else
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
			}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpCckOfdm] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][0],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][1],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][2],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][3],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][4],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][5],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][6]));

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpCckOfdm] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][0],
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][1],
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][2],
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][3],
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][4],
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][5],
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][6]));
			}
#endif /* DBDC_MODE */
			printk("[PowerUpCckOfdm] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][0],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][1],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][2],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][3],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][4],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][5],
				pAd->CommonCfg.cPowerUpCckOfdm[BAND1][6]);
		}

		/* Power Boost (HT20) */
		if (RTMPGetKeyParameter("PowerUpHT20", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (HT20): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value;
						value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
							DBG_LVL_TRACE,
							("[PowerUpHT20] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
				(value) &&
				(i < POWER_UP_CATEGORY_RATE_NUM);
				value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpHT20] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				 pAd->CommonCfg.cPowerUpHt20[BAND0][0],
				 pAd->CommonCfg.cPowerUpHt20[BAND0][1],
				 pAd->CommonCfg.cPowerUpHt20[BAND0][2],
				 pAd->CommonCfg.cPowerUpHt20[BAND0][3],
				 pAd->CommonCfg.cPowerUpHt20[BAND0][4],
				 pAd->CommonCfg.cPowerUpHt20[BAND0][5],
				 pAd->CommonCfg.cPowerUpHt20[BAND0][6]));
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpHT20] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					 pAd->CommonCfg.cPowerUpHt20[BAND1][0],
					 pAd->CommonCfg.cPowerUpHt20[BAND1][1],
					 pAd->CommonCfg.cPowerUpHt20[BAND1][2],
					 pAd->CommonCfg.cPowerUpHt20[BAND1][3],
					 pAd->CommonCfg.cPowerUpHt20[BAND1][4],
					 pAd->CommonCfg.cPowerUpHt20[BAND1][5],
					 pAd->CommonCfg.cPowerUpHt20[BAND1][6]));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (HT40) */
		if (RTMPGetKeyParameter("PowerUpHT40", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (HT40): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";");
					value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_TRACE,
						("[PowerUpHT40] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
				(value) && (i < POWER_UP_CATEGORY_RATE_NUM);
				value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpHT40] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				 pAd->CommonCfg.cPowerUpHt40[BAND0][0],
				 pAd->CommonCfg.cPowerUpHt40[BAND0][1],
				 pAd->CommonCfg.cPowerUpHt40[BAND0][2],
				 pAd->CommonCfg.cPowerUpHt40[BAND0][3],
				 pAd->CommonCfg.cPowerUpHt40[BAND0][4],
				 pAd->CommonCfg.cPowerUpHt40[BAND0][5],
				 pAd->CommonCfg.cPowerUpHt40[BAND0][6]));
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpHT40] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					 pAd->CommonCfg.cPowerUpHt40[BAND1][0],
					 pAd->CommonCfg.cPowerUpHt40[BAND1][1],
					 pAd->CommonCfg.cPowerUpHt40[BAND1][2],
					 pAd->CommonCfg.cPowerUpHt40[BAND1][3],
					 pAd->CommonCfg.cPowerUpHt40[BAND1][4],
					 pAd->CommonCfg.cPowerUpHt40[BAND1][5],
					 pAd->CommonCfg.cPowerUpHt40[BAND1][6]));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT20) */
		if (RTMPGetKeyParameter("PowerUpVHT20", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (VHT20): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";");
					value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_TRACE,
						("[PowerUpVHT20] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
				(value) && (i < POWER_UP_CATEGORY_RATE_NUM);
				value = rstrtok(NULL, ":"), i++) {
				if (pAd->CommonCfg.PowerUpenable[BAND0]) {
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
						= simple_strtol(value, 0, 10);
				} else
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
			}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpVHT20] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				 pAd->CommonCfg.cPowerUpVht20[BAND0][0],
				 pAd->CommonCfg.cPowerUpVht20[BAND0][1],
				 pAd->CommonCfg.cPowerUpVht20[BAND0][2],
				 pAd->CommonCfg.cPowerUpVht20[BAND0][3],
				 pAd->CommonCfg.cPowerUpVht20[BAND0][4],
				 pAd->CommonCfg.cPowerUpVht20[BAND0][5],
				 pAd->CommonCfg.cPowerUpVht20[BAND0][6]));
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpVHT20] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					 pAd->CommonCfg.cPowerUpVht20[BAND1][0],
					 pAd->CommonCfg.cPowerUpVht20[BAND1][1],
					 pAd->CommonCfg.cPowerUpVht20[BAND1][2],
					 pAd->CommonCfg.cPowerUpVht20[BAND1][3],
					 pAd->CommonCfg.cPowerUpVht20[BAND1][4],
					 pAd->CommonCfg.cPowerUpVht20[BAND1][5],
					 pAd->CommonCfg.cPowerUpVht20[BAND1][6]));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT40) */
		if (RTMPGetKeyParameter("PowerUpVHT40", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (VHT40): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value;
						value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_TRACE,
						("[PowerUpVHT40] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
				(value) && (i < POWER_UP_CATEGORY_RATE_NUM);
				value = rstrtok(NULL, ":"), i++) {
				if (pAd->CommonCfg.PowerUpenable[BAND0]) {
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
						= simple_strtol(value, 0, 10);
				} else
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
			}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpVHT40] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				 pAd->CommonCfg.cPowerUpVht40[BAND0][0],
				 pAd->CommonCfg.cPowerUpVht40[BAND0][1],
				 pAd->CommonCfg.cPowerUpVht40[BAND0][2],
				 pAd->CommonCfg.cPowerUpVht40[BAND0][3],
				 pAd->CommonCfg.cPowerUpVht40[BAND0][4],
				 pAd->CommonCfg.cPowerUpVht40[BAND0][5],
				 pAd->CommonCfg.cPowerUpVht40[BAND0][6]));
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpVHT40] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					 pAd->CommonCfg.cPowerUpVht40[BAND1][0],
					 pAd->CommonCfg.cPowerUpVht40[BAND1][1],
					 pAd->CommonCfg.cPowerUpVht40[BAND1][2],
					 pAd->CommonCfg.cPowerUpVht40[BAND1][3],
					 pAd->CommonCfg.cPowerUpVht40[BAND1][4],
					 pAd->CommonCfg.cPowerUpVht40[BAND1][5],
					 pAd->CommonCfg.cPowerUpVht40[BAND1][6]));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT80) */
		if (RTMPGetKeyParameter("PowerUpVHT80", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (VHT80): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value;
						value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_TRACE,
						("[PowerUpVHT80] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
				(value) && (i < POWER_UP_CATEGORY_RATE_NUM);
				value = rstrtok(NULL, ":"), i++) {
				if (pAd->CommonCfg.PowerUpenable[BAND0]) {
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
						= simple_strtol(value, 0, 10);
				} else
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
			}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpVHT80] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				 pAd->CommonCfg.cPowerUpVht80[BAND0][0],
				 pAd->CommonCfg.cPowerUpVht80[BAND0][1],
				 pAd->CommonCfg.cPowerUpVht80[BAND0][2],
				 pAd->CommonCfg.cPowerUpVht80[BAND0][3],
				 pAd->CommonCfg.cPowerUpVht80[BAND0][4],
				 pAd->CommonCfg.cPowerUpVht80[BAND0][5],
				 pAd->CommonCfg.cPowerUpVht80[BAND0][6]));

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpVHT80] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					 pAd->CommonCfg.cPowerUpVht80[BAND1][0],
					 pAd->CommonCfg.cPowerUpVht80[BAND1][1],
					 pAd->CommonCfg.cPowerUpVht80[BAND1][2],
					 pAd->CommonCfg.cPowerUpVht80[BAND1][3],
					 pAd->CommonCfg.cPowerUpVht80[BAND1][4],
					 pAd->CommonCfg.cPowerUpVht80[BAND1][5],
					 pAd->CommonCfg.cPowerUpVht80[BAND1][6]));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT160) */
		if (RTMPGetKeyParameter("PowerUpVHT160", tmpbuf, 32,
					pBuffer, TRUE)) {
			printk("Power Boost (VHT160): %s", __func__);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";");
					value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_TRACE,
						("[PowerUpVHT160] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND1]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = 0;
				}

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, ":");
					(value) &&
					(i < POWER_UP_CATEGORY_RATE_NUM);
					value = rstrtok(NULL, ":"), i++) {
					if (pAd->CommonCfg.PowerUpenable[BAND0]) {
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
							= simple_strtol(value, 0, 10);
					} else
						pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, ":");
				(value) && (i < POWER_UP_CATEGORY_RATE_NUM);
				value = rstrtok(NULL, ":"), i++) {
				if (pAd->CommonCfg.PowerUpenable[BAND0]) {
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i]
						= simple_strtol(value, 0, 10);
				} else
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = 0;
			}
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[PowerUpVHT160] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
				 pAd->CommonCfg.cPowerUpVht160[BAND0][0],
				 pAd->CommonCfg.cPowerUpVht160[BAND0][1],
				 pAd->CommonCfg.cPowerUpVht160[BAND0][2],
				 pAd->CommonCfg.cPowerUpVht160[BAND0][3],
				 pAd->CommonCfg.cPowerUpVht160[BAND0][4],
				 pAd->CommonCfg.cPowerUpVht160[BAND0][5],
				 pAd->CommonCfg.cPowerUpVht160[BAND0][6]));
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_TRACE,
					("[PowerUpVHT160] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
					 pAd->CommonCfg.cPowerUpVht160[BAND1][0],
					 pAd->CommonCfg.cPowerUpVht160[BAND1][1],
					 pAd->CommonCfg.cPowerUpVht160[BAND1][2],
					 pAd->CommonCfg.cPowerUpVht160[BAND1][3],
					 pAd->CommonCfg.cPowerUpVht160[BAND1][4],
					 pAd->CommonCfg.cPowerUpVht160[BAND1][5],
					 pAd->CommonCfg.cPowerUpVht160[BAND1][6]));
			}
#endif /* DBDC_MODE */
		}
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef SINGLE_SKU_V2

		/* TxPower SKU */
		if (RTMPGetKeyParameter("SKUenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.SKUenable[BAND1] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[SKUenable] BAND0: %d, BAND1: %d \n",
						 pAd->CommonCfg.SKUenable[BAND0], pAd->CommonCfg.SKUenable[BAND1]));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[SKUenable] BAND0: %d \n", pAd->CommonCfg.SKUenable[BAND0]));

#else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[SKUenable] BAND0: %d \n", pAd->CommonCfg.SKUenable[BAND0]));
#endif /* DBDC_MODE */
		}

#endif /*SINGLE_SKU_V2 */

		/* CCKTxStream */
		if (RTMPGetKeyParameter("CCKTxStream", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {

#ifdef DBDC_MODE
				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.CCKTxStream[BAND1] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.CCKTxStream[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.CCKTxStream[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}
#else

				switch (i) {
				case 0:
					pAd->CommonCfg.CCKTxStream[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}
#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[CCKTxStream] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.CCKTxStream[BAND0], pAd->CommonCfg.CCKTxStream[BAND1]));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[CCKTxStream] BAND0: %d\n", pAd->CommonCfg.CCKTxStream[BAND0]));

#else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[CCKTxStream] BAND0: %d\n", pAd->CommonCfg.CCKTxStream[BAND0]));
#endif /* DBDC_MODE */
		}

		/* TxPower Percentage */
		if (RTMPGetKeyParameter("PERCENTAGEenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.PERCENTAGEenable[BAND1] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PERCENTAGEenable] BAND0: %d, BAND1: %d \n",
						 pAd->CommonCfg.PERCENTAGEenable[BAND0], pAd->CommonCfg.PERCENTAGEenable[BAND1]));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PERCENTAGEenable] BAND0: %d \n",
						 pAd->CommonCfg.PERCENTAGEenable[BAND0]));

#else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PERCENTAGEenable] BAND0: %d \n",
					 pAd->CommonCfg.PERCENTAGEenable[BAND0]));
#endif /* DBDC_MODE */
		}

		/* TxPower BF Backoff */
		if (RTMPGetKeyParameter("BFBACKOFFenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.BFBACKOFFenable[BAND1] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[BFBACKOFFenable] BAND0: %d, BAND1: %d \n",
						 pAd->CommonCfg.BFBACKOFFenable[BAND0], pAd->CommonCfg.BFBACKOFFenable[BAND1]));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[BFBACKOFFenable] BAND0: %d \n",
						 pAd->CommonCfg.BFBACKOFFenable[BAND0]));

#else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[BFBACKOFFenable] BAND0: %d \n",
					 pAd->CommonCfg.BFBACKOFFenable[BAND0]));
#endif /* DBDC_MODE */
		}

#ifdef RLM_CAL_CACHE_SUPPORT

		/* Calibration Cache Support */
		if (RTMPGetKeyParameter("CalCacheApply", tmpbuf, 32, pBuffer, TRUE)) {
			pAd->CommonCfg.CalCacheApply = (ULONG) simple_strtol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CalCacheApply = %d \n", pAd->CommonCfg.CalCacheApply));
		}

#endif /* RLM_CAL_CACHE_SUPPORT */

		/*BGProtection*/
		if (RTMPGetKeyParameter("BGProtection", tmpbuf, 10, pBuffer, TRUE)) {
			/*#if 0	#ifndef WIFI_TEST*/
			/*		pAd->CommonCfg.UseBGProtection = 2; disable b/g protection for throughput test*/
			/*#else*/
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /*Always On*/
				pAd->CommonCfg.UseBGProtection = 1;
				break;

			case 2: /*Always OFF*/
				pAd->CommonCfg.UseBGProtection = 2;
				break;

			case 0: /*AUTO*/
			default:
				pAd->CommonCfg.UseBGProtection = 0;
				break;
			}

			/*#endif*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BGProtection=%ld\n", pAd->CommonCfg.UseBGProtection));
		}

#ifdef CONFIG_AP_SUPPORT

		/*OLBCDetection*/
		if (RTMPGetKeyParameter("DisableOLBC", tmpbuf, 10, pBuffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /*disable OLBC Detection*/
				pAd->CommonCfg.DisableOLBCDetect = 1;
				break;

			case 0: /*enable OLBC Detection*/
				pAd->CommonCfg.DisableOLBCDetect = 0;
				break;

			default:
				pAd->CommonCfg.DisableOLBCDetect = 0;
				break;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OLBCDetection=%ld\n", pAd->CommonCfg.DisableOLBCDetect));
		}

#endif /* CONFIG_AP_SUPPORT */

		/*TxPreamble*/
		if (RTMPGetKeyParameter("TxPreamble", tmpbuf, 10, pBuffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case Rt802_11PreambleShort:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleShort;
				break;

			case Rt802_11PreambleAuto:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto;
				break;

			case Rt802_11PreambleLong:
			default:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
				break;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxPreamble=%ld\n", pAd->CommonCfg.TxPreamble));
		}

		/*RTSPktThreshold*/
		read_rts_pkt_thld_from_file(pAd, tmpbuf, pBuffer);
		/*RTSThreshold*/
		read_rts_len_thld_from_file(pAd, tmpbuf, pBuffer);
		/*FragThreshold*/
		read_frag_thld_from_file(pAd, tmpbuf, pBuffer);
		/*Udmaenable & UdmaPortNum*/
#ifdef RTMP_UDMA_SUPPORT
		if (RTMPGetKeyParameter("UdmaEnable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0) {
				pAd->CommonCfg.bUdmaFlag = TRUE;/*Enable*/
			} else
				pAd->CommonCfg.bUdmaFlag = FALSE;
		}

		if (RTMPGetKeyParameter("UdmaPortNum", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
				pAd->CommonCfg.UdmaPortNum = UDMA_PORT1;
			else
				pAd->CommonCfg.UdmaPortNum = UDMA_PORT0;
		}
#endif/*RTMP_UDMA_SUPPORT*/
#ifdef VENDOR_FEATURE7_SUPPORT
		if (RTMPGetKeyParameter("VLANID", tmpbuf, 128, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && i < pAd->ApCfg.BssidNum);
				macptr = rstrtok(NULL, ";"), i++) {
				pAd->ApCfg.MBSSID[i].wdev.VLAN_VID = os_str_tol(macptr, 0, 10);
			}
		}
#endif
#ifdef VLAN_SUPPORT
		/*Vlan Tag */
#ifdef CONFIG_AP_SUPPORT
		if (RTMPGetKeyParameter("VLANTag", tmpbuf, 32, pBuffer, TRUE)) {
			UCHAR		*macptr;
			BOOLEAN		bVlan_tag = FALSE;
			struct		wifi_dev *wdev;

			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				bVlan_tag = simple_strtol(macptr, 0, 10);
				if (i >= pAd->ApCfg.BssidNum)
					break;
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				wdev->bVLAN_Tag = bVlan_tag;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("BSS%d VlanTag=%d\n", i, wdev->bVLAN_Tag));
			}
		}
#ifdef APCLI_SUPPORT
		if (RTMPGetKeyParameter("STAVLANTag", tmpbuf, 10, pBuffer, TRUE)) {
			BOOLEAN		bVlan_tag = FALSE;
			struct		wifi_dev *wdev;

			if (simple_strtol(tmpbuf, 0, 10) != 0)
				bVlan_tag = TRUE;

			for (i = 0; i < MAX_APCLI_NUM; i++) {
				wdev = &pAd->ApCfg.ApCliTab[i].wdev;
				wdev->bVLAN_Tag = bVlan_tag;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("APCLI%d VlanTag=%d\n", i, wdev->bVLAN_Tag));
			}
		}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* VLAN_SUPPORT */

		/*TxBurst*/
		if (RTMPGetKeyParameter("TxBurst", tmpbuf, 10, pBuffer, TRUE)) {
			/*#ifdef WIFI_TEST*/
			/*						pAd->CommonCfg.bEnableTxBurst = FALSE;*/
			/*#else*/
			if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
				pAd->CommonCfg.bEnableTxBurst = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bEnableTxBurst = FALSE;

			/*#endif*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxBurst=%d\n", pAd->CommonCfg.bEnableTxBurst));
		}

#ifdef AGGREGATION_SUPPORT

		/*PktAggregate*/
		if (RTMPGetKeyParameter("PktAggregate", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
				pAd->CommonCfg.bAggregationCapable = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bAggregationCapable = FALSE;

#ifdef PIGGYBACK_SUPPORT
			pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
#endif /* PIGGYBACK_SUPPORT */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PktAggregate=%d\n", pAd->CommonCfg.bAggregationCapable));
		}

#else
		pAd->CommonCfg.bAggregationCapable = FALSE;
		pAd->CommonCfg.bPiggyBackCapable = FALSE;
#endif /* AGGREGATION_SUPPORT */
		/* WmmCapable*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		rtmp_read_ap_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef DOT11U_INTERWORKING_IE_SUPPORT
		rtmp_read_ap_InterWorkingIE_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11U_INTERWORKING_IE_SUPPORT */
			/* MaxStaNum*/
			if (RTMPGetKeyParameter("MbssMaxStaNum", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					ApCfg_Set_PerMbssMaxStaNum_Proc(pAd, i, macptr);
				}
			}

			/* IdleTimeout*/
			if (RTMPGetKeyParameter("IdleTimeout", tmpbuf, 10, pBuffer, TRUE))
				ApCfg_Set_IdleTimeout_Proc(pAd, tmpbuf);

			/*NoForwarding*/
			if (RTMPGetKeyParameter("NoForwarding", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
						pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic = TRUE;
					else /*Disable*/
						pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic = FALSE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) NoForwarding=%ld\n", i,
							 pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic));
				}
			}

			/*NoForwardingBTNBSSID*/
			if (RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
				else /*Disable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NoForwardingBTNBSSID=%ld\n",
						 pAd->ApCfg.IsolateInterStaTrafficBTNBSSID));
			}

		/*NoForwardingMBCast*/
            if (RTMPGetKeyParameter("NoForwardingMBCast", tmpbuf, 32, pBuffer, TRUE)) {
                for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
                    if (i >= pAd->ApCfg.BssidNum)
                        break;

				    if (simple_strtol(macptr, 0, 10) != 0)	//Enable
						pAd->ApCfg.MBSSID[i].IsolateInterStaMBCast = TRUE;
					else //Disable
						pAd->ApCfg.MBSSID[i].IsolateInterStaMBCast = FALSE;

                    MTWF_LOG(DBG_CAT_CFG,DBG_CAT_AP, DBG_LVL_OFF, ("I/F(ra%d) NoForwardingMBCast=%d\n", i, pAd->ApCfg.MBSSID[i].IsolateInterStaMBCast));
				}
			}
#ifdef DSCP_QOS_MAP_SUPPORT
			/*DscpQosMapEnable*/
			if (RTMPGetKeyParameter("DscpQosMapEnable", tmpbuf, 10, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= 2)
						break;

					if (simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
						pAd->ApCfg.DscpQosMapSupport[i] = 1;
					else /*Disable*/
						pAd->ApCfg.DscpQosMapSupport[i] = 0;
				}

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					if (pAd->ApCfg.DscpQosMapSupport[0]) {
						pAd->ApCfg.MBSSID[i].DscpQosMapEnable = TRUE;
						pAd->ApCfg.MBSSID[i].DscpQosPoolId = 0;
					} else
						pAd->ApCfg.MBSSID[i].DscpQosMapEnable = FALSE;
				}
#ifdef MULTI_PROFILE
				{
					UINT8 bssNum = multi_profile_get_bss_num(pAd, 1);

					MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF,
						("QosMapping: BssNumber for profile 1 is %d \n", bssNum));
					if (bssNum != 0) {
						for (i = bssNum; i < pAd->ApCfg.BssidNum; i++) {
							if (pAd->ApCfg.DscpQosMapSupport[1]) {
								pAd->ApCfg.MBSSID[i].DscpQosMapEnable = TRUE;
								pAd->ApCfg.MBSSID[i].DscpQosPoolId = 1;
							} else
								pAd->ApCfg.MBSSID[i].DscpQosMapEnable = FALSE;
						}
					}
				}
#endif
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					if (pAd->ApCfg.MBSSID[i].DscpQosMapEnable)
						MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF,
					("Pool id for bss %d is %d \n", i, pAd->ApCfg.MBSSID[i].DscpQosPoolId));
					else
						MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF,
							("DscpQosMapping Not enabled for Bss %d \n", i));
				}
			}
			/*DscpQosMap*/
			if (RTMPGetKeyParameter("DscpQosMap", tmpbuf, 80, pBuffer, TRUE)) {
				int j;
				P_DSCP_QOS_MAP_TABLE_T pQosMapPool;
				UCHAR dscp_buf[16] = {0, 7, 8, 15, 16, 23, 24, 31, 32, 39, 40, 47, 48, 55, 56, 63};

				RTMP_STRING	* macptr2[2];

				macptr2[0] = rstrtok(tmpbuf, ";");
				macptr2[1] = rstrtok(NULL, ";");

				for (j = 0; j < 2; j++) {
					UINT32 ac_map = 0;
					pQosMapPool = &pAd->ApCfg.DscpQosMapTable[j];

					for (i = 0, macptr = rstrtok(macptr2[j], ":"); macptr; macptr = rstrtok(NULL, ":"), i++) {
						UCHAR ac_category;

						if (i > 7)
							break;

						ac_category = simple_strtol(macptr, 0, 10);
						if (ac_category > 7)
							ac_category = 0;
						ac_map = (ac_map | (ac_category << (i*4)));
					}
					if (ac_map == 0)
						ac_map = 3;
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("DSCPQosMAP Setting for Profile %d ==>AC MAP =%08x \n", j, ac_map));
					pQosMapPool->ucPoolValid = 1;
					pQosMapPool->ucDscpExceptionCount = 0;
					pQosMapPool->u4Ac = ac_map;
					memset(pQosMapPool->au2DscpException, 0xff, 42);
					memcpy((UCHAR *)pQosMapPool->au2DscpRange, dscp_buf, 16);
				}
			}
#endif	/*DSCP_QOS_MAP_SUPPORT*/

#ifdef DSCP_PRI_SUPPORT
			read_dscp_pri_param(pAd, tmpbuf, pBuffer);
#endif

			/*HideSSID*/
			if (RTMPGetKeyParameter("HideSSID", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) { /*Enable*/
						pAd->ApCfg.MBSSID[apidx].bHideSsid = TRUE;
#ifdef WSC_V2_SUPPORT
						pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bWpsEnable = FALSE;
#endif /* WSC_V2_SUPPORT */
					} else /*Disable*/
						pAd->ApCfg.MBSSID[apidx].bHideSsid = FALSE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) HideSSID=%d\n", i,
							 pAd->ApCfg.MBSSID[apidx].bHideSsid));
				}
			}

			/*StationKeepAlive*/
			if (RTMPGetKeyParameter("StationKeepAlive", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[apidx].StationKeepAliveTime = os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) StationKeepAliveTime=%d\n", i,
							 pAd->ApCfg.MBSSID[apidx].StationKeepAliveTime));
				}
			}

			/*AutoChannelSkipList*/
			if (RTMPGetKeyParameter("AutoChannelSkipList", tmpbuf, 128, pBuffer, FALSE)) {
				pAd->ApCfg.AutoChannelSkipListNum = delimitcnt(tmpbuf, ";") + 1;


				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->ApCfg.AutoChannelSkipListNum) {
						pAd->ApCfg.AutoChannelSkipList[i] = os_str_tol(macptr, 0, 10);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" AutoChannelSkipList[%d]= %d\n", i,
								 pAd->ApCfg.AutoChannelSkipList[i]));
					} else
						break;
				}
			}

#ifdef BACKGROUND_SCAN_SUPPORT

			if (RTMPGetKeyParameter("DfsZeroWait", tmpbuf, 50, pBuffer, FALSE)) {
				UINT8 DfsZeroWait = os_str_tol(tmpbuf, 0, 10);

				if ((DfsZeroWait == 1)
#ifdef MT_DFS_SUPPORT
					&& IS_SUPPORT_MT_DFS(pAd)
#endif
				   ) {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = TRUE;/*Enable*/
#ifdef MT_DFS_SUPPORT
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, TRUE);
#endif
				} else {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#ifdef MT_DFS_SUPPORT
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
#endif
				}

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DfsZeroWait Support=%d/%d\n", DfsZeroWait,
						 pAd->BgndScanCtrl.DfsZeroWaitSupport));
			}

#ifdef MT_DFS_SUPPORT
			if (RTMPGetKeyParameter("DfsDedicatedZeroWait", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR DfsDedicatedZeroWait = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				if (!pAd->CommonCfg.dbdc_mode) {
					if (DfsDedicatedZeroWait == 0) {
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
#ifdef ONDEMAND_DFS
						pAd->CommonCfg.DfsParameter.bOnDemandZeroWaitSupport = FALSE;
#endif
					} else if (DfsDedicatedZeroWait == 1) {
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
#ifdef ONDEMAND_DFS
						pAd->CommonCfg.DfsParameter.bOnDemandZeroWaitSupport = FALSE;
#endif
					} else if (DfsDedicatedZeroWait == 2) {
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
#ifdef ONDEMAND_DFS
						pAd->CommonCfg.DfsParameter.bOnDemandZeroWaitSupport = FALSE;
					} else if (DfsDedicatedZeroWait == 3) {
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
						pAd->CommonCfg.DfsParameter.bOnDemandZeroWaitSupport = TRUE;
#endif
					} else {
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
#ifdef ONDEMAND_DFS
						pAd->CommonCfg.DfsParameter.bOnDemandZeroWaitSupport = FALSE;
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
					if (DfsDedicatedZeroWait == 4)
						pAd->CommonCfg.DfsParameter.bDFSV10Support = TRUE;
					else
						pAd->CommonCfg.DfsParameter.bDFSV10Support = FALSE;
#endif
					}
				} else {
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				}
			}
			if (RTMPGetKeyParameter("DfsZeroWaitDefault", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR DfsZeroWaitDefault = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = DfsZeroWaitDefault;
			}
			if (RTMPGetKeyParameter("VHT_BW", tmpbuf, 25, pBuffer, TRUE)) {
				for (macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";")) {
					long vhtBw = os_str_tol(macptr, 0, 10);

					if (vhtBw == VHT_BW_8080) {
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;
					}
				}
			}
#endif
			if (RTMPGetKeyParameter("BgndScanSkipCh", tmpbuf, 50, pBuffer, FALSE)) {
				pAd->BgndScanCtrl.SkipChannelNum = delimitcnt(tmpbuf, ";") + 1;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->BgndScanCtrl.SkipChannelNum) {
						pAd->BgndScanCtrl.SkipChannelList[i] = os_str_tol(macptr, 0, 10);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Background Skip Channel list[%d]= %d\n", i,
								 pAd->BgndScanCtrl.SkipChannelList[i]));
					} else
						break;
				}
			}

#endif /* BACKGROUND_SCAN_SUPPORT */

			if (RTMPGetKeyParameter("EDCCAEnable", tmpbuf, 10, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < DBDC_BAND_NUM) {
#ifdef DEFAULT_5G_PROFILE

						if (i == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
							pAd->CommonCfg.ucEDCCACtrl[BAND1] = simple_strtol(macptr, 0, 10);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" EDCCA band[1]= %d \n", pAd->CommonCfg.ucEDCCACtrl[BAND1]));
						} else {
							pAd->CommonCfg.ucEDCCACtrl[BAND0] = simple_strtol(macptr, 0, 10);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" EDCCA band[0]= %d \n", pAd->CommonCfg.ucEDCCACtrl[BAND0]));
						}

#else
						pAd->CommonCfg.ucEDCCACtrl[i] = simple_strtol(macptr, 0, 10);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" EDCCA band[%d]= %d\n", i, pAd->CommonCfg.ucEDCCACtrl[i]));
#endif /* DEFAULT_5G_PROFILE */
					} else
						break;
				}
			}
#ifdef MT_DFS_SUPPORT

			if (RTMPGetKeyParameter("DfsZeroWaitCacTime", tmpbuf, 50, pBuffer, FALSE)) {
				UINT8 OffChnlCacTime = os_str_tol(tmpbuf, 0, 10);
				pAd->CommonCfg.DfsParameter.DfsZeroWaitCacTime = OffChnlCacTime; /* Unit is minute */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DfsZeroWaitCacTime=%d/%d\n",
						 OffChnlCacTime,
						 pAd->CommonCfg.DfsParameter.DfsZeroWaitCacTime));
			}

#endif /* MT_DFS_SUPPORT  */
#ifdef AP_SCAN_SUPPORT

			/*ACSCheckTime*/
			if (RTMPGetKeyParameter("ACSCheckTime", tmpbuf, 32, pBuffer, TRUE)) {
				UINT8 i = 0;
				UINT8 time = 0;
				RTMP_STRING *ptr;
				struct wifi_dev *pwdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

				for (i = 0, ptr = rstrtok(tmpbuf, ";"); ptr; ptr = rstrtok(NULL, ";"), i++) {
					if (i >= DBDC_BAND_NUM)
						break;
					time = os_str_tol(ptr, 0, 10);
#ifndef ACS_CTCC_SUPPORT
					time = time * 3600;/* Hour to second */
#endif
					if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
						/* 5G + 5G */
						pAd->ApCfg.ACSCheckTime[i] = time;
					} else {
						if (WMODE_CAP_5G(pwdev->PhyMode)) {
							/* 5G + 2G */
							if (i == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
#ifdef DBDC_MODE
								/* [5G] + 2G */
								pAd->ApCfg.ACSCheckTime[BAND1] = time;
#endif
							} else {
								/* 5G + [2G] */
								pAd->ApCfg.ACSCheckTime[BAND0] = time;
							}
						} else {
							/* 2G + 5G or 2G only */
							pAd->ApCfg.ACSCheckTime[i] = time;
						}
					}
				}
				for (i = 0; i < DBDC_BAND_NUM; i++) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): ACSCheckTime[%d]=%u seconds\n",
					__func__, i, pAd->ApCfg.ACSCheckTime[i]));
				}
			}
				/* PeriodicPartialScan */
	if (pAd->CommonCfg.dbdc_mode != TRUE) {

			if (RTMPGetKeyParameter("PeriodicPartialScan", tmpbuf, 10, pBuffer, TRUE)) {
				if (simple_strtol(tmpbuf, 0, 10) > 0)  /*Enable*/
					pAd->ScanCtrl.PartialScan.bPeriodicPartialScan = TRUE;
				else
					pAd->ScanCtrl.PartialScan.bPeriodicPartialScan = FALSE;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeriodicPartialScan = %u \n", pAd->ScanCtrl.PartialScan.bPeriodicPartialScan));
			}
			/* PartialScanTriggerPeriod */
			if (RTMPGetKeyParameter("PartialScanTriggerPeriod", tmpbuf, 10, pBuffer, TRUE)) {
				UINT8 TriggerPeriod = 0;
				TriggerPeriod = simple_strtol(tmpbuf, 0, 10);
				if (TriggerPeriod > 0)  /* Enable */
					pAd->ScanCtrl.PartialScan.TriggerPeriod = TriggerPeriod*60*10; /* unit: 100ms */
				else
					pAd->ScanCtrl.PartialScan.TriggerPeriod = DEFAULT_PARTIAL_SCAN_TRIGGER_PERIOD;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeriodicPartialScan = %u\n", pAd->ScanCtrl.PartialScan.bPeriodicPartialScan));
			}

	}

#endif /* AP_SCAN_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		/*ShortSlot*/
		if (RTMPGetKeyParameter("ShortSlot", tmpbuf, 10, pBuffer, TRUE)) {
			RT_CfgSetShortSlot(pAd, tmpbuf);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ShortSlot=%d\n", pAd->CommonCfg.bUseShortSlotTime));
		}

#ifdef TXBF_SUPPORT

		if (cap->FlgHwTxBfCap) {
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)

			/*ITxBfEn*/
			if (RTMPGetKeyParameter("ITxBfEn", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn = (os_str_tol(tmpbuf, 0, 10) != 0);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ITxBfEn = %d\n",
						 pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn));
			}

			/* ITxBfTimeout */
			if (RTMPGetKeyParameter("ITxBfTimeout", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ITxBfTimeout = os_str_tol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ITxBfTimeout = %ld\n", pAd->CommonCfg.ITxBfTimeout));
			}

#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

			/* Set ETxBfEnCond to wdev->wpf_cfg->phy_conf.ETxBfEnCond and pAd->CommonCfg.ETxBfEnCond */
			read_txbf_param_from_file(pAd, tmpbuf, pBuffer);

			/* ETxBfEnCond */
			if (RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, 32, pBuffer, TRUE)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CommnCfg.ETxBfEnCond = %ld\n", pAd->CommonCfg.ETxBfEnCond));

				if (pAd->CommonCfg.ETxBfEnCond)
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = TRUE;
				else
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = FALSE;

				/* MUTxRxEnable*/
				if (RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, 32, pBuffer, TRUE)) {
					pAd->CommonCfg.MUTxRxEnable = os_str_tol(tmpbuf, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MUTxRxEnable = %ld\n", pAd->CommonCfg.MUTxRxEnable));
				}

#ifdef CFG_SUPPORT_MU_MIMO_RA
				if (RTMPGetKeyParameter("MuEnable7615HwPatch", tmpbuf, 10, pBuffer, TRUE)) {
					UINT8 IsEnable = FALSE;

					IsEnable = simple_strtol(tmpbuf, 0, 10);
					pAd->MuHwSwPatch = IsEnable;
				}
#endif

			}

			/* ETxBfTimeout*/
			if (RTMPGetKeyParameter("ETxBfTimeout", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfTimeout = os_str_tol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfTimeout = %ld\n", pAd->CommonCfg.ETxBfTimeout));
			}

			/* ETxBfNoncompress*/
			if (RTMPGetKeyParameter("ETxBfNoncompress", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfNoncompress = os_str_tol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfNoncompress = %d\n", pAd->CommonCfg.ETxBfNoncompress));
			}

			/* ETxBfIncapable */
			if (RTMPGetKeyParameter("ETxBfIncapable", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfIncapable = os_str_tol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfIncapable = %d\n", pAd->CommonCfg.ETxBfIncapable));
			}
		}

#endif /* TXBF_SUPPORT */
#ifdef STREAM_MODE_SUPPORT

		/* StreamMode*/
		if (cap->FlgHwStreamMode) {
			if (RTMPGetKeyParameter("StreamMode", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.StreamMode = (os_str_tol(tmpbuf, 0, 10) & 0x03);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("StreamMode= %d\n", pAd->CommonCfg.StreamMode));
			}

			/* StreamModeMac*/
			for (i = 0; i < STREAM_MODE_STA_NUM; i++) {
				RTMP_STRING tok_str[32];
				sprintf(tok_str, "StreamModeMac%d", i);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE)) {
					int j;

					if (strlen(tmpbuf) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
						continue;

					for (j = 0; j < MAC_ADDR_LEN; j++) {
						AtoH(tmpbuf, &pAd->CommonCfg.StreamModeMac[i][j], 1);
						tmpbuf = tmpbuf + 3;
					}
				}
			}

			if (NdisEqualMemory(ZERO_MAC_ADDR, &pAd->CommonCfg.StreamModeMac[0][0], MAC_ADDR_LEN)) {
				/* set default broadcast mac to entry 0 if user not set it */
				NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0], BROADCAST_ADDR, MAC_ADDR_LEN);
			}
		}

#endif /* STREAM_MODE_SUPPORT */

		/*IEEE80211H*/
		if (RTMPGetKeyParameter("IEEE80211H", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
					pAd->CommonCfg.bIEEE80211H = TRUE;
				else { /*Disable*/
					pAd->CommonCfg.bIEEE80211H = FALSE;
#ifdef BACKGROUND_SCAN_SUPPORT
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#endif
#ifdef MT_DFS_SUPPORT
					pAd->CommonCfg.DfsParameter.bDfsEnable = FALSE;
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Disable DFS/Zero wait=%d/%d\n",
							 __func__,
							 IS_SUPPORT_MT_DFS(pAd),
							 IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)));
#endif
				}

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IEEE80211H=%d\n", pAd->CommonCfg.bIEEE80211H));
			}
		}

		/*RDRegion*/
		if (RTMPGetKeyParameter("RDRegion", tmpbuf, 128, pBuffer, TRUE)) {
#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
			pAd->CommonCfg.DfsParameter.fgRDRegionConfigured = TRUE;
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */
			if ((strncmp(tmpbuf, "JAP_W53", 7) == 0) || (strncmp(tmpbuf, "jap_w53", 7) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP_W53;
				/*pRadarDetect->DfsSessionTime = 15;*/
			} else if ((strncmp(tmpbuf, "JAP_W56", 7) == 0) || (strncmp(tmpbuf, "jap_w56", 7) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP_W56;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else if ((strncmp(tmpbuf, "JAP", 3) == 0) || (strncmp(tmpbuf, "jap", 3) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP;
				/*pRadarDetect->DfsSessionTime = 5;*/
			} else  if ((strncmp(tmpbuf, "FCC", 3) == 0) || (strncmp(tmpbuf, "fcc", 3) == 0)) {
				pAd->CommonCfg.RDDurRegion = FCC;
				/*pRadarDetect->DfsSessionTime = 5;*/
			} else if ((strncmp(tmpbuf, "CE", 2) == 0) || (strncmp(tmpbuf, "ce", 2) == 0)) {
				pAd->CommonCfg.RDDurRegion = CE;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else {
				pAd->CommonCfg.RDDurRegion = CE;
#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
				pAd->CommonCfg.DfsParameter.fgRDRegionConfigured = FALSE;
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */
				/*pRadarDetect->DfsSessionTime = 13;*/
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RDRegion=%d\n", pAd->CommonCfg.RDDurRegion));
		} else {
			pAd->CommonCfg.RDDurRegion = CE;
#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
			pAd->CommonCfg.DfsParameter.fgRDRegionConfigured = FALSE;
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */
			/*pRadarDetect->DfsSessionTime = 13;*/
		}

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
		rtmp_read_rdd_threshold_parms_from_file(pAd, tmpbuf, pBuffer);

		/* RTDetect */
		if (RTMPGetKeyParameter("RTDetect", tmpbuf, 128, pBuffer, TRUE)) {
			ucRDDurRegion = pAd->CommonCfg.RDDurRegion;
			if (pAd->CommonCfg.DfsParameter.fgRDRegionConfigured == TRUE) {
				if ((ucRDDurRegion == JAP_W53) || (ucRDDurRegion == JAP_W56))
					ucRDDurRegion = JAP;
				prRadarThresholdParam = &g_arRadarThresholdParam[ucRDDurRegion];
			} else {
				prRadarThresholdParam = &g_arRadarThresholdParam[3];/* All Area */
			}
			for (i = 0, macptr = rstrtok(tmpbuf, "-"); macptr && (i < RT_NUM); macptr = rstrtok(NULL, "-"), i++) {
				if (prRadarThresholdParam->afgSupportedRT[i] == TRUE)
					prRadarThresholdParam->arRadarType[i].ucRT_ENB = simple_strtol(macptr, 0, 10);
				else
					MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("%s: Unsupported RT-%d\n", __func__, i));
				MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE,
					("%s: RT-%d: RT_ENB = %d\n", __func__, i, prRadarThresholdParam->arRadarType[i].ucRT_ENB));
			}
		}
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

#ifdef SYSTEM_LOG_SUPPORT

		/*WirelessEvent*/
		if (RTMPGetKeyParameter("WirelessEvent", tmpbuf, 10, pBuffer, TRUE)) {
			BOOLEAN FlgIsWEntSup = FALSE;

			if (os_str_tol(tmpbuf, 0, 10) != 0)
				FlgIsWEntSup = TRUE;

			RtmpOsWlanEventSet(pAd, &pAd->CommonCfg.bWirelessEvent, FlgIsWEntSup);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WirelessEvent=%d\n", pAd->CommonCfg.bWirelessEvent));
		}

#endif /* SYSTEM_LOG_SUPPORT */
		/*Security Parameters */
		ReadSecurityParameterFromFile(pAd, tmpbuf, pBuffer);
#ifdef MBO_SUPPORT
		ReadMboParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* MBO_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
		ReadMapParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* MAP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*Access Control List*/
			rtmp_read_acl_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef APCLI_SUPPORT
			rtmp_read_ap_client_from_file(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
			/* Igmp Snooping information*/
			rtmp_read_igmp_snoop_from_file(pAd, tmpbuf, pBuffer);
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef WDS_SUPPORT
			rtmp_read_wds_from_file(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */
#ifdef IDS_SUPPORT
			rtmp_read_ids_from_file(pAd, tmpbuf, pBuffer);
#endif /* IDS_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT

			if (RTMPGetKeyParameter("MACRepeaterEn", tmpbuf, 10, pBuffer, FALSE)) {
				BOOLEAN bEnable = FALSE;

				if (os_str_tol(tmpbuf, 0, 10) != 0)
					bEnable = TRUE;
				else
					bEnable = FALSE;

				AsicSetReptFuncEnable(pAd, bEnable);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MACRepeaterEn=%d\n", pAd->ApCfg.bMACRepeaterEn));
				/* Disable DFS zero wait support */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)

				if (pAd->ApCfg.bMACRepeaterEn) {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m%s:Disable DfsZeroWait\x1b[m\n", __func__));
				}

#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
			}

			if (RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, 10, pBuffer, FALSE)) {
				INT OuiMode = os_str_tol(tmpbuf, 0, 10);

				if (OuiMode == CASUALLY_DEFINE_MAC_ADDR)
					pAd->ApCfg.MACRepeaterOuiMode = CASUALLY_DEFINE_MAC_ADDR;
				else if (OuiMode == VENDOR_DEFINED_MAC_ADDR_OUI)
					pAd->ApCfg.MACRepeaterOuiMode = VENDOR_DEFINED_MAC_ADDR_OUI; /* customer specific */
				else
					pAd->ApCfg.MACRepeaterOuiMode = FOLLOW_CLI_LINK_MAC_ADDR_OUI; /* use Ap-Client first 3 bytes MAC assress (default) */

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MACRepeaterOuiMode=%d\n", pAd->ApCfg.MACRepeaterOuiMode));
			}

#endif /* MAC_REPEATER_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		if (RTMPGetKeyParameter("SE_OFF", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG SeOff = os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.bSeOff = SeOff > 0 ? TRUE : FALSE;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): SE_OFF=%d\n",
					 __func__, pAd->CommonCfg.bSeOff));
		}

		if (RTMPGetKeyParameter("AntennaIndex", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG antenna_index = simple_strtol(tmpbuf, 0, 10);

			if (antenna_index > 28)
				antenna_index = 0;

			if (antenna_index == 24 || antenna_index == 25)
				antenna_index = 0;

			pAd->CommonCfg.ucAntennaIndex = antenna_index;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): antenna_index=%d\n",
					 __func__, pAd->CommonCfg.ucAntennaIndex));
		}

#ifdef DOT11_N_SUPPORT
		read_ht_param_from_file(pAd, tmpbuf, pBuffer);
#ifdef DOT11_VHT_AC
		read_vht_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (RTMPGetKeyParameter("OldChannel_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldChannelCfg(pAd, tmpbuf);
		if (RTMPGetKeyParameter("OldChannel_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldChannelCfg(pAd, tmpbuf);

		if (RTMPGetKeyParameter("OldHTBW_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, FALSE);
		if (RTMPGetKeyParameter("OldHTBW_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, FALSE);

		if (RTMPGetKeyParameter("OldVHTBW_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, TRUE);
		if (RTMPGetKeyParameter("OldVHTBW_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, TRUE);
#endif

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WSC_AP_SUPPORT
			RTMP_STRING tok_str[16] = {0};

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				snprintf(tok_str, sizeof(tok_str), "WscDefaultSSID%d", i + 1);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
					NdisZeroMemory(&pAd->ApCfg.MBSSID[i].wdev.WscControl.WscDefaultSsid, sizeof(NDIS_802_11_SSID));
					NdisMoveMemory(pAd->ApCfg.MBSSID[i].wdev.WscControl.WscDefaultSsid.Ssid, tmpbuf, strlen(tmpbuf));
					pAd->ApCfg.MBSSID[i].wdev.WscControl.WscDefaultSsid.SsidLength = strlen(tmpbuf);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("WscDefaultSSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[i].wdev.WscControl.WscDefaultSsid.Ssid));
				}
			}

			/*WscConfMode*/
			if (RTMPGetKeyParameter("WscConfMode", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT WscConfMode = os_str_tol(macptr, 0, 10);

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (WscConfMode > 0 && WscConfMode < 8)
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfMode = WscConfMode;
					else
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfMode = WSC_DISABLE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("I/F(ra%d) WscConfMode=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfMode));
				}
			}

			/*WscConfStatus*/
			if (RTMPGetKeyParameter("WscConfStatus", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfStatus = (INT) os_str_tol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("I/F(ra%d) WscConfStatus=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfStatus));
				}
			}

			/*WscConfMethods*/
			if (RTMPGetKeyParameter("WscConfMethods", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfigMethods = (USHORT)os_str_tol(macptr, 0, 16);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("I/F(ra%d) WscConfMethods=0x%x\n", i, pAd->ApCfg.MBSSID[i].wdev.WscControl.WscConfigMethods));
				}
			}

			/*WscKeyASCII (0:Hex, 1:ASCII(random length), others: ASCII length, default 8)*/
			if (RTMPGetKeyParameter("WscKeyASCII", tmpbuf, 10, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT Value;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					Value = (INT) os_str_tol(tmpbuf, 0, 10);

					if (Value == 0 || Value == 1)
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscKeyASCII = Value;
					else if (Value >= 8 && Value <= 63)
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscKeyASCII = Value;
					else
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscKeyASCII = 8;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 ("WscKeyASCII=%d\n", pAd->ApCfg.MBSSID[i].wdev.WscControl.WscKeyASCII));
				}
			}

			if (RTMPGetKeyParameter("WscSecurityMode", tmpbuf, 50, pBuffer, TRUE)) {
				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[i].wdev.WscSecurityMode = WPAPSKTKIP;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT tmpMode = 0;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					tmpMode = (INT) os_str_tol(macptr, 0, 10);

					if (tmpMode <= WPAPSKTKIP)
						pAd->ApCfg.MBSSID[i].wdev.WscSecurityMode = tmpMode;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetProfileParameters I/F(ra%d) WscSecurityMode=%d\n",
							 i, pAd->ApCfg.MBSSID[i].wdev.WscSecurityMode));
				}
			}

			/* WCNTest*/
			if (RTMPGetKeyParameter("WCNTest", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN	bEn = FALSE;

				if ((strncmp(tmpbuf, "0", 1) == 0))
					bEn = FALSE;
				else
					bEn = TRUE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[i].wdev.WscControl.bWCNTest = bEn;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WCNTest=%d\n", bEn));
			}

			/*WSC UUID Str*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[i].wdev.WscControl;
				snprintf(tok_str, sizeof(tok_str), "WSC_UUID_Str%d", i + 1);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE)) {
					NdisMoveMemory(&pWpsCtrl->Wsc_Uuid_Str[0], tmpbuf, strlen(tmpbuf));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("UUID_Str[%d]=%s\n", i + 1, pWpsCtrl->Wsc_Uuid_Str));
				}
			}

			/*WSC UUID Hex*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[i].wdev.WscControl;
				snprintf(tok_str, sizeof(tok_str), "WSC_UUID_E%d", i + 1);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE)) {
					AtoH(tmpbuf, &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Wsc_Uuid_E[%d]", i + 1));
					hex_dump("", &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				}
			}

			/* WSC AutoTrigger Disable */
			if (RTMPGetKeyParameter("WscAutoTriggerDisable", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN	bEn = FALSE;

				if ((strncmp(tmpbuf, "0", 1) == 0))
					bEn = FALSE;
				else
					bEn = TRUE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[i].wdev.WscControl.bWscAutoTriggerDisable = bEn;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("bWscAutoTriggerDisable=%d\n", bEn));
			}

#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
			if (RTMPGetKeyParameter("RoamingEnhance", tmpbuf, 32, pBuffer, TRUE))
				pAd->ApCfg.bRoamingEnhance = (simple_strtol(tmpbuf, 0, 10) > 0)?TRUE:FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT

		/*CarrierDetect*/
		if (RTMPGetKeyParameter("CarrierDetect", tmpbuf, 128, pBuffer, TRUE)) {
			if ((strncmp(tmpbuf, "0", 1) == 0))
				pAd->CommonCfg.CarrierDetect.Enable = FALSE;
			else if ((strncmp(tmpbuf, "1", 1) == 0))
				pAd->CommonCfg.CarrierDetect.Enable = TRUE;
			else
				pAd->CommonCfg.CarrierDetect.Enable = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CarrierDetect.Enable=%d\n",
					 pAd->CommonCfg.CarrierDetect.Enable));
		} else
			pAd->CommonCfg.CarrierDetect.Enable = FALSE;

#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef BT_APCLI_SUPPORT
		if (RTMPGetKeyParameter("BTApCliAutoBWSupport", tmpbuf, 128, pBuffer, TRUE)) {
			INT BT_APCLI_Auto_BW_Support = 0;

			BT_APCLI_Auto_BW_Support = os_str_tol(tmpbuf, 0, 10);
			pAd->ApCfg.ApCliAutoBWBTSupport = BT_APCLI_Auto_BW_Support;
		}
#endif

#ifdef BW_VENDOR10_CUSTOM_FEATURE
		if (RTMPGetKeyParameter("ApCliSyncAutoBandSupport", tmpbuf, 128, pBuffer, TRUE)) {
			UCHAR policy = 0, value = 0;

			for (policy = 0, macptr = rstrtok(tmpbuf, ";"); (policy <= BAND_MAX_POLICY && macptr);
			macptr = rstrtok(NULL, ";"), policy++) {
				value = (UCHAR)os_str_tol(macptr, 0, 10);
				if (value == 0)
					continue;

				switch (policy) {
				case SAME_BAND_SYNC:
					SET_APCLI_SYNC_BAND_VALID(pAd, policy);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[%s] Policy %d Band Valid %d\n", __func__, policy, IS_APCLI_SYNC_BAND_VALID(pAd, policy)));
					break;

				case DEAUTH_PEERS:
					SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, TRUE);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[%s] Policy %d Deauth Valid %d\n", __func__, policy, IS_APCLI_SYNC_PEER_DEAUTH_VALID(pAd)));
					break;

				case DIFF_BAND_SYNC:
				case BAND_MAX_POLICY:
				default:
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Invalid Band Policy %d\n", __func__, policy));
					break;
				}
			}
		}

		if (RTMPGetKeyParameter("ApCliSyncAutoBWSupport", tmpbuf, 128, pBuffer, TRUE)) {
			UCHAR policy = 0, value = 0;
			POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
			struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

			if (IS_APCLI_SYNC_BAND_VALID(pAd, SAME_BAND_SYNC) || IS_APCLI_SYNC_BAND_VALID(pAd, DIFF_BAND_SYNC)) {
				for (policy = 0, macptr = rstrtok(tmpbuf, ";"); (policy <= BW_MAX_POLICY && macptr); macptr = rstrtok(NULL, ";"), policy++) {
					value = (UCHAR)os_str_tol(macptr, 0, 10);
					if (value == 0)
						continue;

					switch (policy) {
					case HT_2040_UP_ENBL:
					case HT_4020_DOWN_ENBL:
						SET_APCLI_AUTO_BW_HT_VALID(pAd, policy);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("[%s] Policy %d HT Valid %d\n", __func__, policy, IS_SYNC_BW_POLICY_VALID(pAd, TRUE, policy)));
						break;

					case VHT_80_2040_DOWN_ENBL:
					case VHT_2040_80_UP_ENBL:
						if (wlan_config_get_vht_bw(wdev) > VHT_BW_2040) {
							SET_APCLI_AUTO_BW_VHT_VALID(pAd, (policy-VHT_POLICY_OFFSET));
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								("[%s] Policy %d VHT Valid %d\n", __func__, policy, IS_SYNC_BW_POLICY_VALID(pAd, FALSE, (policy-VHT_POLICY_OFFSET))));

						} else if (policy == VHT_2040_80_UP_ENBL) {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("[%s] Policy VHT 20/40->80 InValid Curret Cap %d\n", __func__, wlan_config_get_vht_bw(wdev)));
						} else if (policy == VHT_80_2040_DOWN_ENBL) {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("[%s] Policy VHT 80->20/40 InValid Curret Cap %d\n", __func__, wlan_config_get_vht_bw(wdev)));
						}
						break;

					case VHT_160_2040_DOWN_ENBL:
					case VHT_160_80_DOWN_ENBL:
					case VHT_2040_160_UP_ENBL:
					case VHT_80_160_UP_ENBL:
					case BW_MAX_POLICY:
					default:
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Invalid BW Policy  %d\n", __func__, policy));
						break;
					}
				}

				if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd)) {
					/* Disable 40 MHz Intolerance & BSS Coex Enable Fields */
					pAd->CommonCfg.bBssCoexEnable = FALSE;
					pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
				} else {
					/* Disable Feature as Policy not updated */
					SET_APCLI_SYNC_BAND_FEATURE_DISABLE(pAd, POLICY_DISABLE);
					SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, POLICY_DISABLE);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Disable Auto Band Policy %d \n",
						__func__, IS_V10_AUTO_BAND_FEATURE_ENBL(pAd)));
				}
			}
		}
#endif

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MCAST_RATE_SPECIFIC

			/* McastPhyMode*/
			if (RTMPGetKeyParameter("McastPhyMode", tmpbuf, 32, pBuffer, TRUE)) {
				UCHAR PhyMode;
				HTTRANSMIT_SETTING *pTransmit;
				struct wifi_dev *wdev = get_default_wdev(pAd);
				UCHAR ht_bw = wlan_config_get_ht_bw(wdev);

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < 2); macptr = rstrtok(NULL, ";"), i++) {
					PhyMode = (UCHAR)os_str_tol(macptr, 0, 10);
					printk("%s: Mcast frame, i=%d,  Mode=%d!\n", __func__, i, PhyMode);
					/* UCHAR PhyMode = os_str_tol(tmpbuf, 0, 10); */
					pTransmit = (i == 0) ? (&pAd->CommonCfg.MCastPhyMode) : (&pAd->CommonCfg.MCastPhyMode_5G);
					pTransmit->field.BW = ht_bw;

					switch (PhyMode) {
					case MCAST_DISABLE: /* disable */
						NdisMoveMemory(pTransmit,
									   &pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING));

						if (i == 0) {
							pTransmit->field.MODE = MODE_CCK;
							pTransmit->field.BW =  BW_20;
							pTransmit->field.MCS = RATE_1;
						} else {
							pTransmit->field.MODE = MODE_OFDM;
							pTransmit->field.BW =  BW_20;
							pTransmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
						}

						break;

					case MCAST_CCK:	/* CCK*/
						if (i == 0) {
							pTransmit->field.MODE = MODE_CCK;
							pTransmit->field.BW =  BW_20;
						} else {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Could not set CCK mode for 5G band so set OFDM!\n"));
							pTransmit->field.MODE = MODE_OFDM;
							pTransmit->field.BW =  BW_20;
							/* pTransmit->field.MCS = OfdmRateToRxwiMCS[RATE_6]; */
						}

						break;

					case MCAST_OFDM:	/* OFDM*/
						pTransmit->field.MODE = MODE_OFDM;
						pTransmit->field.BW =  BW_20;
						break;
#ifdef DOT11_N_SUPPORT

					case MCAST_HTMIX:	/* HTMIX*/
						pTransmit->field.MODE = MODE_HTMIX;
						break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

					case MCAST_VHT: /* VHT */
						pTransmit->field.MODE = MODE_VHT;
						break;
#endif /* DOT11_VHT_AC */

					default:
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Unknown Multicast PhyMode %d.\n", PhyMode));
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set the default mode, MCAST_CCK!\n"));
						pTransmit->field.MODE = MODE_CCK;
						pTransmit->field.BW =  BW_20;
						break;
					}
				}

				if (i == 0) {
					memset(&pAd->CommonCfg.MCastPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
					memset(&pAd->CommonCfg.MCastPhyMode_5G, 0, sizeof(HTTRANSMIT_SETTING));
				} else if (i == 1) { /* single band */
					NdisMoveMemory(&pAd->CommonCfg.MCastPhyMode_5G,
								   &pAd->CommonCfg.MCastPhyMode, sizeof(HTTRANSMIT_SETTING));

					if (pAd->CommonCfg.MCastPhyMode_5G.field.MODE == MODE_CCK)
						pAd->CommonCfg.MCastPhyMode_5G.field.MODE = MODE_OFDM;
				}
			} else {
				/*
				NdisMoveMemory(&pAd->CommonCfg.MCastPhyMode,
								&pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING));
								*/
				memset(&pAd->CommonCfg.MCastPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
				memset(&pAd->CommonCfg.MCastPhyMode_5G, 0, sizeof(HTTRANSMIT_SETTING));
				/* printk("%s: Zero McastPhyMode!\n", __func__); */
			}

			/* McastMcs*/
			if (RTMPGetKeyParameter("McastMcs", tmpbuf, 32, pBuffer, TRUE)) {
				HTTRANSMIT_SETTING *pTransmit;
				UCHAR Mcs;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < 2); macptr = rstrtok(NULL, ";"), i++) {
					Mcs = (UCHAR)os_str_tol(macptr, 0, 10);
					pTransmit = (i == 0) ? (&pAd->CommonCfg.MCastPhyMode) : (&pAd->CommonCfg.MCastPhyMode_5G);
					printk("%s: Mcast frame, i=%d,  MCS=%d!\n", __func__, i, Mcs);

					switch (pTransmit->field.MODE) {
					case MODE_CCK:
						if (i == 1) {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Could not set CCK mode for 5G band!\n"));
							break;
						}

						if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11))
							pTransmit->field.MCS = Mcs;
						else
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n"));

						break;

					case MODE_OFDM:
						if (Mcs > 7)
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS must in range from 0 to 7 for OFDM Mode.\n"));
						else
							pTransmit->field.MCS = Mcs;

						break;

					default:
						pTransmit->field.MCS = Mcs;
						break;
					}
				}
			} else {
				pAd->CommonCfg.MCastPhyMode.field.MCS = RATE_1;
				pAd->CommonCfg.MCastPhyMode_5G.field.MCS = OfdmRateToRxwiMCS[RATE_6];
			}

#ifdef MCAST_BCAST_RATE_SET_SUPPORT
			NdisMoveMemory(&pAd->CommonCfg.BCastPhyMode_5G,
				   &pAd->CommonCfg.MCastPhyMode_5G, sizeof(HTTRANSMIT_SETTING));
			NdisMoveMemory(&pAd->CommonCfg.BCastPhyMode,
				   &pAd->CommonCfg.MCastPhyMode, sizeof(HTTRANSMIT_SETTING));
#endif /* MCAST_BCAST_RATE_SET_SUPPORT */

			/*
			printk("%s: Mcast Mode=%d %d, BW=%d %d, MCS=%d %d\n", __func__,
				pAd->CommonCfg.MCastPhyMode.field.MODE, pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
				pAd->CommonCfg.MCastPhyMode.field.BW, pAd->CommonCfg.MCastPhyMode_5G.field.BW,
				pAd->CommonCfg.MCastPhyMode.field.MCS,  pAd->CommonCfg.MCastPhyMode_5G.field.MCS);
			*/
#endif /* MCAST_RATE_SPECIFIC */

#ifdef MIN_PHY_RATE_SUPPORT
			if (RTMPGetKeyParameter("MinPhyDataRate", tmpbuf, 100, pBuffer, TRUE)) {
				RTMPMinPhyDataRateCfg(pAd, tmpbuf);
			}
			if (RTMPGetKeyParameter("MinPhyBeaconRate", tmpbuf, 100, pBuffer, TRUE)) {
				RTMPMinPhyBeaconRateCfg(pAd, tmpbuf);
			}
			if (RTMPGetKeyParameter("MinPhyMgmtRate", tmpbuf, 100, pBuffer, TRUE)) {
				RTMPMinPhyMgmtRateCfg(pAd, tmpbuf);
			}
			if (RTMPGetKeyParameter("MinPhyBCMCRate", tmpbuf, 100, pBuffer, TRUE)) {
				RTMPMinPhyBcMcRateCfg(pAd, tmpbuf);
			}
			if (RTMPGetKeyParameter("LimitClientSupportRate", tmpbuf, 100, pBuffer, TRUE)) {
				RTMPLimitClientSupportRateCfg(pAd, tmpbuf);
			}
			if (RTMPGetKeyParameter("DisableCCKRate", tmpbuf, 100, pBuffer, TRUE)) {
				RTMPDisableCCKRateCfg(pAd, tmpbuf);
			}
#endif /* MIN_PHY_RATE_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
		rtmp_read_wsc_user_parms_from_file(pAd, tmpbuf, pBuffer);

		/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
		if (RTMPGetKeyParameter("Wsc4digitPinCode", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0)	/* Enable */
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
					else /* Disable */
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("I/F(ra%d) Wsc4digitPinCode=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.WscControl.WscEnrollee4digitPinCode));
				}
			}
#endif /* CONFIG_AP_SUPPORT // */
		}

		if (RTMPGetKeyParameter("WscVendorPinCode", tmpbuf, 256, pBuffer, TRUE)) {
			PWSC_CTRL pWscContrl;
			int bSetOk;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				pWscContrl = &pAd->ApCfg.MBSSID[BSS0].wdev.WscControl;
			}
#endif /* CONFIG_AP_SUPPORT */
			bSetOk = RT_CfgSetWscPinCode(pAd, tmpbuf, pWscContrl);

			if (bSetOk)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - WscVendorPinCode= (%d)\n", __func__, bSetOk));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - WscVendorPinCode: invalid pin code(%s)\n", __func__,
						 tmpbuf));
		}

#ifdef WSC_V2_SUPPORT

		if (RTMPGetKeyParameter("WscV2Support", tmpbuf, 32, pBuffer, TRUE)) {
			UCHAR			bEnable;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					bEnable = (UCHAR)os_str_tol(macptr, 0, 10);
					pAd->ApCfg.MBSSID[i].wdev.WscControl.WscV2Info.bEnableWpsV2 = bEnable;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WscV2Support=%d\n", i, bEnable));
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

#endif /* WSC_V2_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
		FT_rtmp_read_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		/* EntryLifeCheck is used to check */
		if (RTMPGetKeyParameter("EntryLifeCheck", tmpbuf, 256, pBuffer, TRUE)) {
			long LifeCheckCnt = os_str_tol(tmpbuf, 0, 10);

			if ((LifeCheckCnt <= 65535) && (LifeCheckCnt != 0))
				pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
			else
				pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("EntryLifeCheck=%ld\n", pAd->ApCfg.EntryLifeCheck));
		}

#ifdef DOT11K_RRM_SUPPORT
		RRM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT

		if (RTMPGetKeyParameter("VideoTurbine", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.Enable = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video Enable=%d\n", pAd->VideoTurbine.Enable));
		}

		if (RTMPGetKeyParameter("VideoClassifierEnable", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.ClassifierEnable = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video ClassifierEnable=%d\n",
					 pAd->VideoTurbine.ClassifierEnable));
		}

		if (RTMPGetKeyParameter("VideoHighTxMode", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.HighTxMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video HighTxMode=%d\n", pAd->VideoTurbine.HighTxMode));
		}

		if (RTMPGetKeyParameter("VideoTxPwr", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.TxPwr = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video TxPwr=%d\n", pAd->VideoTurbine.TxPwr));
		}

		if (RTMPGetKeyParameter("VideoMCSEnable", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.VideoMCSEnable = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video VideoMCSEnable=%d\n", pAd->VideoTurbine.VideoMCSEnable));
		}

		if (RTMPGetKeyParameter("VideoMCS", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.VideoMCS = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video VideoMCS=%d\n", pAd->VideoTurbine.VideoMCS));
		}

		if (RTMPGetKeyParameter("VideoTxBASize", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.TxBASize = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video TxBASize=%d\n", pAd->VideoTurbine.TxBASize));
		}

		if (RTMPGetKeyParameter("VideoTxLifeTimeMode", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.TxLifeTimeMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video TxLifeTimeMode=%d\n", pAd->VideoTurbine.TxLifeTimeMode));
		}

		if (RTMPGetKeyParameter("VideoTxLifeTime", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.TxLifeTime = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video TxLifeTime=%d\n", pAd->VideoTurbine.TxLifeTime));
		}

		if (RTMPGetKeyParameter("VideoTxRetryLimit", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->VideoTurbine.TxRetryLimit = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Video TxRetryLimit=%d\n", pAd->VideoTurbine.TxRetryLimit));
		}

#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

		/* set GPIO pin for wake-up signal */
		if (RTMPGetKeyParameter("WOW_GPIO", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIO(pAd, tmpbuf);

		/* set WOW enable/disable */
		if (RTMPGetKeyParameter("WOW_Enable", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Enable(pAd, tmpbuf);

		/* set delay time for WOW really enable */
		if (RTMPGetKeyParameter("WOW_Delay", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Delay(pAd, tmpbuf);

		/* set GPIO pulse hold time */
		if (RTMPGetKeyParameter("WOW_Hold", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Hold(pAd, tmpbuf);

		/* set wakeup signal type */
		if (RTMPGetKeyParameter("WOW_InBand", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_InBand(pAd, tmpbuf);

		/* set wakeup interface */
		if (RTMPGetKeyParameter("WOW_Interface", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Interface(pAd, tmpbuf);

		/* set if down interface */
		if (RTMPGetKeyParameter("WOW_IfDown_Support", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_IfDown_Support(pAd, tmpbuf);

		/* set GPIO High Low */
		if (RTMPGetKeyParameter("WOW_GPIOHighLow", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIOHighLow(pAd, tmpbuf);

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */
#ifdef MICROWAVE_OVEN_SUPPORT

		if (RTMPGetKeyParameter("MO_FalseCCATh", tmpbuf, 10, pBuffer, TRUE))
			Set_MO_FalseCCATh_Proc(pAd, tmpbuf);

#endif /* MICROWAVE_OVEN_SUPPORT */

		if (RTMPGetKeyParameter("PS_RETRIEVE", tmpbuf, 10, pBuffer, TRUE)) {
			long PS_RETRIEVE;
			PS_RETRIEVE = os_str_tol(tmpbuf, 0, 10);
			pAd->bPS_Retrieve = PS_RETRIEVE;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PS_RETRIEVE = %lx\n", PS_RETRIEVE));
		}

#ifdef FTM_SUPPORT

		if (RTMPGetKeyParameter("LocCivicAddr", tmpbuf, 256, pBuffer, TRUE)) {
			UINT8 i = 0;
			NdisZeroMemory(pAd->pFtmCtrl->CA_Value, MAX_CIVIC_CA_VALUE_LENGTH);

			for (i = 0; i < MAX_CIVIC_CA_VALUE_LENGTH && *(tmpbuf + i * 2) != 0; i++)
				AtoH(tmpbuf + i * 2, &pAd->pFtmCtrl->CA_Value[i], 1);

			hex_dump("LocCA", pAd->pFtmCtrl->CA_Value, MAX_CIVIC_CA_VALUE_LENGTH);
			NdisZeroMemory(tmpbuf, MAX_CIVIC_CA_VALUE_LENGTH);
		}

		if (RTMPGetKeyParameter("LocCivicAddrType", tmpbuf, 10, pBuffer, TRUE)) {
			/* long CivicAddrType; */
			pAd->pFtmCtrl->Civic.CA_Type = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CivicAddrType = %d\n", pAd->pFtmCtrl->Civic.CA_Type));
		}

		if (RTMPGetKeyParameter("LocCivicAddrLength", tmpbuf, 10, pBuffer, TRUE)) {
			/* long CivicAddrLength; */
			pAd->pFtmCtrl->Civic.CA_Length = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CivicAddrLength = %d\n", pAd->pFtmCtrl->Civic.CA_Length));

			if (pAd->pFtmCtrl->Civic.CA_Length == 0)
				Set_FtmCivicKnown_Proc(pAd, "0");
			else {
				if (pAd->pFtmCtrl->Civic.CA_Length >= MAX_CIVIC_CA_VALUE_LENGTH)
					pAd->pFtmCtrl->Civic.CA_Length = MAX_CIVIC_CA_VALUE_LENGTH;

				Set_FtmCivicKnown_Proc(pAd, "1");
			}
		}

		if (RTMPGetKeyParameter("LocLCI", tmpbuf, 50, pBuffer, TRUE)) {
			/* long CivicAddrLength; */
			if (strlen(tmpbuf) <= 1)
				Set_FtmLciKnown_Proc(pAd, "0");
			else if (strlen(tmpbuf) == 46) {
				PLCI_FIELD pLci = &pAd->pFtmCtrl->LciField;
				NdisZeroMemory(pAd->pFtmCtrl->LciField.byte, sizeof(pAd->pFtmCtrl->LciField.byte));
				FtmMapSigmaCmdToLocLCI(pAd, tmpbuf, pLci);
				Set_FtmLciKnown_Proc(pAd, "1");
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("GOT LCI Length = %d\n", strlen(tmpbuf)));
		}

		if (RTMPGetKeyParameter("LocFloorInfoZ", tmpbuf, 10, pBuffer, TRUE)) {
			long LocFloorInfoZ;
			LocFloorInfoZ = os_str_tol(tmpbuf, 0, 10);
			pAd->pFtmCtrl->LciZ.Floor.word = (UINT16)LocFloorInfoZ;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("LciZ.Floor.word = %d\n", pAd->pFtmCtrl->LciZ.Floor.word));
		}

		if (RTMPGetKeyParameter("LocHeightAboveFloorZ", tmpbuf, 10, pBuffer, TRUE)) {
			long HeightAboveFloorZ;
			HeightAboveFloorZ = os_str_tol(tmpbuf, 0, 10);
			pAd->pFtmCtrl->LciZ.HeightAboveFloor = (UINT16)HeightAboveFloorZ;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("LciZ.HeightAboveFloor = %d\n", pAd->pFtmCtrl->LciZ.HeightAboveFloor));
		}

		if (RTMPGetKeyParameter("LocHeightAboveFloorUncZ", tmpbuf, 10, pBuffer, TRUE)) {
			long LocHeightAboveFloorUncZ;
			LocHeightAboveFloorUncZ = os_str_tol(tmpbuf, 0, 10);
			pAd->pFtmCtrl->LciZ.HeightUncertainty = (UINT8)LocHeightAboveFloorUncZ;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("LciZ.HeightUncertainty = %d\n", pAd->pFtmCtrl->LciZ.HeightUncertainty));
		}

		if (RTMPGetKeyParameter("LocReTxAllowed", tmpbuf, 10, pBuffer, TRUE)) {
			long LocReTxAllowed;
			LocReTxAllowed = os_str_tol(tmpbuf, 0, 10);

			if (LocReTxAllowed)
				pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.RetransAllowed = 0x1;
			else
				pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.RetransAllowed = 0x0;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("LciUsage.RulesAndPolicy.field.RetransAllowed = %d\n"
					  , pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.RetransAllowed));
		}

		if (RTMPGetKeyParameter("LocRadioMsnt", tmpbuf, 10, pBuffer, TRUE)) {
			long LocRadioMsnt = os_str_tol(tmpbuf, 0, 10);

			/* Enable/Disable functions of radio measurement reports */
			switch (LocRadioMsnt) {
			case 0:
				pAd->pFtmCtrl->bSetCivicRpt = FALSE;
				pAd->pFtmCtrl->bSetLciRpt = FALSE;
				break;

			case 1:
				pAd->pFtmCtrl->bSetCivicRpt = FALSE;
				pAd->pFtmCtrl->bSetLciRpt = TRUE;
				break;

			case 2:
				pAd->pFtmCtrl->bSetCivicRpt = TRUE;
				pAd->pFtmCtrl->bSetLciRpt = FALSE;
				break;

			case 3:
				pAd->pFtmCtrl->bSetCivicRpt = TRUE;
				pAd->pFtmCtrl->bSetLciRpt = TRUE;
				break;

			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("ERROR! Illegal LocRadioMsnt = %ld\n", LocRadioMsnt));
				pAd->pFtmCtrl->bSetCivicRpt = TRUE;
				pAd->pFtmCtrl->bSetLciRpt = TRUE;
				break;
			}
		}

		if (RTMPGetKeyParameter("LocNeighResp", tmpbuf, 32, pBuffer, TRUE)) {
			/*not exist in CAPI v08*/
		}

		if (RTMPGetKeyParameter("LocFTMresp", tmpbuf, 32, pBuffer, TRUE)) {
			/*not exist in CAPI v08*/
		}

		if (RTMPGetKeyParameter("LocNeighBssid", tmpbuf, 32, pBuffer, TRUE)) {
			if (strlen(tmpbuf) != 0) {
				COPY_MAC_ADDR(pAd->pFtmCtrl->FtmNeighbor[0].NeighborBSSID, tmpbuf);
				pAd->pFtmCtrl->FtmNeighbor[0].NeighborValid = 1;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("LocNeighborBSSID = %02x:%02x:%02x:%02x:%02x:%02x\n"
					 , PRINT_MAC(pAd->pFtmCtrl->FtmNeighbor[0].NeighborBSSID)));
		}

		if (RTMPGetKeyParameter("LocNeighPHYtype", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->pFtmCtrl->FtmNeighbor[0].NeighborPhyType = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("LocNeighborPhyType = %d\n"
					 , pAd->pFtmCtrl->FtmNeighbor[0].NeighborPhyType));
		}

		if (RTMPGetKeyParameter("LocNeighFTMrespCap", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->pFtmCtrl->FtmNeighbor[0].NeighborFTMrespCap = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("LocNeighborFTMrespCap = %d\n"
					 , pAd->pFtmCtrl->FtmNeighbor[0].NeighborFTMrespCap));
		}

		if (RTMPGetKeyParameter("LocNeighOpClass", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->pFtmCtrl->FtmNeighbor[0].NeighborOpClass = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("LocNeighborOpClass = %d\n"
					 , pAd->pFtmCtrl->FtmNeighbor[0].NeighborOpClass));
		}

		if (RTMPGetKeyParameter("LocNeighChannel", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->pFtmCtrl->FtmNeighbor[0].NeighborChannel = os_str_tol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("LocNeighChannel = %d\n"
					 , pAd->pFtmCtrl->FtmNeighbor[0].NeighborChannel));
		}

		if (RTMPGetKeyParameter("NeighAPInfo1", tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE)) {
			if (strlen(tmpbuf) > 0) {
				UINT NeighborIdx = 0;
				FtmProfileNeighborApParse(pAd, NeighborIdx, tmpbuf);
			}
		}

		if (RTMPGetKeyParameter("NeighAPInfo2", tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE)) {
			if (strlen(tmpbuf) > 0) {
				UINT NeighborIdx = 1;
				FtmProfileNeighborApParse(pAd, NeighborIdx, tmpbuf);
			}
		}

#endif /* FTM_SUPPORT */
#ifdef FW_DUMP_SUPPORT

		if (RTMPGetKeyParameter("FWDump_Path", tmpbuf, 10, pBuffer, TRUE))
			set_fwdump_path(pAd, tmpbuf);

		if (RTMPGetKeyParameter("FWDump_MaxSize", tmpbuf, 10, pBuffer, TRUE))
			set_fwdump_max_size(pAd, tmpbuf);

#endif
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)

		if (RTMPGetKeyParameter("IcapMode", tmpbuf, 10, pBuffer, TRUE)) {
			UINT8 ICapMode; /* 0 : Normal Mode; 1 : Internal Capture; 2 : Wifi Spectrum */
			ICapMode = simple_strtol(tmpbuf, 0, 10);
			pAd->ICapMode = ICapMode;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ICapMode = %d\n", ICapMode));
		}

#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		rtmp_read_vow_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */
#ifdef RED_SUPPORT
		rtmp_read_red_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
		rtmp_read_fq_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* FQ_SCH_SUPPORT */
		rtmp_read_cp_parms_from_file(pAd, tmpbuf, pBuffer);
	} while (0);

	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

#ifdef WSC_INCLUDED
void rtmp_read_wsc_user_parms(
	PWSC_CTRL pWscControl,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	if (RTMPGetKeyParameter("WscManufacturer", tmpbuf, WSC_MANUFACTURE_LEN, buffer, TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.Manufacturer, WSC_MANUFACTURE_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.Manufacturer, tmpbuf, strlen(tmpbuf));

		if (pWscControl->RegData.SelfInfo.Manufacturer[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x01);
	}

	/*WSC_User_ModelName*/
	if (RTMPGetKeyParameter("WscModelName", tmpbuf, WSC_MODELNAME_LEN, buffer, TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelName, WSC_MODELNAME_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelName, tmpbuf, strlen(tmpbuf));

		if (pWscControl->RegData.SelfInfo.ModelName[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x02);
	}

	/*WSC_User_DeviceName*/
	if (RTMPGetKeyParameter("WscDeviceName", tmpbuf, WSC_DEVICENAME_LEN, buffer, TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.DeviceName, WSC_DEVICENAME_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.DeviceName, tmpbuf, strlen(tmpbuf));

		if (pWscControl->RegData.SelfInfo.DeviceName[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x04);
	}

	/*WSC_User_ModelNumber*/
	if (RTMPGetKeyParameter("WscModelNumber", tmpbuf, WSC_MODELNUNBER_LEN, buffer, TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelNumber, WSC_MODELNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelNumber, tmpbuf, strlen(tmpbuf));

		if (pWscControl->RegData.SelfInfo.ModelNumber[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x08);
	}

	/*WSC_User_SerialNumber*/
	if (RTMPGetKeyParameter("WscSerialNumber", tmpbuf, WSC_SERIALNUNBER_LEN, buffer, TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.SerialNumber, WSC_SERIALNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.SerialNumber, tmpbuf, strlen(tmpbuf));

		if (pWscControl->RegData.SelfInfo.SerialNumber[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x10);
	}
}

void rtmp_read_wsc_user_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	PWSC_CTRL           pWscControl;
#ifdef WSC_AP_SUPPORT
	int i = 0;

	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
		pWscControl = &pAd->ApCfg.MBSSID[i].wdev.WscControl;
		rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
	}

#ifdef APCLI_SUPPORT
	pWscControl = &pAd->ApCfg.ApCliTab[0].wdev.WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */
#ifdef WSC_STA_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* WSC_STA_SUPPORT */
}
#endif/*WSC_INCLUDED*/

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
void rtmp_read_vow_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	UINT8		i = 0, j = 0;
	CHAR		*ptok = NULL;
	CHAR		*macptr;
	CHAR		*tmp;
	CHAR		*pwatf_string;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin -->\n",
			 __func__));
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.en_bw_ctrl =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_BW_Ctrl --> %d\n",
				 pAd->vow_cfg.en_bw_ctrl));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.en_airtime_fairness =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Airtime_Fairness_En --> %d\n",
				 pAd->vow_cfg.en_airtime_fairness));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Airtime_Ctrl_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].at_on =  os_str_tol(ptok, 0, 10) != 0 ? TRUE : FALSE;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Airtime_Ctrl_En --> %d\n",
					 i, pAd->vow_bss_cfg[i].at_on));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Rate_Ctrl_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].bw_on =  os_str_tol(ptok, 0, 10) != 0 ? TRUE : FALSE;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Rate_Ctrl_En --> %d\n",
					 i, pAd->vow_bss_cfg[i].bw_on));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_RX_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_rx_time_cfg.rx_time_en =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_RX_En --> %d\n",
				 pAd->vow_rx_time_cfg.rx_time_en));
	}

	/* for gorup setting */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Rate", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].min_rate =  (UINT16)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].min_rate_token = vow_convert_rate_token(pAd, VOW_MIN, i);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Rate --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_rate));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Rate", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].max_rate =  (UINT16)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].max_rate_token = vow_convert_rate_token(pAd, VOW_MAX, i);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Rate --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_rate));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Ratio", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].min_airtime_ratio =  (UINT8)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].min_airtime_token = vow_convert_airtime_token(pAd, VOW_MIN, i);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Ratio --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_airtime_ratio));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Ratio", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].max_airtime_ratio =  (UINT8)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].max_airtime_token = vow_convert_airtime_token(pAd, VOW_MAX, i);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Ratio --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_airtime_ratio));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Refill_Period", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.refill_period =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Refill_Period --> %d\n",
				 pAd->vow_cfg.refill_period));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Rate_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].min_ratebucket_size =  (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Rate_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_ratebucket_size));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Rate_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].max_ratebucket_size =  (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Rate_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_ratebucket_size));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Airtime_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].min_airtimebucket_size =  (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Airtime_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_airtimebucket_size));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Airtime_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].max_airtimebucket_size = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Airtime_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_airtimebucket_size));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Backlog", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].max_backlog_size = (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Backlog --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_backlog_size));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].max_wait_time = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Wait_Time --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_wait_time));
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pAd->vow_bss_cfg[i].dwrr_quantum =  (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_DWRR_Quantum --> %d\n",
					 i, pAd->vow_bss_cfg[i].dwrr_quantum));
		}
	}

	/* for stations */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_VO_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VO] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_VO_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10)));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_VI_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VI] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_VI_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10)));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_BE_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BE] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_BE_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10)));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_BK_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BK] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_BK_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10)));
	}

	/* for group/stations control */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band0", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.dbdc0_search_rule =  os_str_tol(tmpbuf, 0, 10) ? 1 : 0;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_WMM_Search_Rule_Band0 --> %d\n",
				 pAd->vow_cfg.dbdc0_search_rule));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band1", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.dbdc1_search_rule =  os_str_tol(tmpbuf, 0, 10) ? 1 : 0;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_WMM_Search_Rule_Band1 --> %d\n",
				 pAd->vow_cfg.dbdc1_search_rule));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_DWRR_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.sta_max_wait_time =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_DWRR_Max_Wait_Time --> %d\n",
				 pAd->vow_cfg.sta_max_wait_time));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_DWRR_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.group_max_wait_time =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Group_DWRR_Max_Wait_Time --> %d\n",
				 pAd->vow_cfg.group_max_wait_time));
	}

	/* Weigthed Airtime Fairness - Enable/Disable*/
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_watf_en =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_WATF_Enable --> %d\n", pAd->vow_watf_en));
	}

	if (pAd->vow_watf_en) {
		/* Weigthed Airtime Fairness - Different DWRR quantum value*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV0", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv0 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV0 --> %d\n", pAd->vow_watf_q_lv0));
		} else
			pAd->vow_watf_q_lv0 = 4;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV1", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv1 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV1 --> %d\n", pAd->vow_watf_q_lv1));
		} else
			pAd->vow_watf_q_lv1 = 8;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV2", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv2 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV2 --> %d\n", pAd->vow_watf_q_lv2));
		} else
			pAd->vow_watf_q_lv2 = 12;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV3", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv3 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV3 --> %d\n", pAd->vow_watf_q_lv3));
		} else
			pAd->vow_watf_q_lv3 = 16;

		/* Weigthed Airtime Fairness - Different DWRR quantum MAC address list*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
		os_alloc_mem(NULL, (UCHAR **)&pwatf_string, 32);
		os_alloc_mem(NULL, (UCHAR **)&tmp, 32);

		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			snprintf(pwatf_string, 32, "VOW_WATF_MAC_LV%d", i);

			if (RTMPGetKeyParameter(pwatf_string, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (pAd->vow_watf_en)) {
				for (j = 0, macptr = rstrtok(tmpbuf, ","); macptr; macptr = rstrtok(NULL, ","), j++) {
					if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
						continue;

					snprintf(tmp, 32, "%d-%s", i, macptr);
					set_vow_watf_add_entry(pAd, tmp);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d-%s", i, macptr));
				}
			}
		}

		if (pwatf_string != NULL)
			os_free_mem(pwatf_string);

		if (tmp != NULL)
			os_free_mem(tmp);
	}

	/* fast round robin */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (RTMPGetKeyParameter("VOW_STA_FRR_QUANTUM", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_sta_frr_quantum =  (UINT8)simple_strtol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("VOW_STA_FRR_QUANTUM --> %d\n", pAd->vow_sta_frr_quantum));
	}

}
#endif /* CONFIG_AP_SUPPORT */
#endif  /*  VOW_SUPPORT */

#ifdef RED_SUPPORT
void rtmp_read_red_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin -->\n", __func__));
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("RED_Enable", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->red_en =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RED_Enable --> %d\n", pAd->red_en));
	}
}
#endif  /*  RED_SUPPORT */

#ifdef FQ_SCH_SUPPORT
void rtmp_read_fq_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin -->\n", __func__));
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("FQ_Enable", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		if (!(pAd->fq_ctrl.enable & FQ_READY))
			pAd->fq_ctrl.enable = os_str_tol(tmpbuf, 0, 10) != 0 ? FQ_NEED_ON : 0;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("FQ_Enable --> %d\n", pAd->fq_ctrl.enable));
	}
}
#endif  /* FQ_SCH_SUPPORT */


void rtmp_read_cp_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin -->\n", __func__));
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("CP_SUPPORT", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->cp_support =  os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CP_SUPPORT --> %d\n", pAd->cp_support));
	}
}

#ifdef SINGLE_SKU_V2
/* TODO: shiang-usw, for MT76x0 series, currently cannot use this function! */
NDIS_STATUS RTMPSetSkuParam(RTMP_ADAPTER *pAd)
{
    NDIS_STATUS ret;
#if defined(MT7615) || defined(MT7622)
    ret = MtSingleSkuLoadParam(pAd);
#else
    ret = MtPwrLimitLoadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_SKU);
#endif /* defined(MT7615) || defined(MT7622) */
    return ret;
}

NDIS_STATUS RTMPSetBackOffParam(RTMP_ADAPTER *pAd)
{
    NDIS_STATUS ret;
#if defined(MT7615) || defined(MT7622)
    ret = MtBfBackOffLoadParam(pAd);
#else
    ret = MtPwrLimitLoadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF);
#endif /* defined(MT7615) || defined(MT7622) */
    return ret;
}

NDIS_STATUS RTMPResetSkuParam(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7622)
    MtSingleSkuUnloadParam(pAd);
#else
    MtPwrLimitUnloadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_SKU);
#endif /* defined(MT7615) || defined(MT7622) */
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS RTMPResetBackOffParam(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7622)
    MtBfBackOffUnloadParam(pAd);
#else
    MtPwrLimitUnloadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF);
#endif /* defined(MT7615) || defined(MT7622) */
    return NDIS_STATUS_SUCCESS;
}

UCHAR GetSkuChannelBasePwr(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev,
	IN UCHAR			channel)
{
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UCHAR base_pwr = pAd->DefaultTargetPwr;
	UINT8 i, j;
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitSkuList, CH_POWER, List) {
		start_ch = ch->StartChannel;

		if (channel >= start_ch) {
			for (j = 0; j < ch->num; j++) {
				if (channel == ch->Channel[j]) {
					for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++) {
						if (base_pwr > ch->u1PwrLimitCCK[i])
							base_pwr = ch->u1PwrLimitCCK[i];
					}

					for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++) {
						if (base_pwr > ch->u1PwrLimitOFDM[i])
							base_pwr = ch->u1PwrLimitOFDM[i];
					}

					for (i = 0; i < SINGLE_SKU_TABLE_HT_LENGTH; i++) {
						if (base_pwr > ch->u1PwrLimitHT20[i])
							base_pwr = ch->u1PwrLimitHT20[i];
					}

					if (wlan_operate_get_bw(wdev) == BW_40) {
						for (i = 0; i < SINGLE_SKU_TABLE_HT_LENGTH; i++) {
							if (ch->u1PwrLimitHT40[i] == 0)
								break;

							if (base_pwr > ch->u1PwrLimitHT40[i])
								base_pwr = ch->u1PwrLimitHT40[i];
						}
					}

					break;
				}
			}
		}
	}
	return base_pwr;
}

#define	SKU_PHYMODE_CCK_1M_2M				0
#define	SKU_PHYMODE_CCK_5M_11M				1
#define	SKU_PHYMODE_OFDM_6M_9M				2
#define	SKU_PHYMODE_OFDM_12M_18M			3
#define	SKU_PHYMODE_OFDM_24M_36M			4
#define	SKU_PHYMODE_OFDM_48M_54M			5
#define	SKU_PHYMODE_HT_MCS0_MCS1			6
#define	SKU_PHYMODE_HT_MCS2_MCS3			7
#define	SKU_PHYMODE_HT_MCS4_MCS5			8
#define	SKU_PHYMODE_HT_MCS6_MCS7			9
#define	SKU_PHYMODE_HT_MCS8_MCS9			10
#define	SKU_PHYMODE_HT_MCS10_MCS11			11
#define	SKU_PHYMODE_HT_MCS12_MCS13			12
#define	SKU_PHYMODE_HT_MCS14_MCS15			13
#define	SKU_PHYMODE_STBC_MCS0_MCS1			14
#define	SKU_PHYMODE_STBC_MCS2_MCS3			15
#define	SKU_PHYMODE_STBC_MCS4_MCS5			16
#define	SKU_PHYMODE_STBC_MCS6_MCS7			17


VOID InitSkuRateDiffTable(
	IN PRTMP_ADAPTER	pAd)
{
	USHORT		i, value;
	CHAR		BasePwr, Pwr;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value);
	BasePwr = (value >> 8) & 0xFF;
	BasePwr = (BasePwr > 0x1F) ? BasePwr - 0x40 : BasePwr;

	for (i = 0; i < 9; i++) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i * 2, value);
		Pwr = value & 0xFF;
		Pwr = (Pwr > 0x1F) ? Pwr - 0x40 : Pwr;
		pAd->SingleSkuRatePwrDiff[i * 2] = Pwr - BasePwr;
		Pwr = (value >> 8) & 0xFF;
		Pwr = (Pwr > 0x1F) ? Pwr - 0x40 : Pwr;
		pAd->SingleSkuRatePwrDiff[i * 2 + 1] = Pwr - BasePwr;
	}
}
#endif /* SINGLE_SKU_V2 */


INT32 ralinkrate[] = {
	/* CCK */
	2, 4, 11, 22,
	/* OFDM */
	12, 18, 24, 36, 48, 72, 96, 108,
	/* 20MHz, 800ns GI, MCS: 0 ~ 15 */
	13, 26, 39, 52, 78, 104, 117, 130, 26, 52, 78, 104, 156, 208, 234, 260,
	/* 20MHz, 800ns GI, MCS: 16 ~ 23 */
	39, 78, 117, 156, 234, 312, 351, 390,
	/* 40MHz, 800ns GI, MCS: 0 ~ 15 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	/* 40MHz, 800ns GI, MCS: 16 ~ 23 */
	81, 162, 243, 324, 486, 648, 729, 810,
	/* 20MHz, 400ns GI, MCS: 0 ~ 15 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59, 87, 115, 173, 230, 260, 288,
	/* 20MHz, 400ns GI, MCS: 16 ~ 23 */
	43, 87, 130, 173, 260, 317, 390, 433,
	/* 40MHz, 400ns GI, MCS: 0 ~ 15 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	/* 40MHz, 400ns GI, MCS: 16 ~ 23 */
	90, 180, 270, 360, 540, 720, 810, 900
};

UINT32 RT_RateSize = sizeof(ralinkrate);
