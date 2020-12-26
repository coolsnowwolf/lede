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
    dot11ac_vht.h

    Abstract:
	Defined IE/frame structures of 802.11ac (D1.2).

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Shiang Tu  01-11-2012    created for 11ac
 */

#ifdef DOT11_VHT_AC


#ifndef __DOT11AC_VHT_H
#define __DOT11AC_VHT_H

#include "rtmp_type.h"
#include "dot11_base.h"

#define IE_VHT_CAP		191
#define IE_VHT_OP		192
#define IE_EXT_BSS_LOAD			193
#define IE_WIDE_BW_CH_SWITCH		194
#define IE_VHT_TXPWR_ENV			195
#define IE_CH_SWITCH_WRAPPER		196
#define IE_AID						197
#define IE_QUIET_CHANNEL			198


#define BW_SIGNALING_DISABLE	0
#define BW_SIGNALING_STATIC	1
#define BW_SIGNALING_DYNAMIC	2

/*
	IEEE 802.11AC D3.0 sec 8.4.1.50
	Operating Mode field

	ch_width: Channel width
		->	0: 20MHz
			1: 40MHz
			2: 80MHz
			3: 160 or 80+80MHz
			Reserved if Rx Nss subfield is 1
	rx_nss: Rx Nss
		If the Rx Nss Type subfield is 0, indicate the max number of SS STA can rx.
		If the Rx Nss Type subfield is 1, indicate the max number of SS that the STA can receive as a
			beamformee in a SU PPDU using a beamforming steering matrix derived from a VHT
			compressed beamforming report with Feedback Type subfield indicating MU in the VHT
			Compressed Beamforming frames
		->	0: Nss=1
			1: Nss=2
			...
			7: Nss=8

	rx_nss_type:
		->	0: indicate the rx_nss subfield carries the max number of SS that the STA can receive
			1: indicate the rx_nss subfield carries the max number of SS that the STA can receive
				as an SU PPDU using a beamforming steering matrix derived from a VHT compressed
				Beamforming frame with the Feedback Type subfield indicating MU in the VHT compressed
				Beamforming frames.
*/
typedef struct GNU_PACKED _OPERATING_MODE {
#ifdef RT_BIG_ENDIAN
	UCHAR rx_nss_type:1;
	UCHAR rx_nss:3;
	UCHAR rsv2:2;
	UCHAR ch_width:2;
#else
	UCHAR ch_width:2;
	UCHAR rsv2:2;
	UCHAR rx_nss:3;
	UCHAR rx_nss_type:1;
#endif /* RT_BIG_ENDIAN */
} OPERATING_MODE;


/*
	IEEE 802.11AC D3.0 sec 8.4.2.168
	Operating Mode Notification element

	Element ID: 199 (IE_OPERATING_MODE_NOTIFY)
	Length: 1
*/
typedef struct GNU_PACKED _OPERATING_MODE_NOTIFICATION {
	OPERATING_MODE operating_mode;
} OPERATING_MODE_NOTIFICATION;


#define DOT11_VHT_MAX_MPDU_LEN_3895		0
#define DOT11_VHT_MAX_MPDU_LEN_7991		1
#define DOT11_VHT_MAX_MPDU_LEN_11454	2

