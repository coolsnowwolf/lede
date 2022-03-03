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
    vr_ikans.h

    Abstract:
    Only for IKANOS Vx160 or Vx180 platform.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Sample Lin	01-28-2008    Created

 */

#ifndef __VR_IKANS_H__
#define __VR_IKANS_H__

#ifdef IKANOS_VX_1X0
typedef void (*IkanosWlanTxCbFuncP)(void *, void *);

struct IKANOS_TX_INFO {
	struct net_device *netdev;
	IkanosWlanTxCbFuncP *fp;
};
#endif /* IKANOS_VX_1X0 */

#ifndef MODULE_IKANOS
extern void VR_IKANOS_FP_Init(UINT8 BssNum, UINT8 *pApMac);
extern INT32 IKANOS_DataFramesTx(struct sk_buff *pSkb, struct net_device *pNetDev);
extern void IKANOS_DataFrameRx(PRTMP_ADAPTER pAd, struct sk_buff *pSkb);
#else
void VR_IKANOS_FP_Init(UINT8 BssNum, UINT8 *pApMac);
INT32 IKANOS_DataFramesTx(struct sk_buff *pSkb, struct net_device *pNetDev);
void IKANOS_DataFrameRx(PRTMP_ADAPTER pAd, struct sk_buff *pSkb);
#endif /* MODULE_IKANOS */


#endif /* __VR_IKANS_H__ */

/* End of vr_ikans.h */
