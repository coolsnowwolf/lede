/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: _ieee80211.h 2749 2007-10-16 08:58:14Z kelmo $
 */
#ifndef _NET80211__IEEE80211_H_
#define _NET80211__IEEE80211_H_

#include <compat.h>
#ifdef __KERNEL__
#include <linux/in6.h>
#endif

enum ieee80211_phytype {
	IEEE80211_T_DS,			/* direct sequence spread spectrum */
	IEEE80211_T_FH,			/* frequency hopping */
	IEEE80211_T_OFDM,		/* frequency division multiplexing */
	IEEE80211_T_TURBO,		/* high rate OFDM, aka turbo mode */
	IEEE80211_T_HT,			/* HT - full GI */
	IEEE80211_T_MAX
};
#define	IEEE80211_T_CCK	IEEE80211_T_DS	/* more common nomenclature */

/*
 * XXX not really a mode; there are really multiple PHY's
 * Please update ieee80211_chanflags when the definition of
 * ieee80211_phymode changed
 */
enum ieee80211_phymode {
	IEEE80211_MODE_AUTO	= 0,	/* autoselect */
	IEEE80211_MODE_11A	= 1,	/* 5GHz, OFDM */
	IEEE80211_MODE_11B	= 2,	/* 2GHz, CCK */
	IEEE80211_MODE_11G	= 3,	/* 2GHz, OFDM */
	IEEE80211_MODE_FH	= 4,	/* 2GHz, GFSK */
	IEEE80211_MODE_TURBO_A	= 5,	/* 5GHz, OFDM, 2x clock dynamic turbo */
	IEEE80211_MODE_TURBO_G	= 6,	/* 2GHz, OFDM, 2x clock  dynamic turbo*/
	IEEE80211_MODE_11NA		= 7,	/* 5GHz, HT20 */
	IEEE80211_MODE_11NG		= 8,	/* 2GHz, HT20 */
	IEEE80211_MODE_11NG_HT40PM	= 9,	/* 2GHz HT40 */
	IEEE80211_MODE_11NA_HT40PM	= 10,	/* 5GHz HT40 */
	IEEE80211_MODE_11AC_VHT20PM	= 11,	/* 5GHz VHT20 */
	IEEE80211_MODE_11AC_VHT40PM	= 12,	/* 5GHz VHT40 */
	IEEE80211_MODE_11AC_VHT80PM	= 13,	/* 5GHz VHT80 */
	IEEE80211_MODE_11AC_VHT160PM	= 14,	/* 5GHz VHT160 */
	IEEE80211_MODE_MAX		= 15,	/* Always keep this last */
};

/*
#define IEEE80211_BM_11A	(1 << IEEE80211_MODE_11A)
#define IEEE80211_BM_11B	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11G	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11NA	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11NG	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11NG_PLUS	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11NG_MINUS	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11NA_PLUS	(1 << IEEE80211_MODE_11B)
#define IEEE80211_BM_11NA_MINUS	(1 << IEEE80211_MODE_11B)
*/


#define IEEE80211_IS_VHT_20(_c) \
	((_c)->ic_phymode == IEEE80211_MODE_11AC_VHT20PM)

#define IEEE80211_IS_VHT_40(_c) \
	((_c)->ic_phymode == IEEE80211_MODE_11AC_VHT40PM)

#define IEEE80211_IS_VHT_80(_c) \
	((_c)->ic_phymode == IEEE80211_MODE_11AC_VHT80PM)

#define IEEE80211_IS_VHT_160(_c) \
	((_c)->ic_phymode == IEEE80211_MODE_11AC_VHT160PM)

#define IS_IEEE80211_VHT_ENABLED(_c) \
	(IEEE80211_IS_VHT_160(_c) || IEEE80211_IS_VHT_80(_c)\
	|| IEEE80211_IS_VHT_40(_c) || IEEE80211_IS_VHT_20(_c))

enum ieee80211_opmode {
	IEEE80211_M_STA		= 1,	/* infrastructure station */
	IEEE80211_M_IBSS	= 0,	/* IBSS (adhoc) station */
	IEEE80211_M_AHDEMO	= 3,	/* Old lucent compatible adhoc demo */
	IEEE80211_M_HOSTAP	= 6,	/* Software Access Point */
	IEEE80211_M_MONITOR	= 8,	/* Monitor mode */
	IEEE80211_M_WDS		= 2	/* WDS link */
};

/*
 * True if this mode must behave like a DFS master, ie do Channel
 * Check Availability and In Service Monitoring. We need to make sure
 * that all modes cannot send data without being authorized. Such
 * enforcement is not done in monitor mode however.
 */

#define IEEE80211_IS_MODE_DFS_MASTER(_opmode) \
	((_opmode == IEEE80211_M_IBSS) || \
	(_opmode == IEEE80211_M_AHDEMO) || \
	(_opmode == IEEE80211_M_HOSTAP) || \
	(_opmode == IEEE80211_M_WDS))

/*
 * 802.11n
 */

enum ieee80211_11n_htmode {
	IEEE80211_11N_HTAUTO	=  0,
	IEEE80211_11N_HT20	=  1,
	IEEE80211_11N_HT40PLUS	=  2,
	IEEE80211_11N_HT40MINUS	=  3
};

enum ieee80211_cwm_mode {
	IEEE80211_CWM_MODE20,
	IEEE80211_CWM_MODE2040,
	IEEE80211_CWM_MODE40,
	IEEE80211_CWM_MODEMAX

};

enum ieee80211_cwm_extprotspacing {
	IEEE80211_CWM_EXTPROTSPACING20,
	IEEE80211_CWM_EXTPROTSPACING25,
	IEEE80211_CWM_EXTPROTSPACINGMAX
};

enum ieee80211_cwm_width {
	IEEE80211_CWM_WIDTH20,
	IEEE80211_CWM_WIDTH40,
	IEEE80211_CWM_WIDTH80,
	IEEE80211_CWM_WIDTH160,	/* or 80+80 Mhz */
};

enum ieee80211_cwm_extprotmode {
	IEEE80211_CWM_EXTPROTNONE,	/* no protection */
	IEEE80211_CWM_EXTPROTCTSONLY,	/* CTS to self */
	IEEE80211_CWM_EXTPROTRTSCTS,	/* RTS-CTS */
	IEEE80211_CWM_EXTPROTMAX
};

/* CWM (Channel Width Management) Information */
struct ieee80211_cwm {

	/* Configuration */
	enum ieee80211_cwm_mode		cw_mode;		/* CWM mode */
	int8_t				cw_extoffset;		/* CWM Extension Channel Offset */
	enum ieee80211_cwm_extprotmode	cw_extprotmode;		/* CWM Extension Channel Protection Mode */
	enum ieee80211_cwm_extprotspacing cw_extprotspacing;	/* CWM Extension Channel Protection Spacing */

	/* State */
	 enum ieee80211_cwm_width	cw_width;		/* CWM channel width */
};


enum ieee80211_fixed_rate_status {
	DISABLED = 0,
	ENABLED = 1,	/* ieee rate */
};

/* Holds the fixed rate information for each VAP */
#define IEEE80211_MAX_FIXED_RATES		4
struct ieee80211_fixed_rate {
	enum ieee80211_fixed_rate_status	status;
	u_int8_t				rate[IEEE80211_MAX_FIXED_RATES];
	u_int32_t				retries;
	u_int32_t				flag;
#define IEEE80211_FIXED_RATE_F_11AC		0x1
};

/*
 * 802.11g protection mode.
 */
