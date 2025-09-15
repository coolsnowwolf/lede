/***************************************************************************
* MediaTek Inc.
* 4F, No. 2 Technology 5th Rd.
* Science-based Industrial Park
* Hsin-chu, Taiwan, R.O.C.
*
* (c) Copyright 1997-2012, MediaTek, Inc.
*
* All rights reserved. MediaTek source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of MediaTek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of MediaTek Technology, Inc. is obtained.
***************************************************************************

*/

#ifndef __CONFIG_INTERNAL_H__
#define __CONFIG_INTERNAL_H__

#include "mgmt/be_internal.h"

/*
* define struct constructor & deconstructor
*/

VOID phy_cfg_init(struct phy_cfg *obj);
VOID phy_cfg_exit(struct phy_cfg *obj);

/*
*ht phy info related
*/
VOID ht_cfg_init(struct ht_cfg *obj);
VOID ht_cfg_exit(struct ht_cfg *obj);

/*
*vht info related
*/
#ifdef DOT11_VHT_AC
VOID vht_cfg_init(struct vht_cfg *obj);
VOID vht_cfg_exit(struct vht_cfg *obj);
#endif /* #ifdef DOT11_VHT_AC */

/*
* internal export configure loader
*/

#endif