/*
	IEEE 802.11AC D2.0, sec 8.4.2.160.2
	VHT Capabilities Info field

	max_mpdu_len: MAximun MPDU Length
		->Indicate the max MPDU length.
				0: 3895 octets(Max A-MSDU length in HT Cap set to 3839)
				1: 7991 octets(Max A-MSDU length in HT Cap set to 7935)
				2: 11454 octets(Max A-MSDU length in HT Cap set to 7935)
				3: reserved
		->refer to "DOT11_VHT_MAX_MPDU_LEN_xxxx"
	ch_width: Supported Channel Width Set
		->Indicates the channel widths supported by the STA.
				0: the STA does not support either 160 or 80+80 MHz
				1: the STA support 160 MHz
				2: the STA support 160 MHz and 80 + 80 MHz
				3: reserved
	rx_ldpc: Rx LDPC
		-> Indicates support of receiving LDPC coded packets
				0: not support
				1: support
	sgi_80M: Short GI for 80 MHz
		-> Indicates short GI support for the reception of VHT+CBW80 packet
				0: not support
				1: support
	sgi_160M: Short GI for 160 and 80 + 80 MHz
		->Indicates rx short GI for VHT+(CBW160 and CBW80+80) packet
				0: not support
				1: support
	tx_stbc: Tx STBC
		-> Indicates support for tx of at least 2x1 STBC
				0: not support
				1: support
	rx_stbc: Rx STBC
		-> Indicates support for rx of PPDUs using STBC
				0: not support
				1: support 1SS
				2: support 1SS and 2SS
				3: support 1SS, 2SS and 3SS
				4: support 1SS, 2SS, 3SS and 4SS
				5,6,7: reserved
	bfer_cap_su: SU Beamformer Capable
		->Indicates support for operation as a single user beamformer
				0: not support
				1: support
	bfee_cap_su: SU Beamformee Capable
		-> Indicates support for operation as a single user beamformee
				0: not support
				1: support
	cmp_st_num_bfer: Compressed Steering Number of Beamformer Antenna Supported
		-> Beamformee's capability indicateing the max number of beamformer
			antennas the beamformee can support when sending compressed
			beamforming feedback
				If SU beamformee capable, set to the max value minus 1.
				otehrwise, reserved.
	num_snd_dimension: Number of Sounding Dimensions
		-> Beamformer's capability indicating the number of antennas used for
			beamformed transmissions.
				If SU beamformer capable, set to value minus 1.
				otehrwise, reserved.
	bfer_cap_mu: MU Beamformer Capable
		-> Indicates support for operation as an MU beamformer
				0: if not supported or if sent by a non-AP STA
				1: supported
	bfee_cap_mu: MU Beamformee Capable
		-> Indicates support for operation as an MU beamformer
				0: if not supported or if snet by an AP
				1: supported
	vht_txop_ps: VHT TXOP PS
		-> Indicates whether or not the AP supports VHT TXOP Power Save Mode or
			whether or not the STA is in VHT TXOP Power Save Mode
		->When tx by a VHT AP in the VHT Capabilities element included in Beacon,
			ProbeResp, AssocResp, and ReassocResp, frames:
				0: if the VHT AP does not support VHT TXOP PS in the BSS.
				1: if the VHT AP support TXOP PS in the BSS.
		->When tx by a VHT non-AP STA in the VHT Capabilities element included
			in AssocReq, ReassocReq and ProbReq frames:
				0: if the VHT STA is not in TXOP Power Save Mode.
				1: if the VHT STA is in TXOP Power Save Mode.
	htc_vht_cap: +HTC-VHT Capable
		-> Indicates whether or not the STA supports receiving an HT Control
			field in the VHT format
				0: if not support
				1: if support
	max_ampdu_exp: Maximum A-MPDU Length Exponent
		-> Indicates the maximum length of A-MPDU pre-EOF padding that the STA
			can receive.
		->The length defined by this field is equal to 2^(13 + max_ampdu_exp) -1
				0~7 : integer in the range of 0 to 7.
	vht_link_adapt: VHT Link Adaptation Capable
		-> Indicates whether or not the STA support link adaptation using VHT
			variant HT Control field.
		-> This field is ignored if the _HTC-VHT Capble field is set to 0.
				0: (No Feedback), if the STA does not provide VHT MFB
				2: (Unsolicited), if the STA provides only unsolicited VHT MFB
				3: (Both), if the STA can provide VHT MFB in response to VHT MRQ
					and if the STA provides unsolicited VHT MFB.
				1: reserved
	rx_ant_consistency: Rx Antenna Pattern Consistency
		->Indicates the possibility of Rx antenna pattern change
				0: if Rx antenna pattern might change during association
				1: if Rx antenna pattern does not change during association
	tx_ant_consistency: Tx Antenna Pattern Consistency
		->Indicates the possibility of Tx antenna pattern change
				0: if Tx antenna pattern might change during association
				1: if Tx antenna pattern does not change during association
*/
typedef struct GNU_PACKED _VHT_CAP_INFO {
#ifdef RT_BIG_ENDIAN
	UINT32 rsv:2;
	UINT32 tx_ant_consistency:1;
	UINT32 rx_ant_consistency:1;
	UINT32 vht_link_adapt:2;
	UINT32 max_ampdu_exp:3;
	UINT32 htc_vht_cap:1;
	UINT32 vht_txop_ps:1;
	UINT32 bfee_cap_mu:1;
	UINT32 bfer_cap_mu:1;
	UINT32 num_snd_dimension:3;

	UINT32 bfee_sts_cap:3;
	UINT32 bfee_cap_su:1;
	UINT32 bfer_cap_su:1;
	UINT32 rx_stbc:3;

	UINT32 tx_stbc:1;
	UINT32 sgi_160M:1;
	UINT32 sgi_80M:1;
	UINT32 rx_ldpc:1;
	UINT32 ch_width:2;
	UINT32 max_mpdu_len:2;
#else
	UINT32 max_mpdu_len:2;	/* 0: 3895, 1: 7991, 2: 11454, 3: rsv */
	UINT32 ch_width:2;	/* */
	UINT32 rx_ldpc:1;
	UINT32 sgi_80M:1;
	UINT32 sgi_160M:1;
	UINT32 tx_stbc:1;

	UINT32 rx_stbc:3;
	UINT32 bfer_cap_su:1;
	UINT32 bfee_cap_su:1;
	UINT32 bfee_sts_cap:3;

	UINT32 num_snd_dimension:3;
	UINT32 bfer_cap_mu:1;
	UINT32 bfee_cap_mu:1;
	UINT32 vht_txop_ps:1;
	UINT32 htc_vht_cap:1;
	UINT32 max_ampdu_exp:3;
	UINT32 vht_link_adapt:2;
	UINT32 rx_ant_consistency:1;
	UINT32 tx_ant_consistency:1;
	UINT32 rsv:2;
#endif /* RT_BIG_ENDIAN */
} VHT_CAP_INFO;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.160.3
	Rx MCS Map and Tx MCS map, figure 8-401bt

	mcs_ss1: Max MCS for 1SS
	mcs_ss2: Max MCS for 2SS
	mcs_ss3: Max MCS for 3SS
	mcs_ss4: Max MCS for 4SS
	mcs_ss5: Max MCS for 5SS
	mcs_ss6: Max MCS for 6SS
	mcs_ss7: Max MCS for 7SS
	mcs_ss8: Max MCS for 8SS

	The 2-bit MAx MCS for n SS field for each number of spatial streams n = 1~8
	is encoded as following:
		0: indicates support for MCS 0~7
		1: indicates support for MCS 0~8
		2: indicates support for MCS 0~9
		3: indicates that n spatial streams is not supported.
	Note: some MCSs are not be valid for particular bandwidth and number of
		spatial stream combinations.