enum ieee80211_protmode {
	IEEE80211_PROT_NONE	= 0,	/* no protection */
	IEEE80211_PROT_CTSONLY	= 1,	/* CTS to self */
	IEEE80211_PROT_RTSCTS	= 2,	/* RTS-CTS */
};

/*
 * Authentication mode.
 */
enum ieee80211_authmode {
	IEEE80211_AUTH_NONE	= 0,
	IEEE80211_AUTH_OPEN	= 1,	/* open */
	IEEE80211_AUTH_SHARED	= 2,	/* shared-key */
	IEEE80211_AUTH_8021X	= 3,	/* 802.1x */
	IEEE80211_AUTH_AUTO	= 4,	/* auto-select/accept */
	/* NB: these are used only for ioctls */
	IEEE80211_AUTH_WPA	= 5,	/* WPA/RSN w/ 802.1x/PSK */
};

/*
 * Roaming mode is effectively who controls the operation
 * of the 802.11 state machine when operating as a station.
 * State transitions are controlled either by the driver
 * (typically when management frames are processed by the
 * hardware/firmware), the host (auto/normal operation of
 * the 802.11 layer), or explicitly through ioctl requests
 * when applications like wpa_supplicant want control.
 */
enum ieee80211_roamingmode {
	IEEE80211_ROAMING_DEVICE= 0,	/* driver/hardware control */
	IEEE80211_ROAMING_AUTO	= 1,	/* 802.11 layer control */
	IEEE80211_ROAMING_MANUAL= 2,	/* application control */
};

/*
 * Scanning mode controls station scanning work; this is
 * used only when roaming mode permits the host to select
 * the bss to join/channel to use.
 */
enum ieee80211_scanmode {
	IEEE80211_SCAN_DEVICE	= 0,	/* driver/hardware control */
	IEEE80211_SCAN_BEST	= 1,	/* 802.11 layer selects best */
	IEEE80211_SCAN_FIRST	= 2,	/* take first suitable candidate */
};

/* ba state describes the block ack state. block ack should only be sent if
 * ba state is set to established
 */
enum ieee80211_ba_state	{
	IEEE80211_BA_NOT_ESTABLISHED = 0,
	IEEE80211_BA_ESTABLISHED = 1,
	IEEE80211_BA_REQUESTED = 2,
	IEEE80211_BA_FAILED = 5,
	IEEE80211_BA_BLOCKED = 6,
};

#define IEEE80211_BA_IS_COMPLETE(_state) (\
	(_state) == IEEE80211_BA_ESTABLISHED || \
	(_state) == IEEE80211_BA_BLOCKED || \
	(_state) == IEEE80211_BA_FAILED) \

/* ba type describes the block acknowledgement type */
enum ieee80211_ba_type {
	IEEE80211_BA_DELAYED = 0,
	IEEE80211_BA_IMMEDIATE = 1,
};

#define IEEE80211_OPER_CLASS_MAX	256
#define	IEEE80211_OPER_CLASS_BYTES	32
#define	IEEE80211_OPER_CLASS_BYTES_24G	8

/* power index definition */
enum ieee80211_power_index_beamforming {
	PWR_IDX_BF_OFF = 0,
	PWR_IDX_BF_ON = 1,
	PWR_IDX_BF_MAX = 2
};

enum ieee80211_power_index_spatial_stream {
	PWR_IDX_1SS = 0,
	PWR_IDX_2SS = 1,
	PWR_IDX_3SS = 2,
	PWR_IDX_4SS = 3,
	PWR_IDX_SS_MAX = 4
};

enum ieee80211_power_index_bandwidth {
	PWR_IDX_20M = 0,
	PWR_IDX_40M = 1,
	PWR_IDX_80M = 2,
	PWR_IDX_BW_MAX = 3
};

/*
 * Channels are specified by frequency and attributes.
 */
struct ieee80211_channel {
	u_int16_t ic_freq;	/* setting in Mhz */
	u_int32_t ic_flags;	/* see below */
	u_int8_t ic_ieee;	/* IEEE channel number */
	int8_t ic_maxregpower;	/* maximum regulatory tx power in dBm */
	int8_t ic_maxpower;	/* maximum tx power in dBm for the current bandwidth with beam-forming off */
	int8_t ic_minpower;	/* minimum tx power in dBm */
	int8_t ic_maxpower_normal;	/* backup max tx power for short-range workaround */
	int8_t ic_minpower_normal;	/* backup min tx power for short-range workaround */
	int8_t ic_maxpower_table[PWR_IDX_BF_MAX][PWR_IDX_SS_MAX][PWR_IDX_BW_MAX];	/* the maximum powers for different cases */
	u_int32_t ic_radardetected; /* number that radar signal has been detected on this channel */
	u_int8_t ic_center_f_40MHz;
	u_int8_t ic_center_f_80MHz;
	u_int8_t ic_center_f_160MHz;
	u_int32_t ic_ext_flags;
};

#define	IEEE80211_CHAN_MAX	255
#define	IEEE80211_CHAN_BYTES	32	/* howmany(IEEE80211_CHAN_MAX, NBBY) */
#define	IEEE80211_CHAN_ANY	0xffff	/* token for ``any channel'' */
#define	IEEE80211_CHAN_ANYC 	((struct ieee80211_channel *) IEEE80211_CHAN_ANY)

#define	IEEE80211_RADAR_11HCOUNT		1
#define IEEE80211_DEFAULT_CHANCHANGE_TBTT_COUNT	10
#define	IEEE80211_RADAR_TEST_MUTE_CHAN	36	/* Move to channel 36 for mute test */

/* bits 0-3 are for private use by drivers */
/* channel attributes */
#define IEEE80211_CHAN_TURBO	0x00000010	/* Turbo channel */
#define IEEE80211_CHAN_CCK	0x00000020	/* CCK channel */
#define IEEE80211_CHAN_OFDM	0x00000040	/* OFDM channel */
#define IEEE80211_CHAN_2GHZ	0x00000080	/* 2 GHz spectrum channel. */
#define IEEE80211_CHAN_5GHZ	0x00000100	/* 5 GHz spectrum channel */
#define IEEE80211_CHAN_PASSIVE	0x00000200	/* Only passive scan allowed */
#define IEEE80211_CHAN_DYN	0x00000400	/* Dynamic CCK-OFDM channel */
#define IEEE80211_CHAN_GFSK	0x00000800	/* GFSK channel (FHSS PHY) */
#define IEEE80211_CHAN_RADAR	0x00001000	/* Status: Radar found on channel */
#define IEEE80211_CHAN_STURBO	0x00002000	/* 11a static turbo channel only */
#define IEEE80211_CHAN_HALF	0x00004000	/* Half rate channel */
#define IEEE80211_CHAN_QUARTER	0x00008000	/* Quarter rate channel */
#define IEEE80211_CHAN_HT20	0x00010000	/* HT 20 channel */
#define IEEE80211_CHAN_HT40U	0x00020000	/* HT 40 with ext channel above */
#define IEEE80211_CHAN_HT40D	0x00040000	/* HT 40 with ext channel below */
#define IEEE80211_CHAN_HT40	0x00080000	/* HT 40 with ext channel above/below */
#define IEEE80211_CHAN_DFS	0x00100000	/* Configuration: DFS-required channel */
#define IEEE80211_CHAN_DFS_CAC_DONE	0x00200000     /* Status: CAC completed */
#define IEEE80211_CHAN_VHT80	0x00400000	/* VHT 80 */
#define IEEE80211_CHAN_DFS_OCAC_DONE	0x00800000	/* Status: Off-channel CAC completed */
#define IEEE80211_CHAN_DFS_CAC_IN_PROGRESS	0x01000000	/* Status: Valid CAC is in progress */
#define IEEE80211_CHAN_WEATHER	0x02000000	/* Configuration: weather channel */

