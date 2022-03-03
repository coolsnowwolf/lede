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
	rt_config.h

	Abstract:
	Central header file to maintain all include files for all NDIS
	miniport driver routines.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Paul Lin    08-01-2002    created

*/
#ifndef	__RT_CONFIG_H__
#define	__RT_CONFIG_H__


#include "rtmp_comm.h"

#include "rtmp_def.h"
#include "rtmp_chip.h"
#include "rtmp_timer.h"

#ifdef WIFI_DIAG
#include "diag.h"
#endif

#ifdef WH_EZ_SETUP
#include "easy_setup/ez_hooks.h"
#endif


#ifdef AGS_SUPPORT
#include "ags.h"
#endif /* AGS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BAND_STEERING
#include "band_steering_def.h"
#endif /* BAND_STEERING */
#endif
#ifdef CONFIG_FPGA_MODE
#include "fpga/fpga_ctl.h"
#endif

#include "mlme.h"
#include "crypt_md5.h"
#include "crypt_sha2.h"
#include "crypt_hmac.h"
#include "crypt_aes.h"
#include "crypt_arc4.h"
/*#include "rtmp_cmd.h" */
#ifdef ROUTING_TAB_SUPPORT
#include "routing_tab.h"
#endif /* ROUTING_TAB_SUPPORT */
#include "rtmp.h"
#include "wpa.h"
#include "chlist.h"
#include "spectrum.h"
#ifdef CONFIG_AP_SUPPORT
#include "ap.h"
#include "ap_autoChSel.h"
#endif /* CONFIG_AP_SUPPORT */
#include "rt_os_util.h"

#include "eeprom.h"
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
#include "mcu/mcu.h"
#endif

#ifdef RTMP_EFUSE_SUPPORT
#include "efuse.h"
#endif /* RTMP_EFUSE_SUPPORT */

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#include "rt_os_net.h"


#ifdef UAPSD_SUPPORT
#include "uapsd.h"
#endif /* UAPSD_SUPPORT */

#include "tx_power.h"

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#include "ap_mbss.h"
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
#include "ap_wds.h"
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
#include "ap_apcli.h"
#endif /* APCLI_SUPPORT */

#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */

#include "ap_ids.h"
#include "ap_cfg.h"

#ifdef CLIENT_WDS
#include "client_wds.h"
#endif /* CLIENT_WDS */
#endif /* CONFIG_AP_SUPPORT */
#ifdef MWDS
#include "mwds.h"
#endif /* CONFIG_AP_SUPPORT */

#ifdef WH_EVENT_NOTIFIER
#include "event_notifier.h"
#endif /* WH_EVENT_NOTIFIER */

#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */



#ifdef BLOCK_NET_IF
#include "netif_block.h"
#endif /* BLOCK_NET_IF */

#ifdef IGMP_SNOOP_SUPPORT
#include "igmp_snoop.h"
#endif /* IGMP_SNOOP_SUPPORT */


#ifdef CONFIG_ATE
#include "ate_agent.h"
#include "ate.h"
#endif


#ifdef CONFIG_QA
#include "qa_agent.h"
#endif /* CONFIG_QA */

#ifdef CONFIG_QA
#ifndef CONFIG_ATE
#error "For supporting QA GUI, please set HAS_ATE=y and HAS_QA_SUPPORT=y."
#endif /* CONFIG_ATE */
#endif /* CONFIG_QA */



#ifdef WAPI_SUPPORT
#include "wapi.h"
#endif /* WAPI_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#include "rrm.h"
#endif /* DOT11K_RRM_SUPPORT */


#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
#define RALINK_PASSPHRASE	"Ralink"
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
#ifndef APCLI_SUPPORT
#error "Build Apcli for being controlled by NetworkManager or wext, please set HAS_APCLI_SUPPORT=y and HAS_APCLI_WPA_SUPPLICANT=y"
#endif /* APCLI_SUPPORT */
#define WPA_SUPPLICANT_SUPPORT
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */



#ifdef WSC_INCLUDED
#include "crypt_biginteger.h"
#include "crypt_dh.h"
#include "wsc_tlv.h"

#ifdef WSC_NFC_SUPPORT
#include "nfc.h"
#endif /* WSC_NFC_SUPPORT */
#endif /* WSC_INCLUDED */


#ifdef IKANOS_VX_1X0
#include "vr_ikans.h"
#endif /* IKANOS_VX_1X0 */

#ifdef DOT11R_FT_SUPPORT
#include	"ft.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#include "rrm.h"
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
#include "pmf.h"
#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
#include "wnm.h"
#endif /* DOT11V_WNM_SUPPORT */

#ifdef WAPI_SUPPORT
#include "wapi.h"
#endif /* WAPI_SUPPORT */



#include "vendor.h"
#ifdef DOT11_VHT_AC
#include "vht.h"
#endif /* DOT11_VHT_AC */




#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif /* WORKQUEUE_BH / */


#ifdef BAND_STEERING
#include "band_steering.h"
#endif /* BAND_STEERING */



#ifdef RLT_MAC
#include "mac/mac_ral/fce.h"
#endif /* RLT_MAC */

#ifdef MT_MAC
#include "mt_io.h"
#include "mt_ps.h"
#endif

#ifdef CONFIG_SNIFFER_SUPPORT
#include "sniffer/sniffer.h"
#endif /* CONFIG_SNIFFER_SUPPORT */

#ifdef ACL_V2_SUPPORT
#include "ap_acl_v2.h"
#endif /* ACL_V2_SUPPORT */

#ifdef SNIFFER_MIB_CMD
#include "sniffer_mib.h"
#endif /* SNIFFER_MIB_CMD */

#ifdef ALL_NET_EVENT
#include "all_net_event.h"
#endif /* ALL_NET_EVENT */

#ifdef LINUX
#endif /* LINUX */

#ifdef DOT11U_INTERWORKING
#include "dot11u_interworking.h"
#include "gas.h"
#endif

#ifdef CONFIG_DOT11V_WNM
#include "dot11v_wnm.h"
#include "wnm.h"
#endif

#ifdef CONFIG_HOTSPOT
#include "hotspot.h"
#endif

#ifdef CONFIG_PUSH_SUPPORT
#include "event_notifier.h"
#endif
#ifdef WH_EZ_SETUP
//#include "easy_setup/ez_hooks.h"
#include "easy_setup/mt7603_driver_specific_func.h"
#include "easy_setup/ez_hooks_proto.h"
#include "easy_setup/mt7603_chip_ops_api.h"
//#include "easy_setup/mt7603_chip_ops.h"
#endif /* WH_EZ_SETUP */
#ifdef DOT11_SAE_SUPPORT
#include "sae_cmm.h"
#include "sae.h"
#endif /* DOT11_SAE_SUPPORT */

#ifdef CONFIG_OWE_SUPPORT
#include "owe_cmm.h"
#endif /* DOT11_OWE_SUPPORT */

#ifdef MBO_SUPPORT
#include "mbo.h"
#endif/* MBO_SUPPORT */


#ifdef WAPP_SUPPORT
#include "wnm.h"
#include "wapp/wapp_cmm_type.h"
#include "wapp/wapp.h"
#endif/* WAPP_SUPPORT */

#ifdef A4_CONN
#include "a4_conn.h"
#endif

#ifdef DOT11_SAE_SUPPORT
extern  UCHAR OUI_WPA2_AKM_SAE_SHA256[4];
extern  UCHAR OUI_WPA2_PSK_SHA256[4];
extern  UCHAR OUI_WPA2_PSK_AKM[4];
#endif

#ifdef CONFIG_OWE_SUPPORT
extern  UCHAR OUI_WPA2_AKM_OWE[4];
#endif

#endif	/* __RT_CONFIG_H__ */