*/
#define VHT_MCS_CAP_7	0
#define VHT_MCS_CAP_8	1
#define VHT_MCS_CAP_9	2
#define VHT_MCS_CAP_NA	3

typedef struct GNU_PACKED _VHT_MCS_MAP {
#ifdef RT_BIG_ENDIAN
	UINT16 mcs_ss8:2;
	UINT16 mcs_ss7:2;
	UINT16 mcs_ss6:2;
	UINT16 mcs_ss5:2;
	UINT16 mcs_ss4:2;
	UINT16 mcs_ss3:2;
	UINT16 mcs_ss2:2;
	UINT16 mcs_ss1:2;
#else
	UINT16 mcs_ss1:2;
	UINT16 mcs_ss2:2;
	UINT16 mcs_ss3:2;
	UINT16 mcs_ss4:2;
	UINT16 mcs_ss5:2;
	UINT16 mcs_ss6:2;
	UINT16 mcs_ss7:2;
	UINT16 mcs_ss8:2;
#endif /* RT_BIG_ENDIAN */
} VHT_MCS_MAP;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.160.3
	VHT Supported MCS Set field, figure 8-401bs

	rx_mcs_map: Rx MCS Map
		-> Indicates the maximum MCS that can be received for each number of
			spatial streams
				See "VHT_MCS_MAP"
	rx_high_rate: Rx Highest Supported Data Rate
		-> Indicates the maximum data rate that the STA can receive
		-> In unit of 1Mb/s where 1 represents 1Mb/s, and incrementing in steps
			of 1 Mb/s.
		-> If the maximum data rate expressed in Mb/s is not an integer, then
			the value is rounded up to the next integer.
	tx_mcs_map: Tx MCS Map
	tx_high_rate: Tx Highest Supported Data Rate
		-> Indicates the maximum data rate that the STA will transmit
		-> In unit of 1Mb/s where 1 represents 1Mb/s, and incrementing in steps
			of 1 Mb/s.
		-> If the maximum data rate expressed in Mb/s is not an integer, then
			the value is rounded up to the next integer.