#define IEEE80211_DEFAULT_2_4_GHZ_CHANNEL	1
#define IEEE80211_DEFAULT_5_GHZ_CHANNEL		36

#define IEEE80211_MAX_2_4_GHZ_CHANNELS	13
#define IEEE80211_MAX_5_GHZ_CHANNELS	30
#define IEEE80211_MAX_DUAL_CHANNELS     (IEEE80211_MAX_2_4_GHZ_CHANNELS + IEEE80211_MAX_5_GHZ_CHANNELS)
#define CHIPID_2_4_GHZ					0
#define CHIPID_5_GHZ					1
#define CHIPID_DUAL                                     2

/*11AC - 40MHZ flags */
#define IEEE80211_CHAN_VHT40U	IEEE80211_CHAN_HT40U	/* VHT 40 with ext channel above */
#define IEEE80211_CHAN_VHT40D	IEEE80211_CHAN_HT40D	/* VHT 40 with ext channel below */
#define IEEE80211_CHAN_VHT40	IEEE80211_CHAN_HT40	/* VHT 40 with ext channel above/below */
/*11AC - 20MHZ flags */
#define IEEE80211_CHAN_VHT20	IEEE80211_CHAN_HT20	/* VHT 20 channel */

/* below are channel ext attributes(ic_ext_flags) */
/* 11AC - 80MHZ flags */
#define IEEE80211_CHAN_VHT80_LL	0x00000001
#define IEEE80211_CHAN_VHT80_LU	0x00000002
#define IEEE80211_CHAN_VHT80_UL	0x00000004
#define IEEE80211_CHAN_VHT80_UU	0x00000008


/*
 * Useful combinations of channel characteristics.
 */
#define	IEEE80211_CHAN_FHSS \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_GFSK)
#define	IEEE80211_CHAN_A \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define	IEEE80211_CHAN_B \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_CCK)
#define	IEEE80211_CHAN_PUREG \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)
#define	IEEE80211_CHAN_G \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN)
#define IEEE80211_CHAN_108A \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_TURBO)
#define	IEEE80211_CHAN_108G \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_TURBO)
#define	IEEE80211_CHAN_ST \
	(IEEE80211_CHAN_108A | IEEE80211_CHAN_STURBO)
#define	IEEE80211_CHAN_11N \
	(IEEE80211_CHAN_HT20)
#define	IEEE80211_CHAN_11NG \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_HT20)
#define	IEEE80211_CHAN_11NA \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_HT20)
#define	IEEE80211_CHAN_11NG_HT40U \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40U)
#define	IEEE80211_CHAN_11NG_HT40D \
	(IEEE80211_CHAN_2GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40D)
#define	IEEE80211_CHAN_11NA_HT40U \
	(IEEE80211_CHAN_5GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40U)
#define	IEEE80211_CHAN_11NA_HT40D \
	(IEEE80211_CHAN_5GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40D)
#define	IEEE80211_CHAN_11NG_HT40 \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40)
#define	IEEE80211_CHAN_11NA_HT40 \
	(IEEE80211_CHAN_5GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40)

#define IEEE80211_CHAN_11AC \
        (IEEE80211_CHAN_5GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_VHT20 )
#define IEEE80211_CHAN_11AC_VHT40 \
	(IEEE80211_CHAN_5GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_VHT20 | \
	 IEEE80211_CHAN_VHT40)
#define IEEE80211_CHAN_11AC_VHT80 \
	(IEEE80211_CHAN_5GHZ |IEEE80211_CHAN_OFDM |  IEEE80211_CHAN_VHT20 | \
	 IEEE80211_CHAN_VHT40 | IEEE80211_CHAN_VHT80 )

#define	IEEE80211_CHAN_ALL \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_HT20 | \
	 IEEE80211_CHAN_HT40U | IEEE80211_CHAN_HT40D | IEEE80211_CHAN_HT40| \
	 IEEE80211_CHAN_CCK | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_DYN| \
	 IEEE80211_CHAN_VHT20 | IEEE80211_CHAN_VHT40 | IEEE80211_CHAN_VHT80)

#define	IEEE80211_CHAN_ALLTURBO \
	(IEEE80211_CHAN_ALL | IEEE80211_CHAN_TURBO | IEEE80211_CHAN_STURBO)

#define IEEE80211_CHAN_ANYN \
	(IEEE80211_CHAN_HT20 | IEEE80211_CHAN_HT40U | IEEE80211_CHAN_HT40D | \
		IEEE80211_CHAN_HT40 )

#define	IEEE80211_IS_CHAN_CACDONE(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_DFS_CAC_DONE) != 0)
#define	IEEE80211_IS_CHAN_CAC_IN_PROGRESS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_DFS_CAC_IN_PROGRESS) != 0)

#define IEEE80211_IS_CHAN_FHSS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_FHSS) == IEEE80211_CHAN_FHSS)
#define	IEEE80211_IS_CHAN_A(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_A) == IEEE80211_CHAN_A)
#define	IEEE80211_IS_CHAN_B(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_B) == IEEE80211_CHAN_B)
#define	IEEE80211_IS_CHAN_PUREG(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_PUREG) == IEEE80211_CHAN_PUREG)
#define	IEEE80211_IS_CHAN_G(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_G) == IEEE80211_CHAN_G)
#define	IEEE80211_IS_CHAN_ANYG(_c) \
	(IEEE80211_IS_CHAN_PUREG(_c) || IEEE80211_IS_CHAN_G(_c))
#define	IEEE80211_IS_CHAN_ST(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_ST) == IEEE80211_CHAN_ST)
#define	IEEE80211_IS_CHAN_108A(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_108A) == IEEE80211_CHAN_108A)
#define	IEEE80211_IS_CHAN_108G(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_108G) == IEEE80211_CHAN_108G)

#define	IEEE80211_IS_CHAN_2GHZ(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_2GHZ) != 0)
#define	IEEE80211_IS_CHAN_5GHZ(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_5GHZ) != 0)
#define	IEEE80211_IS_CHAN_OFDM(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_OFDM) != 0)
#define	IEEE80211_IS_CHAN_CCK(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_CCK) != 0)
#define	IEEE80211_IS_CHAN_GFSK(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_GFSK) != 0)
#define	IEEE80211_IS_CHAN_TURBO(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_TURBO) != 0)
#define	IEEE80211_IS_CHAN_STURBO(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_STURBO) != 0)
#define	IEEE80211_IS_CHAN_DTURBO(_c) \
	(((_c)->ic_flags & \
	(IEEE80211_CHAN_TURBO | IEEE80211_CHAN_STURBO)) == IEEE80211_CHAN_TURBO)
#define	IEEE80211_IS_CHAN_HALF(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HALF) != 0)
#define	IEEE80211_IS_CHAN_QUARTER(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_QUARTER) != 0)
#define	IEEE80211_IS_CHAN_11N(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_11N) == IEEE80211_CHAN_11N)
#define	IEEE80211_IS_CHAN_11NG(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_11NG) == IEEE80211_CHAN_11NG)
#define	IEEE80211_IS_CHAN_11NA(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_11NA) == IEEE80211_CHAN_11NA)
#define IEEE80211_IS_CHAN_11AC(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_11AC) == IEEE80211_CHAN_11AC)
#define	IEEE80211_IS_CHAN_HT40PLUS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40U) == IEEE80211_CHAN_HT40U)
#define	IEEE80211_IS_CHAN_HT40MINUS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40D) == IEEE80211_CHAN_HT40D)
#define	IEEE80211_IS_CHAN_HT40(_c) \
	(IEEE80211_IS_CHAN_HT40PLUS((_c)) || IEEE80211_IS_CHAN_HT40MINUS((_c)))
