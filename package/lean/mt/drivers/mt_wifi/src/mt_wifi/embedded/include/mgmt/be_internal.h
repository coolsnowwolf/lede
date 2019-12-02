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

#ifndef __BE_INTERNAL_H__
#define __BE_INTERNAL_H__

#include "mgmt/be_export.h"

/*
* private structure definition to prevent direct access
*/
struct wlan_config {
	struct phy_cfg phy_conf;
	struct ht_cfg ht_conf;
	struct vht_cfg vht_conf;
};


struct wlan_operate {
	struct phy_op phy_oper;
	struct ht_op ht_oper;
	struct ht_op_status ht_status;
	struct vht_op vht_oper;
	struct vht_op_status vht_status;
	UCHAR state;
};

/*
* Utility
*/
VOID phy_freq_get_cfg(struct wifi_dev *wdev, struct freq_cfg *cfg);


/*
* internal export configure loader
*/
VOID phy_oper_init(struct wifi_dev *wdev, struct phy_op *obj);
VOID phy_oper_exit(struct phy_op *obj);

/*
*ht related
*/
VOID ht_oper_init(struct wifi_dev *wdev, struct ht_op *obj);
VOID ht_oper_exit(struct ht_op *obj);


/*
*vht info related
*/
VOID vht_oper_init(struct wifi_dev *wdev, struct vht_op *obj);
VOID vht_oper_exit(struct vht_op *obj);

/*
*vht phy op related
*/
VOID vht_op_status_init(struct wifi_dev *wdev, struct vht_op_status *obj);
VOID vht_op_status_exit(struct vht_op_status *obj);

/*
* ht operate related
*/
VOID ht_op_status_init(struct wifi_dev *wdev, struct ht_op_status *obj);
VOID ht_op_status_exit(struct ht_op_status *obj);

/*
* be_phy module
*/
VOID operate_loader_prim_ch(struct wlan_operate *op);
VOID operate_loader_phy(struct wifi_dev *wdev, struct freq_cfg *fcfg);

/*
* be_ht module
*/
VOID operate_loader_ht_bw(struct wlan_operate *op);
VOID operate_loader_ext_cha(struct wlan_operate *op);
VOID operate_loader_frag_thld(struct wlan_operate *op, UINT32 frag_thld);
VOID operate_loader_rts_len_thld(struct wlan_operate *op, UINT32 len_thld);
VOID operate_loader_rts_pkt_thld(struct wlan_operate *op, UCHAR pkt_num);

/*
* be_vht module
*/
VOID operate_loader_vht_bw(struct wlan_operate *op);



#endif /*__BE_INTERNAL_H__*/