*/

/* TODO: shiang-6590, check the layout of this data structure!!!! */
typedef struct GNU_PACKED _VHT_MCS_SET {
#ifdef RT_BIG_ENDIAN
	UINT16 rsv2:3;
	UINT16 tx_high_rate:13;
	struct _VHT_MCS_MAP tx_mcs_map;

	UINT16 rsv:3;
	UINT16 rx_high_rate:13;
	struct _VHT_MCS_MAP rx_mcs_map;
#else
	struct _VHT_MCS_MAP rx_mcs_map;

	UINT16 rx_high_rate:13;
	UINT16 rsv:3;
	struct _VHT_MCS_MAP tx_mcs_map;

	UINT16 tx_high_rate:13;
	UINT16 rsv2:3;
#endif /* RT_BIG_ENDIAN */
} VHT_MCS_SET;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.160.1
	VHT Capabilities Element structure

	eid: Element ID
			191   (IE_VHT_CAP)
	len: Length
			12
	vht_cap: VHT Capabilities Info
		->contains a numner of fields that are used to advertise VHT capabilities
			of a VHT STA
	mcs_set: VHT supported MCS Set
		->Used to convey the combinations of MCSs and spatial streams a STA
			supports for both reception and transmission.
*/
typedef struct GNU_PACKED _VHT_CAP_IE {
	VHT_CAP_INFO vht_cap;
	VHT_MCS_SET mcs_set;
} VHT_CAP_IE;

//
// Size of VHT capabilities IE, excluding element ID and length fields
//
#define SIZE_OF_VHT_CAP_IE    (sizeof(VHT_CAP_IE))

/*
	IEEE 802.11AC D2.0, sec 8.4.2.161
	VHT Operation Information field, figure 8-401bv

	The operation of VHT STAs in the BSS is controlled by the HT Operation
	element and the VHT Operation element.

	ch_width: Channel Width
		-> This field, together with the HT Operation element STA Channel Width
			field, defines the BSS operating channel width.
				0: for 20MHz or 40MHz operating channel width
				1: for 80MHz operating channel width
				2: for 160MHz operating channel width
				3: for 80+80MHz operating channel width
				4~255: reserved
	center_freq_1: Channel Center Frequency Segment 1
		-> Defines the channel center frequency for an 80 and 160MHz VHT BSS
			and the segment 1 channel center frequency for an 80+90MHz VHT BSS.
		-> For 80MHZ or 160MHz operating channel width, indicates the channel
			center frequency index for the 80MHz or 160MHz channel on which the
			VHT BSS operates.
		->For 80+80MHz operating channel width, indicates the channel center
			frequency index for the 80MHz channel of frequency segment 1 on
			which the VHT BSS operates.
		->Set 0 for 20MHz or 40MHz operating channel width.

	center_freq_2: Channel Center Frequency Segment 2
		-> Defines the seg 2 channel center frequency for an 80+80MHz VHT BSS
		->For a 80+80MHz operating channel width, indicates the channel center
			frequency index of the 80MHz channel of frequency segment 2 on
			which the VHT BSS operates. Reserved otherwise.
*/
typedef struct GNU_PACKED _VHT_OP_INFO {
	UINT8 ch_width;
	UINT8 center_freq_1;
	UINT8 center_freq_2;
} VHT_OP_INFO;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.161
	VHT Operation element, figure 8-401bu

	The operation of VHT STAs in the BSS is controlled by the HT Operation
	element and the VHT Operation element.

	eid: Element ID
			192 (IE_VHT_OP)
	len: Length
			5
	vht_op_info: VHT Operation Information
	basic_mcs_set: VHT Basic MCS Set