#define	IEEE80211_IS_CHAN_11NG_HT40PLUS(_c) \
	(IEEE80211_IS_CHAN_11NG((_c)) && IEEE80211_IS_CHAN_HT40PLUS((_c)))
#define	IEEE80211_IS_CHAN_11NG_HT40MINUS(_c) \
	(IEEE80211_IS_CHAN_11NG((_c)) && IEEE80211_IS_CHAN_HT40MINUS((_c)))
#define	IEEE80211_IS_CHAN_11NA_HT40PLUS(_c) \
	(IEEE80211_IS_CHAN_11NA((_c)) && IEEE80211_IS_CHAN_HT40PLUS((_c)))
#define	IEEE80211_IS_CHAN_11NA_HT40MINUS(_c) \
	(IEEE80211_IS_CHAN_11NA((_c)) && IEEE80211_IS_CHAN_HT40MINUS((_c)))
#define IEEE80211_IS_CHAN_ANYN(_c) 1
	//(((_c)->ic_flags & IEEE80211_CHAN_ANYN))

#define	IEEE80211_IS_CHAN_VHT40PLUS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_VHT40U) == IEEE80211_CHAN_VHT40U)
#define	IEEE80211_IS_CHAN_VHT40MINUS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_VHT40D) == IEEE80211_CHAN_VHT40D)
#define IEEE80211_IS_CHAN_VHT40(_c) \
	(IEEE80211_IS_CHAN_VHT40PLUS(_c) || IEEE80211_IS_CHAN_VHT40MINUS(_c))
#define IEEE80211_IS_CHAN_11AC_VHT40PLUS(_c) \
	(IEEE80211_IS_CHAN_11AC(_c) && IEEE80211_IS_CHAN_VHT40PLUS(_c))
#define IEEE80211_IS_CHAN_11AC_VHT40MINUS(_c) \
	(IEEE80211_IS_CHAN_11AC(_c) && IEEE80211_IS_CHAN_VHT40MINUS(_c))

#define IEEE80211_IS_CHAN_VHT80_EDGEPLUS(_c) \
	(((_c)->ic_ext_flags & IEEE80211_CHAN_VHT80_LL) == IEEE80211_CHAN_VHT80_LL)
#define IEEE80211_IS_CHAN_VHT80_CNTRPLUS(_c) \
	(((_c)->ic_ext_flags & IEEE80211_CHAN_VHT80_LU) == IEEE80211_CHAN_VHT80_LU)
#define IEEE80211_IS_CHAN_VHT80_CNTRMINUS(_c) \
	(((_c)->ic_ext_flags & IEEE80211_CHAN_VHT80_UL) == IEEE80211_CHAN_VHT80_UL)
#define IEEE80211_IS_CHAN_VHT80_EDGEMINUS(_c) \
	(((_c)->ic_ext_flags & IEEE80211_CHAN_VHT80_UU) == IEEE80211_CHAN_VHT80_UU)
#define IEEE80211_IS_CHAN_VHT80(_c) \
	(IEEE80211_IS_CHAN_VHT80_EDGEPLUS(_c) || IEEE80211_IS_CHAN_VHT80_EDGEMINUS(_c) || \
	 IEEE80211_IS_CHAN_VHT80_CNTRPLUS(_c) || IEEE80211_IS_CHAN_VHT80_CNTRMINUS(_c))
#define IEEE80211_IS_CHAN_11AC_VHT80_EDGEPLUS(_c) \
	(IEEE80211_IS_CHAN_11AC(_c) && IEEE80211_IS_CHAN_VHT80_EDGEPLUS(_c))
#define IEEE80211_IS_CHAN_11AC_VHT80_CNTRPLUS(_c) \
	(IEEE80211_IS_CHAN_11AC(_c) && IEEE80211_IS_CHAN_VHT80_CNTRPLUS(_c))
#define IEEE80211_IS_CHAN_11AC_VHT80_CNTRMINUS(_c) \
	(IEEE80211_IS_CHAN_11AC(_c) && IEEE80211_IS_CHAN_VHT80_CNTRMINUS(_c))
#define IEEE80211_IS_CHAN_11AC_VHT80_EDGEMINUS(_c) \
	(IEEE80211_IS_CHAN_11AC(_c) && IEEE80211_IS_CHAN_VHT80_EDGEMINUS(_c))

/* mode specific macros */

#define IEEE80211_IS_11NG_MODE(_mode) \
		(((_mode) == IEEE80211_MODE_11NG) ||  \
		 ((_mode) == IEEE80211_MODE_11NG_HT40PLM))

#define IEEE80211_IS_11NA_MODE(_mode) \
		(((_mode) == IEEE80211_MODE_11NA) ||  \
		 ((_mode) == IEEE80211_MODE_11NA_HT40PM) )

#define IEEE80211_IS_11N_MODE(_mode) \
			(IEEE80211_IS_11NA_MODE((_mode)) || IEEE80211_IS_11NG_MODE((_mode)))

/* ni_chan encoding for FH phy */
#define	IEEE80211_FH_CHANMOD		80
#define	IEEE80211_FH_CHAN(set,pat)	(((set) - 1) * IEEE80211_FH_CHANMOD + (pat))
#define	IEEE80211_FH_CHANSET(chan)	((chan) / IEEE80211_FH_CHANMOD + 1)
#define	IEEE80211_FH_CHANPAT(chan)	((chan) % IEEE80211_FH_CHANMOD)

#define IEEE80211_HTCAP_TXBF_CAP_LEN	4

/* Peer RTS config */
#define IEEE80211_PEER_RTS_OFF		0
#define IEEE80211_PEER_RTS_PMP		1
#define IEEE80211_PEER_RTS_DYN		2
#define IEEE80211_PEER_RTS_MAX		2
#define IEEE80211_PEER_RTS_DEFAULT	IEEE80211_PEER_RTS_DYN

/* Dynamic WMM */
#define IEEE80211_DYN_WMM_OFF			0
#define IEEE80211_DYN_WMM_ON			1
#define IEEE80211_DYN_WMM_DEFAULT		IEEE80211_DYN_WMM_ON

#define IEEE80211_DYN_WMM_LOCAL_AIFS_DELTA	-2
#define IEEE80211_DYN_WMM_LOCAL_CWMIN_DELTA	-2
#define IEEE80211_DYN_WMM_LOCAL_CWMAX_DELTA	-3
#define IEEE80211_DYN_WMM_LOCAL_AIFS_MIN	{3, 3, 2, 1}
#define IEEE80211_DYN_WMM_LOCAL_CWMIN_MIN	{2, 4, 2, 2}
#define IEEE80211_DYN_WMM_LOCAL_CWMAX_MIN	{4, 6, 3, 3}
#define IEEE80211_DYN_WMM_BSS_AIFS_DELTA	2
#define IEEE80211_DYN_WMM_BSS_CWMIN_DELTA	2
#define IEEE80211_DYN_WMM_BSS_CWMAX_DELTA	3
#define IEEE80211_DYN_WMM_BSS_AIFS_MAX		{4, 5, 4, 3}
#define IEEE80211_DYN_WMM_BSS_CWMIN_MAX		{4, 6, 4, 3}
#define IEEE80211_DYN_WMM_BSS_CWMAX_MAX		{6, 8, 6, 5}

