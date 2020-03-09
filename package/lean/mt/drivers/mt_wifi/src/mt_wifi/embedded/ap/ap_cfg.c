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
	ap_cfg.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/


#include "rt_config.h"

#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef WSC_LED_SUPPORT
#include "rt_led.h"
#endif /* WSC_LED_SUPPORT */
#endif
#ifdef RLM_CAL_CACHE_SUPPORT
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */

#define A_BAND_REGION_0				0
#define A_BAND_REGION_1				1
#define A_BAND_REGION_2				2
#define A_BAND_REGION_3				3
#define A_BAND_REGION_4				4
#define A_BAND_REGION_5				5
#define A_BAND_REGION_6				6
#define A_BAND_REGION_7				7
#define A_BAND_REGION_8				8
#define A_BAND_REGION_9				9
#define A_BAND_REGION_10			10

#define G_BAND_REGION_0				0
#define G_BAND_REGION_1				1
#define G_BAND_REGION_2				2
#define G_BAND_REGION_3				3
#define G_BAND_REGION_4				4
#define G_BAND_REGION_5				5
#define G_BAND_REGION_6				6


COUNTRY_CODE_TO_COUNTRY_REGION allCountry[] = {
	/* {Country Number, ISO Name, Country Name, Support 11A, 11A Country Region, Support 11G, 11G Country Region} */
	{0,		"DB",	"Debug",				TRUE,	A_BAND_REGION_7,	TRUE,	G_BAND_REGION_5},
	{8,		"AL",	"ALBANIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{12,	"DZ",	"ALGERIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{32,	"AR",	"ARGENTINA",			TRUE,	A_BAND_REGION_3,	TRUE,	G_BAND_REGION_1},
	{51,	"AM",	"ARMENIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{36,	"AU",	"AUSTRALIA",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{40,	"AT",	"AUSTRIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{31,	"AZ",	"AZERBAIJAN",			TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{48,	"BH",	"BAHRAIN",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{112,	"BY",	"BELARUS",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{56,	"BE",	"BELGIUM",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{84,	"BZ",	"BELIZE",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{68,	"BO",	"BOLIVIA",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{76,	"BR",	"BRAZIL",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{96,	"BN",	"BRUNEI DARUSSALAM",	TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{100,	"BG",	"BULGARIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{124,	"CA",	"CANADA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{152,	"CL",	"CHILE",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{156,	"CN",	"CHINA",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{170,	"CO",	"COLOMBIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{188,	"CR",	"COSTA RICA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{191,	"HR",	"CROATIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{196,	"CY",	"CYPRUS",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{203,	"CZ",	"CZECH REPUBLIC",		TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{208,	"DK",	"DENMARK",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{214,	"DO",	"DOMINICAN REPUBLIC",	TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{218,	"EC",	"ECUADOR",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{818,	"EG",	"EGYPT",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{222,	"SV",	"EL SALVADOR",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{233,	"EE",	"ESTONIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{246,	"FI",	"FINLAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{250,	"FR",	"FRANCE",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{268,	"GE",	"GEORGIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{276,	"DE",	"GERMANY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{300,	"GR",	"GREECE",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{320,	"GT",	"GUATEMALA",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{340,	"HN",	"HONDURAS",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{344,	"HK",	"HONG KONG",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{348,	"HU",	"HUNGARY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{352,	"IS",	"ICELAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{356,	"IN",	"INDIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{360,	"ID",	"INDONESIA",			TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{364,	"IR",	"IRAN",					TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{372,	"IE",	"IRELAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{376,	"IL",	"ISRAEL",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{380,	"IT",	"ITALY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{392,	"JP",	"JAPAN",				TRUE,	A_BAND_REGION_9,	TRUE,	G_BAND_REGION_1},
	{400,	"JO",	"JORDAN",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{398,	"KZ",	"KAZAKHSTAN",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{408,	"KP",	"KOREA DEMOCRATIC PEOPLE'S REPUBLIC OF", TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{410,	"KR",	"KOREA REPUBLIC OF",	TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{414,	"KW",	"KUWAIT",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{428,	"LV",	"LATVIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{422,	"LB",	"LEBANON",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{438,	"LI",	"LIECHTENSTEIN",		TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{440,	"LT",	"LITHUANIA",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{442,	"LU",	"LUXEMBOURG",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{446,	"MO",	"MACAU",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{807,	"MK",	"MACEDONIA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{458,	"MY",	"MALAYSIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{484,	"MX",	"MEXICO",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{492,	"MC",	"MONACO",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{504,	"MA",	"MOROCCO",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{528,	"NL",	"NETHERLANDS",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{554,	"NZ",	"NEW ZEALAND",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{578,	"NO",	"NORWAY",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{512,	"OM",	"OMAN",					TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{586,	"PK",	"PAKISTAN",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{591,	"PA",	"PANAMA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{604,	"PE",	"PERU",					TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{608,	"PH",	"PHILIPPINES",			TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{616,	"PL",	"POLAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{620,	"PT",	"PORTUGAL",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{630,	"PR",	"PUERTO RICO",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{634,	"QA",	"QATAR",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{642,	"RO",	"ROMANIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{643,	"RU",	"RUSSIA FEDERATION",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{682,	"SA",	"SAUDI ARABIA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{702,	"SG",	"SINGAPORE",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{703,	"SK",	"SLOVAKIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{705,	"SI",	"SLOVENIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{710,	"ZA",	"SOUTH AFRICA",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{724,	"ES",	"SPAIN",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{752,	"SE",	"SWEDEN",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{756,	"CH",	"SWITZERLAND",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{760,	"SY",	"SYRIAN ARAB REPUBLIC",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{158,	"TW",	"TAIWAN",				TRUE,	A_BAND_REGION_3,	TRUE,	G_BAND_REGION_0},
	{764,	"TH",	"THAILAND",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{780,	"TT",	"TRINIDAD AND TOBAGO",	TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{788,	"TN",	"TUNISIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{792,	"TR",	"TURKEY",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{804,	"UA",	"UKRAINE",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{784,	"AE",	"UNITED ARAB EMIRATES",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{826,	"GB",	"UNITED KINGDOM",		TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{840,	"US",	"UNITED STATES",		TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{858,	"UY",	"URUGUAY",				TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{860,	"UZ",	"UZBEKISTAN",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_0},
	{862,	"VE",	"VENEZUELA",			TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{704,	"VN",	"VIET NAM",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{887,	"YE",	"YEMEN",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{716,	"ZW",	"ZIMBABWE",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{999,	"",	"",	0,	0,	0,	0}
};

#define NUM_OF_COUNTRIES	(sizeof(allCountry)/sizeof(COUNTRY_CODE_TO_COUNTRY_REGION))

static const struct apcfg_parameters apcfg_for_peak = {
	.cfg_mode[0] = 9, /*WirelessMode*/
	.cfg_mode[1] = 14,
	.tx_power_percentage = 100, /*TxPower*/
	.tx_preamble = 1, /*TxPreamble*/
	.conf_len_thld = 2347, /*RTSThreshold*/
	.oper_len_thld = 2347,
	.conf_frag_thld = 2346, /*FragThreshold*/
	.oper_frag_thld = 2346,
	.bEnableTxBurst = 1, /*TxBurst*/
	.bUseShortSlotTime = 1, /*ShortSlot*/
#ifdef DOT11_N_SUPPORT
	.conf_ht_bw = 1, /*HT_BW*/
	.oper_ht_bw = 1,
#ifdef DOT11N_DRAFT3
	.bBssCoexEnable = 0, /*HT_BSSCoexistence*/
#endif

	.ht_tx_streams = 4, /*HT_TxStream*/
	.ht_rx_streams = 4, /*HT_RxStream*/

	.bBADecline = 0, /*HT_BADecline*/
	.AutoBA = 1, /*HT_AutoBA*/
	.AmsduEnable = 1, /*HT_AMSDU*/
	.RxBAWinLimit = 64, /*HT_BAWinSize*/
	.ht_gi = 1, /*HT_GI*/
	.ht_stbc = 1, /*HT_STBC*/
	.ht_ldpc = 1, /*HT_LDPC*/
	.bRdg = 0, /*HT_RDG*/
#endif
	.HT_DisallowTKIP = 1, /*HT_DisallowTKIP*/

#ifdef DOT11_VHT_AC
	.conf_vht_bw = 1, /*VHT_BW, 5G only*/
	.oper_vht_bw = 1,
	.vht_sgi = 1, /*VHT_SGI, 5G only*/
	.vht_stbc = 1, /*VHT_STBC, 5G only*/
	.vht_bw_signal = 0, /*VHT_BW_SIGNAL, 5G only*/
	.vht_ldpc = 1, /*VHT_LDPC, 5G only*/
	.g_band_256_qam = 1, /*G_BAND_256QAM, 2.4G only*/
#endif

	.bIEEE80211H = 1, /*IEEE80211H*/

#ifdef MT_DFS_SUPPORT
	.bDfsEnable = 0, /*DfsEnable, 5G only*/
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	.DfsZeroWaitSupport = 0, /*DfsZeroWait, Single band only*/
#endif

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	.ETxBfEnCond = 0, /*ETxBfEnCond*/
#endif
#endif

	.ITxBfEn = 0, /*ITxBfEn*/

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	.MUTxRxEnable = 0, /*MUTxRxEnable*/
#endif
#endif
	};

#ifdef CFG_SUPPORT_MU_MIMO

/* iwprive test code */

INT32 hqa_mu_get_qd(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_get_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT32 hqa_mu_cal_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT32 hqa_mu_set_snr_offset(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_zero_nss(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_speedup_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_mu_table(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_group(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_gid_up(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_su_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_su_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_gid_up(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_trigger_mu_tx(PRTMP_ADAPTER pAd, RTMP_STRING *arg);


/* get function */
INT ShowMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCalcInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuProfileTxStsCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* set function */
INT SetMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCalculateInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerGIDMgmtFrameProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerMuTxProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerDegroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerGroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerBbpProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef AIR_MONITOR
INT Set_Enable_Air_Monitor_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorRule_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorIndex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorShowAll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorClearCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget0_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget5_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget6_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget7_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget8_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget9_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget10_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget11_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget12_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget13_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget14_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget15_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID Air_Monitor_Pkt_Report_Action(RTMP_ADAPTER *pAd, UCHAR wcid, RX_BLK *pRxBlk);
#endif /* AIR_MONITOR */

#ifdef DSCP_PRI_SUPPORT
INT	Set_Dscp_Pri_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT8	map_index,
	IN	RTMP_STRING * arg);

INT	Set_Dscp_Pri_2G_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);

INT	Set_Dscp_Pri_5G_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);
#endif

#ifdef CFG_SUPPORT_MU_MIMO_RA
/* mura set function */
INT SetMuraPeriodicSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraTestAlgorithmProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraTestAlgorithmInit(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedGroupRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedSndParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraDisableCN3CN4Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* mura get function */
INT GetMuraMonitorStateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT GetMuraPFIDStatProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_CountryString_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_CountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ChGeography_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef SPECIFIC_TX_POWER_SUPPORT
INT Set_AP_PKT_PWR(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* SPECIFIC_TX_POWER_SUPPORT */

INT Set_AP_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_TxRate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_OLBCDetection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_AP_PerMbssMaxStaNum_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_AP_IdleTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef IAPP_SUPPORT
INT	Set_IappPID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* IAPP_SUPPORT */

INT Set_AP_WpaMixPairCipher_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_RekeyInterval_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_RekeyMethod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_PMKCachePeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_ASSOC_REQ_RSSI_THRESHOLD(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_AP_KickStaRssiLow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BasicRate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef REDUCE_TCP_ACK_SUPPORT
INT Set_ReduceAckEnable_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam);

INT Show_ReduceAckInfo_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam);

INT Set_ReduceAckProb_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam);
#endif

INT Set_BeaconPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DtimPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_NoForwarding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NoForwardingBTNSSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_WmmCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_HideSSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_VLANID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_VLANPriority_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VLAN_TAG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AccessPolicy_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLAddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLDelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLShowAll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLClearAll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AutoChannelSel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_PartialScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef AP_SCAN_SUPPORT
INT Set_AutoChannelSelCheckTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* AP_SCAN_SUPPORT */

INT Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_StaCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Sat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
static INT show_apcfg_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_RAInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef TXBF_SUPPORT
INT Show_TxBfInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TXBF_SUPPORT */
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
INT Set_DiagOpt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_diag_cond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Diag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DBG_DAIGNOSE */
#endif /* RTMP_MAC_PCI */


INT show_timer_list(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_wtbl_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_Sat_Reset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_MATTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DOT1X_SUPPORT
INT Set_IEEE8021X_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_PreAuth_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_RADIUS_Server_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RADIUS_Port_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RADIUS_Key_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DeletePMKID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DumpPMKID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RADIUS_MAC_ACL_SUPPORT
INT Set_RADIUS_MacAuth_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RADIUS_CacheTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_RADIUS_acl_cache(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

INT Set_DisConnectSta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef VENDOR_FEATURE7_SUPPORT
INT Set_DisConnectBssSta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_DisConnectAllSta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


#ifdef APCLI_SUPPORT
INT Set_ApCli_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_Bssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_TxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_TxMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_WirelessMode_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
INT Set_ApCli_AutoConnect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
INT Set_ApCli_Trial_Ch_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* APCLI_CONNECTION_TRIAL */


#ifdef MAC_REPEATER_SUPPORT
INT Set_ReptMode_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Cli_Link_Map_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WSC_AP_SUPPORT
INT Set_AP_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef APCLI_SUPPORT
INT Set_ApCli_WscScanMode_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_CERT_SUPPORT
INT Set_ApCli_Cert_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
/* Add for APCLI PMF 5.3.3.3 option test item. (Only Tx De-auth Req. and make sure the pkt can be Encrypted) */
INT ApCliTxDeAuth(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* APCLI_CERT_SUPPORT */

#ifdef WSC_AP_SUPPORT
INT Set_Wps_Cert_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef ROAMING_ENHANCE_SUPPORT
INT Set_RoamingEnhance_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef UAPSD_SUPPORT
INT Set_UAPSD_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* UAPSD_SUPPORT */

#ifdef WSC_AP_SUPPORT
INT Set_WscStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef BB_SOC
INT Set_WscOOB_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_WscStop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscPINCode(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscStatus(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscRegsDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

BOOLEAN WscCheckEnrolleeNonceFromUpnp(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_STRING *pData,
	IN  USHORT			Length,
	IN  PWSC_CTRL       pWscControl);

UCHAR	WscRxMsgTypeFromUpnp(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  RTMP_STRING *pData,
	IN	USHORT				Length);

INT	    WscGetConfForUpnp(
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL       pWscControl);

#ifdef CON_WPS
INT     Set_ConWpsApCliMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT     Set_ConWpsApcliAutoPreferIface_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT     Set_ConWpsApCliDisabled_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT     Set_ConWpsApDisabled_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CON_WPS */
INT	Set_AP_WscConfMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_AP_WscConfStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_AP_WscPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_WscSecurityMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_WscMultiByteCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


INT	Set_WscVersion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef VENDOR_FEATURE6_SUPPORT
INT	Set_WscUUID_STR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscUUID_HEX_E_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* VENDOR_FEATURE6_SUPPORT */

#ifdef WSC_V2_SUPPORT
INT	Set_WscV2Support_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscVersion2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscExtraTlvTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscExtraTlvType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscExtraTlvData_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscSetupLock_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscFragment_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscFragmentSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscMaxPinAttack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscSetupLockTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WSC_V2_SUPPORT */
INT	Set_WscAutoTriggerDisable_Proc(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_STRING	 *arg);
#endif /* WSC_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
INT Set_McastPhyMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_McastMcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_McastRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MCAST_RATE_SPECIFIC */

#ifdef DOT11N_DRAFT3
INT Set_OBSSScanParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AP2040ReScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11N_DRAFT3 */

INT Set_EntryLifeCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef AP_QLOAD_SUPPORT
INT Set_QloadClr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* QLOAD ALARM */
INT Set_QloadAlarmTimeThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_QloadAlarmNumThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT Set_MemDebug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CONFIG_AP_SUPPORT
INT Set_PowerSaveLifeTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_AP_SUPPORT */


#ifdef DYNAMIC_VGA_SUPPORT
INT	Set_DyncVgaEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_false_cca_hi_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_false_cca_low_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef MT_MAC
INT	Set_AP_TimEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT setApTmrEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_TmrCalResult_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CUT_THROUGH
INT Set_CutThrough_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CUT_THROUGH */

#ifdef DBG
INT    set_tim_update_proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg);

INT    Set_AP_DumpTime_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg);

INT    Set_BcnStateCtrl_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg);
#endif /*DBG*/

#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT Set_KtoFlash_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_RDCE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef TX_AGG_ADJUST_WKR
INT Set_AggAdjWkr_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* TX_AGG_ADJUST_WKR */

#ifdef RLM_CAL_CACHE_SUPPORT
INT Set_RLM_Cal_Cache_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* RLM_CAL_CACHE_SUPPORT */

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT Set_PBC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

#endif /*PKT_BUDGET_CTRL_SUPPORT*/

INT Set_BWF_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

#ifdef CONFIG_HOTSPOT_R2
INT Set_CR4_Hotspot_Flag(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef HTC_DECRYPT_IOT
INT Set_HTC_Err_TH_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
INT Set_Entry_HTC_Err_Cnt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
INT Set_WTBL_AAD_OM_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* HTC_DECRYPT_IOT */

#ifdef DHCP_UC_SUPPORT
INT Set_DHCP_UC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* DHCP_UC_SUPPORT */
#endif /* MT_MAC */

#ifdef CONFIG_TX_DELAY
INT Set_TX_Batch_Cnt_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);

INT Set_Pkt_Min_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);

INT Set_Pkt_Max_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);

INT Set_TX_Delay_Timeout_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif

#ifdef DBG_STARVATION
static INT show_starv_info_proc(
	struct _RTMP_ADAPTER *ad,
	char *arg)
{
	starv_log_dump(&ad->starv_log_ctrl);
	return TRUE;
}
#endif /*DBG_STARVATION*/

static struct {
	RTMP_STRING *name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, RTMP_STRING *arg);
} *PRTMP_PRIVATE_SET_PROC, RTMP_PRIVATE_SUPPORT_PROC[] = {
	{"RateAlg",						Set_RateAlg_Proc},
#ifdef NEW_RATE_ADAPT_SUPPORT
	{"PerThrdAdj",					Set_PerThrdAdj_Proc},
	{"LowTrafficThrd",				Set_LowTrafficThrd_Proc},
	{"TrainUpRule",					Set_TrainUpRule_Proc},
	{"TrainUpRuleRSSI",				Set_TrainUpRuleRSSI_Proc},
	{"TrainUpLowThrd",				Set_TrainUpLowThrd_Proc},
	{"TrainUpHighThrd",				Set_TrainUpHighThrd_Proc},
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",			Set_CountryRegionABand_Proc},
	{"CountryString",				Set_CountryString_Proc},
	{"CountryCode",				Set_CountryCode_Proc},
#ifdef EXT_BUILD_CHANNEL_LIST
	{"ChGeography",				Set_ChGeography_Proc},
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef AIR_MONITOR
	{"mnt_en",				Set_Enable_Air_Monitor_Proc},
	{"mnt_rule",				Set_MonitorRule_Proc},
	{"mnt_sta",				Set_MonitorTarget_Proc},
	{"mnt_idx",				Set_MonitorIndex_Proc},
	{"mnt_show",				Set_MonitorShowAll_Proc},
	{"mnt_clr",				Set_MonitorClearCounter_Proc},
	{"mnt_sta0",				Set_MonitorTarget0_Proc},
	{"mnt_sta1",				Set_MonitorTarget1_Proc},
	{"mnt_sta2",				Set_MonitorTarget2_Proc},
	{"mnt_sta3",				Set_MonitorTarget3_Proc},
	{"mnt_sta4",				Set_MonitorTarget4_Proc},
	{"mnt_sta5",				Set_MonitorTarget5_Proc},
	{"mnt_sta6",				Set_MonitorTarget6_Proc},
	{"mnt_sta7",				Set_MonitorTarget7_Proc},
	{"mnt_sta8",				Set_MonitorTarget8_Proc},
	{"mnt_sta9",				Set_MonitorTarget9_Proc},
	{"mnt_sta10",				Set_MonitorTarget10_Proc},
	{"mnt_sta11",				Set_MonitorTarget11_Proc},
	{"mnt_sta12",				Set_MonitorTarget12_Proc},
	{"mnt_sta13",				Set_MonitorTarget13_Proc},
	{"mnt_sta14",				Set_MonitorTarget14_Proc},
	{"mnt_sta15",				Set_MonitorTarget15_Proc},
#endif /* AIR_MONITOR */
	{"SSID",						Set_AP_SSID_Proc},
	{"WirelessMode",				Set_WirelessMode_Proc},
	{"BasicRate",					Set_BasicRate_Proc},
	{"ShortSlot",					Set_ShortSlot_Proc},
	{"Channel",					Set_Channel_Proc},
#ifdef REDUCE_TCP_ACK_SUPPORT
	{"ReduceAckEnable",             Set_ReduceAckEnable_Proc},
	{"ReduceAckProb",               Set_ReduceAckProb_Proc},
#endif
	{"BeaconPeriod",				Set_BeaconPeriod_Proc},
	{"DtimPeriod",					Set_DtimPeriod_Proc},
	{"TxPower",					Set_TxPower_Proc},
	{"BGProtection",				Set_BGProtection_Proc},
	{"DisableOLBC",				Set_OLBCDetection_Proc},
	{"TxPreamble",				Set_TxPreamble_Proc},
	{"RTSThreshold",				Set_RTSThreshold_Proc},
	{"FragThreshold",				Set_FragThreshold_Proc},
	{"TxBurst",					Set_TxBurst_Proc},
	{"MbssMaxStaNum",					Set_AP_PerMbssMaxStaNum_Proc},
#ifdef RTMP_MAC_PCI
	{"ShowRF",					Set_ShowRF_Proc},
#endif /* RTMP_MAC_PCI */
	{"IdleTimeout",					Set_AP_IdleTimeout_Proc},
#ifdef DOT11_N_SUPPORT
	{"BASetup",					Set_BASetup_Proc},
	{"BADecline",					Set_BADecline_Proc},
	{"SendMIMOPS",				Set_SendSMPSAction_Proc},
	{"BAOriTearDown",				Set_BAOriTearDown_Proc},
	{"BARecTearDown",				Set_BARecTearDown_Proc},
	{"HtBw",						Set_HtBw_Proc},
	{"HtMcs",						Set_HtMcs_Proc},
	{"HtGi",						Set_HtGi_Proc},
	{"HtOpMode",					Set_HtOpMode_Proc},
	{"HtStbc",					Set_HtStbc_Proc},
	{"HtExtcha",					Set_HtExtcha_Proc},
	{"HtMpduDensity",				Set_HtMpduDensity_Proc},
	{"HtBaWinSize",				Set_HtBaWinSize_Proc},
	{"HtMIMOPS",					Set_HtMIMOPSmode_Proc},
	{"HtRdg",						Set_HtRdg_Proc},
	{"HtLinkAdapt",				Set_HtLinkAdapt_Proc},
	{"HtAmsdu",					Set_HtAmsdu_Proc},
	{"HtAutoBa",					Set_HtAutoBa_Proc},
	{"HtProtect",					Set_HtProtect_Proc},
	{"HtMimoPs",					Set_HtMimoPs_Proc},
	{"HtTxStream",				Set_HtTxStream_Proc},
	{"HtRxStream",				Set_HtRxStream_Proc},
	{"ForceShortGI",				Set_ForceShortGI_Proc},
	{"ForceGF",	Set_ForceGF_Proc},
	{"HtTxBASize",					Set_HtTxBASize_Proc},
	{"BurstMode",					Set_BurstMode_Proc},
#ifdef GREENAP_SUPPORT
	{"GreenAP",					Set_GreenAP_Proc},
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{"PcieAspm",				set_pcie_aspm_dym_ctrl_cap_proc},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
	{"HtDisallowTKIP",				Set_HtDisallowTKIP_Proc},
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	{"VhtBw",					Set_VhtBw_Proc},
	{"VhtStbc",					Set_VhtStbc_Proc},
	{"VhtBwSignal",				set_VhtBwSignal_Proc},
	{"VhtDisallowNonVHT",		Set_VhtDisallowNonVHT_Proc},
#endif /* DOT11_VHT_AC */

#ifdef IAPP_SUPPORT
	{"IappPID",					Set_IappPID_Proc},
#endif /* IAPP_SUPPORT */

#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",				Set_PktAggregate_Proc},
#endif /* AGGREGATION_SUPPORT */

	{"WmmCapable",				Set_AP_WmmCapable_Proc},

	{"NoForwarding",				Set_NoForwarding_Proc},
	{"NoForwardingBTNBSSID",		Set_NoForwardingBTNSSID_Proc},
	{"HideSSID",					Set_HideSSID_Proc},
	{"IEEE80211H",				Set_IEEE80211H_Proc},
	{"VLANID",					Set_VLANID_Proc},
	{"VLANPriority",				Set_VLANPriority_Proc},
	{"VLANTag",					Set_VLAN_TAG_Proc},
	{"AuthMode",					Set_SecAuthMode_Proc},
	{"EncrypType",					Set_SecEncrypType_Proc},
	{"WpaMixPairCipher",			Set_AP_WpaMixPairCipher_Proc},
	{"RekeyInterval",				Set_AP_RekeyInterval_Proc},
	{"RekeyMethod",				Set_AP_RekeyMethod_Proc},
	{"DefaultKeyID",				Set_SecDefaultKeyID_Proc},
	{"Key1",						Set_SecKey1_Proc},
	{"Key2",						Set_SecKey2_Proc},
	{"Key3",						Set_SecKey3_Proc},
	{"Key4",						Set_SecKey4_Proc},
	{"AccessPolicy",				Set_AccessPolicy_Proc},
	{"ACLAddEntry",					Set_ACLAddEntry_Proc},
	{"ACLDelEntry",					Set_ACLDelEntry_Proc},
	{"ACLShowAll",					Set_ACLShowAll_Proc},
	{"ACLClearAll",					Set_ACLClearAll_Proc},
	{"WPAPSK",					Set_SecWPAPSK_Proc},
	{"RadioOn",					Set_RadioOn_Proc},
	{"Lp",	Set_Lp_Proc},
#ifdef SPECIFIC_TX_POWER_SUPPORT
	{"PktPwr",						Set_AP_PKT_PWR},
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	{"AssocReqRssiThres",           Set_AP_ASSOC_REQ_RSSI_THRESHOLD},
	{"KickStaRssiLow",				Set_AP_KickStaRssiLow_Proc},
#ifdef AP_SCAN_SUPPORT
	{"SiteSurvey",					Set_SiteSurvey_Proc},
	{"AutoChannelSel",				Set_AutoChannelSel_Proc},
	{"PartialScan",					Set_PartialScan_Proc},
	{"ACSCheckTime",				Set_AutoChannelSelCheckTime_Proc},
#endif /* AP_SCAN_SUPPORT */
	{"ResetCounter",				Set_ResetStatCounter_Proc},
	{"DisConnectSta",				Set_DisConnectSta_Proc},
#ifdef VENDOR_FEATURE7_SUPPORT
	{"DisConnectBssSta",			Set_DisConnectBssSta_Proc},
#endif
	{"DisConnectAllSta",			Set_DisConnectAllSta_Proc},
#ifdef DOT1X_SUPPORT
	{"IEEE8021X",					Set_IEEE8021X_Proc},
	{"PreAuth",						Set_PreAuth_Proc},
	{"PMKCachePeriod",				Set_AP_PMKCachePeriod_Proc},
	{"own_ip_addr",					Set_OwnIPAddr_Proc},
	{"EAPifname",					Set_EAPIfName_Proc},
	{"PreAuthifname",				Set_PreAuthIfName_Proc},
	{"RADIUS_Server",				Set_RADIUS_Server_Proc},
	{"RADIUS_Port",					Set_RADIUS_Port_Proc},
	{"RADIUS_Key",					Set_RADIUS_Key_Proc},
	{"DeletePMKID",					Set_DeletePMKID_Proc},
	{"DumpPMKID",					Set_DumpPMKID_Proc},
#ifdef RADIUS_MAC_ACL_SUPPORT
	{"RADIUS_MacAuth_Enable",                       Set_RADIUS_MacAuth_Enable_Proc},
	{"RADIUS_CacheTimeout",                         Set_RADIUS_CacheTimeout_Proc},
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */
#ifdef DBG
	{"Debug",						Set_Debug_Proc},
	{"DebugCat",					Set_DebugCategory_Proc},
#endif /* DBG */
#ifdef RANDOM_PKT_GEN
	{"TxCtrl",					Set_TxCtrl_Proc},
#endif
#ifdef CSO_TEST_SUPPORT
	{"CsCtrl",					Set_CsCtrl_Proc},
#endif

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	{"RBIST_SwitchMode",	        Set_RBIST_Switch_Mode},
	{"RBIST_CaptureStart",	        Set_RBIST_Capture_Start},
	{"RBIST_CaptureStatus",	        Get_RBIST_Capture_Status},
	{"RBIST_RawDataProc",           Get_RBIST_Raw_Data_Proc},
	{"RBIST_IQDataProc",            Get_RBIST_IQ_Data_Proc},
	{"WirelessInfo",                Get_System_Wireless_Info},
#endif/* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

	{"IRR_ADC",                     Set_IRR_ADC},
	{"IRR_RxGain",                  Set_IRR_RxGain},
	{"IRR_TTG",                     Set_IRR_TTG},
	{"IRR_TTGOnOff",                Set_IRR_TTGOnOff},

#ifdef MT_DFS_SUPPORT /* Jelly20150301 */
	{"ShowDfsCh",                   Set_DfsChannelShow_Proc},
	{"ShowDfsBw",                   Set_DfsBwShow_Proc},
	{"ShowDfsRDMode",               Set_DfsRDModeShow_Proc},

	{"ShowDfsRegion",               Set_DfsRDDRegionShow_Proc},
	{"RadarDetectStart",            Set_RadarDetectStart_Proc},
	{"RadarDetectStop",             Set_RadarDetectStop_Proc},
	{"ByPassCac",                   Set_ByPassCac_Proc},
	{"ShowDfsNOP",			Show_DfsNonOccupancy_Proc},
	{"DfsNOPClean",			Set_DfsNOP_Proc},
	{"RDDReport",			Set_RDDReport_Proc},

	/* DFS zero wait */
	{"DfsZeroWaitCacTime",          Set_DfsZeroWaitCacTime_Proc},
	{"DfsDedicatedBwCh",		Set_DedicatedBwCh_Proc},
	{"DfsModifyChList",		Set_ModifyChannelList_Proc},
	{"DfsDynamicCtrl",		Set_DfsZeroWaitDynamicCtrl_Proc},
	{"DfsForceNOP", 		Set_DfsZeroWaitNOP_Proc},
	{"DfsTargetCh",			Set_DfsTargetCh_Proc},
#endif

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
	{"RadarShow",					Set_RadarShow_Proc},
#ifdef CARRIER_DETECTION_SUPPORT
	{"CarrierDetect",				Set_CarrierDetect_Proc},
	{"CarrierCriteria",				Set_CarrierCriteria_Proc},
	{"CarrierReCheck",				Set_CarrierReCheck_Proc},
	{"CarrierGoneThreshold",			Set_CarrierGoneThreshold_Proc},
	{"CarrierDebug",				Set_CarrierDebug_Proc},
	{"Delta",						Set_CarrierDelta_Proc},
	{"DivFlag",						Set_CarrierDivFlag_Proc},
	{"CarrThrd",					Set_CarrierThrd_Proc},
	/* v2 functions */
	{"SymRund",					Set_CarrierSymRund_Proc},
	{"CarrMask",					Set_CarrierMask_Proc},
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) */

#ifdef CONFIG_ATE
	{"ATE",	SetATE},
	{"ATEMPSDUMP", SetATEMPSDump},
	{"ATEMPSPHY", SetATEMPSPhyMode},
	{"ATEMPSRATE", SetATEMPSRate},
	{"ATEMPSPATH", SetATEMPSPath},
	{"ATEMPSLEN", SetATEMPSPayloadLen},
	{"ATEMPSTXCNT", SetATEMPSPktCnt},
	{"ATEMPSTXPWR", SetATEMPSPwr},
	{"ATEMPSNSS", SetATEMPSNss},
	{"ATEMPSPKTBW", SetATEMPSPktBw},
	{"ATEMPSTXSTART", SetATEMPSStart},
	{"ATELOGEN", SetATELOGEnable},
	{"ATELOGDUMP", SetATELOGDump},
	{"ATEMACTRX", SetATEMACTRx},
	{"ATETXSTREAM", SetATETxStream},
	{"ATERXSTREAM", SetATERxStream},
	{"ATETXSENABLE", SetATETxSEnable},
	{"ATERXFILTER", SetATERxFilter},
	{"ATELOGDIS", SetATELOGDisable},
	{"ATEDEQCNT", SetATEDeqCnt},
	{"ATEQID", SetATEQid},
	{"ATEDA", SetATEDa},
	{"ATESA", SetATESa},
	{"ADCDump", SetADCDump},
	{"ATEBSSID", SetATEBssid},
	{"ATECHANNEL", SetATEChannel},
	{"ATEDUTYCYCLE", set_ate_duty_cycle},
	{"ATEPKTTXTIME", set_ate_pkt_tx_time},
	{"ATECTRLBANDIDX", set_ate_control_band_idx},
#ifdef MT7615
	{"ATECHANNELEXT", set_ate_channel_ext},
	{"ATESTARTTXEXT", set_ate_start_tx_ext},
#endif /* MT7615 */
	{"ATEINITCHAN", SetATEInitChan},
#ifdef RTMP_TEMPERATURE_CALIBRATION
	{"ATETEMPCAL", SetATETempCal},
	{"ATESHOWTSSI",	SetATEShowTssi},
#endif /* RTMP_TEMPERATURE_CALIBRATION */
	{"ATETXPOW0", SetATETxPower0},
	{"ATETXPOW1", SetATETxPower1},
	{"ATETXPOW2", SetATETxPower2},
	{"ATETXPOW3", SetATETxPower3},
	{"ATEFORCETXPOWER", SetATEForceTxPower},
	{"ATETXPOWEVAL", SetATETxPowerEvaluation},
	{"ATETXANT", SetATETxAntenna},
	{"ATERXANT", SetATERxAntenna},
	{"ATERFPOWER", SetATERFPower},
	{"ATEDIGITALPOWER", SetATEDigitalPower},
	{"ATEDCOFFSETI", SetATEDCOffset_I},
	{"ATEDCOFFSETQ", SetATEDCOffset_Q},
	{"ATETXFREQOFFSET", SetATETxFreqOffset},
	{"ATETXBW", SetATETxBw},
	{"ATETXLEN", SetATETxLength},
	{"ATETXCNT", SetATETxCount},
	{"ATETXMCS", SetATETxMcs},
	{"ATEVHTNSS", SetATEVhtNss},
	{"ATETXLDPC", SetATETxLdpc},
	{"ATETXSTBC", SetATETxStbc},
	{"ATETXMODE", SetATETxMode},
	{"ATETXGI", SetATETxGi},
	{"ATERXFER", SetATERxFer},
	{"ATERRF", SetATEReadRF},
#if (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF))
	{"ATEWRF1",	SetATEWriteRF1},
	{"ATEWRF2", SetATEWriteRF2},
	{"ATEWRF3", SetATEWriteRF3},
	{"ATEWRF4",	SetATEWriteRF4},
#endif /* (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF)) */
	{"ATELDE2P", SetATELoadE2p},
	{"ATERE2P", SetATEReadE2p},
#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUPPORT */
	{"ATEAUTOALC", SetATEAutoAlc},
	{"ATETEMPSENSOR", SetATETempSensor},
	{"ATEIPG", SetATEIpg},
	{"ATEPAYLOAD", SetATEPayload},
	{"ATEFIXEDPAYLOAD", SetATEFixedPayload},
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
	{"ATETxBfInit",              SetATETxBfDutInitProc},
	{"ATETxBfGdInit",            SetATETxBfGdInitProc},
	{"ATETxBfChanProfileUpdate", SetATETxBfChanProfileUpdate},
	{"ATETXBF",                  SetATETXBFProc},
	{"ATETXSOUNDING",            SetATETxSoundingProc},
	{"ATEIBfGdCal",              SetATEIBfGdCal},
	{"ATEIBfInstCal",            SetATEIBfInstCal},
	{"ATETxBfLnaGain",           SetATETxBfLnaGain},
	{"ATEIBfProfileConfig",      SetATEIBfProfileUpdate},
	{"ATEEBfProfileConfig",      SetATEEBfProfileConfig},
	{"ATETxBfProfileRead",       SetATETxBfProfileRead},
	{"ATETxPacketWithBf",        SetATETxPacketWithBf},
	{"ATEIBFPhaseE2pUpdate",     SetATETxBfPhaseE2pUpdate},
	{"ATEIBFPhaseComp",          SetATEIBfPhaseComp},
	{"ATEIBFPhaseVerify",        SetATEIBfPhaseVerify},
	{"ATEConTxETxBfGdProc",      SetATEConTxETxBfGdProc},
	{"ATEConTxETxBfInitProc",    SetATEConTxETxBfInitProc},
	{"ATESPE",                   SetATESpeIdx},
	{"ATETXEBF",                 SetATEEBfTx},
	{"ATEEBFCE",                 SetATEEBFCE},
	{"ATEEBFCEInfo",             SetATEEBFCEInfo},
	{"ATEEBFCEHELP",             SetATEEBFCEHelp},
#endif
#endif /* TXBF_SUPPORT */
	{"ATETTR", SetATETtr},
	{"ATESHOW", SetATEShow},
	{"ATEHELP", SetATEHelp},
#ifdef CONFIG_QA
	{"TxStop", SetTxStop},
	{"RxStop", SetRxStop},
#ifdef DBG
	{"EERead", SetEERead},
	{"EEWrite",	SetEEWrite},
	{"BBPRead", SetBBPRead},
	{"BBPWrite", SetBBPWrite},
#endif /* DBG */
#endif /* CONFIG_QA */

#if defined(MT7615)

#ifdef SINGLE_SKU_V2
	{"ATESINGLESKU", SetATESingleSKUEn},
#endif /* SINGLE_SKU_V2 */

	{"BFBackoffMode", SetATEBFBackoffMode},
	{"ATEPowerPercent", SetATEPowerPercentEn},
	{"ATEPowerDropLevel", SetATEPowerPercentCtrl},
	{"ATEBFBackoff", SetATEBFBackoffEn},
	{"ATETempComp", SetATETempCompEn},
	{"ATETSSI", SetATETSSIEn},
	{"ATETXPOWERCTRL", SetATETxPowerCtrlEn},
#endif /* defined(MT7615) */

#endif /* CONFIG_ATE */

#ifdef APCLI_SUPPORT
	{"ApCliEnable",				Set_ApCli_Enable_Proc},
	{"ApCliSsid",					Set_ApCli_Ssid_Proc},
	{"ApCliBssid",					Set_ApCli_Bssid_Proc},
	{"ApCliAuthMode",				Set_SecAuthMode_Proc},
	{"ApCliEncrypType",			Set_SecEncrypType_Proc},
	{"ApCliDefaultKeyID",			Set_SecDefaultKeyID_Proc},
	{"ApCliWPAPSK",				Set_SecWPAPSK_Proc},
	{"ApCliKey1",					Set_SecKey1_Proc},
	{"ApCliKey2",					Set_SecKey2_Proc},
	{"ApCliKey3",					Set_SecKey3_Proc},
	{"ApCliKey4",					Set_SecKey4_Proc},
	{"ApCliTxMode",					Set_ApCli_TxMode_Proc},
	{"ApCliTxMcs",					Set_ApCli_TxMcs_Proc},
#ifdef APCLI_CONNECTION_TRIAL
	/*
	 for Trial the root AP which locates on another channel
	 what if the connection is ok, it will make BSSID switch to the new channel.
	*/
	{"ApCliTrialCh",				Set_ApCli_Trial_Ch_Proc},
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef DBDC_MODE
	{"ApCliWirelessMode",					Set_ApCli_WirelessMode_Proc},
#endif /*DBDC_MODE*/
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	{"ApCliAutoConnect",			Set_ApCli_AutoConnect_Proc},
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	{"MACRepeaterEn",			    Set_ReptMode_Enable_Proc},
	{"CliLinkMAP",                  Set_Cli_Link_Map_Proc},
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WSC_AP_SUPPORT
	{"ApCliWscSsid",				Set_AP_WscSsid_Proc},
	{"ApCliWscScanMode",			Set_ApCli_WscScanMode_Proc},
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_CERT_SUPPORT
	{"ApCliCertEnable",				Set_ApCli_Cert_Enable_Proc},
	/* Add for APCLI PMF 5.3.3.3 option test item. (Only Tx De-auth Req. and make sure the pkt can be Encrypted) */
	{"ApCliTxDeAuth",				ApCliTxDeAuth},
#endif /* APCLI_CERT_SUPPORT */

#ifdef WSC_AP_SUPPORT
	{"WpsCertEnable", 			Set_Wps_Cert_Enable_Proc},
#endif


#ifdef DOT11W_PMF_SUPPORT
	{"ApCliPMFMFPC",                                         Set_ApCliPMFMFPC_Proc},
	{"ApCliPMFMFPR",                                         Set_ApCliPMFMFPR_Proc},
	{"ApCliPMFSHA256",                                      Set_ApCliPMFSHA256_Proc},
#endif /* DOT11W_PMF_SUPPORT */
#ifdef ROAMING_ENHANCE_SUPPORT
	{"RoamingEnhance",			Set_RoamingEnhance_Enable_Proc},
#endif /* ROAMING_ENHANCE_SUPPORT */

#endif /* APCLI_SUPPORT */
#ifdef WSC_AP_SUPPORT
#ifdef CON_WPS
	{"ConWpsApCliMode",			Set_ConWpsApCliMode_Proc},
	{"ConWpsApCliDisabled",                     Set_ConWpsApCliDisabled_Proc},
	{"ConWpsApDisabled",                     Set_ConWpsApDisabled_Proc},
	{"ConWpsApcliPreferIface",		Set_ConWpsApcliAutoPreferIface_Proc},
#endif /* CON_WPS */
	{"WscConfMode",				Set_AP_WscConfMode_Proc},
	{"WscConfStatus",				Set_AP_WscConfStatus_Proc},
	{"WscMode",					Set_AP_WscMode_Proc},
	{"WscStatus",					Set_WscStatus_Proc},
	{"WscGetConf",				Set_AP_WscGetConf_Proc},
	{"WscPinCode",				Set_AP_WscPinCode_Proc},
	{"WscStop",		Set_WscStop_Proc},
	{"WscGenPinCode",	Set_WscGenPinCode_Proc},
	{"WscVendorPinCode",            Set_WscVendorPinCode_Proc},
	{"WscSecurityMode",				Set_AP_WscSecurityMode_Proc},
	{"WscMultiByteCheck",			Set_AP_WscMultiByteCheck_Proc},
	{"WscVersion",					Set_WscVersion_Proc},
#ifdef VENDOR_FEATURE6_SUPPORT
	/* HEX : 32 Length */
	{"WscUUID_E",					Set_WscUUID_HEX_E_Proc},
	/* 37 Length */
	{"WscUUID_Str",					Set_WscUUID_STR_Proc},
#endif /* VENDOR_FEATURE6_SUPPORT */
#ifdef WSC_V2_SUPPORT
	{"WscV2Support",				Set_WscV2Support_Proc},
	{"WscVersion2",				Set_WscVersion2_Proc},
	{"WscExtraTlvTag",				Set_WscExtraTlvTag_Proc},
	{"WscExtraTlvType",				Set_WscExtraTlvType_Proc},
	{"WscExtraTlvData",			Set_WscExtraTlvData_Proc},
	{"WscSetupLock",				Set_WscSetupLock_Proc},
	{"WscFragment",					Set_WscFragment_Proc},
	{"WscFragmentSize",			Set_WscFragmentSize_Proc},
	{"WscMaxPinAttack",			Set_WscMaxPinAttack_Proc},
	{"WscSetupLockTime",			Set_WscSetupLockTime_Proc},
#endif /* WSC_V2_SUPPORT */
	{"WscAutoTriggerDisable",		Set_WscAutoTriggerDisable_Proc},
#endif /* WSC_AP_SUPPORT */
#ifdef UAPSD_SUPPORT
	{"UAPSDCapable",				Set_UAPSD_Proc},
#endif /* UAPSD_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
	{"IgmpSnEnable",				Set_IgmpSn_Enable_Proc},
	{"IgmpAdd",					Set_IgmpSn_AddEntry_Proc},
	{"IgmpDel",					Set_IgmpSn_DelEntry_Proc},
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
	{"McastPhyMode",				Set_McastPhyMode},
	{"McastMcs",					Set_McastMcs},
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	{"FixedTxMode",                 Set_FixedTxMode_Proc},
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	{"OpMode",					Set_OpMode_Proc},
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef DSCP_PRI_SUPPORT
	{"DscpPri_2.4G",				Set_Dscp_Pri_2G_Proc},
	{"DscpPri_5G",					Set_Dscp_Pri_5G_Proc},
#endif

#ifdef TXBF_SUPPORT
#if (!defined(MT_MAC))
	{"TxBfTag",					Set_TxBfTag_Proc},
	{"ReadITxBf",				Set_ReadITxBf_Proc},
	{"WriteITxBf",				Set_WriteITxBf_Proc},
	{"StatITxBf",				Set_StatITxBf_Proc},
	{"ReadETxBf",				Set_ReadETxBf_Proc},
	{"WriteETxBf",				Set_WriteETxBf_Proc},
	{"StatETxBf",				Set_StatETxBf_Proc},
	{"ITxBfTimeout",			Set_ITxBfTimeout_Proc},
	{"ETxBfTimeout",			Set_ETxBfTimeout_Proc},
	{"ITxBfCal",				Set_ITxBfCal_Proc},
	{"ITxBfLnaCal",				Set_ITxBfLnaCal_Proc},
#endif /* MT76x2, MT_MAC */
	{"InvTxBfTag",				Set_InvTxBfTag_Proc},
	{"ITxBfDivCal",				Set_ITxBfDivCal_Proc},

	{"ITxBfEn",					Set_ITxBfEn_Proc},
	{"ETxBfEnCond",				Set_ETxBfEnCond_Proc},
	{"ETxBfCodebook",			Set_ETxBfCodebook_Proc},
	{"ETxBfCoefficient",		Set_ETxBfCoefficient_Proc},
	{"ETxBfGrouping",			Set_ETxBfGrouping_Proc},
	{"ETxBfNoncompress",		Set_ETxBfNoncompress_Proc},
	{"ETxBfIncapable",			Set_ETxBfIncapable_Proc},
	{"NoSndgCntThrd",			Set_NoSndgCntThrd_Proc},
	{"NdpSndgStreams",			Set_NdpSndgStreams_Proc},


#ifdef MT_MAC
	{"TxBfTxApply",             Set_TxBfTxApply},
	{"TriggerSounding",			Set_Trigger_Sounding_Proc},
	{"StopSounding",			Set_Stop_Sounding_Proc},
	{"StaRecBfUpdate",          Set_StaRecBfUpdate},
	{"StaRecBfRead",            Set_StaRecBfRead},
	{"TxBfAwareCtrl",           Set_TxBfAwareCtrl},
#ifdef CONFIG_ATE
	{"StaRecCmmUpdate",         Set_StaRecCmmUpdate},
	{"BssInfoUpdate",           Set_BssInfoUpdate},
	{"DevInfoUpdate",           Set_DevInfoUpdate},
	{"ManualAssoc",             SetATEAssocProc},
#endif /* CONFIG_ATE */
	{"TxBfPfmuMemAlloc",        Set_TxBfPfmuMemAlloc},
	{"TxBfPfmuMemRelease",      Set_TxBfPfmuMemRelease},
	{"TxBfPfmuMemAllocMapRead", Set_TxBfPfmuMemAllocMapRead},
	{"TxBfProfileTagHelp",      Set_TxBfProfileTag_Help},
	{"TxBfProfileTagInValid",   Set_TxBfProfileTag_InValid},
	{"TxBfProfileTagPfmuIdx",   Set_TxBfProfileTag_PfmuIdx},
	{"TxBfProfileTagBfType",    Set_TxBfProfileTag_BfType},
	{"TxBfProfileTagBw",        Set_TxBfProfileTag_DBW},
	{"TxBfProfileTagSuMu",      Set_TxBfProfileTag_SuMu},
	{"TxBfProfileTagMemAlloc",  Set_TxBfProfileTag_Mem},
	{"TxBfProfileTagMatrix",    Set_TxBfProfileTag_Matrix},
	{"TxBfProfileTagSnr",       Set_TxBfProfileTag_SNR},
	{"TxBfProfileTagSmtAnt",    Set_TxBfProfileTag_SmartAnt},
	{"TxBfProfileTagSeIdx",     Set_TxBfProfileTag_SeIdx},
	{"TxBfProfileTagRmsdThrd",  Set_TxBfProfileTag_RmsdThrd},
	{"TxBfProfileTagMcsThrd",   Set_TxBfProfileTag_McsThrd},
	{"TxBfProfileTagTimeOut",   Set_TxBfProfileTag_TimeOut},
	{"TxBfProfileTagDesiredBw", Set_TxBfProfileTag_DesiredBW},
	{"TxBfProfileTagDesiredNc", Set_TxBfProfileTag_DesiredNc},
	{"TxBfProfileTagDesiredNr", Set_TxBfProfileTag_DesiredNr},
	{"TxBfProfileTagRead",      Set_TxBfProfileTagRead},
	{"TxBfProfileTagWrite",     Set_TxBfProfileTagWrite},
	{"TxBfProfileDataRead",     Set_TxBfProfileDataRead},
	{"TxBfProfileDataWrite",    Set_TxBfProfileDataWrite},
	{"TxBfProfilePnRead",       Set_TxBfProfilePnRead},
	{"TxBfProfilePnWrite",      Set_TxBfProfilePnWrite},
#ifdef TXBF_DYNAMIC_DISABLE
	{"TxBfDisable",             Set_TxBfDynamicDisable_Proc},
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* MT_MAC */

#endif /* TXBF_SUPPORT */
#ifdef VHT_TXBF_SUPPORT
	{"VhtNDPA",				    Set_VhtNDPA_Sounding_Proc},
#endif /* VHT_TXBF_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	{"wf_fwd",		Set_WifiFwd_Proc},
	{"wf_fwd_down",		Set_WifiFwd_Down},
	{"wf_fwd_acs",	Set_WifiFwdAccessSchedule_Proc},
	{"wf_fwd_hij",	Set_WifiFwdHijack_Proc},
	{"wf_fwd_bpdu", Set_WifiFwdBpdu_Proc},
	{"wf_fwd_rep",	Set_WifiFwdRepDevice},
	{"wf_fwd_show",	Set_WifiFwdShowEntry},
	{"wf_fwd_del",	Set_WifiFwdDeleteEntry},
	{"pkt_src_show",   Set_PacketSourceShowEntry},
	{"pkt_src_del",	Set_PacketSourceDeleteEntry},
	{"wf_fwd_bridge",	Set_WifiFwdBridge_Proc},
#endif /* CONFIG_WIFI_PKT_FWD */


#ifdef MT_MAC
#ifdef DBG
	{"FixedRate",				Set_Fixed_Rate_Proc},
	{"FixedRateFallback",		Set_Fixed_Rate_With_FallBack_Proc},
	{"RaDebug",					Set_RA_Debug_Proc},
#endif /* DBG */
#endif /* MT_MAC */


#ifdef STREAM_MODE_SUPPORT
	{"StreamMode",				Set_StreamMode_Proc},
	{"StreamModeMac",			Set_StreamModeMac_Proc},
	{"StreamModeMCS",			Set_StreamModeMCS_Proc},
#endif /* STREAM_MODE_SUPPORT */

	{"LongRetry",	Set_LongRetryLimit_Proc},
	{"ShortRetry",	Set_ShortRetryLimit_Proc},
	{"AutoFallBack",		Set_AutoFallBack_Proc},
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
	{"DiagOpt",					    Set_DiagOpt_Proc},
	{"diag_cond",                   Set_diag_cond_Proc},
#endif /* DBG_DIAGNOSE */
#endif /* RTMP_MAC_PCI */

	{"MeasureReq",					Set_MeasureReq_Proc},
	{"TpcReq",						Set_TpcReq_Proc},
	{"TpcReqByAddr",				Set_TpcReqByAddr_Proc},
	{"PwrConstraint",				Set_PwrConstraint},
#ifdef TPC_SUPPORT
	{"TpcCtrl",						Set_TpcCtrl_Proc},
	{"TpcEn",						Set_TpcEnable_Proc},
#endif /* TPC_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	{"BcnReq",						Set_BeaconReq_Proc},
	{"BcnReqRandInt",				Set_BeaconReq_RandInt_Proc},
	{"LinkReq",						Set_LinkMeasureReq_Proc},
	{"RrmEnable",					Set_Dot11kRRM_Enable_Proc},
	{"TxReq",						Set_TxStreamMeasureReq_Proc},


	/* only for selftesting and debugging. */
	{"rrm",						Set_RRM_Selftest_Proc},
#endif /* DOT11K_RRM_SUPPORT */
	{"RegDomain",					Set_Reg_Domain_Proc},
#ifdef CONFIG_DOT11V_WNM
	{"WNMCertEnable",				Set_WNMCertEnable_Proc},
#endif
#ifdef DOT11N_DRAFT3
	{"OBSSScanParam",				Set_OBSSScanParam_Proc},
	{"AP2040Rescan",			    Set_AP2040ReScan_Proc},
	{"HtBssCoex",					Set_HT_BssCoex_Proc},
	{"HtBssCoexApCntThr",			Set_HT_BssCoexApCntThr_Proc},
#endif /* DOT11N_DRAFT3 */
	{"EntryLifeCheck",				Set_EntryLifeCheck_Proc},
#ifdef DOT11R_FT_SUPPORT
	{"ft",							FT_Ioctl},
	{"ftenable",					Set_FT_Enable},
	{"ftmdid",						Set_FT_Mdid},
	{"ftr0khid",					Set_FT_R0khid},
	{"ftric",						Set_FT_RIC},
	{"ftotd",						Set_FT_OTD},
#endif /* DOT11R_FT_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
	{"efuseLoadFromBin",			set_eFuseLoadFromBin_Proc}, /* For backward compatible, the usage is the same as bufferLoadFromBin + bufferWriteBack */
	{"efuseFreeNumber",			    set_eFuseGetFreeBlockCount_Proc},
	{"efuseDump",				    set_eFusedump_Proc},
#ifdef CONFIG_ATE
	{"bufferLoadFromEfuse",		    Set_LoadEepromBufferFromEfuse_Proc},
#ifdef BB_SOC
	{"efuseBufferModeWriteBack",	set_BinModeWriteBack_Proc},
#else
	{"efuseBufferModeWriteBack",	set_eFuseBufferModeWriteBack_Proc}, /* For backward compatible, the usage is the same as bufferWriteBack */
#endif
#endif /* CONFIG_ATE */
#endif /* RTMP_EFUSE_SUPPORT */
	{"bufferLoadFromBin",			Set_LoadEepromBufferFromBin_Proc},
	{"bufferWriteBack",			Set_EepromBufferWriteBack_Proc},
	{"bufferMode",			        Set_bufferMode_Proc},

#ifdef CAL_FREE_IC_SUPPORT
	{"bufferLoadFromCalFree",		Set_LoadCalFreeData_Proc},
	{"CheckCalFree",		        Set_CheckCalFree_Proc},
#endif

#ifdef RF_LOCKDOWN
	{"CalFreeApply",                SetCalFreeApply},
	{"WriteEffuseRFpara",           SetWriteEffuseRFpara},
#endif /* RF_LOCKDOWN */

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
	{"WlanLed",					    Set_WlanLed_Proc},
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef AP_QLOAD_SUPPORT
	{"qloadclr",					Set_QloadClr_Proc},
	{"qloadalarmtimethres",			Set_QloadAlarmTimeThreshold_Proc}, /* QLOAD ALARM */
	{"qloadalarmnumthres",			Set_QloadAlarmNumThreshold_Proc}, /* QLOAD ALARM */
#endif /* AP_QLOAD_SUPPORT */

	{"ra_interval",					Set_RateAdaptInterval},

#ifdef THERMAL_PROTECT_SUPPORT
	{"tpc",						    set_thermal_protection_criteria_proc},
	{"tpc_duty",					set_thermal_protection_admin_ctrl_duty_proc},
#endif /* THERMAL_PROTECT_SUPPORT */

#ifdef SMART_ANTENNA
	{"sa",						    Set_SmartAnt_Proc},
	{"sa_msc",					    Set_McsStableCnt_Proc},
	{"sa_mcs",					    Set_SA_McsBound_Proc},
	{"sa_sta",					    Set_SA_Station_Proc},
	{"sa_starule",				    Set_SA_StationCandRule_Proc},
	{"sa_mode",					Set_SA_Mode_Proc},
	{"sa_txNss",					set_SA_txNss_Proc},
	{"sa_ant",					    Set_SA_StaticAntPair_Proc},
	{"sa_agsp",					Set_SA_AGSP_Proc},
	{"sa_tseq",					Set_SA_TrainSeq_Proc},
	{"sa_tdelay",					Set_SA_TrainDelay_Proc},
	{"sa_tcond",					Set_SA_TrainCond_Proc},
	{"sa_rssivar",					Set_SA_RssiVariance_Proc},
	{"sa_rssith",					Set_SA_RssiThreshold_Proc},
	{"sa_skipconf",				Set_SA_SkipConfirmStage_Proc},
	{"sa_tcand",					Set_SA_AntCand_Proc},
	{"sa_tp",					    Set_TestPeriod_Proc},
	{"sa_tc",					    Set_MaxAntennaTry_Proc},
	{"sa_dbg",					    Set_DbgLogOn_Proc},
#endif /* SMART_ANTENNA // */

	{"memdebug",					Set_MemDebug_Proc},

#ifdef CONFIG_AP_SUPPORT
	{"pslifetime",					Set_PowerSaveLifeTime_Proc},

#ifdef MBSS_SUPPORT
	{"MBSSWirelessMode",			Set_MBSS_WirelessMode_Proc},
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	{"VcoPeriod",					Set_VcoPeriod_Proc},


#ifdef DOT11W_PMF_SUPPORT
	{"PMFMFPC",                     Set_PMFMFPC_Proc},
	{"PMFMFPR",                     Set_PMFMFPR_Proc},
	{"PMFSHA256",                   Set_PMFSHA256_Proc},
	{"PMFSA_Q",			Set_PMFSA_Q_Proc},
#endif /* DOT11W_PMF_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT
	{"MO_FalseCCATh",				Set_MO_FalseCCATh_Proc},
#endif /* MICROWAVE_OVEN_SUPPORT */

	{"no_bcn",					set_no_bcn},

#if defined(WFA_VHT_PF) || defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
	{"force_amsdu",				set_force_amsdu},
#endif /* defined(WFA_VHT_PF) || defined(MT7603_FPGA) */

#ifdef WFA_VHT_PF
	{"nss_mcs_opt",				set_vht_nss_mcs_opt},	/* SIGMA */
	{"opt_md_notif_ie",			set_vht_opmode_notify_ie}, /* SIGMA */
	{"force_op",					set_force_operating_mode},
	{"force_noack",				set_force_noack},
	{"force_vht_sgi",				set_force_vht_sgi},
	{"force_vht_stbc",				set_force_vht_tx_stbc},
	{"ext_cca",					set_force_ext_cca},
#endif /* WFA_VHT_PF */
	{"rf",						SetRF},
	{"tssi_enable", set_tssi_enable},
#ifdef DYNAMIC_VGA_SUPPORT
	{"DyncVgaEnable", Set_DyncVgaEnable_Proc},
	{"fc_hth", set_false_cca_hi_th},
	{"fc_lth", set_false_cca_low_th},
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef MT_MAC
	{"TimEnable",				Set_AP_TimEnable_Proc},/* only for development purpose!! iwpriv ra0 set */
	{"TmrVer",                  setTmrVerProc},
	{"TmrEnable",				setTmrEnableProc},/* 0: disalbe, 1: initialiter, 2: responser. */
	{"TmrCal",                  SetTmrCalProc},
#ifdef FTM_SUPPORT
	{"FtmReq",					Set_FtmReqTx_Proc},
	{"FtmAsap",					Set_FtmAsapReq_Proc},
	{"FtmMinDelta",				Set_FtmMinDelta_Proc},
	{"FtmNum",					Set_FtmNum_Proc},
	{"FtmBurstExp",				Set_FtmBurstExp_Proc},
	{"FtmBurstDur",				Set_FtmBurstDuration_Proc},
	{"FtmBurstPeriod",			Set_FtmBurstPeriod_Proc},
	{"FtmLciValue",				Set_FtmLciValue_Proc},
	{"FtmLciLat",				Set_FtmLciLat_ThisAP_Proc},
	{"FtmLciLng",				Set_FtmLciLng_ThisAP_Proc},
	{"FtmLciAlt",				Set_FtmLciAlt_ThisAP_Proc},
	{"FtmLciKnown",				Set_FtmLciKnown_Proc},
	{"FtmLci",					Set_FtmLciFlag_Proc},
	{"FtmZ",					Set_FtmZFlag_Proc},
	{"FtmCivicKnown",			Set_FtmCivicKnown_Proc},
	{"FtmCivic",				Set_FtmCivicFlag_Proc},
	{"Anqp",         Send_ANQP_Req_For_Test},
	{"Nrpt",		Send_NeighborReq_For_Test},
	{"RM",						Send_RadioMeasurement_Req_For_Test},
	{"RMMinAp",					Set_FtmRMMinimumApCount_Proc},
	{"RMRandomInt",					Set_FtmRMRandomizationInterval_Proc},
#endif /* FTM_SUPPORT */

	/* support CR4 cmds */
	{"cr4_query",                   set_cr4_query},
	{"cr4_set",                     set_cr4_set},
	{"cr4_capability",              set_cr4_capability},
	{"cr4_debug",                   set_cr4_debug},


	{"dump_remap_cr",               dump_remap_cr_content},
	{"ReCal",                       set_re_calibration},
	{"FwPhyOperation",		set_fw_phy_operation},
	{"get_fid",     set_get_fid},
	{"fwlog", set_fw_log},
	{"isrcmd",		set_isr_cmd},
	{"txop",		set_txop_cfg},
	{"rts",		set_rts_cfg},
	{"fwset",                   set_fw_cmd},
	{"fwget",                   get_fw_cmd},

#ifdef FW_DUMP_SUPPORT
	{"fwdump_maxsize", set_fwdump_max_size},
	{"fwdump_path", set_fwdump_path},
	{"fwdump_print", fwdump_print},
#endif
	{"protect", set_manual_protect},
	{"rdg", set_manual_rdg},
	{"cca_en", set_cca_en},
#ifdef DBG
	{"DumpTime",                Set_AP_DumpTime_Proc},/* only for development purpose!! iwpriv ra0 set */
	{"BcnStateCtrl",            Set_BcnStateCtrl_Proc},/* only for development purpose!! iwpriv ra0 set */
	{"trigger_tim",             set_tim_update_proc},
#endif
#ifdef CUT_THROUGH
	{"cut_through",		Set_CutThrough_Proc},
#endif /* CUT_THROUGH */
#ifdef RTMP_MAC_PCI
	{"pse_dbg", SetPSEWatchDog_Proc},
	{"pdma_dbg", Set_PDMAWatchDog_Proc},
#endif
	{"get_thermal_sensor", Set_themal_sensor},
#ifdef CFG_SUPPORT_MU_MIMO
	{"hqa_mu_cal_init_mcs",         hqa_mu_cal_init_mcs},
	{"hqa_mu_cal_lq",               hqa_mu_cal_lq},
	{"hqa_su_cal_lq",               hqa_su_cal_lq},
	{"hqa_mu_set_snr_offset",       hqa_mu_set_snr_offset},
	{"hqa_mu_set_zero_nss",         hqa_mu_set_zero_nss},
	{"hqa_mu_set_speedup_lq",       hqa_mu_set_speedup_lq},
	{"hqa_mu_set_mu_tbl",           hqa_mu_set_mu_table},
	{"hqa_mu_set_group",            hqa_mu_set_group},
	{"hqa_mu_set_enable",           hqa_mu_set_enable},
	{"hqa_mu_set_gid_up",           hqa_mu_set_gid_up},
	{"hqa_mu_set_trigger_mu_tx",    hqa_mu_set_trigger_mu_tx},

	/* jeffrey, 20141116 */
	/* the followings are relative to MU debbuging/verification, and not use for ATE */
	{"set_mu_profile",              SetMuProfileProc},              /* set MU profile */
	{"set_mu_grouptbl",             SetGroupTblEntryProc},          /* set group table entry */
	{"set_mu_clustertbl",           SetClusterTblEntryProc},        /* set cluster table entry */
	{"set_mu_enable",               SetMuEnableProc},           /* set MU enable or disable */
	{"set_mu_groupuserthreshold",   SetGroupUserThresholdProc},     /* set group threshold */
	{"set_mu_groupnssthreshold",    SetGroupNssThresholdProc},      /* set group NSS */
	{"set_mu_txreqmintime",         SetTxReqMinTimeProc},           /* set TX req min. time */
	{"set_mu_calcinitmcs",          SetCalculateInitMCSProc},       /* set calculate init MCS */
	{"set_mu_sunsscheck",           SetSuNssCheckProc},             /* set enable or disable NSS check */
	{"set_mu_txopdefault",          SetTxopDefaultProc},            /* set MU enable or disable */
	{"set_mu_sulossthreshold",      SetSuLossThresholdProc},        /* set SU loss threshold */
	{"set_mu_mugainthreshold",      SetMuGainThresholdProc},        /* set MU gain threshold */
	{"set_mu_secondaryacpolicy",    SetSecondaryAcPolicyProc},      /* set secondary AC policy */
	{"set_mu_grouptbldmcsmask",     SetGroupTblDmcsMaskProc},       /* set group table DMCS mask enable or disable */
	{"set_mu_maxgroupsearchcnt",    SetMaxGroupSearchCntProc},      /* set Max group search count */
	/* the followings are relative to trigger MU-flow test command */
	{"set_mu_send_gid_mgmt_frame",  SetTriggerGIDMgmtFrameProc},    /* set trigger GID mgmt. frame */
	{"set_mu_trigger_mutx",    SetTriggerMuTxProc},     /* set trigger MU TX */
	{"set_mu_trigger_degroup",      SetTriggerDegroupProc},     /* set trigger MU degrouping */
	{"set_mu_trigger_group",        SetTriggerGroupProc},       /* set trigger MU grouping */
	{"set_mu_trigger_bbp",          SetTriggerBbpProc},         /* set trigger LQ */
	{"set_mu_trigger_sounding",     SetTriggerSndProc},         /* set trigger MU sounding */
	/* the followings are relative to channel model setting */
#endif

#ifdef VOW_SUPPORT
	/* VOW GROUP table */
	{"vow_min_rate_token",  set_vow_min_rate_token},
	{"vow_max_rate_token",  set_vow_max_rate_token},
	{"vow_min_airtime_token",  set_vow_min_airtime_token},
	{"vow_max_airtime_token",  set_vow_max_airtime_token},
	{"vow_min_rate_bucket",  set_vow_min_rate_bucket},
	{"vow_max_rate_bucket",  set_vow_max_rate_bucket},
	{"vow_min_airtime_bucket",  set_vow_min_airtime_bucket},
	{"vow_max_airtime_bucket",  set_vow_max_airtime_bucket},
	{"vow_max_wait_time", set_vow_max_wait_time},
	{"vow_max_backlog_size", set_vow_max_backlog_size},


	/* VOW CTRL */
	{"vow_bw_enable", set_vow_bw_en},
	{"vow_refill_en", set_vow_refill_en},
	{"vow_airtime_fairness_en", set_vow_airtime_fairness_en},
	{"vow_txop_switch_bss_en", set_vow_txop_switch_bss_en},
	{"vow_dbdc_search_rule", set_vow_dbdc_search_rule},
	{"vow_refill_period", set_vow_refill_period},
	{"vow_bss_enable", set_vow_bss_en},


	{"vow_airtime_control_en", set_vow_airtime_ctrl_en},
	{"vow_bw_control_en", set_vow_bw_ctrl_en},

	/* group other */
	{"vow_bss_dwrr_quantum", set_vow_bss_dwrr_quantum},
	{"vow_group_dwrr_max_wait_time", set_vow_group_dwrr_max_wait_time},
	{"vow_group2band_map", set_vow_group2band_map},

	/* VOW STA table */
	{"vow_sta_dwrr_quantum", set_vow_sta_dwrr_quantum},
	{"vow_sta_dwrr_quantum_id", set_vow_sta_dwrr_quantum_id},
	{"vow_sta_ac_priority", set_vow_sta_ac_priority},
	{"vow_sta_pause", set_vow_sta_pause},
	{"vow_sta_psm", set_vow_sta_psm},
	{"vow_sta_group", set_vow_sta_group},
	{"vow_dwrr_max_wait_time", set_vow_dwrr_max_wait_time},

	/* STA fast round robin */
	{"vow_sta_frr_quantum", set_vow_sta_frr_quantum},

	/* USER */
	{"vow_min_rate", set_vow_min_rate},
	{"vow_max_rate", set_vow_max_rate},
	{"vow_min_ratio", set_vow_min_ratio},
	{"vow_max_ratio", set_vow_max_ratio},

	/* RX airtime */
	{"vow_rx_counter_clr", set_vow_rx_counter_clr},
	{"vow_rx_airtime_en", set_vow_rx_airtime_en},
	{"vow_rx_ed_offset", set_vow_rx_ed_offset},
	{"vow_rx_obss_backoff", set_vow_rx_obss_backoff},
	/* {"vow_rx_add_obss", set_vow_rx_add_obss}, */
	/* {"vow_rx_add_non_wifi", set_vow_rx_add_non_wifi}, */
	{"vow_rx_wmm_backoff", set_vow_rx_wmm_backoff},
	{"vow_om_wmm_backoff", set_vow_rx_om_wmm_backoff},
	{"vow_repeater_wmm_backoff", set_vow_rx_repeater_wmm_backoff},
	{"vow_rx_non_qos_backoff",  set_vow_rx_non_qos_backoff},
	{"vow_rx_bss_wmmset", set_vow_rx_bss_wmmset},
	{"vow_rx_om_wmm_sel", set_vow_rx_om_wmm_select},

	/* airtime estimator */
	{"vow_at_est_en", set_vow_at_est_en},
	{"vow_at_mon_period", set_vow_at_mon_period},

	/* badnode detector */
	{"vow_bn_en", set_vow_bn_en},
	{"vow_bn_mon_period", set_vow_bn_mon_period},
	{"vow_bn_fallback_th", set_vow_bn_fallback_th},
	{"vow_bn_per_th", set_vow_bn_per_th},

	/* airtime counter test */
	{"vow_counter_test", set_vow_counter_test_en},
	{"vow_counter_test_period", set_vow_counter_test_period},
	{"vow_counter_test_band", set_vow_counter_test_band},
	{"vow_counter_test_avgcnt", set_vow_counter_test_avgcnt},
	{"vow_counter_test_target", set_vow_counter_test_target},

	/* DVT */
	{"vow_dvt_en", set_vow_dvt_en},
	{"vow_monitor_sta", set_vow_monitor_sta},
	{"vow_show_sta", set_vow_show_sta},
	{"vow_monitor_bss", set_vow_monitor_bss},
	{"vow_monitor_mbss", set_vow_monitor_mbss},
	{"vow_show_mbss", set_vow_show_mbss},
	{"vow_avg_num", set_vow_avg_num},

	/*WATF*/
	{"vow_watf_en", set_vow_watf_en},
	{"vow_watf_q", set_vow_watf_q},
	{"vow_watf_add_entry", set_vow_watf_add_entry},
	{"vow_watf_del_entry", set_vow_watf_del_entry},


	/* help */
	{"vow_help", set_vow_help},



	/*
	{"vow_rx_add_obss", set_vow_rx_add_obss},
	{"vow_rx_add_non_wifi", set_vow_rx_add_non_wifi},
	*/
#endif /* VOW_SUPPORT */

#ifdef RED_SUPPORT
	{"red_en",						set_red_enable},
	{"red_show_sta",				set_red_show_sta},
	{"red_tar_delay",				set_red_target_delay},
	{"red_debug_en",				set_red_debug_enable},
	{"red_dump_reset",				set_red_dump_reset},
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	{"fq_en",                                              set_fq_enable},
	{"fq_debug_en",                                set_fq_debug_enable},
	{"fq_listmap",					set_fq_dbg_listmap},
	{"fq_linklist",					set_fq_dbg_linklist},
#endif /* RRSCH_SUPPORT */
	{"cp_support",					set_cp_support_en},
	{"RxvRecordEn",					SetRxvRecordEn},
	{"RxRateRecordEn",				SetRxRateRecordEn},
#ifdef CFG_SUPPORT_MU_MIMO_RA
	{"mura_periodic_sounding",      SetMuraPeriodicSndProc},         /* set trigger MURGA Algorithm sounding */
	{"mura_algorithm_test",         SetMuraTestAlgorithmProc},
	{"mura_algorithm_init",         SetMuraTestAlgorithmInit},
	{"mura_algorithm_fixed",        SetMuraFixedRateProc},
	{"mura_algorithm_fixed_group",  SetMuraFixedGroupRateProc},
	{"mura_sounding_fixed_param",   SetMuraFixedSndParamProc},
	{"mura_disabe_CN3_CN4",			SetMuraDisableCN3CN4Proc},
	{"mura_mobility_en",              SetMuraMobilityCtrlProc},
    {"mura_mobility_interval_ctrl",   SetMuraMobilityIntervalCtrlProc},
    {"mura_mobility_snr_ctrl",        SetMuraMobilitySNRCtrlProc},
    {"mura_mobility_threshold_ctrl",  SetMuraMobilityThresholdCtrlProc},
    {"mura_mobility_snd_cnt",         SetMuraMobilitySndCountProc},
    {"mura_mobility_mode_ctrl",       SetMuraMobilityModeCtrlProc},
    {"mura_mobility_log_ctrl",        SetMuraMobilityLogCtrlProc},
    {"mura_mobility_test_ctrl",       SetMuraMobilityTestCtrlProc},
#endif

#endif /* MT_MAC */
#ifdef BACKGROUND_SCAN_SUPPORT
	{"bgndscan",               set_background_scan},
	{"bgndscantest",		set_background_scan_test},
	{"bgndscannotify",		set_background_scan_notify},
	{"bgndscancfg",               set_background_scan_cfg},
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef NEW_SET_RX_STREAM
	{"RxStream",                        Set_RxStream_Proc},
#endif
#ifdef ERR_RECOVERY
	{"ErrDetectOn", Set_ErrDetectOn_Proc},
	{"ErrDetectMode", Set_ErrDetectMode_Proc},
#endif /* ERR_RECOVERY */
#ifdef CUT_THROUGH
	{"CtLowWaterMark", Set_CtLowWaterMark_Proc},
#endif /*CUT_THROUGH*/
#ifdef SMART_CARRIER_SENSE_SUPPORT
	{"SCSEnable", Set_SCSEnable_Proc},
	{"SCSCfg", Set_SCSCfg_Proc},
	{"SCSPd", Set_SCSPd_Proc},
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef BAND_STEERING
	{"BndStrgEnable",		Set_BndStrg_Enable},
	{"BndStrgBssIdx",		Set_BndStrg_BssIdx},
	{"BndStrgParam",    Set_BndStrg_Param},
#ifdef BND_STRG_DBG
	{"BndStrgMntAddr",	Set_BndStrg_MonitorAddr},
#endif /* BND_STRG_DBG */
#endif /* BAND_STEERING */

#ifdef TXPWRMANUAL
	{"TxPwrManualSet",  SetTxPwrManualCtrl},
#endif /* TXPWRMANUAL */

	{"SKUCtrl",	            SetSKUCtrl},
	{"PercentageCtrl",	    SetPercentageCtrl},
	{"PowerDropCtrl",	    SetPowerDropCtrl},
	{"BFBackoffCtrl",	    SetBfBackoffCtrl},
	{"ThermoCompCtrl",	    SetThermoCompCtrl},
	{"RFTxAnt",             SetRfTxAnt},
	{"TxPowerInfo",         SetTxPowerInfo},
	{"TOAECtrl",            SetTOAECtrl},
	{"EDCCACtrl",           SetEDCCACtrl},
	{"SKUInfo",             SetSKUInfo},
	{"BFBackoffInfo",       SetBFBackoffInfo},
	{"CCKTxStream",         SetCCKTxStream},
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{"ETSISetFixWbIbRssiCtrl",	SetFixWbIbRssiCtrl},
	{"ETSISetRssiThCtrl",		SetRssiThCtrl},
	{"ETSISetCheckThCtrl",		SetCheckThCtrl},
	{"ETSISetAdaptRxBlockCtrl", SetAdaptRxBlockCtrl},
    {"ETSISetWbRssiDirectCtrl",   SetWbRssiDirectCtrl},
    {"ETSISetIbRssiDirectCtrl",   SetIbRssiDirectCtrl},
#endif /* end ETSI_RX_BLOCKER_SUPPORT */
	{"MUTxPower",           SetMUTxPower},
	{"BFNDPATxDCtrl",       SetBFNDPATxDCtrl},
	{"TxPowerCompInfo",     SetTxPowerCompInfo},
	{"ThermalManualMode",   SetThermalManualCtrl},
#ifdef TX_POWER_CONTROL_SUPPORT
	{"TxPowerBoostCtrl",	SetTxPowerBoostCtrl},
#endif

#ifdef LED_CONTROL_SUPPORT
	{"led_setting", Set_Led_Proc},
#endif /* LED_CONTROL_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	{"ktoflash_debug", Set_KtoFlash_Debug_Proc},
	{"RDCE", Set_RDCE_Proc},
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
	{"hw_nat_register", set_hnat_register},
	{"MibBucket", Set_MibBucket_Proc},
#ifdef RLM_CAL_CACHE_SUPPORT
	{"rlm_cal_cache",  Set_RLM_Cal_Cache_Debug_Proc},
#endif /* RLM_CAL_CACHE_SUPPORT */
#ifdef PKT_BUDGET_CTRL_SUPPORT
	{"pbc_ubound", Set_PBC_Proc},
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
	{"bwf", Set_BWF_Enable_Proc},
#ifdef TX_AGG_ADJUST_WKR
	{"agg_adj_wkr", Set_AggAdjWkr_Proc},
#endif /* TX_AGG_ADJUST_WKR */
#ifdef HTC_DECRYPT_IOT
	{"htc_th",		Set_HTC_Err_TH_Proc},
	{"htc_entry_err_cnt",		Set_Entry_HTC_Err_Cnt_Proc},
	{"wtbl_addom",		Set_WTBL_AAD_OM_Proc},
#endif /* HTC_DECRYPT_IOT */
#ifdef DHCP_UC_SUPPORT
	{"dhcp_uc",		Set_DHCP_UC_Proc},
#endif /* DHCP_UC_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	{"hs_flag", Set_CR4_Hotspot_Flag},
#endif /* CONFIG_HOTSPOT_R2 */

	{"ser", set_ser},
#ifdef CONFIG_TX_DELAY
	{"tx_batch_cnt", Set_TX_Batch_Cnt_Proc},
	{"tx_delay_timeout", Set_TX_Delay_Timeout_Proc},
	{"tx_pkt_min_len", Set_Pkt_Min_Len_Proc},
	{"tx_pkt_max_len", Set_Pkt_Max_Len_Proc},
#endif
	{"fix_amsdu", set_fix_amsdu},
	{"rx_max_cnt", set_rx_max_cnt},
	{"rx1_max_cnt", set_rx1_max_cnt},
#ifdef MBO_SUPPORT
	{"mbo_nr",		SetMboNRIndicateProc},
#endif /* MBO_SUPPORT */
	{"rx_delay_ctl", set_rx_dly_ctl},
	{"tx_delay_ctl", set_tx_dly_ctl},
	{"vie_op", vie_oper_proc},
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	INT (*set_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
} *PRTMP_PRIVATE_SHOW_PROC, RTMP_PRIVATE_SHOW_SUPPORT_PROC[] = {
	{"stainfo",			    Show_MacTable_Proc},
#ifdef MEM_ALLOC_INFO_SUPPORT
	{"meminfo",			Show_MemInfo_Proc},
	{"pktmeminfo",		Show_PktInfo_Proc},
#endif /* MEM_ALLOC_INFO_SUPPORT */
#ifdef MT_MAC
	{"psinfo",			Show_PSTable_Proc},
	{"wtbl",				show_wtbl_proc},
	{"wtbltlv",				show_wtbltlv_proc},
	{"mibinfo", show_mib_proc},
	{"amsduinfo", show_amsdu_proc},
	{"wifi_sys", show_wifi_sys},
#ifdef DBDC_MODE
	{"dbdcinfo",		ShowDbdcProc},
#endif
	{"channelinfo", ShowChCtrl},
#ifdef GREENAP_SUPPORT
	{"greenapinfo",		ShowGreenAPProc},
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{"pcieaspminfo",		show_pcie_aspm_dym_ctrl_cap_proc},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
	{"txopinfo", show_tx_burst_info},
	{"tmacinfo", ShowTmacInfo},
	{"agginfo", ShowAggInfo},
	{"manual_txop", ShowManualTxOP},
	{"pseinfo", ShowPseInfo},
	{"psedata", ShowPseData},
	{"pleinfo",             ShowPLEInfo},
	{"txdinfo",             show_TXD_proc},
	{"dumpmem",             show_mem_proc},
	{"protectinfo", show_protect_info},
	{"ccainfo", show_cca_info},
#ifdef MT_FDB
	{"fdbn9log",            show_fdb_n9_log},
	{"fdbcr4log",           show_fdb_cr4_log},
#endif /* MT_FDB */
#ifdef CUT_THROUGH
	{"ctinfo",              ShowCutThroughInfo},
#endif /* CUT_THROUGH */

	{"dschinfo", show_dmasch_proc},

#endif /* MT_MAC */
	{"sta_tr",				Show_sta_tr_proc},
	{"peerinfo",			show_stainfo_proc},
	{"stacountinfo",			Show_StaCount_Proc},
	{"secinfo",			Show_APSecurityInfo_Proc},
	{"descinfo",			Show_DescInfo_Proc},
	{"driverinfo",			show_driverinfo_proc},
	{"apcfginfo",			show_apcfg_info},
	{"devinfo",			show_devinfo_proc},
	{"sysinfo",			show_sysinfo_proc},
	{"trinfo",	show_trinfo_proc},
	{"tpinfo",	            show_tpinfo_proc},
	{"pwrinfo", chip_show_pwr_info},
	{"txqinfo",			show_txqinfo_proc},
	{"swqinfo", show_swqinfo},
	{"efuseinfo",			    show_efuseinfo_proc},
	{"e2pinfo",			    show_e2pinfo_proc},
	{"cr4_pdma_probe",              dump_cr4_pdma_debug_probe},
#ifdef WDS_SUPPORT
	{"wdsinfo",				Show_WdsTable_Proc},
#endif /* WDS_SUPPORT */
#ifdef DOT11_N_SUPPORT
	{"bainfo",				Show_BaTable_Proc},
	{"channelset",				Show_ChannelSet_Proc},
#endif /* DOT11_N_SUPPORT */
	{"stat",				Show_Sat_Proc},
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
	{"diag",				Show_Diag_Proc},
#endif /* DBG_DIAGNOSE */
#endif /* RTMP_MAC_PCI */
	{"stat_reset",			Show_Sat_Reset_Proc},
#ifdef IGMP_SNOOP_SUPPORT
	{"igmpinfo",			Set_IgmpSn_TabDisplay_Proc},
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef MCAST_RATE_SPECIFIC
	{"mcastrate",			Show_McastRate},
#endif /* MCAST_RATE_SPECIFIC */
#ifdef MAT_SUPPORT
	{"matinfo",			Show_MATTable_Proc},
#endif /* MAT_SUPPORT */
#ifdef MT_DFS_SUPPORT
	{"DfsProvideChList",		Show_available_BwCh_Proc},
	{"DfsNOP",			Show_DfsNonOccupancy_Proc},
	{"DfsNOPOfChList", 		Show_NOP_Of_ChList},
	{"DfsTargetInfo",		Show_Target_Ch_Info},
#endif
#ifdef DOT11R_FT_SUPPORT
	{"ftinfo",				Show_FTConfig_Proc},
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	{"rrminfo",				RRM_InfoDisplay_Proc},
#endif /* DOT11K_RRM_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	{"qload",				Show_QoSLoad_Proc},
#endif /* AP_QLOAD_SUPPORT */
	{"TmrCal",				Show_TmrCalResult_Proc},
#ifdef FTM_SUPPORT
#ifdef FTM_INITIATOR
	{"FtmRxTmrQ",			Show_FtmRxTmrQ_Proc},
#endif /* FTM_INITIATOR */
	{"FtmEntry",			Show_FtmEntry_Proc},
	{"FtmPidList",			Show_FtmPidList_Proc},
	{"FtmLciValue",			Show_FtmLciValue_Proc},
#endif /* FTM_SUPPORT */
#ifdef APCLI_SUPPORT
	{"connStatus",			RTMPIoctlConnStatus},
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	{"reptinfo",			Show_Repeater_Cli_Proc},
	{"rept_table",			Show_ReptTable_Proc},
#endif /* MAC_REPEATER_SUPPORT */
#ifdef SMART_ANTENNA
	{"sainfo",				Show_SA_CfgInfo_Proc},
	{"sadbginfo",			Show_SA_DbgInfo_Proc},
#endif /* SMART_ANTENNA // */

	{"rainfo",				Show_RAInfo_Proc},
#ifdef TXBF_SUPPORT
	{"txbfinfo",			Show_TxBfInfo_Proc},
#endif /* TXBF_SUPPORT */
#ifdef MBSS_SUPPORT
	{"mbss",			Show_MbssInfo_Display_Proc},
#endif /* MBSS_SUPPORT */
#ifdef WSC_AP_SUPPORT
	{"WscPeerList",		WscApShowPeerList},
	{"WscPin",			WscApShowPin},
#endif /* WSC_AP_SUPPORT */
	{"rfinfo", ShowRFInfo},
	{"bbpinfo", ShowBBPInfo},
	{"wfintcnt", ShowWifiInterruptCntProc},
#ifdef CFG_SUPPORT_MU_MIMO
	{"hqa_mu_get_init_mcs",         hqa_mu_get_init_mcs},
	{"hqa_mu_get_qd",               hqa_mu_get_qd},
	{"hqa_mu_get_lq",               hqa_mu_get_lq},
	{"hqa_su_get_lq",               hqa_su_get_lq},

	{"get_mu_enable",               ShowMuEnableProc},                  /* show mu enable or disable */
	{"get_mu_profile",              ShowMuProfileProc},                 /* show mu profile entry */
	{"get_mu_grouptbl",             ShowGroupTblEntryProc},             /* show group table entry */
	{"get_mu_clustertbl",           ShowClusterTblEntryProc},           /* show cluster table entry */
	{"get_mu_groupuserthreshold",   ShowGroupUserThresholdProc},        /* show group user threshold */
	{"get_mu_groupnssthreshold",    ShowGroupNssThresholdProc},         /* show group NSS threshold */
	{"get_mu_txreqmintime",         ShowTxReqMinTimeProc},              /* show tx req. min. time */
	{"get_mu_sunsscheck",           ShowSuNssCheckProc},                /* show SU Nss check enable or disable */
	{"get_mu_calcinitmcs",          ShowCalcInitMCSProc},               /* show Init MCS */
	{"get_mu_txopdefault",          ShowTxopDefaultProc},               /* show TXOP default */
	{"get_mu_sulossthreshold",      ShowSuLossThresholdProc},           /* show SU loss threshold */
	{"get_mu_mugainthreshold",      ShowMuGainThresholdProc},           /* show MU gain threshold */
	{"get_mu_secondaryacpolicy",    ShowSecondaryAcPolicyProc},         /* show secondary AC policay */
	{"get_mu_grouptbldmcsmask",     ShowGroupTblDmcsMaskProc},          /* show group table DMCS mask enable or disable */
	{"get_mu_maxgroupsearchcnt",    ShowMaxGroupSearchCntProc},         /* show max. group table search count */
	{"get_mu_txstatus",             ShowMuProfileTxStsCntProc},         /* show mu profile tx status */
#endif

#ifdef CFG_SUPPORT_MU_MIMO_RA
	{"mura_algorithm_monitor",  GetMuraMonitorStateProc},
	{"get_mura_pfid_stat",      GetMuraPFIDStatProc},
#endif

	{"hwctrl", Show_HwCtrlStatistic_Proc},

#ifdef VOW_SUPPORT
	/* VOW RX */
	{"vow_rx_time", show_vow_rx_time},
	/* {"vow_get_sta_token", show_vow_get_sta_token}, */
	{"vow_sta_conf", show_vow_sta_conf},
	{"vow_all_sta_conf", show_vow_all_sta_conf},
	{"vow_bss_conf", show_vow_bss_conf},
	{"vow_all_bss_conf", show_vow_all_bss_conf},
	{"vow_info", show_vow_info},

	/* {"vow_status", show_vow_status} */

	/* CR dump */
	{"vow_dump_sta", show_vow_dump_sta},
	{"vow_dump_bss_bitmap", show_vow_dump_bss_bitmap},
	{"vow_dump_bss", show_vow_dump_bss},
	{"vow_dump_vow", show_vow_dump_vow},
	{"vow_show_sta_dtoken", vow_show_sta_dtoken},
	{"vow_show_bss_dtoken", vow_show_bss_dtoken},
	{"vow_show_bss_atoken", vow_show_bss_atoken},
	{"vow_show_bss_ltoken", vow_show_bss_ltoken},

	/* DVT */
	{"vow_show_queue", vow_show_queue_status},

	/*WATF*/
	{"vow_watf_info", show_vow_watf_info},

	/* help */
	{"vow_help", show_vow_help},
#endif /* VOW_SUPPORT */
#ifdef RED_SUPPORT
	{"red_info", show_red_info},
#endif/* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	{"fq_info", show_fq_info},
#endif
	{"timer_list", show_timer_list},
	{"wtbl_stat", show_wtbl_state},
#ifdef SMART_CARRIER_SENSE_SUPPORT
	{"SCSInfo", Show_SCSinfo_proc},
#endif /* SMART_CARRIER_SENSE_SUPPORT */
	{"MibBucket",       Show_MibBucket_Proc},
#ifdef REDUCE_TCP_ACK_SUPPORT
	{"ReduceAckShow",       Show_ReduceAckInfo_Proc},
#endif
	{"EDCCAStatus",     ShowEDCCAStatus},
#ifdef BAND_STEERING
	{"BndStrgList",		Show_BndStrg_List},
	{"BndStrgInfo",		Show_BndStrg_Info},
#ifdef VENDOR_FEATURE5_SUPPORT
	{"BndStrgNvramtable",		Show_BndStrg_NvramTable},
#endif /* VENDOR_FEATURE5_SUPPORT */
#endif /* BAND_STEERING */
	{"radio_info", show_radio_info_proc},
#ifdef BACKGROUND_SCAN_SUPPORT
	{"bgndscaninfo", show_background_scan_info},
#endif
#ifdef ERR_RECOVERY
	{"serinfo",		ShowSerProc},
	{"ser",			ShowSerProc2},
#endif
	{"bcninfo",		ShowBcnProc},
#ifdef RADIUS_MAC_ACL_SUPPORT
	{"RadiusAclCache",      show_RADIUS_acl_cache},
#endif /* RADIUS_MAC_ACL_SUPPORT */
#ifdef DBG_STARVATION
	{"starv_info", show_starv_info_proc},
#endif /*DBG_STARVATION*/
#ifdef TX_POWER_CONTROL_SUPPORT
	{"TxPowerBoostInfo",	ShowTxPowerBoostInfo},
#endif /* TX_POWER_CONTROL_SUPPORT */
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{"ETSIShowRssiThInfo",		        ShowRssiThInfo},
#endif /* ETSI_RX_BLOCKER_SUPPORT */
	{"qdisc_dump", set_qiscdump_proc},
#ifdef MBO_SUPPORT
	{"mbo",			ShowMboStatProc},
#endif /* MBO_SUPPORT */
	{"l1profile",			ShowL1profile},
	{NULL,}
};

/**
 * @addtogroup embedded_ioctl
 * @{
 * @code AP RX IOCTL
 */

static struct {
	RTMP_STRING *name;
	INT (*rx_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
} *PRTMP_PRIVATE_RX_PROC, RTMP_PRIVATE_RX_SUPPORT_PROC[] = {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	{"stat",          Show_Rx_Statistic},
	{"vector",      Set_Rx_Vector_Control},
#endif/* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	{NULL,}
};


INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	RTMP_STRING *this_char, *value;
	INT Status = NDIS_STATUS_SUCCESS;

	UCHAR *tmp = NULL, *buf = NULL;

	os_alloc_mem(NULL, (UCHAR **)&buf, pIoctlCmdStr->u.data.length + 1);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, pIoctlCmdStr->u.data.pointer, pIoctlCmdStr->u.data.length)) {
		os_free_mem(buf);
		return -EFAULT;
	}
	/* Play safe - take care of a situation in which user-space didn't NULL terminate */
	buf[pIoctlCmdStr->u.data.length] = 0;

	/* Use tmp to parse string, because strsep() would change it */
	tmp = buf;

	while ((this_char = strsep((char **)&tmp, ",")) != NULL) {

		if (!*this_char)
			continue;

		value = strchr(this_char, '=');

		if (value != NULL)
			*value++ = 0;

		if (!value
#ifdef WSC_AP_SUPPORT
			&& (
				(strcmp(this_char, "WscStop") != 0) &&
				(strcmp(this_char, "ser") != 0) &&
#ifdef BB_SOC
				(strcmp(this_char, "WscResetPinCode") != 0) &&
#endif
				(strcmp(this_char, "WscGenPinCode") != 0)
			)
#endif /* WSC_AP_SUPPORT */
#ifdef SMART_ANTENNA
			&& (strcmp(this_char, "sa") != 0)
#endif /* SMART_ANTENNA */
		   )
			continue;

		for (PRTMP_PRIVATE_SET_PROC = RTMP_PRIVATE_SUPPORT_PROC; PRTMP_PRIVATE_SET_PROC->name; PRTMP_PRIVATE_SET_PROC++) {
			if (!strcasecmp(this_char, PRTMP_PRIVATE_SET_PROC->name)) {
				if (!PRTMP_PRIVATE_SET_PROC->set_proc(pAd, value)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}

				break;  /*Exit for loop. */
			}
		}

		if (PRTMP_PRIVATE_SET_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char,
					 value));
			break;
		}
	}
	os_free_mem(buf);

	return Status;
}


INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	RTMP_STRING *this_char, *value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

	UCHAR *tmp = NULL, *buf = NULL;

	os_alloc_mem(NULL, (UCHAR **)&buf, pIoctlCmdStr->u.data.length + 1);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, pIoctlCmdStr->u.data.pointer, pIoctlCmdStr->u.data.length)) {
		os_free_mem(buf);
		return -EFAULT;
	}
	/* Play safe - take care of a situation in which user-space didn't NULL terminate */
	buf[pIoctlCmdStr->u.data.length] = 0;

	/* Use tmp to parse string, because strsep() would change it */
	tmp = buf;

	while ((this_char = strsep((char **)&tmp, ",")) != NULL) {
		if (!*this_char)
			continue;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Before check, this_char=%s\n", __func__, this_char));
		value = strchr(this_char, '=');

		if (value) {
			if (strlen(value) > 1) {
				*value = 0;
				value++;
			} else
				value = NULL;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): after check, this_char=%s, value=%s\n", __func__,
				 this_char, (value == NULL ? "" : value)));

		for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name;
			 PRTMP_PRIVATE_SHOW_PROC++) {
			if (!strcasecmp(this_char, PRTMP_PRIVATE_SHOW_PROC->name)) {
				if (!PRTMP_PRIVATE_SHOW_PROC->set_proc(pAd, value)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}

				break;  /*Exit for loop. */
			}
		}

		if (PRTMP_PRIVATE_SHOW_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
#ifdef RTMP_RBUS_SUPPORT

			if (pAd->infType == RTMP_DEV_INF_RBUS) {
				for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name; PRTMP_PRIVATE_SHOW_PROC++)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", PRTMP_PRIVATE_SHOW_PROC->name));
			}

#endif /* RTMP_RBUS_SUPPORT */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char,
					 value));
			break;
		}
	}

	os_free_mem(buf);
	return Status;
}

#ifdef VENDOR_FEATURE6_SUPPORT
#define	ASSO_MAC_LINE_LEN	(1+19+4+4+4+4+8+7+7+7+7+10+6+6+6+6+7+7+7+1)
VOID RTMPAPGetAssoMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	UINT32 DataRate = 0;
	INT i;
	char *msg;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (MAX_LEN_OF_MAC_TABLE * ASSO_MAC_LINE_LEN));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		return;
	}

	memset(msg, 0, MAX_LEN_OF_MAC_TABLE * ASSO_MAC_LINE_LEN);
	sprintf(msg + strlen(msg), "\n%-19s%-4s%-4s%-4s%-4s%-8s",
			"MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS");
	sprintf(msg + strlen(msg), "%-7s%-7s%-7s%-7s", "RSSI0", "RSSI1", "RSSI2", "RSSI3");
	sprintf(msg + strlen(msg), "%-10s%-6s%-6s%-6s%-6s%-7s%-7s%-7s\n", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate", "TIME");

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry) || (IS_ENTRY_APCLI(pEntry)
#ifdef MAC_REPEATER_SUPPORT
			 || IS_ENTRY_REPEATER(pEntry)
#endif /* MAC_REPEATER_SUPPORT */
										))
			&& (pEntry->Sst == SST_ASSOC)) {
			if ((strlen(msg) + ASSO_MAC_LINE_LEN) >= (MAX_LEN_OF_MAC_TABLE * ASSO_MAC_LINE_LEN))
				break;

			DataRate = 0;
			DataRate = 0;
			/* getRate(pEntry->HTPhyMode, &DataRate); */
			RtmpDrvMaxRateGet(pAd, pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.ShortGI,
							  pEntry->HTPhyMode.field.BW, pEntry->HTPhyMode.field.MCS,
							  (pEntry->MaxHTPhyMode.field.MCS >> 4) + 1, (UINT32 *)&DataRate);
			DataRate /= 500000;
			sprintf(msg + strlen(msg), "%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr));
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->Aid);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->apidx);
			sprintf(msg + strlen(msg), "%-4d", (int)pEntry->PsMode);
			sprintf(msg + strlen(msg), "%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			sprintf(msg + strlen(msg), "%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
			sprintf(msg + strlen(msg), "%-7d%-7d%-7d%-7d", pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
			sprintf(msg + strlen(msg), "%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			sprintf(msg + strlen(msg), "%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

			if (pEntry->HTPhyMode.field.MODE == MODE_VHT)
				sprintf(msg + strlen(msg), "%dS-M%-3d", ((pEntry->HTPhyMode.field.MCS >> 4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
			else
#endif /* DOT11_VHT_AC */
				sprintf(msg + strlen(msg), "%-6d", pEntry->HTPhyMode.field.MCS);

			sprintf(msg + strlen(msg), "%-6d", pEntry->HTPhyMode.field.ShortGI);
			sprintf(msg + strlen(msg), "%-6d", pEntry->HTPhyMode.field.STBC);
			sprintf(msg + strlen(msg), "%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			sprintf(msg + strlen(msg), "%-7d", (int)DataRate);
			sprintf(msg + strlen(msg), "%-7d", (int)pEntry->StaConnectTime);
			sprintf(msg + strlen(msg), "%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0);
			sprintf(msg + strlen(msg), "\n");
		}
	}

	/* for compatible with old API just do the printk to console*/
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));

	os_free_mem(msg);
}
#endif /* VENDOR_FEATURE6_SUPPORT */

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	INT Status = NDIS_STATUS_SUCCESS;

	if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_mac_table"))
		RTMPAR9IoctlGetMacTable(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_stat2"))
		RTMPIoctlGetSTAT2(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_radio_dyn_info"))
		RTMPIoctlGetRadioDynInfo(pAd, pIoctlCmdStr);

#ifdef WSC_AP_SUPPORT
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_profile"))
		RTMPAR9IoctlWscProfile(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_pincode"))
		RTMPIoctlWscPINCode(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_status"))
		RTMPIoctlWscStatus(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wps_dyn_info"))
		RTMPIoctlGetWscDynInfo(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wps_regs_dyn_info"))
		RTMPIoctlGetWscRegsDynInfo(pAd, pIoctlCmdStr);

#endif
	return Status;
}
#endif /*AR9_MAPI_SUPPORT*/
#endif/*AR9_INF*/

INT RTMPAPSetInformation(
	IN PRTMP_ADAPTER pAd,
	INOUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd)
{
	RTMP_IOCTL_INPUT_STRUCT *wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
	UCHAR Addr[MAC_ADDR_LEN];
	INT Status = NDIS_STATUS_SUCCESS;
#ifdef SNMP_SUPPORT
	/*snmp */
	UINT						KeyIdx = 0;
	PNDIS_AP_802_11_KEY			pKey = NULL;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						ctmp;
#endif /* SNMP_SUPPORT */
	NDIS_802_11_WEP_STATUS              WepStatus;
	NDIS_802_11_AUTHENTICATION_MODE     AuthMode = Ndis802_11AuthModeMax;
	NDIS_802_11_SSID                    Ssid;
#ifdef HOSTAPD_SUPPORT
	MAC_TABLE_ENTRY						*pEntry;
	STA_TR_ENTRY *tr_entry;
	struct ieee80211req_mlme			mlme;
	struct ieee80211req_key				Key;
	struct ieee80211req_del_key			delkey;
	UINT8 Wcid;
	BSS_STRUCT *pMbss;
	WSC_LV_INFO            WscIEBeacon;
	WSC_LV_INFO            WscIEProbeResp;
	int i;
#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	switch (cmd & 0x7FFF) {
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/

	case OID_802_11_DEAUTHENTICATION:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_DEAUTHENTICATION\n"));

		if (wrq->u.data.length != sizeof(MLME_DEAUTH_REQ_STRUCT))
			Status  = -EINVAL;
		else {
			MAC_TABLE_ENTRY *pEntry = NULL;
			MLME_DEAUTH_REQ_STRUCT *pInfo = NULL;
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

			if (Elem == NULL) {
				Status = -ENOMEM;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_DEAUTHENTICATION, Failed!!\n"));
				break;
			}

#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
			{
				if (Elem) {
					pInfo = (MLME_DEAUTH_REQ_STRUCT *) Elem->Msg;
					Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);
					pEntry = MacTableLookup(pAd, pInfo->Addr);

					if (pEntry != NULL) {
						Elem->Wcid = pEntry->wcid;
						MlmeEnqueue(pAd, AP_AUTH_STATE_MACHINE, APMT2_MLME_DEAUTH_REQ,
									sizeof(MLME_DEAUTH_REQ_STRUCT), Elem, 0);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->Reason));
					}

					os_free_mem(Elem);
				}
			}
		}

		break;
#ifdef IAPP_SUPPORT

	case RT_SET_IAPP_PID: {
		unsigned IappPid;

		if (sizeof(IappPid) != wrq->u.data.length) {
			Status = -EFAULT;
			break;
		}

		if (copy_from_user(&IappPid, wrq->u.data.pointer, wrq->u.data.length))
			Status = -EFAULT;
		else {
			RTMP_GET_OS_PID(pObj->IappPid, IappPid);
			pObj->IappPid_nr = IappPid;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_SET_APD_PID::(IappPid=%u)\n", IappPid));
		}
	}
	break;
#endif /* IAPP_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	case RT_SET_FT_STATION_NOTIFY:
	case RT_SET_FT_KEY_REQ:
	case RT_SET_FT_KEY_RSP:
	case RT_FT_KEY_SET:
	case RT_FT_NEIGHBOR_REPORT:
	case RT_FT_NEIGHBOR_REQUEST:
	case RT_FT_NEIGHBOR_RESPONSE:
	case RT_FT_ACTION_FORWARD: {
		UCHAR *pBuffer;

		FT_MEM_ALLOC(pAd, &pBuffer, wrq->u.data.length + 1);

		if (pBuffer == NULL)
			break;

		if (copy_from_user(pBuffer, wrq->u.data.pointer, wrq->u.data.length)) {
			Status = -EFAULT;
			FT_MEM_FREE(pAd, pBuffer);
			break;
		}

		switch (cmd & 0x7FFF) {
		case RT_SET_FT_STATION_NOTIFY:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_SET_FT_STATION_NOTIFY\n"));
			FT_KDP_StationInform(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_SET_FT_KEY_REQ:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_SET_FT_KEY_REQ\n"));
			FT_KDP_IOCTL_KEY_REQ(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_SET_FT_KEY_RSP:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_SET_FT_KEY_RSP\n"));
			FT_KDP_KeyResponseToUs(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_KEY_SET:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_FT_KEY_SET\n"));
			/* Note: the key must be ended by 0x00 */
			pBuffer[wrq->u.data.length] = 0x00;
			FT_KDP_CryptKeySet(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_NEIGHBOR_REPORT:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_FT_NEIGHBOR_REPORT\n"));
#ifdef FT_KDP_FUNC_INFO_BROADCAST
			FT_KDP_NeighborReportHandle(pAd, pBuffer, wrq->u.data.length);
#endif /* FT_KDP_FUNC_INFO_BROADCAST */
			break;

		case RT_FT_NEIGHBOR_REQUEST:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_FT_NEIGHBOR_REPORT\n"));
			FT_KDP_NeighborRequestHandle(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_NEIGHBOR_RESPONSE:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_FT_NEIGHBOR_RESPONSE\n"));
			FT_KDP_NeighborResponseHandle(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_ACTION_FORWARD:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[ra%d] RT_FT_ACTION_FORWARD\n", pObj->ioctl_if));
			FT_RRB_ActionHandle(pAd, pObj->ioctl_if, pBuffer, wrq->u.data.length);
			break;
		}

		FT_MEM_FREE(pAd, pBuffer);
	}
	break;

	case OID_802_11R_SUPPORT:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;
			ULONG value;

			Status = copy_from_user(&value, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable = (value == 0 ? FALSE : TRUE);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11R_SUPPORT(=%d)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable));
		}

		break;

	case OID_802_11R_MDID:
		if (wrq->u.data.length != FT_MDID_LEN)
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;

			Status = copy_from_user(pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtMdId, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11R_MDID(=%c%c)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtMdId[0],
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtMdId[0]));
			/*#ifdef WH_EZ_SETUP
				Dynamic update of MdId in ez security Info not supported currently
			#endif */
		}

		break;

	case OID_802_11R_R0KHID:
		if (wrq->u.data.length <= FT_ROKH_ID_LEN)
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;

			Status = copy_from_user(pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khId, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khIdLen = wrq->u.data.length;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11R_OID_802_11R_R0KHID(=%s) Len=%d\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khId,
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khIdLen));
		}

		break;

	case OID_802_11R_RIC:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;
			ULONG value;

			Status = copy_from_user(&value, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.RsrReqCap = (value == 0 ? FALSE : TRUE);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11R_RIC(=%d)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable));
		}

		break;

	case OID_802_11R_OTD:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;
			ULONG value;

			Status = copy_from_user(&value, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.FtOverDs = (value == 0 ? FALSE : TRUE);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11R_OTD(=%d)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable));
		}

		break;
#endif /* DOT11R_FT_SUPPORT */

	case RT_SET_APD_PID: {
		unsigned long apd_pid;

		if (sizeof(apd_pid) != wrq->u.data.length) {
			Status = -EFAULT;
			break;
		}

		if (copy_from_user(&apd_pid, wrq->u.data.pointer, wrq->u.data.length))
			Status = -EFAULT;
		else {
			RTMP_GET_OS_PID(pObj->apd_pid, apd_pid);
			pObj->apd_pid_nr = apd_pid;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_SET_APD_PID::(ApdPid=%lu)\n", apd_pid));
		}
	}
	break;

	case RT_SET_DEL_MAC_ENTRY:
		if (wrq->u.data.length != MAC_ADDR_LEN) {
			Status = -EFAULT;
			break;
		}

		if (copy_from_user(Addr, wrq->u.data.pointer, wrq->u.data.length))
			Status = -EFAULT;
		else {
			MAC_TABLE_ENTRY *pEntry = NULL;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_SET_DEL_MAC_ENTRY::(%02x:%02x:%02x:%02x:%02x:%02x)\n",
					 Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));

			if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
				struct wifi_dev *wdev = NULL;

				wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
				pEntry = MacTableLookup2(pAd, Addr, wdev);

			} else
			pEntry = MacTableLookup(pAd, Addr);

			if (pEntry) {
#ifdef MAC_REPEATER_SUPPORT

				/*
					Need to delete repeater entry if this is mac repeater entry.
				*/
				if (pAd->ApCfg.bMACRepeaterEn) {
					UCHAR apCliIdx, CliIdx;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEntry->Addr, TRUE);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("Delete (%02x:%02x:%02x:%02x:%02x:%02x) mac repeater entry\n",
							  Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));

					if (pReptEntry) {
						apCliIdx = pReptEntry->MatchApCliIdx;
						CliIdx = pReptEntry->MatchLinkIdx;
						pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCFG_DEL_MAC_ENTRY;
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL,
									(64 + MAX_EXT_MAC_ADDR_SIZE * apCliIdx + CliIdx));
						RTMP_MLME_HANDLER(pAd);
					}
				}

#endif /* MAC_REPEATER_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

				/*
					If AP send de-auth to Apple STA,
					Apple STA will re-do auth/assoc and security handshaking with AP again.
					@20150313
				*/
				if (IS_FT_RSN_STA(pEntry))
					MacTableDeleteEntry(pAd, pEntry->Aid, Addr);
				else
#endif /* DOT11R_FT_SUPPORT */
					MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
			}
		}

		break;
#ifdef WSC_AP_SUPPORT
#ifdef CON_WPS

	case RT_OID_WSC_SET_CON_WPS_STOP: {
		UCHAR       apidx = pObj->ioctl_if;
		PWSC_UPNP_CTRL_WSC_BAND_STOP pWscUpnpBandStop;
		PWSC_CTRL pWpsCtrl = NULL;
		INT         IsAPConfigured;

		os_alloc_mem(NULL, (UCHAR **)&pWscUpnpBandStop, sizeof(WSC_UPNP_CTRL_WSC_BAND_STOP));

		if (pWscUpnpBandStop) {
			Status = copy_from_user(pWscUpnpBandStop, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CON_WPS BAND_STOP_CMD From[%s], isApCli[%d], is2gBand[%d]\n",
					 pWscUpnpBandStop->ifName, pWscUpnpBandStop->isApCli, pWscUpnpBandStop->is2gBand));

			if (pWscUpnpBandStop->isApCli) {
				UCHAR i;
				struct wifi_dev *apcli_wdev;

				for (i = 0; i < MAX_APCLI_NUM; i++) {
					pWpsCtrl = &pAd->ApCfg.ApCliTab[i].wdev.WscControl;
					apcli_wdev = &pAd->ApCfg.ApCliTab[i].wdev;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CON_WPS FROM IOCTL: Stop the %s WPS, state [%d]\n",
							 apcli_wdev->if_dev->name, pWpsCtrl->WscState));

					if (pWpsCtrl->WscState != WSC_STATE_OFF) {
						WscStop(pAd, TRUE, pWpsCtrl);
						pWpsCtrl->WscConfMode = WSC_DISABLE;
					}
				}
			} else {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

				pWpsCtrl = &wdev->WscControl;
				IsAPConfigured = pWpsCtrl->WscConfStatus;

				if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					(pWpsCtrl->bWscTrigger == TRUE)) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("FROM IOCTL CON_WPS[%d]: Stop the AP Wsc Machine\n", apidx));
					WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
					WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0,
										apidx, NULL, 0, AP_MODE);
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
					WscStop(pAd, FALSE, pWpsCtrl);
				}
			}

			os_free_mem(pWscUpnpBandStop);
		}
	}
	break;
#endif /* CON_WPS */

	case RT_OID_WSC_SET_SELECTED_REGISTRAR: {
		PUCHAR      upnpInfo;
		UCHAR	    apidx = pObj->ioctl_if;
#ifdef HOSTAPD_SUPPORT

		if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
			Status = -EINVAL;
		} else {
#endif /*HOSTAPD_SUPPORT*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("WSC::RT_OID_WSC_SET_SELECTED_REGISTRAR, wrq->u.data.length=%d!\n", wrq->u.data.length));
			os_alloc_mem(pAd, (UCHAR **)&upnpInfo, wrq->u.data.length);

			if (upnpInfo) {
				int len, Status;

				Status = copy_from_user(upnpInfo, wrq->u.data.pointer, wrq->u.data.length);

				if (Status == NDIS_STATUS_SUCCESS) {
					len = wrq->u.data.length;

					if ((pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfMode & WSC_PROXY)) {
						WscSelectedRegistrar(pAd, upnpInfo, len, apidx);

						if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimerRunning == TRUE) {
							BOOLEAN Cancelled;

							RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimer, &Cancelled);
						}

						/* 2mins time-out timer */
						RTMPSetTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
						pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimerRunning = TRUE;
					}
				}

				os_free_mem(upnpInfo);
			} else
				Status = -EINVAL;

#ifdef HOSTAPD_SUPPORT
		}

#endif /*HOSTAPD_SUPPORT*/
	}
	break;

	case RT_OID_WSC_EAPMSG: {
		RTMP_WSC_U2KMSG_HDR *msgHdr = NULL;
		PUCHAR pUPnPMsg = NULL;
		UINT msgLen = 0, Machine = 0, msgType = 0;
		int retVal, senderID = 0;
		struct wifi_dev *wdev;
#ifdef HOSTAPD_SUPPORT
		UCHAR	    apidx = pObj->ioctl_if;

		if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
			Status = -EINVAL;
		} else {
#endif /*HOSTAPD_SUPPORT*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WSC::RT_OID_WSC_EAPMSG, wrq->u.data.length=%d, ioctl_if=%d\n",
					 wrq->u.data.length, pObj->ioctl_if));
			msgLen = wrq->u.data.length;
			os_alloc_mem(pAd, (UCHAR **)&pUPnPMsg, msgLen);

			if (pUPnPMsg == NULL)
				Status = -EINVAL;
			else {
				int HeaderLen;
				RTMP_STRING *pWpsMsg;
				UINT WpsMsgLen;
				PWSC_CTRL pWscControl;
				BOOLEAN	bGetDeviceInfo = FALSE;

				NdisZeroMemory(pUPnPMsg, msgLen);
				retVal = copy_from_user(pUPnPMsg, wrq->u.data.pointer, msgLen);
				msgHdr = (RTMP_WSC_U2KMSG_HDR *)pUPnPMsg;
				wdev = wdev_search_by_address(pAd, &msgHdr->Addr1[0]);

				if (!wdev) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s:: cannot find wdev by addr(%02x:%02x:%02x:%02x:%02x:%02x).\n",
							  __func__, PRINT_MAC(msgHdr->Addr1)));
					os_free_mem(pUPnPMsg);
					Status = -EINVAL;
					break;
				}

				senderID = get_unaligned((INT32 *)(&msgHdr->Addr2[0]));
				/*senderID = *((int *)&msgHdr->Addr2); */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_EAPMSG++++++++\n\n"));
				hex_dump("MAC::", &msgHdr->Addr3[0], MAC_ADDR_LEN);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_EAPMSG++++++++\n\n"));
				HeaderLen = LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);
				pWpsMsg = (RTMP_STRING *)&pUPnPMsg[HeaderLen];
				WpsMsgLen = msgLen - HeaderLen;
				/*assign the STATE_MACHINE type */
				Machine = WSC_STATE_MACHINE;
				msgType = WSC_EAPOL_UPNP_MSG;
				pWscControl = &wdev->WscControl;

				/* If AP is unconfigured, WPS state machine will be triggered after received M2. */
				if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
#ifdef WSC_V2_SUPPORT
					&& (pWscControl->WscV2Info.bWpsEnable || (pWscControl->WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
				   ) {
					if (strstr(pWpsMsg, "SimpleConfig") &&
						!pWscControl->EapMsgRunning &&
						!pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
						/* GetDeviceInfo */
						WscInit(pAd, FALSE, pObj->ioctl_if);
						/* trigger wsc re-generate public key */
						pWscControl->RegData.ReComputePke = 1;
						bGetDeviceInfo = TRUE;
					} else if (WscRxMsgTypeFromUpnp(pAd, pWpsMsg, WpsMsgLen) == WSC_MSG_M2 &&
							   !pWscControl->EapMsgRunning &&
							   !pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
						/* Check Enrollee Nonce of M2 */
						if (WscCheckEnrolleeNonceFromUpnp(pAd, pWpsMsg, WpsMsgLen, pWscControl)) {
							WscGetConfWithoutTrigger(pAd, pWscControl, TRUE);
							pWscControl->WscState = WSC_STATE_SENT_M1;
						}
					}
				}

				retVal = MlmeEnqueueForWsc(pAd, msgHdr->envID, senderID, Machine, msgType, msgLen, pUPnPMsg, wdev);

				if ((retVal == FALSE) && (msgHdr->envID != 0)) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeEnqueuForWsc return False and envID=0x%x!\n",
							 msgHdr->envID));
					Status = -EINVAL;
				}

				os_free_mem(pUPnPMsg);
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_EAPMSG finished!\n"));
#ifdef HOSTAPD_SUPPORT
		}

#endif /*HOSTAPD_SUPPORT*/
	}
	break;

	case RT_OID_WSC_READ_UFD_FILE:
		if (wrq->u.data.length > 0) {
			RTMP_STRING *pWscUfdFileName = NULL;
			UCHAR apIdx = pObj->ioctl_if;
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apIdx];
			WSC_CTRL *pWscCtrl = &pMbss->wdev.WscControl;

			os_alloc_mem(pAd, (UCHAR **)&pWscUfdFileName, wrq->u.data.length + 1);

			if (pWscUfdFileName) {
				RTMPZeroMemory(pWscUfdFileName, wrq->u.data.length + 1);

				if (copy_from_user(pWscUfdFileName, wrq->u.data.pointer, wrq->u.data.length))
					Status = -EFAULT;
				else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_READ_UFD_FILE (WscUfdFileName=%s)\n",
							 pWscUfdFileName));

					if (pWscCtrl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) {
						if (WscReadProfileFromUfdFile(pAd, apIdx, pWscUfdFileName)) {
							pWscCtrl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
							APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
							APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);
						}
					} else {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_READ_UFD_FILE: AP is configured.\n"));
						Status = -EINVAL;
					}
				}

				os_free_mem(pWscUfdFileName);
			} else
				Status = -ENOMEM;
		} else
			Status = -EINVAL;

		break;

	case RT_OID_WSC_WRITE_UFD_FILE:
		if (wrq->u.data.length > 0) {
			RTMP_STRING *pWscUfdFileName = NULL;
			UCHAR apIdx = pObj->ioctl_if;
			WSC_CTRL *pWscCtrl = &pAd->ApCfg.MBSSID[apIdx].wdev.WscControl;

			os_alloc_mem(pAd, (UCHAR **)&pWscUfdFileName, wrq->u.data.length + 1);

			if (pWscUfdFileName) {
				RTMPZeroMemory(pWscUfdFileName, wrq->u.data.length + 1);

				if (copy_from_user(pWscUfdFileName, wrq->u.data.pointer, wrq->u.data.length))
					Status = -EFAULT;
				else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_WRITE_UFD_FILE (WscUfdFileName=%s)\n",
							 pWscUfdFileName));

					if (pWscCtrl->WscConfStatus == WSC_SCSTATE_CONFIGURED)
						WscWriteProfileToUfdFile(pAd, apIdx, pWscUfdFileName);
					else {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_WSC_WRITE_UFD_FILE: AP is un-configured.\n"));
						Status = -EINVAL;
					}
				}

				os_free_mem(pWscUfdFileName);
			} else
				Status = -ENOMEM;
		} else
			Status = -EINVAL;

		break;

	case RT_OID_WSC_UUID:
		if (wrq->u.data.length == (UUID_LEN_STR - 1)) {
			UCHAR		apIdx = pObj->ioctl_if;

			pAd->ApCfg.MBSSID[apIdx].wdev.WscControl.Wsc_Uuid_Str[0] = '\0';
			Status = copy_from_user(&pAd->ApCfg.MBSSID[apIdx].wdev.WscControl.Wsc_Uuid_Str[0],
									wrq->u.data.pointer,
									wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("UUID ASCII string: %s\n",
					 pAd->ApCfg.MBSSID[apIdx].wdev.WscControl.Wsc_Uuid_Str));
		} else if (wrq->u.data.length == UUID_LEN_HEX) {
			UCHAR		apIdx = pObj->ioctl_if, ii;

			Status = copy_from_user(&pAd->ApCfg.MBSSID[apIdx].wdev.WscControl.Wsc_Uuid_E[0],
									wrq->u.data.pointer,
									wrq->u.data.length);

			for (ii = 0; ii < 16; ii++)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x",
						 (pAd->ApCfg.MBSSID[apIdx].wdev.WscControl.Wsc_Uuid_E[ii] & 0xff)));
		} else
			Status = -EINVAL;

		break;
#endif /* WSC_AP_SUPPORT */
#ifdef SNMP_SUPPORT

	case OID_802_11_SHORTRETRYLIMIT:
		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&ShortRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
			AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_SHORTRETRYLIMIT (ShortRetryLimit=%ld)\n",
					 ShortRetryLimit));
		}

		break;

	case OID_802_11_LONGRETRYLIMIT:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT\n"));

		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&LongRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
			AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT (,LongRetryLimit=%ld)\n",
					 LongRetryLimit));
		}

		break;

	case OID_802_11_WEPDEFAULTKEYVALUE: {
		UINT KeyIdx;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE\n"));
		os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);

		if (pKey == NULL) {
			Status = -EINVAL;
			break;
		}

		Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);

		/*pKey = &WepKey; */

		if (pKey->Length != wrq->u.data.length) {
			Status = -EINVAL;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n"));
		}

		KeyIdx = pKey->KeyIndex & 0x0fffffff;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pKey->KeyIndex =%d, pKey->KeyLength=%d\n", pKey->KeyIndex,
				 pKey->KeyLength));

		/* it is a shared key */
		if (KeyIdx > 4)
			Status = -EINVAL;
		else {
			pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen = (UCHAR) pKey->KeyLength;
			NdisMoveMemory(&pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].Key, &pKey->KeyMaterial,
						   pKey->KeyLength);

			if (pKey->KeyIndex & 0x80000000) {
				/* Default key for tx (shared key) */
				pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId = (UCHAR) KeyIdx;
			}

			/*RestartAPIsRequired = TRUE; */
		}

		os_free_mem(pKey);
		break;
	}

	case OID_802_11_WEPDEFAULTKEYID:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYID\n"));

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else
			Status = copy_from_user(&pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId, wrq->u.data.pointer, wrq->u.data.length);

		break;

	case OID_802_11_CURRENTCHANNEL:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_CURRENTCHANNEL\n"));

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			RTMP_STRING ChStr[5] = {0};

			Status = copy_from_user(&ctmp, wrq->u.data.pointer, wrq->u.data.length);
			snprintf(ChStr, sizeof(ChStr), "%d", ctmp);
			Set_Channel_Proc(pAd, ChStr);
		}

		break;
#endif /* SNMP_SUPPORT */
#ifdef DOT1X_SUPPORT

	case OID_802_DOT1X_PMKID_CACHE:
		RTMPIoctlAddPMKIDCache(pAd, wrq);
		break;

	case OID_802_DOT1X_RADIUS_DATA:
		Dot1xIoctlRadiusData(pAd, wrq);
		break;

	case OID_802_DOT1X_WPA_KEY:
		Dot1xIoctlAddWPAKey(pAd, wrq);
		break;

	case OID_802_DOT1X_STATIC_WEP_COPY:
		Dot1xIoctlStaticWepCopy(pAd, wrq);
		break;

	case OID_802_DOT1X_IDLE_TIMEOUT:
		RTMPIoctlSetIdleTimeout(pAd, wrq);
		break;
#ifdef RADIUS_MAC_ACL_SUPPORT

	case OID_802_DOT1X_RADIUS_ACL_NEW_CACHE:
		RTMPIoctlAddRadiusMacAuthCache(pAd, wrq);
		break;

	case OID_802_DOT1X_RADIUS_ACL_DEL_CACHE:
		RTMPIoctlDelRadiusMacAuthCache(pAd, wrq);
		break;

	case OID_802_DOT1X_RADIUS_ACL_CLEAR_CACHE:
		RTMPIoctlClearRadiusMacAuthCache(pAd, wrq);
		break;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

	case OID_802_11_AUTHENTICATION_MODE: {
		struct wifi_dev *wdev = NULL;
		UCHAR apIdx = pObj->ioctl_if;
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apIdx];

		if (wrq->u.data.length != sizeof(NDIS_802_11_AUTHENTICATION_MODE))
			Status  = -EINVAL;
		else {
			UINT32 AKMMap = 0;

			Status = copy_from_user(&AuthMode, wrq->u.data.pointer, wrq->u.data.length);

			if (AuthMode > Ndis802_11AuthModeMax) {
				Status  = -EINVAL;
				break;
			}

			AKMMap = SecAuthModeOldToNew(AuthMode);
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
			{
				wdev = &pMbss->wdev;

				if (wdev->SecConfig.AKMMap != AKMMap) {
					/* Config has changed */
					pAd->bConfigChanged = TRUE;
				}

				wdev->SecConfig.AKMMap = AKMMap;
			}

			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_AUTHENTICATION_MODE (=0x%x)\n",
					 wdev->SecConfig.AKMMap));
		}

		if (wdev) {
			APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
			APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);
		}

		break;
	}

	case OID_802_11_WEP_STATUS:
		if (wrq->u.data.length != sizeof(NDIS_802_11_WEP_STATUS))
			Status  = -EINVAL;
		else {
			UINT32 EncryType = 0;

			Status = copy_from_user(&WepStatus, wrq->u.data.pointer, wrq->u.data.length);
			EncryType = SecEncryModeOldToNew(WepStatus);
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
			{
				UCHAR apIdx = pObj->ioctl_if;
				BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apIdx];
				struct wifi_dev *wdev = &pMbss->wdev;

				/* Since TKIP, AES, WEP are all supported. It should not have any invalid setting */
				if (EncryType != 0) {
					if (wdev->SecConfig.PairwiseCipher != EncryType) {
						/* Config has changed */
						pAd->bConfigChanged = TRUE;
					}

					wdev->SecConfig.PairwiseCipher = EncryType;

					if (IS_CIPHER_TKIP(EncryType) && IS_CIPHER_CCMP128(EncryType))
						SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
					else
						wdev->SecConfig.GroupCipher = EncryType;
				} else {
					Status  = -EINVAL;
					break;
				}

				APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
				APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::OID_802_11_WEP_STATUS (=0x%x)\n", EncryType));
			}
		}

		break;

	case OID_802_11_SSID:
		if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
			Status = -EINVAL;
		else {
			RTMP_STRING *pSsidString = NULL;

			Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);

			if (Ssid.SsidLength > MAX_LEN_OF_SSID)
				Status = -EINVAL;
			else {
				if (Ssid.SsidLength == 0)
					Status = -EINVAL;
				else {
					os_alloc_mem(NULL, (UCHAR **)&pSsidString, MAX_LEN_OF_SSID + 1);

					if (pSsidString) {
						NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID + 1);
						NdisMoveMemory(pSsidString, Ssid.Ssid, Ssid.SsidLength);
#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/
						{
							NdisZeroMemory((PCHAR)pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,
									MAX_LEN_OF_SSID);
							strncpy((PCHAR)pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,
									pSsidString
									, MAX_LEN_OF_SSID);
							pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid[MAX_LEN_OF_SSID] =
									(CHAR)'\0';
							pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen =
									strlen(pSsidString);
						}

						os_free_mem(pSsidString);
					} else
						Status = -ENOMEM;
				}
			}
		}

		break;
#ifdef VENDOR_FEATURE6_SUPPORT

	case OID_802_11_PASSPHRASES: {
		INT i;
		BSS_STRUCT *pMBSSStruct;
		INT retval;
		NDIS80211PSK tmpPSK;

		pObj = (POS_COOKIE) pAd->OS_Cookie;
		NdisZeroMemory(&tmpPSK, sizeof(tmpPSK));
		Status = copy_from_user(&tmpPSK, wrq->u.data.pointer, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("Set::OID_802_11_PASSPHRASE\n"));

		for (i = 0 ; i < tmpPSK.WPAKeyLen ; i++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%c", tmpPSK.WPAKey[i]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("\n"));
		pMBSSStruct = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		retval = SetWPAPSKKey(pAd, &tmpPSK.WPAKey[0], tmpPSK.WPAKeyLen, (PUCHAR)pMBSSStruct->Ssid, pMBSSStruct->SsidLen, pMBSSStruct->PMK);

		if (retval == FALSE)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,  ("PassPhrase Generate Fail\n"));

#ifdef WSC_AP_SUPPORT
		NdisZeroMemory(pMBSSStruct->WscControl.WpaPsk, 64);
		pMBSSStruct->WscControl.WpaPskLen = tmpPSK.WPAKeyLen;
		NdisMoveMemory(pMBSSStruct->WscControl.WpaPsk, &tmpPSK.WPAKey[0], pMBSSStruct->WscControl.WpaPskLen);
#endif /* WSC_AP_SUPPORT */
	}
	break;
#endif /* VENDOR_FEATURE6_SUPPORT */
#ifdef HOSTAPD_SUPPORT

	case HOSTAPD_OID_SET_802_1X:/*pure 1x is enabled. */
		Set_IEEE8021X_Proc(pAd, "1");
		break;

	case HOSTAPD_OID_SET_KEY: {
		UINT KeyIdx;

		Status  = -EINVAL;
		pObj = (POS_COOKIE) pAd->OS_Cookie;
		UINT apidx = pObj->ioctl_if;
		struct wifi_dev *wdev = NULL;

		if (wrq->u.data.length != sizeof(struct ieee80211req_key) || !wrq->u.data.pointer)
			break;

		Status = copy_from_user(&Key, wrq->u.data.pointer, wrq->u.data.length);
		pEntry = MacTableLookup(pAd, Key.ik_macaddr);

		if ((Key.ik_type == CIPHER_WEP64) || (Key.ik_type == CIPHER_WEP128)) { /*dynamic wep with 1x */
			if (pEntry) { /*pairwise key */
				pEntry->PairwiseKey.KeyLen = Key.ik_keylen;
				NdisMoveMemory(pEntry->PairwiseKey.Key, Key.ik_keydata, Key.ik_keylen);
				pEntry->PairwiseKey.CipherAlg = Key.ik_type;
				KeyIdx = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].DefaultKeyId;
				AsicAddPairwiseKeyEntry(
					pAd,
					(UCHAR)pEntry->wcid,
					&pEntry->PairwiseKey);
				RTMPAddWcidAttributeEntry(
					pAd,
					pEntry->func_tb_idx,
					KeyIdx, /* The value may be not zero */
					pEntry->PairwiseKey.CipherAlg,
					pEntry);
			} else { /*group key */
				pMbss = &pAd->ApCfg.MBSSID[apidx];
				KeyIdx = Key.ik_keyix & 0x0fff;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("ra%d KeyIdx=%d\n", apidx, KeyIdx));

				/* it is a shared key */
				if (KeyIdx < 4) {
					pAd->SharedKey[apidx][KeyIdx].KeyLen = (UCHAR) Key.ik_keylen;
					NdisMoveMemory(pAd->SharedKey[apidx][KeyIdx].Key, &Key.ik_keydata, Key.ik_keylen);

					if (Key.ik_keyix & 0x8000) {
						/* Default key for tx (shared key) */
						MTWF_LOG(DBG_CAT_CFG,
								 DBG_SUBCAT_ALL,
								 DBG_LVL_TRACE,
								 ("ra%d DefaultKeyId=%d\n", apidx, KeyIdx));
						pMbss->DefaultKeyId = (UCHAR) KeyIdx;
					}

					/*pMbss->DefaultKeyId=1; */
					pAd->SharedKey[apidx][KeyIdx].CipherAlg = Key.ik_type;
					AsicAddSharedKeyEntry(
						pAd,
						apidx,
						KeyIdx,
						&pAd->SharedKey[apidx][KeyIdx]
					);
					RTMPAddWcidAttributeEntry(
						pAd,
						apidx,
						KeyIdx,
						pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg,
						NULL);
				}
			}
		} else if (pEntry) {
			KeyIdx = Key.ik_keyix & 0x0fff;

			if (pEntry->WepStatus == Ndis802_11TKIPEnable) {
				pEntry->PairwiseKey.KeyLen = LEN_TK;
				NdisMoveMemory(&pEntry->PTK[OFFSET_OF_PTK_TK], Key.ik_keydata, Key.ik_keylen);
				NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], Key.ik_keylen);
			}

			if (pEntry->WepStatus == Ndis802_11AESEnable) {
				pEntry->PairwiseKey.KeyLen = LEN_TK;
				NdisMoveMemory(&pEntry->PTK[OFFSET_OF_PTK_TK], Key.ik_keydata, OFFSET_OF_PTK_TK);
				NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], Key.ik_keylen);
			}

			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;

			if (pEntry->WepStatus == Ndis802_11TKIPEnable)
				pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
			else if (pEntry->WepStatus == Ndis802_11AESEnable)
				pEntry->PairwiseKey.CipherAlg = CIPHER_AES;

			pEntry->PairwiseKey.CipherAlg = Key.ik_type;
			AsicAddPairwiseKeyEntry(
				pAd,
				(UCHAR)pEntry->wcid,
				&pEntry->PairwiseKey);
		} else {
			pMbss = &pAd->ApCfg.MBSSID[apidx];
			wdev = &pMbss->wdev;
			KeyIdx = Key.ik_keyix & 0x0fff;
			/*if (Key.ik_keyix & 0x8000) */
			{
				pMbss->DefaultKeyId = (UCHAR) KeyIdx;
			}

			if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11TKIPEnable) {
				pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen = LEN_TK;
				NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].Key, Key.ik_keydata, 16);
				NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].RxMic, (Key.ik_keydata + 16 + 8), 8);
				NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].TxMic, (Key.ik_keydata + 16), 8);
			}

			if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11AESEnable) {
				pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen = LEN_TK;
				NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].Key, Key.ik_keydata, 16);
				NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].RxMic, (Key.ik_keydata + 16 + 8), 8);
				NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].TxMic, (Key.ik_keydata + 16), 8);
			}

			pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg  = CIPHER_NONE;

			if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11TKIPEnable)
				pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg = CIPHER_TKIP;
			else if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11AESEnable)
				pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg = CIPHER_AES;

			hex_dump("Key.ik_keydata,", (unsigned char *) Key.ik_keydata, 32);
			AsicAddSharedKeyEntry(
				pAd,
				apidx,
				pMbss->DefaultKeyId,
				&pAd->SharedKey[apidx][pMbss->DefaultKeyId]
			);
			GET_GroupKey_WCID(wdev, Wcid);
			/*RTMPAddWcidAttributeEntry(
			pAd,
			apidx,
			pMbss->DefaultKeyId,
			pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg,
			NULL);*/
		}

		break;
	}

	case HOSTAPD_OID_DEL_KEY:
		Status  = -EINVAL;

		if (wrq->u.data.length != sizeof(struct ieee80211req_del_key) || !wrq->u.data.pointer)
			break;

		Status = copy_from_user(&delkey, wrq->u.data.pointer, wrq->u.data.length);
		pEntry = MacTableLookup(pAd, delkey.idk_macaddr);

		if (pEntry) {
			/* clear the previous Pairwise key table */
			if (pEntry->wcid != 0) {
				NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));
				AsicRemovePairwiseKeyEntry(pAd, (UCHAR)pEntry->wcid);
			}
		} else if ((delkey.idk_macaddr == NULL) && (delkey.idk_keyix < 4))
			/* remove group key */
			AsicRemoveSharedKeyEntry(pAd, pEntry->func_tb_idx, delkey.idk_keyix);

		break;

	case HOSTAPD_OID_SET_STA_AUTHORIZED:/*for portsecured flag. */
		if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
			pEntry = MacTableLookup(pAd, mlme.im_macaddr);

			if (!pEntry)
				Status = -EINVAL;
			else {
				tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

				switch (mlme.im_op) {
				case IEEE80211_MLME_AUTHORIZE:
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					pEntry->WpaState = AS_PTKINITDONE;/*wpa state machine is not in use. */
					/*pAd->StaCfg[0].PortSecured= WPA_802_1X_PORT_SECURED; */
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					break;

				case IEEE80211_MLME_UNAUTHORIZE:
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
					break;

				default:
					Status = -EINVAL;
				}
			}
		}

		break;

	case HOSTAPD_OID_STATIC_WEP_COPY: {
		UINT KeyIdx;
		INT	 apidx;

		if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
			pEntry = MacTableLookup(pAd, mlme.im_macaddr);

			if (!pEntry)
				Status = -EINVAL;
			else {
				/*Status  = -EINVAL; */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("Woody HOSTAPD_OID_STATIC_WEP_COPY IEEE8021X=%d WepStatus=%d\n", pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.IEEE8021X,
						  pEntry->WepStatus));

				if (pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.IEEE8021X != TRUE)
					break;

				if (pEntry->WepStatus != Ndis802_11WEPEnabled)
					break;

				apidx = pObj->ioctl_if;
				pMbss = &pAd->ApCfg.MBSSID[apidx];
				KeyIdx = pMbss->DefaultKeyId;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("Woody HOSTAPD_OID_STATIC_WEP_COPY=%d\n", KeyIdx));
				pEntry->AuthMode = pAd->ApCfg.MBSSID[apidx].AuthMode;
				pEntry->PairwiseKey.KeyLen = pAd->SharedKey[apidx][KeyIdx].KeyLen;
				NdisMoveMemory(pEntry->PairwiseKey.Key, pAd->SharedKey[apidx][KeyIdx].Key, pAd->SharedKey[apidx][KeyIdx].KeyLen);
				pEntry->PairwiseKey.CipherAlg = pAd->SharedKey[apidx][KeyIdx].CipherAlg;
				AsicAddPairwiseKeyEntry(
					pAd,
					(UCHAR)pEntry->wcid,
					&pEntry->PairwiseKey);
				RTMPAddWcidAttributeEntry(
					pAd,
					pEntry->func_tb_idx,
					KeyIdx, /* The value may be not zero */
					pEntry->PairwiseKey.CipherAlg,
					pEntry);
			}
		}

		break;
	}

	case HOSTAPD_OID_SET_STA_DEAUTH:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::HOSTAPD_OID_SET_STA_DEAUTH\n"));
		MLME_DEAUTH_REQ_STRUCT *pInfo;
		MLME_QUEUE_ELEM *Elem;

		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

		if (Elem) {
			pInfo = (MLME_DEAUTH_REQ_STRUCT *) Elem->Msg;

			if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
				Status  = -EINVAL;
			else {
				Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
				NdisMoveMemory(pInfo->Addr, mlme.im_macaddr, MAC_ADDR_LEN);
				pEntry = MacTableLookup(pAd, pInfo->Addr);

				if (pEntry != NULL) {
					pInfo->Reason = mlme.im_reason;
					Elem->Wcid = pEntry->wcid;
					MlmeEnqueue(pAd, AP_AUTH_STATE_MACHINE, APMT2_MLME_DEAUTH_REQ, sizeof(MLME_DEAUTH_REQ_STRUCT), Elem, 0);
				}
			}

			os_free_mem(Elem);
		}

		break;

	case HOSTAPD_OID_SET_STA_DISASSOC:/*hostapd request to disassoc the station. */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::HOSTAPD_OID_SET_STA_DISASSOC\n"));
		MLME_DISASSOC_REQ_STRUCT DisassocReq;

		if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
			NdisMoveMemory(DisassocReq.Addr, mlme.im_macaddr, MAC_ADDR_LEN);
			DisassocReq.Reason = mlme.im_reason;
			MlmeEnqueue(pAd, AP_ASSOC_STATE_MACHINE, APMT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, 0);
		}

		break;

	case OID_HOSTAPD_SUPPORT:/*notify the driver to support hostapd. */
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			BOOLEAN hostapd_enable;
			int v, apidx;

			apidx = pObj->ioctl_if;
			Status = copy_from_user(&hostapd_enable, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("OID_HOSTAPD_SUPPORT apidx=%d\n", apidx));
			pAd->ApCfg.MBSSID[apidx].Hostapd = hostapd_enable;
			BSS_STRUCT *pMBSSStruct;

			for (v = 0; v < MAX_MBSSID_NUM(pAd); v++) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("ApCfg->MBSSID[%d].Hostapd == %s\n", v,
						  (pAd->ApCfg.MBSSID[v].Hostapd == Hostapd_EXT ? "TRUE" : "FALSE")));
				pMBSSStruct = &pAd->ApCfg.MBSSID[v];
				pMBSSStruct->WPAREKEY.ReKeyInterval = 0;
				pMBSSStruct->WPAREKEY.ReKeyMethod = DISABLE_REKEY;
			}
		}

		break;

	case HOSTAPD_OID_COUNTERMEASURES:/*report txtsc to hostapd. */
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			BOOLEAN countermeasures_enable;

			Status = copy_from_user(&countermeasures_enable, wrq->u.data.pointer, wrq->u.data.length);

			if (countermeasures_enable) {
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Receive CM Attack Twice within 60 seconds ====>>>\n"));
					/* send wireless event - for counter measures */
					pAd->ApCfg.CMTimerRunning = FALSE;

					for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
						MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

						/* happened twice within 60 sec,  AP SENDS disaccociate all associated STAs.  All STA's transition to State 2 */
						if (IS_ENTRY_CLIENT(pEntry))
							MlmeDeAuthAction(pAd, &pAd->MacTab.Content[i], REASON_MIC_FAILURE, FALSE);
					}

					/* Further,  ban all Class 3 DATA transportation for a period of 60 sec */
					/* disallow new association , too */
					pAd->ApCfg.BANClass3Data = TRUE;
				}
			} else
				pAd->ApCfg.BANClass3Data = FALSE;
		}

		break;

	case HOSTAPD_OID_SET_WPS_BEACON_IE:/*pure 1x is enabled. */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HOSTAPD_OID_SET_WPS_BEACON_IE\n"));

		if (wrq->u.data.length != sizeof(WSC_LV_INFO))
			Status  = -EINVAL;
		else {
			INT apidx;

			apidx = pObj->ioctl_if;
			pAd->ApCfg.MBSSID[apidx].HostapdWPS = TRUE;
			BSS_STRUCT *pMBSSStruct;

			NdisZeroMemory(&WscIEBeacon, sizeof(WSC_LV_INFO));
			Status = copy_from_user(&WscIEBeacon, wrq->u.data.pointer, wrq->u.data.length);
			pMBSSStruct = &pAd->ApCfg.MBSSID[apidx];
			NdisMoveMemory(pMBSSStruct->WscIEBeacon.Value, WscIEBeacon.Value, WscIEBeacon.ValueLen);
			pMBSSStruct->WscIEBeacon.ValueLen = WscIEBeacon.ValueLen;
			UpdateBeaconHandler(
				pAd,
				wdev,
				BCN_UPDATE_IE_CHG);
		}

		break;

	case HOSTAPD_OID_SET_WPS_PROBE_RESP_IE:/*pure 1x is enabled. */
		apidx = pObj->ioctl_if;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HOSTAPD_OID_SET_WPS_PROBE_RESP_IE\n"));

		if (wrq->u.data.length != sizeof(WSC_LV_INFO)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HOSTAPD_OID_SET_WPS_PROBE_RESP_IE failed\n"));
			Status  = -EINVAL;
		} else {
			INT apidx;
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

			apidx = pObj->ioctl_if;
			pAd->ApCfg.MBSSID[apidx].HostapdWPS = TRUE;
			BSS_STRUCT *pMBSSStruct;

			NdisZeroMemory(&WscIEProbeResp, sizeof(WSC_LV_INFO));
			Status = copy_from_user(&WscIEProbeResp, wrq->u.data.pointer, wrq->u.data.length);
			pMBSSStruct = &pAd->ApCfg.MBSSID[apidx];
			NdisMoveMemory(pMBSSStruct->WscIEProbeResp.Value, WscIEProbeResp.Value, WscIEProbeResp.ValueLen);
			pMBSSStruct->WscIEProbeResp.ValueLen = WscIEProbeResp.ValueLen;
			UpdateBeaconHandler(
				pAd,
				wdev,
				BCN_UPDATE_IE_CHG);
		}

		break;
#endif /*HOSTAPD_SUPPORT*/
#ifdef CONFIG_HOTSPOT

	case OID_802_11_HS_TEST:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("hotspot test\n"));
		break;

	case OID_802_11_HS_IE: {
		UCHAR *IE;

		os_alloc_mem(NULL, (UCHAR **)&IE, wrq->u.data.length);
		Status = copy_from_user(IE, wrq->u.data.pointer, wrq->u.data.length);
		Set_AP_IE(pAd, IE, wrq->u.data.length);
		os_free_mem(IE);
	}
	break;

	case OID_802_11_HS_ANQP_RSP: {
		UCHAR *Buf;
		struct anqp_rsp_data *rsp_data;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		rsp_data = (struct anqp_rsp_data *)Buf;
		Send_ANQP_Rsp(pAd,
					  rsp_data->peer_mac_addr,
					  rsp_data->anqp_rsp,
					  rsp_data->anqp_rsp_len);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_HS_ONOFF: {
		UCHAR *Buf;
		struct hs_onoff *onoff;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		onoff = (struct hs_onoff *)Buf;
		Set_HotSpot_OnOff(pAd, onoff->hs_onoff, onoff->event_trigger, onoff->event_type);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_HS_PARAM_SETTING: {
		UCHAR *Buf;
		struct hs_param_setting *param_setting;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		param_setting = (struct hs_param_setting *)Buf;
		Set_HotSpot_Param(pAd, param_setting->param, param_setting->value);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_HS_RESET_RESOURCE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("hotspot reset some resource\n"));
		Clear_Hotspot_All_IE(pAd);
		/* Clear_All_PROXY_TABLE(pAd); */
		break;
#ifdef CONFIG_HOTSPOT_R2

	case OID_802_11_HS_SASN_ENABLE: {
		UCHAR *Buf;
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pObj->ioctl_if].HotSpotCtrl;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		pHSCtrl->bASANEnable = Buf[0];
		/* for 7615 offload to CR4 */
		hotspot_update_bssflag(pAd, fgASANEnable, Buf[0], pHSCtrl);
		hotspot_update_bss_info_to_cr4(pAd, pObj->ioctl_if);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_BSS_LOAD: {
		UCHAR *Buf;
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pObj->ioctl_if].HotSpotCtrl;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		pHSCtrl->QLoadTestEnable = Buf[0];
		pHSCtrl->QLoadCU = Buf[1];
		memcpy(&pHSCtrl->QLoadStaCnt, &Buf[2], 2);
		os_free_mem(Buf);
	}
	break;
#endif
#endif
#ifdef CONFIG_DOT11V_WNM
#ifndef CONFIG_HOTSPOT_R2/* def WNM_NEW_API */

	case OID_802_11_WNM_COMMAND: {
		UCHAR *Buf;
		struct wnm_command *cmd_data;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		cmd_data = (struct wnm_command *)Buf;

		if (wnm_handle_command(pAd, cmd_data) != NDIS_STATUS_SUCCESS)
			Status = -EINVAL;

		os_free_mem(Buf);
	}
	break;
#endif /*WNM_NEW_API*/
#ifdef CONFIG_HOTSPOT_R2

	case OID_802_11_WNM_BTM_REQ: {
		UCHAR *Buf;
		MAC_TABLE_ENTRY *pEntry = NULL;
		struct btm_req_data *req_data;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		req_data = (struct btm_req_data *)Buf;
		pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);

		if (pEntry == NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : NO ENTRY!!!!!!\n", __func__));
		} /*else if (pEntry->BssTransitionManmtSupport != 1) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("OID_802_11_WNM_BTM_REQ::the peer does not support BTM(%d)\n",
					  pEntry->BssTransitionManmtSupport));
		} */else if (IS_AKM_OPEN(pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.AKMMap) ||
				   ((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
					(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED))) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("btm1\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("peer_mac_addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
					  req_data->peer_mac_addr[0],
					  req_data->peer_mac_addr[1],
					  req_data->peer_mac_addr[2],
					  req_data->peer_mac_addr[3],
					  req_data->peer_mac_addr[4],
					  req_data->peer_mac_addr[5]));
			Send_BTM_Req(pAd,
						 req_data->peer_mac_addr,
						 req_data->btm_req,
						 req_data->btm_req_len);
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!btm2\n"));
			pEntry->IsBTMReqValid = TRUE;
			os_alloc_mem(pEntry->ReqbtmData, (UCHAR **)&pEntry->ReqbtmData, sizeof(struct btm_req_data) + req_data->btm_req_len);
			memcpy(pEntry->ReqbtmData, Buf, sizeof(struct btm_req_data) + req_data->btm_req_len);
		}

		os_free_mem(Buf);
	}
	break;

	case OID_802_11_WNM_NOTIFY_REQ: {
		UCHAR *Buf;
		MAC_TABLE_ENTRY *pEntry;
		struct wnm_req_data *req_data;
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pObj->ioctl_if].HotSpotCtrl;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		req_data = (struct wnm_req_data *)Buf;
		pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);

		if (pEntry == NULL) {
		} else if (((pHSCtrl->HotSpotEnable) && IS_AKM_OPEN(pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.AKMMap)) ||
				   ((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
					(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED))) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wnm1\n"));
			Send_WNM_Notify_Req(pAd,
								req_data->peer_mac_addr,
								req_data->wnm_req,
								req_data->wnm_req_len,
								req_data->type);
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wnm2\n"));
			pEntry->IsWNMReqValid = TRUE;
			os_alloc_mem(pEntry->ReqData, (UCHAR **)&pEntry->ReqData, sizeof(struct wnm_req_data) + req_data->wnm_req_len);
			memcpy(pEntry->ReqData, Buf, sizeof(struct wnm_req_data) + req_data->wnm_req_len);
		}

		os_free_mem(Buf);
	}
	break;

	case OID_802_11_QOSMAP_CONFIGURE: {
		UCHAR *Buf;
		MAC_TABLE_ENTRY *pEntry;
		struct qosmap_data *req_data;
		unsigned int i;
		UCHAR PoolID = 0;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		req_data = (struct qosmap_data *)Buf;
		pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);

		if (pEntry != NULL) {
			/* clear previous data */
			pEntry->DscpExceptionCount = 0;
			memset(pEntry->DscpRange, 0xff, 16);
			memset(pEntry->DscpException, 0xff, 42);
			pEntry->DscpExceptionCount = req_data->qosmap_len - 16;
			memcpy((UCHAR *)pEntry->DscpRange, &req_data->qosmap[pEntry->DscpExceptionCount], 16);

			if (pEntry->DscpExceptionCount != 0)
				memcpy((UCHAR *)pEntry->DscpException, req_data->qosmap, pEntry->DscpExceptionCount);

			PoolID = hotspot_qosmap_add_pool(pAd, pEntry);
			hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, PoolID);
			Send_QOSMAP_Configure(pAd,
								  req_data->peer_mac_addr,
								  req_data->qosmap,
								  req_data->qosmap_len,
								  pEntry->func_tb_idx);
		} else if ((req_data->peer_mac_addr[0] == 0)
				   && (req_data->peer_mac_addr[1] == 0)
				   && (req_data->peer_mac_addr[2] == 0)
				   && (req_data->peer_mac_addr[3] == 0)
				   && (req_data->peer_mac_addr[4] == 0)
				   && (req_data->peer_mac_addr[5] == 0)) {
			/* Special MAC 00:00:00:00:00:00 for HS2 QoS Map Change using. */
			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				pEntry = &pAd->MacTab.Content[i];

				if ((IS_ENTRY_CLIENT(pEntry))
					&& (pEntry->Sst == SST_ASSOC)) {
					if (pEntry->QosMapSupport) {
						pEntry->DscpExceptionCount = 0;
						memset(pEntry->DscpRange, 0xff, 16);
						memset(pEntry->DscpException, 0xff, 42);
						pEntry->DscpExceptionCount = req_data->qosmap_len - 16;
						memcpy((UCHAR *)pEntry->DscpRange,
							   &req_data->qosmap[pEntry->DscpExceptionCount], 16);

						if (pEntry->DscpExceptionCount != 0) {
							memcpy((UCHAR *)pEntry->DscpException,
								   req_data->qosmap, pEntry->DscpExceptionCount);
						}

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
								 ("send QoS map frame: apidx=%d\n", pEntry->func_tb_idx));
						Send_QOSMAP_Configure(pAd,
											  pEntry->Addr,
											  req_data->qosmap,
											  req_data->qosmap_len,
											  pEntry->func_tb_idx);
					}
				}
			}
		}

		os_free_mem(Buf);
	}
	break;
#endif
#endif
#ifdef	DOT11K_RRM_SUPPORT

	case OID_802_11_RRM_COMMAND:
		Status = rrm_MsgHandle(pAd, wrq);

		if (Status != NDIS_STATUS_SUCCESS)
			Status = -NDIS_STATUS_FAILURE;

		break;
#endif
#ifdef MBO_SUPPORT
	case OID_802_11_MBO_MSG:
	{
		UCHAR *Buf;
		struct hs_param_setting *param_setting;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		param_setting = (struct hs_param_setting *)Buf;
		MBO_MsgHandle(pAd, param_setting->param, param_setting->value);
		os_free_mem(Buf);
	}
		break;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
	case OID_WAPP_EVENT:
	{
		UCHAR *buf;
		struct wapp_req *req;

		os_alloc_mem(NULL, (UCHAR **)&buf, wrq->u.data.length);
		copy_from_user(buf, wrq->u.data.pointer, wrq->u.data.length);
		req = (struct wapp_req *)buf;
		wapp_event_handle(pAd, req);
		os_free_mem(buf);
	}
		break;
#endif /* WAPP_SUPPORT */
#ifdef VOW_SUPPORT
#define VOW_CMD_STR_LEN 16

	case OID_802_11_VOW_BW_EN:
		/* not used now */
		break;

	case OID_802_11_VOW_BW_AT_EN: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(cfg, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				sprintf(buf, "%d-%d", group, cfg->val[group]);
				set_vow_airtime_ctrl_en(pAd, buf);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_BW_AT_EN(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]));
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_BW_TPUT_EN: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(cfg, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				sprintf(buf, "%d-%d", group, cfg->val[group]);
				set_vow_bw_ctrl_en(pAd, buf);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_BW_TPUT_EN(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]));
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_ATF_EN: {
		UCHAR *val, buf[VOW_CMD_STR_LEN];

		os_alloc_mem(val, (UCHAR **)&val, wrq->u.data.length);

		if (val == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(val, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			sprintf(buf, "%d", val[0]);
			set_vow_airtime_fairness_en(pAd, buf);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_ATF_EN(0x%08x) -> val %d\n", cmd, val[0]));
		} else
			Status = -EFAULT;

		os_free_mem(val);
	}
	break;

	case OID_802_11_VOW_RX_EN: {
		UCHAR *val, buf[VOW_CMD_STR_LEN];

		os_alloc_mem(val, (UCHAR **)&val, wrq->u.data.length);

		if (val == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(val, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			sprintf(buf, "%d", val[0]);
			set_vow_rx_airtime_en(pAd, buf);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_RX_EN(0x%08x) -> val %d\n", cmd, val[0]));
		} else
			Status = -EFAULT;

		os_free_mem(val);
	}
	break;

	case OID_802_11_VOW_GROUP_MAX_RATE: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(cfg, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				sprintf(buf, "%d-%d", group, cfg->val[group]);
				set_vow_max_rate(pAd, buf);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_GROUP_MAX_RATE(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]));
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_GROUP_MIN_RATE: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(cfg, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				sprintf(buf, "%d-%d", group, cfg->val[group]);
				set_vow_min_rate(pAd, buf);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_GROUP_MIN_RATE(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]));
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_GROUP_MAX_RATIO: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(cfg, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				sprintf(buf, "%d-%d", group, cfg->val[group]);
				set_vow_max_ratio(pAd, buf);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_GROUP_MAX_RATIO(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]));
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_GROUP_MIN_RATIO: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(cfg, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				sprintf(buf, "%d-%d", group, cfg->val[group]);
				set_vow_min_ratio(pAd, buf);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OID_802_11_VOW_GROUP_MIN_RATIO(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]));
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;
#endif /* VOW_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	case OID_802_11_WIFISPECTRUM_SET_PARAMETER: {
		P_RBIST_CAP_START_T pSpectrumInfo;
		UINT_32 ret;

		os_alloc_mem(pAd, (UCHAR **)&pSpectrumInfo, wrq->u.data.length);
		ret = copy_from_user(pSpectrumInfo, wrq->u.data.pointer, wrq->u.data.length);
		pSpectrumInfo->u4BW = Get_System_Bw_Info(pAd, pSpectrumInfo->u4CaptureNode);
		chip_spectrum_start(pAd, (UINT8 *)pSpectrumInfo);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s :\n Trigger = 0x%08x\n"
				" RingCapEn  = 0x%08x\n TriggerEvent  = 0x%08x\n CaptureNode = 0x%08x\n CaptureLen = 0x%08x\n"
				" CapStopCycle = 0x%08x\n BW = 0x%08x\n MACTriggerEvent = 0x%08x\n SourceAddrLSB = 0x%08x\n"
				" SourceAddrMSB = 0x%08x\n BandIdx = 0x%08x\n", __func__, pSpectrumInfo->fgTrigger, pSpectrumInfo->fgRingCapEn,
				pSpectrumInfo->u4TriggerEvent, pSpectrumInfo->u4CaptureNode, pSpectrumInfo->u4CaptureLen, pSpectrumInfo->u4CapStopCycle,
				pSpectrumInfo->u4BW, pSpectrumInfo->u4MACTriggerEvent, pSpectrumInfo->u4SourceAddressLSB,
				pSpectrumInfo->u4SourceAddressMSB, pSpectrumInfo->u4BandIdx));

		os_free_mem(pSpectrumInfo);
	}
	break;

	case OID_802_11_WIFISPECTRUM_GET_CAPTURE_STATUS: {
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumStatus != NULL)
			Status = ops->SpectrumStatus(pAd);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}

		if (Status != NDIS_STATUS_SUCCESS)
			Status = -NDIS_STATUS_FAILURE;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("OID_802_11_WIFISPECTRUM_GET_CAPTURE_STATUS Status : %d\n", Status));
	}
	break;

	case OID_802_11_WIFISPECTRUM_DUMP_DATA: {
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumCmdRawDataProc != NULL)
			Status = ops->SpectrumCmdRawDataProc(pAd);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : The function is not hooked !!\n", __func__));
		}

		if (Status != NDIS_STATUS_SUCCESS)
			Status = -NDIS_STATUS_FAILURE;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("OID_802_11_WIFISPECTRUM_DUMP_DATA Status : %d\n", Status));
	}
	break;
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef BAND_STEERING
	case OID_BNDSTRG_MSG:
		BndStrg_MsgHandle(pAd, wrq, pObj->ioctl_if);
		break;
#ifdef VENDOR_FEATURE5_SUPPORT
	case OID_BNDSTRG_GET_NVRAM:
		BndStrg_GetNvram(pAd, wrq, pObj->ioctl_if);
		break;

	case OID_BNDSTRG_SET_NVRAM:
		BndStrg_SetNvram(pAd, wrq, pObj->ioctl_if);
		break;
#endif /* VENDOR_FEATURE5_SUPPORT */
#endif /* BAND_STEERING */

#ifdef MT_DFS_SUPPORT
	case OID_DFS_ZERO_WAIT:
		Status = ZeroWaitDfsCmdHandler(pAd, wrq);
		break;
#endif

	case OID_802_11_VENDOR_IE_ADD:
	case OID_802_11_VENDOR_IE_UPDATE:
	case OID_802_11_VENDOR_IE_REMOVE:
	{
		UCHAR *Buf;
		struct vie_op_data_s *vie_op_data;
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		UINT32 length = 0;
		UINT32 oui_oitype = 0;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		vie_op_data = (struct vie_op_data_s *)Buf;
		length = vie_op_data->vie_length;
		NdisMoveMemory((UCHAR *)&oui_oitype, vie_op_data->oui_oitype, sizeof(UINT32));

		if ((cmd & 0x7FFF) == VIE_REMOVE) {
			if (remove_vie(pAd,
				       wdev,
				       vie_op_data->frm_type_map,
				       oui_oitype,
				       length,
				       vie_op_data->app_ie_ctnt) == NDIS_STATUS_FAILURE) {
				Status = NDIS_STATUS_FAILURE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): OID_802_11_VENDOR_IE_REMOVE failed.\n", __func__));
			}
		} else {
			if (add_vie(pAd,
				    wdev,
				    vie_op_data->frm_type_map,
				    oui_oitype,
				    length,
				    vie_op_data->app_ie_ctnt) == NDIS_STATUS_FAILURE) {
				Status = NDIS_STATUS_FAILURE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): OID_802_11_VENDOR_IE_ADD failed.\n", __func__));
			}
		}

		os_free_mem(Buf);
	}
		break;
	case OID_802_11_VENDOR_IE_SHOW:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("OID_802_11_VENDOR_IE_SHOW not finish yet\n"));
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::unknown IOCTL's subcmd = 0x%08x\n", cmd));
		Status = -EOPNOTSUPP;
		break;
	}

	return Status;
}


INT RTMPAPQueryInformation(
	IN RTMP_ADAPTER *pAd,
	IN OUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd)
{
	RTMP_IOCTL_INPUT_STRUCT	 *wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
	INT	Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_STRING driverVersion[16];
	UCHAR apidx = pObj->ioctl_if;
#ifdef WSC_AP_SUPPORT
	UINT WscPinCode = 0;
	PWSC_PROFILE pProfile;
	PWSC_CTRL pWscControl;
#endif /* WSC_AP_SUPPORT */
#if defined(SNMP_SUPPORT) || defined(VENDOR_FEATURE6_SUPPORT)
	ULONG ulInfo;
#endif /*  defined(SNMP_SUPPORT) || defined(VENDOR_FEATURE6_SUPPORT) */
#ifdef SNMP_SUPPORT
	DefaultKeyIdxValue *pKeyIdxValue;
	INT valueLen;
	TX_RTY_CFG_STRUC tx_rty_cfg;
	ULONG ShortRetryLimit, LongRetryLimit;
	UCHAR snmp_tmp[64];
#endif /* SNMP_SUPPORT */
#ifdef HOSTAPD_SUPPORT
	struct default_group_key group_key;
	struct ieee80211req_key ik;
	unsigned char *p;
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;
	struct ieee80211req_wpaie wpaie;
	BSS_STRUCT *pMbss;
#endif /*HOSTAPD_SUPPORT*/
#if (defined(APCLI_SUPPORT) || defined(WH_EZ_SETUP))
	NDIS_802_11_SSID                    Ssid;
#endif
#ifdef APCLI_SUPPORT
	UCHAR ifIndex;
	BOOLEAN apcliEn = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif/*APCLI_SUPPORT*/
	NDIS_802_11_STATISTICS	 *pStatistics;
#ifdef DOT1X_SUPPORT
	INT IEEE8021X = 0;
#endif /* DOT1X_SUPPORT */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode = Ndis802_11AuthModeMax;
	struct wifi_dev *wdev = NULL;

	/* For all ioctl to this function, we assume that's query for AP/APCLI/GO device */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		if (apidx >= pAd->ApCfg.BssidNum)
			return -EFAULT;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	}

	switch (cmd) {
#ifdef DOT1X_SUPPORT

	case OID_802_11_SET_IEEE8021X:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_SET_IEEE8021X\n"));
		wrq->u.data.length = sizeof(INT);

		if (IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev))
			IEEE8021X = 1;
		else
			IEEE8021X = 0;

		Status = copy_to_user(wrq->u.data.pointer, &IEEE8021X, wrq->u.data.length);
		break;
#endif /* DOT1X_SUPPORT */

	case OID_802_11_AUTHENTICATION_MODE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_AUTHENTICATION_MODE\n"));
		wrq->u.data.length = sizeof(NDIS_802_11_AUTHENTICATION_MODE);
		AuthMode = SecAuthModeNewToOld(pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.AKMMap);
		Status = copy_to_user(wrq->u.data.pointer, &AuthMode, wrq->u.data.length);
		break;
#ifdef APCLI_SUPPORT

	case OID_802_11_BSSID:
		if ((pObj->ioctl_if_type != INT_APCLI)
#ifdef VENDOR_FEATURE6_SUPPORT
			&& (pObj->ioctl_if_type != INT_MAIN)
#endif /* VENDOR_FEATURE6_SUPPORT */
		   )
			return FALSE;

		ifIndex = pObj->ioctl_if;

		if (pObj->ioctl_if_type == INT_APCLI) {
			pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
			apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

			if (!apcliEn)
				return FALSE;

			Status = copy_to_user(wrq->u.data.pointer, pApCliEntry->MlmeAux.Bssid, sizeof(NDIS_802_11_MAC_ADDRESS));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("IOCTL::SIOCGIWAP(=%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pApCliEntry->MlmeAux.Bssid)));
		}

#ifdef VENDOR_FEATURE6_SUPPORT
		else if (pObj->ioctl_if_type == INT_MAIN) {
			if (wdev) {
				wrq->u.data.length = MAC_ADDR_LEN;
				Status = copy_to_user(wrq->u.data.pointer, &wdev->bssid[0], wrq->u.data.length);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_BSSID (%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(wdev->bssid)));
			}
		}

#endif /* VENDOR_FEATURE6_SUPPORT */
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_BSSID(=EMPTY)\n"));
			Status = -ENOTCONN;
		}

		break;
#endif/*APCLI_SUPPORT*/
#if (defined(APCLI_SUPPORT) || defined(WH_EZ_SETUP))

	case OID_802_11_SSID:
		ifIndex = pObj->ioctl_if;
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			pApCliEntry =  &pAd->ApCfg.ApCliTab[ifIndex];
			apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

			if (!apcliEn)
				return FALSE;

			Ssid.SsidLength = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(Ssid.Ssid, pApCliEntry->CfgSsid, Ssid.SsidLength);
		}

#endif /* APCLI_SUPPORT */
		wrq->u.data.length = sizeof(NDIS_802_11_SSID);
		Status = copy_to_user(wrq->u.data.pointer, &Ssid, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query OID_802_11_SSID (Len=%d, ssid=%s)\n", Ssid.SsidLength,
				 Ssid.Ssid));
		break;
#endif

	case RT_OID_VERSION_INFO:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_VERSION_INFO\n"));
		wrq->u.data.length = strlen(AP_DRIVER_VERSION);
		snprintf(&driverVersion[0], sizeof(driverVersion), "%s", AP_DRIVER_VERSION);
		driverVersion[wrq->u.data.length] = '\0';

		if (copy_to_user(wrq->u.data.pointer, &driverVersion, wrq->u.data.length))
			Status = -EFAULT;

		break;
#ifdef VENDOR_FEATURE6_SUPPORT

	case RT_OID_802_11_PHY_MODE: {
		UCHAR *temp_wmode = NULL;

		if (wdev) {
			ulInfo = (ULONG)wdev->PhyMode;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			temp_wmode = wmode_2_str(wdev->PhyMode);
		}

		if (temp_wmode != NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_PHY_MODE (=%lu), %s\n", ulInfo, temp_wmode));
			os_free_mem(temp_wmode);
			temp_wmode = NULL;
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_PHY_MODE (=%lu), Null\n", ulInfo));
	}
	break;

	case OID_802_11_CHANNEL_WIDTH: {
		wrq->u.data.length = sizeof(UCHAR);
		ulInfo =  wlan_operate_get_ht_bw(wdev);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_CHANNEL_WIDTH (=%lu)\n", ulInfo));
		break;
	}
	break;

	case RT_OID_802_11_COUNTRY_REGION:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.CountryRegionForABand;
		ulInfo = (ulInfo << 8) | (pAd->CommonCfg.CountryRegion);

		if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_COUNTRY_REGION (=%lu)\n", ulInfo));
		break;

	case OID_802_11_BEACON_PERIOD:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.BeaconPeriod;

		if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_BEACON_PERIOD (=%lu)\n", ulInfo));
		break;

	case RT_OID_802_11_TX_POWER_LEVEL_1:
		wrq->u.data.length = sizeof(UINT8);
		ulInfo = pAd->CommonCfg.ucTxPowerPercentage[BAND0];
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.ucTxPowerPercentage[BAND0], wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_TX_POWER_LEVEL_1 (=%u)\n", pAd->CommonCfg.ucTxPowerPercentage[BAND0]));
		break;

	case RT_OID_802_11_QUERY_WMM:
		if (wdev) {
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->bWmmCapable, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_QUERY_WMM (=%d)\n", wdev->bWmmCapable));
		}

		break;

	case RT_OID_802_11_PREAMBLE:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.TxPreamble;
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_PREAMBLE(=%lu)\n", pAd->CommonCfg.TxPreamble));
		break;

	case OID_802_11_HT_STBC:
		wrq->u.data.length = sizeof(UCHAR);
		ulInfo = wlan_config_get_ht_stbc(wdev);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_HT_STBC(=%lu)\n", ulInfo));
		break;

	case OID_802_11_UAPSD:
		if (wdev) {
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->UapsdInfo.bAPSDCapable, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_UAPSD (=%d)\n", wdev->UapsdInfo.bAPSDCapable));
		}

		break;

	case OID_802_11_TX_PACKET_BURST:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bEnableTxBurst, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_TX_PACKET_BURST (=%d)\n", pAd->CommonCfg.bEnableTxBurst));
		break;

	case OID_802_11_COEXISTENCE:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bBssCoexEnable, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_COEXISTENCE (=%d)\n", pAd->CommonCfg.bBssCoexEnable));
		break;

	case OID_802_11_AMSDU:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.BACapability.field.AmsduEnable;
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_AMSDU (=%lu)\n", ulInfo));
		break;

	case OID_802_11_AMPDU:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.BACapability.field.AutoBA;
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_AMPDU (=%lu)\n", ulInfo));
		break;

	case OID_802_11_ASSOLIST:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_ASSOLIST\n"));
		RTMPAPGetAssoMacTable(pAd, wrq);
		break;
#ifdef WSC_AP_SUPPORT

	case OID_802_11_CURRENT_CRED:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_CURRENT_CRED\n"));
		RTMPGetCurrentCred(pAd, wrq);
		break;
#endif /* WSC_AP_SUPPORT */
#endif /* VENDOR_FEATURE6_SUPPORT */

	case OID_802_11_NETWORK_TYPES_SUPPORTED:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_NETWORK_TYPES_SUPPORTED\n"));
		wrq->u.data.length = sizeof(UCHAR);

		if (copy_to_user(wrq->u.data.pointer, &pAd->RfIcType, wrq->u.data.length))
			Status = -EFAULT;

		break;
#ifdef IAPP_SUPPORT

	case RT_QUERY_SIGNAL_CONTEXT: {
		BOOLEAN FlgIs11rSup = FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_QUERY_SIGNAL_CONTEXT\n"));
#ifdef DOT11R_FT_SUPPORT
		FlgIs11rSup = TRUE;
#endif /* DOT11R_FT_SUPPORT */

		if (FlgIs11rSup == FALSE)
			Status = -EFAULT;

#ifdef DOT11R_FT_SUPPORT
		else {
			FT_KDP_SIGNAL *pFtKdp;
			FT_KDP_EVT_HEADER *pEvtHdr;
			/* query signal content for 11r */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_QUERY_FT_KDP_CONTEXT\n"));
			FT_KDP_EventGet(pAd, &pFtKdp);

			if (pFtKdp != NULL)
				pEvtHdr = (FT_KDP_EVT_HEADER *)pFtKdp->Content;

			/* End of if */

			if ((pFtKdp != NULL) &&
				((RT_SIGNAL_STRUC_HDR_SIZE + pEvtHdr->EventLen) <=
				 wrq->u.data.length)) {
				/* copy the event */
				if (copy_to_user(
						wrq->u.data.pointer,
						pFtKdp,
						RT_SIGNAL_STRUC_HDR_SIZE + pEvtHdr->EventLen)) {
					wrq->u.data.length = 0;
					Status = -EFAULT;
				} else {
					wrq->u.data.length = RT_SIGNAL_STRUC_HDR_SIZE;
					wrq->u.data.length += pEvtHdr->EventLen;
				}

				FT_MEM_FREE(pAd, pFtKdp);
			} else {
				/* no event is queued */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ft_kdp> no event is queued!\n"));
				wrq->u.data.length = 0;
			}
		}

#endif /* DOT11R_FT_SUPPORT */
	}
	break;
#ifdef DOT11R_FT_SUPPORT

	case RT_FT_DATA_ENCRYPT:
	case RT_FT_DATA_DECRYPT: {
		UCHAR *pBuffer;
		UINT32 DataLen;

		DataLen = wrq->u.data.length;

		/*
			Make sure the data length is multiple of 8
			due to AES_KEY_WRAP() limitation.
		*/
		if (DataLen & 0x07)
			DataLen += 8 - (DataLen & 0x07);

		/* End of if */
		FT_MEM_ALLOC(pAd, &pBuffer, DataLen + FT_KDP_KEY_ENCRYPTION_EXTEND);

		if (pBuffer == NULL)
			break;

		NdisZeroMemory(pBuffer, DataLen + FT_KDP_KEY_ENCRYPTION_EXTEND);

		if (copy_from_user(pBuffer, wrq->u.data.pointer, wrq->u.data.length)) {
			Status = -EFAULT;
			FT_MEM_FREE(pAd, pBuffer);
			break;
		}

		switch (cmd) {
		case RT_FT_DATA_ENCRYPT:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_FT_DATA_ENCRYPT\n"));
			FT_KDP_DataEncrypt(pAd, (UCHAR *)pBuffer, &DataLen);
			break;

		case RT_FT_DATA_DECRYPT:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_FT_DATA_DECRYPT\n"));
			FT_KDP_DataDecrypt(pAd, (UCHAR *)pBuffer, &DataLen);
			break;
		}

		wrq->u.data.length = DataLen;

		if (copy_to_user(wrq->u.data.pointer, pBuffer, wrq->u.data.length))
			Status = -EFAULT;

		FT_MEM_FREE(pAd, pBuffer);
	}
	break;

	case RT_OID_802_11R_INFO: {
		UCHAR apidx = pObj->ioctl_if;
		PFT_CONFIG_INFO pFtConfig;
		PFT_CFG pFtCfg;

		os_alloc_mem(pAd, (UCHAR **)&pFtConfig, sizeof(FT_CONFIG_INFO));

		if (pFtConfig == NULL)
			break;

		pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;
		NdisZeroMemory(pFtConfig, sizeof(FT_CONFIG_INFO));
		pFtConfig->FtSupport = pFtCfg->FtCapFlag.Dot11rFtEnable;
		pFtConfig->FtRicSupport = pFtCfg->FtCapFlag.RsrReqCap;
		pFtConfig->FtOtdSupport = pFtCfg->FtCapFlag.FtOverDs;
		NdisMoveMemory(pFtConfig->MdId, pFtCfg->FtMdId, FT_MDID_LEN);
		pFtConfig->R0KHIdLen = pFtCfg->FtR0khIdLen;
		NdisMoveMemory(pFtConfig->R0KHId, pFtCfg->FtR0khId, pFtCfg->FtR0khIdLen);
		wrq->u.data.length = sizeof(FT_CONFIG_INFO);
		Status = copy_to_user(wrq->u.data.pointer, pFtConfig, wrq->u.data.length);
		os_free_mem(pFtConfig);
	}
	break;
#endif /* DOT11R_FT_SUPPORT */
#endif /* IAPP_SUPPORT */

#ifdef CONFIG_11KV_API_SUPPORT
#ifdef CONFIG_DOT11V_WNM
		case RT_QUERY_WNM_CAPABILITY:
		{
			PUCHAR p_wnm_query_data = NULL;
			PMAC_TABLE_ENTRY pEntry = NULL;
			PUINT8 p_cap = NULL;
			struct wnm_command *p_wnm_command = NULL;
			PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[apidx].WNMCtrl;
			UINT16 TotalLen = 0;

			TotalLen = wrq->u.data.length;
			if (TotalLen != sizeof(*p_wnm_command)+MAC_ADDR_LEN+1) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
					("RT_QUERY_WNM_CAPABILITY: length(%d) check failed\n",
					TotalLen));
				Status = EINVAL;
				break;
			}

			if (!pWNMCtrl->WNMBTMEnable) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					("RT_QUERY_WNM_CAPABILITY: btm off\n"));
				Status = EINVAL;
				break;
			}

			os_alloc_mem(NULL, (UCHAR **)&p_wnm_command, wrq->u.data.length);
			if (p_wnm_command == NULL) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					("RT_QUERY_WNM_CAPABILITY: no memory!!!\n"));
				Status = ENOMEM;
				break;
			}

			if (copy_from_user(p_wnm_command, wrq->u.data.pointer, wrq->u.data.length)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("RT_QUERY_WNM_CAPABILITY: copy from user failed!\n"));
				Status = EFAULT;
				os_free_mem(p_wnm_command);
				break;
			}

			p_wnm_query_data = p_wnm_command->command_body;

			/*first six bytes of data is sta mac*/
			pEntry = MacTableLookup(pAd, p_wnm_query_data);
			if (!pEntry ||
				!(IS_AKM_OPEN(pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.AKMMap) ||
				((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
				(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED)))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("RT_QUERY_WNM_CAPABILITY: "));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					("STA(%02x:%02x:%02x:%02x:%02x:%02x)not associates with AP!\n",
					PRINT_MAC(p_wnm_query_data)));
				Status = EINVAL;
				os_free_mem(p_wnm_command);
				break;
			}
			/*check for btm capablility*/
			p_cap = (UINT8 *)(p_wnm_query_data+MAC_ADDR_LEN);
			*p_cap = 0;

			if (p_wnm_command->command_id ==
				OID_802_11_WNM_CMD_QUERY_BTM_CAP) {
				if (pEntry->BssTransitionManmtSupport) {
					*p_cap = 1;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("RT_QUERY_WNM_CAPABILITY: BTMCap=%d\n", (*p_cap)));
				}
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("RT_QUERY_WNM_CAPABILITY: only check btm cap now\n"));
				Status = EINVAL;
				os_free_mem(p_wnm_command);
				break;
			}
			if (copy_to_user(wrq->u.data.pointer, (PUCHAR)p_wnm_command, wrq->u.data.length)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("RT_QUERY_WNM_CAPABILITY: copy to user failed!\n"));
				Status = EFAULT;
				os_free_mem(p_wnm_command);
				break;
			}
			os_free_mem(p_wnm_command);
			break;
		}
#endif
#ifdef DOT11K_RRM_SUPPORT
		case RT_QUERY_RRM_CAPABILITY:
		{
			PUCHAR p_rrm_query_data = NULL;
			PMAC_TABLE_ENTRY pEntry = NULL;
			p_rrm_command_t p_rrm_command = NULL;
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
			PUINT8 p_cap = NULL;
			UINT16 TotalLen = 0;

			TotalLen = wrq->u.data.length;
			if (TotalLen != sizeof(*p_rrm_command) + MAC_ADDR_LEN + 8) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
					("RT_QUERY_RRM_CAPABILITY: length(%d) check failed\n",
					TotalLen));
				Status = EINVAL;
				break;
			}
			if (!pMbss->wdev.RrmCfg.bDot11kRRMEnable) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
					("%s(): RT_QUERY_RRM_CAPABILITY: rrm off, IF Index: %d\n", __func__, apidx));
				Status = EINVAL;
				break;
			}
			os_alloc_mem(NULL, (UCHAR **)&p_rrm_command, wrq->u.data.length);
			if (p_rrm_command == NULL) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					("RT_QUERY_RRM_CAPABILITY : no memory!!!\n"));
				Status = ENOMEM;
				break;
			}

			if (copy_from_user(p_rrm_command, wrq->u.data.pointer, wrq->u.data.length)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("RT_QUERY_RRM_CAPABILITY: copy from user failed!\n"));
				Status = EFAULT;
				os_free_mem(p_rrm_command);
				break;
			}

			p_rrm_query_data = p_rrm_command->command_body;

			/*first six bytes of data is sta mac*/
			pEntry = MacTableLookup(pAd, p_rrm_query_data);
			if (!pEntry ||
				!(IS_AKM_OPEN(pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.AKMMap) ||
				((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
				(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED)))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("RT_QUERY_RRM_CAPABILITY: "));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					("STA(%02x:%02x:%02x:%02x:%02x:%02x) not associates with AP!\n",
					PRINT_MAC(p_rrm_query_data)));
				Status = EINVAL;
				os_free_mem(p_rrm_command);
				break;
			}
			/*check for btm capablility*/
			p_cap = (UINT8 *)(p_rrm_query_data+MAC_ADDR_LEN);
			memset(p_cap, 0, 8);

			if (p_rrm_command->command_id ==
				OID_802_11_RRM_CMD_QUERY_CAP) {
				memcpy(p_cap, (PUCHAR)&(pEntry->RrmEnCap), sizeof(pEntry->RrmEnCap));
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("RT_QUERY_WNM_CAPABILITY: only check rrm cap now\n"));
				Status = EINVAL;
				os_free_mem(p_rrm_command);
				break;
			}
			if (copy_to_user(wrq->u.data.pointer, (PUCHAR)p_rrm_command, wrq->u.data.length)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("RT_QUERY_RRM_CAPABILITY: copy to user failed!\n"));
				Status = EFAULT;
				os_free_mem(p_rrm_command);
				break;
			}
			os_free_mem(p_rrm_command);
			break;
		}
#endif
#endif /* CONFIG_11KV_API_SUPPORT */

#ifdef WSC_AP_SUPPORT
#ifdef VENDOR_FEATURE7_SUPPORT
	case ARRIS_OID_WSC_QUERY_STATE: {
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer,
			&pAd->ApCfg.MBSSID[apidx].WscControl.WscState, wrq->u.data.length);
	}
	break;
#endif
	case RT_OID_WSC_QUERY_STATUS: {
		INT WscStatus;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_QUERY_STATUS\n"));
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			INT ApCliIdx = pObj->ioctl_if;

			APCLI_MR_APIDX_SANITY_CHECK(ApCliIdx);
			WscStatus = pAd->ApCfg.ApCliTab[ApCliIdx].wdev.WscControl.WscStatus;
		} else
#endif /* APCLI_SUPPORT */
			{
				WscStatus = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus;
			}

		wrq->u.data.length = sizeof(INT);

		if (copy_to_user(wrq->u.data.pointer, &WscStatus, wrq->u.data.length))
			Status = -EFAULT;

		break;
	}

	case RT_OID_WSC_PIN_CODE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_PIN_CODE\n"));
		wrq->u.data.length = sizeof(UINT);
		/*WscPinCode = GenerateWpsPinCode(pAd, FALSE, apidx); */
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

		WscPinCode = pWscControl->WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		break;
#ifdef APCLI_SUPPORT

	case RT_OID_APCLI_WSC_PIN_CODE:
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_APCLI_WSC_PIN_CODE\n"));
		wrq->u.data.length = sizeof(UINT);
		/*WscPinCode = GenerateWpsPinCode(pAd, TRUE, apidx); */
		WscPinCode = pAd->ApCfg.ApCliTab[apidx].wdev.WscControl.WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		break;
#endif /* APCLI_SUPPORT */

	case RT_OID_WSC_UUID:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_QUERY_UUID\n"));
		wrq->u.data.length = UUID_LEN_STR;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

		if (copy_to_user(wrq->u.data.pointer, &pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_MAC_ADDRESS:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_MAC_ADDRESS\n"));
		wrq->u.data.length = MAC_ADDR_LEN;

		if (copy_to_user(wrq->u.data.pointer, pAd->ApCfg.MBSSID[apidx].wdev.bssid, wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_CONFIG_STATUS:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_CONFIG_STATUS\n"));
		wrq->u.data.length = sizeof(UCHAR);

		if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus, wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING\n"));

		if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscState > WSC_STATE_WAIT_M2) {
			wrq->u.data.length = sizeof(WSC_PEER_DEV_INFO);

			if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscPeerInfo, wrq->u.data.length))
				Status = -EFAULT;
		} else
			Status = -EFAULT;

		break;

	case RT_OID_802_11_WSC_QUERY_PROFILE:
		wrq->u.data.length = sizeof(WSC_PROFILE);
		os_alloc_mem(pAd, (UCHAR **)&pProfile, sizeof(WSC_PROFILE));

		if (pProfile == NULL) {
			Status = -EFAULT;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_OID_802_11_WSC_QUERY_PROFILE fail!\n"));
			break;
		}

#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				APCLI_MR_APIDX_SANITY_CHECK(apidx);
				pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("IF(apcli : %d) RT_OID_802_11_WSC_QUERY_PROFILE :: This command is from apcli interface now.\n", apidx));
			} else
#endif /* APCLI_SUPPORT */
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

		RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
		NdisMoveMemory(pProfile, &pWscControl->WscProfile, sizeof(WSC_PROFILE));

		if ((pProfile->Profile[0].AuthType == WSC_AUTHTYPE_OPEN) && (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_NONE)) {
			pProfile->Profile[0].KeyLength = 0;
			NdisZeroMemory(pProfile->Profile[0].Key, 64);
		}

		if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			Status = -EFAULT;

		os_free_mem(pProfile);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_WSC_QUERY_PROFILE\n"));
		break;
#ifdef WSC_V2_SUPPORT

	case RT_OID_WSC_V2_SUPPORT:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_V2_SUPPORT (=%d)\n",
				 pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bEnableWpsV2));
		wrq->u.data.length = sizeof(BOOLEAN);

		if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bEnableWpsV2,
						 wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_FRAGMENT_SIZE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_WSC_FRAGMENT_SIZE (=%d)\n",
				 pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscFragSize));
		wrq->u.data.length = sizeof(USHORT);

		if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscFragSize, wrq->u.data.length))
			Status = -EFAULT;

		break;
#endif /* WSC_V2_SUPPORT */
#endif /* WSC_AP_SUPPORT */
#ifdef LLTD_SUPPORT

	case RT_OID_GET_LLTD_ASSO_TABLE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::Get LLTD association table\n"));

		if ((wrq->u.data.pointer == NULL) || (apidx != MAIN_MBSSID))
			Status = -EFAULT;
		else {
			INT						    i;
			RT_LLTD_ASSOICATION_TABLE	AssocTab;

			AssocTab.Num = 0;

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC)) {
					COPY_MAC_ADDR(AssocTab.Entry[AssocTab.Num].Addr, &pAd->MacTab.Content[i].Addr);
					AssocTab.Entry[AssocTab.Num].phyMode = pAd->ApCfg.MBSSID[apidx].wdev.PhyMode;
					AssocTab.Entry[AssocTab.Num].MOR = RateIdToMbps[pAd->ApCfg.MBSSID[apidx].MaxTxRate] * 2;
					AssocTab.Num += 1;
				}
			}

			wrq->u.data.length = sizeof(RT_LLTD_ASSOICATION_TABLE);

			if (copy_to_user(wrq->u.data.pointer, &AssocTab, wrq->u.data.length)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));
				Status = -EFAULT;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AssocTab.Num = %d\n", AssocTab.Num));
		}

		break;
#ifdef APCLI_SUPPORT

	case RT_OID_GET_REPEATER_AP_LINEAGE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Not Support : Get repeater AP lineage.\n"));
		break;
#endif /* APCLI_SUPPORT */
#endif /* LLTD_SUPPORT */
#ifdef DOT1X_SUPPORT

	case OID_802_DOT1X_CONFIGURATION:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::Get Radius setting(%lu)\n",
				 (ULONG)sizeof(DOT1X_CMM_CONF)));
		Dot1xIoctlQueryRadiusConf(pAd, wrq);
		break;

	case OID_802_DOT1X_QUERY_STA_AID:
		RTMPIoctlQueryStaAid(pAd, wrq);
		break;
#ifdef RADIUS_ACCOUNTING_SUPPORT

	case OID_802_DOT1X_QUERY_STA_DATA:
		RTMPIoctlQueryStaData(pAd, wrq);
		break;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
	case OID_802_DOT1X_QUERY_STA_RSN:
		RTMPIoctlQueryStaRsn(pAd, wrq);
		break;
#endif /* DOT1X_SUPPORT */

	case RT_OID_802_11_MAC_ADDRESS:
		wrq->u.data.length = MAC_ADDR_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.bssid, wrq->u.data.length);
		break;
#ifdef SNMP_SUPPORT

	case RT_OID_802_11_MANUFACTUREROUI:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_MANUFACTUREROUI\n"));
		wrq->u.data.length = ManufacturerOUI_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTURERNAME:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_MANUFACTURERNAME\n"));
		wrq->u.data.length = strlen(ManufacturerNAME);
		Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
		break;

	case RT_OID_802_11_RESOURCETYPEIDNAME:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_RESOURCETYPEIDNAME\n"));
		wrq->u.data.length = strlen(ResourceTypeIdName);
		Status = copy_to_user(wrq->u.data.pointer, ResourceTypeIdName, wrq->u.data.length);
		break;

	case RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED: {
		ULONG ulInfo;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED\n"));
		ulInfo = 1; /* 1 is support wep else 2 is not support. */
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;
	}

	case RT_OID_802_11_POWERMANAGEMENTMODE: {
		ULONG ulInfo;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_POWERMANAGEMENTMODE\n"));
		ulInfo = 1; /* 1 is power active else 2 is power save. */
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;
	}

	case OID_802_11_WEPDEFAULTKEYVALUE:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_WEPDEFAULTKEYVALUE\n"));
		pKeyIdxValue = wrq->u.data.pointer;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("KeyIdxValue.KeyIdx = %d,\n", pKeyIdxValue->KeyIdx));
		valueLen = pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen;
		NdisMoveMemory(pKeyIdxValue->Value,
					   &pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].Key,
					   valueLen);
		pKeyIdxValue->Value[valueLen] = '\0';
		wrq->u.data.length = sizeof(DefaultKeyIdxValue);
		Status = copy_to_user(wrq->u.data.pointer, pKeyIdxValue, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("DefaultKeyId = %d, total len = %d, str len=%d, KeyValue= %02x %02x %02x %02x\n",
				  pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId, wrq->u.data.length,
				  pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen,
				  pAd->SharedKey[pObj->ioctl_if][0].Key[0],
				  pAd->SharedKey[pObj->ioctl_if][1].Key[0],
				  pAd->SharedKey[pObj->ioctl_if][2].Key[0],
				  pAd->SharedKey[pObj->ioctl_if][3].Key[0]));
		break;

	case OID_802_11_WEPDEFAULTKEYID:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_WEPDEFAULTKEYID\n"));
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DefaultKeyId =%d\n",
				 pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId));
		break;

	case RT_OID_802_11_WEPKEYMAPPINGLENGTH:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_WEPKEYMAPPINGLENGTH\n"));
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer,
							  &pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen,
							  wrq->u.data.length);
		break;

	case OID_802_11_SHORTRETRYLIMIT:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_SHORTRETRYLIMIT\n"));
		wrq->u.data.length = sizeof(ULONG);
		ShortRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ShortRetryLimit =%ld\n", ShortRetryLimit));
		Status = copy_to_user(wrq->u.data.pointer, &ShortRetryLimit, wrq->u.data.length);
		break;

	case OID_802_11_LONGRETRYLIMIT:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_LONGRETRYLIMIT\n"));
		wrq->u.data.length = sizeof(ULONG);
		LongRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("LongRetryLimit =%ld\n", LongRetryLimit));
		Status = copy_to_user(wrq->u.data.pointer, &LongRetryLimit, wrq->u.data.length);
		break;

	case RT_OID_802_11_PRODUCTID:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_PRODUCTID\n"));
#ifdef RTMP_PCI_SUPPORT

		if (IS_PCI_INF(pAd)) {
			USHORT  device_id;

			if (((POS_COOKIE)pAd->OS_Cookie)->pci_dev != NULL)
				pci_read_config_word(((POS_COOKIE)pAd->OS_Cookie)->pci_dev, PCI_DEVICE_ID, &device_id);
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" pci_dev = NULL\n"));

			snprintf((RTMP_STRING *)snmp_tmp, sizeof(snmp_tmp), "%04x %04x\n", NIC_PCI_VENDOR_ID, device_id);
		}

#endif /* RTMP_PCI_SUPPORT */
		wrq->u.data.length = strlen((RTMP_STRING *) snmp_tmp);
		Status = copy_to_user(wrq->u.data.pointer, snmp_tmp, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTUREID:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_MANUFACTUREID\n"));
		wrq->u.data.length = strlen(ManufacturerNAME);
		Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
		break;
#endif /* SNMP_SUPPORT */
#if (defined(SNMP_SUPPORT) || defined(WH_EZ_SETUP) || defined(VENDOR_FEATURE6_SUPPORT))

	case OID_802_11_CURRENTCHANNEL:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_CURRENTCHANNEL\n"));
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.channel, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Status=%d\n", Status));
		break;
#endif

	case OID_802_11_STATISTICS:
		os_alloc_mem(pAd, (UCHAR **)&pStatistics, sizeof(NDIS_802_11_STATISTICS));

		if (pStatistics) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_STATISTICS\n"));
			/* add the most up-to-date h/w raw counters into software counters */
			/*NICUpdateRawCountersNew(pAd);*/
			pStatistics->TransmittedFragmentCount.QuadPart = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart +
					pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart;
			pStatistics->MulticastTransmittedFrameCount.QuadPart = pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart;
			pStatistics->FailedCount.QuadPart = pAd->WlanCounters[0].FailedCount.QuadPart;
			pStatistics->RetryCount.QuadPart = pAd->WlanCounters[0].RetryCount.QuadPart;
			pStatistics->MultipleRetryCount.QuadPart = pAd->WlanCounters[0].MultipleRetryCount.QuadPart;
			pStatistics->RTSSuccessCount.QuadPart = pAd->WlanCounters[0].RTSSuccessCount.QuadPart;
			pStatistics->RTSFailureCount.QuadPart = pAd->WlanCounters[0].RTSFailureCount.QuadPart;
			pStatistics->ACKFailureCount.QuadPart = pAd->WlanCounters[0].ACKFailureCount.QuadPart;
			pStatistics->FrameDuplicateCount.QuadPart = pAd->WlanCounters[0].FrameDuplicateCount.QuadPart;
			pStatistics->ReceivedFragmentCount.QuadPart = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
			pStatistics->MulticastReceivedFrameCount.QuadPart = pAd->WlanCounters[0].MulticastReceivedFrameCount.QuadPart;
#ifdef DBG
			pStatistics->FCSErrorCount = pAd->RalinkCounters.RealFcsErrCount;
#else
			pStatistics->FCSErrorCount.QuadPart = pAd->WlanCounters[0].FCSErrorCount.QuadPart;
			pStatistics->FrameDuplicateCount.u.LowPart = pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart / 100;
#endif
			pStatistics->TransmittedFrameCount.QuadPart = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart;
			pStatistics->WEPUndecryptableCount.QuadPart = pAd->WlanCounters[0].WEPUndecryptableCount.QuadPart;
			wrq->u.data.length = sizeof(NDIS_802_11_STATISTICS);
			Status = copy_to_user(wrq->u.data.pointer, pStatistics, wrq->u.data.length);
			os_free_mem(pStatistics);
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_STATISTICS(mem alloc failed)\n"));
			Status = -EFAULT;
		}

		break;

	case RT_OID_802_11_PER_BSS_STATISTICS: {
		PMBSS_STATISTICS pMbssStat;
		INT apidx = pObj->ioctl_if;
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];

		os_alloc_mem(pAd, (UCHAR **) &pMbssStat, sizeof(MBSS_STATISTICS));
		NdisZeroMemory(pMbssStat, sizeof(MBSS_STATISTICS));
		pMbssStat->TransmittedByteCount = pMbss->TransmittedByteCount;
		pMbssStat->ReceivedByteCount =  pMbss->ReceivedByteCount;
		pMbssStat->TxCount =  pMbss->TxCount;
		pMbssStat->RxCount =  pMbss->RxCount;
		pMbssStat->RxErrorCount =  pMbss->RxErrorCount;
		pMbssStat->RxDropCount =  pMbss->RxDropCount;
		pMbssStat->TxErrorCount =  pMbss->TxErrorCount;
		pMbssStat->TxDropCount =  pMbss->TxDropCount;
		pMbssStat->ucPktsTx =  pMbss->ucPktsTx;
		pMbssStat->ucPktsRx =  pMbss->ucPktsRx;
		pMbssStat->mcPktsTx =  pMbss->mcPktsTx;
		pMbssStat->mcPktsRx =  pMbss->mcPktsRx;
		pMbssStat->bcPktsTx = pMbss->bcPktsTx;
		pMbssStat->bcPktsRx = pMbss->bcPktsRx;
		wrq->u.data.length = sizeof(MBSS_STATISTICS);
		Status = copy_to_user(wrq->u.data.pointer, pMbssStat, wrq->u.data.length);
		os_free_mem(pMbssStat);
	}
	break;
#ifdef HOSTAPD_SUPPORT

	case HOSTAPD_OID_GETWPAIE:/*report wpa ie of the new station to hostapd. */
		if (wrq->u.data.length != sizeof(wpaie))
			Status = -EINVAL;
		else if (copy_from_user(&wpaie, wrq->u.data.pointer, IEEE80211_ADDR_LEN))
			Status = -EFAULT;
		else {
			pEntry = MacTableLookup(pAd, wpaie.wpa_macaddr);

			if (!pEntry) {
				Status = -EINVAL;
				break;
			}

			NdisZeroMemory(wpaie.rsn_ie, sizeof(wpaie.rsn_ie));

			/* For WPA1, RSN_IE=221 */
			if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK)
				|| (pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
				|| (pEntry->AuthMode == Ndis802_11AuthModeWPA1WPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
			   ) {
				int ielen = pEntry->RSNIE_Len;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pEntry->RSNIE_Len=%d\n", pEntry->RSNIE_Len));

				if (ielen > sizeof(wpaie.rsn_ie))
					ielen = sizeof(wpaie.rsn_ie) - 1;

				p = wpaie.rsn_ie;
				hex_dump("HOSTAPD_OID_GETWPAIE woody==>pEntry->RSN_IE", (unsigned char *)pEntry->RSN_IE, ielen);
				NdisMoveMemory(p, pEntry->RSN_IE, ielen);
			}
		}

		if (copy_to_user(wrq->u.data.pointer, &wpaie, sizeof(wpaie)))
			Status = -EFAULT;

		break;

	case HOSTAPD_OID_GET_SEQ:/*report txtsc to hostapd. */
		pMbss = &pAd->ApCfg.MBSSID[apidx];

		if (wrq->u.data.length != sizeof(ik))
			Status = -EINVAL;
		else if (copy_from_user(&ik, wrq->u.data.pointer, IEEE80211_ADDR_LEN))
			Status = -EFAULT;
		else {
			NdisZeroMemory(&ik.ik_keytsc, sizeof(ik.ik_keytsc));
			p = (unsigned char *)&ik.ik_keytsc;
			NdisMoveMemory(p + 2, pAd->SharedKey[apidx][pMbss->DefaultKeyId].TxTsc, 6);

			if (copy_to_user(wrq->u.data.pointer, &ik, sizeof(ik)))
				Status = -EFAULT;
		}

		break;

	case HOSTAPD_OID_GET_1X_GROUP_KEY:/*report default group key to hostapd. */
		pMbss = &pAd->ApCfg.MBSSID[apidx];

		if (wrq->u.data.length != sizeof(group_key))
			Status = -EINVAL;
		else {
			if (pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen != 0 && pAd->SharedKey[apidx][pMbss->DefaultKeyId].Key != NULL) {
				group_key.ik_keyix = pMbss->DefaultKeyId;
				group_key.ik_keylen = pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen;
				NdisMoveMemory(group_key.ik_keydata, pAd->SharedKey[apidx][pMbss->DefaultKeyId].Key,
							   pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen);

				if (copy_to_user(wrq->u.data.pointer, &group_key, sizeof(group_key)))
					Status = -EFAULT;
			}
		}

		break;
#endif/*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	case OID_GEN_MEDIA_CONNECT_STATUS: {
		ULONG ApCliIdx = pObj->ioctl_if;
		NDIS_MEDIA_STATE MediaState;
		PMAC_TABLE_ENTRY pEntry;
		STA_TR_ENTRY *tr_entry;
		PAPCLI_STRUCT pApCliEntry;

		if (pObj->ioctl_if_type != INT_APCLI) {
			Status = -EOPNOTSUPP;
			break;
		}

		APCLI_MR_APIDX_SANITY_CHECK(ApCliIdx);
		pApCliEntry = &pAd->ApCfg.ApCliTab[ApCliIdx];

		if (pApCliEntry->MacTabWCID >= MAX_LEN_OF_MAC_TABLE) {
			Status = -EOPNOTSUPP;
			break;
		}

		pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];

		if (!IS_ENTRY_APCLI(pEntry) && !IS_ENTRY_REPEATER(pEntry)) {
			Status = -EOPNOTSUPP;
			break;
		}

		if ((pAd->ApCfg.ApCliTab[ApCliIdx].Valid == TRUE)
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			MediaState = NdisMediaStateConnected;
		else
			MediaState = NdisMediaStateDisconnected;

		wrq->u.data.length = sizeof(NDIS_MEDIA_STATE);
		Status = copy_to_user(wrq->u.data.pointer, &MediaState, wrq->u.data.length);
	}
	break;
#endif /* APCLI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT

	case RT_OID_802_11_SNR_0:
		if (wdev && wdev->LastSNR0 > 0) {
			ULONG ulInfo;

			ulInfo = ConvertToSnr(pAd, wdev->LastSNR0);
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_SNR_0(0x=%lx)\n", ulInfo));
		} else
			Status = -EFAULT;

		break;

	case RT_OID_802_11_SNR_1:
		if (wdev && (pAd->Antenna.field.RxPath > 1) && (wdev->LastSNR1 > 0)) {
			ULONG ulInfo;

			ulInfo = ConvertToSnr(pAd, wdev->LastSNR1);
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_SNR_1(%lx, LastSNR1=%d)\n", ulInfo,
					 wdev->LastSNR1));
		} else
			Status = -EFAULT;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::RT_OID_802_11_SNR_1, Status=%d\n", Status));
		break;
#endif /* RTMP_RBUS_SUPPORT */
	case OID_802_11_ACL_LIST:
		if (wrq->u.data.length < sizeof(RT_802_11_ACL))
			Status = -EINVAL;
		else
			Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));

		break;
#ifdef CONFIG_HOTSPOT
#ifdef CONFIG_DOT11V_WNM

	case OID_802_11_WNM_IPV4_PROXY_ARP_LIST: {
		BSS_STRUCT *pMbss;
		PUCHAR pProxyARPTable;
		UINT32 ARPTableLen;

		pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		ARPTableLen = IPv4ProxyARPTableLen(pAd, pMbss);
		os_alloc_mem(NULL, &pProxyARPTable, ARPTableLen);
		GetIPv4ProxyARPTable(pAd, pMbss, &pProxyARPTable);
		wrq->u.data.length = ARPTableLen;
		Status = copy_to_user(wrq->u.data.pointer, pProxyARPTable, ARPTableLen);
		os_free_mem(pProxyARPTable);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::OID_802_11_WNM_PROXY_ARP_LIST\n"));
	break;

	case OID_802_11_WNM_IPV6_PROXY_ARP_LIST: {
		BSS_STRUCT *pMbss;
		PUCHAR pProxyARPTable;
		UINT32 ARPTableLen;

		pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		ARPTableLen = IPv6ProxyARPTableLen(pAd, pMbss);
		os_alloc_mem(NULL, &pProxyARPTable, ARPTableLen);
		GetIPv6ProxyARPTable(pAd, pMbss, &pProxyARPTable);
		wrq->u.data.length = ARPTableLen;
		Status = copy_to_user(wrq->u.data.pointer, pProxyARPTable, ARPTableLen);
		os_free_mem(pProxyARPTable);
	}
	break;
#endif

	case OID_802_11_SECURITY_TYPE: {
		BSS_STRUCT *pMbss;
		PUCHAR pType;
		struct security_type *SecurityType;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Query:OID_802_11_SECURITY_TYPE\n"));
		os_alloc_mem(NULL, &pType, sizeof(*SecurityType));
		SecurityType = (struct security_type *)pType;
		pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		SecurityType->ifindex = pObj->ioctl_if;
		SecurityType->auth_mode = SecAuthModeNewToOld(pMbss->wdev.SecConfig.AKMMap);
		SecurityType->encryp_type = SecEncryModeNewToOld(pMbss->wdev.SecConfig.PairwiseCipher);
		wrq->u.data.length = sizeof(*SecurityType);
		Status = copy_to_user(wrq->u.data.pointer, pType, sizeof(*SecurityType));
		os_free_mem(pType);
	}
	break;

	case OID_802_11_HS_BSSID: {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		wrq->u.data.length = 6;
		Status = copy_to_user(wrq->u.data.pointer, pMbss->wdev.bssid, 6);
	}
	break;
#ifdef CONFIG_HOTSPOT_R2

	case OID_802_11_HS_OSU_SSID: {
		wrq->u.data.length = pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen; /* +2; */
		/* tmpbuf[0] = IE_SSID; */
		/* tmpbuf[1] = pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen; */
		/* memcpy(&tmpbuf[2], pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid, pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen); */
		/* Status = copy_to_user(wrq->u.data.pointer, tmpbuf, wrq->u.data.length); */
		Status = copy_to_user(wrq->u.data.pointer, pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,
							  pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\033[1;32m %s, %u OID_802_11_HS_OSU_SSID [%s]\033[0m\n"
				 , __func__, __LINE__, pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid));
		/* this is an osu ssid , disable 11U capability */
		pAd->ApCfg.MBSSID[pObj->ioctl_if].GASCtrl.b11U_enable = FALSE;
	}
	break;
#endif /* CONFIG_HOTSPOT_R2 */
#endif
#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef DOT11_N_SUPPORT
		case ARRIS_OID_HTEXTCHA_QUERY_VALUE: {
			PUCHAR Buf = NULL;

			os_alloc_mem(pAd, (UCHAR **)&Buf, wrq->u.data.length);
			if (!Buf) {
				Status = -EINVAL;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Memory is not available\n"));
			}
			Show_HtExtcha_Proc(pAd, Buf, wrq->u.data.length);
			Status = copy_to_user(wrq->u.data.pointer, Buf, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HtExtCha is:%s", Buf));
			os_free_mem(Buf);
		}
		break;
#endif
#endif
#ifdef WIFI_SPECTRUM_SUPPORT
	case OID_802_11_WIFISPECTRUM_GET_CAPTURE_BW: {
		UINT32 CapNode;
		UCHAR CapBw;

		CapNode = Get_System_CapNode_Info(pAd);
		CapBw = Get_System_Bw_Info(pAd, CapNode);
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &CapBw, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("Query::OID_802_11_WIFISPECTRUM_GET_CAPTURE_BW CapBw = %d\n", CapBw));
	}
	break;

	case OID_802_11_WIFISPECTRUM_GET_CENTRAL_FREQ: {
		UINT32 CapNode;
		USHORT CenFreq;

		CapNode = Get_System_CapNode_Info(pAd);
		CenFreq = Get_System_CenFreq_Info(pAd, CapNode);
		wrq->u.data.length = sizeof(USHORT);
		Status = copy_to_user(wrq->u.data.pointer, &CenFreq, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("Query::OID_802_11_WIFISPECTRUM_GET_CENTRAL_FREQ CenFreq = %d\n", CenFreq));
	}
	break;
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef MT_DFS_SUPPORT
	case OID_DFS_ZERO_WAIT:
		Status = ZeroWaitDfsQueryCmdHandler(pAd, wrq);
		break;
#endif

	default:
		Status = -EOPNOTSUPP;

		if (Status == -EOPNOTSUPP) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("Query::unknown IOCTL's subcmd = 0x%08x, apidx=%d\n", cmd, apidx));
		}

		break;
	}

	return Status;
}

/*
    ==========================================================================
    Description:
	Set Country Code.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config CountryCode status!!\n",
				 __func__));
		return TRUE;
	}

#endif /* RF_LOCKDOWN */
#ifdef EXT_BUILD_CHANNEL_LIST
	/* reset temp table status */
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(arg) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		NdisZeroMemory(pAd->CommonCfg.CountryCode,
					   sizeof(pAd->CommonCfg.CountryCode));
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_CountryCode_Proc::(bCountryFlag=%d, CountryCode=%s)\n",
			 pAd->CommonCfg.bCountryFlag, pAd->CommonCfg.CountryCode));
	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ChGeography_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Geography;

	Geography = os_str_tol(arg, 0, 10);

	if (Geography <= BOTH)
		pAd->CommonCfg.Geography = Geography;
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Set_ChannelGeography_Proc::(wrong setting. 0: Out-door, 1: in-door, 2: both)\n"));

	pAd->CommonCfg.CountryCode[2] =
		(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ChannelGeography_Proc:: Geography = %s\n",
			 pAd->CommonCfg.Geography == ODOR ? "out-door" : (pAd->CommonCfg.Geography == IDOR ? "in-door" : "both")));
	/* After Set ChGeography need invoke SSID change procedural again for Beacon update. */
	/* it's no longer necessary since APStartUp will rebuild channel again. */
	/*BuildChannelListEx(pAd); */
	return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST */


/*
    ==========================================================================
    Description:
	Set Country String.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryString_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT   index = 0;
	INT   success = TRUE;
	RTMP_STRING name_buffer[40] = {0};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	BOOLEAN IsSupport2G = HcIsRfSupport(pAd, RFIC_24GHZ);
#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(arg) <= 38) {
		if (strlen(arg) < 4) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Set_CountryString_Proc::Parameter of CountryString are too short !\n"));
			return FALSE;
		}

		for (index = 0; index < strlen(arg); index++) {
			if ((arg[index] >= 'a') && (arg[index] <= 'z'))
				arg[index] = toupper(arg[index]);
		}

		for (index = 0; index < NUM_OF_COUNTRIES; index++) {
			NdisZeroMemory(name_buffer, 40);
			snprintf(name_buffer, sizeof(name_buffer), "\"%s\"", (RTMP_STRING *) allCountry[index].pCountryName);

			if (strncmp((RTMP_STRING *) allCountry[index].pCountryName, arg, strlen(arg)) == 0)
				break;
			else if (strncmp(name_buffer, arg, strlen(arg)) == 0)
				break;
		}

		if (index == NUM_OF_COUNTRIES)
			success = FALSE;
	} else
		success = FALSE;

	if (success == TRUE) {
		if (pAd->CommonCfg.CountryRegion & 0x80) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Set_CountryString_Proc::parameter of CountryRegion in eeprom is programmed\n"));
			success = FALSE;
		} else {
			success = FALSE;

			if (IsSupport2G) {
				if (allCountry[index].SupportGBand == TRUE) {
					pAd->CommonCfg.CountryRegion = (UCHAR) allCountry[index].RegDomainNum11G;
					success = TRUE;
				} else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The Country are not Support G Band Channel\n"));
			}

			if (IsSupport5G) {
				if (allCountry[index].SupportABand == TRUE) {
					pAd->CommonCfg.CountryRegionForABand = (UCHAR) allCountry[index].RegDomainNum11A;
					success = TRUE;
				} else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The Country are not Support A Band Channel\n"));
			}
		}
	}

	if (success == TRUE) {
		os_zero_mem(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		os_move_mem(pAd->CommonCfg.CountryCode, allCountry[index].IsoName, 2);
		pAd->CommonCfg.CountryCode[2] = ' ';
		/* After Set ChGeography need invoke SSID change procedural again for Beacon update. */
		/* it's no longer necessary since APStartUp will rebuild channel again. */
		/*BuildChannelList(pAd); */
		pAd->CommonCfg.bCountryFlag = TRUE;
		/* if set country string, driver needs to be reset */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Set_CountryString_Proc::(CountryString=%s CountryRegin=%d CountryCode=%s)\n",
				  allCountry[index].pCountryName, pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryCode));
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_CountryString_Proc::Parameters out of range\n"));

	return success;
}

VOID restart_ap(void *wdev_obj)
{
	struct wifi_dev *wdev = wdev_obj;
	BSS_STRUCT *mbss = wdev->func_dev;

	UpdateBeaconHandler(
		wdev->sys_handle,
		wdev,
		BCN_UPDATE_DISABLE_TX);
	wifi_sys_linkdown(wdev);
	wifi_sys_close(wdev);
	wifi_sys_open(wdev);
	wifi_sys_linkup(wdev, NULL);
	if (IS_SECURITY(&wdev->SecConfig))
		mbss->CapabilityInfo |= 0x0010;
	else
		mbss->CapabilityInfo &= (~0x0010);
	UpdateBeaconHandler(wdev->sys_handle, wdev, BCN_UPDATE_IE_CHG);
}

/*
    ==========================================================================
    Description:
	Set SSID
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT success = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	struct DOT11_H *pDot11h = NULL;

	if (((pObj->ioctl_if < HW_BEACON_MAX_NUM)) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
		struct wifi_dev *wdev = NULL;

		pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		wdev = &pMbss->wdev;
		NdisZeroMemory(pMbss->Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pMbss->Ssid, arg, strlen(arg));
		pMbss->SsidLen = (UCHAR)strlen(arg);
		success = TRUE;

		if (wdev == NULL)
			return FALSE;

		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			return FALSE;
		{
			ApSendBroadcastDeauth(pAd, wdev);
			if (IS_SECURITY(&wdev->SecConfig))
				pMbss->CapabilityInfo |= 0x0010;
			else
				pMbss->CapabilityInfo &= ~(0x0010);
			APSecInit(pAd, wdev);
			restart_ap(&pMbss->wdev);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("I/F(ra%d) Set_SSID_Proc::(Len=%d,Ssid=%s)\n",
				pObj->ioctl_if,
				pMbss->SsidLen, pMbss->Ssid));
		}
	} else
		success = FALSE;

	return success;
}


/*
    ==========================================================================
    Description:
	Set TxRate
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_TxRate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	NdisZeroMemory(pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRates, MAX_LEN_OF_SUPPORTED_RATES);
	pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRatesIndex = os_str_tol(arg, 0, 10);
	/* todo RTMPBuildDesireRate(pAd, pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRatesIndex); */
	/*todo MlmeUpdateTxRates(pAd); */
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set BasicRate
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_BasicRate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	ULONG BasicRateBitmap;

	BasicRateBitmap = (ULONG) os_str_tol(arg, 0, 10);

	if (BasicRateBitmap > 4095) /* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
		return FALSE;

	pAd->CommonCfg.BasicRateBitmap = BasicRateBitmap;
	pAd->CommonCfg.BasicRateBitmapOld = BasicRateBitmap;
	MlmeUpdateTxRates(pAd, FALSE, (UCHAR)pObj->ioctl_if);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BasicRate_Proc::(BasicRateBitmap=0x%08lx)\n",
			 pAd->CommonCfg.BasicRateBitmap));
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set Beacon Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BeaconPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	USHORT BeaconPeriod;
	INT   success = FALSE;

	BeaconPeriod = (USHORT) os_str_tol(arg, 0, 10);

	if ((BeaconPeriod >= 20) && (BeaconPeriod < 1024)) {
		pAd->CommonCfg.BeaconPeriod = BeaconPeriod;
		success = TRUE;
#ifdef AP_QLOAD_SUPPORT
		/* re-calculate QloadBusyTimeThreshold */
		QBSS_LoadAlarmReset(pAd);
#endif /* AP_QLOAD_SUPPORT */
	} else
		success = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BeaconPeriod_Proc::(BeaconPeriod=%d)\n",
			 pAd->CommonCfg.BeaconPeriod));
	return success;
}

/*
    ==========================================================================
    Description:
	Set Dtim Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_DtimPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	USHORT DtimPeriod;
	INT   success = FALSE;

	DtimPeriod = (USHORT) os_str_tol(arg, 0, 10);

	if ((DtimPeriod >= 1) && (DtimPeriod <= 255)) {
		pAd->ApCfg.DtimPeriod = DtimPeriod;
		success = TRUE;
	} else
		success = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_DtimPeriod_Proc::(DtimPeriod=%d)\n", pAd->ApCfg.DtimPeriod));
	return success;
}


/*
    ==========================================================================
    Description:
	Disable/enable OLBC detection manually
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_OLBCDetection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	switch (os_str_tol(arg, 0, 10)) {
	case 0: /*enable OLBC detect */
		pAd->CommonCfg.DisableOLBCDetect = 0;
		break;

	case 1: /*disable OLBC detect */
		pAd->CommonCfg.DisableOLBCDetect = 1;
		break;

	default:  /*Invalid argument */
		return FALSE;
	}

	return TRUE;
}


/*
*
*/
INT set_qiscdump_proc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
		pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	PNET_DEV ndev = NULL;

	if (wdev == NULL)
		return FALSE;

	ndev = wdev->if_dev;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): wdev=%p, idx=%d, dev=%p, %s\n",
		__func__, wdev, wdev->wdev_idx, ndev, ndev->name));

#ifdef CONFIG_DBG_QDISC
	if (wdev && wdev->func_dev)
		os_system_tx_queue_dump(ndev);
#endif
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set WmmCapable Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN	bWmmCapable;
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev;

	bWmmCapable = os_str_tol(arg, 0, 10);
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	if (bWmmCapable == 1)
		wdev->bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		wdev->bWmmCapable = FALSE;
	else
		return FALSE;  /*Invalid argument */

	pAd->ApCfg.MBSSID[pObj->ioctl_if].bWmmCapableOrg =
		pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.bWmmCapable;
#ifdef RTL865X_FAST_PATH

	if (!isFastPathCapable(pAd)) {
		rtlairgo_fast_tx_unregister();
		rtl865x_extDev_unregisterUcastTxDev(pAd->net_dev);
	}

#endif
#ifdef DOT11_N_SUPPORT
	/*Sync with the HT relate info. In N mode, we should re-enable it */
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WmmCapable_Proc::(bWmmCapable=%d)\n",
			 pObj->ioctl_if, wdev->bWmmCapable));
	return TRUE;
}


INT	Set_AP_PerMbssMaxStaNum_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT			apidx = pObj->ioctl_if;

	return ApCfg_Set_PerMbssMaxStaNum_Proc(pAd, apidx, arg);
}

/*
    ==========================================================================
    Description:
	Set session idle timeout
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_IdleTimeout_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	return ApCfg_Set_IdleTimeout_Proc(pAd, arg);
}
/*
    ==========================================================================
    Description:
	Set No Forwarding Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_NoForwarding_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	ULONG NoForwarding;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	NoForwarding = os_str_tol(arg, 0, 10);

	if (NoForwarding == 1)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].IsolateInterStaTraffic = TRUE;
	else if (NoForwarding == 0)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].IsolateInterStaTraffic = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_NoForwarding_Proc::(NoForwarding=%ld)\n",
			 pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].IsolateInterStaTraffic));
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set No Forwarding between each SSID
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_NoForwardingBTNSSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	ULONG NoForwarding;

	NoForwarding = os_str_tol(arg, 0, 10);

	if (NoForwarding == 1)
		pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
	else if (NoForwarding == 0)
		pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_NoForwardingBTNSSID_Proc::(NoForwarding=%ld)\n",
			 pAd->ApCfg.IsolateInterStaTrafficBTNBSSID));
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set Hide SSID Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HideSSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN bHideSsid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	bHideSsid = os_str_tol(arg, 0, 10);

	if (bHideSsid == 1)
		bHideSsid = TRUE;
	else if (bHideSsid == 0)
		bHideSsid = FALSE;
	else
		return FALSE;  /*Invalid argument */

	if (pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid != bHideSsid)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid = bHideSsid;

#ifdef WSC_V2_SUPPORT

	if (pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.WscV2Info.bEnableWpsV2)
		WscOnOff(pAd, pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid);

#endif /* WSC_V2_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_HideSSID_Proc::(HideSSID=%d)\n", pObj->ioctl_if,
			 pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set VLAN's ID field
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLANID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	wdev->VLAN_VID = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_VLANID_Proc::(VLAN_VID=%d)\n",
			 pObj->ioctl_if, wdev->VLAN_VID));
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set VLAN's priority field
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLANPriority_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	wdev->VLAN_Priority = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_VLANPriority_Proc::(VLAN_Priority=%d)\n",
			 pObj->ioctl_if, wdev->VLAN_Priority));
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set enable or disable carry VLAN in the air
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLAN_TAG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN	bVLAN_Tag;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;

	bVLAN_Tag = os_str_tol(arg, 0, 10);

	if (bVLAN_Tag == 1)
		bVLAN_Tag = TRUE;
	else if (bVLAN_Tag == 0)
		bVLAN_Tag = FALSE;
	else
		return FALSE;  /* Invalid argument */

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	wdev->bVLAN_Tag = bVLAN_Tag;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_VLAN_TAG_Proc::(VLAN_Tag=%d)\n",
			 pObj->ioctl_if, wdev->bVLAN_Tag));
	return TRUE;
}


INT	Set_AP_WpaMixPairCipher_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	/*
		In WPA-WPA2 mix mode, it provides a more flexible cipher combination.
		-	WPA-AES and WPA2-TKIP
		-	WPA-AES and WPA2-TKIPAES
		-	WPA-TKIP and WPA2-AES
		-	WPA-TKIP and WPA2-TKIPAES
		-	WPA-TKIPAES and WPA2-AES
		-	WPA-TKIPAES and WPA2-TKIP
		-	WPA-TKIPAES and WPA2-TKIPAES (default)
	 */
	Set_SecAuthMode_Proc(pAd, arg);
	Set_SecEncrypType_Proc(pAd, arg);
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set WPA rekey interval value
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_RekeyInterval_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ULONG value_interval;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;
	value_interval = os_str_tol(arg, 0, 10);

	if ((value_interval >= 10) && (value_interval < MAX_GROUP_REKEY_INTERVAL))
		pSecConfig->GroupReKeyInterval = value_interval;
	else /*Default*/
		pSecConfig->GroupReKeyInterval = DEFAULT_GROUP_REKEY_INTERVAL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) GroupKey ReKeyInterval=%ld seconds\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyInterval));
	return TRUE;
}

#ifdef SPECIFIC_TX_POWER_SUPPORT
INT Set_AP_PKT_PWR(
	IN  PRTMP_ADAPTER    pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	INT input;

	input = os_str_tol(arg, 0, 10);

	/*
	  Tx_PWR_ADJ[3:0] From 0 to 7 is Positive & add with Tx Power (dB),
	      From 8 to 15 is minus with Tx Power mapping to -16 to -2 (step by 2),
	      Default value: 0.

	  [0x13BC]TX_ALC_MONITOR, 13:8
		  TX_ALC_REQ_ADJ TX ALC Req Saturated[5:0], unit (0.5dB)
	*/

	if ((input >= 0) && (input <= 15))
		pAd->ApCfg.MBSSID[apidx].TxPwrAdj = input;
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP[%d]->PktPwr: Out of Range\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP[%d]->PktPwr: %d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].TxPwrAdj));
	return TRUE;
}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
/*
    ==========================================================================
    Description:
	Set WPA rekey method
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_RekeyMethod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (rtstrcasecmp(arg, "TIME") == TRUE)
		pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_TIME;
	else if (rtstrcasecmp(arg, "PKT") == TRUE)
		pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_PACKET;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) GroupKey ReKeyMethod=%x\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyMethod));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set PMK-cache period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_PMKCachePeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	UINT32 val = os_str_tol(arg, 0, 10);

	pAd->ApCfg.MBSSID[apidx].PMKCachePeriod = val * 60 * OS_HZ;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) Set_AP_PMKCachePeriod_Proc=%ld\n",
			 apidx, pAd->ApCfg.MBSSID[apidx].PMKCachePeriod));
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set AssocReq RSSI Threshold to reject STA with weak signal.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_AP_ASSOC_REQ_RSSI_THRESHOLD(
	IN  PRTMP_ADAPTER    pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	UINT j;
	CHAR rssi;

	rssi = os_str_tol(arg, 0, 10);

	if (rssi == 0)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Disable AP_ASSOC_REQ_RSSI_THRESHOLD\n"));
	else if (rssi > 0 || rssi < -100) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_AP_ASSOC_REQ_RSSI_THRESHOLD Value Error.\n"));
		return FALSE;
	}

	pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold = rssi;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) Set_AP_ASSOC_REQ_RSSI_THRESHOLD=%d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold));

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d. ==> %d\n", j, pAd->ApCfg.MBSSID[j].AssocReqRssiThreshold));

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set lower limit for AP kicking out a STA.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_AP_KickStaRssiLow_Proc(
	IN  PRTMP_ADAPTER    pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	UINT j;
	CHAR rssi;

	rssi = os_str_tol(arg, 0, 10);

	if (rssi == 0)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Disable RssiLowForStaKickOut Function\n"));
	else if (rssi > 0 || rssi < -100) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RssiLowForStaKickOut Value Error.\n"));
		return FALSE;
	}

	pAd->ApCfg.MBSSID[apidx].RssiLowForStaKickOut = rssi;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) RssiLowForStaKickOut=%d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].RssiLowForStaKickOut));

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%d. ==> %d\n", j, pAd->ApCfg.MBSSID[j].RssiLowForStaKickOut));

	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set Access ctrol policy
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AccessPolicy_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	switch (os_str_tol(arg, 0, 10)) {
	case 0: /*Disable */
		pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = 0;
		break;

	case 1: /* Allow All, and ACL is positive. */
		pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = 1;
		break;

	case 2: /* Reject All, and ACL is negative. */
		pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = 2;
		break;

	default: /*Invalid argument */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_AccessPolicy_Proc::Invalid argument (=%s)\n", arg));
		return FALSE;
	}

	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_AccessPolicy_Proc::(AccessPolicy=%ld)\n",
			 pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy));
	return TRUE;
}


/* Replaced by Set_ACLAddEntry_Proc() and Set_ACLClearAll_Proc() */

/*
    ==========================================================================
    Description:
	Add one entry or several entries(if allowed to)
		into Access control mac table list
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLAddEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	/*	RT_802_11_ACL			acl; */
	RT_802_11_ACL			*pacl = NULL;
	RTMP_STRING *this_char;
	RTMP_STRING *value;
	INT						i, j;
	BOOLEAN					isDuplicate = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num >= (MAX_NUM_OF_ACL_LIST - 1)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("The AccessControlList is full, and no more entry can join the list!\n"));
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pacl, sizeof(RT_802_11_ACL));

	if (pacl == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return FALSE;
	}

	NdisZeroMemory(pacl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(pacl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}

		if (strlen(this_char) != 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal MAC address length!\n"));
			continue;
		}

		for (i = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal MAC address format or octet!\n"));
				/* Do not use "continue" to replace "break" */
				break;
			}

			AtoH(value, &macAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
			continue;

		/* Check if this entry is duplicate. */
		isDuplicate = FALSE;

		for (j = 0; j < pacl->Num; j++) {
			if (memcmp(pacl->Entry[j].Addr, &macAddr, 6) == 0) {
				isDuplicate = TRUE;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("You have added an entry before :\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The duplicate entry is %02x:%02x:%02x:%02x:%02x:%02x\n",
						 macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
			}
		}

		if (!isDuplicate)
			NdisMoveMemory(pacl->Entry[pacl->Num++].Addr, &macAddr, MAC_ADDR_LEN);

		if (pacl->Num == MAX_NUM_OF_ACL_LIST) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("The AccessControlList is full, and no more entry can join the list!\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The last entry of ACL is %02x:%02x:%02x:%02x:%02x:%02x\n",
					 macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
			break;
		}
	}

	ASSERT(pacl->Num < MAX_NUM_OF_ACL_LIST);
	NdisZeroMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, pacl, sizeof(RT_802_11_ACL));
	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Set::%s(Policy=%ld, Entry#=%ld)\n",
			  __func__, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy,
			  pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num));
#ifdef DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("=============== Entry ===============\n"));

	for (i = 0; i < pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num; i++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Entry #%02d: ", i + 1));

		for (j = 0; j < MAC_ADDR_LEN; j++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%02X ", pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Entry[i].Addr[j]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
	}

#endif

	if (pacl != NULL)
		os_free_mem(pacl);

	return TRUE;
}


/*
    ==========================================================================
    Description:
	Delete one entry or several entries(if allowed to)
		from Access control mac table list
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLDelEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	UCHAR					nullAddr[MAC_ADDR_LEN];
	RT_802_11_ACL			acl;
	RTMP_STRING *this_char;
	RTMP_STRING *value;
	INT						i, j;
	BOOLEAN					isFound = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	NdisZeroMemory(&acl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&acl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	NdisZeroMemory(nullAddr, MAC_ADDR_LEN);

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}

		if (strlen(this_char) != 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal MAC address length!\n"));
			continue;
		}

		for (i = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal MAC address format or octet!\n"));
				/* Do not use "continue" to replace "break" */
				break;
			}

			AtoH(value, &macAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
			continue;

		/* Check if this entry existed. */
		isFound = FALSE;

		for (j = 0; j < acl.Num; j++) {
			if (memcmp(acl.Entry[j].Addr, &macAddr, MAC_ADDR_LEN) == 0) {
				isFound = TRUE;
				NdisZeroMemory(acl.Entry[j].Addr, MAC_ADDR_LEN);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("The entry %02x:%02x:%02x:%02x:%02x:%02x founded will be deleted!\n",
						  macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
			}
		}

		if (!isFound) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The entry %02x:%02x:%02x:%02x:%02x:%02x is not in the list!\n",
					 macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
		}
	}

	NdisZeroMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = acl.Policy;
	ASSERT(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num == 0);
	i = 0;

	for (j = 0; j < acl.Num; j++) {
		if (memcmp(acl.Entry[j].Addr, &nullAddr, MAC_ADDR_LEN) == 0)
			continue;
		else
			NdisMoveMemory(&(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Entry[i++]), acl.Entry[j].Addr, MAC_ADDR_LEN);
	}

	pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num = i;
	ASSERT(acl.Num >= pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num);
	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::%s(Policy=%ld, Entry#=%ld)\n",
			 __func__, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy,
			 pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num));
#ifdef DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=============== Entry ===============\n"));

	for (i = 0; i < pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num; i++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Entry #%02d: ", i + 1));

		for (j = 0; j < MAC_ADDR_LEN; j++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%02X ", pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Entry[i].Addr[j]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
	}

#endif
	return TRUE;
}


/* for ACL policy message */
#define ACL_POLICY_TYPE_NUM	3
char const *pACL_PolicyMessage[ACL_POLICY_TYPE_NUM] = {
	"the Access Control feature is disabled",						/* 0 : Disable */
	"only the following entries are allowed to join this BSS",			/* 1 : Allow */
	"all the following entries are rejected to join this BSS",			/* 2 : Reject */
};


/*
    ==========================================================================
    Description:
	Dump all the entries in the Access control
		mac table list of a specified BSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLShowAll_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	RT_802_11_ACL			acl;
	BOOLEAN					bDumpAll = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT						i, j;

	bDumpAll = os_str_tol(arg, 0, 10);

	if (bDumpAll == 1)
		bDumpAll = TRUE;
	else if (bDumpAll == 0) {
		bDumpAll = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Your input is 0!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The Access Control List will not be dumped!\n"));
		return TRUE;
	} else {
		return FALSE;  /* Invalid argument */
	}

	NdisZeroMemory(&acl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&acl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));

	/* Check if the list is already empty. */
	if (acl.Num == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The Access Control List is empty!\n"));
		return TRUE;
	}

	ASSERT(((bDumpAll == 1) && (acl.Num > 0)));
	/* Show the corresponding policy first. */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("=============== Access Control Policy ===============\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Policy is %ld : ", acl.Policy));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s\n", pACL_PolicyMessage[acl.Policy]));
	/* Dump the entry in the list one by one */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("===============  Access Control List  ===============\n"));

	for (i = 0; i < acl.Num; i++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Entry #%02d: ", i + 1));

		for (j = 0; j < MAC_ADDR_LEN; j++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%02X ", acl.Entry[i].Addr[j]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
	}

	return TRUE;
}


/*
    ==========================================================================
    Description:
	Clear all the entries in the Access control
		mac table list of a specified BSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLClearAll_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	/*	RT_802_11_ACL			acl; */
	RT_802_11_ACL			*pacl = NULL;
	BOOLEAN					bClearAll = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	bClearAll = os_str_tol(arg, 0, 10);

	if (bClearAll == 1)
		bClearAll = TRUE;
	else if (bClearAll == 0) {
		bClearAll = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Your input is 0!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The Access Control List will be kept unchanged!\n"));
		return TRUE;
	} else {
		return FALSE;  /* Invalid argument */
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pacl, sizeof(RT_802_11_ACL));

	if (pacl == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return FALSE;
	}

	NdisZeroMemory(pacl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(pacl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));

	/* Check if the list is already empty. */
	if (pacl->Num == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The Access Control List is empty!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("No need to clear the Access Control List!\n"));

		if (pacl != NULL)
			os_free_mem(pacl);

		return TRUE;
	}

	ASSERT(((bClearAll == 1) && (pacl->Num > 0)));

	/* Clear the entry in the list one by one */
	/* Keep the corresponding policy unchanged. */
	do {
		NdisZeroMemory(pacl->Entry[pacl->Num - 1].Addr, MAC_ADDR_LEN);
		pacl->Num -= 1;
	} while (pacl->Num > 0);

	ASSERT(pacl->Num == 0);
	NdisZeroMemory(&(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList), sizeof(RT_802_11_ACL));
	NdisMoveMemory(&(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList), pacl, sizeof(RT_802_11_ACL));
	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);

	if (pacl != NULL)
		os_free_mem(pacl);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::%s(Policy=%ld, Entry#=%ld)\n",
			 __func__, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy,
			 pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num));
	return TRUE;
}

#ifdef DBG
static void _rtmp_hexdump(int level, const char *title, const UINT8 *buf,
						  size_t len, int show)
{
	size_t i;

	if (level < DebugLevel)
		return;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s - hexdump(len=%lu):", title, (unsigned long) len));

	if (show) {
		for (i = 0; i < len; i++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 (" %02x", buf[i]));
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 (" [REMOVED]"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
}

void rtmp_hexdump(int level, const char *title, const UINT8 *buf, size_t len)
{
	_rtmp_hexdump(level, title, buf, len, 1);
}
#endif




/*
    ==========================================================================
    Description:
	Reset statistics counter

    Arguments:
	pAdapter            Pointer to our adapter
	arg

    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
	Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set site_survey
    ==========================================================================
*/

/*
    ==========================================================================
    Description:
	Issue a Auto-Channel Selection command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 set AutoChannelSel=1
			Ues the number of AP to choose
			2.) iwpriv ra0 set AutoChannelSel=2
			Ues the False CCA count to choose
			3.) iwpriv ra0 set AutoChannelSel=3
			Ues the channel busy count to choose
    ==========================================================================
*/
INT Set_AutoChannelSel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;

	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	Ssid.SsidLength = 0;

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		if (strlen(arg) > 0) {
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		} else { /*ANY ssid */
			Ssid.SsidLength = 0;
			memcpy(Ssid.Ssid, "", 0);
		}
	}

	if (strcmp(arg, "1") == 0)
		pAd->ApCfg.AutoChannelAlg = ChannelAlgApCnt;
	else if (strcmp(arg, "2") == 0)
		pAd->ApCfg.AutoChannelAlg = ChannelAlgCCA;
	else if (strcmp(arg, "3") == 0)
		pAd->ApCfg.AutoChannelAlg = ChannelAlgBusyTime;
	else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_AutoChannelSel_Proc Alg isn't defined\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_AutoChannelSel_Proc Alg=%d\n", pAd->ApCfg.AutoChannelAlg));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[42m%s: Alg = %d \x1b[m\n", __func__, pAd->ApCfg.AutoChannelAlg));

	if (pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR IfIdx;
		struct wifi_dev *pwdev = NULL;

		if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
			IfIdx = pObj->ioctl_if;
			pwdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
			AutoChSelScanStart(pAd, pwdev);
		}
	} else if (Ssid.SsidLength == 0)
		ApSiteSurvey(pAd, &Ssid, SCAN_PASSIVE, TRUE);
	else
		ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, TRUE);

	return TRUE;
}
INT Set_PartialScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	UINT8       bPartialScanning;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;
	bPartialScanning = os_str_tol(arg, 0, 10);

	if (bPartialScanning > 0) {
		if (((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) &&
			(ifIndex < HW_BEACON_MAX_NUM))
			pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

#ifdef APCLI_SUPPORT
		else if ((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
			pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;

#endif /* APCLI_SUPPORT */
	}

	pAd->ScanCtrl.PartialScan.bScanning = bPartialScanning ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): bScanning = %u\n", __func__, pAd->ScanCtrl.PartialScan.bScanning));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set a periodic check time for auto channel selection (unit: hour)
   Arguments:
       pAdapter                    Pointer to our adapter

    Return Value:
	TRUE if success, FALSE otherwise

    Note:
	Usage:
		iwpriv ra0 set ACSCheckTime=Hour

    ==========================================================================
*/
INT Set_AutoChannelSelCheckTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx, Hour;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IfIdx;
	struct wifi_dev *pwdev = NULL;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pwdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		BandIdx = HcGetBandByWdev(pwdev);
		Hour = simple_strtol(arg, 0, 10);
		pAd->ApCfg.ACSCheckTime[BandIdx] = Hour * 3600; /* Hour to second */
		pAd->ApCfg.ACSCheckCount[BandIdx] = 0; /* Reset counter */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\x1b[42m%s(): ACSCheckTime[%d]=%u seconds(%u hours)\x1b[m\n",
				  __func__, BandIdx, pAd->ApCfg.ACSCheckTime[BandIdx], Hour));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("\x1b[41m%s(): Not support current interface type = %u!!\x1b[m\n",
		__func__, pObj->ioctl_if_type));
	}

	return TRUE;
}
#endif /* AP_SCAN_SUPPORT */

static INT show_apcfg_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	struct apcfg_parameters apcfg_para_setting;
	LONG cfg_mode;
	UCHAR wmode;
	POS_COOKIE pObj = NULL;
	CHAR str[10] = "";

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("show ap cfg info:\n"));

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj == NULL) {
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("pObj is NULL\n"));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	if (wdev == NULL) {
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("wdev is NULL\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16s%-8s\n", " ", "WIFI_DRIVER", "PEAK_VALUE"));

	/*WirelessMode*/
	wmode = wdev->PhyMode;
	cfg_mode = wmode_2_cfgmode(wmode);
	if (WMODE_CAP_2G(wmode))
		apcfg_para_setting.cfg_mode[0] = cfg_mode;
	else if (WMODE_CAP_5G(wmode))
		apcfg_para_setting.cfg_mode[1] = cfg_mode;

	if (cfg_mode == 9)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16ld%ld\n", "WirelessMode", cfg_mode,
			apcfg_for_peak.cfg_mode[0]));
	else if (cfg_mode == 14)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16ld%ld\n", "WirelessMode", cfg_mode,
			apcfg_for_peak.cfg_mode[1]));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16ld%ld/%ld\n", "WirelessMode", cfg_mode,
			apcfg_for_peak.cfg_mode[0], apcfg_for_peak.cfg_mode[1]));

	apcfg_para_setting.tx_power_percentage = 0;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16lu%lu\n", "TxPower",
		apcfg_para_setting.tx_power_percentage,
		apcfg_for_peak.tx_power_percentage));

	/*TxPreamble*/
	apcfg_para_setting.tx_preamble = pAd->CommonCfg.TxPreamble;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16lu%lu\n", "TxPreamble",
		apcfg_para_setting.tx_preamble,
		apcfg_for_peak.tx_preamble));

	/*RTSThreshold*/
	apcfg_para_setting.conf_len_thld = wlan_config_get_rts_len_thld(wdev);
	apcfg_para_setting.oper_len_thld = wlan_operate_get_rts_len_thld(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "RTSThreshold(config)",
		apcfg_para_setting.conf_len_thld,
		apcfg_for_peak.conf_len_thld));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "RTSThreshold(operate)",
		apcfg_para_setting.oper_len_thld,
		apcfg_for_peak.oper_len_thld));

	/*FragThreshold*/
	apcfg_para_setting.conf_frag_thld = wlan_config_get_frag_thld(wdev);
	apcfg_para_setting.oper_frag_thld = wlan_operate_get_frag_thld(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "FragThreshold(config)",
		apcfg_para_setting.conf_frag_thld,
		apcfg_for_peak.conf_frag_thld));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "FragThreshold(operate)",
		apcfg_para_setting.oper_frag_thld,
		apcfg_for_peak.oper_frag_thld));

	/*TxBurst*/
	apcfg_para_setting.bEnableTxBurst = pAd->CommonCfg.bEnableTxBurst;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "TxBurst",
		apcfg_para_setting.bEnableTxBurst,
		apcfg_for_peak.bEnableTxBurst));

	/*ShortSlot*/
	apcfg_para_setting.bUseShortSlotTime = pAd->CommonCfg.bUseShortSlotTime;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "ShortSlot",
		apcfg_para_setting.bUseShortSlotTime,
		apcfg_for_peak.bUseShortSlotTime));

#ifdef DOT11_N_SUPPORT
	/*HT_BW*/
	apcfg_para_setting.conf_ht_bw = wlan_config_get_ht_bw(wdev);
	apcfg_para_setting.oper_ht_bw = wlan_operate_get_ht_bw(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_BW(config)",
		apcfg_para_setting.conf_ht_bw,
		apcfg_for_peak.conf_ht_bw));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_BW(operate)",
		apcfg_para_setting.oper_ht_bw,
		apcfg_for_peak.oper_ht_bw));

#ifdef DOT11N_DRAFT3
	/*HT_BSSCoexistence */
	apcfg_para_setting.bBssCoexEnable = pAd->CommonCfg.bBssCoexEnable;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_BSSCoexistence",
		apcfg_para_setting.bBssCoexEnable,
		apcfg_for_peak.bBssCoexEnable));
#endif

	/*HT_TxStream */
	apcfg_para_setting.ht_tx_streams = wlan_config_get_tx_stream(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_TxStream",
		apcfg_para_setting.ht_tx_streams,
		(pAd->CommonCfg.dbdc_mode ? (apcfg_for_peak.ht_tx_streams - 2) : apcfg_for_peak.ht_tx_streams)));

	/*HT_RxStream */
	apcfg_para_setting.ht_rx_streams = wlan_config_get_rx_stream(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_RxStream",
		apcfg_para_setting.ht_rx_streams,
		(pAd->CommonCfg.dbdc_mode ? (apcfg_for_peak.ht_rx_streams - 2) : apcfg_for_peak.ht_rx_streams)));

	/*HT_BADecline*/
	apcfg_para_setting.bBADecline = pAd->CommonCfg.bBADecline;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_BADecline",
		apcfg_para_setting.bBADecline,
		apcfg_for_peak.bBADecline));

	/*HT_AutoBA*/
	apcfg_para_setting.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_AutoBA",
		apcfg_para_setting.AutoBA,
		apcfg_for_peak.AutoBA));

	/*HT_AMSDU*/
	apcfg_para_setting.AmsduEnable = pAd->CommonCfg.BACapability.field.AmsduEnable;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_AMSDU",
		apcfg_para_setting.AmsduEnable,
		apcfg_for_peak.AmsduEnable));

	/*HT_BAWinSize*/
	apcfg_para_setting.RxBAWinLimit = pAd->CommonCfg.BACapability.field.RxBAWinLimit;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_BAWinSize",
		apcfg_para_setting.RxBAWinLimit,
		apcfg_for_peak.RxBAWinLimit));

	/*HT_GI*/
	apcfg_para_setting.ht_gi = pAd->CommonCfg.RegTransmitSetting.field.ShortGI;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_GI",
		apcfg_para_setting.ht_gi,
		apcfg_for_peak.ht_gi));

	/*HT_STBC*/
	apcfg_para_setting.ht_stbc = wlan_config_get_ht_stbc(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_STBC",
		apcfg_para_setting.ht_stbc,
		apcfg_for_peak.ht_stbc));

	/*HT_LDPC*/
	apcfg_para_setting.ht_ldpc = wlan_config_get_ht_ldpc(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_LDPC",
		apcfg_para_setting.ht_ldpc,
		apcfg_for_peak.ht_ldpc));

	/*HT_RDG*/
	apcfg_para_setting.bRdg = pAd->CommonCfg.bRdg;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_RDG",
		apcfg_para_setting.bRdg,
		apcfg_for_peak.bRdg));
#endif

	/*HT_DisallowTKIP*/
	apcfg_para_setting.HT_DisallowTKIP = pAd->CommonCfg.HT_DisallowTKIP;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "HT_DisallowTKIP",
		apcfg_para_setting.HT_DisallowTKIP,
		apcfg_for_peak.HT_DisallowTKIP));

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_5G(wmode)) {
		/*VHT_BW*/
		apcfg_para_setting.conf_vht_bw = wlan_config_get_vht_bw(wdev);
		apcfg_para_setting.oper_vht_bw = wlan_operate_get_vht_bw(wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "VHT_BW(config)",
			apcfg_para_setting.conf_vht_bw,
			apcfg_for_peak.conf_vht_bw));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "VHT_BW(operate)",
			apcfg_para_setting.oper_vht_bw,
			apcfg_for_peak.oper_vht_bw));

		/*VHT_SGI */
		apcfg_para_setting.vht_sgi = wlan_config_get_vht_sgi(wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "VHT_SGI",
			apcfg_para_setting.vht_sgi,
			apcfg_for_peak.vht_sgi));

		/*VHT_STBC*/
		apcfg_para_setting.vht_stbc = wlan_config_get_vht_stbc(wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "VHT_STBC",
			apcfg_para_setting.vht_stbc,
			apcfg_for_peak.vht_stbc));

		/*VHT_BW_SIGNAL*/
		apcfg_para_setting.vht_bw_signal = wlan_config_get_vht_bw_sig(wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "VHT_BW_SIGNAL",
			apcfg_para_setting.vht_bw_signal,
			apcfg_for_peak.vht_bw_signal));

		/*VHT_LDPC*/
		apcfg_para_setting.vht_ldpc = wlan_config_get_vht_ldpc(wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "VHT_LDPC",
			apcfg_para_setting.vht_ldpc,
			apcfg_for_peak.vht_ldpc));
	}

	if (WMODE_CAP_2G(wmode)) {
		/*G_BAND_256QAM*/
		apcfg_para_setting.g_band_256_qam = pAd->CommonCfg.g_band_256_qam;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "G_BAND_256QAM",
			apcfg_para_setting.g_band_256_qam,
			apcfg_for_peak.g_band_256_qam));
	}
#endif

	/*IEEE80211H*/
	apcfg_para_setting.bIEEE80211H = pAd->CommonCfg.bIEEE80211H;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "IEEE80211H",
		apcfg_para_setting.bIEEE80211H,
		apcfg_for_peak.bIEEE80211H));

#ifdef MT_DFS_SUPPORT
	/*DfsEnable*/
	if (WMODE_CAP_5G(wmode)) {
		apcfg_para_setting.bDfsEnable = pAd->CommonCfg.DfsParameter.bDfsEnable;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "DfsEnable",
			apcfg_para_setting.bDfsEnable,
			apcfg_for_peak.bDfsEnable));
	}
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	/*DfsZeroWait*/
	if (!(pAd->CommonCfg.dbdc_mode)) {
		apcfg_para_setting.DfsZeroWaitSupport = pAd->BgndScanCtrl.DfsZeroWaitSupport;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%-24s%-16d%d\n", "DfsZeroWait",
			apcfg_para_setting.DfsZeroWaitSupport,
			apcfg_for_peak.DfsZeroWaitSupport));
	}
#endif

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	/*ETxBfEnCond*/
	apcfg_para_setting.ETxBfEnCond = pAd->CommonCfg.ETxBfEnCond;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16lu%lu\n", "CommonCfg.ETxBfEnCond",
		apcfg_para_setting.ETxBfEnCond,
		apcfg_for_peak.ETxBfEnCond));

	/*ETxBfEnCond*/
	apcfg_para_setting.ETxBfEnCond = wlan_config_get_etxbf(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16lu%lu\n", "ETxBfEnCond",
		apcfg_para_setting.ETxBfEnCond,
		apcfg_for_peak.ETxBfEnCond));
#endif
#endif

	/*ITxBfEn*/
	apcfg_para_setting.ITxBfEn = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16d%d\n", "ITxBfEn",
		apcfg_para_setting.ITxBfEn,
		apcfg_for_peak.ITxBfEn));

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	/*MUTxRxEnable*/
	apcfg_para_setting.MUTxRxEnable = pAd->CommonCfg.MUTxRxEnable;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16lu%lu\n", "MUTxRxEnable",
		apcfg_para_setting.MUTxRxEnable,
		apcfg_for_peak.MUTxRxEnable));
#endif
#endif

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------------------\n"));

	/*external channel*/
	apcfg_para_setting.channel = wdev->channel;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16u\n", "current channel",
		apcfg_para_setting.channel));

	apcfg_para_setting.ext_channel = wlan_operate_get_ext_cha(wdev);

	if (apcfg_para_setting.ext_channel == EXTCHA_ABOVE)
		sprintf(str, "ABOVE");
	else if (apcfg_para_setting.ext_channel == EXTCHA_BELOW)
		sprintf(str, "BELOW");
	else
		sprintf(str, "NONE");

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%-24s%-16s\n", "extension channel", str));

	return TRUE;
}

INT Show_StaCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	ADD_HT_INFO_IE *addht;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\nHT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n\n%-19s%-4s%-12s%-12s%-12s%-12s%-12s%-12s\n",
			  "MAC", "AID", "TxPackets", "RxPackets", "TxBytes", "RxBytes", "TP(Tx)", "TP(Rx)"));

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst == SST_ASSOC)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%02X:%02X:%02X:%02X:%02X:%02X  ",
					  pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
					  pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-4d", (int)pEntry->Aid));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-12lu", (ULONG)pEntry->TxPackets.QuadPart));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-12lu", (ULONG)pEntry->RxPackets.QuadPart));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-12lu", (ULONG)pEntry->TxBytes));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-12lu", (ULONG)pEntry->RxBytes));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%lu %-12s", (pEntry->AvgTxBytes >> 17), "Mbps")); /* (n Bytes x 8) / (1024*1024) = (n >> 17) */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%lu %-12s", (pEntry->AvgRxBytes >> 17), "Mbps"));/* (n Bytes x 8) / (1024*1024) = (n >> 17) */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\n"));
		}
	}

	return TRUE;
}


INT Show_RAInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LowTrafficThrd: %d\n", pAd->CommonCfg.lowTrafficThrd));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpRule: %d\n", pAd->CommonCfg.TrainUpRule));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpRuleRSSI: %d\n", pAd->CommonCfg.TrainUpRuleRSSI));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpLowThrd: %d\n", pAd->CommonCfg.TrainUpLowThrd));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpHighThrd: %d\n", pAd->CommonCfg.TrainUpHighThrd));
#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("StreamMode: %d\n", pAd->CommonCfg.StreamMode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("StreamModeMCS: 0x%04x\n", pAd->CommonCfg.StreamModeMCS));
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ITxBfEn: %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ITxBfTimeout: %ld\n", pAd->CommonCfg.ITxBfTimeout));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETxBfTimeout: %ld\n", pAd->CommonCfg.ETxBfTimeout));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CommonCfg.ETxBfEnCond: %ld\n", pAd->CommonCfg.ETxBfEnCond));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETxBfNoncompress: %d\n", pAd->CommonCfg.ETxBfNoncompress));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETxBfIncapable: %d\n", pAd->CommonCfg.ETxBfIncapable));
#ifdef TXBF_DYNAMIC_DISABLE
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucAutoSoundingCtrl: %d\n",
			 pAd->CommonCfg.ucAutoSoundingCtrl));
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* TXBF_SUPPORT */
	return TRUE;
}

#ifdef TXBF_SUPPORT
INT Show_TxBfInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	HT_CAPABILITY_IE *ht_cap, HtCapabilityTmp;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	HT_BF_CAP *pTxBFCap;
	UCHAR ucBandIdx = 0, ucEBfCap;
#ifdef VHT_TXBF_SUPPORT
	VHT_CAP_INFO vht_cap;
#endif /* VHT_TXBF_SUPPORT */

	if (cap->FlgHwTxBfCap) {
		wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
		if (!wdev)
			return FALSE;

		if (wdev->pHObj) {
		    ucBandIdx = HcGetBandByWdev(wdev);
		}
		ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
		NdisMoveMemory(&HtCapabilityTmp, ht_cap, sizeof(HT_CAPABILITY_IE));
		if (HcIsBfCapSupport(wdev) == FALSE) {
			ucEBfCap = wlan_config_get_etxbf(wdev);
			wlan_config_set_etxbf(wdev, SUBF_OFF);
			mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
			wlan_config_set_etxbf(wdev, ucEBfCap);
		}
		pTxBFCap = &HtCapabilityTmp.TxBFCap;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band Index:%d\n", ucBandIdx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IsBfBand:%d\n", HcIsBfCapSupport(wdev)));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CommonCfg.ETxBfEnCond:%ld\n", pAd->CommonCfg.ETxBfEnCond));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETxBfEnCond:%d\n", wlan_config_get_etxbf(wdev)));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CommonCfg.ITxBfEn:%d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ITxBfEn:%d\n", wlan_config_get_itxbf(wdev)));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT TxBF Cap:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  TxBFRecCapable:%d\n", pTxBFCap->TxBFRecCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  RxSoundCapable:%d\n", pTxBFCap->RxSoundCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  TxSoundCapable:%d\n", pTxBFCap->TxSoundCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  RxNDPCapable:%d\n", pTxBFCap->RxNDPCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  TxNDPCapable:%d\n", pTxBFCap->TxNDPCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ImpTxBFCapable:%d\n", pTxBFCap->ImpTxBFCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Calibration:%d\n", pTxBFCap->Calibration));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ExpCSICapable:%d\n", pTxBFCap->ExpCSICapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ExpNoComSteerCapable:%d\n", pTxBFCap->ExpNoComSteerCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ExpComSteerCapable:%d\n", pTxBFCap->ExpComSteerCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ExpCSIFbk:%d\n", pTxBFCap->ExpCSIFbk));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ExpNoComBF:%d\n", pTxBFCap->ExpNoComBF));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ExpComBF:%d\n", pTxBFCap->ExpComBF));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MinGrouping:%d\n", pTxBFCap->MinGrouping));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  CSIBFAntSup:%d\n", pTxBFCap->CSIBFAntSup));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  NoComSteerBFAntSup:%d\n", pTxBFCap->NoComSteerBFAntSup));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ComSteerBFAntSup:%d\n", pTxBFCap->ComSteerBFAntSup));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  CSIRowBFSup:%d\n", pTxBFCap->CSIRowBFSup));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  ChanEstimation:%d\n", pTxBFCap->ChanEstimation));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  rsv:%d\n", pTxBFCap->rsv));

#ifdef VHT_TXBF_SUPPORT
		NdisCopyMemory(&vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));

		ucEBfCap = wlan_config_get_etxbf(wdev);
		if (HcIsBfCapSupport(wdev) == FALSE) {
			wlan_config_set_etxbf(wdev, SUBF_OFF);
		}
		mt_WrapSetVHTETxBFCap(pAd, wdev, &vht_cap);
		wlan_config_set_etxbf(wdev, ucEBfCap);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT TxBF Cap:\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  num_snd_dimension:%d\n", vht_cap.num_snd_dimension));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  bfee_sts_cap:%d\n", vht_cap.bfee_sts_cap));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  bfee_cap_su:%d\n", vht_cap.bfee_cap_su));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  bfer_cap_su:%d\n", vht_cap.bfer_cap_su));

#endif /* VHT_TXBF_SUPPORT */
	}
    return TRUE;
}
#endif /* TXBF_SUPPORT */

#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
INT Set_DiagOpt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG diagOpt;
	/*POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */
	diagOpt = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DiagOpt=%ld!\n", diagOpt));
	return TRUE;
}


INT Set_diag_cond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 cond;

	cond = os_str_tol(arg, 0, 10);
	pAd->DiagStruct.diag_cond = cond;
	return TRUE;
}


INT Show_Diag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RtmpDiagStruct *pDiag = NULL;
	UCHAR i, start, stop, que_idx;
	unsigned long irqFlags;

	os_alloc_mem(pAd, (UCHAR **)&pDiag, sizeof(RtmpDiagStruct));

	if (!pDiag) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():AllocMem failed!\n", __func__));
		return FALSE;
	}

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqFlags);
	NdisMoveMemory(pDiag, &pAd->DiagStruct, sizeof(RtmpDiagStruct));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqFlags);

	if (pDiag->inited == FALSE)
		goto done;

	start = pDiag->ArrayStartIdx;
	stop = pDiag->ArrayCurIdx;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Start=%d, stop=%d!\n\n", start, stop));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("    %-12s", "Time(Sec)"));

	for (i = 1; i < DIAGNOSE_TIME; i++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-7d", i));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    -------------------------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Tx Info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("    %-12s", "TxDataCnt\n"));

	for (que_idx = 0; que_idx < WMM_NUM_OF_AC; que_idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\tQueue[%d]:", que_idx));

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-7d", pDiag->diag_info[i].TxDataCnt[que_idx]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s", "TxFailCnt"));

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-7d", pDiag->diag_info[i].TxFailCnt));

#ifdef DBG_TX_AGG_CNT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s", "TxAggCnt"));

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-7d", pDiag->diag_info[i].TxAggCnt));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n"));
#endif /* DBG_TX_AGG_CNT */
#ifdef DBG_TXQ_DEPTH
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("DeQueue Info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s\n", "DeQueueFunc Called Distribution"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\t"));

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-8d", pDiag->diag_info[i].deq_called));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s\n", "DeQueueRound(Per-Call) Distribution"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\t"));

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-8d", pDiag->diag_info[i].deq_round));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s\n", "DeQueueCount(Per-Round) Distribution"));

	for (SwQNumLevel = 0; SwQNumLevel < 9; SwQNumLevel++) {
		if (SwQNumLevel == 8)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\t>%-5d",  SwQNumLevel));
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\t%-6d", SwQNumLevel));

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%-7d", pDiag->diag_info[i].deq_cnt[SwQNumLevel]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s%d", "Sw-Queued TxSwQCnt for WCID ", pDiag->wcid));

	for (que_idx = 0; que_idx < WMM_NUM_OF_AC; que_idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\n    %s[%d]\n", "Queue", que_idx));

		for (SwQNumLevel = 0; SwQNumLevel < 9; SwQNumLevel++) {
			if (SwQNumLevel == 8)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("\t>%-5d",  SwQNumLevel));
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("\t%-6d", SwQNumLevel));

			for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%-7d", pDiag->diag_info[i].TxSWQueCnt[que_idx][SwQNumLevel]));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\n"));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("	%-12s\n", "TxEnQFailCnt"));

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\t%-7d", pDiag->diag_info[i].enq_fall_cnt[que_idx]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n	  %s\n", "DeQueFailedCnt:Reason NotTxResource"));

	for (que_idx = 0; que_idx < WMM_NUM_OF_AC; que_idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\n    %s[%d]:", "Queue", que_idx));

		for (i = start; i != stop;	i = (i + 1) % DIAGNOSE_TIME)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\t%-7d", pDiag->diag_info[i].deq_fail_no_resource_cnt[que_idx]));
	}

#endif /* DBG_TXQ_DEPTH */
#ifdef DOT11_N_SUPPORT
#ifdef DBG_TX_AGG_CNT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s\n", "Tx-Agged AMPDUCnt"));

	for (McsIdx = 0; McsIdx < 16; McsIdx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t%-6d", (McsIdx + 1)));

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%d(%d%%)  ", pDiag->diag_info[i].TxAMPDUCnt[McsIdx],
					  pDiag->diag_info[i].TxAMPDUCnt[McsIdx] ? (pDiag->diag_info[i].TxAMPDUCnt[McsIdx] * 100 / pDiag->diag_info[i].TxAggCnt) :
					  0));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
	}

#endif /* DBG_TX_AGG_CNT */
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rx Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("    %-12s", "RxDataCnt"));

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-7d", pDiag->diag_info[i].RxDataCnt));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n    %-12s", "RxCrcErrCnt"));

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%-7d", pDiag->diag_info[i].RxCrcErrCnt));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\n-------------\n"));
done:
	os_free_mem(pDiag);
	return TRUE;
}
#endif /* DBG_DIAGNOSE */
#endif /* RTMP_MAC_PCI */


INT Show_Sat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 temperature = 0;

	RTMP_GET_TEMPERATURE(pAd, &temperature);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("CurrentTemperature              = %d\n", temperature));

	/* Sanity check for calculation of sucessful count */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TransmitCountFromOS = %d\n", pAd->WlanCounters[0].TransmitCountFrmOs.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TransmittedFragmentCount = %lld\n",
			  (INT64)pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart +
			  pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("MulticastTransmittedFrameCount = %d\n", pAd->WlanCounters[0].MulticastTransmittedFrameCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("FailedCount = %d\n", pAd->WlanCounters[0].FailedCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("RetryCount = %d\n", pAd->WlanCounters[0].RetryCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("MultipleRetryCount = %d\n", pAd->WlanCounters[0].MultipleRetryCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("RTSSuccessCount = %d\n", pAd->WlanCounters[0].RTSSuccessCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("RTSFailureCount = %d\n", pAd->WlanCounters[0].RTSFailureCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ACKFailureCount = %d\n", pAd->WlanCounters[0].ACKFailureCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("FrameDuplicateCount = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ReceivedFragmentCount = %d\n", pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("MulticastReceivedFrameCount = %d\n", pAd->WlanCounters[0].MulticastReceivedFrameCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer));
#ifdef DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("RealFcsErrCount = %d\n", pAd->RalinkCounters.RealFcsErrCount.u.LowPart));
#else
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("FCSErrorCount = %d\n", pAd->WlanCounters[0].FCSErrorCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("FrameDuplicateCount.LowPart = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart / 100));
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TransmittedFrameCount = %d\n", pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("WEPUndecryptableCount = %d\n", pAd->WlanCounters[0].WEPUndecryptableCount.u.LowPart));
#ifdef OUI_CHECK_SUPPORT
	{
		INT32 i = 0;

		for (i = 0; i < DBDC_BAND_NUM; i++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("band %d RxHWLookupWcidMismatchCount = %ld\n",
					 i, (ULONG)pAd->WlanCounters[i].RxHWLookupWcidErrCount.u.LowPart));
	}
#endif
#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("\n===Some 11n statistics variables:\n"));
	/* Some 11n statistics variables */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TxAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TxAMSDUCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("RxAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.RxAMSDUCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TransmittedAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TransmittedMPDUsInAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("TransmittedOctetsInAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("MPDUInReceivedAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.MPDUInReceivedAMPDUCount.u.LowPart));
#ifdef DOT11N_DRAFT3
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("fAnyStaFortyIntolerant=%d\n", pAd->MacTab.fAnyStaFortyIntolerant));
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	{
		int apidx;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("-- IF-ra%d --\n", apidx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Packets Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxCount));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Packets Sent = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].TxCount));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Bytes Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].ReceivedByteCount));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Byte Sent = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].TransmittedByteCount));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Error Packets Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxErrorCount));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Drop Received Packets = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxDropCount));
#ifdef WSC_INCLUDED

			if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfMode != WSC_DISABLE) {
				WSC_CTRL *pWscCtrl;

				pWscCtrl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("WscInfo:\n"
						  "\tWscConfMode=%d\n"
						  "\tWscMode=%s\n"
						  "\tWscConfStatus=%d\n"
						  "\tWscPinCode=%d\n"
						  "\tWscState=0x%x\n"
						  "\tWscStatus=0x%x\n",
						  pWscCtrl->WscConfMode,
						  ((pWscCtrl->WscMode == WSC_PIN_MODE) ? "PIN" : "PBC"),
						  pWscCtrl->WscConfStatus, pWscCtrl->WscEnrolleePinCode,
						  pWscCtrl->WscState, pWscCtrl->WscStatus));
			}

#endif /* WSC_INCLUDED */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("-- IF-ra%d end --\n", apidx));
		}
	}
	{
		int i, j, k, maxMcs = MAX_MCS_SET - 1;
		PMAC_TABLE_ENTRY pEntry;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("\n%02x:%02x:%02x:%02x:%02x:%02x - ", PRINT_MAC(pEntry->Addr)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%-4d\n", (int)pEntry->Aid));

				for (j = maxMcs; j >= 0; j--) {
					if ((pEntry->TXMCSExpected[j] != 0) || (pEntry->TXMCSFailed[j] != 0)) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("MCS[%02d]: Expected %u, Successful %u (%d%%), Failed %u\n",
						j, pEntry->TXMCSExpected[j], pEntry->TXMCSSuccessful[j],
						pEntry->TXMCSExpected[j] ? (100 * pEntry->TXMCSSuccessful[j]) / pEntry->TXMCSExpected[j] : 0,
						pEntry->TXMCSFailed[j]));

						for (k = maxMcs; k >= 0; k--) {
							if (pEntry->TXMCSAutoFallBack[j][k] != 0) {
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("\t\t\tAutoMCS[%02d]: %u (%d%%)\n", k, pEntry->TXMCSAutoFallBack[j][k],
								(100 * pEntry->TXMCSAutoFallBack[j][k]) / pEntry->TXMCSExpected[j]));
							}
						}
					}
				}
			}
		}
	}

#ifdef CONFIG_HOTSPOT
	{
		PHOTSPOT_CTRL pHSCtrl;
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;
		PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;
#ifdef CONFIG_DOT11V_WNM
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[apidx].WNMCtrl;
#endif
		pHSCtrl = &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("hotspot enable                    = %d\n", pHSCtrl->HotSpotEnable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("daemon ready                  = %d\n", pHSCtrl->HSDaemonReady));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("hotspot DGAFDisable               = %d\n", pHSCtrl->DGAFDisable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("hotspot L2Filter              = %d\n", pHSCtrl->L2Filter));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("hotspot ICMPv4Deny                = %d\n", pHSCtrl->ICMPv4Deny));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("hotspot QosMapEnable              = %d\n", pHSCtrl->QosMapEnable));
#ifdef CONFIG_DOT11V_WNM
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("proxy arp enable              = %d\n", pWNMCtrl->ProxyARPEnable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("WNMNotify enable              = %d\n", pWNMCtrl->WNMNotifyEnable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("hotspot OSEN enable                   = %d\n", pHSCtrl->bASANEnable));
#endif
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("GAS come back delay                       = %d\n", pGASCtrl->cb_delay));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
	}
#endif /* CONFIG_HOTSPOT */
#if defined(CONFIG_DOT11V_WNM) && !defined(CONFIG_HOTSPOT)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].WNMCtrl;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("WNM BSS Transition Management enable = %d\n", pWNMCtrl->WNMBTMEnable));
	}
#endif
#ifdef CONFIG_DOT11U_INTERWORKING
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;
		PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("GAS enable                       = %d\n", pGASCtrl->b11U_enable));
	}
#endif /* CONFIG_DOT11U_INTERWORKING */


#ifdef DOT11_N_SUPPORT
	/* Display Tx Aggregation statistics */
	DisplayTxAgg(pAd);
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}



INT Show_Sat_Reset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* Sanity check for calculation of sucessful count */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("TransmittedFragmentCount = %lld\n",
			  (INT64)pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart +
			  pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("MulticastTransmittedFrameCount = %d\n", pAd->WlanCounters[0].MulticastTransmittedFrameCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("FailedCount = %d\n", pAd->WlanCounters[0].FailedCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("RetryCount = %d\n", pAd->WlanCounters[0].RetryCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("MultipleRetryCount = %d\n", pAd->WlanCounters[0].MultipleRetryCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("RTSSuccessCount = %d\n", pAd->WlanCounters[0].RTSSuccessCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("RTSFailureCount = %d\n", pAd->WlanCounters[0].RTSFailureCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ACKFailureCount = %d\n", pAd->WlanCounters[0].ACKFailureCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("FrameDuplicateCount = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ReceivedFragmentCount = %d\n", pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("MulticastReceivedFrameCount = %d\n", pAd->WlanCounters[0].MulticastReceivedFrameCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer));
#ifdef DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("RealFcsErrCount = %d\n", pAd->RalinkCounters.RealFcsErrCount.u.LowPart));
#else
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("FCSErrorCount = %d\n", pAd->WlanCounters[0].FCSErrorCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("FrameDuplicateCount.LowPart = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart / 100));
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("TransmittedFrameCount = %d\n", pAd->WlanCounters[0].TransmittedFrameCount.u.LowPart));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("WEPUndecryptableCount = %d\n", pAd->WlanCounters[0].WEPUndecryptableCount.u.LowPart));
	pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart = 0;
	pAd->WlanCounters[0].MulticastTransmittedFrameCount.u.LowPart = 0;
	pAd->WlanCounters[0].FailedCount.u.LowPart = 0;
	pAd->WlanCounters[0].RetryCount.u.LowPart = 0;
	pAd->WlanCounters[0].MultipleRetryCount.u.LowPart = 0;
	pAd->WlanCounters[0].RTSSuccessCount.u.LowPart = 0;
	pAd->WlanCounters[0].RTSFailureCount.u.LowPart = 0;
	pAd->WlanCounters[0].ACKFailureCount.u.LowPart = 0;
	pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart = 0;
	pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart = 0;
	pAd->WlanCounters[0].MulticastReceivedFrameCount.u.LowPart = 0;
	pAd->Counters8023.RxNoBuffer = 0;
#ifdef DBG
	pAd->RalinkCounters.RealFcsErrCount.u.LowPart = 0;
#else
	pAd->WlanCounters[0].FCSErrorCount.u.LowPart = 0;
	pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart = 0;
#endif
	pAd->WlanCounters[0].TransmittedFrameCount.u.LowPart = 0;
	pAd->WlanCounters[0].WEPUndecryptableCount.u.LowPart = 0;
	{
		int i, j, k, maxMcs = 15;
		PMAC_TABLE_ENTRY pEntry;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("\n%02X:%02X:%02X:%02X:%02X:%02X - ",
						  pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
						  pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%-4d\n", (int)pEntry->Aid));

				for (j = maxMcs; j >= 0; j--) {
					if ((pEntry->TXMCSExpected[j] != 0) || (pEntry->TXMCSFailed[j] != 0)) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								 ("MCS[%02d]: Expected %u, Successful %u (%d%%), Failed %u\n",
								  j, pEntry->TXMCSExpected[j], pEntry->TXMCSSuccessful[j],
								  pEntry->TXMCSExpected[j] ? (100 * pEntry->TXMCSSuccessful[j]) / pEntry->TXMCSExpected[j] : 0,
								  pEntry->TXMCSFailed[j]
								 ));

						for (k = maxMcs; k >= 0; k--) {
							if (pEntry->TXMCSAutoFallBack[j][k] != 0) {
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
										 ("\t\t\tAutoMCS[%02d]: %u (%d%%)\n", k, pEntry->TXMCSAutoFallBack[j][k],
										  (100 * pEntry->TXMCSAutoFallBack[j][k]) / pEntry->TXMCSExpected[j]));
							}
						}
					}
				}
			}

			for (j = 0; j < (maxMcs + 1); j++) {
				pEntry->TXMCSExpected[j] = 0;
				pEntry->TXMCSSuccessful[j] = 0;
				pEntry->TXMCSFailed[j] = 0;

				for (k = maxMcs; k >= 0; k--)
					pEntry->TXMCSAutoFallBack[j][k] = 0;
			}
		}
	}
#ifdef DOT11_N_SUPPORT
	/* Display Tx Aggregation statistics */
	DisplayTxAgg(pAd);
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}


#ifdef MAT_SUPPORT
INT Show_MATTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	extern VOID dumpIPMacTb(MAT_STRUCT * pMatCfg, int index);
	extern NDIS_STATUS dumpSesMacTb(MAT_STRUCT * pMatCfg, int hashIdx);
	extern NDIS_STATUS dumpUidMacTb(MAT_STRUCT * pMatCfg, int hashIdx);
	extern NDIS_STATUS dumpIPv6MacTb(MAT_STRUCT * pMatCfg, int hashIdx);
	dumpIPMacTb(&pAd->MatCfg, -1);
	dumpSesMacTb(&pAd->MatCfg, -1);
	dumpUidMacTb(&pAd->MatCfg, -1);
	dumpIPv6MacTb(&pAd->MatCfg, -1);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Default BroadCast Address=%02x:%02x:%02x:%02x:%02x:%02x!\n", BROADCAST_ADDR[0], BROADCAST_ADDR[1],
			  BROADCAST_ADDR[2], BROADCAST_ADDR[3], BROADCAST_ADDR[4], BROADCAST_ADDR[5]));
	return TRUE;
}
#endif /* MAT_SUPPORT */


#ifdef DOT1X_SUPPORT
/*
    ==========================================================================
    Description:
	UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	UCHAR				apidx;
	NDIS_AP_802_11_KEY	 *pKey;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	apidx =	(UCHAR) pObj->ioctl_if;
	pKey = (PNDIS_AP_802_11_KEY) wrq->u.data.pointer;
	pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

	if (IS_AKM_WPA2(pSecConfig->AKMMap)
		|| IS_AKM_WPA3(pSecConfig->AKMMap)) {
		if (pKey->KeyLength == 32) {
			UCHAR	digest[80], PMK_key[20], macaddr[MAC_ADDR_LEN];

			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s: pKey->KeyLength = %d\n", __func__, pKey->KeyLength));
			/* Calculate PMKID */
			NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
			NdisMoveMemory(&PMK_key[8], pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
			NdisMoveMemory(&PMK_key[14], pKey->addr, MAC_ADDR_LEN);
			if (IS_AKM_SHA384(pSecConfig->AKMMap))
				RT_HMAC_SHA384(pKey->KeyMaterial, LEN_PMK_SHA384, PMK_key, 20, digest, LEN_PMKID);
			else
				RT_HMAC_SHA1(pKey->KeyMaterial, LEN_PMK, PMK_key, 20, digest, SHA1_DIGEST_SIZE);
			NdisMoveMemory(macaddr, pKey->addr, MAC_ADDR_LEN);
			RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
					  apidx,
					  macaddr,
					  digest,
					  pKey->KeyMaterial,
					  pKey->KeyLength);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("WPA2(pre-auth):(%02x:%02x:%02x:%02x:%02x:%02x)Calc PMKID=%02x:%02x:%02x:%02x:%02x:%02x\n",
					  pKey->addr[0], pKey->addr[1], pKey->addr[2], pKey->addr[3], pKey->addr[4], pKey->addr[5], digest[0], digest[1],
					  digest[2], digest[3], digest[4], digest[5]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PMK =%02x:%02x:%02x:%02x-%02x:%02x:%02x:%02x\n",
					 pKey->KeyMaterial[0], pKey->KeyMaterial[1],
					 pKey->KeyMaterial[2], pKey->KeyMaterial[3], pKey->KeyMaterial[4], pKey->KeyMaterial[5], pKey->KeyMaterial[6],
					 pKey->KeyMaterial[7]));
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set::RT_OID_802_11_WPA2_ADD_PMKID_CACHE ERROR or is wep key\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== RTMPIoctlAddPMKIDCache\n"));
}


/*
    ==========================================================================
    Description:
	UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlStaticWepCopy(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	MAC_TABLE_ENTRY *pEntry;
	UCHAR	MacAddr[MAC_ADDR_LEN];
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	apidx = (UCHAR) pObj->ioctl_if;
	ASIC_SEC_INFO Info = {0};
	UCHAR	KeyIdx;
	UINT32 len;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlStaticWepCopy-IF(ra%d)\n", apidx));

	if (wrq->u.data.length != sizeof(MacAddr)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPIoctlStaticWepCopy: the length isn't match (%d)\n",
				 wrq->u.data.length));
		return;
	}

	len = copy_from_user(&MacAddr, wrq->u.data.pointer, wrq->u.data.length);
	pEntry = MacTableLookup(pAd, MacAddr);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPIoctlStaticWepCopy: the mac address isn't match\n"));
		return;
	}

	KeyIdx = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId;

	/* need to copy the default shared-key to pairwise key table for this entry in 802.1x mode */
	if (pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.WepKey[KeyIdx].KeyLen == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: Can not get Default shared-key (index-%d)\n", KeyIdx));
		return;
	}

	pEntry->SecConfig.AKMMap = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.AKMMap;
	pEntry->SecConfig.PairwiseCipher = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher;
	pEntry->SecConfig.PairwiseKeyId = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId;
	NdisMoveMemory(pEntry->SecConfig.WepKey, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.WepKey,
				   sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pEntry->SecConfig.PairwiseCipher;
	Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
	HW_ADDREMOVE_KEYTABLE(pAd, &Info);
}

/*
    ==========================================================================
    Description:
	UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	MAC_TABLE_ENTRY			*pEntry;
	PDOT1X_IDLE_TIMEOUT		pIdleTime;

	if (wrq->u.data.length != sizeof(DOT1X_IDLE_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the length is mis-match\n", __func__));
		return;
	}

	pIdleTime = (PDOT1X_IDLE_TIMEOUT)wrq->u.data.pointer;
	pEntry = MacTableLookup(pAd, pIdleTime->StaAddr);

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the entry is empty\n", __func__));
		return;
	}

	pEntry->NoDataIdleCount = 0;
	/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	pEntry->StaIdleTimeout = pIdleTime->idle_timeout;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : Update Idle-Timeout(%d) from dot1x daemon\n",
			 __func__, pEntry->StaIdleTimeout));
}

#ifdef RADIUS_MAC_ACL_SUPPORT
PRT_802_11_RADIUS_ACL_ENTRY RadiusFindAclEntry(
	PLIST_HEADER		pCacheList,
	IN	PUCHAR		pMacAddr)
{
	PRT_802_11_RADIUS_ACL_ENTRY	pAclEntry = NULL;
	RT_LIST_ENTRY		        *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pAclEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;

	while (pAclEntry != NULL) {
		if (NdisEqualMemory(pAclEntry->Addr, pMacAddr, MAC_ADDR_LEN))
			return pAclEntry;

		pListEntry = pListEntry->pNext;
		pAclEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;
	}

	return NULL;
}

VOID RTMPIoctlAddRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RT_802_11_ACL_ENTRY newCache;
	UCHAR   apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PRT_802_11_RADIUS_ACL_ENTRY pAclEntry = NULL;

	apidx = (UCHAR) pObj->ioctl_if;

	if (pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy != RADIUS_MAC_AUTH_ENABLE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RADIUS_MAC_AUTH Function State in Disable.\n"));
		return;
	}

	/* From userSpace struct using RT_802_11_ACL_ENTRY */
	if (wrq->u.data.length != sizeof(RT_802_11_ACL_ENTRY)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : the length is mis-match\n", __func__));
		return;
	}

	copy_from_user(&newCache, wrq->u.data.pointer, wrq->u.data.length);
	pAclEntry = RadiusFindAclEntry(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList, newCache.Addr);

	if (pAclEntry) {
		/* Replace the Cache if exist */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[%d] Found %02x:%02x:%02x:%02x:%02x:%02x in Cache And Update Result to %d.\n",
				  apidx, PRINT_MAC(newCache.Addr), newCache.Rsv));
		pAclEntry->result = newCache.Rsv;
		return;
	}

	/* Add new Cache */
	os_alloc_mem(NULL, (UCHAR **)&pAclEntry, sizeof(RT_802_11_RADIUS_ACL_ENTRY));

	if (pAclEntry) {
		NdisZeroMemory(pAclEntry, sizeof(RT_802_11_RADIUS_ACL_ENTRY));
		pAclEntry->pNext = NULL;
		NdisMoveMemory(pAclEntry->Addr, newCache.Addr, MAC_ADDR_LEN);
		pAclEntry->result = newCache.Rsv;
		insertTailList(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList, (RT_LIST_ENTRY *)pAclEntry);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[%d] New %02x:%02x:%02x:%02x:%02x:%02x res(%d) in Cache(%d).\n",
				  apidx,
				  PRINT_MAC(pAclEntry->Addr), pAclEntry->result,
				  pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList.size));
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Error in alloc mem in New Radius ACL Function.\n"));
}

VOID RTMPIoctlDelRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	UCHAR   apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	RT_LIST_ENTRY     *pListEntry = NULL;
	UCHAR macBuf[MAC_ADDR_LEN];

	apidx = (UCHAR) pObj->ioctl_if;

	if (wrq->u.data.length != MAC_ADDR_LEN) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : the length is mis-match\n", __func__));
		return;
	}

	copy_from_user(&macBuf, wrq->u.data.pointer, wrq->u.data.length);
	pListEntry = (RT_LIST_ENTRY *)RadiusFindAclEntry(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList,
				 macBuf);

	if (pListEntry) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%d]Del %02x:%02x:%02x:%02x:%02x:%02x in Cache(%d).\n", apidx,
				 PRINT_MAC(macBuf), pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList.size));
		delEntryList(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList, pListEntry);
		os_free_mem(pListEntry);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%d]STA %02x:%02x:%02x:%02x:%02x:%02x not in Cache.\n",
				 apidx, PRINT_MAC(macBuf)));
	}
}

VOID RTMPIoctlClearRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR   apidx = (UCHAR) pObj->ioctl_if;
	RT_LIST_ENTRY     *pListEntry = NULL;
	PLIST_HEADER    pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

	if (pListHeader->size == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%d] Radius ACL Cache already in empty.\n", apidx));
		return;
	}

	pListEntry = pListHeader->pHead;

	while (pListEntry != NULL) {
		/*Remove ListEntry from Header*/
		removeHeadList(pListHeader);
		os_free_mem(pListEntry);
		pListEntry = pListHeader->pHead;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Clean [%d] Radius ACL Cache.\n", apidx));
}
#endif /* RADIUS_MAC_ACL_SUPPORT */

VOID RTMPIoctlQueryStaAid(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	DOT1X_QUERY_STA_AID macBuf;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (wrq->u.data.length != sizeof(DOT1X_QUERY_STA_AID)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the length is mis-match\n", __func__));
		return;
	}

	if (copy_from_user(&macBuf, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __func__));
		return;
	}

	pEntry = MacTableLookup(pAd, macBuf.StaAddr);

	if (pEntry != NULL) {
		wrq->u.data.length = sizeof(DOT1X_QUERY_STA_AID);
		macBuf.aid = pEntry->Aid;

		if (copy_to_user(wrq->u.data.pointer, &macBuf, wrq->u.data.length))
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __func__));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Query::OID_802_DOT1X_QUERY_STA_AID(%02x:%02x:%02x:%02x:%02x:%02x, AID=%d)\n",
				  PRINT_MAC(macBuf.StaAddr), macBuf.aid));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Query::OID_802_DOT1X_QUERY_STA_AID(%02x:%02x:%02x:%02x:%02x:%02x, Not Found)\n",
				  PRINT_MAC(macBuf.StaAddr)));
	}
}


VOID RTMPIoctlQueryStaRsn(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	struct DOT1X_QUERY_STA_RSN sta_rsn;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (wrq->u.data.length != sizeof(struct DOT1X_QUERY_STA_RSN)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_ERROR, ("%s : the length is mis-match\n", __func__));
		return;
	}

	if (copy_from_user(&sta_rsn, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __func__));
		return;
	}

	pEntry = MacTableLookup(pAd, sta_rsn.sta_addr);

	if (pEntry != NULL) {
		wrq->u.data.length = sizeof(struct DOT1X_QUERY_STA_RSN);
		sta_rsn.akm = pEntry->SecConfig.AKMMap;
		sta_rsn.pairwise_cipher = pEntry->SecConfig.PairwiseCipher;
		sta_rsn.group_cipher = pEntry->SecConfig.GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
		sta_rsn.group_mgmt_cipher = pEntry->SecConfig.PmfCfg.igtk_cipher;
#else
		sta_rsn.group_mgmt_cipher = 0;
#endif

		if (copy_to_user(wrq->u.data.pointer, &sta_rsn, wrq->u.data.length))
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __func__));

		MTWF_LOG(DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_TRACE,
				 ("Query::OID_802_DOT1X_QUERY_STA_RSN(%02x:%02x:%02x:%02x:%02x:%02x)\n",
				  PRINT_MAC(sta_rsn.sta_addr)));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_TRACE,
				("%s: AKM=%x, pairwise=%x, group_cipher=%x, group_mgmt_cipher=%x\n",
				__func__, sta_rsn.akm, sta_rsn.pairwise_cipher, sta_rsn.group_cipher, sta_rsn.group_mgmt_cipher));
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_TRACE,
				 ("Query::OID_802_DOT1X_QUERY_STA_RSN(%02x:%02x:%02x:%02x:%02x:%02x, Not Found)\n",
				  PRINT_MAC(sta_rsn.sta_addr)));
	}
}


#ifdef RADIUS_ACCOUNTING_SUPPORT
VOID RTMPIoctlQueryStaData(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	DOT1X_QUERY_STA_DATA macBuf;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT Status;

	if (wrq->u.data.length != sizeof(DOT1X_QUERY_STA_DATA)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the length is mis-match\n", __func__));
		return;
	}

	Status = copy_from_user(&macBuf, wrq->u.data.pointer, wrq->u.data.length);
	pEntry = MacTableLookup(pAd, macBuf.StaAddr);

	if (pEntry != NULL) {
		wrq->u.data.length = sizeof(DOT1X_QUERY_STA_DATA);
		macBuf.rx_bytes = pEntry->RxBytes;
		macBuf.tx_bytes = pEntry->TxBytes;
		macBuf.rx_packets = pEntry->RxPackets.u.LowPart;
		macBuf.tx_packets = pEntry->TxPackets.u.LowPart;

		if (copy_to_user(wrq->u.data.pointer, &macBuf, wrq->u.data.length))
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Query::OID_802_DOT1X_QUERY_STA_AID(%02x:%02x:%02x:%02x:%02x:%02x, Not Found)\n",
				  PRINT_MAC(macBuf.StaAddr)));
	}
}
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

#endif /* DOT1X_SUPPORT */


#if defined(DBG) || (defined(BB_SOC) && defined(CONFIG_ATE))

/*
    ==========================================================================
    Description:
	Read / Write BBP
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 bbp               ==> read all BBP
	       2.) iwpriv ra0 bbp 1             ==> read BBP where RegID=1
	       3.) iwpriv ra0 bbp 1=10		    ==> write BBP R1=0x10
    ==========================================================================
*/
VOID RTMPAPIoctlBBP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("IOCTL::(iwpriv) Command not Support!\n"));
}

/* CFG TODO: double define in ap_cfg / sta_cfg */
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
#endif

#ifdef RTMP_RF_RW_SUPPORT
/*
    ==========================================================================
    Description:
	Read / Write RF register
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 rf		==> read all RF registers
	       2.) iwpriv ra0 rf 1		==> read RF where RegID=1
	       3.) iwpriv ra0 rf 1=10	==> write RF R1=0x10
    ==========================================================================
*/
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
#endif
}
#endif /* RTMP_RF_RW_SUPPORT */
#endif /*#ifdef DBG */

/*
    ==========================================================================
    Description:
	Read / Write E2PROM
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 e2p 0	==> read E2PROM where Addr=0x0
	       2.) iwpriv ra0 e2p 0=1234    ==> write E2PROM where Addr=0x0, value=1234
    ==========================================================================
*/
VOID RTMPAPIoctlE2PROM(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING *this_char, *value;
	INT					j = 0, k = 0;
	RTMP_STRING *mpool, *msg;  /*msg[1024]; */
	RTMP_STRING *arg, *ptr;
	USHORT				eepAddr = 0;
	UCHAR				temp[16];
	RTMP_STRING temp2[16];
	USHORT				eepValue;
	BOOLEAN				bIsPrintAllE2PROM = FALSE;
	UINT_32 start = 0;
	UINT_32 end = 0;
	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (4096 + 256 + 12));

	if (mpool == NULL)
		return;

	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	arg = (RTMP_STRING *)((ULONG)(msg + 4096 + 3) & (ULONG)~0x03);
	memset(msg, 0x00, 4096);
	memset(arg, 0x00, 256);

	if (
#ifdef LINUX
		(wrq->u.data.length > 1) /* If no parameter, dump all e2p. */
#endif /* LINUX */
	) {
#ifdef LINUX
		if (copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length))
			goto done;
#else
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#endif
		ptr = arg;
		sprintf(msg, "\n");

		/*Parsing Read or Write */
		while ((this_char = strsep((char **)&ptr, ",")) != NULL) {

			if (!*this_char)
				continue;

			value = strchr(this_char, '=');

			if (value != NULL)
				*value++ = 0;

			if (!value || !*value) {
				/*Read */

				/* Sanity check */
				if (strstr(this_char, ":")) {
					UINT_32 e2p_end = EEPROM_SIZE;
					UINT_32 e2p_print_lmt = EEPROM_SIZE;

#if defined(RTMP_FLASH_SUPPORT)
					if (pAdapter->E2pAccessMode == E2P_FLASH_MODE)
						e2p_end = get_dev_eeprom_size(pAdapter);
#endif
					sscanf(arg, "%4x:%4x", &start, &end);

					if ((start + end) > 0 && end > start && end < e2p_end) {
						if (end - start > e2p_print_lmt) {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
										("Maximum display %d bytes, display 0x%x to 0x%x\n",
											e2p_print_lmt, start, start+e2p_print_lmt));
							end = start+e2p_print_lmt;
						}
						bIsPrintAllE2PROM = TRUE;
					}
					break;
				} else if (strlen(this_char) > 4) {
					break;
				}

				j = strlen(this_char);

				while (j-- > 0) {
					if (this_char[j] > 'f' || this_char[j] < '0')
						goto done; /*return; */
				}

				/* E2PROM addr */
				k = j = strlen(this_char);

				while (j-- > 0)
					this_char[4 - k + j] = this_char[j];

				while (k < 4)
					this_char[3 - k++] = '0';

				this_char[4] = '\0';

				if (strlen(this_char) == 4) {
					AtoH(this_char, temp, 2);
					eepAddr = *temp * 256 + temp[1];

					if (eepAddr < 0xFFFF) {
						RT28xx_EEPROM_READ16(pAdapter, eepAddr, eepValue);
						sprintf(msg + strlen(msg), "[0x%04X]:0x%04X  ", eepAddr, eepValue);
					} else {
						/*Invalid parametes, so default printk all bbp */
						break;
					}
				}
			} else {
				/*Write */
				NdisMoveMemory(&temp2, value, strlen(value));
				temp2[strlen(value)] = '\0';

				/* Sanity check */
				if ((strlen(this_char) > 4) || strlen(temp2) > 8)
					break;

				j = strlen(this_char);

				while (j-- > 0) {
					if (this_char[j] > 'f' || this_char[j] < '0')
						goto done; /* return; */
				}

				j = strlen(temp2) - 1;

				while (j >= 0) {
					if (temp2[j] > 'f' || temp2[j] < '0')
						goto done; /* return; */

					j--;
				}

				/* MAC Addr */
				k = j = strlen(this_char);

				while (j-- > 0)
					this_char[4 - k + j] = this_char[j];

				while (k < 4)
					this_char[3 - k++] = '0';

				this_char[4] = '\0';
				/* MAC value */
				k = strlen(temp2);
				j = strlen(temp2) - 1;

				while (j >= 0) {
					if ((4 - k + j) < 0)
						break;

					temp2[4 - k + j] = temp2[j];
					j--;
				}

				while (k < 4)
					temp2[3 - k++] = '0';

				temp2[4] = '\0';
				AtoH(this_char, temp, 2);
				eepAddr = *temp * 256 + temp[1];
				AtoH(temp2, temp, 2);
				eepValue = *temp * 256 + temp[1];
				RT28xx_EEPROM_WRITE16(pAdapter, eepAddr, eepValue);
				sprintf(msg + strlen(msg), "[0x%02X]:%02X  ", eepAddr, eepValue);
			}
		}
	} else
		bIsPrintAllE2PROM = TRUE;

	if (bIsPrintAllE2PROM) {
		sprintf(msg, "\n");

		if ((start + end) == 0) {
			start = 0;
			end = EEPROM_SIZE;
		}

		/* E2PROM Registers */
		for (eepAddr = start; eepAddr < end; eepAddr += 2) {
			RT28xx_EEPROM_READ16(pAdapter, eepAddr, eepValue);
			sprintf(msg + strlen(msg), "[0x%04X]:%04X  ", eepAddr, eepValue);

			if ((eepAddr & 0x6) == 0x6)
				sprintf(msg + strlen(msg), "\n");
		}

	}

	if (strlen(msg) == 1)
		sprintf(msg + strlen(msg), "===>Error command format!");

	/* Copy the information into the user buffer */
	AP_E2PROM_IOCTL_PostCtrl(wrq, msg);
done:
	os_free_mem(mpool);

	if (wrq->u.data.flags != RT_OID_802_11_HARDWARE_REGISTER)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==RTMPIoctlE2PROM\n"));
}


/* #define ENHANCED_STAT_DISPLAY	// Display PER and PLR statistics */


/*
    ==========================================================================
    Description:
	Read statistics counter
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 stat 0	==> Read statistics counter
    ==========================================================================
*/
static VOID ShowAmpduCounter(RTMP_ADAPTER *pAd, UCHAR BandIdx, RTMP_STRING *msg)
{
	COUNTER_802_11 *WlanCounter = &pAd->WlanCounters[BandIdx];
	ULONG per;

	sprintf(msg + strlen(msg), "BandIdx: %d\n", BandIdx);
	sprintf(msg + strlen(msg), "TX AGG Range 1 (1)              = %ld\n", (LONG)(WlanCounter->TxAggRange1Count.u.LowPart));
	sprintf(msg + strlen(msg), "TX AGG Range 2 (2~5)            = %ld\n", (LONG)(WlanCounter->TxAggRange2Count.u.LowPart));
	sprintf(msg + strlen(msg), "TX AGG Range 3 (6~15)           = %ld\n", (LONG)(WlanCounter->TxAggRange3Count.u.LowPart));
	sprintf(msg + strlen(msg), "TX AGG Range 4 (>15)            = %ld\n", (LONG)(WlanCounter->TxAggRange4Count.u.LowPart));
	{
		ULONG mpduTXCount;

		mpduTXCount = WlanCounter->AmpduSuccessCount.u.LowPart;
		sprintf(msg + strlen(msg), "AMPDU Tx success                = %ld\n", mpduTXCount);
		per = mpduTXCount == 0 ? 0 : 1000 * (WlanCounter->AmpduFailCount.u.LowPart) / (WlanCounter->AmpduFailCount.u.LowPart +
				mpduTXCount);
		sprintf(msg + strlen(msg), "AMPDU Tx fail count             = %ld, PER=%ld.%1ld%%\n",
				(ULONG)WlanCounter->AmpduFailCount.u.LowPart,
				per / 10, per % 10);
	}
}

VOID RTMPIoctlStatistics(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status;
	RTMP_STRING *msg;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	ULONG txCount = 0;
	UINT32 rxCount = 0;
#ifdef ENHANCED_STAT_DISPLAY
	ULONG per;
	INT i;
#endif
#ifdef RTMP_EFUSE_SUPPORT
	UINT efusefreenum = 0;
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef BB_SOC
	ULONG txPackets = 0, rxPackets = 0, txBytes = 0, rxBytes = 0;
	UCHAR index = 0;
#endif
	BOOLEAN isfound = FALSE;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR) * (2048));

	if (msg == NULL)
		return;

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);

	memset(msg, 0x00, 1600);
	sprintf(msg, "\n");

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("ra offload=%d\n", cap->fgRateAdaptFWOffload));
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		HTTRANSMIT_SETTING LastTxRate;

		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT | GET_TX_STAT_LAST_TX_RATE, ucBand, 0, &rTxStatResult);
		pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart += (rTxStatResult.u4TotalTxCount -
				rTxStatResult.u4TotalTxFailCount);
		pAd->WlanCounters[ucBand].FailedCount.u.LowPart += rTxStatResult.u4TotalTxFailCount;
		pAd->WlanCounters[ucBand].CurrentBwTxCount.u.LowPart += rTxStatResult.u4CurrBwTxCnt;
		pAd->WlanCounters[ucBand].OtherBwTxCount.u.LowPart += rTxStatResult.u4OtherBwTxCnt;

		LastTxRate.field.MODE = rTxStatResult.rLastTxRate.MODE;
		LastTxRate.field.BW = rTxStatResult.rLastTxRate.BW;
		LastTxRate.field.ldpc = rTxStatResult.rLastTxRate.ldpc ? 1 : 0;
		LastTxRate.field.ShortGI = rTxStatResult.rLastTxRate.ShortGI ? 1 : 0;
		LastTxRate.field.STBC = rTxStatResult.rLastTxRate.STBC;

		if (LastTxRate.field.MODE == MODE_VHT)
			LastTxRate.field.MCS = (((rTxStatResult.rLastTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rLastTxRate.MCS;
		else
			LastTxRate.field.MCS = rTxStatResult.rLastTxRate.MCS;

		pAd->LastTxRate = (USHORT)(LastTxRate.word);
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#ifdef CONFIG_ATE
	if (ATE_ON(pAd)) {
		txCount = pAd->ATECtrl.TxDoneCount;
		rxCount = pAd->ATECtrl.rx_stat.RxTotalCnt[0];
	} else
#endif /* CONFIG_ATE */
	{
		txCount = pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart;
		rxCount = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
	}


	sprintf(msg + strlen(msg), "Tx success                      = %ld\n", txCount);
#ifdef ENHANCED_STAT_DISPLAY
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		per = txCount == 0 ? 0 : 1000 * (pAd->WlanCounters[ucBand].FailedCount.u.LowPart) /
			  (pAd->WlanCounters[ucBand].FailedCount.u.LowPart + txCount);
		sprintf(msg + strlen(msg), "Tx fail count                   = %ld, PER=%ld.%1ld%%\n",
				(ULONG)pAd->WlanCounters[ucBand].FailedCount.u.LowPart,
				per / 10, per % 10);
		sprintf(msg + strlen(msg), "Current BW Tx count             = %ld\n", (ULONG)pAd->WlanCounters[ucBand].CurrentBwTxCount.u.LowPart);
		sprintf(msg + strlen(msg), "Other BW Tx count               = %ld\n", (ULONG)pAd->WlanCounters[ucBand].OtherBwTxCount.u.LowPart);
	}

	sprintf(msg + strlen(msg), "Rx success                      = %ld\n", (ULONG)rxCount);

#ifdef CONFIG_QA
	if (ATE_ON(pAd))
		per = rxCount == 0 ? 0 : 1000 * (pAd->WlanCounters[0].FCSErrorCount.u.LowPart) /
			  (pAd->WlanCounters[0].FCSErrorCount.u.LowPart + rxCount);
	else
#endif /* CONFIG_QA */
		per = pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart == 0 ? 0 : 1000 *
			  (pAd->WlanCounters[0].FCSErrorCount.u.LowPart) / (pAd->WlanCounters[0].FCSErrorCount.u.LowPart +
					  pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart);

	sprintf(msg + strlen(msg), "Rx with CRC                     = %ld, PER=%ld.%1ld%%\n",
			(ULONG)pAd->WlanCounters[0].FCSErrorCount.u.LowPart, per / 10, per % 10);
	sprintf(msg + strlen(msg), "Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer);
#endif /* ENHANCED_STAT_DISPLAY */

#ifdef CONFIG_QA
	if (ATE_ON(pAd)) {
		struct _ATE_RX_STATISTIC rx_stat = pAd->ATECtrl.rx_stat;

		if (pAd->ATECtrl.RxAntennaSel == 0) {
			sprintf(msg + strlen(msg), "RSSI-A                          = %ld\n",
					(LONG)(rx_stat.LastRssi[0] - pAd->BbpRssiToDbmDelta));
			sprintf(msg + strlen(msg), "RSSI-B (if available)           = %ld\n",
					(LONG)(rx_stat.LastRssi[1] - pAd->BbpRssiToDbmDelta));
			sprintf(msg + strlen(msg), "RSSI-C (if available)           = %ld\n\n",
					(LONG)(rx_stat.LastRssi[2] - pAd->BbpRssiToDbmDelta));
			sprintf(msg + strlen(msg), "RSSI-D (if available)           = %ld\n\n",
					(LONG)(rx_stat.LastRssi[3] - pAd->BbpRssiToDbmDelta));
		} else
			sprintf(msg + strlen(msg), "RSSI                            = %ld\n",
					(LONG)(rx_stat.LastRssi[0] - pAd->BbpRssiToDbmDelta));
	} else
#endif /* CONFIG_QA */
	{
#ifdef ENHANCED_STAT_DISPLAY
#ifdef CCK_LQ_SUPPORT
		if (IS_MT7615(pAd))
			sprintf(msg+strlen(msg), "LQ(CCK)                   = %d\n", pAd->Avg_LQ);
#endif
#ifdef LTF_SNR_SUPPORT
		if (IS_MT7615(pAd))
			sprintf(msg+strlen(msg), "LTF-SNR(OFDM)                   = %d\n", pAd->Avg_LTFSNR);
#endif
#ifdef NF_SUPPORT
		if (IS_MT7615(pAd)) {
			sprintf(msg+strlen(msg), "NF                              = %d\n", pAd->Avg_NF);
			sprintf(msg+strlen(msg), "SNR                             = %ld %ld %ld %ld\n",
					(LONG)(pAd->ApCfg.RssiSample.LastRssi[0] - pAd->Avg_NF),
					(LONG)(pAd->ApCfg.RssiSample.LastRssi[1] - pAd->Avg_NF),
					(LONG)(pAd->ApCfg.RssiSample.LastRssi[2] - pAd->Avg_NF),
					(LONG)(pAd->ApCfg.RssiSample.LastRssi[3] - pAd->Avg_NF));
		}
#endif
		/* Display Last Rx Rate and BF SNR of first Associated entry in MAC table */
		if (pAd->MacTab.Size > 0) {
			static char *phyMode[5] = {"CCK", "OFDM", "MM", "GF", "VHT"};

			for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);

				if ((pEntry->wdev == NULL) || HcGetBandByWdev(pEntry->wdev) != ucBand)
					continue;
				if ((IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) || IS_ENTRY_WDS(pEntry) || IS_ENTRY_APCLI(pEntry)) {
					/* sprintf(msg+strlen(msg), "sta mac: %02x:%02x:%02x:%02x:%02x:%02x\n", pEntry->wdev->if_addr[0], pEntry->wdev->if_addr[1],  pEntry->wdev->if_addr[2],  pEntry->wdev->if_addr[3],  pEntry->wdev->if_addr[4],  pEntry->wdev->if_addr[5]); */
					UINT32 lastRxRate = pEntry->LastRxRate;
					UINT32 lastTxRate = pEntry->LastTxRate;

					isfound = TRUE;
					sprintf(msg + strlen(msg), "RSSI                            = %ld %ld %ld %ld\n",
								(LONG)(pEntry->RssiSample.AvgRssi[0] - pAd->BbpRssiToDbmDelta),
								(LONG)(pEntry->RssiSample.AvgRssi[1] - pAd->BbpRssiToDbmDelta),
								(LONG)(pEntry->RssiSample.AvgRssi[2] - pAd->BbpRssiToDbmDelta),
								(LONG)(pEntry->RssiSample.AvgRssi[3] - pAd->BbpRssiToDbmDelta));

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					if (cap->fgRateAdaptFWOffload == TRUE) {
						if (pEntry->bAutoTxRateSwitch == TRUE) {
							EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
							HTTRANSMIT_SETTING LastTxRate;

							MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
							LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
							LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
							LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
							LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
							LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

							if (LastTxRate.field.MODE == MODE_VHT)
								LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
							else
								LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

							lastTxRate = (UINT32)(LastTxRate.word);
						}
					}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))  {
						StatRateToString(pAd, msg, 0, lastTxRate);
						StatRateToString(pAd, msg, 1, lastRxRate);
					} else
#endif /* MT_MAC */
					{
						sprintf(msg + strlen(msg), "Last TX Rate                    = MCS%d, %2dM, %cGI, %s%s\n",
								lastTxRate & 0x7F,  ((lastTxRate >> 7) & 0x1) ? 40 : 20,
								((lastTxRate >> 8) & 0x1) ? 'S' : 'L',
								phyMode[(lastTxRate >> 14) & 0x3],
								((lastTxRate >> 9) & 0x3) ? ", STBC" : " ");
						sprintf(msg + strlen(msg), "Last RX Rate                    = MCS%d, %2dM, %cGI, %s%s\n",
								lastRxRate & 0x7F,  ((lastRxRate >> 7) & 0x1) ? 40 : 20,
								((lastRxRate >> 8) & 0x1) ? 'S' : 'L',
								phyMode[(lastRxRate >> 14) & 0x3],
								((lastRxRate >> 9) & 0x3) ? ", STBC" : " ");
					}

					break;
				}
			}
		}

		if (isfound == FALSE) {
			sprintf(msg+strlen(msg), "RSSI                            = %ld %ld %ld %ld\n",
						(LONG)MINIMUM_POWER_VALUE,
						(LONG)MINIMUM_POWER_VALUE,
						(LONG)MINIMUM_POWER_VALUE,
						(LONG)MINIMUM_POWER_VALUE);
		}
#ifdef MT_MAC
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			ShowAmpduCounter(pAd, ucBand, msg);

			if (pAd->CommonCfg.bTXRX_RXV_ON) {
				sprintf(msg + strlen(msg), "/* Condition Number should enable mode4 of 0x6020_426c */\n");
				sprintf(msg + strlen(msg),
						"--10 packets Condition Number   = [%d|%d|%d|%d|%d|%d|%d|%d|%d|%d]\n",
						(UINT8)(pAd->rxv2_cyc3[0] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[1] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[2] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[3] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[4] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[5] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[6] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[7] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[8] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[9] & 0xff)
					   );
			}
		}

#endif /* MT_MAC */
#else
		sprintf(msg + strlen(msg), "RSSI-A                          = %ld\n",
				(LONG)(pAd->ApCfg.RssiSample.LastRssi[0] - pAd->BbpRssiToDbmDelta));
		sprintf(msg + strlen(msg), "RSSI-B (if available)           = %ld\n",
				(LONG)(pAd->ApCfg.RssiSample.LastRssi[1] - pAd->BbpRssiToDbmDelta));
		sprintf(msg + strlen(msg), "RSSI-C (if available)           = %ld\n\n",
				(LONG)(pAd->ApCfg.RssiSample.LastRssi[2] - pAd->BbpRssiToDbmDelta));
#endif /* ENHANCED_STAT_DISPLAY */
	}

#if /*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_SCHEDULE)

	if (pAd->Flags & fRTMP_ADAPTER_RADIO_OFF)
		sprintf(msg + strlen(msg), "Enable Wireless LAN		= %s\n", "0");
	else
		sprintf(msg + strlen(msg), "Enable Wireless LAN		= %s\n", "1");

	sprintf(msg + strlen(msg), "\n");
#endif /*TCSUPPORT_COMPILE*/
	/*
	 * Let "iwpriv ra0 stat" can print out Tx/Rx Packet and Byte count.
	 * Therefore, we can parse them out in cfg_manager. --Trey */
#ifdef BB_SOC

	for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
		rxPackets += (ULONG)pAd->ApCfg.MBSSID[index].RxCount;
		txPackets += (ULONG)pAd->ApCfg.MBSSID[index].TxCount;
		rxBytes += (ULONG)pAd->ApCfg.MBSSID[index].ReceivedByteCount;
		txBytes += (ULONG)pAd->ApCfg.MBSSID[index].TransmittedByteCount;
	}

	sprintf(msg + strlen(msg), "Packets Received       = %lu\n", rxPackets);
	sprintf(msg + strlen(msg), "Packets Sent           = %lu\n", txPackets);
	sprintf(msg + strlen(msg), "Bytes Received         = %lu\n", rxBytes);
	sprintf(msg + strlen(msg), "Bytes Sent             = %lu\n", txBytes);
	sprintf(msg + strlen(msg), "\n");
#endif
#ifdef RTMP_EFUSE_SUPPORT

	if (pAd->bUseEfuse == TRUE) {
		eFuseGetFreeBlockCount(pAd, &efusefreenum);
		sprintf(msg + strlen(msg), "efuseFreeNumber                 = %d\n", efusefreenum);
	}

#endif /* RTMP_EFUSE_SUPPORT */
		sprintf(msg+strlen(msg), "driver_version  %s\n", AP_DRIVER_VERSION);
	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
#if defined(TXBF_SUPPORT) && defined(ENHANCED_STAT_DISPLAY)
#endif /* defined(TXBF_SUPPORT) && defined(ENHANCED_STAT_DISPLAY) */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==RTMPIoctlStatistics\n"));
}

INT RTMPIoctlRXStatistics(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *this_char, *value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	os_alloc_mem(NULL, (UCHAR **)&this_char, wrq->u.data.length + 1);
	if (!this_char)
		return -ENOMEM;

	if (copy_from_user(this_char, wrq->u.data.pointer, wrq->u.data.length)) {
		os_free_mem(this_char);
		return -EFAULT;
	}
	this_char[wrq->u.data.length] = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Before check, this_char = %s\n"
			 , __func__, this_char));
	value = strchr(this_char, '=');

	if (value) {
		if (strlen(value) > 1) {
			*value = 0;
			value++;
		} else
			value = NULL;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): After check, this_char = %s, value = %s\n"
			 , __func__, this_char, (value == NULL ? "" : value)));

	for (PRTMP_PRIVATE_RX_PROC = RTMP_PRIVATE_RX_SUPPORT_PROC; PRTMP_PRIVATE_RX_PROC->name; PRTMP_PRIVATE_RX_PROC++) {
		if (!strcmp(this_char, PRTMP_PRIVATE_RX_PROC->name)) {
			if (!PRTMP_PRIVATE_RX_PROC->rx_proc(pAd, value, wrq)) {
				/*FALSE:Set private failed then return Invalid argument */
				Status = -EINVAL;
			}

			break;  /*Exit for loop. */
		}
	}

	if (PRTMP_PRIVATE_RX_PROC->name == NULL) {
		/*Not found argument */
		Status = -EINVAL;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IOCTL::(iwpriv) Command not Support [%s = %s]\n", this_char, value));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	os_free_mem(this_char);
	return Status;
}

#ifdef DOT11_N_SUPPORT
/*
    ==========================================================================
    Description:
	Get Block ACK Table
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 get_ba_table
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	/*char *msg; */
	UCHAR	TotalEntry, i, j, index;
	QUERYBA_TABLE		*BAT;

	BAT = vmalloc(sizeof(QUERYBA_TABLE));

	if (BAT == NULL)
		return;

	RTMPZeroMemory(BAT, sizeof(QUERYBA_TABLE));
	TotalEntry = pAd->MacTab.Size;
	index = 0;

	for (i = 0; ((VALID_UCAST_ENTRY_WCID(pAd, i)) && (TotalEntry > 0)); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->TXBAbitmap) && (index < 32)) {
			NdisMoveMemory(BAT->BAOriEntry[index].MACAddr, pEntry->Addr, 6);

			for (j = 0; j < 8; j++) {
				if (pEntry->BAOriWcidArray[j] != 0)
					BAT->BAOriEntry[index].BufSize[j] = pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]].BAWinSize;
				else
					BAT->BAOriEntry[index].BufSize[j] = 0;
			}

			TotalEntry--;
			index++;
			BAT->OriNum++;
		}
	}

	TotalEntry = pAd->MacTab.Size;
	index = 0;

	for (i = 0; ((VALID_UCAST_ENTRY_WCID(pAd, i)) && (TotalEntry > 0)); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->RXBAbitmap) && (index < 32)) {
			NdisMoveMemory(BAT->BARecEntry[index].MACAddr, pEntry->Addr, 6);
			BAT->BARecEntry[index].BaBitmap = (UCHAR)pEntry->RXBAbitmap;

			for (j = 0; j < 8; j++) {
				if (pEntry->BARecWcidArray[j] != 0)
					BAT->BARecEntry[index].BufSize[j] = pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]].BAWinSize;
				else
					BAT->BARecEntry[index].BufSize[j] = 0;
			}

			TotalEntry--;
			index++;
			BAT->RecNum++;
		}
	}

	wrq->u.data.length = sizeof(QUERYBA_TABLE);

	if (copy_to_user(wrq->u.data.pointer, BAT, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));

	vfree(BAT);
}
#endif /* DOT11_N_SUPPORT */


#ifdef APCLI_SUPPORT
INT Set_ApCli_Enable_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	struct wifi_dev *wdev = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	Enable = os_str_tol(arg, 0, 16);
	pAd->ApCfg.ApCliTab[ifIndex].Enable = (Enable > 0) ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("I/F(apcli%d) Set_ApCli_Enable_Proc::(enable = %d)\n",
			  ifIndex,
			  pAd->ApCfg.ApCliTab[ifIndex].Enable));
	wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
#ifdef APCLI_CONNECTION_TRIAL

	if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
		ApCliIfDown(pAd);

	/*Fix for TGac 5.2.57*/
	wlan_operate_set_prim_ch(wdev, wdev->channel);

	return TRUE;
}


INT Set_ApCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	INT success = FALSE;
	APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
		wdev = &apcli_entry->wdev;
		/* bring apcli interface down first */
		apcliEn = apcli_entry->Enable;
#ifdef APCLI_CONNECTION_TRIAL

		if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */

			if (apcliEn == TRUE) {
				apcli_entry->Enable = FALSE;
				ApCliIfDown(pAd);
			}

#ifdef APCLI_CONNECTION_TRIAL
		}

#endif /* APCLI_CONNECTION_TRIAL */
		apcli_entry->bPeerExist = FALSE;
		NdisZeroMemory(apcli_entry->CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(apcli_entry->CfgSsid, arg, strlen(arg));
		apcli_entry->CfgSsidLen = (UCHAR)strlen(arg);
		success = TRUE;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_Ssid_Proc::(Len=%d,Ssid=%s)\n",
				 ifIndex, apcli_entry->CfgSsidLen, apcli_entry->CfgSsid));
		apcli_entry->Enable = apcliEn;
	} else
		success = FALSE;

	return success;
}


INT Set_ApCli_Bssid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	INT i;
	RTMP_STRING *value;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

	/* bring apcli interface down first */
	if (apcliEn == TRUE) {
		pAd->ApCfg.ApCliTab[ifIndex].Enable = FALSE;
#ifdef APCLI_CONNECTION_TRIAL

		if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
			ApCliIfDown(pAd);
	}

	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);

	if (strlen(arg) == 17) { /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
				return FALSE;  /* Invalid */

			AtoH(value, &pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[i], 1);
		}

		if (i != 6)
			return FALSE;  /* Invalid */
	}

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ApCli_Bssid_Proc (%2X:%2X:%2X:%2X:%2X:%2X)\n",
			 pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[0],
			 pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[1],
			 pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[2],
			 pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[3],
			 pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[4],
			 pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[5]));
	pAd->ApCfg.ApCliTab[ifIndex].Enable = apcliEn;
	return TRUE;
}


INT Set_ApCli_TxMode_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	wdev = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev;
	wdev->DesiredTransmitSetting.field.FixedTxMode = RT_CfgSetFixedTxPhyMode(arg);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_TxMode_Proc = %d\n",
			 pObj->ioctl_if, wdev->DesiredTransmitSetting.field.FixedTxMode));
	return TRUE;
}

INT Set_ApCli_WirelessMode_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;
	UCHAR WirelessMode =  os_str_tol(arg, 0, 10);
	UCHAR PhyMode;
	APCLI_STRUCT *apcli_entry;
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	apcli_entry = &pAd->ApCfg.ApCliTab[pObj->ioctl_if];
	wdev = &apcli_entry->wdev;
	rate = &wdev->rate;
	PhyMode = cfgmode_2_wmode(WirelessMode);

	if (!APCLI_IF_UP_CHECK(pAd, pObj->ioctl_if))
		return FALSE;

	/* apcli always follow per band's channel */
	if (WMODE_CAP_5G(PhyMode))
		wdev->channel = HcGetChannelByRf(pAd, RFIC_5GHZ);
	else
		wdev->channel = HcGetChannelByRf(pAd, RFIC_24GHZ);

	if (wdev_do_linkdown(wdev) != TRUE)
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) linkdown fail!\n", __func__));

	os_msec_delay(100);

	if (wdev_do_close(wdev) != TRUE)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));

	/* fixed race condition between hw control task */
	os_msec_delay(100);
	wdev->PhyMode = PhyMode;
	HcAcquireRadioForWdev(pAd, wdev);
	/* Change channel state to NONE */
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
	BuildChannelList(pAd, wdev);
	RTMPSetPhyMode(pAd, wdev, PhyMode);
	/*update rate info for wdev*/
	RTMPUpdateRateInfo(wdev->PhyMode, rate);
	/* sync bw ext ch from mbss wdev */
	{
		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			int mbss_idx;

			for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum ; mbss_idx++) {
				if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode) {
					update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[mbss_idx].wdev);
					break;
				}
			}
		} else {
			/* align phy mode to BSS0 by default */
			wdev->PhyMode = pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode;
			update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[BSS0].wdev);
		}
	}
	os_msec_delay(100);

	/* Security initial  */
	if (wdev->SecConfig.AKMMap == 0x0)
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);

	if (wdev->SecConfig.PairwiseCipher == 0x0) {
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (wdev_do_open(wdev) != TRUE)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));

	os_msec_delay(100);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_WirelessMode_Proc = %d\n",
			 pObj->ioctl_if, PhyMode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("SupRate[0]=%x\n", rate->SupRate[0]));
	return TRUE;
}


INT Set_ApCli_TxMcs_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	wdev = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev;
	wdev->DesiredTransmitSetting.field.MCS =
		RT_CfgSetTxMCSProc(arg, &wdev->bAutoTxRateSwitch);

	if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_TxMcs_Proc = AUTO\n", pObj->ioctl_if));
	else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_TxMcs_Proc = %d\n",
				 pObj->ioctl_if, wdev->DesiredTransmitSetting.field.MCS));
	}

	return TRUE;
}

#ifdef APCLI_CONNECTION_TRIAL
INT Set_ApCli_Trial_Ch_Proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	CHAR *str;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (ifIndex != (pAd->ApCfg.ApCliNum - 1)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n\rI/F(apcli%d) can not run connection trial, use apcli%d\n",
				  ifIndex, (MAX_APCLI_NUM - 1)));
		return FALSE;
	}

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n\rI/F(apcli%d) can not run connection trial with DBDC mode\n",
				  ifIndex));
		return FALSE;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pApCliEntry->TrialCh = os_str_tol(arg, 0, 10);

	if (pApCliEntry->TrialCh)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) pApCliEntry->TrialCh = %d\n", ifIndex,
				 pApCliEntry->TrialCh));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) pApCliEntry->TrialCh = %d\n", ifIndex,
				 pApCliEntry->TrialCh));

#ifdef DBDC_MODE

	if ((WMODE_CAP_2G(pApCliEntry->wdev.PhyMode) && pApCliEntry->TrialCh > 14) ||
		(WMODE_CAP_5G(pApCliEntry->wdev.PhyMode) && pApCliEntry->TrialCh <= 14)) {
		pApCliEntry->TrialCh = 0;
		str = wmode_2_str(pApCliEntry->wdev.PhyMode);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("I/F(apcli%d) TrialCh = %d is not in this phy mode(%s)\n",
				  ifIndex, pApCliEntry->TrialCh, str));
		if (str)
			os_free_mem(str);
		return FALSE;
	}

#endif
	return TRUE;
}
#endif /* APCLI_CONNECTION_TRIAL */


#ifdef MAC_REPEATER_SUPPORT
INT Set_Cli_Link_Map_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE          pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR               ifIndex;
	UCHAR MbssIdx = 0;
	struct wifi_dev *cli_link_wdev = NULL;
	struct wifi_dev *mbss_link_wdev = NULL;

	if (pAd->ApCfg.bMACRepeaterEn != TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Rept has not been enabled yet.\n"));
		return FALSE;
	}

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MSTA))
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (ifIndex > MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("wrong cli link idx:%d to set link map.\n",
				  ifIndex));
		return FALSE;
	}

	cli_link_wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	MbssIdx = os_str_tol(arg, 0, 10);

	if (MbssIdx > HW_BEACON_MAX_NUM) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("wrong mbss idx:%d to set link map.\n",
				  MbssIdx));
		return FALSE;
	}

	mbss_link_wdev = &pAd->ApCfg.MBSSID[MbssIdx].wdev;
	UpdateMbssCliLinkMap(pAd, MbssIdx, cli_link_wdev, mbss_link_wdev);
	return TRUE;
}

INT Set_ReptMode_Enable_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UCHAR Enable = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MACRepeaterEn = %d, Repeater Mode (%s)\n",
			 pAd->ApCfg.bMACRepeaterEn, (Enable ? "ON" : "OFF")));
	AsicSetReptFuncEnable(pAd, Enable);
	return TRUE;
}



#endif /* MAC_REPEATER_SUPPORT */

#ifdef APCLI_AUTO_CONNECT_SUPPORT
/*
    ==========================================================================
    Description:
	Trigger Apcli Auto connect to find the missed AP.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

/*
0 : Disable apcli auto connect
1 : User Trigger Scan Mode
2 : Partial Scan Mode
3 : Driver Trigger Scan Mode
*/

INT Set_ApCli_AutoConnect_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR				ifIndex;
	AP_ADMIN_CONFIG *pApCfg;
	NDIS_802_11_SSID Ssid;
	long scan_mode = os_str_tol(arg, 0, 10);
	struct wifi_dev *wdev = NULL;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;


	pApCfg = &pAd->ApCfg;
	ifIndex = pObj->ioctl_if;
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	wdev = &pApCfg->ApCliTab[ifIndex].wdev;

	if (ApScanRunning(pAd, wdev) == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ApCli_AutoConnect_Proc() is still running\n"));
		return FALSE;
	}
	if (scan_mode == 0) {/* disable it */
		pAd->ScanCtrl.PartialScan.bScanning = FALSE;
		pApCfg->ApCliTab[ifIndex].AutoConnectFlag = FALSE;
		pApCfg->ApCliAutoConnectRunning[ifIndex] = FALSE;
		pApCfg->ApCliAutoBWAdjustCnt[ifIndex] = 0;
		pApCfg->ApCliAutoConnectType[ifIndex] = 0;
		pApCfg->bPartialScanEnable[ifIndex] = FALSE;
		pApCfg->bPartialScanning[ifIndex] = FALSE;
		return TRUE;
	}

	pApCfg->ApCliTab[ifIndex].AutoConnectFlag = TRUE;
	Set_ApCli_Enable_Proc(pAd, "0");
	pApCfg->ApCliAutoConnectRunning[ifIndex] = TRUE;
	pApCfg->ApCliAutoBWAdjustCnt[ifIndex] = 0;
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_AutoConnect_Proc::(Len=%d,Ssid=%s)\n",
			 ifIndex, pApCfg->ApCliTab[ifIndex].CfgSsidLen, pApCfg->ApCliTab[ifIndex].CfgSsid));

	/*
		use site survey function to trigger auto connecting (when pAd->ApCfg.ApAutoConnectRunning == TRUE)
	*/
	switch (scan_mode) {
	case 2:/* partial scan */
		pAd->ScanCtrl.PartialScan.pwdev = &pApCfg->ApCliTab[ifIndex].wdev;
		pAd->ScanCtrl.PartialScan.bScanning = TRUE;
			pApCfg->ApCliAutoConnectType[ifIndex] = TRIGGER_SCAN_BY_USER;
			pApCfg->bPartialScanEnable[ifIndex] = TRUE;
			pApCfg->bPartialScanning[ifIndex] = TRUE;
		break;
	case 3: /* full scan and autoconnect in ApCliCtrlJoinReqTimeoutAction, debug purpose */
			pAd->ScanCtrl.PartialScan.bScanning = FALSE;
			pApCfg->ApCliAutoConnectType[ifIndex] = TRIGGER_SCAN_BY_DRIVER;
			pApCfg->bPartialScanEnable[ifIndex] = FALSE;
			pApCfg->bPartialScanning[ifIndex] = FALSE;
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pApCfg->ApCliTab[ifIndex].wdev);
		break;
	case 1:/* full scan and autoconnect in RTMPRepeaterReconnectionCheck */
	default:
			pAd->ScanCtrl.PartialScan.bScanning = FALSE;
			pApCfg->ApCliAutoConnectType[ifIndex] = TRIGGER_SCAN_BY_USER;
			pApCfg->bPartialScanEnable[ifIndex] = FALSE;
			pApCfg->bPartialScanning[ifIndex] = FALSE;
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pApCfg->ApCliTab[ifIndex].wdev);
		break;
	}

	return TRUE;
}
#endif  /* APCLI_AUTO_CONNECT_SUPPORT */


#ifdef WSC_AP_SUPPORT
INT Set_AP_WscSsid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			ifIndex = pObj->ioctl_if;
	PWSC_CTRL	    pWscControl = &pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));

	if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pWscControl->WscSsid.Ssid, arg, strlen(arg));
		pWscControl->WscSsid.SsidLength = strlen(arg);
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, arg, strlen(arg));
		pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = (UCHAR)strlen(arg);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscSsid_Proc:: (Select SsidLen=%d,Ssid=%s)\n",
				 pWscControl->WscSsid.SsidLength, pWscControl->WscSsid.Ssid));
	} else
		return FALSE;	/*Invalid argument */

	return TRUE;
}

#ifdef APCLI_SUPPORT
INT Set_ApCli_WscScanMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR Mode = TRIGGER_FULL_SCAN;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	PWSC_CTRL pWscControl = &pAd->ApCfg.ApCliTab[ifIndex].WscControl;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	Mode = simple_strtol(arg, 0, 10);
	if (Mode != TRIGGER_PARTIAL_SCAN)
		Mode = TRIGGER_FULL_SCAN;

#ifdef AP_SCAN_SUPPORT
	if ((pWscControl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) &&
		(Mode != TRIGGER_PARTIAL_SCAN)) {
		pAd->ScanCtrl.PartialScan.pwdev = NULL;
		pAd->ScanCtrl.PartialScan.LastScanChannel = 0;
		pAd->ScanCtrl.PartialScan.bScanning = FALSE;
	}
#endif /* AP_SCAN_SUPPORT */
	pWscControl->WscApCliScanMode = Mode;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: (WscApCliScanMode=%d)\n", __func__, Mode));

	return TRUE;
}
#endif /* APCLI_SUPPORT */

#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_CERT_SUPPORT
INT Set_ApCli_Cert_Enable_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UCHAR wmode;
	struct wifi_dev *wdev = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	wmode = wdev->PhyMode;
	Enable = os_str_tol(arg, 0, 16);
	pAd->bApCliCertTest = (Enable > 0) ? TRUE : FALSE;

	if (pAd->bApCliCertTest == TRUE) {
		pAd->CommonCfg.bEnableTxBurst = FALSE;
		/* TGn case 5.2.47 & 5.2.50 */
		pAd->CommonCfg.bSeOff = 1;
		if (WMODE_CAP_2G(wmode))
			pAd->CommonCfg.g_band_256_qam = FALSE;
		/* TGn case 5.2.3 */
		HW_IO_WRITE32(pAd, RMAC_NAVOPT, 0);

#if defined(WHNAT_SUPPORT) && defined(PKT_BUDGET_CTRL_SUPPORT)
		/*
		 * Decrease budget control queue size if wifi nat is enabled
		 * Only 1024 hardware tokens are available to the host
		 * whereas budget control AC queue sizes are larger than it.
		 * These will cause packets in sw queue to be dropped
		 * due to non availability of tokens.
		 * WMM traffic differentiation test cases will fail since
		 * pkts will be dropped without priority check.
		 *
		 * for TGn case 5.2.27 & 5.2.30
		 */
		if ((pAd->CommonCfg.whnat_en) &&
		   (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))) {
			pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE / 2;
			pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK / 2;
			pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO / 2;
			pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI / 2;
		}
#endif
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Set_ApCli_Cert_Enable_Proc::(enable = %d)\n",
			 ifIndex, pAd->bApCliCertTest));
	return TRUE;
}

/* Add for APCLI PMF 5.3.3.3 option test item. (Only Tx De-auth Req. and make sure the pkt can be Encrypted) */
INT ApCliTxDeAuth(
	IN PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	USHORT Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s : ifIndex=%u\n", __func__, ifIndex));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
	}

#endif /* MAC_REPEATER_SUPPORT */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS)
		return FALSE;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s: DE-AUTH request (Reason=%d)...\n", __func__, Reason));
	ApCliMgtMacHeaderInit(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid,
						  pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, ifIndex);
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		COPY_MAC_ADDR(DeauthHdr.Addr2, pAd->ApCfg.pRepeaterCliPool[CliIdx].CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
	MakeOutgoingFrame(pOutBuffer,           &FrameLen,
					  sizeof(HEADER_802_11), &DeauthHdr,
					  2,                    &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}

#endif /* APCLI_CERT_SUPPORT */
#ifdef ROAMING_ENHANCE_SUPPORT
INT Set_RoamingEnhance_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	Enable = simple_strtol(arg, 0, 10);
	pAd->ApCfg.bRoamingEnhance = (Enable > 0) ? TRUE : FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) %s::(enable = %d)\n",
		ifIndex, __func__, pAd->ApCfg.bRoamingEnhance));
	return TRUE;
}
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */


#ifdef WSC_AP_SUPPORT
INT Set_Wps_Cert_Enable_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	Enable = os_str_tol(arg, 0, 16);
	pAd->bWpsCertTest = (Enable > 0) ? TRUE : FALSE;


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Wps_Cert_Enable_Proc::(enable = %d)\n",
			  pAd->bWpsCertTest));
	return TRUE;
}
#endif



#ifdef WSC_AP_SUPPORT
#ifdef CON_WPS
static  INT WscPushConcurrentPBCAction(
	IN      PRTMP_ADAPTER   pAd,
	IN      PWSC_CTRL   pWscControl,
	IN      BOOLEAN     bIsApCli)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	INT                 idx;

	if (bIsApCli)
		pWscControl->WscConfMode = WSC_ENROLLEE;
	else
		pWscControl->WscConfMode = WSC_REGISTRAR;

	WscInit(pAd, bIsApCli, apidx);
	pWscControl->WscMode = WSC_PBC_MODE;
	WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
	WscStop(pAd, bIsApCli, pWscControl);
	pWscControl->RegData.ReComputePke = 1;
	WscInitRegistrarPair(pAd, pWscControl, apidx);

	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

	pWscControl->bWscAutoTigeer = FALSE;

	if (bIsApCli) {
		pAd->ApCfg.ApCliTab[apidx].Enable = FALSE;
		ApCliIfDown(pAd);
		pWscControl->WscSsid.SsidLength = 0;
		NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
		pWscControl->WscPBCBssCount = 0;
		/* WPS - SW PBC */
		WscPushPBCAction(pAd, pWscControl);
	} else {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, TRUE, DEV_PASS_ID_PBC,
						 pWscControl->WscConfigMethods, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, TRUE, DEV_PASS_ID_PBC,
							pWscControl->WscConfigMethods, apidx, NULL, 0, AP_MODE);
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
		pWscControl->Wsc2MinsTimerRunning = TRUE;
		pWscControl->WscStatus = STATUS_WSC_LINK_UP;
		pWscControl->bWscTrigger = TRUE;
		RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
		WscClearPeerList(&pWscControl->WscPeerList);
		RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
	}

	return TRUE;
}

INT     Set_ConWpsApCliMode_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT Mode = CON_WPS_APCLI_BAND_AUTO;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	Mode = os_str_tol(arg, 0, 10);

	if (Mode >= CON_WPS_APCLI_BAND_MAX)
		return FALSE;

	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApCliMode = Mode;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApCliMode = Mode;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Oppos: %s, Mode = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, Mode));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Mode = %d\n",
				 __func__, pAd->net_dev->name, Mode));
	}

#else
	UINT Mode = CON_WPS_APCLI_BAND_AUTO;

	Mode = os_str_tol(arg, 0, 10);

	if (Mode >= CON_WPS_APCLI_BAND_MAX)
		return FALSE;

	pAd->ApCfg.ConWpsApCliMode = Mode;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Mode = %d\n",
			 __func__, pAd->net_dev->name, Mode));
#endif
	return TRUE;
}

INT     Set_ConWpsApcliAutoPreferIface_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT PreferIface = CON_WPS_APCLI_AUTO_PREFER_IFACE0;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	PreferIface = os_str_tol(arg, 0, 10);

	if (PreferIface >= CON_WPS_APCLI_AUTO_PREFER_IFACE_MAX)
		return FALSE;

	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApcliAutoPreferIface = PreferIface;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApcliAutoPreferIface = PreferIface;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Oppos: %s, PreferIface = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, PreferIface));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, PreferIface = %d\n",
				 __func__, pAd->net_dev->name, PreferIface));
	}

#else
	UINT PreferIface = CON_WPS_APCLI_AUTO_PREFER_IFACE0;

	PreferIface = os_str_tol(arg, 0, 10);
	pAd->ApCfg.ConWpsApcliAutoPreferIface = PreferIface;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, PreferIface = %d\n",
			 __func__, pAd->net_dev->name, PreferIface));
#endif
	return TRUE;
}

INT     Set_ConWpsApCliDisabled_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT Disabled = FALSE;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	Disabled = os_str_tol(arg, 0, 10);
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApCliDisableSetting = Disabled;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApCliDisableSetting = Disabled;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Oppos: %s, ApClient Disabled = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, Disabled));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, ApClient Disabled = %d\n",
				 __func__, pAd->net_dev->name, Disabled));
	}

#else
	UINT Disabled = FALSE;

	Disabled = os_str_tol(arg, 0, 10);
	pAd->ApCfg.ConWpsApCliDisableSetting = Disabled;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, ApClient Disabled = %d\n",
			 __func__, pAd->net_dev->name, Disabled));
#endif
	return TRUE;
}

INT	Set_ConWpsApDisabled_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT Disabled = FALSE;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	Disabled = simple_strtol(arg, 0, 10);
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApDisableSetting = Disabled;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApDisableSetting = Disabled;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Oppos: %s, Ap Disabled = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, Disabled));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Ap Disabled = %d\n",
				 __func__, pAd->net_dev->name, Disabled));
	}

#else
	UINT Disabled = FALSE;

	Disabled = simple_strtol(arg, 0, 10);
	pAd->ApCfg.ConWpsApDisableSetting = Disabled;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Now: %s, Ap Disabled = %d\n",
			 __func__, pAd->net_dev->name, Disabled));
#endif
	return TRUE;
}
#endif /* CON_WPS */

INT	 Set_AP_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT         ConfModeIdx;
	/*INT         IsAPConfigured; */
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	BOOLEAN     bFromApCli = FALSE;
	PWSC_CTRL   pWscControl;
#ifdef CON_WPS
	struct wifi_dev *pWdev = NULL;
#endif
	ConfModeIdx = os_str_tol(arg, 0, 10);
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef CON_WPS

	if (ConfModeIdx == WSC_ENROLLEE_REGISTRAR) {
		UINT ApClientWcid = 0;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WPS is using concurrent WPS now apidx=%d\n", apidx));

		if ((pAd->ApCfg.ConWpsApCliDisableSetting == TRUE) &&
			(apidx < pAd->ApCfg.ApCliNum) &&
			(pAd->ApCfg.ApCliTab[apidx].CtrlCurrState == APCLI_CTRL_CONNECTED)) {
			ApClientWcid = pAd->ApCfg.ApCliTab[apidx].MacTabWCID;

			if ((pAd->MacTab.Content[ApClientWcid].Sst == SST_ASSOC) &&
				(pAd->MacTab.tr_entry[ApClientWcid].PortSecured == WPA_802_1X_PORT_SECURED))
				pAd->ApCfg.ConWpsApCliDisabled = TRUE;
		}

		if (pAd->ApCfg.ConWpsApCliDisabled == TRUE)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Diabled the ApClient when using concurrent WPS now.\n"));
		else if ((apidx < pAd->ApCfg.ApCliNum) &&
				 (pAd->ApCfg.ApCliTab[apidx].Enable)) {
			pWdev =  &(pAd->ApCfg.ApCliTab[apidx].wdev);

			if (pWdev != NULL) {
				pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
				pWscControl->conWscStatus = CON_WPS_STATUS_APCLI_RUNNING;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("Set_AP_WscConfMode_Proc enter  WscPushConcurrentPBCAction(apcli%d) pWscControl=0x%p\n", apidx, pWscControl));
				RTMPZeroMemory(pAd->ApCfg.ApCliTab[apidx].wdev.WscControl.IfName, IFNAMSIZ);
				RTMPMoveMemory(pAd->ApCfg.ApCliTab[apidx].wdev.WscControl.IfName, pWdev->if_dev->name, IFNAMSIZ);
				WscPushConcurrentPBCAction(pAd, pWscControl, TRUE);
			}
		}

		if (pAd->ApCfg.ConWpsApDisableSetting == TRUE)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Disable the AP when using concurrent WPS now\n"));
		else {
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			pWscControl->conWscStatus = CON_WPS_STATUS_AP_RUNNING;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set_AP_WscConfMode_Proc enter  WscPushConcurrentPBCAction(ra%d)\n",
					 apidx));
			WscPushConcurrentPBCAction(pAd, pWscControl, FALSE);
		}

		return TRUE;
	}

#endif /* CON_WPS */
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscConfMode_Proc:: This command is from apcli interface now.\n", apidx));
	} else
#endif /* APCLI_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(ra%d) Set_WscConfMode_Proc:: This command is from ra interface now.\n", apidx));
		}

	pWscControl->bWscTrigger = FALSE;

	if ((ConfModeIdx & WSC_ENROLLEE_PROXY_REGISTRAR) == WSC_DISABLE) {
		pWscControl->WscConfMode = WSC_DISABLE;
		pWscControl->WscStatus = STATUS_WSC_NOTUSED;

		if (bFromApCli)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(apcli%d) Set_WscConfMode_Proc:: WPS is disabled.\n",
					 apidx));
		else {
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscConfMode_Proc:: WPS is disabled.\n", apidx));
			/* Clear WPS IE in Beacon and ProbeResp */
			wdev->WscIEBeacon.ValueLen = 0;
			wdev->WscIEProbeResp.ValueLen = 0;
			UpdateBeaconHandler(
				pAd,
				wdev,
				BCN_UPDATE_IE_CHG);
		}
	} else {
#ifdef APCLI_SUPPORT

		if (bFromApCli) {
			if (ConfModeIdx == WSC_ENROLLEE) {
				pWscControl->WscConfMode = WSC_ENROLLEE;
				WscInit(pAd, TRUE, apidx);
			} else {
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("IF(apcli%d) Set_WscConfMode_Proc:: Ap Client only supports Enrollee mode.(ConfModeIdx=%d)\n",
						  apidx, ConfModeIdx));
				return FALSE;
			}
		} else
#endif /* APCLI_SUPPORT */
		{
			pWscControl->WscConfMode = (ConfModeIdx & WSC_ENROLLEE_PROXY_REGISTRAR);
			WscInit(pAd, FALSE, apidx);
		}

		pWscControl->WscStatus = STATUS_WSC_IDLE;
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli)
		memcpy(mac_addr, &pAd->ApCfg.ApCliTab[apidx].wdev.if_addr[0], MAC_ADDR_LEN);
	else
#endif /* APCLI_SUPPORT */
	{
		memcpy(mac_addr, &pAd->ApCfg.MBSSID[apidx].wdev.bssid[0], MAC_ADDR_LEN);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscConfMode_Proc::(WscConfMode(0~7)=%d)\n",
			  PRINT_MAC(mac_addr), pWscControl->WscConfMode));
	return TRUE;
}

INT	Set_AP_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR       IsAPConfigured = 0;
	INT         IsSelectedRegistrar;
	USHORT      WscMode;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscConfStatus_Proc:: Ap Client doesn't need this command.\n", apidx));
		return FALSE;
	}

#endif /* APCLI_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bWpsEnable == FALSE) &&
		(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bEnableWpsV2)) {
		pAd->ApCfg.MBSSID[apidx].wdev.WscIEBeacon.ValueLen = 0;
		pAd->ApCfg.MBSSID[apidx].wdev.WscIEProbeResp.ValueLen = 0;
		return FALSE;
	}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	IsAPConfigured = (UCHAR)os_str_tol(arg, 0, 10);
	IsSelectedRegistrar = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscSelReg;

	if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

	if ((IsAPConfigured  > 0) && (IsAPConfigured  <= 2)) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus = IsAPConfigured;
		/* Change SC State of WPS IE in Beacon and ProbeResp */
		WscBuildBeaconIE(pAd, IsAPConfigured, IsSelectedRegistrar, WscMode, 0, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, IsSelectedRegistrar, WscMode, 0, apidx, NULL, 0,
							AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(ra%d) Set_WscConfStatus_Proc:: Set failed!!(WscConfStatus=%s), WscConfStatus is 1 or 2\n",
				  apidx, arg));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(ra%d) Set_WscConfStatus_Proc:: WscConfStatus is not changed (%d)\n",
				  apidx, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus));
		return FALSE;  /*Invalid argument */
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscConfStatus_Proc::(WscConfStatus=%d)\n",
			  PRINT_MAC(pAd->ApCfg.MBSSID[apidx].wdev.bssid),
			  pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus));
	return TRUE;
}

INT	Set_AP_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT         WscMode;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	PWSC_CTRL   pWscControl;
	BOOLEAN     bFromApCli = FALSE;
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscMode_Proc:: This command is from apcli interface now.\n",
				  apidx));
	} else
#endif /* APCLI_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(ra%d) Set_WscMode_Proc:: This command is from ra interface now.\n",
					  apidx));
		}

	WscMode = os_str_tol(arg, 0, 10);

	if ((WscMode  > 0) && (WscMode  <= 2)) {
		pWscControl->WscMode = WscMode;

		if (WscMode == WSC_PBC_MODE)
			WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Set_WscMode_Proc:: Set failed!!(Set_WscMode_Proc=%s), WscConfStatus is 1 or 2\n",
				  arg));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Set_WscMode_Proc:: WscMode is not changed (%d)\n",
				  pWscControl->WscMode));
		return FALSE;  /*Invalid argument */
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli)
		memcpy(mac_addr, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, MAC_ADDR_LEN);
	else
#endif /* APCLI_SUPPORT */
	{
		memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscMode_Proc::(WscMode=%d)\n",
			  PRINT_MAC(mac_addr), pWscControl->WscMode));
	return TRUE;
}

INT	Set_WscStatus_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscStatus_Proc::(WscStatus=%d)\n",
			 apidx, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus));
	return TRUE;
}

#define WSC_GET_CONF_MODE_EAP	1
#define WSC_GET_CONF_MODE_UPNP	2
INT	Set_AP_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT                 WscMode, wscGetConfMode = 0;
	INT                 IsAPConfigured;
	PWSC_CTRL           pWscControl;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
	INT	                idx;
	POS_COOKIE          pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	            apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	BOOLEAN             bFromApCli = FALSE;
#ifdef APCLI_SUPPORT
	BOOLEAN			apcliEn = pAd->ApCfg.ApCliTab[apidx].Enable;
#endif /* APCLI_SUPPORT */
#ifdef WSC_V2_SUPPORT
	PWSC_V2_INFO		pWscV2Info = NULL;
#endif /* WSC_V2_SUPPORT */
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		if (pAd->ApCfg.ApCliTab[apidx].ApCliInit == FALSE) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(apcli%d) Set_AP_WscGetConf_Proc:: ApCli is disabled.\n", apidx));
			return FALSE;
		}

		bFromApCli = TRUE;
		apidx &= (~MIN_NET_DEVICE_FOR_APCLI);
		pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_AP_WscGetConf_Proc:: This command is from apcli interface now.\n", apidx));
	} else
#endif /* APCLI_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(ra%d) Set_AP_WscGetConf_Proc:: This command is from ra interface now.\n",
					  apidx));
		}

	NdisZeroMemory(mac_addr, MAC_ADDR_LEN);
#ifdef WSC_V2_SUPPORT
	pWscV2Info = &pWscControl->WscV2Info;
#endif /* WSC_V2_SUPPORT */
	wscGetConfMode = os_str_tol(arg, 0, 10);
	IsAPConfigured = pWscControl->WscConfStatus;
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if ((pWscControl->WscConfMode == WSC_DISABLE)
#ifdef WSC_V2_SUPPORT
		|| ((pWscV2Info->bWpsEnable == FALSE) && (pWscV2Info->bEnableWpsV2))
#endif /* WSC_V2_SUPPORT */
	   ) {
		pWscControl->bWscTrigger = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Set_WscGetConf_Proc: WPS is disabled.\n"));
		return FALSE;
	}

	WscStop(pAd, bFromApCli, pWscControl);
	/* trigger wsc re-generate public key */
	pWscControl->RegData.ReComputePke = 1;

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

	WscInitRegistrarPair(pAd, pWscControl, apidx);

	/* Enrollee 192 random bytes for DH key generation */
	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

#ifdef APCLI_SUPPORT

	if (bFromApCli) {
		/* bring apcli interface down first */
		pAd->ApCfg.ApCliTab[apidx].Enable = FALSE;
		ApCliIfDown(pAd);

		if (WscMode == DEV_PASS_ID_PIN) {
			NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
						   pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, 6);
			pAd->ApCfg.ApCliTab[apidx].Enable = apcliEn;
		} else {
			pWscControl->WscSsid.SsidLength = 0;
			NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
			pWscControl->WscPBCBssCount = 0;
			/* WPS - SW PBC */
			WscPushPBCAction(pAd, pWscControl);
		}

		NdisMoveMemory(mac_addr, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, MAC_ADDR_LEN);
	} else
#endif /* APCLI_SUPPORT */
	{
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		WscBuildBeaconIE(pAd, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, apidx,
							NULL, 0, AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
		NdisMoveMemory(mac_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli && (WscMode == DEV_PASS_ID_PBC))
		;
	else
#endif /* APCLI_SUPPORT */
	{
		/* 2mins time-out timer */
		RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
		pWscControl->Wsc2MinsTimerRunning = TRUE;
		pWscControl->WscStatus = STATUS_WSC_LINK_UP;
		pWscControl->bWscTrigger = TRUE;
	}

	pWscControl->bWscAutoTigeer = FALSE;

	if (!bFromApCli) {
		if (WscMode == DEV_PASS_ID_PIN) {
			WscAssignEntryMAC(pAd, pWscControl);
			WscSendUPnPConfReqMsg(pAd, pWscControl->EntryIfIdx,
								  (PUCHAR)pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx].Ssid,
								  pAd->ApCfg.MBSSID[apidx].wdev.bssid, 3, 0, AP_MODE);
		} else {
			RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
			WscClearPeerList(&pWscControl->WscPeerList);
			RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
		}
	}

#ifdef WSC_LED_SUPPORT
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscGetConf_Proc trigger WSC state machine, wscGetConfMode=%d\n",
			  PRINT_MAC(mac_addr), wscGetConfMode));
	return TRUE;
}

INT	Set_AP_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT        PinCode = 0;
	BOOLEAN     validatePin, bFromApCli = FALSE;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	PWSC_CTRL   pWscControl;
#define IsZero(c) ('0' == (c) ? TRUE:FALSE)
	PinCode = os_str_tol(arg, 0, 10); /* When PinCode is 03571361, return value is 3571361. */
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscPinCode_Proc:: This command is from apcli interface now.\n", apidx));
	} else
#endif /* APCLI_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("IF(ra%d) Set_WscPinCode_Proc:: This command is from ra interface now.\n", apidx));
		}

	if (strlen(arg) == 4)
		validatePin = TRUE;
	else
		validatePin = ValidateChecksum(PinCode);

	if (validatePin) {
		if (pWscControl->WscRejectSamePinFromEnrollee &&
			(PinCode == pWscControl->WscLastPinFromEnrollee)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PIN authentication or communication error occurs!!\n"
					 "Registrar does NOT accept the same PIN again!(PIN:%s)\n", arg));
			return FALSE;
		}

		pWscControl->WscPinCode = PinCode;
		pWscControl->WscLastPinFromEnrollee = pWscControl->WscPinCode;
		pWscControl->WscRejectSamePinFromEnrollee = FALSE;

		/* PIN Code */
		if (strlen(arg) == 4) {
			pWscControl->WscPinCodeLen = 4;
			pWscControl->RegData.PinCodeLen = 4;
			NdisMoveMemory(pWscControl->RegData.PIN, arg, 4);
		} else {
			pWscControl->WscPinCodeLen = 8;

			if (IsZero(*arg)) {
				pWscControl->RegData.PinCodeLen = 8;
				NdisMoveMemory(pWscControl->RegData.PIN, arg, 8);
			} else
				WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
		}
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Set failed!!(Set_WscPinCode_Proc=%s), PinCode Checksum invalid\n", arg));
		return FALSE;  /*Invalid argument */
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli)
		memcpy(mac_addr, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, MAC_ADDR_LEN);
	else
#endif /* APCLI_SUPPORT */
	{
		memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscPinCode_Proc::(PinCode=%d)\n",
			  PRINT_MAC(mac_addr), pWscControl->WscPinCode));
	return TRUE;
}

INT	Set_WscOOB_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	char        *pTempSsid = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscPinCode_Proc:: Ap Client doesn't need this command.\n", apidx));
		return FALSE;
	}

#endif /* APCLI_SUPPORT */
	Set_WscSetupLock_Proc(pAd, "0");
	Set_AP_WscConfStatus_Proc(pAd, "1");
	Set_SecAuthMode_Proc(pAd, "WPA2PSK");
	Set_SecEncrypType_Proc(pAd, "AES");
	pTempSsid = vmalloc(33);

	if (pTempSsid) {
		memset(pTempSsid, 0, 33);
		snprintf(pTempSsid, 33, "RalinkInitialAP%02X%02X%02X",
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[3],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[4],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[5]);
		Set_AP_SSID_Proc(pAd, pTempSsid);
		vfree(pTempSsid);
	}

	Set_SecWPAPSK_Proc(pAd, "RalinkInitialAPxx1234");
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscOOB_Proc\n", apidx));
	return TRUE;
}

INT	Set_WscStop_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	PWSC_CTRL   pWscControl;
	BOOLEAN     bFromApCli = FALSE;
#ifdef HOSTAPD_SUPPORT

	if (pAd->ApCfg.MBSSID[apidx].Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}

#endif /*HOSTAPD_SUPPORT*/
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscStop_Proc:: This command is from apcli interface now.\n", apidx));
	} else
#endif /* APCLI_SUPPORT */
	{
		bFromApCli = FALSE;
		pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(ra%d) Set_WscStop_Proc:: This command is from ra interface now.\n", apidx));
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli) {
		WscStop(pAd, TRUE, pWscControl);
		pWscControl->WscConfMode = WSC_DISABLE;
	} else
#endif /* APCLI_SUPPORT */
	{
		INT	 IsAPConfigured = pWscControl->WscConfStatus;
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
		WscStop(pAd, FALSE, pWscControl);
	}

	pWscControl->bWscTrigger = FALSE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<===== Set_WscStop_Proc"));
	return TRUE;
}

#ifdef VENDOR_FEATURE6_SUPPORT
/* copy from RTMPIoctlWscProfile() but the strue is use WSC_CONFIGURED_VALUE_2 */
VOID RTMPGetCurrentCred(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	* wrq)
{
	WSC_CONFIGURED_VALUE_2 Profile;
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
#ifdef HOSTAPD_SUPPORT

	if (pMbss->Hostapd == TRUE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return;
	}

#endif /*HOSTAPD_SUPPORT*/
	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE_2));
	Profile.WscConfigured = pMbss->WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32);
	NdisMoveMemory(Profile.WscSsid, pMbss->Ssid, pMbss->SsidLen);
	Profile.WscSsidLen = pMbss->SsidLen;
	Profile.WscAuthMode = WscGetAuthType(wdev->SecConfig.AKMMap);
	Profile.WscEncrypType = WscGetEncryType(wdev->SecConfig.PairwiseCipher);
	NdisZeroMemory(Profile.WscWPAKey, 64);

	if (Profile.WscEncrypType == 2) {
		Profile.DefaultKeyIdx = wdev->SecConfig.PairwiseKeyId + 1;
		{
			int i;

			for (i = 0; i < wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen; i++) {
				snprintf((RTMP_STRING *) Profile.WscWPAKey, sizeof(Profile.WscWPAKey),
						 "%s%02x", Profile.WscWPAKey,
						 wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].Key[i]);
			}

			Profile.WscWPAKeyLen = wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen;
		}
	} else if (Profile.WscEncrypType >= 4) {
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pMbss->WscControl.WpaPsk,
					   pMbss->WscControl.WpaPskLen);
		Profile.WscWPAKeyLen = pMbss->WscControl.WpaPskLen;
	} else
		Profile.DefaultKeyIdx = 1;

	wrq->u.data.length = sizeof(Profile);

	if (copy_to_user(wrq->u.data.pointer, &Profile, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));

	/*	msg = (RTMP_STRING *)kmalloc(sizeof(CHAR)*(2048), MEM_ALLOC_FLAG); */
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR) * (2048));

	if (msg == NULL)
		return;

	memset(msg, 0x00, 2048);
	sprintf(msg, "%s", "\n");

	if (Profile.WscEncrypType == 1)
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	else if (Profile.WscEncrypType == 2)
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType", "DefaultKeyID", "Key");
	else
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");

	if (Profile.WscConfigured == 1)
		sprintf(msg + strlen(msg), "%-12s", "No");
	else
		sprintf(msg + strlen(msg), "%-12s", "Yes");

	sprintf(msg + strlen(msg), "%-33s", Profile.WscSsid);

	if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
		sprintf(msg + strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		sprintf(msg + strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));

	if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) || IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher))
		sprintf(msg + strlen(msg), "%-12s", "TKIPAES");
	else
		sprintf(msg + strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
		sprintf(msg + strlen(msg), "%s\n", "");
	else if (Profile.WscEncrypType == 2) {
		sprintf(msg + strlen(msg), "%-13d", Profile.DefaultKeyIdx);
		sprintf(msg + strlen(msg), "%-26s\n", Profile.WscWPAKey);
	} else if (Profile.WscEncrypType >= 4)
		sprintf(msg + strlen(msg), "%-64s\n", Profile.WscWPAKey);

#ifdef INF_AR9
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
#endif/* INF_AR9 */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	os_free_mem(msg);
}
#endif /* VENDOR_FEATURE6_SUPPORT */

/*
    ==========================================================================
    Description:
	Get WSC Profile
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 get_wsc_profile
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPIoctlWscProfile(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	WSC_CONFIGURED_VALUE Profile;
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
#ifdef HOSTAPD_SUPPORT

	if (pMbss->Hostapd == Hostapd_EXT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS is control by hostapd now.\n"));
		return;
	}

#endif /*HOSTAPD_SUPPORT*/
	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
	Profile.WscConfigured = pMbss->wdev.WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32 + 1);
	NdisMoveMemory(Profile.WscSsid, pMbss->Ssid, pMbss->SsidLen);
	Profile.WscSsid[pMbss->SsidLen] = '\0';
	Profile.WscAuthMode = WscGetAuthType(wdev->SecConfig.AKMMap);
	Profile.WscEncrypType = WscGetEncryType(wdev->SecConfig.PairwiseCipher);
	NdisZeroMemory(Profile.WscWPAKey, 64 + 1);

	if (Profile.WscEncrypType == 2) {
		Profile.DefaultKeyIdx = wdev->SecConfig.PairwiseKeyId + 1;
		{
			int i;

			for (i = 0; i < wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen; i++) {
				snprintf((RTMP_STRING *) Profile.WscWPAKey, sizeof(Profile.WscWPAKey),
						 "%s%02x", Profile.WscWPAKey,
						 wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].Key[i]);
			}

			Profile.WscWPAKey[(wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen) * 2] = '\0';
		}
	} else if (Profile.WscEncrypType >= 4) {
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pMbss->wdev.WscControl.WpaPsk,
					   pMbss->wdev.WscControl.WpaPskLen);
		Profile.WscWPAKey[pMbss->wdev.WscControl.WpaPskLen] = '\0';
	} else
		Profile.DefaultKeyIdx = 1;

	wrq->u.data.length = sizeof(Profile);

	if (copy_to_user(wrq->u.data.pointer, &Profile, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR) * (2048));

	if (msg == NULL)
		return;

	memset(msg, 0x00, 2048);
	sprintf(msg, "%s", "\n");

	if (Profile.WscEncrypType == 1)
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	else if (Profile.WscEncrypType == 2)
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType",
				"DefaultKeyID", "Key");
	else
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");

	if (Profile.WscConfigured == 1)
		sprintf(msg + strlen(msg), "%-12s", "No");
	else
		sprintf(msg + strlen(msg), "%-12s", "Yes");

	sprintf(msg + strlen(msg), "%-33s", Profile.WscSsid);

	if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
		sprintf(msg + strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		sprintf(msg + strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));

	if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) || IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher))
		sprintf(msg + strlen(msg), "%-12s", "TKIPAES");
	else
		sprintf(msg + strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
		sprintf(msg + strlen(msg), "%s\n", "");
	else if (Profile.WscEncrypType == 2) {
		sprintf(msg + strlen(msg), "%-13d", Profile.DefaultKeyIdx);
		sprintf(msg + strlen(msg), "%-26s\n", Profile.WscWPAKey);
	} else if (Profile.WscEncrypType >= 4)
		sprintf(msg + strlen(msg), "%-64s\n", Profile.WscWPAKey);

#ifdef INF_AR9
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
#endif/* INF_AR9 */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	os_free_mem(msg);
}

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)

/*
    ==========================================================================
    Description:
	Get WSC Profile
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 ar9_show get_wsc_profile
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPAR9IoctlWscProfile(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	WSC_CONFIGURED_VALUE Profile;
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
	Profile.WscConfigured = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32 + 1);
	NdisMoveMemory(Profile.WscSsid, pAd->ApCfg.MBSSID[apidx].Ssid,
				   pAd->ApCfg.MBSSID[apidx].SsidLen);
	Profile.WscSsid[pAd->ApCfg.MBSSID[apidx].SsidLen] = '\0';
	Profile.WscAuthMode = WscGetAuthType(wdev->SecConfig.AKMMap);
	Profile.WscEncrypType = WscGetEncryType(wdev->SecConfig.PairwiseCipher);
	NdisZeroMemory(Profile.WscWPAKey, 64 + 1);

	if (Profile.WscEncrypType == 2) {
		Profile.DefaultKeyIdx = wdev->SecConfig.PairwiseKeyId + 1;
		{
			int i;

			for (i = 0; i < wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen; i++) {
				snprintf((RTMP_STRING *) Profile.WscWPAKey, sizeof(Profile.WscWPAKey),
						 "%s%02x", Profile.WscWPAKey,
						 wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].Key[i]);
			}

			Profile.WscWPAKey[(wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen) * 2] = '\0';
		}
	} else if (Profile.WscEncrypType >= 4) {
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPsk,
					   pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen);
		Profile.WscWPAKey[pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen] = '\0';
	} else
		Profile.DefaultKeyIdx = 1;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR) * (2048));

	if (msg == NULL)
		return;

	memset(msg, 0x00, 2048);
	sprintf(msg, "%s", "\n");

	if (Profile.WscEncrypType == 1)
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	else if (Profile.WscEncrypType == 2)
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType",
				"DefaultKeyID", "Key");
	else
		sprintf(msg + strlen(msg), "%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");

	if (Profile.WscConfigured == 1)
		sprintf(msg + strlen(msg), "%-12s", "No");
	else
		sprintf(msg + strlen(msg), "%-12s", "Yes");

	sprintf(msg + strlen(msg), "%-33s", Profile.WscSsid);

	if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
		sprintf(msg + strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		sprintf(msg + strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));

	if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) || IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher))
		sprintf(msg + strlen(msg), "%-12s", "TKIPAES");
	else
		sprintf(msg + strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
		sprintf(msg + strlen(msg), "%s\n", "");
	else if (Profile.WscEncrypType == 2) {
		sprintf(msg + strlen(msg), "%-13d", Profile.DefaultKeyIdx);
		sprintf(msg + strlen(msg), "%-26s\n", Profile.WscWPAKey);
	} else if (Profile.WscEncrypType >= 4)
		sprintf(msg + strlen(msg), "%-64s\n", Profile.WscWPAKey);

	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	}
	os_free_mem(msg);
}

VOID RTMPIoctlWscPINCode(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	UCHAR        tempPIN[9] = {0};

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (128));

	if (msg == NULL)
		return;

	memset(msg, 0x00, 128);
	sprintf(msg, "%s", "\n");
	sprintf(msg + strlen(msg), "WSC_PINCode=");

	if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCode) {
		if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCodeLen == 8)
			sprintf((RTMP_STRING *) tempPIN, "%08u", pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCode);
		else
			sprintf((RTMP_STRING *) tempPIN, "%04u", pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCode);

		sprintf(msg, "%s%s\n", msg, tempPIN);
	}

	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	}
	os_free_mem(msg);
}

VOID RTMPIoctlWscStatus(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (128));

	if (msg == NULL)
		return;

	memset(msg, 0x00, 128);
	sprintf(msg, "%s", "\n");
	sprintf(msg + strlen(msg), "WSC_Status=");
	sprintf(msg, "%s%d\n", msg, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus);
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	}
	os_free_mem(msg);
}

VOID RTMPIoctlGetWscDynInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT apidx, configstate;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (pAd->ApCfg.BssidNum * (14 * 128)));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		return;
	}

	memset(msg, 0, pAd->ApCfg.BssidNum * (14 * 128));
	sprintf(msg, "%s", "\n");

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss =  &pAd->ApCfg.MBSSID[apidx];

		if (pMbss->wdev.WscControl.WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
			configstate = 0;
		else
			configstate = 1;

		sprintf(msg + strlen(msg), "ra%d\n", apidx);
#ifdef BB_SOC
		sprintf(msg + strlen(msg), "DeviceName = %s\n", (pMbss->wdev.WscControl.RegData.SelfInfo.DeviceName));
#endif
		sprintf(msg + strlen(msg), "UUID = %s\n", (pMbss->wdev.WscControl.Wsc_Uuid_Str));
		sprintf(msg + strlen(msg), "wpsVersion = 0x%x\n", WSC_VERSION);
		sprintf(msg + strlen(msg), "setuoLockedState = %d\n", 0);
		sprintf(msg + strlen(msg), "configstate = %d\n", configstate);
		sprintf(msg + strlen(msg), "lastConfigError = %d\n", 0);
	}

	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));

	os_free_mem(msg);
}

VOID RTMPIoctlGetWscRegsDynInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT apidx;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (pAd->ApCfg.BssidNum * (14 * 128)));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __func__));
		return;
	}

	memset(msg, 0, pAd->ApCfg.BssidNum * (14 * 128));
	sprintf(msg, "%s", "\n");

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss =  &pAd->ApCfg.MBSSID[apidx];
		sprintf(msg + strlen(msg), "ra%d\n", apidx);
#ifdef BB_SOC
		sprintf(msg + strlen(msg), "DeviceName = %s\n", (pMbss->wdev.WscControl.RegData.SelfInfo.DeviceName));
#endif
		sprintf(msg + strlen(msg), "UUID_R = %s\n", (pMbss->wdev.WscControl.RegData.PeerInfo.Uuid));
	}

	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));

	os_free_mem(msg);
}
#endif /* defined(AR9_MAPI_SUPPORT) || defined(BB_SOC) */
#endif /* defined(INF_AR9) || defined(BB_SOC) */

BOOLEAN WscCheckEnrolleeNonceFromUpnp(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_STRING *pData,
	IN  USHORT			Length,
	IN  PWSC_CTRL       pWscControl)
{
	USHORT	WscType, WscLen;
	USHORT  WscId = WSC_ID_ENROLLEE_NONCE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("check Enrollee Nonce\n"));

	/* We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
	while (Length > 4) {
		WSC_IE	TLV_Recv;
		char ZeroNonce[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		if (WscType == WscId) {
			if (RTMPCompareMemory(pWscControl->RegData.SelfNonce, pData, 16) == 0) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Nonce match!!\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- WscCheckNonce\n"));
				return TRUE;
			} else if (NdisEqualMemory(pData, ZeroNonce, 16)) {
				/* Intel external registrar will send WSC_NACK with enrollee nonce */
				/* "10 1A 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00" */
				/* when AP is configured and user selects not to configure AP. */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Zero Enrollee Nonce!!\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- WscCheckNonce\n"));
				return TRUE;
			}
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Nonce mismatch!!\n"));
	return FALSE;
}

UCHAR	WscRxMsgTypeFromUpnp(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  RTMP_STRING *pData,
	IN	USHORT				Length)
{
	USHORT WscType, WscLen;
	{   /* Eap-Esp(Messages) */
		/* the first TLV item in EAP Messages must be WSC_IE_VERSION */
		NdisMoveMemory(&WscType, pData, 2);

		if (ntohs(WscType) != WSC_ID_VERSION)
			goto out;

		/* Not Wsc Start, We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
		while (Length > 4) {
			/* arm-cpu has packet alignment issue, it's better to use memcpy to retrieve data */
			NdisMoveMemory(&WscType, pData, 2);
			NdisMoveMemory(&WscLen,  pData + 2, 2);
			WscLen = ntohs(WscLen);

			if (ntohs(WscType) == WSC_ID_MSG_TYPE)
				return (*(pData + 4));	/* Found the message type */

			pData  += (WscLen + 4);
			Length -= (WscLen + 4);
		}
	}
out:
	return  WSC_MSG_UNKNOWN;
}

VOID RTMPIoctlSetWSCOOB(
	IN PRTMP_ADAPTER pAd)
{
	char        *pTempSsid = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;

	pObj->pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscPinCode_Proc:: Ap Client doesn't need this command.\n", apidx));
		return;
	}

#endif /* APCLI_SUPPORT */
	Set_WscSetupLock_Proc(pAd, "0");
	Set_AP_WscConfStatus_Proc(pAd, "1");
	Set_SecAuthMode_Proc(pAd, "WPAPSK");
	Set_SecEncrypType_Proc(pAd, "TKIP");
	pTempSsid = vmalloc(33);

	if (pTempSsid) {
		memset(pTempSsid, 0, 33);
		snprintf(pTempSsid, 33, "RalinkInitialAP%02X%02X%02X",
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[3],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[4],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[5]);
		Set_AP_SSID_Proc(pAd, pTempSsid);
		vfree(pTempSsid);
	}

	Set_SecWPAPSK_Proc(pAd, "RalinkInitialAPxx1234");
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscOOB_Proc\n", apidx));
}

/*
	==========================================================================
	Description:
	Set Wsc Security Mode
	0 : WPA2PSK AES
	1 : WPA2PSK TKIP
	2 : WPAPSK AES
	3 : WPAPSK TKIP
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT	Set_AP_WscSecurityMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;

	if (strcmp(arg, "0") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPA2PSKAES;
	else if (strcmp(arg, "1") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPA2PSKTKIP;
	else if (strcmp(arg, "2") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPAPSKAES;
	else if (strcmp(arg, "3") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPAPSKTKIP;
	else
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscSecurityMode_Proc::(WscSecurityMode=%d)\n",
			 apidx, pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode));
	return TRUE;
}

INT Set_AP_WscMultiByteCheck_Proc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			apidx = pObj->ioctl_if;
	BOOLEAN			bEnable = FALSE;
	PWSC_CTRL		pWpsCtrl = NULL;
	BOOLEAN			bFromApCli = FALSE;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWpsCtrl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(apcli%d) Set_WscConfMode_Proc:: This command is from apcli interface now.\n", apidx));
	} else
#endif /* APCLI_SUPPORT */
	{
		bFromApCli = FALSE;
		pWpsCtrl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("IF(ra%d) Set_WscConfMode_Proc:: This command is from ra interface now.\n", apidx));
	}

	if (strcmp(arg, "0") == 0)
		bEnable = FALSE;
	else if (strcmp(arg, "1") == 0)
		bEnable = TRUE;
	else
		return FALSE;

	if (pWpsCtrl->bCheckMultiByte != bEnable)
		pWpsCtrl->bCheckMultiByte = bEnable;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_AP_WscMultiByteCheck_Proc::(bCheckMultiByte=%d)\n",
			 apidx, pWpsCtrl->bCheckMultiByte));
	return TRUE;
}

INT	Set_WscVersion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	version = (UCHAR)os_str_tol(arg, 0, 16);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscVersion_Proc::(version=%x)\n", version));
	pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.RegData.SelfInfo.Version = version;
	return TRUE;
}

#ifdef VENDOR_FEATURE6_SUPPORT
INT	Set_WscUUID_STR_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (strlen(arg) ==	(UUID_LEN_STR - 1)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscUUID_E_Proc[%d]::(arg=%s)\n", pObj->ioctl_if, arg));
		pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl.Wsc_Uuid_Str[UUID_LEN_STR - 1] = 0;
		NdisMoveMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl.Wsc_Uuid_Str[0], arg, strlen(arg));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ERROR Set_WscUUID_E_Proc[%d]::(arg=%s), Leng(%d) is incorrect!\n", pObj->ioctl_if, arg, (int)strlen(arg)));
		return FALSE;
	}
}


INT	Set_WscUUID_HEX_E_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (strlen(arg) ==	(UUID_LEN_HEX * 2)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscUUID_HEX_E_Proc[%d]::(arg=%s)\n", pObj->ioctl_if, arg));
		AtoH(arg, &pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl.Wsc_Uuid_E[0], UUID_LEN_HEX);
		hex_dump("Set_WscUUID_HEX_E_Proc OK:", &pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl.Wsc_Uuid_E[0], UUID_LEN_HEX);
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ERROR Set_WscUUID_HEX_E_Proc[%d]::(arg=%s), Leng(%d) is incorrect!\n", pObj->ioctl_if, arg, (int)strlen(arg)));
		return FALSE;
	}
}
#endif /* VENDOR_FEATURE6_SUPPORT */

#ifdef WSC_V2_SUPPORT
INT	Set_WscFragment_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	bool_flag = (UCHAR)os_str_tol(arg, 0, 16);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscFragment_Proc::(bool_flag=%d)\n", bool_flag));
	pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.bWscFragment = bool_flag;
	return TRUE;
}

INT	Set_WscFragmentSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	USHORT		WscFragSize = (USHORT)os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscFragmentSize_Proc::(WscFragSize=%d)\n", WscFragSize));

	if ((WscFragSize >= 128) && (WscFragSize <= 300))
		pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.WscFragSize = WscFragSize;

	return TRUE;
}

INT	Set_WscSetupLock_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		bEnable = (UCHAR)os_str_tol(arg, 0, 10);
	PWSC_CTRL	pWscControl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl;

	if (bEnable == 0) {
		BOOLEAN bCancelled = FALSE;

		pWscControl->PinAttackCount = 0;

		if (pWscControl->WscSetupLockTimerRunning)
			RTMPCancelTimer(&pWscControl->WscSetupLockTimer, &bCancelled);

		WscSetupLockTimeout(NULL, pWscControl, NULL, NULL);
	} else {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

		pWscControl->bSetupLock = TRUE;
		WscBuildBeaconIE(pAd,
						 pWscControl->WscConfStatus,
						 FALSE,
						 0,
						 0,
						 pObj->ioctl_if,
						 NULL,
						 0,
						 AP_MODE);
		WscBuildProbeRespIE(pAd,
							WSC_MSGTYPE_AP_WLAN_MGR,
							pWscControl->WscConfStatus,
							FALSE,
							0,
							0,
							pObj->ioctl_if,
							NULL,
							0,
							AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscSetupLock_Proc::(bSetupLock=%d)\n",
			 pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.bSetupLock));
	return TRUE;
}

INT	Set_WscV2Support_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR bEnable = (UCHAR)os_str_tol(arg, 0, 10);
	PWSC_CTRL pWscControl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl;

	if (bEnable == 0)
		pWscControl->WscV2Info.bEnableWpsV2 = FALSE;
	else
		pWscControl->WscV2Info.bEnableWpsV2 = TRUE;

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		/*
			WPS V2 doesn't support Chiper WEP and TKIP.
		*/
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (IS_CIPHER_WEP_TKIP_ONLY(pSecConfig->PairwiseCipher)
			|| (pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid))
			WscOnOff(pAd, wdev->func_idx, TRUE);
		else
			WscOnOff(pAd, wdev->func_idx, FALSE);

		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
	} else
		WscInit(pAd, FALSE, pObj->ioctl_if);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscV2Support_Proc::(bEnableWpsV2=%d)\n",
			 pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.WscV2Info.bEnableWpsV2));
	return TRUE;
}

INT	Set_WscVersion2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	version = (UCHAR)os_str_tol(arg, 0, 16);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscVersion2_Proc::(version=%x)\n", version));

	if (version >= 0x20)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.RegData.SelfInfo.Version2 = version;
	else
		return FALSE;

	return TRUE;
}

INT	Set_WscExtraTlvTag_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	USHORT		new_tag = (USHORT)os_str_tol(arg, 0, 16);

	pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.WscV2Info.ExtraTlv.TlvTag = new_tag;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscExtraTlvTag_Proc::(new_tag=0x%04X)\n", new_tag));
	return TRUE;
}

INT	Set_WscExtraTlvType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		type = (UCHAR)os_str_tol(arg, 0, 10);

	pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.WscV2Info.ExtraTlv.TlvType = type;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscExtraTlvType_Proc::(type=%d)\n", type));
	return TRUE;
}

INT	Set_WscExtraTlvData_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT			DataLen = (UINT)strlen(arg);
	PWSC_TLV		pWscTLV = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.WscV2Info.ExtraTlv;
	INT				i;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscExtraTlvData_Proc::(DataLen = %d)\n", DataLen));

	if ((DataLen != 0) && (pWscTLV->TlvType == TLV_HEX)) {
		for (i = 0; i < DataLen; i++) {
			if (!isxdigit(*(arg + i)))
				return FALSE;  /*Not Hex value; */
		}
	}

	if (pWscTLV->pTlvData) {
		os_free_mem(pWscTLV->pTlvData);
		pWscTLV->pTlvData = NULL;
	}

	if (DataLen == 0)
		return TRUE;

	pWscTLV->TlvLen = 0;
	os_alloc_mem(NULL, &pWscTLV->pTlvData, DataLen);

	if (pWscTLV->pTlvData) {
		if (pWscTLV->TlvType == TLV_ASCII) {
			NdisMoveMemory(pWscTLV->pTlvData, arg, DataLen);
			pWscTLV->TlvLen = DataLen;
		} else {
			pWscTLV->TlvLen = DataLen / 2;
			AtoH(arg, pWscTLV->pTlvData, pWscTLV->TlvLen);
		}

		return TRUE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Set_WscExtraTlvData_Proc::os_alloc_mem fail\n"));
	return FALSE;
}

INT	Set_WscMaxPinAttack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		MaxPinAttack = (UCHAR)os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscMaxPinAttack_Proc::(MaxPinAttack=%d)\n", MaxPinAttack));
	pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.MaxPinAttack = MaxPinAttack;
	return TRUE;
}


INT	Set_WscSetupLockTime_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT		SetupLockTime = (UINT)os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscSetupLockTime_Proc::(SetupLockTime=%d)\n",
			 SetupLockTime));
	pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl.SetupLockTime = SetupLockTime;
	return TRUE;
}

#endif /* WSC_V2_SUPPORT */

INT	Set_WscAutoTriggerDisable_Proc(
	IN	RTMP_ADAPTER	 *pAd,
	IN	RTMP_STRING		*arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR bEnable = (UCHAR)os_str_tol(arg, 0, 10);
	PWSC_CTRL pWscCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.WscControl;

	if (bEnable == 0)
		pWscCtrl->bWscAutoTriggerDisable = FALSE;
	else
		pWscCtrl->bWscAutoTriggerDisable = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscAutoTriggerDisable_Proc::(bWscAutoTriggerDisable=%d)\n",
			 pWscCtrl->bWscAutoTriggerDisable));
	return TRUE;
}

#endif /* WSC_AP_SUPPORT */


#ifdef IAPP_SUPPORT
INT	Set_IappPID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	unsigned long IappPid;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	IappPid = os_str_tol(arg, 0, 10);
	RTMP_GET_OS_PID(pObj->IappPid, IappPid);
	pObj->IappPid_nr = IappPid;
	/*	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pObj->IappPid = %d", GET_PID_NUMBER(pObj->IappPid))); */
	return TRUE;
} /* End of Set_IappPID_Proc */
#endif /* IAPP_SUPPORT */


INT	Set_DisConnectSta_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT						i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (strlen(arg) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	if (NdisEqualMemory(&macAddr[0], &BROADCAST_ADDR[0], MAC_ADDR_LEN)) {
		Set_DisConnectAllSta_Proc(pAd, "2");
		return TRUE;
	}

	pEntry = MacTableLookup(pAd, macAddr);

	if (pEntry) {
		MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
		/*		MacTableDeleteEntry(pAd, pEntry->wcid, Addr); */
	}

	return TRUE;
}

#ifdef VENDOR_FEATURE7_SUPPORT
INT	Set_DisConnectBssSta_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT	i, bssId = -1;

	bssId = os_str_tol(arg, 0, 10);
	if (bssId >= pAd->ApCfg.BssidNum)
		return FALSE;
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC
			&& pEntry->func_tb_idx == bssId){
			MlmeDeAuthAction(pAd, pEntry, REASON_DEAUTH_STA_LEAVING, FALSE);
		}
	}
	return TRUE;
}
#endif
INT Set_DisConnectAllSta_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	INT i;
	MAC_TABLE_ENTRY *pEntry;

#ifdef DOT11W_PMF_SUPPORT
	CHAR value = os_str_tol(arg, 0, 10);

	if (value == 2) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("[PMF]%s:: apidx=%d\n", __func__, pObj->ioctl_if));
		APMlmeKickOutAllSta(pAd, pObj->ioctl_if, REASON_DEAUTH_STA_LEAVING);

		for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s: MacTableDeleteEntry %x:%x:%x:%x:%x:%x\n",
						 __func__, PRINT_MAC(pEntry->Addr)));
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			}
		}
	} else
#endif /* DOT11W_PMF_SUPPORT */
	{
		UCHAR *pOutBuffer = NULL;
		NDIS_STATUS NStatus;
		HEADER_802_11 DeAuthHdr;
		USHORT Reason;
		ULONG FrameLen = 0;

		for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];
			if (IS_ENTRY_CLIENT(pEntry)) {
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					/* Before reset MacTable, send disassociation packet to client.*/
					if (pEntry->Sst == SST_ASSOC) {
						/*  send out a De-authentication request frame*/
						NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
						if (NStatus != NDIS_STATUS_SUCCESS) {
							MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								(" MlmeAllocateMemory fail  ..\n"));
							return FALSE;
						}

						Reason = REASON_NO_LONGER_VALID;
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
							Reason, PRINT_MAC(pEntry->Addr)));
						MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
								pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
						MakeOutgoingFrame(pOutBuffer, &FrameLen,
								sizeof(HEADER_802_11), &DeAuthHdr,
								2, &Reason,
								END_OF_ARGS);

						MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
						MlmeFreeMemory(pOutBuffer);
						RtmpusecDelay(5000);
					}
				}
#endif /* CONFIG_AP_SUPPORT */
				/* Delete a entry via WCID */
				MacTableDeleteEntry(pAd, i, pEntry->Addr);
			}
		}
	}

	return TRUE;
}


#ifdef DOT1X_SUPPORT
/*
    ==========================================================================
    Description:
	Set IEEE8021X.
	This parameter is 1 when 802.1x-wep turn on, otherwise 0
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE8021X_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (os_str_tol(arg, 0, 10) != 0)	/*Enable*/
		pSecConfig->IEEE8021X = TRUE;
	else /*Disable*/
		pSecConfig->IEEE8021X = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) IEEE8021X=%d\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->IEEE8021X));
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set pre-authentication enable or disable when WPA/WPA2 turn on
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_PreAuth_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (os_str_tol(arg, 0, 10) != 0) /*Enable*/
		pSecConfig->PreAuth = TRUE;
	else /*Disable*/
		pSecConfig->PreAuth = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) PreAuth=%d\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuth));
	return TRUE;
}

INT	Set_OwnIPAddr_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;
	SetWdevOwnIPAddr(pSecConfig, arg);
	return TRUE;
}

INT	Set_EAPIfName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (strlen(arg) > 0 && strlen(arg) <= IFNAMSIZ) {
		pSecConfig->EAPifname_len = strlen(arg);
		NdisMoveMemory(pSecConfig->EAPifname, arg, strlen(arg));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EAPifname=%s, len=%d\n",
				 pSecConfig->EAPifname, pSecConfig->EAPifname_len));
	}

	return TRUE;
}

INT	Set_PreAuthIfName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (strlen(arg) > 0 && strlen(arg) <= IFNAMSIZ) {
		pSecConfig->PreAuthifname_len = strlen(arg);
		NdisMoveMemory(pSecConfig->PreAuthifname, arg, strlen(arg));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PreAuthifname=%s, len=%d\n",
				 pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len));
	}

	return TRUE;
}

INT	Set_RADIUS_Server_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UINT32 ip_addr;
	INT count;
	RTMP_STRING *macptr;
	INT srv_cnt = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	for (count = 0, macptr = rstrtok(arg, ";"); (macptr &&
			count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL, ";"), count++) {
		if (rtinet_aton(macptr, &ip_addr)) {
			PRADIUS_SRV_INFO pSrvInfo = &pSecConfig->radius_srv_info[srv_cnt];

			pSrvInfo->radius_ip = ip_addr;
			srv_cnt++;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d), radius_ip(seq-%d)=%s\n",
					 apidx, srv_cnt, arg));
		}

		if (srv_cnt > 0)
			pSecConfig->radius_srv_num = srv_cnt;
	}

	return TRUE;
}

INT	Set_RADIUS_Port_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr;
	INT count;
	INT srv_cnt = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	for (count = 0, macptr = rstrtok(arg, ";"); (macptr &&
			count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL, ";"), count++) {
		if (srv_cnt < pSecConfig->radius_srv_num) {
			PRADIUS_SRV_INFO pSrvInfo = &pSecConfig->radius_srv_info[srv_cnt];

			pSrvInfo->radius_port = (UINT32) os_str_tol(macptr, 0, 10);
			srv_cnt++;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d), radius_port(seq-%d)=%d\n",
					 apidx, srv_cnt, pSrvInfo->radius_port));
		}
	}

	return TRUE;
}

INT	Set_RADIUS_Key_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr;
	INT count;
	INT srv_cnt = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	for (count = 0, macptr = rstrtok(arg, ";"); (macptr &&
			count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL, ";"), count++) {
		if (strlen(macptr) > 0 && strlen(macptr) < 65 && srv_cnt < pSecConfig->radius_srv_num) {
			PRADIUS_SRV_INFO pSrvInfo = &pSecConfig->radius_srv_info[srv_cnt];

			pSrvInfo->radius_key_len = strlen(macptr);
			NdisMoveMemory(pSrvInfo->radius_key, macptr, pSrvInfo->radius_key_len);
			srv_cnt++;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d), radius_key(seq-%d)=%s, len=%d\n",
					 apidx, srv_cnt, pSrvInfo->radius_key, pSrvInfo->radius_key_len));
		}
	}

	return TRUE;
}


INT	Set_DeletePMKID_Proc(RTMP_ADAPTER *pAd,
						 RTMP_STRING *arg)  /* for testing sending deauth frame if PMKID not found */
{
	UCHAR apidx = os_str_tol(arg, 0, 10);
	INT32 i = 0;

	for (i = 0; i < MAX_PMKID_COUNT; i++)
		RTMPDeletePMKIDCache(&pAd->ApCfg.PMKIDCache, apidx, i);

	return TRUE;
}


INT	Set_DumpPMKID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR imput = os_str_tol(arg, 0, 10);
	INT32 i = 0;

	if (imput == 1) {
		for (i = 0; i < MAX_PMKID_COUNT; i++) {
			PAP_BSSID_INFO pBssInfo = &pAd->ApCfg.PMKIDCache.BSSIDInfo[i];

			if (pBssInfo->Valid) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("Cacheidx = %d, Mbssidx = %d, Mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
						  i, pBssInfo->Mbssidx, PRINT_MAC(pBssInfo->MAC)));
			}
		}
	}

	return TRUE;
}

#ifdef RADIUS_MAC_ACL_SUPPORT
INT show_RADIUS_acl_cache(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	PRT_802_11_RADIUS_ACL_ENTRY pCacheEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	PLIST_HEADER pListHeader = NULL;

	pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

	if (pListHeader->size != 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IF(ra%d), Radius ACL Cache List:\n", apidx));
		pListEntry = pListHeader->pHead;
		pCacheEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;

		while (pCacheEntry != NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x:%02x:%02x:%02x:%02x:%02x --> %d\n",
					 PRINT_MAC(pCacheEntry->Addr), pCacheEntry->result));
			pListEntry = pListEntry->pNext;
			pCacheEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;
		}
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IF(ra%d), Radius ACL Cache empty\n", apidx));

	return TRUE;
}

INT Set_RADIUS_CacheTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	CHAR        val  = os_str_tol(arg, 0, 10);

	if (val > 0) {
		pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCacheTimeout = val;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ra[%d] Radius Cache Timeout: %d\n",
				 apidx, val));
		return TRUE;
	}

	return FALSE;
}

INT Set_RADIUS_MacAuth_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	CHAR        val  = os_str_tol(arg, 0, 10);

	if (val == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy = RADIUS_MAC_AUTH_DISABLE;
	else if (val == 1)
		pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy = RADIUS_MAC_AUTH_ENABLE;
	else
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IF(ra%d), Radius MAC Auth: %d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy));
	return TRUE;
}
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

#ifdef UAPSD_SUPPORT
INT Set_UAPSD_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IdMbss = pObj->ioctl_if;

	if (os_str_tol(arg, 0, 10) != 0)
		pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo.bAPSDCapable = TRUE;
	else
		pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo.bAPSDCapable = FALSE;

	return TRUE;
} /* End of Set_UAPSD_Proc */
#endif /* UAPSD_SUPPORT */



#ifdef MCAST_RATE_SPECIFIC
INT Set_McastPhyMode(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	HTTRANSMIT_SETTING *pTransmit;
	BOOLEAN isband5g, tmp_band;
	UCHAR cfg_ht_bw;
	struct wifi_dev *wdev = NULL;
	INT i = 0;
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR PhyMode = os_str_tol(arg, 0, 10);

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Do nothing! This device interface is NOT AP mode!\n"));
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Invalid device interface!\n"));
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	isband5g = (wdev->channel > 14) ? TRUE : FALSE;

	if (isband5g) {
		if (PhyMode == MCAST_CCK) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("We could not set CCK mode for multicast frames in 5G band!\n"));
			return FALSE;
		}

		pTransmit = &pAd->CommonCfg.MCastPhyMode_5G;
	} else {
#ifdef DOT11_VHT_AC

		if (PhyMode == MCAST_VHT) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("We could not set VHT mode for multicast frames in 2.4G band!\n"));
			return FALSE;
		}

#endif /* DOT11_VHT_AC */
		pTransmit = &pAd->CommonCfg.MCastPhyMode;
	}

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	pTransmit->field.BW = cfg_ht_bw;

	switch (PhyMode) {
	case MCAST_DISABLE: /* disable */
		NdisMoveMemory(pTransmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
		pTransmit->field.BW =  BW_20;
		/* NdisMoveMemory(&pAd->CommonCfg.MCastPhyMode, &pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING)); */
		break;

	case MCAST_CCK:	/* CCK */
		pTransmit->field.MODE = MODE_CCK;
		pTransmit->field.BW =  BW_20;

		if ((pTransmit->field.MCS > 11) || (pTransmit->field.MCS > 3 && pTransmit->field.MCS < 8))
			pTransmit->field.MCS = 3;

		break;

	case MCAST_OFDM:	/* OFDM */
		pTransmit->field.MODE = MODE_OFDM;
		pTransmit->field.BW =  BW_20;

		if (pTransmit->field.MCS > 7)
			pTransmit->field.MCS = 7;

		break;
#ifdef DOT11_N_SUPPORT

	case MCAST_HTMIX:	/* HTMIX */
		pTransmit->field.MODE = MODE_HTMIX;

		if ((isband5g) && (wlan_operate_get_bw(wdev) > BW_20))
			pTransmit->field.BW =  BW_40;
		else
			pTransmit->field.BW =  BW_20;

		break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	case MCAST_VHT: /* VHT */
		pTransmit->field.MODE = MODE_VHT;
		pTransmit->field.BW = wlan_operate_get_bw(wdev);
		break;
#endif /* DOT11_VHT_AC */

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("unknown Muticast PhyMode %d.\n", PhyMode));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("0:Disable, 1:CCK, 2:OFDM, 3:HTMIX, 4:VHT.\n"));
		status = FALSE;
		break;
	}

	if (!status)
		return FALSE;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		tmp_band = (wdev->channel > 14) ? TRUE : FALSE;

		if (tmp_band != isband5g)
			continue;

		NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
		bss_info_argument.bss_state = BSS_ACTIVE;
		bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
		bss_info_argument.u4BssInfoFeature = BSS_INFO_BROADCAST_INFO_FEATURE;
		memmove(&bss_info_argument.BcTransmit, pTransmit, sizeof(HTTRANSMIT_SETTING));
		memmove(&bss_info_argument.McTransmit, pTransmit, sizeof(HTTRANSMIT_SETTING));

		if (AsicBssInfoUpdate(pAd, bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("Fail to apply the bssinfo, BSSID=%d!\n", i));
	}

	return TRUE;
}


INT Set_McastMcs(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	BOOLEAN isband5g, tmp_band;
	HTTRANSMIT_SETTING *pTransmit;
	struct wifi_dev *wdev = NULL;
	INT i = 0;
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR Mcs = os_str_tol(arg, 0, 10);

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Do nothing! This device interface is NOT AP mode!\n"));
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Invalid device interface!\n"));
		return FALSE;
	}

	if (Mcs > 15) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Mcs must be in range of 0 to 15\n"));
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	isband5g = (wdev->channel > 14) ? TRUE : FALSE;
	pTransmit = (isband5g) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);

	switch (pTransmit->field.MODE) {
	case MODE_CCK:
		if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11))
			pTransmit->field.MCS = Mcs;
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n"));
			status = FALSE;
		}

		break;

	case MODE_OFDM:
		if (Mcs > 7) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("MCS must in range from 0 to 7 for OFDM Mode.\n"));
			status = FALSE;
		} else
			pTransmit->field.MCS = Mcs;

		break;

	default:
		pTransmit->field.MCS = Mcs;
		break;
	}

	if (!status)
		return FALSE;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		tmp_band = (wdev->channel > 14) ? TRUE : FALSE;

		if (tmp_band != isband5g)
			continue;

		NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
		bss_info_argument.bss_state = BSS_ACTIVE;
		bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
		bss_info_argument.u4BssInfoFeature = BSS_INFO_BROADCAST_INFO_FEATURE;
		memmove(&bss_info_argument.BcTransmit, pTransmit, sizeof(HTTRANSMIT_SETTING));
		memmove(&bss_info_argument.McTransmit, pTransmit, sizeof(HTTRANSMIT_SETTING));

		if (AsicBssInfoUpdate(pAd, bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("Fail to apply the bssinfo, BSSID=%d!\n", i));
	}

	return TRUE;
}

INT Show_McastRate(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	HTTRANSMIT_SETTING *pTransmit;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Do nothing! This device interface is NOT AP mode!\n"));
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Invalid device interface!\n"));
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pTransmit = (wdev->channel > 14) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Mcast PhyMode = %d\n", pTransmit->field.MODE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Mcast Mcs = %d\n", pTransmit->field.MCS));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Mcast BW = %d\n", pTransmit->field.BW));
	return TRUE;
}
#endif /* MCAST_RATE_SPECIFIC */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
INT Set_OBSSScanParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ObssScanValue;
	UINT Idx;
	RTMP_STRING *thisChar;

	Idx = 0;

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		ObssScanValue = (INT) os_str_tol(thisChar, 0, 10);

		switch (Idx) {
		case 0:
			if (ObssScanValue < 5 || ObssScanValue > 1000)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue));
			else {
				pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000 */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n",
						 ObssScanValue));
			}

			break;

		case 1:
			if (ObssScanValue < 10 || ObssScanValue > 1000)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue));
			else {
				pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000 */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanActiveDwell=%d\n",
						 ObssScanValue));
			}

			break;

		case 2:
			pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n",
					 ObssScanValue));
			break;

		case 3:
			if (ObssScanValue < 200 || ObssScanValue > 10000)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue));
			else {
				pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000 */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n",
						 ObssScanValue));
			}

			break;

		case 4:
			if (ObssScanValue < 20 || ObssScanValue > 10000)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue));
			else {
				pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000 */
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
			pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
					 ObssScanValue));
			break;
		}

		Idx++;
	}

	if (Idx != 7) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Wrong OBSSScanParamtetrs format in ioctl cmd!!!!! Use default value\n"));
		pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000 */
		pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000 */
		pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second */
		pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
		pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
		pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
		pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage */
	}

	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n",
			 pAd->CommonCfg.Dot11BssWidthChanTranDelay));
	return TRUE;
}

INT	Set_AP2040ReScan_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	APOverlappingBSSScan(pAd, wdev);
	/* apply setting */
	SetCommonHtVht(pAd, wdev);
	wlan_operate_set_prim_ch(wdev, wdev->channel);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_AP2040ReScan_Proc() Trigger AP ReScan !!!\n"));
	return TRUE;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

INT Set_EntryLifeCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG LifeCheckCnt = (ULONG) os_str_tol(arg, 0, 10);

	if (LifeCheckCnt <= 65535)
		pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("LifeCheckCnt must in range of 0 to 65535\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EntryLifeCheck Cnt = %ld.\n", pAd->ApCfg.EntryLifeCheck));
	return TRUE;
}

INT	ApCfg_Set_PerMbssMaxStaNum_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN INT				apidx,
	IN RTMP_STRING *arg)
{
	pAd->ApCfg.MBSSID[apidx].MaxStaNum = (UCHAR)os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) %s::(MaxStaNum=%d)\n",
			 apidx, __func__, pAd->ApCfg.MBSSID[apidx].MaxStaNum));
	return TRUE;
}

INT	ApCfg_Set_IdleTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG idle_time;

	idle_time = os_str_tol(arg, 0, 10);

	if (idle_time < MAC_TABLE_MIN_AGEOUT_TIME)
		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_MIN_AGEOUT_TIME;
	else
		pAd->ApCfg.StaIdleTimeout = idle_time;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : IdleTimeout=%d\n", __func__, pAd->ApCfg.StaIdleTimeout));
	return TRUE;
}

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	BOOLEAN bWlanLed;
#if defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT)

	if (!IS_RBUS_INF(pAd)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : Support RBUS interface only\n", __func__));
		return TRUE;
	}

#endif /* defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT) */
	bWlanLed = (BOOLEAN) os_str_tol(arg, 0, 10);
	{
		if (bWlanLed)
			RTMPStartLEDMode(pAd);
		else
			RTMPExitLEDMode(pAd);
	};
	return TRUE;
}
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

INT	Set_MemDebug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef VENDOR_FEATURE2_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Number of Packet Allocated = %lu\n", OS_NumOfPktAlloc));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Number of Packet Freed = %lu\n", OS_NumOfPktFree));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Offset of Packet Allocated/Freed = %lu\n",
			 OS_NumOfPktAlloc - OS_NumOfPktFree));
#endif /* VENDOR_FEATURE2_SUPPORT */
	return TRUE;
}


#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/

#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Set power save life time.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
INT	Set_PowerSaveLifeTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->MacTab.MsduLifeTime = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set new life time = %d\n", pAd->MacTab.MsduLifeTime));
	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef MBSS_SUPPORT
/*
========================================================================
Routine Description:
	Show MBSS information.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
extern UCHAR *wmode_2_str(UCHAR wmode);


INT	Show_MbssInfo_Display_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 IdBss;
	UCHAR PhyMode;
	CHAR *mod_str = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n\tBSS Idx\t\tPhy Mode\n"));

	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		PhyMode = pAd->ApCfg.MBSSID[IdBss].wdev.PhyMode;
		mod_str = wmode_2_str(PhyMode);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\t%d\t\t%s\n", IdBss, mod_str));

		if (mod_str)
			os_free_mem(mod_str);
	}

	return TRUE;
} /* End of Show_MbssInfo_Display_Proc */
#endif /* MBSS_SUPPORT */


#ifdef HOSTAPD_SUPPORT
VOID RtmpHostapdSecuritySet(
	IN	RTMP_ADAPTER			*pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrqin)
{
	if (wrqin->u.data.length > 20 && MAX_LEN_OF_RSNIE > wrqin->u.data.length && wrqin->u.data.pointer) {
		UCHAR RSNIE_Len[2];
		UCHAR RSNIe[2];
		int offset_next_ie = 0;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ioctl SIOCSIWGENIE pAd->IoctlIF=%d\n", apidx));
		RSNIe[0] =  *(UINT8 *)wrqin->u.data.pointer;

		if (IE_WPA != RSNIe[0] && IE_RSN != RSNIe[0]) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IE %02x != 0x30/0xdd\n", RSNIe[0]));
			Status = -EINVAL;
			break;
		}

		RSNIE_Len[0] =  *((UINT8 *)wrqin->u.data.pointer + 1);

		if (wrqin->u.data.length != RSNIE_Len[0] + 2) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IE use WPA1 WPA2\n"));
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].RSN_IE[1], MAX_LEN_OF_RSNIE);
			RSNIe[1] =  *(UINT8 *)wrqin->u.data.pointer;
			RSNIE_Len[1] =  *((UINT8 *)wrqin->u.data.pointer + 1);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IE1 %02x %02x\n", RSNIe[1], RSNIE_Len[1]));
			pAd->ApCfg.MBSSID[apidx].RSNIE_Len[1] = RSNIE_Len[1];
			NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].RSN_IE[1], (UCHAR *)(wrqin->u.data.pointer) + 2, RSNIE_Len[1]);
			offset_next_ie = RSNIE_Len[1] + 2;
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IE use only %02x\n", RSNIe[0]));

		NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].RSN_IE[0], MAX_LEN_OF_RSNIE);
		RSNIe[0] =  *(((UINT8 *)wrqin->u.data.pointer) + offset_next_ie);
		RSNIE_Len[0] =  *(((UINT8 *)wrqin->u.data.pointer) + offset_next_ie + 1);

		if (IE_WPA != RSNIe[0] && IE_RSN != RSNIe[0]) {
			Status = -EINVAL;
			break;
		}

		pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0] = RSNIE_Len[0];
		NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].RSN_IE[0], ((UCHAR *)(wrqin->u.data.pointer)) + 2 + offset_next_ie,
					   RSNIE_Len[0]);
		UpdateBeaconHandler(
			pAd,
			&pAd->ApCfg.MBSSID[apidx].wdev,
			BCN_UPDATE_IF_STATE_CHG);
	}
}
#endif /* HOSTAPD_SUPPORT */

#ifdef MT_MAC
/* Only for TimReq frame generating */
/* TODO: Tim service establish. */
/* used to mapping, tttt hwidx and  related CR */
static TTTT_CR_BSSID_IDX_MAP_T g_arTtttCrHwBssidMapTable[] = {
	{HW_BSSID_0,    LPON_TT0TPCR,    LPON_TT4TPCR},
	{HW_BSSID_1,    LPON_TT1TPCR,    LPON_TT5TPCR},
	{HW_BSSID_2,    LPON_TT2TPCR,    LPON_TT6TPCR},
	{HW_BSSID_3,    LPON_TT2TPCR,    LPON_TT7TPCR},
};

INT	Set_AP_TimEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 interval = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;
	UINT32 Value = 0;
	struct wifi_dev *wdev = NULL;
	UINT8 OmacIdx = 0;
	UINT8 BssidIdxMapped = 0;
	PTTTT_CR_BSSID_IDX_MAP_T    pTtttCrMapTable = NULL;
	UINT32 u4TtttEnCr = 0, u4TtttIntervelCr = 0;
	BOOLEAN     fgEnable = FALSE;

	interval = os_str_tol(arg, 0, 10);

	if ((interval < 0) || (interval > 255)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: interval is incorrect!!\n", __func__));
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[APIndex];
	wdev = &pMbss->wdev;

	if (wdev != NULL) {
		OmacIdx = wdev->OmacIdx;
		if (wdev_tim_buf_init(pAd, &wdev->bcn_buf.tim_buf) == FALSE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: tim packet init failed!\n", __func__));
			return FALSE;
		}

		/* ext mbss tttt, depends on MBSS_0 */
		if (OmacIdx >= HW_BSSID_MAX)
			BssidIdxMapped = HW_BSSID_0;
		else
			BssidIdxMapped = OmacIdx;

		MTWF_LOG(DBG_CAT_CFG,
				 DBG_SUBCAT_ALL,
				 DBG_LVL_ERROR,
				 ("%s: BssidIdxMapped = %d, OmacIdx = %x\n",
				  __func__,
				  BssidIdxMapped,
				  OmacIdx));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: idx = %d, no wdev!?\n", __func__, APIndex));
		return FALSE;
	}

	pTtttCrMapTable = &g_arTtttCrHwBssidMapTable[BssidIdxMapped];
	u4TtttEnCr = pTtttCrMapTable->u4TTTTEnableCr;
	u4TtttIntervelCr = pTtttCrMapTable->u4TTTTIntervalCr;

	if (interval > 0) { /* valid interval --> enable */
		fgEnable = TRUE;
		MAC_IO_WRITE32(pAd, LPON_PTTISR, IDX_DEFAULT_PRETTTT_INTERVAL(BssidIdxMapped));
		MAC_IO_READ32(pAd, u4TtttEnCr, &Value);
		Value &= ~TTTTn_OFFSET_OF_TBTTn_MASK;
		Value |= DEFAULT_TTTT_OFFSET_IN_MS;
		Value |= TTTTn_CAL_EN;
		MAC_IO_WRITE32(pAd, u4TtttEnCr, Value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("u4TtttEnCr = %x, Value = %x\n", u4TtttEnCr, Value));
		MAC_IO_WRITE32(pAd, u4TtttIntervelCr, SET_TTTT_PERIOD(pAd, interval));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("u4TtttIntervelCr = %x, Value = %x\n", u4TtttIntervelCr,
				 SET_TTTT_PERIOD(pAd, interval)));
		MAC_IO_READ32(pAd, LPON_MPTCR4, &Value);
		Value |= TTTT0_PERIODTIMER_EN |
				 TTTT0_TIMEUP_EN |
				 PRETTTT0TIMEUP_EN |
				 PRETTTT0_TRIG_EN |
				 PRETTTT0INT_EN;
		MAC_IO_WRITE32(pAd, LPON_MPTCR4, Value);
		MAC_IO_READ32(pAd, AGG_ARCR, &Value);
		Value = Value | DUAL_BTIM_EN;
		MAC_IO_WRITE32(pAd, AGG_ARCR, Value);
		Value = 0 | AGG_BTIMRR_TX_MODE(Legacy_CCK) | AGG_BTIMRR_TX_RATE(LONG_PREAMBLE_1M);
		MAC_IO_WRITE32(pAd, AGG_BTIMRR0, Value);
		/*TODO: other omac idx rate setting.*/
		MAC_IO_READ32(pAd, ARB_BTIMCR0, &Value);
		Value |= (1 << OmacIdx);
		MAC_IO_WRITE32(pAd, ARB_BTIMCR0, Value);/* enable BTIM_EN bit in ARB_BTIMCR0 */
		MAC_IO_WRITE32(pAd, ARB_BTIMCR1, Value);/* enable 2 TIM broadcast */
#ifndef BCN_OFFLOAD_SUPPORT
		MAC_IO_WRITE32(pAd, HWIER4, 0x80008000);/* enable TTTT0/PRETTTT0 interrupt. */
#endif
	} else { /* interval 0 --> disable */
		MAC_IO_READ32(pAd, ARB_BTIMCR0, &Value);
		Value &= ~(1 << OmacIdx);
		MAC_IO_WRITE32(pAd, ARB_BTIMCR0, Value);
		MAC_IO_WRITE32(pAd, ARB_BTIMCR1, Value);
	}

	AsicSetExtTTTTHwCRSetting(pAd, APIndex, fgEnable);
	return TRUE;
}

INT
Show_TmrCalResult_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg
)
{
	if (pAd->pTmrCtrlStruct)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): TmrCalResult=0x%X\n", __func__, pAd->pTmrCtrlStruct->TmrCalResult));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): (X) pTmrCtrlStruct is NULL\n", __func__));

	return TRUE;
}

#ifdef DBG
INT set_tim_update_proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg)
{
	UpdateBeaconHandler(pAd, NULL, BCN_UPDATE_TIM);
	return TRUE;
}

INT Set_AP_DumpTime_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg)
{
	int apidx = 0, i = 0;
	BSS_STRUCT *pMbss;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n\t%-10s\t%-10s\n", "PreTBTTTime", "TBTTTime"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%-10lu\t%-10lu\n", pAd->HandlePreInterruptTime,
			 pAd->HandleInterruptTime));

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss = &pAd->ApCfg.MBSSID[apidx];

		if (!BeaconTransmitRequired(pAd, &pMbss->wdev, TRUE))
			continue;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n%s:%d\tBcn_State:%d\t%-10s: %d\n", "Apidx", apidx,
				 pMbss->wdev.bcn_buf.bcn_state, "recover", pMbss->bcn_recovery_num));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t%-10s\t%-10s\t%-10s\t%-10s\n", "WriteBcnRing", "BcnDmaDone",
				 "TXS_TSF", "TXS_SN"));

		for (i = 0; i < MAX_TIME_RECORD; i++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Idx[%d]:\t%-10lu\t%-10lu\t%-10lu\t%-10lu\n", i,
					 pMbss->WriteBcnDoneTime[i], pMbss->BcnDmaDoneTime[i], pMbss->TXS_TSF[i], pMbss->TXS_SN[i]));
	}

	return TRUE;
}

INT Set_BcnStateCtrl_Proc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	UCHAR bcn_state;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	bcn_state = os_str_tol(arg, 0, 10);

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: chipcap is not HIF_MT\n", __func__));
		return FALSE;
	}

	if ((bcn_state < BCN_TX_IDLE) || (bcn_state > BCN_TX_DMA_DONE)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: bcn_state is incorrect!!\n", __func__));
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[apIndex];
	ASSERT(pMbss);

	if (pMbss == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pMbss == NULL!!\n", __func__));
		return FALSE;
	}

	if (pMbss->wdev.bcn_buf.bcn_state != BCN_TX_IDLE) {
		RTMP_SEM_LOCK(&hif->BcnRingLock);
		pMbss->wdev.bcn_buf.bcn_state = bcn_state;
		RTMP_SEM_UNLOCK(&hif->BcnRingLock);
	}

	return TRUE;
}
#endif

INT setApTmrEnableProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	LONG enable;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;
	UINT32  value = 0;
	struct wifi_dev *wdev;
	UCHAR bw;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	enable = os_str_tol(arg, 0, 10);

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: chip_cap is not HIF_MT\n", __func__));
		return FALSE;
	}

	if ((enable < 0) || (enable > 2)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: enable is incorrect!!\n", __func__));
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[apIndex];
	ASSERT(pMbss);

	if (pMbss == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pMbss == NULL!!\n", __func__));
		return FALSE;
	}

	wdev = &pMbss->wdev;
	bw = wlan_operate_get_bw(wdev);

	switch (enable) {
	case TMR_INITIATOR: {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: enable TMR report, as Initialiter\n", __func__));
		RTMP_IO_READ32(pAd, RMAC_TMR_PA, &value);
		value = value | BIT31;
		value = value & ~BIT30;
		RTMP_IO_WRITE32(pAd, RMAC_TMR_PA, value);
		cap->TmrEnable = 1;
		MtCmdTmrCal(pAd,
					enable,
					(pMbss->wdev.channel > 14 ? _A_BAND : _G_BAND),
					bw,
					0,/* Ant 0 at present */
					TMR_INITIATOR);
	}
	break;

	case TMR_RESPONDER: {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: enable TMR report, as Responser\n", __func__));
		RTMP_IO_READ32(pAd, RMAC_TMR_PA, &value);
		value = value | BIT31;
		value = value | BIT30;
		value = value | 0x34;/* Action frame register */
		RTMP_IO_WRITE32(pAd, RMAC_TMR_PA, value);
		cap->TmrEnable = 2;
		MtCmdTmrCal(pAd,
					enable,
					(pMbss->wdev.channel > 14 ? _A_BAND : _G_BAND),
					bw,
					0,/* Ant 0 at present */
					TMR_RESPONDER);
	}
	break;

	case TMR_DISABLE:
	default: {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: disable TMR report\n", __func__));
		RTMP_IO_READ32(pAd, RMAC_TMR_PA, &value);
		value = value & ~BIT31;
		RTMP_IO_WRITE32(pAd, RMAC_TMR_PA, value);
		cap->TmrEnable = FALSE;
		MtCmdTmrCal(pAd,
					enable,
					(pMbss->wdev.channel > 14 ? _A_BAND : _G_BAND),
					bw,
					0,/* Ant 0 at present */
					TMR_DISABLE);
	}
	}

	return TRUE;
}


#ifdef CUT_THROUGH
INT Set_CutThrough_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT mode;

	mode = os_str_tol(arg, 0, 10);
	cut_through_set_mode(pAd->PktTokenCb, mode);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s():Set CutThroughType as %d\n",
			  __func__, cut_through_get_mode(pAd->PktTokenCb)));
	return TRUE;
}
#endif /* CUT_THROUGH */
#endif /* MT_MAC */


/*
========================================================================
Routine Description:
	Driver Ioctl for AP.

Arguments:
	pAdSrc			- WLAN control block pointer
	wrq				- the IOCTL parameters
	cmd				- the command ID
	subcmd			- the sub-command ID
	pData			- the IOCTL private data pointer
	Data			- the IOCTL private data

Return Value:
	NDIS_STATUS_SUCCESS	- IOCTL OK
	Otherwise			- IOCTL fail

Note:
========================================================================
*/
INT RTMP_AP_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS;

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_SET:
		Status = RTMPAPPrivIoctlSet(pAd, wrq);
		break;

	case CMD_RT_PRIV_IOCTL:
		if (subcmd & OID_GET_SET_TOGGLE)
			Status = RTMPAPSetInformation(pAd, wrq,  (INT)subcmd);
		else {
#ifdef LLTD_SUPPORT

			if (subcmd == RT_OID_GET_PHY_MODE) {
				if (pData != NULL) {
					UINT modetmp = 0;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Query::Get phy mode (%02X)\n",
							 pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode));
					modetmp = (UINT) pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode;
					wrq->u.data.length = 1;

					/**(ULONG *)pData = (ULONG)pAd->ApCfg.MBSS[MAIN_MBSSID].wdev.PhyMode; */
					if (copy_to_user(pData, &modetmp, wrq->u.data.length))
						Status = -EFAULT;
				} else
					Status = -EFAULT;
			} else
#endif /* LLTD_SUPPORT */
				Status = RTMPAPQueryInformation(pAd, wrq, (INT)subcmd);
		}

		break;

	case CMD_RTPRIV_IOCTL_SHOW:
		Status = RTMPAPPrivIoctlShow(pAd, wrq);
		break;
#ifdef WSC_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_SET_WSCOOB:
		RTMPIoctlSetWSCOOB(pAd);
		break;
#endif/*WSC_AP_SUPPORT*/

	case CMD_RTPRIV_IOCTL_GET_MAC_TABLE:
		RTMPIoctlGetMacTable(pAd, wrq);
		break;
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)

	case CMD_RTPRIV_IOCTL_GSITESURVEY:
		RTMPIoctlGetSiteSurvey(pAd, wrq);
		break;
#endif /* AP_SCAN_SUPPORT */

	case CMD_RTPRIV_IOCTL_STATISTICS:
		RTMPIoctlStatistics(pAd, wrq);
		break;

	case CMD_MTPRIV_IOCTL_RD:
		RTMPIoctlRvRDebug(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_RX_STATISTICS:
		Status = RTMPIoctlRXStatistics(pAd, wrq);
		break;
#ifdef WSC_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_WSC_PROFILE:
		RTMPIoctlWscProfile(pAd, wrq);
		break;
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

	case CMD_RTPRIV_IOCTL_QUERY_BATABLE:
		RTMPIoctlQueryBaTable(pAd, wrq);
		break;
#endif /* DOT11_N_SUPPORT */

	case CMD_RTPRIV_IOCTL_E2P:
		RTMPAPIoctlE2PROM(pAd, wrq);
		break;
#ifdef DBG

	case CMD_RTPRIV_IOCTL_BBP:
		RTMPAPIoctlBBP(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_MAC:
		RTMPIoctlMAC(pAd, wrq);
		break;
#ifdef RTMP_RF_RW_SUPPORT

	case CMD_RTPRIV_IOCTL_RF:
		RTMPAPIoctlRF(pAd, wrq);
		break;
#endif /* RTMP_RF_RW_SUPPORT */
#endif /* DBG */

	case CMD_RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT:
		RTMPIoctlGetMacTableStaInfo(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIFHWADDR:
		if (pObj->ioctl_if < MAX_MBSSID_NUM(pAd))
			NdisCopyMemory((RTMP_STRING *) wrq->u.name, (RTMP_STRING *) pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.bssid, 6);

		break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWESSID: {
		RT_CMD_AP_IOCTL_SSID *pSSID = (RT_CMD_AP_IOCTL_SSID *)pData;
#ifdef APCLI_SUPPORT

		if (pSSID->priv_flags == INT_APCLI) {
			if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].Valid == TRUE) {
				pSSID->length = pAd->ApCfg.ApCliTab[pObj->ioctl_if].SsidLen;
				pSSID->pSsidStr = (char *)&pAd->ApCfg.ApCliTab[pObj->ioctl_if].Ssid;
			} else {
				pSSID->length = 0;
				pSSID->pSsidStr = NULL;
			}
		} else
#endif /* APCLI_SUPPORT */
		{
			pSSID->length = pAd->ApCfg.MBSSID[pSSID->apidx].SsidLen;
			pSSID->pSsidStr = (char *)pAd->ApCfg.MBSSID[pSSID->apidx].Ssid;
		}
	}
	break;
#ifdef MBSS_SUPPORT

	case CMD_RTPRIV_IOCTL_MBSS_BEACON_UPDATE:
		/* Carter, TODO. check this oid. */
		UpdateBeaconHandler(
			pAd,
			NULL,
			BCN_UPDATE_PRETBTT);
		break;

	case CMD_RTPRIV_IOCTL_MBSS_INIT:
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s -> CMD_RTPRIV_IOCTL_MBSS_INIT\n", __func__));
		MBSS_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_MBSS_REMOVE:
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s -> CMD_RTPRIV_IOCTL_MBSS_REMOVE\n", __func__));
		MBSS_Remove(pAd);
		break;
#ifdef MT_MAC

	case CMD_RTPRIV_IOCTL_MBSS_CR_ENABLE:
		if (ext_mbss_hw_cr_enable(pData) != 0)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_MBSS_CR_DISABLE:
		if (ext_mbss_hw_cr_disable(pData) != 0)
			return NDIS_STATUS_FAILURE;

		break;
#endif /* MT_MAC */
#endif /* MBSS_SUPPORT */

	case CMD_RTPRIV_IOCTL_WSC_INIT: {
#ifdef APCLI_SUPPORT
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
		PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
		APCLI_STRUCT *pApCliEntry = (APCLI_STRUCT *)pData;
		WSC_CTRL *wsc_ctrl;

		wsc_ctrl = &pApCliEntry->wdev.WscControl;
		WscGenerateUUID(pAd, &wsc_ctrl->Wsc_Uuid_E[0],
						&wsc_ctrl->Wsc_Uuid_Str[0], 0, FALSE, TRUE);
		wsc_ctrl->bWscFragment = FALSE;
		wsc_ctrl->WscFragSize = 128;
		wsc_ctrl->WscRxBufLen = 0;
		wsc_ctrl->pWscRxBuf = NULL;
		os_alloc_mem(pAd, &wsc_ctrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		if (wsc_ctrl->pWscRxBuf)
			NdisZeroMemory(wsc_ctrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		wsc_ctrl->WscTxBufLen = 0;
		wsc_ctrl->pWscTxBuf = NULL;
		os_alloc_mem(pAd, &wsc_ctrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		if (wsc_ctrl->pWscTxBuf)
			NdisZeroMemory(wsc_ctrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		initList(&wsc_ctrl->WscPeerList);
		NdisAllocateSpinLock(pAd, &wsc_ctrl->WscPeerListSemLock);
		wsc_ctrl->PinAttackCount = 0;
		wsc_ctrl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
		pWscV2Info = &wsc_ctrl->WscV2Info;
		pWscV2Info->bWpsEnable = TRUE;
		pWscV2Info->ExtraTlv.TlvLen = 0;
		pWscV2Info->ExtraTlv.TlvTag = 0;
		pWscV2Info->ExtraTlv.pTlvData = NULL;
		pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
		pWscV2Info->bEnableWpsV2 = TRUE;
#endif /* WSC_V2_SUPPORT */
		WscInit(pAd, TRUE, Data);
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
	}
	break;
#ifdef APCLI_SUPPORT

	case CMD_RTPRIV_IOCTL_APC_UP:
		ApCliIfUp(pAd);
		break;

	case CMD_RTPRIV_IOCTL_APC_DISCONNECT:
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, Data);
		RTMP_MLME_HANDLER(pAd);
		break;

	case CMD_RTPRIV_IOCTL_APC_INIT:
		APCli_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_APC_REMOVE:
		ApCli_Remove(pAd);
		break;
#endif /* APCLI_SUPPORT */

	case CMD_RTPRIV_IOCTL_PREPARE: {
		RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pData;

		pConfig->Status = RTMP_AP_IoctlPrepare(pAd, pData);

		if (pConfig->Status != 0)
			return NDIS_STATUS_FAILURE;
	}
	break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWAP: {
		UCHAR *pBssidDest = (UCHAR *)pData;
		PCHAR pBssidStr;
#ifdef APCLI_SUPPORT

		if (Data == INT_APCLI) {
			if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].Valid == TRUE)
				pBssidStr = (PCHAR)&APCLI_ROOT_BSSID_GET(pAd, pAd->ApCfg.ApCliTab[pObj->ioctl_if].MacTabWCID);
			else
				pBssidStr = NULL;
		} else
#endif /* APCLI_SUPPORT */
		{
			pBssidStr = (PCHAR) &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.bssid[0];
		}

		if (pBssidStr != NULL) {
			memcpy(pBssidDest, pBssidStr, ETH_ALEN);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IOCTL::SIOCGIWAP(=%02x:%02x:%02x:%02x:%02x:%02x)\n",
					 PRINT_MAC(pBssidStr)));
		} else
			memset(pBssidDest, 0, ETH_ALEN);
	}
	break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ:
		/* handle for SIOCGIWRATEQ */
	{
		RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
		HTTRANSMIT_SETTING HtPhyMode;
#ifdef APCLI_SUPPORT
			if (pRate->priv_flags == INT_APCLI)
				HtPhyMode = pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.HTPhyMode;
			else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
				if (pRate->priv_flags == INT_WDS)
					HtPhyMode = pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev.HTPhyMode;
				else
#endif /* WDS_SUPPORT */
				{
					HtPhyMode = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.HTPhyMode;
				}

		RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
						  HtPhyMode.field.BW, HtPhyMode.field.MCS,
						  pAd->Antenna.field.TxPath,
						  (UINT32 *)&pRate->BitRate);
	}
	break;
#ifdef HOSTAPD_SUPPORT

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ:
		RtmpHostapdSecuritySet(pAd, wrq);
		break;
#endif /* HOSTAPD_SUPPORT */

	default:
		Status = RTMP_COM_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);
		break;
	}

	return Status;
}


#ifdef DYNAMIC_VGA_SUPPORT
INT Set_DyncVgaEnable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = (Enable > 0) ? TRUE : FALSE;

	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable == TRUE)
		dynamic_vga_enable(pAd);
	else
		dynamic_vga_disable(pAd);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Set_DyncVgaEnable_Proc::(enable = %d)\n",
			  pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable));
	return TRUE;
}


INT set_false_cca_hi_th(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 val = os_str_tol(arg, 0, 10);

	pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = (val <= 0) ? 800 : val;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::(false cca high threshould = %d)\n",
			 __func__, pAd->CommonCfg.lna_vga_ctl.nFalseCCATh));
	return TRUE;
}


INT set_false_cca_low_th(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 val = os_str_tol(arg, 0, 10);

	pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = (val <= 0) ? 10 : val;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::(false cca low threshould = %d)\n",
			 __func__, pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh));
	return TRUE;
}
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef CONFIG_HOTSPOT
static INT Set_AP_HS_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 EID,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
	PGAS_CTRL pGasCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;

	switch (EID) {
	case IE_INTERWORKING:
		os_alloc_mem(NULL, &pGasCtrl->InterWorkingIE, IELen);
		NdisMoveMemory(pGasCtrl->InterWorkingIE, IE, IELen);
		pGasCtrl->InterWorkingIELen = IELen;
		pHSCtrl->AccessNetWorkType  = (*(IE + 2)) & 0x0F;

		if (IELen > 3) {
			pHSCtrl->IsHessid = TRUE;

			if (IELen == 7)
				NdisMoveMemory(pHSCtrl->Hessid, IE + 3, MAC_ADDR_LEN);
			else
				NdisMoveMemory(pHSCtrl->Hessid, IE + 5, MAC_ADDR_LEN);
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Interworking IE\n"));
		break;

	case IE_ADVERTISEMENT_PROTO:
		os_alloc_mem(NULL, &pGasCtrl->AdvertisementProtoIE, IELen);
		NdisMoveMemory(pGasCtrl->AdvertisementProtoIE, IE, IELen);
		pGasCtrl->AdvertisementProtoIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Advertisement Protocol IE\n"));
		break;

	case IE_QOS_MAP_SET: {
		int tmp = 0;
		char *pos = (char *)(IE + 2);

		os_alloc_mem(NULL, &pHSCtrl->QosMapSetIE, IELen);
		NdisMoveMemory(pHSCtrl->QosMapSetIE, IE, IELen);
		pHSCtrl->QosMapSetIELen = IELen;

		for (tmp = 0; tmp < (IELen - 16 - 2) / 2; tmp++) {
			pHSCtrl->DscpException[tmp] = *pos & 0xff;
			pHSCtrl->DscpException[tmp] |= (*(pos + 1) & 0xff) << 8;
			pos += 2;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DscpException[%d]:0x%x\n", tmp, pHSCtrl->DscpException[tmp]));
		}

		for (tmp = 0; tmp < 8; tmp++) {
			pHSCtrl->DscpRange[tmp] = *pos & 0xff;
			pHSCtrl->DscpRange[tmp] |= (*(pos + 1) & 0xff) << 8;
			pos += 2;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DscpRange[%d]:0x%x\n", tmp, pHSCtrl->DscpRange[tmp]));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("=========================================== Set Qos MAP Set IE\n"));
		break;
	}

	case IE_ROAMING_CONSORTIUM:
		os_alloc_mem(NULL, &pHSCtrl->RoamingConsortiumIE, IELen);
		NdisMoveMemory(pHSCtrl->RoamingConsortiumIE, IE, IELen);
		pHSCtrl->RoamingConsortiumIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Roaming Consortium IE\n"));
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown IE(EID = %d)\n", __func__, EID));
		break;
	}

	return TRUE;
}
#endif

static INT Set_AP_VENDOR_SPECIFIC_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OUIType,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
#ifdef CONFIG_HOTSPOT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
#endif

	switch (OUIType) {
#ifdef CONFIG_HOTSPOT

	case OUI_P2P:
		os_alloc_mem(NULL, &pHSCtrl->P2PIE, IELen);
		NdisMoveMemory(pHSCtrl->P2PIE, IE, IELen);
		pHSCtrl->P2PIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set P2P IE\n"));
		break;

	case OUI_HS2_INDICATION:
		os_alloc_mem(NULL, &pHSCtrl->HSIndicationIE, IELen);
		NdisMoveMemory(pHSCtrl->HSIndicationIE, IE, IELen);
		pHSCtrl->HSIndicationIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set HS2.0 Indication IE\n"));
		break;
#endif

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown OUIType = %d\n", __func__, OUIType));
		break;
	}

	return TRUE;
}

#ifdef CONFIG_DOT11V_WNM
static INT Set_AP_WNM_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 EID,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl =  &pAd->ApCfg.MBSSID[apidx].WNMCtrl;

	switch (EID) {
	case IE_TIME_ADVERTISEMENT:
		os_alloc_mem(NULL, &pWNMCtrl->TimeadvertisementIE, IELen);
		NdisMoveMemory(pWNMCtrl->TimeadvertisementIE, IE, IELen);
		pWNMCtrl->TimeadvertisementIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Time Advertisement IE\n"));
		break;

	case IE_TIME_ZONE:
		os_alloc_mem(NULL, &pWNMCtrl->TimezoneIE, IELen);
		NdisMoveMemory(pWNMCtrl->TimezoneIE, IE, IELen);
		pWNMCtrl->TimezoneIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Time Zone IE\n"));
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown IE(EID = %d)\n", __func__, EID));
		break;
	}

	return TRUE;
}
#endif

INT Set_AP_IE(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
	UINT8 EID;
	UINT8 OUIType;

	EID = *IE;

	switch (EID) {
#ifdef CONFIG_HOTSPOT

	case IE_INTERWORKING:
	case IE_ADVERTISEMENT_PROTO:
	case IE_QOS_MAP_SET:
	case IE_ROAMING_CONSORTIUM:
		Set_AP_HS_IE(pAd, EID, IE, IELen);
		break;
#endif
#ifdef CONFIG_DOT11V_WNM

	case IE_TIME_ADVERTISEMENT:
	case IE_TIME_ZONE:
		Set_AP_WNM_IE(pAd, EID, IE, IELen);
		break;
#endif

	case IE_VENDOR_SPECIFIC:
		OUIType = *(IE + 5);
		Set_AP_VENDOR_SPECIFIC_IE(pAd, OUIType, IE, IELen);
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown IE(EID = %d)\n", __func__, EID));
		break;
	}

	return TRUE;
}

#ifdef CONFIG_HOTSPOT
INT Send_ANQP_Rsp(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *ANQPRsp,
	IN UINT32 ANQPRspLen)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
	PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;
	GAS_EVENT_DATA *Event;
	GAS_PEER_ENTRY *GASPeerEntry;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag, *Tmp;
	UINT32 Len = 0, i, QueryRspOffset = 0;
	BOOLEAN Cancelled;
	BOOLEAN IsFound = FALSE;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	/* Cancel PostReply timer after receiving daemon response */
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, PeerMACAddr)) {
			if (GASPeerEntry->PostReplyTimerRunning) {
				RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
				GASPeerEntry->PostReplyTimerRunning = FALSE;
			}

			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + ANQPRspLen);

	if (!Buf) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
		goto error0;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + ANQPRspLen);
	Event = (GAS_EVENT_DATA *)Buf;
	Event->ControlIndex = apidx;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;

	if ((ANQPRspLen > pHSCtrl->MMPDUSize) ||
	    ((pGASCtrl->cb_delay != 0) && (pHSCtrl->bHSOnOff == 0))) {

		Event->EventType = GAS_RSP_MORE;
		Len += 2;
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, PeerMACAddr)) {
				IsFound = TRUE;
				Event->u.GAS_RSP_MORE_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;

				if ((ANQPRspLen % pHSCtrl->MMPDUSize) == 0)
					GASPeerEntry->GASRspFragNum = ANQPRspLen / pHSCtrl->MMPDUSize;
				else
					GASPeerEntry->GASRspFragNum = (ANQPRspLen / pHSCtrl->MMPDUSize) + 1;

				GASPeerEntry->CurrentGASFragNum = 0;

				for (i = 0; i < GASPeerEntry->GASRspFragNum; i++) {
					os_alloc_mem(NULL, (UCHAR **)&GASQueryRspFrag, sizeof(*GASQueryRspFrag));

					if (!GASQueryRspFrag) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
						RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
						goto error1;
					}

					GASPeerEntry->AllocResource++;
					NdisZeroMemory(GASQueryRspFrag, sizeof(*GASQueryRspFrag));
					GASQueryRspFrag->GASRspFragID = i;

					if (i < (GASPeerEntry->GASRspFragNum - 1))
						GASQueryRspFrag->FragQueryRspLen = pHSCtrl->MMPDUSize;
					else
						GASQueryRspFrag->FragQueryRspLen = ANQPRspLen - (pHSCtrl->MMPDUSize * i);

					os_alloc_mem(NULL, (UCHAR **)&GASQueryRspFrag->FragQueryRsp,
								 GASQueryRspFrag->FragQueryRspLen);
					GASPeerEntry->AllocResource++;

					if (!GASQueryRspFrag->FragQueryRsp) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
						RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
						goto error2;
					}

					NdisMoveMemory(GASQueryRspFrag->FragQueryRsp, &ANQPRsp[QueryRspOffset],
								   GASQueryRspFrag->FragQueryRspLen);
					QueryRspOffset += GASQueryRspFrag->FragQueryRspLen;
					DlListAddTail(&GASPeerEntry->GASQueryRspFragList,
								  &GASQueryRspFrag->List);
				}

				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		if (!IsFound) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Can not find peer address in GASPeerList\n", __func__));
			goto error1;
		}

		Event->u.GAS_RSP_MORE_DATA.StatusCode = 0;
		Len += 2;
		Event->u.GAS_RSP_MORE_DATA.GASComebackDelay = pGASCtrl->cb_delay;
		Len += 2;
		Event->u.GAS_RSP_MORE_DATA.AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL;
		Len += 1;
		GASPeerEntry->CurrentState = WAIT_GAS_RSP;
		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP_MORE, Len, Buf, 0);
	} else {
		Event->EventType = GAS_RSP;
		Len += 2;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, PeerMACAddr)) {
				IsFound = TRUE;
				printk("GAS RSP DialogToken = %x\n", GASPeerEntry->DialogToken);
				Event->u.GAS_RSP_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;
			}
			break;
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		if (!IsFound) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Can not find peer address in GASPeerList\n", __func__));
			goto error1;
		}

		Event->u.GAS_RSP_DATA.StatusCode = 0;
		Len += 2;
		Event->u.GAS_RSP_DATA.GASComebackDelay = 0;
		Len += 2;
		Event->u.GAS_RSP_DATA.AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL;
		Len += 1;
		Event->u.GAS_RSP_DATA.QueryRspLen = ANQPRspLen;
		Len += 2;

		NdisMoveMemory(Event->u.GAS_RSP_DATA.QueryRsp, ANQPRsp, ANQPRspLen);
		Len += ANQPRspLen;

		GASPeerEntry->CurrentState = WAIT_GAS_RSP;

		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP, Len, Buf, 0);
	}

	os_free_mem(Buf);
	return TRUE;
error2:
	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEachSafe(GASQueryRspFrag, Tmp, &GASPeerEntry->GASQueryRspFragList,
					  GAS_QUERY_RSP_FRAGMENT, List) {
		DlListDel(&GASQueryRspFrag->List);
		os_free_mem(GASQueryRspFrag);
	}
	DlListInit(&GASPeerEntry->GASQueryRspFragList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
error1:
	os_free_mem(Buf);
error0:
	return FALSE;
}

#endif

#ifdef CONFIG_DOT11V_WNM
INT Send_BTM_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *BTMReq,
	IN UINT32 BTMReqLen)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	BTM_EVENT_DATA *Event;
	BTM_PEER_ENTRY *BTMPeerEntry;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, PeerMACAddr)) {
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&BTMPeerEntry, sizeof(*BTMPeerEntry));

		if (!BTMPeerEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
			goto error0;
		}

		NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));
		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->ControlIndex = APIndex;
		NdisMoveMemory(BTMPeerEntry->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
		BTMPeerEntry->DialogToken = 1;
		BTMPeerEntry->Priv = pAd;
		RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMRspTimer,
					  GET_TIMER_FUNCTION(WaitPeerBTMRspTimeout), BTMPeerEntry, FALSE);
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + BTMReqLen);

	if (!Buf) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
		goto error1;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + BTMReqLen);

	Event = (BTM_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	Event->EventType = BTM_REQ;
	Len += 2;
	Event->u.BTM_REQ_DATA.DialogToken = BTMPeerEntry->DialogToken;
	Len += 1;
	Event->u.BTM_REQ_DATA.BTMReqLen = BTMReqLen;
	Len += 2;
	NdisMoveMemory(Event->u.BTM_REQ_DATA.BTMReq, BTMReq, BTMReqLen);
	Len += BTMReqLen;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("\nbefore adding BSS Transition Candidate List Entries::BTMReqLen=%d, Len=%d\n",
			  BTMReqLen, Len));
	MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_REQ, Len, Buf, 0);
	os_free_mem(Buf);
	return TRUE;
error1:

	if (!IsFound)
		os_free_mem(BTMPeerEntry);

error0:
	return FALSE;
}

#ifdef CONFIG_HOTSPOT_R2
INT Send_WNM_Notify_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *WNMNotifyReq,
	IN UINT32 WNMNotifyReqLen,
	IN UINT32 type)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	WNM_NOTIFY_EVENT_DATA *Event;
	WNM_NOTIFY_PEER_ENTRY *WNMNotifyPeerEntry;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, PeerMACAddr)) {
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&WNMNotifyPeerEntry, sizeof(*WNMNotifyPeerEntry));

		if (!WNMNotifyPeerEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
			goto error0;
		}

		NdisZeroMemory(WNMNotifyPeerEntry, sizeof(*WNMNotifyPeerEntry));
		WNMNotifyPeerEntry->CurrentState = WNM_NOTIFY_REQ;
		WNMNotifyPeerEntry->ControlIndex = APIndex;
		NdisMoveMemory(WNMNotifyPeerEntry->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
		WNMNotifyPeerEntry->DialogToken = 1;
		WNMNotifyPeerEntry->Priv = pAd;
		RTMPInitTimer(pAd, &WNMNotifyPeerEntry->WaitPeerWNMNotifyRspTimer,
					  GET_TIMER_FUNCTION(WaitPeerWNMNotifyRspTimeout), WNMNotifyPeerEntry, FALSE);
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->WNMNotifyPeerList, &WNMNotifyPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + WNMNotifyReqLen);

	if (!Buf) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
		goto error1;
	}

	/* RTMPusecDelay(200000); //200ms */
	NdisZeroMemory(Buf, sizeof(*Event) + WNMNotifyReqLen);
	Event = (WNM_NOTIFY_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	Event->EventType = type; /* WNM_NOTIFY_REQ; */
	Len += 2;
	Event->u.WNM_NOTIFY_REQ_DATA.DialogToken = WNMNotifyPeerEntry->DialogToken;
	Len += 1;
	Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen = WNMNotifyReqLen;
	Len += 2;
	NdisMoveMemory(Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReq, WNMNotifyReq, WNMNotifyReqLen);
	Len += WNMNotifyReqLen;
	MlmeEnqueue(pAd, WNM_NOTIFY_STATE_MACHINE, WNM_NOTIFY_REQ, Len, Buf, 0);
	os_free_mem(Buf);
	return TRUE;
error1:

	if (!IsFound)
		os_free_mem(WNMNotifyPeerEntry);

error0:
	return FALSE;
}

INT Send_QOSMAP_Configure(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *QosMapBuf,
	IN UINT32	QosMapLen,
	IN UINT8	Apidx)
{
	MLME_QOS_ACTION_STRUCT QosMapConfig;
	QOSMAP_SET *pQOSMap = &QosMapConfig.QOSMap;

	NdisZeroMemory(&QosMapConfig, sizeof(MLME_QOS_ACTION_STRUCT));
	COPY_MAC_ADDR(QosMapConfig.Addr, PeerMACAddr);
	QosMapConfig.ActionField = ACTION_QOSMAP_CONFIG;
	QosMapConfig.apidx = Apidx;
	pQOSMap->DSCP_Field_Len = QosMapLen;
	NdisMoveMemory(pQOSMap->DSCP_Field, QosMapBuf, QosMapLen);
	MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_QOS_CATE, sizeof(MLME_QOS_ACTION_STRUCT), (PVOID)&QosMapConfig, 0);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

/* for debug */
INT Set_CR4_Hotspot_Flag(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 Flag = os_str_tol(arg, 0, 10);
	PHOTSPOT_CTRL pHSCtrl;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	pHSCtrl->HotSpotEnable = IS_HOTSPOT_ENABLE(Flag);
	pHSCtrl->DGAFDisable = IS_DGAF_DISABLE(Flag);
	pHSCtrl->bASANEnable = IS_ASAN_ENABLE(Flag);
	pHSCtrl->QosMapEnable = IS_QOSMAP_ENABLE(Flag);
	pWNMCtrl->ProxyARPEnable = IS_PROXYARP_ENABLE(Flag);
	NdisZeroMemory(&HotspotInfoUpdateT, sizeof(HotspotInfoUpdateT));
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateBssCapability;
	HotspotInfoUpdateT.ucHotspotBssFlags = Flag;
	HotspotInfoUpdateT.ucHotspotBssId = APIndex;
	MtCmdHotspotInfoUpdate(pAd, HotspotInfoUpdateT);
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s ===> Update BSS:%d  HotspotFlag:0x%x\n"
			 , __func__, HotspotInfoUpdateT.ucHotspotBssId, HotspotInfoUpdateT.ucHotspotBssFlags));
	hotspot_bssflag_dump(Flag);
	return TRUE;
}

#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT Set_KtoFlash_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	pAd->KtoFlashDebug = (Enable > 0) ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Set_KtoFlash_Debug_Proc::(enable = %d)\n",
			  pAd->KtoFlashDebug));
	return TRUE;
}
INT Set_RDCE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT Type;
	INT BW;
	INT Band;
	INT i4Recv;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", &(Type), &(BW), &(Band));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Type %d,BW %d,Band %d\n",
				 __func__, Type, BW, Band));

		if (i4Recv != 3) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
			return FALSE;
		}

		MtCmdRDCE(pAd, Type, BW, Band);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
		return FALSE;
	}

	return TRUE;
}

#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef RLM_CAL_CACHE_SUPPORT
INT Set_RLM_Cal_Cache_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	rlmCalCacheStatus(pAd->rlmCalCache);
	rlmCalCacheDump(pAd->rlmCalCache);
	return TRUE;
}
#endif /* RLM_CAL_CACHE_SUPPORT */

INT Set_BWF_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	MtCmdSetBWFEnable(pAd, (UINT8)Enable);
	return TRUE;
}

#ifdef TX_AGG_ADJUST_WKR
INT Set_AggAdjWkr_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	pAd->TxAggAdjsut = os_str_tol(arg, 0, 16);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TxAggAdjsut = %u\n", pAd->TxAggAdjsut));
	return TRUE;
}
#endif /* TX_AGG_ADJUST_WKR */

#ifdef CONFIG_TX_DELAY
INT Set_TX_Batch_Cnt_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	tx_delay_ctl->tx_process_batch_cnt = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("TxProcessBatchCnt = %u\n", tx_delay_ctl->tx_process_batch_cnt));
	return TRUE;
}

INT Set_Pkt_Min_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	tx_delay_ctl->min_pkt_len = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("min_pkt_len = %u\n", tx_delay_ctl->min_pkt_len));
	return TRUE;
}

INT Set_Pkt_Max_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	tx_delay_ctl->max_pkt_len = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("min_pkt_len = %u\n", tx_delay_ctl->max_pkt_len));
	return TRUE;
}

INT Set_TX_Delay_Timeout_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	tx_delay_ctl->que_agg_timeout_value = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("que_agg_timeout_value = %u\n", tx_delay_ctl->que_agg_timeout_value));
	return TRUE;
}
#endif

#ifdef HTC_DECRYPT_IOT
INT Set_WTBL_AAD_OM_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	HW_SET_ASIC_WCID_AAD_OM(pAd, 1, Enable);
	return TRUE;
}


INT Set_HTC_Err_TH_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)

{
	UINT value;

	value = os_str_tol(arg, 0, 10);

	if (value)
		pAd->HTC_ICV_Err_TH = value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ pAd->HTC_ICV_Err_TH=%u\n", pAd->HTC_ICV_Err_TH));
	return TRUE;
}

INT Set_Entry_HTC_Err_Cnt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)

{
	UINT value;
	PMAC_TABLE_ENTRY pEntry;

	value = os_str_tol(arg, 0, 10);
	pEntry = &pAd->MacTab.Content[1];

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		ULONG Now32;

		NdisGetSystemUpTime(&Now32);
		pEntry->HTC_ICVErrCnt = value;
		/* Rx HTC and FAIL decryp case! */
		/* if (rxd_base->RxD2.IcvErr == 1) */
		{
			if (pEntry->HTC_ICVErrCnt++ > pAd->HTC_ICV_Err_TH) {
				pEntry->HTC_ICVErrCnt = 0; /* reset the history */

				if (pEntry->HTC_AAD_OM_Force == 0) {
					pEntry->HTC_AAD_OM_Force = 1;
					HW_SET_ASIC_WCID_AAD_OM(pAd, 1, 1);
				} else {
					pEntry->HTC_AAD_OM_Force = 0;
					HW_SET_ASIC_WCID_AAD_OM(pAd, 1, 0);
				}
			}
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ pEntry->HTC_ICVErrCnt=%u\n", pEntry->HTC_ICVErrCnt));
	}

	return TRUE;
}
#endif /* HTC_DECRYPT_IOT */

#ifdef DHCP_UC_SUPPORT
INT Set_DHCP_UC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT value;

	value = os_str_tol(arg, 0, 10);

	if (value)
		pAd->DhcpUcEnable = TRUE;
	else
		pAd->DhcpUcEnable = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ pAd->DhcpUcEnable=%u\n", pAd->DhcpUcEnable));
	return TRUE;
}
#endif /* DHCP_UC_SUPPORT */

#ifdef AIR_MONITOR

/* SMESH */
#define SMESH_RX_CTL							BIT(20)
#define SMESH_RX_CTL_OFFSET						20
#define SMESH_RX_MGT							BIT(19)
#define SMESH_RX_MGT_OFFSET						19
#define SMESH_RX_DATA							BIT(18)
#define SMESH_RX_DATA_OFFSET					18
#define SMESH_RX_A1								BIT(17)
#define SMESH_RX_A2								BIT(16)
#define SMESH_ADDR_EN				BITS(0, 7)

/* MAR1 */
#define MAR1_MAR_GROUP_MASK			BITS(30, 31)
#define MAR1_MAR_GROUP_OFFSET			30
#define MAR1_MAR_HASH_MODE_BSSID1		BIT(30)
#define MAR1_MAR_HASH_MODE_BSSID2		BIT(31)
#define MAR1_MAR_HASH_MODE_BSSID3		BITS(30, 31)
#define MAR1_ADDR_INDEX_MASK			BITS(24, 29)
#define MAR1_ADDR_INDEX_OFFSET			24
#define MAR1_READ				0
#define MAR1_WRITE				BIT(17)
#define MAR1_ACCESS_START_STATUS		BIT(16)

void apply_mntr_ruleset(PRTMP_ADAPTER pAd, UINT32 *pu4SMESH)
{
	if (pAd->MntRuleBitMap & RULE_CTL)
		*pu4SMESH |= SMESH_RX_CTL;
	else
		*pu4SMESH &= ~SMESH_RX_CTL;

	if (pAd->MntRuleBitMap & RULE_MGT)
		*pu4SMESH |= SMESH_RX_MGT;
	else
		*pu4SMESH &= ~SMESH_RX_MGT;

	if (pAd->MntRuleBitMap & RULE_DATA)
		*pu4SMESH |= SMESH_RX_DATA;
	else
		*pu4SMESH &= ~SMESH_RX_DATA;

	if (pAd->MntRuleBitMap & RULE_A1)
		*pu4SMESH |= SMESH_RX_A1;
	else
		*pu4SMESH &= ~SMESH_RX_A1;

	if (pAd->MntRuleBitMap & RULE_A2)
		*pu4SMESH |= SMESH_RX_A2;
	else
		*pu4SMESH &= ~SMESH_RX_A2;
}


INT Set_Enable_Air_Monitor_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	UCHAR mnt_enable = 0, band_idx, flush_all = FALSE;
	UINT32 i, u4SMESH = 0, u4MAR0 = 0, u4MAR1 = 0;
	UCHAR *p = ZERO_MAC_ADDR, muar_idx = 0;
	BOOLEAN bSMESHEn = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	mnt_enable = (UCHAR)simple_strtol(arg, 0, 10);
	band_idx = HcGetBandByWdev(wdev);

	if (band_idx == BAND0) {
		HW_IO_READ32(pAd, RMAC_SMESH, &u4SMESH);

		if ((u4SMESH & SMESH_ADDR_EN)) {
			bSMESHEn = TRUE;
			flush_all = TRUE;
		}
	} else if (band_idx == BAND1) {
		HW_IO_READ32(pAd, RMAC_SMESH_B1, &u4SMESH);

		if ((u4SMESH & SMESH_ADDR_EN)) {
			bSMESHEn = TRUE;
			flush_all = TRUE;
		}
	}

	if (bSMESHEn != mnt_enable) {
		if (mnt_enable == 0) {
			if (flush_all == TRUE) {
				for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
					pMuarGroup = &pAd->MntGroupTable[i];

					if (pMuarGroup->bValid && (pMuarGroup->Band == band_idx))
						NdisZeroMemory(pMuarGroup, sizeof(*pMuarGroup));
				}

				u4MAR0 = ((UINT32)p[0]) | ((UINT32)p[1]) << 8 |
						 ((UINT32)p[2]) << 16 | ((UINT32)p[3]) << 24;
				u4MAR1 = ((UINT32)p[4]) | ((UINT32)p[5]) << 8;

				for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
					pMntEntry = &pAd->MntTable[i];

					if (!pMntEntry->bValid || (pMntEntry->Band != band_idx))
						continue;

					muar_idx = pMntEntry->muar_idx;
					u4MAR1 |= MAR1_WRITE | MAR1_ACCESS_START_STATUS | (muar_idx << MAR1_ADDR_INDEX_OFFSET)
							  | (((UINT32)0 << MAR1_MAR_GROUP_OFFSET) & MAR1_MAR_GROUP_MASK);
					HW_IO_WRITE32(pAd, RMAC_MAR0, u4MAR0);
					HW_IO_WRITE32(pAd, RMAC_MAR1, u4MAR1);
					pMacEntry = pMntEntry->pMacEntry;

					if (pMacEntry) {
						if (band_idx == BAND1)
							pMacEntry->mnt_band &= ~MNT_BAND1;
						else
							pMacEntry->mnt_band &= ~MNT_BAND0;

						if (pMacEntry->mnt_band == 0) { /* no more use for other band */
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
									 ("%s::call MacTableDeleteEntry(WCID=%d)- %02X:%02X:%02X:%02X:%02X:%02X\n",
									  __func__, pMacEntry->wcid, PRINT_MAC(pMacEntry->Addr)));
							MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
						}
					}

					pAd->MonitrCnt[band_idx]--;
					NdisZeroMemory(pMntEntry, sizeof(*pMntEntry));
				}

				if (band_idx == DBDC_BAND0) {
					HW_IO_READ32(pAd, RMAC_SMESH, &u4SMESH);
					u4SMESH &= ~SMESH_ADDR_EN;
					HW_IO_WRITE32(pAd, RMAC_SMESH, u4SMESH);
				} else if (band_idx == DBDC_BAND1) {
					HW_IO_READ32(pAd, RMAC_SMESH_B1, &u4SMESH);
					u4SMESH &= ~SMESH_ADDR_EN;
					HW_IO_WRITE32(pAd, RMAC_SMESH_B1, u4SMESH);
				} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():: wrong band index(%d)\n",
							 __func__, band_idx));
					return FALSE;
				}
			}

			if ((pAd->MonitrCnt[BAND0] + pAd->MonitrCnt[BAND1]) == 0) {
				pAd->MntEnable = 0;
				NdisZeroMemory(&pAd->MntTable, sizeof(pAd->MntTable));
				NdisZeroMemory(&pAd->MntGroupTable, sizeof(pAd->MntGroupTable));
			}
		} else {
			if (band_idx == DBDC_BAND0) {
				HW_IO_READ32(pAd, RMAC_SMESH, &u4SMESH);
				apply_mntr_ruleset(pAd, &u4SMESH);
				u4SMESH |= SMESH_ADDR_EN | u4SMESH;
				HW_IO_WRITE32(pAd, RMAC_SMESH, u4SMESH);
			} else if (band_idx == DBDC_BAND1) {
				HW_IO_READ32(pAd, RMAC_SMESH_B1, &u4SMESH);
				apply_mntr_ruleset(pAd, &u4SMESH);
				u4SMESH |= SMESH_ADDR_EN | u4SMESH;
				HW_IO_WRITE32(pAd, RMAC_SMESH_B1, u4SMESH);
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():: wrong band index(%d)\n",
						 __func__, band_idx));
				return FALSE;
			}

			pAd->MntEnable = 1;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return TRUE;
}


INT	Set_MonitorRule_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	INT ret = TRUE;
	RTMP_STRING *this_char = NULL, *value = NULL;
	INT idx = 0;
	UCHAR rx_rule[3], band_idx;
	UINT32 u4SMESH = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}

		if (strlen(this_char) != 5) { /* the acceptable format is like 0:1:1 with length 5 */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("illegal length! (acceptable format 0:1:1 length 5)\n"));
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 1) || (!isxdigit(*value))) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal format!\n"));
				break;
			}

			rx_rule[idx++] = (UCHAR)os_str_tol(value, 0, 10);
		}

		if (idx != 3)
			continue;
	}

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pAd->MntRuleBitMap &= ~RULE_CTL;
	pAd->MntRuleBitMap &= ~RULE_MGT;
	pAd->MntRuleBitMap &= ~RULE_DATA;
	pAd->MntRuleBitMap |= (RULE_DATA & ((UINT32)rx_rule[0] << RULE_DATA_OFFSET))
					| (RULE_MGT & ((UINT32)rx_rule[1] << RULE_MGT_OFFSET))
					| (RULE_CTL & ((UINT32)rx_rule[2] << RULE_CTL_OFFSET));

	band_idx = HcGetBandByWdev(wdev);

	if (band_idx == DBDC_BAND0) {
		HW_IO_READ32(pAd, RMAC_SMESH, &u4SMESH);
		u4SMESH &= ~SMESH_RX_DATA;
		u4SMESH &= ~SMESH_RX_MGT;
		u4SMESH &= ~SMESH_RX_CTL;
		u4SMESH |= (SMESH_RX_DATA & ((UINT32)rx_rule[0] << SMESH_RX_DATA_OFFSET))
				   | (SMESH_RX_MGT & ((UINT32)rx_rule[1] << SMESH_RX_MGT_OFFSET))
				   | (SMESH_RX_CTL & ((UINT32)rx_rule[2] << SMESH_RX_CTL_OFFSET));
		HW_IO_WRITE32(pAd, RMAC_SMESH, u4SMESH);
	} else if (band_idx == DBDC_BAND1) {
		HW_IO_READ32(pAd, RMAC_SMESH_B1, &u4SMESH);
		u4SMESH &= ~SMESH_RX_DATA;
		u4SMESH &= ~SMESH_RX_MGT;
		u4SMESH &= ~SMESH_RX_CTL;
		u4SMESH |= (SMESH_RX_DATA & ((UINT32)rx_rule[0] << SMESH_RX_DATA_OFFSET))
				   | (SMESH_RX_MGT & ((UINT32)rx_rule[1] << SMESH_RX_MGT_OFFSET))
				   | (SMESH_RX_CTL & ((UINT32)rx_rule[2] << SMESH_RX_CTL_OFFSET));
		HW_IO_WRITE32(pAd, RMAC_SMESH_B1, u4SMESH);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():: wrong band index(%d)\n",
				 __func__, band_idx));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}


INT	Set_MonitorTarget_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE;
	RTMP_STRING *this_char = NULL;
	RTMP_STRING *value = NULL;
	INT idx = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}

		if (strlen(this_char) != 17) { /* the acceptable format of MAC address is like 01:02:03:04:05:06 with length 17 */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n"));
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal MAC address format or octet!\n"));
				break;
			}

			AtoH(value, &pAd->curMntAddr[idx++], 1);
		}

		if (idx != MAC_ADDR_LEN)
			continue;
	}

	for (idx = 0; idx < MAC_ADDR_LEN; idx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X ", pAd->curMntAddr[idx]));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}


INT Set_MonitorIndex_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE, i;
	UINT32 u4MAR0 = 0, u4MAR1 = 0, u4DBDC_CTRL = 0;
	UCHAR *p = ZERO_MAC_ADDR, mnt_idx = 0, band_idx = 0, muar_idx = 0, muar_group_base = 0;
	BOOLEAN bCreate = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);
	mnt_idx = (UCHAR)simple_strtol(arg, 0, 10);

	if (mnt_idx < MAX_NUM_OF_MONITOR_STA) {
		pAd->MntIdx = mnt_idx;
		pMntEntry = &pAd->MntTable[mnt_idx];
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("The index is over the maximum limit.\n"));
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(ZERO_MAC_ADDR, pAd->curMntAddr)) {
		if (pMntEntry->bValid) {
			pMacEntry = pMntEntry->pMacEntry;

			if (pMacEntry) {
				if (band_idx == BAND1)
					pMacEntry->mnt_band &= ~MNT_BAND1;
				else
					pMacEntry->mnt_band &= ~MNT_BAND0;

				if (pMacEntry->mnt_band == 0) /* no more use for other band */
					MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
			}

			if (pMntEntry->muar_group_idx < MAX_NUM_OF_MONITOR_GROUP)
				pMuarGroup = &pAd->MntGroupTable[pMntEntry->muar_group_idx];

			if (pMuarGroup) {
				pMuarGroup->Count--;

				if (pMuarGroup->Count == 0)
					pMuarGroup->bValid = FALSE;
			}

			pMntEntry->bValid = FALSE;

			if (pAd->MonitrCnt[band_idx] > 0)
				pAd->MonitrCnt[band_idx]--;

			muar_idx = pMntEntry->muar_idx;
		} else
			return TRUE;
	} else {
		if (pMntEntry->bValid) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("The index of existed monitor entry.\n"));
			return TRUE;
		}

		if ((pAd->MonitrCnt[BAND0] + pAd->MonitrCnt[BAND1]) >= MAX_NUM_OF_MONITOR_STA) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("The monitor number extends to maximum limit(%d).\n", MAX_NUM_OF_MONITOR_STA));
			return FALSE;
		}

		for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
			pMuarGroup = &pAd->MntGroupTable[i];

			if (!pMuarGroup->bValid) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMuarGroup->MuarGroupBase = MONITOR_MUAR_BASE_INDEX + i * MAX_NUM_PER_GROUP;
				pMuarGroup->bValid = TRUE;
				pMuarGroup->Band = band_idx;
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + pMuarGroup->Count++);
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			} else if ((pMuarGroup->Count < MAX_NUM_PER_GROUP) &&
					   (pMuarGroup->Band == band_idx)) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + pMuarGroup->Count++);
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			}
		}

		if (bCreate) {
			COPY_MAC_ADDR(pMntEntry->addr, pAd->curMntAddr);
			pMacEntry = MacTableLookup(pAd, pMntEntry->addr);
			if (pMacEntry == NULL) {
				pMacEntry = MacTableInsertEntry(
								pAd,
								pMntEntry->addr,
								wdev,
								ENTRY_CAT_MONITOR,
								OPMODE_STA,
								TRUE);

				if (wdev_do_conn_act(wdev, pMacEntry) != TRUE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): connect fail!!\n", __func__));
					return FALSE;
				}
			}

			if (pMacEntry) {
				p = pMntEntry->addr;
				pAd->MonitrCnt[band_idx]++;
				pMntEntry->bValid = TRUE;
				pMntEntry->Band = band_idx;
				pMacEntry->mnt_idx[band_idx] = mnt_idx;

				if (band_idx == BAND1)
					pMacEntry->mnt_band |= MNT_BAND1;
				else
					pMacEntry->mnt_band |= MNT_BAND0;

				pMntEntry->pMacEntry = pMacEntry;
			}

			HW_IO_READ32(pAd, CFG_DBDC_CTRL1, &u4DBDC_CTRL);

			if (band_idx == BAND1)
				u4DBDC_CTRL |= (1 << (muar_group_base / MAX_NUM_PER_GROUP));
			else
				u4DBDC_CTRL &= ~(1 << (muar_group_base / MAX_NUM_PER_GROUP));

			HW_IO_WRITE32(pAd, CFG_DBDC_CTRL1, u4DBDC_CTRL);
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Can't create a monitor entry!\n"));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("index: %d\n", mnt_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("entry: %02X:%02X:%02X:%02X:%02X:%02X\n", PRINT_MAC(p)));
	u4MAR0 = ((UINT32)p[0]) | ((UINT32)p[1]) << 8 |
			 ((UINT32)p[2]) << 16 | ((UINT32)p[3]) << 24;
	u4MAR1 = ((UINT32)p[4]) | ((UINT32)p[5]) << 8;
	u4MAR1 |= MAR1_WRITE | MAR1_ACCESS_START_STATUS | (muar_idx << MAR1_ADDR_INDEX_OFFSET)
			  | (((UINT32)0 << MAR1_MAR_GROUP_OFFSET) & MAR1_MAR_GROUP_MASK);
	HW_IO_WRITE32(pAd, RMAC_MAR0, u4MAR0);
	HW_IO_WRITE32(pAd, RMAC_MAR1, u4MAR1);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("WriteCR 0x%08X:0x%08X 0x%08X:0x%08X\n", RMAC_MAR0, u4MAR0, RMAC_MAR1, u4MAR1));
	ret = Set_Enable_Air_Monitor_Proc(pAd, (pAd->MonitrCnt[band_idx] > 0) ? "1" : "0");

	do {
		HW_IO_READ32(pAd, RMAC_MAR1, &u4MAR1);
	} while (u4MAR1 & MAR1_ACCESS_START_STATUS);

	HW_IO_READ32(pAd, RMAC_MAR0, &u4MAR0);
	p[0] = u4MAR0 & 0xFF;
	p[1] = (u4MAR0 & 0xFF00) >> 8;
	p[2] = (u4MAR0 & 0xFF0000) >> 16;
	p[3] = (u4MAR0 & 0xFF000000) >> 24;
	p[4] = u4MAR1 & 0xFF;
	p[5] = (u4MAR1 & 0xFF00) >> 8;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("entry: %02X:%02X:%02X:%02X:%02X:%02X\n", PRINT_MAC(p)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}


INT	Set_MonitorShowAll_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE;
	UCHAR i = 0;
	MNT_STA_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Monitor Enable: %d\n", pAd->MntEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Index last set: %d\n", pAd->MntIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  BAND0 Count: %d\n", pAd->MonitrCnt[BAND0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  BAND1 Count: %d\n", pAd->MonitrCnt[BAND1]));

	for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
		pEntry = &pAd->MntTable[i];

		if (pEntry->bValid) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band%d: Monitor STA[%d]\t", pEntry->Band, i));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X\t", PRINT_MAC(pEntry->addr)));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("[DATA]=%08lu\t", pEntry->data_cnt));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("[MGMT]=%08lu\t", pEntry->mgmt_cnt));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("[CNTL]=%08lu\t", pEntry->cntl_cnt));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("[TOTAL]=%08lu\t", pEntry->Count));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RSSI:%d,%d,%d,%d\n",
					 pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
					 pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]));
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}


INT	Set_MonitorClearCounter_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE;
	UCHAR i = 0;
	MNT_STA_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
		pEntry = pAd->MntTable + i;
		pEntry->data_cnt = 0;
		pEntry->mgmt_cnt = 0;
		pEntry->cntl_cnt = 0;
		pEntry->Count = 0;
		NdisZeroMemory(&pEntry->RssiSample, sizeof(RSSI_SAMPLE));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}


INT	Set_Enable_MonitorTarget_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg,
	IN USHORT	index)
{
	INT ret = TRUE;
	CHAR str[8];

	ret = Set_MonitorTarget_Proc(pAd, arg);

	if (!ret)
		return ret;

	if (index < MAX_NUM_OF_MONITOR_STA) {
		sprintf(str, "%u", index);
		ret = Set_MonitorIndex_Proc(pAd, str);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("The index is over the maximum limit.\n"));
		ret = FALSE;
	}

	return ret;
}


INT	Set_MonitorTarget0_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 0);
}


INT	Set_MonitorTarget1_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 1);
}


INT	Set_MonitorTarget2_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 2);
}


INT	Set_MonitorTarget3_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 3);
}


INT	Set_MonitorTarget4_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 4);
}


INT	Set_MonitorTarget5_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 5);
}


INT	Set_MonitorTarget6_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 6);
}


INT	Set_MonitorTarget7_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 7);
}


INT	Set_MonitorTarget8_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 8);
}


INT	Set_MonitorTarget9_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 9);
}


INT	Set_MonitorTarget10_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 10);
}


INT	Set_MonitorTarget11_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 11);
}


INT	Set_MonitorTarget12_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 12);
}


INT	Set_MonitorTarget13_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 13);
}


INT	Set_MonitorTarget14_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 14);
}


INT	Set_MonitorTarget15_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 15);
}


VOID Air_Monitor_Pkt_Report_Action(
	IN RTMP_ADAPTER	*pAd,
	IN UCHAR	wcid,
	IN RX_BLK	*pRxBlk)
{
	UCHAR FrameBuf[512];
	AIR_RAW AirRaw;
	HTTRANSMIT_SETTING HTSetting;
	UCHAR Apidx = MAIN_MBSSID, BandIdx = 0, Channel = 0;
	UCHAR s_addr[MAC_ADDR_LEN];
	UCHAR ETH_P_AIR_MONITOR[LENGTH_802_3_TYPE] = {0x51, 0xA0};
	UINT32 frame_len = 0, offset = 0, i;
	struct sk_buff *skb = NULL;
	FRAME_CONTROL *fc = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[wcid];
	MNT_STA_ENTRY *pMntEntry = NULL;
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;

	if (!rxd_base)
		return;

	Channel = rxd_base->RxD1.ChFreq;

	if (Channel == 0)
		return;

	BandIdx = HcGetBandByChannel(pAd, Channel);

	if (BandIdx >= BAND_NUM)
		return;

	pMntEntry = pAd->MntTable + pMacEntry->mnt_idx[BandIdx];

	if (!pMntEntry->bValid)
		return;

	switch (fc->Type) {
	case FC_TYPE_MGMT:
		if (pAd->MntRuleBitMap & RULE_MGT)
				pMntEntry->mgmt_cnt++;
			else
				goto done;
			break;

	case FC_TYPE_CNTL:
		if (pAd->MntRuleBitMap & RULE_CTL)
				pMntEntry->cntl_cnt++;
			else
				goto done;
			break;

	case FC_TYPE_DATA:
		if (pAd->MntRuleBitMap & RULE_DATA)
				pMntEntry->data_cnt++;
			else
				goto done;
			break;

	default:
		goto done;
	}

	pMntEntry->Count++;
	NdisMoveMemory(pMntEntry->RssiSample.AvgRssi,
				   pRxBlk->rx_signal.raw_rssi, sizeof(pMntEntry->RssiSample.AvgRssi));

	for (i = 0; i < pAd->Antenna.field.RxPath; i++) {
		if (pMntEntry->RssiSample.AvgRssi[i] > 0)
			pMntEntry->RssiSample.AvgRssi[i] = -127;
	}

	/* Init frame buffer */
	NdisZeroMemory(FrameBuf, sizeof(FrameBuf));
	NdisZeroMemory(&AirRaw, sizeof(AirRaw));
	/* Fake a Source Address for transmission */
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[Apidx].wdev.if_addr);

	if (s_addr[1] == 0xff)
		s_addr[1] = 0;
	else
		s_addr[1]++;

	/* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[Apidx].wdev.if_addr, s_addr, ETH_P_AIR_MONITOR);
	offset += LENGTH_802_3;
	/* For Rate Info */
	HTSetting.field.MODE    = pRxBlk->rx_rate.field.MODE;
	HTSetting.field.MCS     = pRxBlk->rx_rate.field.MCS;
	HTSetting.field.BW      = pRxBlk->rx_rate.field.BW;
	HTSetting.field.ShortGI = pRxBlk->rx_rate.field.ShortGI;
	getRate(HTSetting, &AirRaw.wlan_radio_tap.RATE);
	AirRaw.wlan_radio_tap.PHYMODE = pRxBlk->rx_rate.field.MODE;
	AirRaw.wlan_radio_tap.MCS     = pRxBlk->rx_rate.field.MCS;
	AirRaw.wlan_radio_tap.BW      = pRxBlk->rx_rate.field.BW;
	AirRaw.wlan_radio_tap.ShortGI = pRxBlk->rx_rate.field.ShortGI;
#ifdef DOT11_VHT_AC

	if (AirRaw.wlan_radio_tap.PHYMODE >= MODE_VHT) {
		AirRaw.wlan_radio_tap.MCS = (pRxBlk->rx_rate.field.MCS & 0xf);
		AirRaw.wlan_radio_tap.STREAM  = (pRxBlk->rx_rate.field.MCS >> 4) + 1;
	} else
#endif /* DOT11_VHT_AC */
		if (AirRaw.wlan_radio_tap.PHYMODE == MODE_OFDM) {
			AirRaw.wlan_radio_tap.MCS = getLegacyOFDMMCSIndex(pRxBlk->rx_rate.field.MCS);
			AirRaw.wlan_radio_tap.STREAM  = (pRxBlk->rx_rate.field.MCS >> 4) + 1;
		} else {
			AirRaw.wlan_radio_tap.MCS = (pRxBlk->rx_rate.field.MCS % 8);
			AirRaw.wlan_radio_tap.STREAM  = (pRxBlk->rx_rate.field.MCS >> 3) + 1;
		}

	/* For RSSI */
	for (i = 0; i < pAd->Antenna.field.RxPath; i++)
		AirRaw.wlan_radio_tap.RSSI[i] = pMntEntry->RssiSample.AvgRssi[i];

	AirRaw.wlan_radio_tap.Channel = Channel;

	/* For 802.11 Header */
	NdisMoveMemory(&AirRaw.wlan_header.FC, fc, sizeof(*fc));
	AirRaw.wlan_header.Duration = pRxBlk->Duration;
	AirRaw.wlan_header.SN = pRxBlk->SN;
	AirRaw.wlan_header.FN = pRxBlk->FN;
	COPY_MAC_ADDR(AirRaw.wlan_header.Addr1, pRxBlk->Addr1);
	COPY_MAC_ADDR(AirRaw.wlan_header.Addr2, pRxBlk->Addr2);
	COPY_MAC_ADDR(AirRaw.wlan_header.Addr3, pRxBlk->Addr3);

	if (fc->ToDs == 1 && fc->FrDs == 1)
		COPY_MAC_ADDR(AirRaw.wlan_header.Addr4, pRxBlk->Addr4);

	/* Prepare payload*/
	NdisCopyMemory(&FrameBuf[offset], (CHAR *)&AirRaw, sizeof(AirRaw));
	offset += sizeof(AirRaw);
	frame_len = offset;
	/* Create skb */
	skb = dev_alloc_skb((frame_len + 2));

	if (!skb) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s : Error! Can't allocate a skb.\n", __func__));
		return;
	}

	SET_OS_PKT_NETDEV(skb, pAd->ApCfg.MBSSID[Apidx].wdev.if_dev);
	/* 16 byte align the IP header */
	skb_reserve(skb, 2);
	/* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), FrameBuf, frame_len);
	/* End this frame */
	skb_put(skb, frame_len);
	/* Report to upper layer */
	RtmpOsPktProtocolAssign(skb);
	RtmpOsPktRcvHandle(skb);
done:
	return;
}

#endif /* AIR_MONITOR */


#ifdef DSCP_PRI_SUPPORT
INT	Set_Dscp_Pri_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT8	map_index,
	IN	RTMP_STRING *arg)
{
	RTMP_STRING *this_char;
	UINT8	dscpValue;
	INT8 pri;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s arg=%s\n", __func__, arg));
	this_char = strsep((char **)&arg, ";");
	if (this_char == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s value not defined for Dscp and Priority\n", __func__));
		return FALSE;
	}

	dscpValue = simple_strtol(this_char, 0, 10);
	if ((dscpValue < 0) || (dscpValue > 63)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s Invalid Dscp Value Valid Value between 0 to 63\n",
					__func__));
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s Priority not defined for Dscp %d\n", __func__, dscpValue));
		return FALSE;
	}
	pri = simple_strtol(arg, 0, 10);

	if (pri < -1  || pri > 7) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s Invalid Priority value Valid value between 0 to 7\n", __func__));
		return FALSE;
	}

	if (pri == 0)
		pri = 3;
	pAd->dscp_pri_map[map_index][dscpValue] = pri;

	return TRUE;
}
INT	Set_Dscp_Pri_2G_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	return Set_Dscp_Pri_Proc(pAd, DSCP_PRI_2G_MAP, arg);
}
INT	Set_Dscp_Pri_5G_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	return Set_Dscp_Pri_Proc(pAd, DSCP_PRI_5G_MAP, arg);
}
#endif