*/
typedef struct GNU_PACKED _VHT_OP_IE {
	VHT_OP_INFO vht_op_info;
	VHT_MCS_MAP basic_mcs_set;
} VHT_OP_IE;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.163
	Wide Bandwidth Channel Switch element, figure 8-401bx

	included in the Channel Switch Announcement frames.

	new_ch_width: New STA Channel Width
	center_freq_1: New Channel Center Frequency Segment 1
	center_freq_2: New Channel Center Frequency Segment 2

	The definition of upper subfields is the same as "VHT_OP_INFO"
*/
typedef struct GNU_PACKED _WIDE_BW_CH_SWITCH_ELEMENT {
	UINT8 new_ch_width;
	UINT8 center_freq_1;
	UINT8 center_freq_2;
} WIDE_BW_CH_SWITCH_ELEMENT;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.164
	VHT Transmit Power Envelope element
*/
typedef struct GNU_PACKED _CH_SEG_PAIR {
	UINT8 ch_center_freq;
	UINT8 seg_ch_width;
} CH_SEG_PAIR;


/*
	max_tx_pwr_cnt:
		0: Local Maximum Transmit Power For 20 MHz.
		1: Local Maximum Transmit Power For 20, 40MHz
		2: Local Maximum Transmit Power For 20, 40, 80MHz
		3: Local Maximum Transmit Power For 20, 40, 80, 160/80+80MHz
		4~7: rsv

	max_tx_pwr_interpretation:
		0: EIRP
		1~7: rsv
*/
#define TX_PWR_INTERPRET_EIRP		0
typedef struct GNU_PACKED _VHT_TX_PWR_INFO_ {
#ifdef RT_BIG_ENDIAN
	UINT8 rsv6:2;
	UINT8 max_tx_pwr_interpretation:3;
	UINT8 max_tx_pwr_cnt:3;
#else
	UINT8 max_tx_pwr_cnt:3;
	UINT8 max_tx_pwr_interpretation:3;
	UINT8 rsv6:2;
#endif
} VHT_TX_PWR_INFO;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.164
	VHT Transmit Power Envelope element

	max_txpwr: Maximum Transmit Power
		-> Define the maximum transmit power limit of the tx bandwidth defined
			by the VHT Transmit Power Envelop element. The Maximum Transmit
			Power field is a 8 bit 2's complement signed integer in the range of
			-64 dBm to 63.5 dBm with a 0.5 dB step.

	NOTE: The following two subfields may repeated as needed.
		center_freq_1: Channel Center Frequency Segment
		ch_seg_width: Segment Channel Width
*/
typedef struct GNU_PACKED _VHT_TXPWR_ENV_IE {
	VHT_TX_PWR_INFO tx_pwr_info;
	UINT8 tx_pwr_bw[4];
} VHT_TXPWR_ENV_IE;


typedef struct  GNU_PACKED _VHT_CONTROL {
#ifdef RT_BIG_ENDIAN
	UINT32 RDG:1;
	UINT32 ACConstraint:1;
	UINT32 unso_mfb:1;
	UINT32 fb_tx_type:1;
	UINT32 coding:1;
	UINT32 gid_h:3;
	UINT32 mfb_snr:6;
	UINT32 mfb_bw:2;
	UINT32 mfb_mcs:4;
	UINT32 mfb_n_sts:3;
	UINT32 mfsi_gidl:3;
	UINT32 stbc_ind:1;
	UINT32 comp_msi:2;
	UINT32 mrq:1;
	UINT32 rsv:1;
	UINT32 vht:1;
#else
	UINT32 vht:1;
	UINT32 rsv:1;
	UINT32 mrq:1;
	UINT32 comp_msi:2;
	UINT32 stbc_ind:1;
	UINT32 mfsi_gidl:3;
	UINT32 mfb_n_sts:3;
	UINT32 mfb_mcs:4;
	UINT32 mfb_bw:2;
	UINT32 mfb_snr:6;
	UINT32 gid_h:3;
	UINT32 coding:1;
	UINT32 fb_tx_type:1;
	UINT32 unso_mfb:1;
	UINT32 ACConstraint:1;
	UINT32 RDG:1;
#endif
} VHT_CONTROL;