/*
 * 802.11 rate set.
 */
#define	IEEE80211_RATE_SIZE	8		/* 802.11 standard */
#define	IEEE80211_AG_RATE_MAXSIZE	12		/* Non 11n Rates */

#define	IEEE80211_RATE_MAXSIZE		30		/* max rates we'll handle */
#define	IEEE80211_HT_RATE_MAXSIZE	77		/* Total number of 802.11n rates */
#define	IEEE80211_HT_RATE_SIZE		128
#define IEEE80211_SANITISE_RATESIZE(_rsz) \
	((_rsz > IEEE80211_RATE_MAXSIZE) ? IEEE80211_RATE_MAXSIZE : _rsz)


/* For legacy hardware - leaving it as is for now */

#define IEEE80211_RATE_MCS	0x8000
#define IEEE80211_RATE_MCS_VAL	0x7FFF

//#define IEEE80211_RATE_IDX_ENTRY(val, idx) (val&(0xff<<(8*idx))>>(idx*8))
#define IEEE80211_RATE_IDX_ENTRY(val, idx) (((val&(0xff<<(idx*8)))>>(idx*8)))

/*
 * 11n A-MPDU & A-MSDU limits . XXX
 */
#define IEEE80211_AMPDU_LIMIT_MIN   (1 * 1024)
#define IEEE80211_AMPDU_LIMIT_MAX   (64 * 1024 - 1)
#define IEEE80211_AMSDU_LIMIT_MAX   4096


/*
 * 11ac MPDU size limit
 */
#define IEEE80211_MPDU_VHT_1K		1500
#define IEEE80211_MPDU_VHT_4K		3895
#define IEEE80211_MPDU_VHT_8K		7991
#define IEEE80211_MPDU_VHT_11K		11454
#define IEEE80211_MPDU_ENCAP_OVERHEAD_MAX	64 /* enough for mpdu header 36 + crypto 20 + fcs 4 */

/*
 * 11n and 11ac AMSDU sizes
 */
#define IEEE80211_AMSDU_NONE		0
#define IEEE80211_AMSDU_HT_4K		3839
#define IEEE80211_AMSDU_HT_8K		7935
#define IEEE80211_AMSDU_VHT_1K		(IEEE80211_MPDU_VHT_1K - IEEE80211_MPDU_ENCAP_OVERHEAD_MAX)
#define IEEE80211_AMSDU_VHT_4K		(IEEE80211_MPDU_VHT_4K - IEEE80211_MPDU_ENCAP_OVERHEAD_MAX)
#define IEEE80211_AMSDU_VHT_8K		(IEEE80211_MPDU_VHT_8K - IEEE80211_MPDU_ENCAP_OVERHEAD_MAX)
#define IEEE80211_AMSDU_VHT_11K		(IEEE80211_MPDU_VHT_11K - IEEE80211_MPDU_ENCAP_OVERHEAD_MAX)

/*
 * 11n MCS set limits
 */
#define IEEE80211_HT_MAXMCS_SET				10
#define IEEE80211_HT_MAXMCS_SET_SUPPORTED		10
#define IEEE80211_HT_MAXMCS_BASICSET_SUPPORTED		2
#define IEEE80211_MCS_PER_STREAM			8
/*
 * B0-B2: MCS index, B3-B6: MCS set index, B8: BASIC RATE
 */
#define IEEE80211_HT_BASIC_RATE		0x80
#define IEEE80211_HT_MCS_MASK		0x07
#define IEEE80211_HT_MCS_SET_MASK	0x78
#define IEEE80211_HT_RATE_TABLE_IDX_MASK	0x7F

#define IEEE80211_HT_MCS_VALUE(_v) \
		((_v) & IEEE80211_HT_MCS_MASK)

#define IEEE80211_HT_MCS_SET_IDX(_v) \
		(((_v) & IEEE80211_HT_MCS_SET_MASK) >> 3)

#define IEEE80211_HT_IS_BASIC_RATE(_v) \
		(((_v) & IEEE80211_HT_BASIC_RATE) == IEEE80211_HT_BASIC_RATE)

/* index number in the set will be (_i - 1) if (_i != 0) */
#define IEEE80211_HT_MCS_IDX(_m,_i) \
		{ \
			u_int8_t temp = (_m); \
			_i = 0; \
			while (temp) \
			{ \
				temp = temp >> 1; \
				_i++; \
			} \
			if(_i) \
				_i--; \
			else \
				_i = 0xFF; \
		}

/* rate table index = (MCS set index << 3) + MCS index */
#define IEEE80211_HT_RATE_TABLE_IDX(_s,_i) \
		(((_s) << 3) + (_i))

/* Supported rate label */
#define IEEE80211_RATE_11MBPS	22

#if 0
/* integer portion of HT rates */
u_int16_t ht_rate_table_20MHz_400[] = {
							7,
							14,
							21,
							28,
							43,
							57,
							65,
							72,
							14,
							28,
							43,
							57,
							86,
							115,
							130,
							144
						};

u_int16_t ht_rate_table_20MHz_800[] = {
							6,
							13,
							19,
							26,
							39,
							52,
							58,
							65,
							13,
							26,
							39,
							52,
							78,
							104,
							117,
							130
						};

u_int16_t ht_rate_table_40MHz_400[] = {
							15,
							30,
							45,
							60,
							90,
							120,
							135,
							150,
							30,
							60,
							90,
							120,
							180,
							240,
							270,
							300
						};

u_int16_t ht_rate_table_40MHz_800[] = {
							13,
							27,
							40,
							54,
							81,
							108,
							121,
							135,
							27,
							54,
							81,
							108,
							162,
							216,
							243,
							270
						};
#endif

struct ieee80211_rateset {
	u_int8_t		rs_legacy_nrates; /* Number of legacy rates */
	u_int8_t		rs_nrates; /* Total rates = Legacy + 11n */
	u_int8_t		rs_rates[IEEE80211_RATE_MAXSIZE];
};

struct ieee80211_ht_rateset {
	u_int8_t		rs_legacy_nrates; /* Number of legacy rates */
	u_int8_t		rs_nrates; /* Total rates = Legacy + 11n */
	u_int8_t		rs_rates[IEEE80211_HT_RATE_MAXSIZE];
};

struct ieee80211_roam {
	int8_t			rssi11a;	/* rssi thresh for 11a bss */
	int8_t			rssi11b;	/* for 11g sta in 11b bss */
	int8_t			rssi11bOnly;	/* for 11b sta */
	u_int8_t		pad1;
	u_int8_t		rate11a;	/* rate thresh for 11a bss */
	u_int8_t		rate11b;	/* for 11g sta in 11b bss */
	u_int8_t		rate11bOnly;	/* for 11b sta */
	u_int8_t		pad2;
};
struct ieee80211_htcap {
	u_int16_t		cap;		/* HT capabilities */
	u_int8_t		numtxspstr; /* Number of Tx spatial streams */
	u_int8_t		numrxstbcstr;	/* Number of Rx stbc streams */
	u_int8_t		pwrsave;	/* HT power save mode */
	u_int8_t		mpduspacing;	/* MPDU density */
	u_int16_t		maxmsdu;	/* Max MSDU size */
	u_int16_t		maxampdu;	/* maximum rx A-MPDU factor */
	u_int8_t		mcsset[IEEE80211_HT_MAXMCS_SET_SUPPORTED]; /* HT MCS set */
	u_int16_t		maxdatarate;	/* HT max data rate */
	u_int16_t		extcap;		/* HT extended capability */
	u_int8_t		mcsparams;	/* HT MCS params */
	u_int8_t		hc_txbf[IEEE80211_HTCAP_TXBF_CAP_LEN];	/* HT transmit beamforming capabilities */
} __packed;

