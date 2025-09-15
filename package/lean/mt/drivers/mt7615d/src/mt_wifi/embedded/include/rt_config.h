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


#ifdef AGS_SUPPORT
#include "ags.h"
#endif /* AGS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BAND_STEERING
#include "band_steering_def.h"
#endif /* BAND_STEERING */

#endif /* CONFIG_AP_SUPPORT */



#ifdef MT_MAC
#ifdef CUT_THROUGH
#include "cut_through.h"
#endif /* CUT_THROUGH */
#endif /* MT_MAC */

#include "mlme.h"
/*#include "rtmp_cmd.h" */

/* #ifdef VOW_SUPPORT */
/* VOW support */
#include "ap_vow.h"
/* #endif */ /* VOW_SUPPORT */

#include "qm.h"
#include "rtmp.h"
#include "security/sec.h"
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
#include "sta.h"
#endif /* APCLI_SUPPORT */

#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */

#include "ap_ids.h"
#include "ap_cfg.h"

#ifdef CLIENT_WDS
#include "client_wds.h"
#endif /* CLIENT_WDS */

#ifdef ROUTING_TAB_SUPPORT
#include "routing_tab.h"
#endif /* ROUTING_TAB_SUPPORT */

#ifdef A4_CONN
#include "a4_conn.h"
#endif



#endif /* CONFIG_AP_SUPPORT */

#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */



#ifdef BLOCK_NET_IF
#include "netif_block.h"
#endif /* BLOCK_NET_IF */

#ifdef IGMP_SNOOP_SUPPORT
#include "igmp_snoop.h"
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
#include "map.h"
#endif
#ifdef WAPP_SUPPORT
#include "wapp/wapp_cmm_type.h"
#endif /* WAPP_SUPPORT */
#ifdef CONFIG_ATE
#include "ate_agent.h"
#include "ate.h"
#include "mt_testmode.h"
#include "testmode_common.h"
#include <linux/utsname.h>
#endif /* CONFIG_ATE */

#ifdef WCX_SUPPORT
#include <mach/mt_boot.h>
#include "meta_agent.h"
#endif /* WCX_SUPPORT */

#ifdef CONFIG_QA
/* #include "qa_agent.h" */
#include "testmode_ioctl.h"
#include "LoopBack.h"
#endif /* CONFIG_QA */

#ifdef CONFIG_QA
#ifndef CONFIG_ATE
#error "For supporting QA GUI, please set HAS_ATE=y and HAS_QA_SUPPORT=y."
#endif /* CONFIG_ATE */
#endif /* CONFIG_QA */


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
#include "ap_repeater.h"
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef DOT11K_RRM_SUPPORT
#include "rrm.h"
#endif /* DOT11K_RRM_SUPPORT */

#ifdef MBO_SUPPORT
#include "mbo.h"
#endif /* MBO_SUPPORT */


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
#include "security/crypt_biginteger.h"
#include "security/crypt_dh.h"
#include "wsc_tlv.h"

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
#include "security/pmf.h"
#endif /* DOT11W_PMF_SUPPORT */


#ifdef FTM_SUPPORT
#include "ftm.h"
#endif /* FTM_SUPPORT */




#ifdef DOT11_VHT_AC
#include "vht.h"
#endif /* DOT11_VHT_AC */

#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
#include "video.h"
#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif /* WORKQUEUE_BH / */

#ifdef MT_MAC
#ifdef CFG_SUPPORT_MU_MIMO
/* TODO: NeedModify-Jeffrey, shall we integrate the data structure definition in ap_mumimo.h and sta_mumimo.c? */
#ifdef CONFIG_AP_SUPPORT
#include "ap_mumimo.h"
#endif /* CONFIG_AP_SUPPORT */
#endif

#ifdef CFG_SUPPORT_MU_MIMO_RA
#ifdef CONFIG_AP_SUPPORT
#include "ap_mura.h"
#endif /* CONFIG_AP_SUPPORT */
#endif

#endif /* MT_MAC */

#ifdef BAND_STEERING
#include "band_steering.h"
#endif /* BAND_STEERING */

#ifdef WAPP_SUPPORT
#include "wapp/wapp.h"
#endif /* WAPP_SUPPORT */

#ifdef TXBF_SUPPORT
#ifdef MT_MAC
#include "txbf/mt_txbf.h"
#else
#include "rt_txbf.h"
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
#include "icap.h"
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef SMART_ANTENNA
#include "smartant.h"
#endif /* SMART_ANTENNA */

#ifdef MT_MAC
#include "mt_io.h"
#include "protocol/tmr.h"
#endif

#ifdef SNIFFER_SUPPORT
#include "sniffer/sniffer.h"
#endif /* SNIFFER_SUPPORT */

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
#include "cfg80211extr.h"
#include "cfg80211_cmm.h"
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef CONFIG_DOT11U_INTERWORKING
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

#ifdef LINUX
#ifdef CONFIG_TRACE_SUPPORT
#include "os/trace.h"
#endif
#endif

#include "fp_qm.h"
#include "tm.h"
#include "hw_ctrl.h"
#include "hdev_ctrl.h"

#include "protocol/protection.h"

#ifdef SINGLE_SKU_V2
#include "txpwr/single_sku.h"
#endif

#include "bcn.h"

#ifdef BACKGROUND_SCAN_SUPPORT
#include "bgnd_scan.h"
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "scs.h"
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
#include "cmm_tcprack.h"
#endif

#include "wlan_config/config_export.h"
#include "mgmt/be_export.h"

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
#include <linux/foe_hook.h>
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#define MCAST_WCID_TO_REMOVE 0 /* Pat: */

#if defined(RED_SUPPORT)
#include "ra_ac_q_mgmt.h"
#endif /* RED_SUPPORT */
#include "fq_qm.h"
#include "cmm_rvr_dbg.h"
#ifdef  CONFIG_STEERING_API_SUPPORT
#include <linux/rtc.h>
#endif
#ifdef WIFI_DIAG
#include "os/diag.h"
#endif

#include "misc_app.h"

#endif	/* __RT_CONFIG_H__ */