/*
	802.11 AC Draft3.1 - Section 8.3.1.19, Figure 8-29j

	token_num: Sounding Dialog Token Number
			Contains a value selected by the beamformer to identify the VHT NDP
			Announcment frame.
*/
typedef struct GNU_PACKED _SNDING_DIALOG_TOKEN {
#ifdef RT_BIG_ENDIAN
	UINT8 token_num:6;
	UINT8 rsv:2;
#else
	UINT8 rsv:2;
	UINT8 token_num:6;
#endif /* RT_BIG_ENDIAN */
} SNDING_DIALOG_TOKEN;


/*
	802.11 AC Draft3.1 - Section 8.3.1.19, Figure 8-29k

	aid12: AID12
			the 12 least significiant bits of the AID of a STA expected to
			process the following VHT NDP and prepare the sounding
			feedback. Equal to 0 if the STA is the AP, mesh STA or STA
			that is a member of an IBSS
	fb_type: Feedback Type
			Indicates the type of feedback requested
			0: SU, 1: MU
	nc_idx: Nc_Index
			If the fb_type field indicates MU, then Nc Index indicates the
				number of columns, Nc, in the compressed Beamforming
				Feedback Matrix subfield minus one:
					Set to 0 to request Nc=1,
					Set to 1 to request Nc=2,
					...
					Set to 7 to request Nc=8,
			Reserved if the Feedback Type Field indicates SU.
*/
typedef enum _SNDING_FB_TYPE {
	SNDING_FB_SU = 0,
	SNDING_FB_MU = 1,
} SNDING_FB_TYPE;

typedef struct GNU_PACKED _SNDING_STA_INFO {
#ifdef RT_BIG_ENDIAN
	UINT16 nc_idx:3;
	UINT16 fb_type:1;
	UINT16 aid12:12;
#else
	UINT16 aid12:12;
	UINT16 fb_type:1;
	UINT16 nc_idx:3;
#endif /* RT_BIG_ENDIAN */
} SNDING_STA_INFO;


/*
	802.11 AC Draft3.1 - Section 8.3.1.19, Figure 8-29i

	VHT NDP Announcment frame format

	fc: Frame Control

	duration: Duration

	ra: RA
		If the VHT NDPA frame contains only one STA Info field
			=> the RA field is set to the address of the STA
			identified by the AID in the STA info field.
		If the VHT NDPA frame contains more than one STA Info field,
			=> the RA field is set to the broadcast address.
	ta: TA
		The address of the STA transmitting the VHT NDPA frame.

	token: Sounding Dialog Token, refer to "SNDING_DIALOG_TOKEN"

	sta_info: STA Info 1, ..., STA Info n, refer to "SNDING_STA_INFO"
		The VHT NDPA frame contains at least one STA Info field.

*/
typedef struct GNU_PACKED _VHT_NDPA_FRAME {
	FRAME_CONTROL fc;
	USHORT duration;
	UCHAR ra[MAC_ADDR_LEN];
	UCHAR ta[MAC_ADDR_LEN];
	SNDING_DIALOG_TOKEN token;
	SNDING_STA_INFO sta_info[0];
} VHT_NDPA_FRAME;

typedef struct GNU_PACKED _NDPA_PKT {
	USHORT frm_ctrl;
	USHORT duration;
	UINT8 ra[MAC_ADDR_LEN];
	UINT8 ta[MAC_ADDR_LEN];
	UINT8 snd_seq;
} DNPA_PKT;

typedef struct GNU_PACKED _PLCP_SERVICE_FIELD {
#ifdef RT_BIG_ENDIAN
	UINT8 rsv7:1;
	UINT8 cbw_in_non_ht:2;
	UINT8 dyn_bw:1;
	UINT8 rsv03:4;
#else
	UINT8 rsv03:4;
	UINT8 dyn_bw:1;
	UINT8 cbw_in_non_ht:2;
	UINT8 rsv7:1;
#endif /* RT_BIG_ENDIAN */
} PLCP_SERVICE_FIELD;

#endif /* __DOT11AC_VHT_H */

#endif /* DOT11_VHT_AC */