struct ieee80211_htinfo {
	u_int8_t		ctrlchannel;	/* control channel */
	u_int8_t		byte1;		/* ht ie byte 1 */
	u_int8_t		byte2;		/* ht ie byte 2 */
	u_int8_t		byte3;		/* ht ie byte 3 */
	u_int8_t		byte4;		/* ht ie byte 4 */
	u_int8_t		byte5;		/* ht ie byte 5 */
	u_int8_t		sigranularity;	/* signal granularity */
	u_int8_t		choffset;	/* external channel offset */
	u_int8_t		opmode;		/* operational mode */
	u_int8_t		basicmcsset[IEEE80211_HT_MAXMCS_BASICSET_SUPPORTED]; /* basic MCS set */
} __packed;

/* VHT capabilities MIB */

/* Maximum MPDU Length B0-1 */
enum ieee80211_vht_maxmpdu {
	IEEE80211_VHTCAP_MAX_MPDU_3895 = 0,
	IEEE80211_VHTCAP_MAX_MPDU_7991,
	IEEE80211_VHTCAP_MAX_MPDU_11454,
	IEEE80211_VHTCAP_MAX_MPDU_RESERVED,
};

/* Supported Channel Width Set B2-3 */
enum ieee80211_vht_chanwidth {
	IEEE80211_VHTCAP_CW_80M_ONLY = 0,
	IEEE80211_VHTCAP_CW_160M,
	IEEE80211_VHTCAP_CW_160_AND_80P80M,
	IEEE80211_VHTCAP_CW_RESERVED,
};

/* RX STBC B8-10 */
enum ieee80211_vht_rxstbc {
	IEEE80211_VHTCAP_RX_STBC_NA = 0,
	IEEE80211_VHTCAP_RX_STBC_UPTO_1,
	IEEE80211_VHTCAP_RX_STBC_UPTO_2,
	IEEE80211_VHTCAP_RX_STBC_UPTO_3,
	IEEE80211_VHTCAP_RX_STBC_UPTO_4,
};

/* RX STS B13-15 */
enum ieee80211_vht_rxsts {
	IEEE80211_VHTCAP_RX_STS_1 = 0,
	IEEE80211_VHTCAP_RX_STS_2,
	IEEE80211_VHTCAP_RX_STS_3,
	IEEE80211_VHTCAP_RX_STS_4,
	IEEE80211_VHTCAP_RX_STS_5,
	IEEE80211_VHTCAP_RX_STS_6,
	IEEE80211_VHTCAP_RX_STS_7,
	IEEE80211_VHTCAP_RX_STS_8
};

/* SOUNDING DIM B16-18 */
enum ieee80211_vht_numsnd {
	IEEE80211_VHTCAP_SNDDIM_1 = 0,
	IEEE80211_VHTCAP_SNDDIM_2,
	IEEE80211_VHTCAP_SNDDIM_3,
	IEEE80211_VHTCAP_SNDDIM_4,
	IEEE80211_VHTCAP_SNDDIM_5,
	IEEE80211_VHTCAP_SNDDIM_6,
	IEEE80211_VHTCAP_SNDDIM_7,
	IEEE80211_VHTCAP_SNDDIM_8
};

/* Maximum A-MPDU Length exponent B23-25 */
/* 2^(13 + Max A-MPDU) -1 */
enum ieee80211_vht_maxampduexp {
	IEEE80211_VHTCAP_MAX_A_MPDU_8191,		/* (2^13) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_16383,	/* (2^14) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_32767,	/* (2^15) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_65535,	/* (2^16) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_131071,	/* (2^17) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_262143,	/* (2^18) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_524287,	/* (2^19) -1 */
	IEEE80211_VHTCAP_MAX_A_MPDU_1048575,	/* (2^20) -1 */
};

/* VHT link Adaptation capable B26-27 */
enum ieee80211_vht_lnkadptcap {
	IEEE80211_VHTCAP_LNKADAPTCAP_NO_FEEDBACK,
	IEEE80211_VHTCAP_LNKADAPTCAP_RESERVED,
	IEEE80211_VHTCAP_LNKADAPTCAP_UNSOLICITED,
	IEEE80211_VHTCAP_LNKADAPTCAP_BOTH,
};

/* VHT MCS supported */
enum ieee80211_vht_mcs_supported {
	IEEE80211_VHT_MCS_0_7,
	IEEE80211_VHT_MCS_0_8,
	IEEE80211_VHT_MCS_0_9,
	IEEE80211_VHT_MCS_NA,	// Spatial stream not supported
};

/* VHT NSS */
enum ieee80211_vht_nss {
	IEEE80211_VHT_NSS1 = 1,
	IEEE80211_VHT_NSS2,
	IEEE80211_VHT_NSS3,
	IEEE80211_VHT_NSS4,
	IEEE80211_VHT_NSS5,
	IEEE80211_VHT_NSS6,
	IEEE80211_VHT_NSS7,
	IEEE80211_VHT_NSS8,
};

struct ieee80211_vhtcap {
	u_int32_t			cap_flags;
	u_int32_t			maxmpdu;
	u_int32_t			chanwidth;
	u_int32_t			rxstbc;
	u_int8_t			bfstscap;
	u_int8_t			numsounding;
	u_int32_t			maxampduexp;
	u_int32_t			lnkadptcap;
	u_int16_t			rxmcsmap;
	u_int16_t			rxlgimaxrate;
	u_int16_t			txmcsmap;
	u_int16_t			txlgimaxrate;
	u_int8_t			bfstscap_save;
} __packed;

/* VHT capability macros */
#define VHT_SUPPORTS_MCS0_9_FOR_4SS_BIT	0x0080
#define VHT_SUPPORTS_MCS0_8_FOR_4SS_BIT	0x0040
#define VHT_SUPPORTS_MCS0_9_FOR_3SS_BIT	0x0020
#define VHT_SUPPORTS_MCS0_8_FOR_3SS_BIT	0x0010
#define VHT_SUPPORTS_MCS0_9_FOR_2SS_BIT	0x0008
#define VHT_SUPPORTS_MCS0_8_FOR_2SS_BIT	0x0004
#define VHT_SUPPORTS_MCS0_9_FOR_1SS_BIT	0x0002
#define VHT_SUPPORTS_MCS0_8_FOR_1SS_BIT	0x0001

#define IEEE80211_VHT_HAS_4SS(rxmcsmap) \
	!((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_4SS_BIT) && \
	(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_4SS_BIT))

#define IEEE80211_VHT_HAS_3SS(rxmcsmap) \
	!((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_3SS_BIT) && \
	(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_3SS_BIT))

#define IEEE80211_VHT_HAS_2SS(rxmcsmap) \
	!((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_2SS_BIT) && \
	(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_2SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_8_FOR_4SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_4SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_4SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_9_FOR_4SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_4SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_4SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_8_FOR_3SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_3SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_3SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_9_FOR_3SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_3SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_3SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_8_FOR_2SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_2SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_2SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_9_FOR_2SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_2SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_2SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_8_FOR_1SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_1SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_1SS_BIT))

