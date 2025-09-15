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
	mt_bbp.h
*/

#ifndef __MT_BBP_H__
#define __MT_BBP_H__

INT32 MTShowAllBBP(struct _RTMP_ADAPTER *pAd);
INT32 MTShowPartialBBP(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);
INT mt_phy_probe(struct _RTMP_ADAPTER *pAd);

#endif /* __MT_BBP_H__ */