#define IEEE80211_VHT_SUPPORTS_MCS0_9_FOR_1SS(rxmcsmap)	\
	((rxmcsmap & VHT_SUPPORTS_MCS0_9_FOR_1SS_BIT) && \
	!(rxmcsmap & VHT_SUPPORTS_MCS0_8_FOR_1SS_BIT))

/* VHT Operation element */
/* VHT Operation Information subfields */
enum ieee80211_vhtop_chanwidth {
	IEEE80211_VHTOP_CHAN_WIDTH_20_40MHZ,
	IEEE80211_VHTOP_CHAN_WIDTH_80MHZ,
	IEEE80211_VHTOP_CHAN_WIDTH_160MHZ,
	IEEE80211_VHTOP_CHAN_WIDTH_80PLUS80MHZ,
};

#define IEEE80211_VHT_MAXMCS_SET_SUPPORTED	10

struct ieee80211_vhtop {
	u_int32_t			chanwidth;
	u_int8_t			centerfreq0;
	u_int8_t			centerfreq1;
	u_int16_t			basicvhtmcsnssset;
} __packed;

/* Max number of MU groups */
#define IEEE80211_MU_GRP_NUM_MAX	64

/* Max number of nodes in a MU group */
#define IEEE80211_MU_GRP_NODES_MAX	4

/* VHT MU membership & user position arrays */
struct ieee80211_vht_mu_grp {
#define IEEE80211_VHT_GRP_1ST_BIT_OFFSET	1
#define IEEE80211_VHT_GRP_MAX_BIT_OFFSET	62
#define IEEE80211_VHT_GRP_MEMBERSHIP_ARRAY_SIZE	(IEEE80211_MU_GRP_NUM_MAX/(sizeof(u_int8_t)*8))
#define IEEE80211_VHT_USR_POS_ARRAY_SIZE	((IEEE80211_MU_GRP_NODES_MAX >> 1)*	\
							IEEE80211_MU_GRP_NUM_MAX/(sizeof(u_int8_t)*8))
	u_int8_t member[IEEE80211_VHT_GRP_MEMBERSHIP_ARRAY_SIZE];
	u_int8_t pos[IEEE80211_VHT_USR_POS_ARRAY_SIZE];
} __packed;

struct ieee80211_action_data {
	u_int8_t cat;				/* category identifier */
	u_int8_t action;			/* action identifier */
	void *params;
};

struct ba_action_req {
	u_int8_t		tid;		/* TID */
	u_int16_t		seq;		/* sequence number of first frame to be block acked */
	u_int8_t		frag;       /* fragment number of first frame to be block acked */
	enum ieee80211_ba_type type;/* block ack type */
	u_int16_t		buff_size;	/* suggested re-order buffer size */
	u_int16_t		timeout;	/* block ack timeout if no transfer */
};

struct ba_action_resp {
	u_int8_t		tid;		/* TID */
	u_int16_t		seq;		/* sequence number of first frame to be block acked */
	u_int8_t		frag;		/* fragment number of first frame to be block acked */
	enum ieee80211_ba_type type;/* block ack type */
	u_int16_t		buff_size;	/* actual re-order buffer size */
	u_int16_t		reason;		/* block ack negotiation status */
	u_int16_t		timeout;	/* negotiated block ack timeout if no transfer */
};

struct ba_action_del {
	u_int8_t		tid;		/* TID */
	u_int16_t		reason;		/* block ack termination reason */
	u_int8_t		initiator;	/* initiator/ recipient of block ack negotiation */
};

struct ht_action_nc_beamforming {
	u_int16_t		num_sts;		/* number of space time streams, Nc */
	u_int16_t		num_txchains;	/* number of transmit chains, Nr */
	u_int8_t		snr[2];			/* SNR for received space time streams */
	u_int16_t		size_matrices;	/* size of beamforming matrices in bytes */
	u_int8_t		*matrices;		/* pointer to beamforming matrices */
	u_int8_t		bw_mode;		/* bwmode = 0 for 20Mhz and 1 for 40 M */

};

struct ht_action_channelswitch {
	u_int8_t		ch_width;		/* switched channel width */
};

struct ht_action_sm_powersave {
	u_int8_t		sm_power_mode;		/* new power mode */
	u_int8_t		sm_power_enabled;	/* power save enabled */
};

struct ht_action_antennasel {
	u_int8_t		antenna_sel;	/* antenna selection: bit number corresponds
									to antenna number */
};

struct ht_action_mimo_ctrl {
	u_int8_t		num_columns;	/* Nc in received beamforming matrices */
	u_int8_t		num_rows;		/* Nr in received beamforming matrices */
	u_int8_t		chan_width;	/* Channel Width 0=20, 1 =40 */
	u_int8_t		num_grouping;	/* Ng in received beamforming matrices */
	u_int8_t		num_coeffsize;	/* Nb in received beamforming matrices */
	u_int8_t		snr[2];			/* SNR as seen by sender of action frame */
	u_int32_t		matrices[1024];		/* pointer to beamforming matrices,
										contents must be copied */
};

#ifdef CONFIG_QVSP
/**
 * The following structure definitions are for passing in data to the
 * management send function to generate action frames for VSP.
 */
struct ieee80211_qvsp_act {
	uint8_t oui[3];
	uint8_t type;
};

struct ieee80211_qvsp_strm_id {
	union {
		struct in6_addr	ipv6;
		__be32		ipv4;
	} saddr;
	union {
		struct in6_addr ipv6;
		__be32		ipv4;
	} daddr;
	__be16 sport;
	__be16 dport;
	uint8_t ip_version;
	uint8_t ip_proto;
	uint8_t ac;
} __packed;

#define IEEE8021_QVSP_MAX_ACT_ITEMS 32

struct ieee80211_qvsp_strm_dis_attr {
	uint32_t throt_policy;
	uint32_t throt_rate;
	uint32_t demote_rule;
	uint32_t demote_state;
};

struct ieee80211_qvsp_act_strm_ctrl {
	struct ieee80211_qvsp_act header;
	uint8_t strm_state;
	uint8_t count;
	struct ieee80211_qvsp_strm_dis_attr dis_attr;
	struct ieee80211_qvsp_strm_id strm_items[IEEE8021_QVSP_MAX_ACT_ITEMS];
};

struct ieee80211_qvsp_act_cfg_item {
	uint32_t index;
	uint32_t value;
};

struct ieee80211_qvsp_act_cfg {
	struct ieee80211_qvsp_act header;
	uint8_t count;
	struct ieee80211_qvsp_act_cfg_item cfg_items[IEEE8021_QVSP_MAX_ACT_ITEMS];
};
#endif

typedef void (*ppq_callback_success)(void *ctx);
typedef void (*ppq_callback_fail)(void *ctx, int32_t reason);

struct ieee80211_meas_request_ctrl {
	u_int8_t meas_type;
	unsigned long expire;
	ppq_callback_success fn_success;
	ppq_callback_fail fn_fail;
	union {
		struct _req_basic {
			u_int64_t start_tsf;
			u_int16_t duration_ms;
			u_int8_t channel;
		} basic;
		struct _req_cca {
			u_int64_t start_tsf;
			u_int16_t duration_ms;
			u_int8_t channel;
		} cca;
		struct _req_rpi {
			u_int64_t start_tsf;
			u_int16_t duration_ms;
			u_int8_t channel;
		} rpi;
		struct _req_sta_stats {
			void *sub_item;
			u_int16_t duration_tu;
			u_int8_t group_id;
		} sta_stats;
		struct _req_qtn_cca {
			u_int16_t duration_tu;
		} qtn_cca;
		struct _req_chan_load {
			u_int8_t channel;
			u_int16_t duration_ms;
		} chan_load;
		struct _req_noise_his {
			u_int8_t channel;
			u_int16_t duration_ms;
		} noise_his;
		struct _req_beacon {
			u_int8_t op_class;
			u_int8_t channel;
			u_int8_t duration_ms;
			u_int8_t mode;
			u_int8_t bssid[6];
		} beacon;
		struct _req_frame {
			u_int8_t op_class;
			u_int8_t channel;
			u_int16_t duration_ms;
			u_int8_t type;
			u_int8_t mac_address[6];
		} frame;
		struct _req_tran_stream_cat {
			u_int16_t duration_ms;
			u_int8_t peer_sta[6];
			u_int8_t tid;
			u_int8_t bin0;
		} tran_stream_cat;
		struct _req_multicast_diag {
			u_int16_t duration_ms;
			u_int8_t group_mac[6];
		} multicast_diag;
	} u;
};

struct ieee80211_meas_report_ctrl {
	u_int8_t meas_type;
	u_int8_t report_mode;
	u_int8_t token;		/* dialog token */
	u_int8_t meas_token;	/* measurement token */
	u_int8_t autonomous;	/* 1: autonomous report */
	union {
		struct _rep_basic {
			u_int8_t channel;
			u_int8_t basic_report;
			u_int16_t duration_tu;
			u_int64_t start_tsf;
		} basic;
		struct _rep_cca {
			u_int8_t channel;
			u_int8_t cca_report;
			u_int16_t duration_tu;
			u_int64_t start_tsf;
		} cca;
		struct _rep_rpi {
			u_int64_t start_tsf;
			u_int16_t duration_tu;
			u_int8_t channel;
			u_int8_t rpi_report[8];
		} rpi;
		struct _rep_sta_stats {
			void *sub_item;
			u_int16_t duration_tu;
			u_int8_t group_id;
		} sta_stats;
		struct _rep_qtn_cca {
			u_int64_t start_tsf;
			u_int16_t duration_ms;
			u_int8_t channel;
			u_int8_t qtn_cca_report;
			u_int32_t sp_fail;
			u_int32_t lp_fail;
			u_int16_t others_time;
			u_int8_t *extra_ie;
			u_int16_t extra_ie_len;
		} qtn_cca;
		struct _rep_chan_load {
			u_int8_t op_class;
			u_int8_t channel;
			u_int16_t duration_tu;
			u_int8_t channel_load;
		} chan_load;
		struct _rep_noise_his {
			u_int8_t op_class;
			u_int8_t channel;
			u_int16_t duration_tu;
			u_int8_t antenna_id;
			u_int8_t anpi;
			u_int8_t ipi[11];
		} noise_his;
		struct _rep_beacon {
			u_int8_t op_class;
			u_int8_t channel;
			u_int16_t duration_tu;
			u_int8_t reported_frame_info;
			u_int8_t rcpi;
			u_int8_t rsni;
			u_int8_t bssid[6];
			u_int8_t antenna_id;
			u_int8_t parent_tsf[4];
		} beacon;
		struct _rep_frame {
			void *sub_item;
			u_int8_t op_class;
			u_int8_t channel;
			u_int16_t duration_tu;
		} frame;
		struct _rep_tran_stream_cat {
			u_int16_t duration_tu;
			u_int8_t peer_sta[6];
			u_int8_t tid;
			u_int8_t reason;
			u_int32_t tran_msdu_cnt;
			u_int32_t msdu_discard_cnt;
			u_int32_t msdu_fail_cnt;
			u_int32_t msdu_mul_retry_cnt;
			u_int32_t qos_lost_cnt;
			u_int32_t avg_queue_delay;
			u_int32_t avg_tran_delay;
			u_int8_t bin0_range;
			u_int32_t bins[6];
		} tran_stream_cat;
		struct _rep_multicast_diag {
			u_int16_t duration_tu;
			u_int8_t group_mac[6];
			u_int8_t reason;
			u_int32_t mul_rec_msdu_cnt;
			u_int16_t first_seq_num;
			u_int16_t last_seq_num;
			u_int16_t mul_rate;
		} multicast_diag;
	} u;
};

struct stastats_subele_vendor {
	u_int32_t flags;
	u_int8_t sequence;
};

struct frame_report_subele_frame_count {
	u_int8_t ta[6];
	u_int8_t bssid[6];
	u_int8_t phy_type;
	u_int8_t avg_rcpi;
	u_int8_t last_rsni;
	u_int8_t last_rcpi;
	u_int8_t antenna_id;
	u_int16_t frame_count;
};

/* TPC actions */
struct ieee80211_action_tpc_request {
	unsigned long expire;
	ppq_callback_success fn_success;
	ppq_callback_fail fn_fail;
};

struct ieee80211_action_tpc_report {
	uint8_t		rx_token;
	int8_t		tx_power;
	int8_t		link_margin;
};

struct ppq_request_param {
	unsigned long expire;
	ppq_callback_success fn_success;
	ppq_callback_fail fn_fail;
};

struct ieee80211_link_measure_request {
	struct ppq_request_param ppq;
};

struct ieee80211_link_measure_report {
	uint8_t token;
	struct ieee80211_action_tpc_report tpc_report;
	uint8_t recv_antenna_id;
	uint8_t tran_antenna_id;
	uint8_t rcpi;
	uint8_t rsni;
};

struct ieee80211_neighbor_report_request {
	struct ppq_request_param ppq;
};

struct ieee80211_neighbor_report_request_item {
	uint8_t bssid[6];
	uint32_t bssid_info;
	uint8_t operating_class;
	uint8_t channel;
	uint8_t phy_type;
};

struct ieee80211_neighbor_report_response {
	uint8_t token;
	uint8_t bss_num;
	struct ieee80211_neighbor_report_request_item *neighbor_report_ptr[32];
};

#define IEEE80211_MAXIMUM_TIMESTAMP_DIFF_NC_BF	1000000

#define	IEEE80211_TXPOWER_MAX	100	/* .5 dBm units */
#define	IEEE80211_TXPOWER_MIN	0	/* kill radio */

#define	IEEE80211_DTIM_MAX	15	/* max DTIM period */
#define	IEEE80211_DTIM_MIN	1	/* min DTIM period */
#define	IEEE80211_DTIM_DEFAULT	3	/* default DTIM period */

#define	IEEE80211_BINTVAL_MAX	5000	/* max beacon interval (TU's) */
#define	IEEE80211_BINTVAL_MIN	25	/* min beacon interval (TU's) */
#define	IEEE80211_BINTVAL_DEFAULT 100	/* default beacon interval (TU's) */
#define IEEE80211_BINTVAL_VALID(_bi) \
	((IEEE80211_BINTVAL_MIN <= (_bi)) && \
	 ((_bi) <= IEEE80211_BINTVAL_MAX))
#define IEEE80211_BINTVAL_SANITISE(_bi) \
	(IEEE80211_BINTVAL_VALID(_bi) ? \
	 (_bi) : IEEE80211_BINTVAL_DEFAULT)

#define IEEE80211_SCAN_TBL_LEN_MAX_DFLT	2000

#define IEEE80211_BWSTR_20	"20"
#define IEEE80211_BWSTR_40	"40"
#define IEEE80211_BWSTR_80	"80"
#define IEEE80211_BWSTR_160	"160"
#define IEEE80211_BWSTR_80P80	"80+80"

#endif /* _NET80211__IEEE80211_H_ */
