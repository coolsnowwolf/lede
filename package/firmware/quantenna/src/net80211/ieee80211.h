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
 */
#ifndef _NET80211_IEEE80211_H_
#define _NET80211_IEEE80211_H_
#include <compat.h>
#include "net80211/_ieee80211.h"
#include "net80211/ieee80211_qos.h"
#include "net80211/ieee80211_dfs_reentry.h"
#include <qtn/qtn_config.h>

/*
 * 802.11 protocol definitions.
 */

#define	IEEE80211_ADDR_LEN	6		/* size of 802.11 address */
/* is 802.11 address multicast/broadcast? */
#define	IEEE80211_IS_MULTICAST(_a)	(*(_a) & 0x01)

/* IEEE 802.11 PLCP header */
struct ieee80211_plcp_hdr {
	uint16_t	i_sfd;
	uint8_t	i_signal;
	uint8_t	i_service;
	uint16_t	i_length;
	uint16_t	i_crc;
} __packed;

#define IEEE80211_PLCP_SFD	0xF3A0
#define IEEE80211_PLCP_SERVICE  0x00

/*
 * generic definitions for IEEE 802.11 frames
 */
struct ieee80211_frame {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_addr3[IEEE80211_ADDR_LEN];
	uint8_t i_seq[2];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
	/* see below */
} __packed;

struct ieee80211_qosframe {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_addr3[IEEE80211_ADDR_LEN];
	uint8_t i_seq[2];
	uint8_t i_qos[2];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
	/* see below */
} __packed;

struct ieee80211_htframe {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_addr1[IEEE80211_ADDR_LEN];
	uint8_t	i_addr2[IEEE80211_ADDR_LEN];
	uint8_t	i_addr3[IEEE80211_ADDR_LEN];
	uint8_t	i_seq[2];
	uint8_t	i_qos[2];
	uint8_t	i_ht[4];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
	/* see below */
} __packed;

struct ieee80211_qoscntl {
	uint8_t i_qos[2];
};

struct ieee80211_ht_qosframe {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_addr1[IEEE80211_ADDR_LEN];
	uint8_t	i_addr2[IEEE80211_ADDR_LEN];
	uint8_t	i_addr3[IEEE80211_ADDR_LEN];
	uint8_t	i_seq[2];
	uint8_t	i_qos[2];
	uint8_t	i_ht[4];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
	/* see below */
} __packed;

struct ieee80211_htcntl {
	uint8_t	i_ht[4];
};

struct ieee80211_frame_addr4 {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_addr3[IEEE80211_ADDR_LEN];
	uint8_t i_seq[2];
	uint8_t i_addr4[IEEE80211_ADDR_LEN];
} __packed;


struct ieee80211_qosframe_addr4 {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_addr3[IEEE80211_ADDR_LEN];
	uint8_t i_seq[2];
	uint8_t i_addr4[IEEE80211_ADDR_LEN];
	uint8_t i_qos[2];
} __packed;

#define IEEE80211_HT_CAPABLE		1
#define IEEE80211_NON_HT_CAPABLE	0

struct ieee80211_htframe_addr4 {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_addr1[IEEE80211_ADDR_LEN];
	uint8_t	i_addr2[IEEE80211_ADDR_LEN];
	uint8_t	i_addr3[IEEE80211_ADDR_LEN];
	uint8_t	i_seq[2];
	uint8_t	i_addr4[IEEE80211_ADDR_LEN];
	uint8_t	i_ht[4];
} __packed;

struct ieee80211_ht_qosframe_addr4 {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_addr1[IEEE80211_ADDR_LEN];
	uint8_t	i_addr2[IEEE80211_ADDR_LEN];
	uint8_t	i_addr3[IEEE80211_ADDR_LEN];
	uint8_t	i_seq[2];
	uint8_t	i_addr4[IEEE80211_ADDR_LEN];
	uint8_t	i_qos[2];
	uint8_t	i_ht[4];
} __packed;

#define IEEE80211_IS_4ADDRESS(__wh)	\
		(((__wh)->i_fc[1] & IEEE80211_FC1_DIR_MASK) == IEEE80211_FC1_DIR_DSTODS)

struct ieee80211_ctlframe_addr2 {
	uint8_t i_fc[2];
	__le16 i_aidordur; /* AID or duration */
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
} __packed;

struct ieee80211_vht_su_ndpa {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_diagtoken;
	uint8_t i_sta1_info[2];
} __packed;

struct ieee80211_bar_frame {
        u_int8_t i_fc[2];
        u_int8_t i_dur[2];
        u_int8_t i_addr1[IEEE80211_ADDR_LEN];
        u_int8_t i_addr2[IEEE80211_ADDR_LEN];
        u_int8_t i_bar_ctl[2];
        __le16   i_back_seq;
} __packed;

struct ieee80211_vht_mu_ndpa {
	uint8_t i_fc[2];
	__le16 i_dur;
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_diagtoken;
	uint8_t data[0];
} __packed;

struct ieee80211_vht_mu_rpt_poll {
	uint8_t i_fc[2];
	__le16 i_dur;
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_fbseg_map;
} __packed;

#define	IEEE80211_FC0_VERSION_MASK		0x03
#define	IEEE80211_FC0_VERSION_SHIFT		0
#define	IEEE80211_FC0_VERSION_0			0x00
#define	IEEE80211_FC0_TYPE_MASK			0x0c
#define	IEEE80211_FC0_TYPE_SHIFT		2
#define	IEEE80211_FC0_TYPE_MGT			0x00
#define	IEEE80211_FC0_TYPE_CTL			0x04
#define	IEEE80211_FC0_TYPE_DATA			0x08

#define	IEEE80211_FC0_SUBTYPE_MASK		0xf0
#define	IEEE80211_FC0_SUBTYPE_SHIFT		4
/* for TYPE_MGT */
#define	IEEE80211_FC0_SUBTYPE_ASSOC_REQ		0x00
#define	IEEE80211_FC0_SUBTYPE_ASSOC_RESP	0x10
#define	IEEE80211_FC0_SUBTYPE_REASSOC_REQ	0x20
#define	IEEE80211_FC0_SUBTYPE_REASSOC_RESP	0x30
#define	IEEE80211_FC0_SUBTYPE_PROBE_REQ		0x40
#define	IEEE80211_FC0_SUBTYPE_PROBE_RESP	0x50
#define	IEEE80211_FC0_SUBTYPE_BEACON		0x80
#define	IEEE80211_FC0_SUBTYPE_ATIM		0x90
#define	IEEE80211_FC0_SUBTYPE_DISASSOC		0xa0
#define	IEEE80211_FC0_SUBTYPE_AUTH		0xb0
#define	IEEE80211_FC0_SUBTYPE_DEAUTH		0xc0
#define IEEE80211_FC0_SUBTYPE_ACTION		0xd0
#define IEEE80211_FC0_SUBTYPE_ACTION_NOACK	0xe0
/* for TYPE_CTL */
#define	IEEE80211_FC0_SUBTYPE_VHT_RPT_POLL      0x40
#define	IEEE80211_FC0_SUBTYPE_VHT_NDPA		0x50
#define	IEEE80211_FC0_SUBTYPE_BAR		0x80
#define	IEEE80211_FC0_SUBTYPE_BA		0x90
#define	IEEE80211_FC0_SUBTYPE_PS_POLL		0xa0
#define	IEEE80211_FC0_SUBTYPE_RTS		0xb0
#define	IEEE80211_FC0_SUBTYPE_CTS		0xc0
#define	IEEE80211_FC0_SUBTYPE_ACK		0xd0
#define	IEEE80211_FC0_SUBTYPE_CF_END		0xe0
#define	IEEE80211_FC0_SUBTYPE_CF_END_ACK	0xf0
/* for TYPE_DATA (bit combination) */
#define	IEEE80211_FC0_SUBTYPE_DATA		0x00
#define	IEEE80211_FC0_SUBTYPE_CF_ACK		0x10
#define	IEEE80211_FC0_SUBTYPE_CF_POLL		0x20
#define	IEEE80211_FC0_SUBTYPE_CF_ACPL		0x30
#define	IEEE80211_FC0_SUBTYPE_NODATA		0x40
#define	IEEE80211_FC0_SUBTYPE_CFACK		0x50
#define	IEEE80211_FC0_SUBTYPE_CFPOLL		0x60
#define	IEEE80211_FC0_SUBTYPE_CF_ACK_CF_ACK	0x70
#define	IEEE80211_FC0_SUBTYPE_QOS		0x80
#define	IEEE80211_FC0_SUBTYPE_QOS_NULL		0xc0

#define	IEEE80211_FC1_DIR_MASK			0x03
#define	IEEE80211_FC1_DIR_NODS			0x00	/* STA->STA */
#define	IEEE80211_FC1_DIR_TODS			0x01	/* STA->AP  */
#define	IEEE80211_FC1_DIR_FROMDS		0x02	/* AP ->STA */
#define	IEEE80211_FC1_DIR_DSTODS		0x03	/* AP ->AP  */

#define	IEEE80211_FC1_MORE_FRAG			0x04
#define	IEEE80211_FC1_RETRY			0x08
#define	IEEE80211_FC1_PWR_MGT			0x10
#define	IEEE80211_FC1_MORE_DATA			0x20
#define	IEEE80211_FC1_PROT			0x40
#define	IEEE80211_FC1_WEP			0x40
#define	IEEE80211_FC1_ORDER			0x80

#define	IEEE80211_SEQ_FRAG_MASK			0x000f
#define	IEEE80211_SEQ_FRAG_SHIFT		0
#define	IEEE80211_SEQ_SEQ_MASK			0xfff0
#define	IEEE80211_SEQ_SEQ_SHIFT			4
#define IEEE80211_SEQ_RANGE			4096
#define IEEE80211_SEQ_ORDERLAG			64

#define IEEE80211_MU_NDPA_TOKEN_MASK		0xFC
#define IEEE80211_MU_NDPA_TOKEN_SHIFT		2
#define IEEE80211_MU_NDPA_RSRV_MASK		0x03
#define IEEE80211_MU_NDPA_RSRV_SHIFT		0

#define IEEE80211_SEQ_ADD(seq, offset)  \
	(((seq) + (offset)) & (IEEE80211_SEQ_RANGE - 1))
#define IEEE80211_SEQ_SUB(seq, offset)					\
	(((seq) + IEEE80211_SEQ_RANGE - (offset)) & (IEEE80211_SEQ_RANGE - 1))
#define IEEE80211_SEQ_DIFF(seq_front, seq_back)				\
	(((seq_front) + IEEE80211_SEQ_RANGE - (seq_back)) & (IEEE80211_SEQ_RANGE - 1))
#define IEEE80211_SEQ_INORDER_LAG(seq_front, seq_back, seq_lag)		\
	(IEEE80211_SEQ_DIFF((seq_front), (seq_back)) < (seq_lag))
#define IEEE80211_SEQ_INORDER(seq_front, seq_back)			\
	IEEE80211_SEQ_INORDER_LAG((seq_front), (seq_back), IEEE80211_SEQ_ORDERLAG)

#define	IEEE80211_SEQ_LEQ(a,b)	((int)((a)-(b)) <= 0)
#define	IEEE80211_SEQ_EQ(a,b)	((a) == (b))

#define	IEEE80211_NWID_LEN			32

#define	IEEE80211_QOS_TXOP			0x00ff
/* bit 8 is reserved */
#define	IEEE80211_QOS_ACKPOLICY			0x60
#define	IEEE80211_QOS_ACKPOLICY_S		5
#define	IEEE80211_QOS_EOSP			0x10
#define	IEEE80211_QOS_EOSP_S			4
#define	IEEE80211_QOS_TID			0x0f
#define IEEE80211_QOS_A_MSDU_PRESENT 		0x80
#define IEEE80211_QOS_BLOCK_ACK_POLICY          0x60

/* bit 1 is reserved */
#define IEEE80211_HTC0_TRQ				0x02
#define IEEE80211_HTC0_MAI_MASK			0x3C
#define IEEE80211_HTC0_MAI_SHIFT		2
#define IEEE80211_HTC0_MFSI_LOW_MASK	0xC0
#define IEEE80211_HTC0_MFSI_LOW_SHIFT	6

#define IEEE80211_HTC1_MFSI_HIGH		0x01
#define IEEE80211_HTC1_MFB_ASEL_MASK	0xFE
#define IEEE80211_HTC1_MFB_ASEL_SHIFT	1

#define IEEE80211_HTC2_CALIB_POS_MASK	0x03
#define IEEE80211_HTC2_CALIB_POS_SHIFT	0
#define IEEE80211_HTC2_CALIB_SEQ_MASK	0x0C
#define IEEE80211_HTC2_CALIB_SEQ_SHIFT	2
/* bits 4-5 are reserved */
#define IEEE80211_HTC2_CSI_STEER_MASK	0xC0
#define IEEE80211_HTC2_CSI_STEER_SHIFT	6

#define IEEE80211_HTC3_NDP_ANNOUNCE		0x01
/* bits 1-5 are reserved */
#define IEEE80211_HTC3_AC_CONSTRAINT	0x40
#define IEEE80211_HTC3_MORE_PPDU_RDG	0x80






/*
 * Country/Region Codes from MS WINNLS.H
 * Numbering from ISO 3166
 * XXX belongs elsewhere
 *
 * First 2 entries taken from ieee80211.c ...
 */
enum CountryCode {
    CTRY_DEBUG                = 0x1ff,   /* debug, = 511 radix 10 */
    CTRY_DEFAULT              = 0,       /* default or not defined */

    CTRY_AFGHANISTAN          = 4,       /* Afghanistan */
    CTRY_ALBANIA              = 8,       /* Albania */
    CTRY_ALGERIA              = 12,      /* Algeria */
    CTRY_AMERICAN_SAMOA	      = 16,      /* American Samoa */
    CTRY_ANDORRA              = 20,      /* Andorra */
    CTRY_ANGOLA               = 24,      /* Angola */
    CTRY_ANGUILLA             = 660,
    CTRY_ANTARTICA            = 10,      /* Antartica */
    CTRY_ANTIGUA              = 28,	     /* Antigua and Barbuda */
    CTRY_ARGENTINA            = 32,      /* Argentina */
    CTRY_ARMENIA              = 51,      /* Armenia */
    CTRY_ARUBA                = 533,	 /* Aruba */
    CTRY_AUSTRALIA            = 36,      /* Australia */
    CTRY_AUSTRIA              = 40,      /* Austria */
    CTRY_AZERBAIJAN           = 31,      /* Azerbaijan */
    CTRY_BAHAMAS              = 44,	     /* Bahamas */
    CTRY_BAHRAIN              = 48,      /* Bahrain */
    CTRY_BANGLADESH           = 50,	     /* Bangladesh */
    CTRY_BARBADOS             = 52,
    CTRY_BELARUS              = 112,     /* Belarus */
    CTRY_BELGIUM              = 56,      /* Belgium */
    CTRY_BELIZE               = 84,      /* Belize */
    CTRY_BENIN                = 204,
    CTRY_BERMUDA              = 60,
    CTRY_BHUTAN               = 64,
    CTRY_BOLIVIA              = 68,      /* Bolivia */
    CTRY_BOSNIA_AND_HERZEGOWINA = 70,
    CTRY_BOTSWANA             = 72,
    CTRY_BOUVET_ISLAND	      = 74,
    CTRY_BRAZIL               = 76,      /* Brazil */
    CTRY_BRITISH_INDIAN_OCEAN_TERRITORY = 86,
    CTRY_BRUNEI_DARUSSALAM    = 96,      /* Brunei Darussalam */
    CTRY_BULGARIA             = 100,     /* Bulgaria */
    CTRY_BURKINA_FASO         = 854,
    CTRY_BURUNDI              = 108,
    CTRY_CAMBODIA             = 116,
    CTRY_CAMEROON             = 120,
    CTRY_CANADA               = 124,     /* Canada */
    CTRY_CAPE_VERDE           = 132,
    CTRY_CAYMAN_ISLANDS	      = 136,
    CTRY_CENTRAL_AFRICAN_REPUBLIC = 140,
    CTRY_CHAD                 = 148,
    CTRY_CHILE                = 152,     /* Chile */
    CTRY_CHINA                = 156,     /* People's Republic of China */
    CTRY_CHRISTMAS_ISLAND     = 162,
    CTRY_COCOS_ISLANDS	      = 166,
    CTRY_COLOMBIA             = 170,     /* Colombia */
    CTRY_COMOROS              = 174,
    CTRY_CONGO                = 178,
    CTRY_COOK_ISLANDS	      = 184,
    CTRY_COSTA_RICA           = 188,     /* Costa Rica */
    CTRY_COTE_DIVOIRE	      = 384,
    CTRY_CROATIA              = 191,     /* Croatia */
    CTRY_CYPRUS               = 196,
    CTRY_CZECH                = 203,     /* Czech Republic */
    CTRY_DENMARK              = 208,     /* Denmark */
    CTRY_DJIBOUTI             = 262,
    CTRY_DOMINICA             = 212,
    CTRY_DOMINICAN_REPUBLIC   = 214,     /* Dominican Republic */
    CTRY_ECUADOR              = 218,     /* Ecuador */
    CTRY_EUROPE               = 200,     /* European Union */
    CTRY_EGYPT                = 818,     /* Egypt */
    CTRY_EL_SALVADOR          = 222,     /* El Salvador */
    CTRY_EQUATORIAL_GUINEA    = 226,
    CTRY_ERITREA              = 232,
    CTRY_ESTONIA              = 233,     /* Estonia */
    CTRY_ETHIOPIA             = 210,
    CTRY_FALKLAND_ISLANDS     = 238,	 /* (Malvinas) */
    CTRY_FAEROE_ISLANDS       = 234,     /* Faeroe Islands */
    CTRY_FIJI                 = 242,
    CTRY_FINLAND              = 246,     /* Finland */
    CTRY_FRANCE               = 250,     /* France */
    CTRY_FRANCE2              = 255,     /* France2 */
    CTRY_FRENCH_GUIANA	      = 254,
    CTRY_FRENCH_POLYNESIA     = 258,
    CTRY_FRENCH_SOUTHERN_TERRITORIES	= 260,
    CTRY_GABON                = 266,
    CTRY_GAMBIA               = 270,
    CTRY_GEORGIA              = 268,     /* Georgia */
    CTRY_GERMANY              = 276,     /* Germany */
    CTRY_GHANA                = 288,
    CTRY_GIBRALTAR            = 292,
    CTRY_GREECE               = 300,     /* Greece */
    CTRY_GREENLAND            = 304,
    CTRY_GRENADA              = 308,
    CTRY_GUADELOUPE           = 312,
    CTRY_GUAM                 = 316,
    CTRY_GUATEMALA            = 320,     /* Guatemala */
    CTRY_GUINEA               = 324,
    CTRY_GUINEA_BISSAU	      = 624,
    CTRY_GUYANA               = 328,
    CTRY_HAITI                = 332,
    CTRY_HONDURAS             = 340,     /* Honduras */
    CTRY_HONG_KONG            = 344,     /* Hong Kong S.A.R., P.R.C. */
    CTRY_HUNGARY              = 348,     /* Hungary */
    CTRY_ICELAND              = 352,     /* Iceland */
    CTRY_INDIA                = 356,     /* India */
    CTRY_INDONESIA            = 360,     /* Indonesia */
    CTRY_IRAN                 = 364,     /* Iran */
    CTRY_IRAQ                 = 368,     /* Iraq */
    CTRY_IRELAND              = 372,     /* Ireland */
    CTRY_ISRAEL               = 376,     /* Israel */
    CTRY_ITALY                = 380,     /* Italy */
    CTRY_JAMAICA              = 388,     /* Jamaica */
    CTRY_JAPAN                = 392,     /* Japan */
    CTRY_JAPAN1               = 393,     /* Japan (JP1) */
    CTRY_JAPAN2               = 394,     /* Japan (JP0) */
    CTRY_JAPAN3               = 395,     /* Japan (JP1-1) */
    CTRY_JAPAN4               = 396,     /* Japan (JE1) */
    CTRY_JAPAN5               = 397,     /* Japan (JE2) */
    CTRY_JAPAN6               = 399,	 /* Japan (JP6) */
    CTRY_JAPAN7               = 900,	 /* Japan */
    CTRY_JAPAN8               = 901,	 /* Japan */
    CTRY_JAPAN9               = 902,	 /* Japan */
    CTRY_JAPAN10	      = 903,	 /* Japan */
    CTRY_JAPAN11	      = 904,	 /* Japan */
    CTRY_JAPAN12	      = 905,	 /* Japan */
    CTRY_JAPAN13	      = 906,	 /* Japan */
    CTRY_JAPAN14	      = 907,	 /* Japan */
    CTRY_JAPAN15	      = 908,	 /* Japan */
    CTRY_JAPAN16	      = 909,	 /* Japan */
    CTRY_JAPAN17	      = 910,	 /* Japan */
    CTRY_JAPAN18	      = 911,	 /* Japan */
    CTRY_JAPAN19	      = 912,	 /* Japan */
    CTRY_JAPAN20	      = 913,	 /* Japan */
    CTRY_JAPAN21	      = 914,	 /* Japan */
    CTRY_JAPAN22	      = 915,	 /* Japan */
    CTRY_JAPAN23	      = 916,	 /* Japan */
    CTRY_JAPAN24	      = 917,	 /* Japan */
    CTRY_JAPAN25	      = 918,	 /* Japan */
    CTRY_JAPAN26	      = 919,	 /* Japan */
    CTRY_JAPAN27	      = 920,	 /* Japan */
    CTRY_JAPAN28	      = 921,	 /* Japan */
    CTRY_JAPAN29	      = 922,	 /* Japan */
    CTRY_JAPAN30	      = 923,	 /* Japan */
    CTRY_JAPAN31	      = 924,	 /* Japan */
    CTRY_JAPAN32	      = 925,	 /* Japan */
    CTRY_JAPAN33	      = 926,	 /* Japan */
    CTRY_JAPAN34	      = 927,	 /* Japan */
    CTRY_JAPAN35	      = 928,	 /* Japan */
    CTRY_JAPAN36	      = 929,	 /* Japan */
    CTRY_JAPAN37	      = 930,	 /* Japan */
    CTRY_JAPAN38	      = 931,	 /* Japan */
    CTRY_JAPAN39	      = 932,	 /* Japan */
    CTRY_JAPAN40	      = 933,	 /* Japan */
    CTRY_JAPAN41	      = 934,	 /* Japan */
    CTRY_JAPAN42	      = 935,	 /* Japan */
    CTRY_JAPAN43	      = 936,	 /* Japan */
    CTRY_JAPAN44	      = 937,	 /* Japan */
    CTRY_JAPAN45	      = 938,	 /* Japan */
    CTRY_JAPAN46	      = 939,	 /* Japan */
    CTRY_JAPAN47	      = 940,	 /* Japan */
    CTRY_JAPAN48	      = 941,	 /* Japan */
    CTRY_JORDAN               = 400,     /* Jordan */
    CTRY_KAZAKHSTAN           = 398,     /* Kazakhstan */
    CTRY_KENYA                = 404,     /* Kenya */
    CTRY_KOREA_NORTH          = 408,     /* North Korea */
    CTRY_KOREA_ROC            = 410,     /* South Korea */
    CTRY_KOREA_ROC2           = 411,     /* South Korea */
    CTRY_KUWAIT               = 414,     /* Kuwait */
    CTRY_LATVIA               = 428,     /* Latvia */
    CTRY_LEBANON              = 422,     /* Lebanon */
    CTRY_LIBYA                = 434,     /* Libya */
    CTRY_LIECHTENSTEIN        = 438,     /* Liechtenstein */
    CTRY_LITHUANIA            = 440,     /* Lithuania */
    CTRY_LUXEMBOURG           = 442,     /* Luxembourg */
    CTRY_MACAU                = 446,     /* Macau */
    CTRY_MACEDONIA            = 807,     /* the Former Yugoslav Republic of Macedonia */
    CTRY_MALAYSIA             = 458,     /* Malaysia */
    CTRY_MEXICO               = 484,     /* Mexico */
    CTRY_MONACO               = 492,     /* Principality of Monaco */
    CTRY_MOROCCO              = 504,     /* Morocco */
    CTRY_NEPAL                = 524,	 /* Nepal */
    CTRY_NETHERLANDS          = 528,     /* Netherlands */
    CTRY_NEW_ZEALAND          = 554,     /* New Zealand */
    CTRY_NICARAGUA            = 558,     /* Nicaragua */
    CTRY_NORWAY               = 578,     /* Norway */
    CTRY_OMAN                 = 512,     /* Oman */
    CTRY_PAKISTAN             = 586,     /* Islamic Republic of Pakistan */
    CTRY_PANAMA               = 591,     /* Panama */
    CTRY_PARAGUAY             = 600,     /* Paraguay */
    CTRY_PERU                 = 604,     /* Peru */
    CTRY_PHILIPPINES          = 608,     /* Republic of the Philippines */
    CTRY_POLAND               = 616,     /* Poland */
    CTRY_PORTUGAL             = 620,     /* Portugal */
    CTRY_PUERTO_RICO          = 630,     /* Puerto Rico */
    CTRY_QATAR                = 634,     /* Qatar */
    CTRY_ROMANIA              = 642,     /* Romania */
    CTRY_RUSSIA               = 643,     /* Russia */
    CTRY_SAUDI_ARABIA         = 682,     /* Saudi Arabia */
    CTRY_SINGAPORE            = 702,     /* Singapore */
    CTRY_SLOVAKIA             = 703,     /* Slovak Republic */
    CTRY_SLOVENIA             = 705,     /* Slovenia */
    CTRY_SOUTH_AFRICA         = 710,     /* South Africa */
    CTRY_SPAIN                = 724,     /* Spain */
    CTRY_SRILANKA             = 144,	 /* Sri Lanka */
    CTRY_SWEDEN               = 752,     /* Sweden */
    CTRY_SWITZERLAND          = 756,     /* Switzerland */
    CTRY_SYRIA                = 760,     /* Syria */
    CTRY_TAIWAN               = 158,     /* Taiwan */
    CTRY_THAILAND             = 764,     /* Thailand */
    CTRY_TRINIDAD_Y_TOBAGO    = 780,     /* Trinidad y Tobago */
    CTRY_TUNISIA              = 788,     /* Tunisia */
    CTRY_TURKEY               = 792,     /* Turkey */
    CTRY_UAE                  = 784,     /* U.A.E. */
    CTRY_UKRAINE              = 804,     /* Ukraine */
    CTRY_UNITED_KINGDOM       = 826,     /* United Kingdom */
    CTRY_UNITED_STATES        = 840,     /* United States */
    CTRY_UNITED_STATES_FCC49  = 842,     /* United States (Public Safety)*/
    CTRY_URUGUAY              = 858,     /* Uruguay */
    CTRY_UZBEKISTAN           = 860,     /* Uzbekistan */
    CTRY_VENEZUELA            = 862,     /* Venezuela */
    CTRY_VIET_NAM             = 704,     /* Viet Nam */
    CTRY_YEMEN                = 887,     /* Yemen */
    CTRY_ZIMBABWE             = 716      /* Zimbabwe */
};

#define IEEE80211_IE_ID_LEN_SIZE 2

/*
 * Generic information element
 */
struct ieee80211_ie {
	uint8_t id;
	uint8_t len;
	uint8_t info[0];
} __packed;

/*
 * Country information element.
 */
#define IEEE80211_COUNTRY_MAX_TRIPLETS (83)
struct ieee80211_ie_country {
	uint8_t country_id;
	uint8_t country_len;
	uint8_t country_str[3];
	uint8_t country_triplet[IEEE80211_COUNTRY_MAX_TRIPLETS * 3];
} __packed;

/*
 * Channel Switch Announcement information element.
 */
struct ieee80211_ie_csa {
	uint8_t csa_id;	/* IEEE80211_ELEMID_CHANSWITCHANN */
	uint8_t csa_len;	/* == 3 */
	uint8_t csa_mode;	/* Channel Switch Mode: 1 == stop transmission until CS */
	uint8_t csa_chan;	/* New Channel Number */
	uint8_t csa_count;	/* TBTTs until Channel Switch happens */
} __packed;

/* for Spectrum Management Actions. Table 20e in 802.11h $7.4.1 */
#define IEEE80211_ACTION_S_MEASUREMENT_REQUEST 0
#define IEEE80211_ACTION_S_MEASUREMENT_REPORT  1
#define IEEE80211_ACTION_S_TPC_REQUEST         2
#define IEEE80211_ACTION_S_TPC_REPORT          3
#define IEEE80211_ACTION_S_CHANSWITCHANN       4

/* for csa_mode. It must be either 0 or 1. 1 means that the receiver shall stop
 * sending until CS. 0 imposes no requirement. See 7.3.2.20 */
#define IEEE80211_CSA_CAN_STOP_TX       0
#define IEEE80211_CSA_MUST_STOP_TX      1

/* minimal Channel Switch Count in the initial announcement */
#define IEEE80211_CSA_PROTECTION_PERIOD 3

/* maximum allowed deviance of measurement of intervals between CSA in Beacons */
#define IEEE80211_CSA_SANITY_THRESHOLD 100

/* Quantenna CSA tsf ie, to complement an 802.11h CSA ie. More timing precision */
struct ieee80211_ie_qtn_csa_tsf {
	uint8_t id;	/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;   /* length in bytes */
	uint8_t qtn_ie_oui[3];		/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t qtn_ie_type;		/* IE type */
	uint64_t tsf;			/* TSF at which channel change happens. */
} __packed;

/* Quantenna SCS IE */
#define QTN_SCS_IE_TYPE_STA_INTF_RPT        0x1
struct ieee80211_ie_qtn_scs {
	uint8_t id;			/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;                    /* length in bytes */
	uint8_t qtn_ie_oui[3];		/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t qtn_ie_type;		/* IE type */
	uint8_t scs_ie_type;            /* for future expansion and backward compatibility */
	/* following depends on scs_ie_type */
	uint32_t sp_fail;		/* short preamble failure in last second */
	uint32_t lp_fail;		/* long preamble failure in last second */
	uint16_t others_time;		/* rx + tx time for all nodes */
	uint16_t extra_ie_len;		/* extra ie len */
	uint8_t extra_ie[0];		/* tdls stats */
} __packed;
#define QTN_SCS_IE_LEN_MIN              7    /* till scs ie type */
#define QTN_SCS_IE_STA_INTF_RPT_LEN_MIN     (QTN_SCS_IE_LEN_MIN + 8)

#define IEEE80211_IS_ALL_SET(__flags__, __msb__)	\
	(((__flags__) & ((1 << ((__msb__)+1)) - 1)) == ((1 << ((__msb__)+1)) - 1))

/* does frame have QoS sequence control data */
#define	IEEE80211_QOS_HAS_SEQ(wh) \
	(((wh)->i_fc[0] & \
	  (IEEE80211_FC0_TYPE_MASK | IEEE80211_FC0_SUBTYPE_QOS)) == \
	  (IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS))

#define WME_QOSINFO_COUNT	0x0f  /* Mask for Param Set Count field */
#define	WMM_OUI_BYTES		0x00, 0x50, 0xf2
/*
 * WME/802.11e information element.
 */
struct ieee80211_ie_wme {
	uint8_t wme_id;		/* IEEE80211_ELEMID_VENDOR */
	uint8_t wme_len;	/* length in bytes */
	uint8_t wme_oui[3];	/* 0x00, 0x50, 0xf2 */
	uint8_t wme_type;	/* OUI type */
	uint8_t wme_subtype;	/* OUI subtype */
	uint8_t wme_version;	/* spec revision */
	uint8_t wme_info;	/* QoS info */
} __packed;

/*
 * WME/802.11e Tspec Element
 */
struct ieee80211_wme_tspec {
	uint8_t ts_id;
	uint8_t ts_len;
	uint8_t ts_oui[3];
	uint8_t ts_oui_type;
	uint8_t ts_oui_subtype;
	uint8_t ts_version;
	uint8_t ts_tsinfo[3];
	uint8_t ts_nom_msdu[2];
	uint8_t ts_max_msdu[2];
	uint8_t ts_min_svc[4];
	uint8_t ts_max_svc[4];
	uint8_t ts_inactv_intv[4];
	uint8_t ts_susp_intv[4];
	uint8_t ts_start_svc[4];
	uint8_t ts_min_rate[4];
	uint8_t ts_mean_rate[4];
	uint8_t ts_max_burst[4];
	uint8_t ts_min_phy[4];
	uint8_t ts_peak_rate[4];
	uint8_t ts_delay[4];
	uint8_t ts_surplus[2];
	uint8_t ts_medium_time[2];
} __packed;

/*
 * WME AC parameter field
 */

struct ieee80211_wme_acparams {
	uint8_t acp_aci_aifsn;
	uint8_t acp_logcwminmax;
	uint16_t acp_txop;
} __packed;

#define IEEE80211_WME_PARAM_LEN	24
#define WME_NUM_TID		16	/* 16 tids */
#define WME_NUM_AC		4	/* 4 AC categories */
#define WME_TID_UNKNOWN		(-1)
#define WME_TID_NONQOS		(-2)
#define WME_TID_VALID(_tid)	(((_tid) >= 0) && ((_tid) < WME_NUM_TID))

#define WME_PARAM_ACI		0x60	/* Mask for ACI field */
#define WME_PARAM_ACI_S		5	/* Shift for ACI field */
#define WME_PARAM_ACM		0x10	/* Mask for ACM bit */
#define WME_PARAM_ACM_S		4	/* Shift for ACM bit */
#define WME_PARAM_AIFSN		0x0f	/* Mask for aifsn field */
#define WME_PARAM_AIFSN_S	0	/* Shift for aifsn field */
#define WME_PARAM_LOGCWMIN	0x0f	/* Mask for CwMin field (in log) */
#define WME_PARAM_LOGCWMIN_S	0	/* Shift for CwMin field */
#define WME_PARAM_LOGCWMAX	0xf0	/* Mask for CwMax field (in log) */
#define WME_PARAM_LOGCWMAX_S	4	/* Shift for CwMax field */

#define WME_AC_TO_TID(_ac) (       \
	((_ac) == WME_AC_VO) ? 6 : \
	((_ac) == WME_AC_VI) ? 5 : \
	((_ac) == WME_AC_BK) ? 1 : \
	0)

#define TID_TO_WME_AC(_tid)				\
	((((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE :	\
	 ((_tid) < 3) ? WME_AC_BK :	\
	 ((_tid) < 6) ? WME_AC_VI :	\
	 WME_AC_VO)

/*
 * WME Parameter Element
 */
struct ieee80211_wme_param {
	uint8_t param_id;
	uint8_t param_len;
	uint8_t param_oui[3];
	uint8_t param_oui_type;
	uint8_t param_oui_sybtype;
	uint8_t param_version;
	uint8_t param_qosInfo;
	uint8_t param_reserved;
	struct ieee80211_wme_acparams	params_acParams[WME_NUM_AC];
} __packed;

/*
 * WME U-APSD qos info field defines
 */
#define WME_CAPINFO_UAPSD_EN			0x00000080
#define WME_CAPINFO_UAPSD_VO			0x00000001
#define WME_CAPINFO_UAPSD_VI			0x00000002
#define WME_CAPINFO_UAPSD_BK			0x00000004
#define WME_CAPINFO_UAPSD_BE			0x00000008
#define WME_CAPINFO_UAPSD_ACFLAGS_SHIFT		0
#define WME_CAPINFO_UAPSD_ACFLAGS_MASK		0xF
#define WME_CAPINFO_UAPSD_MAXSP_SHIFT		5
#define WME_CAPINFO_UAPSD_MAXSP_MASK		0x3
#define WME_CAPINFO_IE_OFFSET			8
#define WME_UAPSD_MAXSP(_qosinfo) (((_qosinfo) >> WME_CAPINFO_UAPSD_MAXSP_SHIFT) & WME_CAPINFO_UAPSD_MAXSP_MASK)
#define WME_UAPSD_AC_ENABLED(_ac, _qosinfo) ( (1<<(3 - (_ac))) &   \
		(((_qosinfo) >> WME_CAPINFO_UAPSD_ACFLAGS_SHIFT) & WME_CAPINFO_UAPSD_ACFLAGS_MASK) )

struct ieee80211_extcap_param {
	u_int8_t param_id;
	u_int8_t param_len;
	u_int8_t ext_cap[8];
} __packed;

/* byte 7 */
#define IEEE80211_EXTCAP_OPMODE_NOTIFICATION	0x40
#define IEEE80211_EXTCAP_MAX_MSDU_IN_AMSDU	0xC0
#define IEEE80211_EXTCAP_MAX_MSDU_IN_AMSDU_S	6

/*
 * Atheros Advanced Capability information element.
 */
struct ieee80211_ie_athAdvCap {
	uint8_t athAdvCap_id;		/* IEEE80211_ELEMID_VENDOR */
	uint8_t athAdvCap_len;		/* length in bytes */
	uint8_t athAdvCap_oui[3];	/* 0x00, 0x03, 0x7f */
	uint8_t athAdvCap_type;		/* OUI type */
	uint8_t athAdvCap_subtype;	/* OUI subtype */
	uint8_t athAdvCap_version;	/* spec revision */
	uint8_t athAdvCap_capability;	/* Capability info */
	uint16_t athAdvCap_defKeyIndex;
} __packed;

/*
 * Atheros XR information element.
 */
struct ieee80211_xr_param {
	uint8_t param_id;
	uint8_t param_len;
	uint8_t param_oui[3];
	uint8_t param_oui_type;
	uint8_t param_oui_sybtype;
	uint8_t param_version;
	uint8_t param_Info;
	uint8_t param_base_bssid[IEEE80211_ADDR_LEN];
	uint8_t param_xr_bssid[IEEE80211_ADDR_LEN];
	uint16_t param_xr_beacon_interval;
	uint8_t param_base_ath_capability;
	uint8_t param_xr_ath_capability;
} __packed;

/* Atheros capabilities */
#define IEEE80211_ATHC_TURBOP	0x0001		/* Turbo Prime */
#define IEEE80211_ATHC_COMP	0x0002		/* Compression */
#define IEEE80211_ATHC_FF	0x0004		/* Fast Frames */
#define IEEE80211_ATHC_XR	0x0008		/* Xtended Range support */
#define IEEE80211_ATHC_AR	0x0010		/* Advanced Radar support */
#define IEEE80211_ATHC_BURST	0x0020		/* Bursting - not negotiated */
#define IEEE80211_ATHC_WME	0x0040		/* CWMin tuning */
#define IEEE80211_ATHC_BOOST	0x0080		/* Boost */

/*
 * Quantenna Flags information element.
 * Fields up to qtn_ie_implicit_ba_tid are backwards-compatible with Envy images.
 */
struct ieee80211_ie_qtn {
	uint8_t qtn_ie_id;		/* IEEE80211_ELEMID_VENDOR */
	uint8_t qtn_ie_len;		/* length in bytes */
	uint8_t qtn_ie_oui[3];		/* QTN_OUI - 0x00, 0x26, 0x86 */
	uint8_t qtn_ie_type;		/* IE type */
	uint8_t qtn_ie_flags;		/* See below */

	/* V2 fields */
	uint8_t qtn_ie_implicit_ba_tid;/* Implicit block ACKs, set up directly after assoc */
	uint8_t qtn_ie_my_flags;	/* See below */

	/* V3 fields */
	/* Implicit block ACK with variable size - overrides v2 implicit BA field. */
	uint8_t qtn_ie_implicit_ba_tid_h;
	uint8_t qtn_ie_implicit_ba_size; /* Size of implicit BA >> 2 */

	/* V4 fields */
	uint8_t qtn_ie_vsp_version;	/* VSP version */

	/* V5 fields */
	uint32_t qtn_ie_ver_sw;
	uint16_t qtn_ie_ver_hw;
	uint16_t qtn_ie_ver_platform_id;
	uint32_t qtn_ie_ver_timestamp;
	uint32_t qtn_ie_ver_flags;
	uint32_t qtn_ie_rate_train;
} __packed;

#ifdef CONFIG_QVSP
/*
 * Quantenna WME information element.
 */
struct ieee80211_ie_qtn_wme {
	uint8_t qtn_ie_id;		/* IEEE80211_ELEMID_VENDOR */
	uint8_t qtn_ie_len;		/* length in bytes */
	uint8_t qtn_ie_oui[3];		/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t qtn_ie_type;		/* IE type */
	uint8_t qtn_wme_ie_version;
	struct ieee80211_wme_param qtn_wme_ie;
} __packed;
#endif

#define QTN_PAIRING_TLV_HASH_LEN 32
/*
 * QTN Pairing TLV element.
 *  Format:
 *   Type(1byte)   |   len(2bytes)    |  SHA-256 hash(32bytes)
 *      0x1        |        35        |     SHA-256 hash material of pairing
 */
struct ieee80211_ie_qtn_pairing_tlv {
	uint8_t qtn_pairing_tlv_type;
	uint16_t qtn_pairing_tlv_len;
	uint8_t qtn_pairing_tlv_hash[QTN_PAIRING_TLV_HASH_LEN];
} __packed;

/*
 * QTN Pairing IE
 *  Format:
 *  IE ID(1byte)    |     IE len(1byte)    |     IE OUI(3bytes)    | IE content(pairing)
 *     0xdd         |       38             |        00 26 86       |     Pairing TLV
 *
 */
struct ieee80211_ie_qtn_pairing {
	uint8_t qtn_pairing_ie_id;
	uint8_t qtn_pairing_ie_len;
	uint8_t qtn_pairing_ie_oui[3];
	struct ieee80211_ie_qtn_pairing_tlv qtn_pairing_tlv;
} __packed;

#define IEEE80211_QTN_IE_BA_SIZE_SH 2

enum ieee80211_vsp_version {
	IEEE80211_QTN_VSP_V_NONE,
	IEEE80211_QTN_VSP_V1,
};

#ifdef CONFIG_QVSP

#ifdef TOPAZ_PLATFORM
/* Disable Station side control for QTM-Lite */
#define IEEE80211_QTN_VSP_VERSION	IEEE80211_QTN_VSP_V_NONE
#else
#define IEEE80211_QTN_VSP_VERSION	IEEE80211_QTN_VSP_V1
#endif
struct ieee80211_ie_vsp_item {
	uint8_t	index;
	uint32_t	value;
} __packed;

struct ieee80211_ie_vsp {
	uint8_t	id;
	uint8_t	len;
	uint8_t	oui[3];
	uint8_t	type;
	uint8_t	item_cnt;
	struct ieee80211_ie_vsp_item item[0];
} __packed;

#else /* not CONFIG_QVSP */

#define IEEE80211_QTN_VSP_VERSION	IEEE80211_QTN_VSP_V_NONE

#endif /* CONFIG_QVSP */

#define IEEE80211_QTN_TYPE_ENVY_LEGACY(qtnie) \
	((qtnie)->qtn_ie_len <= (&(qtnie)->qtn_ie_my_flags - &(qtnie)->qtn_ie_oui[0]))
#define IEEE80211_QTN_TYPE_ENVY(qtnie) \
	((IEEE80211_QTN_TYPE_ENVY_LEGACY(qtnie)) || \
	 ((qtnie)->qtn_ie_my_flags & IEEE80211_QTN_ENVY))

#define IEEE80211_QTN_FLAGS_ENVY	(IEEE80211_QTN_BRIDGEMODE | IEEE80211_QTN_BF_VER1)
#define IEEE80211_QTN_FLAGS_ENVY_DFLT	IEEE80211_QTN_BF_VER1
#ifdef TXBF_6_STA_BF
#define IEEE80211_QTN_CAPS_DFLT		IEEE80211_QTN_BF_VER2 | IEEE80211_QTN_BF_VER3 | \
					IEEE80211_QTN_BF_VER4 | IEEE80211_QTN_TX_AMSDU
#else
#define IEEE80211_QTN_CAPS_DFLT		IEEE80211_QTN_BF_VER2 | IEEE80211_QTN_BF_VER3 | \
					IEEE80211_QTN_TX_AMSDU
#endif

/*
 * These flags are used in the following two fields.
 * - qtn_ie_flags contains the sender's settings, except in an association response, where
 *   it contains confirmation of the settings received from the peer station.  These flags
 *   must remain backwards-compatible with Envy images.
 * - qtn_ie_my_flags always contains the sender's settings.  It is not sent by Envy systems.
 */
#define IEEE80211_QTN_BRIDGEMODE	0x01		/* Use 4-addr headers */
#define IEEE80211_QTN_BF_VER1		0x02		/* Envy beamforming */
#define IEEE80211_QTN_BF_VER2		0x04		/* Ruby 2 stream beamforming */
#define IEEE80211_QTN_LNCB		0x08		/* Multicast packets in the local network
							 * control block are 4 address encapsulated.
							 */
#define IEEE80211_QTN_BF_VER3		0x10		/* Ruby 4 stream non-standard beamforming */
#define IEEE80211_QTN_ENVY		0x20		/* Envy with 'my flags' field in the IE. */
#define IEEE80211_QTN_BF_VER4		0x40		/* 4 strm standard bf with tone grouping */
#define IEEE80211_QTN_TX_AMSDU		0x80		/* Ruby TX AMSDU */

#define IEEE80211_QTN_IE_GE_V2(_qtnie)	((_qtnie->qtn_ie_len + IEEE80211_IE_ID_LEN_SIZE) >	\
					offsetof(struct ieee80211_ie_qtn, qtn_ie_my_flags))
#define IEEE80211_QTN_IE_GE_V3(_qtnie)	((_qtnie->qtn_ie_len + IEEE80211_IE_ID_LEN_SIZE) >	\
					offsetof(struct ieee80211_ie_qtn, qtn_ie_implicit_ba_size))
#define IEEE80211_QTN_IE_GE_V4(_qtnie)	((_qtnie->qtn_ie_len + IEEE80211_IE_ID_LEN_SIZE) >	\
					offsetof(struct ieee80211_ie_qtn, qtn_ie_vsp_version))
#define IEEE80211_QTN_IE_GE_V5(_qtnie)	((_qtnie->qtn_ie_len + IEEE80211_IE_ID_LEN_SIZE) >	\
					offsetof(struct ieee80211_ie_qtn, qtn_ie_rate_train))

/* Quantenna TDLS Discovery Response clients information element */
struct ieee80211_ie_qtn_tdls_clients {
	uint8_t        qtn_ie_id;              /* IEEE80211_ELEMID_VENDOR */
	uint8_t        qtn_ie_len;
	uint8_t        qtn_ie_oui[3];          /* 0x00, 0x26, 0x86 */
	uint8_t        qtn_ie_type;            /* IEEE_QTN_IE_TYPE_TDLS_CLIENTS */
	uint8_t        qtn_ie_mac_cnt;         /* Number of downstream MAC addresses */
#define IEEE80211_QTN_IE_DOWNSTREAM_MAC_MAX    16
	uint8_t        qtn_ie_mac[0];          /* Start of downstream MAC addresses */
} __packed;

/*
 * Management Notification Frame
 */
struct ieee80211_mnf {
	uint8_t mnf_category;
	uint8_t mnf_action;
	uint8_t mnf_dialog;
	uint8_t mnf_status;
} __packed;
#define	MNF_SETUP_REQ	0
#define	MNF_SETUP_RESP	1
#define	MNF_TEARDOWN	2

/*
 * Management Action Frames
 */

/* generic frame format */
struct ieee80211_action {
	uint8_t	ia_category;
	uint8_t	ia_action;
} __packed;

/* categories */
#define IEEE80211_ACTION_CAT_SPEC_MGMT		0	/* Spectrum MGMT */
#define IEEE80211_ACTION_CAT_QOS		1	/* qos */
#define IEEE80211_ACTION_CAT_DLS		2	/* dls */
#define IEEE80211_ACTION_CAT_BA			3	/* block ack */
#define IEEE80211_ACTION_CAT_PUBLIC		4	/* Public */
#define IEEE80211_ACTION_CAT_RM			5	/* Radio measurement */
#define IEEE80211_ACTION_CAT_FBSS		6	/* Fast BSS */
#define IEEE80211_ACTION_CAT_HT			7	/* HT */
#define IEEE80211_ACTION_CAT_SA_QUERY		8	/* SA Query */
#define IEEE80211_ACTION_CAT_PROT_DUAL_PA	9	/* Protected Dual of Public Action */
#define IEEE80211_ACTION_CAT_WNM		10	/* WNM */
#define IEEE80211_ACTION_CAT_UNPROT_WNM		11	/* Unprotected WNM */
#define IEEE80211_ACTION_CAT_TDLS		12	/* TDLS */
#define IEEE80211_ACTION_CAT_MESH		13	/* Mesh */
#define IEEE80211_ACTION_CAT_MULTIHOP		14	/* Multihop */
#define IEEE80211_ACTION_CAT_SELF_PROT		15	/* self protected */

#define IEEE80211_ACTION_CAT_VHT		21	/* VHT */
#define IEEE80211_ACTION_CAT_VEND_PROT	126	/* Protected Vendor specific Action frame */
#define IEEE80211_ACTION_CAT_VENDOR		0x7F	/* Vendor specific Action frame */

/* Public Action Frames (7.4.7.1) */
#define IEEE80211_ACTION_PUB_GAS_IREQ		10  /* GAS Service Initial Request */
#define IEEE80211_ACTION_PUB_GAS_IRESP		11  /* GAS Service Initial Response */
#define IEEE80211_ACTION_PUB_GAS_CREQ		12  /* GAS Comeback Request */
#define IEEE80211_ACTION_PUB_GAS_CRESP		13  /* GAS Comeback Response */
#define IEEE80211_ACTION_PUB_TDLS_DISC_RESP	14  /* TDLS Discovery Response */

static __inline__ int ieee80211_action_is_a_gas(const struct ieee80211_action *ia)
{
	return (ia->ia_category == IEEE80211_ACTION_CAT_PUBLIC) &&
		(ia->ia_action >= IEEE80211_ACTION_PUB_GAS_IREQ) &&
		(ia->ia_action <= IEEE80211_ACTION_PUB_GAS_CRESP);
}

/* TDLS Action Frame details (7.4.11) */
#define IEEE80211_ACTION_TDLS_SETUP_REQ        0   /* Setup Request */
#define IEEE80211_ACTION_TDLS_SETUP_RESP       1   /* Setup Response */
#define IEEE80211_ACTION_TDLS_SETUP_CONFIRM    2   /* Setup Confirm */
#define IEEE80211_ACTION_TDLS_TEARDOWN         3   /* Teardown */
#define IEEE80211_ACTION_TDLS_PTI              4   /* Peer Traffic Indication */
#define IEEE80211_ACTION_TDLS_CS_REQ           5   /* Channel Switch Request */
#define IEEE80211_ACTION_TDLS_CS_RESP          6   /* Channel Switch Response */
#define IEEE80211_ACTION_TDLS_PEER_PSM_REQ     7   /* Peer PSM Request */
#define IEEE80211_ACTION_TDLS_PEER_PSM_RESP    8   /* Peer PSM Response */
#define IEEE80211_ACTION_TDLS_PEER_TRAF_RESP   9   /* Peer Traffic Response */
#define IEEE80211_ACTION_TDLS_DISC_REQ         10  /* Discovery Request */

struct ieee80211_ie_power_capability {
	uint8_t	id;
	uint8_t	len;
	uint8_t	min_txpwr;
	uint8_t	max_txpwr;
} __packed;

struct ieee80211_ie_tpc_report {
	uint8_t	id;
	uint8_t	len;
	uint8_t tran_power;
	uint8_t link_margin;
} __packed;

#define IEEE80211_CCA_REQMODE_PARALLEL	(1 << 0)
#define IEEE80211_CCA_REQMODE_ENABLE	(1 << 1)
#define IEEE80211_CCA_REQMODE_REQUEST	(1 << 2)
#define IEEE80211_CCA_REQMODE_REPORT	(1 << 3)
#define IEEE80211_CCA_REQMODE_DURA_MAN	(1 << 4)

#define IEEE80211_CCA_REPMODE_LATE	(1 << 0)
#define IEEE80211_CCA_REPMODE_INCAP	(1 << 1)
#define IEEE80211_CCA_REPMODE_REFUSE	(1 << 2)

/* Spectrum Management */
#define IEEE80211_CCA_MEASTYPE_BASIC	0x00	/* Basic Request */
#define IEEE80211_CCA_MEASTYPE_CCA	0x01	/* Clear Channel Assessment Request */
#define IEEE80211_CCA_MEASTYPE_RPI	0x02	/* Receiver Power Indicator (RPI) histogram Request */
/* Radio Measurement */
#define IEEE80211_RM_MEASTYPE_CH_LOAD	0x03	/* Channel Load Request */
#define IEEE80211_RM_MEASTYPE_NOISE	0x04	/* Noise histogram Request */
#define IEEE80211_RM_MEASTYPE_BEACON	0x05	/* Beacon Request */
#define IEEE80211_RM_MEASTYPE_FRAME	0x06	/* Frame Request */
#define IEEE80211_RM_MEASTYPE_STA	0x07	/* STA statistics Request */
#define IEEE80211_RM_MEASTYPE_LCI	0x08	/* LCI Request */
#define IEEE80211_RM_MEASTYPE_CATEGORY	0x09	/* Transmit stream/Category Request */
#define IEEE80211_RM_MEASTYPE_MUL_DIAG	0x0A	/* Multicast diagnostics request */
#define IEEE80211_RM_MEASTYPE_LOC_CIVIC	0x0B	/* Location Civic request */
#define IEEE80211_RM_MEASTYPE_LOC_ID	0x0C	/* Location Identifier request */
#define IEEE80211_RM_MEASTYPE_QTN_CCA	0xFE	/* QTN CCA extension */
#define IEEE80211_RM_MEASTYPE_PAUSE	0xFF	/* Measurement Pause Request */

/* for Radio Measurement Actions. Table 7-57a in 802.11k $7.4.6 */
#define IEEE80211_ACTION_R_MEASUREMENT_REQUEST	0
#define IEEE80211_ACTION_R_MEASUREMENT_REPORT	1
#define IEEE80211_ACTION_R_LINKMEASURE_REQUEST	2
#define IEEE80211_ACTION_R_LINKMEASURE_REPORT	3
#define IEEE80211_ACTION_R_NEIGHBOR_REQUEST	4
#define IEEE80211_ACTION_R_NEIGHBOR_REPORT	5

struct ieee80211_action_sm_measurement_header {
	uint8_t	ia_category;
	uint8_t	ia_action;
	uint8_t	am_token;
	uint8_t	am_data[0];
}__packed;

/* RM - radio measurement request */
struct ieee80211_action_radio_measure_request {
	struct ieee80211_action	am_header;
	uint8_t	am_token;
	uint16_t	am_rep_num;
	uint8_t	am_data[0];
}__packed;

/* RM - radio measurement report */
struct ieee80211_action_radio_measure_report {
	struct ieee80211_action	am_header;
	uint8_t	am_token;
	uint8_t	am_data[0];
}__packed;

/*
 * 802.11h measurement request/report element
 * 802.11k measurement request/report element
 * common part
 */
struct ieee80211_ie_measure_comm {
	uint8_t id;		/* IEEE80211_ELEMID_MEASREQ = 38 */
	uint8_t len;		/* 14 for known types */
	uint8_t token;	/* Non-zero number for diff. measurement reqs. */
	uint8_t mode;	/* bits: 1 enable, 2 req, 3 report, 0,4-7 reserved */
	uint8_t type;	/* basic = 0, cca = 1, rpi histogram = 2 */
	uint8_t data[0];	/* variable format according to meas_type */
} __packed;

struct ieee80211_ie_measreq {
	uint8_t chan_num;	/* channel number */
	uint64_t start_tsf;	/* starting time in tsf */
	uint16_t duration_tu;	/* measurement duration in TU */
} __packed;

/*
 * 802.11k measurement request element of sta statistics
 * for PM module collect sta statistics
 * See 802.11k 2003 7.3.2.21.8
 */
struct ieee80211_ie_measreq_sta_stat {
	uint8_t peer_mac[IEEE80211_ADDR_LEN];	/* Peer Mac Address */
	uint16_t random_interval;	/* randomization interval */
	uint16_t duration_tu;	/* measurement duration in TU */
	uint8_t group_id;		/*	group identity	*/
	uint8_t data[0];	/*	Optional sub-elements in variable length	*/
} __packed;

struct ieee80211_ie_measreq_chan_load {
	uint8_t operating_class;
	uint8_t channel_num;
	uint16_t random_interval_tu;
	uint16_t duration_tu;
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measreq_noise_his {
	uint8_t operating_class;
	uint8_t channel_num;
	uint16_t random_interval_tu;
	uint16_t duration_tu;
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measreq_beacon {
	uint8_t operating_class;
	uint8_t channel_num;
	uint16_t random_interval_tu;
	uint16_t duration_tu;
	uint8_t measure_mode;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measreq_frame {
	uint8_t operating_class;
	uint8_t channel_num;
	uint16_t random_interval_tu;
	uint16_t duration_tu;
	uint8_t frame_request_type;
#define FRAME_COUNT_REPORT	1

	uint8_t mac_addr[IEEE80211_ADDR_LEN];
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measreq_trans_stream_cat {
	uint16_t random_interval_tu;
	uint16_t duration_tu;
	uint8_t peer_sta_addr[IEEE80211_ADDR_LEN];
	uint8_t tid;
	uint8_t bin0_range;
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measreq_multicast_diag {
	uint16_t random_interval_tu;
	uint16_t duration_tu;
	uint8_t group_mac_addr[IEEE80211_ADDR_LEN];
	uint8_t data[0];
} __packed;

struct ieee80211_subie_multicast_triggered_reporting {
	uint8_t sub_id;
	uint8_t len;
	uint8_t condition;
	uint8_t inactivity_timeout;
	uint8_t reactivation_delay;
} __packed;

struct ieee80211_action_rm_link_measure_request {
	struct ieee80211_action	at_header;
	uint8_t token;
	uint8_t tran_power_used;
	uint8_t max_tran_power;
	uint8_t data[0];
} __packed;

struct ieee80211_action_rm_neighbor_report_request {
	struct ieee80211_action	at_header;
	uint8_t token;
	uint8_t data[0];
} __packed;

/*
 * 802.11h measurement report element
 * see 8.4.2.24 IEEE 802.11-2012
 */
struct ieee80211_ie_measrep_basic {
	uint8_t	chan_num;	/* channel number */
	uint64_t start_tsf;	/* starting time in tsf */
	uint16_t duration_tu;	/* measurement duration in TU */
	uint8_t	basic_report;	/* basic report data */
} __packed;

struct ieee80211_ie_measrep_cca {
	uint8_t	chan_num;	/* channel number */
	uint64_t start_tsf;	/* starting time in tsf */
	uint16_t duration_tu;	/* measurement duration in TU */
	uint8_t	cca_report;	/* cca report data */
#define IEEE80211_MEASURE_BASIC_REPORT_BSS		(1 << 0)
#define IEEE80211_MEASURE_BASIC_REPORT_OFDM_PRE		(1 << 1)
#define IEEE80211_MEASURE_BASIC_REPORT_UNDEF		(1 << 2)
#define IEEE80211_MEASURE_BASIC_REPORT_RADAR		(1 << 3)
#define IEEE80211_MEASURE_BASIC_REPORT_UMMEASURE	(1 << 4)
} __packed;

struct ieee80211_ie_measrep_rpi {
	uint8_t	chan_num;	/* channel number */
	uint64_t start_tsf;	/* starting time in tsf */
	uint16_t duration_tu;	/* measurement duration in TU */
	uint8_t	rpi_report[8];	/* rpi report data */
} __packed;

/*
 * 802.11k measurement report element of sta statistics
 * for PM module collect sta statistics
 * See 802.11k 2003 7.3.2.22.8
 */
struct ieee80211_ie_measrep_sta_stat {
	uint16_t duration_tu;	/* measurement duration in TU */
	uint8_t group_id;		/*	group identity	*/
	uint8_t data[0];	/*	Optional sub-elements in variable length	*/
} __packed;

#define IEEE80211_RM_MEAS_SUBTYPE_LEN_MIN	2

/* Quantenna RM group ie, to complement an 802.11k group ie. Node statistics & parameters */
struct ieee80211_ie_qtn_rm_measure_sta {
	uint8_t id;	/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;   /* length in bytes */
	uint8_t qtn_ie_oui[3];		/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t seq;	/* sequence */
	uint8_t type;				/* Which group (special or all) contains in the data. */
	uint8_t data[0];
} __packed;

struct ieee80211_ie_qtn_rm_txstats {
	uint64_t tx_bytes;
	uint32_t tx_pkts;
	uint32_t tx_discard;
	uint32_t tx_err;
	uint32_t tx_ucast;		/* unicast */
	uint32_t tx_mcast;		/* multicast */
	uint32_t tx_bcast;		/* broadcast */
} __packed;

struct ieee80211_ie_qtn_rm_rxstats {
	uint64_t rx_bytes;
	uint32_t rx_pkts;
	uint32_t rx_discard;
	uint32_t rx_err;
	uint32_t rx_ucast;		/* unicast */
	uint32_t rx_mcast;		/* multicast */
	uint32_t rx_bcast;		/* broadcast */
} __packed;

struct ieee80211_ie_qtn_rm_sta_all {
	struct ieee80211_ie_qtn_rm_txstats tx_stats;
	struct ieee80211_ie_qtn_rm_rxstats rx_stats;
	u_int32_t max_queued;
	u_int32_t link_quality;
	u_int32_t rssi_dbm;
	u_int32_t bandwidth;
	u_int32_t snr;
	u_int32_t tx_phy_rate;
	u_int32_t rx_phy_rate;
	u_int32_t cca;	/* Reserved for cca */
	u_int32_t br_ip;
	u_int32_t rssi;
	u_int32_t hw_noise;
	u_int8_t soc_macaddr[IEEE80211_ADDR_LEN];
	u_int32_t soc_ipaddr;
} __packed;

/*
 * Statistics Group data format for STB
 */
struct ieee80211_ie_rm_sta_grp221 {
	uint8_t soc_macaddr[IEEE80211_ADDR_LEN];
	uint8_t rssi;
	uint8_t phy_noise;
} __packed;

/* dot11Counters Group */
struct ieee80211_rm_sta_stats_group0 {
	uint32_t dot11TransmittedFragmentCount;
	uint32_t dot11MulticastTransmittedFrameCount;
	uint32_t dot11FailedCount;
	uint32_t dot11ReceivedFragmentCount;
	uint32_t dot11MulticastReceivedFrameCount;
	uint32_t dot11FCSErrorCount;
	uint32_t dot11TransmittedFrameCount;
} __packed;

/* dot11MACStatistics Group */
struct ieee80211_rm_sta_stats_group1 {
	uint32_t dot11RetryCount;
	uint32_t dot11MultipleRetryCount;
	uint32_t dot11FrameDuplicateCount;
	uint32_t dot11RTSSuccessCount;
	uint32_t dot11RTSFailureCount;
	uint32_t dot11ACKFailureCount;
} __packed;

/* dot11QosCounters Group for UP0-UP7 */
struct ieee80211_rm_sta_stats_group2to9 {
	uint32_t dot11QosTransmittedFragmentCount;
	uint32_t dot11QosFailedCount;
	uint32_t dot11QosRetryCount;
	uint32_t dot11QosMultipleRetryCount;
	uint32_t dot11QosFrameDuplicateCount;
	uint32_t dot11QosRTSSuccessCount;
	uint32_t dot11QosRTSFailureCount;
	uint32_t dot11QosACKFailureCount;
	uint32_t dot11QosReceivedFragmentCount;
	uint32_t dot11QosTransmittedFrameCount;
	uint32_t dot11QosDiscardedFrameCount;
	uint32_t dot11QosMPDUsReceivedCount;
	uint32_t dot11QosRetriesReceivedCount;
} __packed;

/* dot11BSSAverageAccessDelay Group (only available at an AP) */
struct ieee80211_rm_sta_stats_group10 {
	uint32_t dot11STAStatisticsAPAverageAccessDelay;
	uint32_t dot11STAStatisticsAverageAccessDelayBestEffort;
	uint32_t dot11STAStatisticsAverageAccessDelayBackGround;
	uint32_t dot11STAStatisticsAverageAccessDelayVideo;
	uint32_t dot11STAStatisticsAverageAccessDelayVoice;
	uint32_t dot11STAStatisticsStationCount;
	uint32_t dot11STAStatisticsChannelUtilization;
} __packed;

struct ieee80211_rm_sta_stats_group11 {
	uint32_t dot11TransmittedAMSDUCount;
	uint32_t dot11FailedAMSDUCount;
	uint32_t dot11RetryAMSDUCount;
	uint32_t dot11MultipleRetryAMSDUCount;
	uint32_t dot11TransmittedOctetsInAMSDUCount;
	uint32_t dot11AMSDUAckFailureCounnt;
	uint32_t dot11ReceivedAMSDUCount;
	uint32_t dot11ReceivedOctetsInAMSDUCount;
} __packed;

struct ieee80211_rm_sta_stats_group12 {
	uint32_t dot11TransmittedAMPDUCount;
	uint32_t dot11TransmittedMPDUsInAMPDUCount;
	uint64_t dot11TransmittedOctetsInAMPDUCount;
	uint32_t dot11AMPDUReceivedCount;
	uint32_t dot11MPDUInReceivedAMPDUCount;
	uint64_t dot11ReceivedOctetsInAMPDUCount;
	uint32_t dot11AMPDUDelimiterCRCErrorCount;
} __packed;

struct ieee80211_rm_sta_stats_group13 {
	uint32_t dot11ImplicitBARFailureCount;
	uint32_t dot11ExplicitBARFailureCount;
	uint32_t dot11ChannelWidthSwitchCount;
	uint32_t dot11TwentyMHzFrameTransmittedCount;
	uint32_t dot11FortyMHzFrameTransmittedCount;
	uint32_t dot11TwentyMHzFrameReceivedCount;
	uint32_t dot11FortyMHzFrameReceivedCount;
	uint32_t dot11PSMPUTTGrantDuration;
	uint32_t dot11PSMPUTTUsedDuration;
} __packed;

struct ieee80211_rm_sta_stats_group14 {
	uint32_t dot11GrantedRDGUsedCount;
	uint32_t dot11GrantedRDGUnusedCount;
	uint32_t dot11TransmittedFramesInGrantedRDGCount;
	uint64_t dot11TransmittedOctetsInGrantedRDGCount;
	uint32_t dot11DualCTSSuccessCount;
	uint32_t dot11DualCTSFailureCount;
	uint32_t dot11RTSLSIGSuccessCount;
	uint32_t dot11RTSLSIGFailureCount;
} __packed;

struct ieee80211_rm_sta_stats_group15 {
	uint32_t dot11BeamformingFrameCount;
	uint32_t dot11STBCCTSSuccessCount;
	uint32_t dot11STBCCTSFailureCount;
	uint32_t dot11nonSTBCCTSSuccessCount;
	uint32_t dot11nonSTBCCTSFailureCount;
} __packed;

struct ieee80211_rm_sta_stats_group16 {
	uint32_t dot11RSNAStatsCMACICVErrors;
	uint32_t dot11RSNAStatsCMACReplays;
	uint32_t dot11RSNAStatsRobustMgmtCCMPReplays;
	uint32_t dot11RSNAStatsTKIPICVErrors;
	uint32_t dot11RSNAStatsTKIPReplays;
	uint32_t dot11RSNAStatsCCMPDecryptErrors;
	uint32_t dot11RSNAStatsCCMPReplays;
} __packed;

/*
 * STA Statistics QTN specific
 */
enum RadioMeasureQTNElementID {
	RM_QTN_TX_STATS			=	0,
	RM_QTN_RX_STATS			=	1,
	RM_QTN_MAX_QUEUED		=	2,
	RM_QTN_LINK_QUALITY		=	3,
	RM_QTN_RSSI_DBM			=	4,
	RM_QTN_BANDWIDTH		=	5,
	RM_QTN_SNR			=	6,
	RM_QTN_TX_PHY_RATE		=	7,
	RM_QTN_RX_PHY_RATE		=	8,
	RM_QTN_CCA			=	9,
	RM_QTN_BR_IP			=	10,
	RM_QTN_RSSI			=	11,
	RM_QTN_HW_NOISE			=	12,
	RM_QTN_SOC_MACADDR		=	13,
	RM_QTN_SOC_IPADDR		=	14,
	RM_QTN_MAX			=	RM_QTN_SOC_IPADDR,
	RM_QTN_UNKNOWN			=	15,
	RM_QTN_CTRL_START		=	16,
	RM_QTN_RESET_CNTS		=	16,
	RM_QTN_RESET_QUEUED		=	17,
	RM_QTN_CTRL_END			=	17,
};
#define RM_QTN_MEASURE_MASK ((1 << (RM_QTN_CTRL_END + 1)) - 1)

/*
 * STA Statistic for Group221 specific
 */
enum RadioMeasureGrp221ElementID {
	RM_GRP221_RSSI			=	(RM_QTN_CTRL_END + 1),
	RM_GRP221_PHY_NOISE		=	(RM_QTN_CTRL_END + 2),
	RM_GRP221_SOC_MAC		=	(RM_QTN_CTRL_END + 3),
};

extern const uint8_t ieee80211_meas_sta_qtn_report_subtype_len[RM_QTN_CTRL_END + 1];

/* Standard CCA Flag to used */
#define RM_STANDARD_CCA 0x1009
#define IEEE80211_11K_CCA_INTF_SCALE 255
/*
 * CCA radio measurement report field
 */
struct cca_rm_rep_data {
	uint8_t ch_num;
	uint8_t tm_start[8];
	uint8_t m_duration[2];
	uint8_t busy_frac;
} __packed;

/* CCA report IE*/
struct ieee80211_ie_rm_measure_cca_rep {
	uint8_t id;
	uint8_t len;
	uint8_t rm_token;
	uint8_t rm_rep_mode;
	uint8_t rm_rep_type;
	struct cca_rm_rep_data rep_data;
	struct ieee80211_ie_qtn_scs scs_data;
} __packed;

struct ieee80211_ie_measrep_chan_load {
	uint8_t operating_class;
	uint8_t channel_num;
	uint8_t start_time[8];
	uint16_t duration_tu;
	uint8_t channel_load;
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measrep_noise_his {
	uint8_t operating_class;
	uint8_t channel_num;
	uint8_t start_time[8];
	uint16_t duration_tu;
	uint8_t antenna_id;
	uint8_t anpi;
	uint8_t ipi[11];
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measrep_beacon {
	uint8_t operating_class;
	uint8_t channel_num;
	uint8_t start_time[8];
	uint16_t duration_tu;
	uint8_t reported_frame_info;
	uint8_t rcpi;
	uint8_t rsni;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint8_t antenna_id;
	uint8_t parent_tsf[4];
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measrep_frame {
	uint8_t operating_class;
	uint8_t channel_num;
	uint8_t start_time[8];
	uint16_t duration_tu;
	uint8_t data[0];
} __packed;

#define IEEE80211_FRAME_REPORT_SUBELE_FRAME_COUNT_REPORT	1

struct ieee80211_subie_section_frame_entry {
	uint8_t id;
	uint8_t len;
	uint8_t transmit_address[IEEE80211_ADDR_LEN];
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint8_t phy_type;
	uint8_t avg_rcpi;
	uint8_t last_rsni;
	uint8_t last_rcpi;
	uint8_t anntenna_id;
	uint16_t frame_cnt;
	uint8_t data[0];
} __packed;

struct ieee80211_ie_measrep_trans_stream_cat {
	uint8_t start_time[8];
	uint16_t duration_tu;
	uint8_t peer_sta_address[IEEE80211_ADDR_LEN];
	uint8_t tid;
	uint8_t reason;
	uint32_t tran_msdu_cnt;
	uint32_t msdu_discarded_cnt;
	uint32_t msdu_failed_cnt;
	uint32_t msdu_mul_retry_cnt;
	uint32_t qos_cf_lost_cnt;
	uint32_t avg_queue_delay;
	uint32_t avg_trans_delay;
	uint8_t bin0_range;
	uint32_t bin0;
	uint32_t bin1;
	uint32_t bin2;
	uint32_t bin3;
	uint32_t bin4;
	uint32_t bin5;
} __packed;

struct ieee80211_ie_measrep_multicast_diag {
	uint8_t measure_time[8];
	uint16_t duration_tu;
	uint8_t group_mac_addr[IEEE80211_ADDR_LEN];
	uint8_t reason;
	uint32_t mul_rx_msdu_cnt;
	uint16_t first_seq_num;
	uint16_t last_seq_num;
	uint16_t mul_rate;
} __packed;

struct ieee80211_action_rm_link_measure_report {
	struct ieee80211_action at_header;
	uint8_t token;
	struct ieee80211_ie_tpc_report tpc_report;
	uint8_t recv_antenna_id;
	uint8_t tran_antenna_id;
	uint8_t rcpi;
	uint8_t rsni;
	uint8_t data[0];
} __packed;

struct ieee80211_action_rm_neighbor_report_response {
	struct ieee80211_action at_header;
	uint8_t token;
	uint8_t data[0];
} __packed;

struct ieee80211_ie_neighbor_report {
	uint8_t id;
	uint8_t len;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint32_t bssid_info;
#define BSSID_INFO_AP_NOT_REACHABLE		(1 << 0)
#define BSSID_INFO_AP_UNKNOWN			(2 << 0)
#define BSSID_INFO_AP_REACHABLE			(3 << 0)
#define BSSID_INFO_SECURITY_COPY		(1 << 2)
#define BSSID_INFO_KEY_SCOPE_COPY		(1 << 3)
#define BSSID_INFO_CAP_SPECTRUM_MANAGEMENT	(1 << 4)
#define BSSID_INFO_CAP_QOS			(1 << 5)
#define BSSID_INFO_CAP_APSD			(1 << 6)
#define BSSID_INFO_CAP_RADIO_MEASUREMENT	(1 << 7)
#define BSSID_INFO_CAP_DELAYED_BA		(1 << 8)
#define BSSID_INFO_CAP_IMMEDIATE_BA		(1 << 9)
#define BSSID_INFO_MOBILITY_DOMAIN		(1 << 10)
#define BSSID_INFO_HIGH_THROUGHPUT		(1 << 11)
	uint8_t operating_class;
	uint8_t channel;
	uint8_t phy_type;
	uint8_t data[0];
} __packed;

/* HT actions */
#define IEEE80211_ACTION_HT_TXCHWIDTH		0	/* recommended transmission channel width */
#define IEEE80211_ACTION_HT_MIMOPWRSAVE		1	/* MIMO power save */
#define IEEE80211_ACTION_HT_NCBEAMFORMING	5	/* HT non compressed beamforming report */
#define IEEE80211_ACTION_HT_CBEAMFORMING	6	/* HT compressed beamforming report */

/* VHT actions */
#define IEEE80211_ACTION_VHT_CBEAMFORMING	0	/* VHT compressed beamforming report */
#define IEEE80211_ACTION_VHT_MU_GRP_ID          1       /* VHT MU GRP ID mgmt */
#define IEEE80211_ACTION_VHT_OPMODE_NOTIFICATION	2	/* VHT Operating mode Notification */

/* HT - recommended transmission channel width */
struct ieee80211_action_ht_txchwidth {
	struct ieee80211_action		at_header;
	u_int8_t			at_chwidth;
} __packed;

#define IEEE80211_A_HT_TXCHWIDTH_20	0
#define IEEE80211_A_HT_TXCHWIDTH_2040	1


/* HT - MIMO Power Save */
struct ieee80211_action_ht_mimopowersave {
	struct ieee80211_action		am_header;
	uint8_t				am_enable_mode;
} __packed;

/* HT - Non compressed beam forming */

struct ht_mimo_ctrl {
	uint16_t			am_mimoctrl;
	uint32_t			am_timestamp;
} __packed;

#define IEEE80211_HT_MIMO_CTRL_NC_M			0x0003
#define IEEE80211_HT_MIMO_CTRL_NC_S			0
#define IEEE80211_HT_MIMO_CTRL_NR_M			0x000C
#define IEEE80211_HT_MIMO_CTRL_NR_S			2
#define IEEE80211_HT_MIMO_CTRL_CH_WIDTH_20	0x0000
#define IEEE80211_HT_MIMO_CTRL_CH_WIDTH_40	0x0010
#define IEEE80211_HT_MIMO_CTRL_NG_M			0x0060
#define IEEE80211_HT_MIMO_CTRL_NG_S			5
#define IEEE80211_HT_MIMO_CTRL_NB_M		0x0180
#define IEEE80211_HT_MIMO_CTRL_NB_S		7
#define IEEE80211_HT_MIMO_CTRL_CODEBOOK_M	0x0600
#define IEEE80211_HT_MIMO_CTRL_CODEBOOK_S	9
#define IEEE80211_HT_MIMO_CTRL_SEG_M		0x3800
#define IEEE80211_HT_MIMO_CTRL_SEG_S		11


enum {
	IEEE80211_HT_MIMO_CTRL_NC_1 = 0,
	IEEE80211_HT_MIMO_CTRL_NC_2,
	IEEE80211_HT_MIMO_CTRL_NC_3,
	IEEE80211_HT_MIMO_CTRL_NC_4,
};

enum {
	IEEE80211_HT_MIMO_CTRL_NR_1 = 0,
	IEEE80211_HT_MIMO_CTRL_NR_2,
	IEEE80211_HT_MIMO_CTRL_NR_3,
	IEEE80211_HT_MIMO_CTRL_NR_4,
};

enum {
	IEEE80211_HT_MIMO_CTRL_NG_NONE = 0,
	IEEE80211_HT_MIMO_CTRL_NG_2,
	IEEE80211_HT_MIMO_CTRL_NG_4,
	IEEE80211_HT_MIMO_CTRL_NG_RESERVED,
};

enum {
	IEEE80211_HT_MIMO_CTRL_NB_4 = 0,
	IEEE80211_HT_MIMO_CTRL_NB_2,
	IEEE80211_HT_MIMO_CTRL_NB_6,
	IEEE80211_HT_MIMO_CTRL_NB_8,
};

struct ieee80211_action_ht_bf {
	struct ieee80211_action	am_header;
	struct ht_mimo_ctrl	am_mimo_ctrl;
	uint8_t			am_bf_report[0]; /* start of beamforming report */
} __packed;

/* VHT - Tx Beamforming */
struct vht_mimo_ctrl {
	uint8_t		am_mimoctrl[3];
} __packed;

/* VHT - Operating mode notification */
struct ieee80211_action_vht_opmode_notification {
	struct ieee80211_action		am_header;
	u_int8_t			am_opmode;
} __packed;

#define IEEE80211_VHT_OPMODE_CHWIDTH		0x03
#define IEEE80211_VHT_OPMODE_CHWIDTH_S		0
#define IEEE80211_VHT_OPMODE_RXNSS		0x70
#define IEEE80211_VHT_OPMODE_RXNSS_S		4
#define IEEE80211_VHT_OPMODE_RXNSS_TYPE		0x80
#define IEEE80211_VHT_OPMODE_RXNSS_TYPE_S	7

#define IEEE80211_VHT_MIMO_CTRL_NC_M		0x000007
#define IEEE80211_VHT_MIMO_CTRL_NC_S		0
#define IEEE80211_VHT_MIMO_CTRL_NR_M		0x000038
#define IEEE80211_VHT_MIMO_CTRL_NR_S		3
#define IEEE80211_VHT_MIMO_CTRL_CH_BW_M		0x0000C0
#define IEEE80211_VHT_MIMO_CTRL_CH_BW_S		6
#define IEEE80211_VHT_MIMO_CTRL_CH_WIDTH_20	0x000000
#define IEEE80211_VHT_MIMO_CTRL_CH_WIDTH_40	0x000040
#define IEEE80211_VHT_MIMO_CTRL_CH_WIDTH_80	0x000080
#define IEEE80211_VHT_MIMO_CTRL_CH_WIDTH_160	0x0000C0
#define IEEE80211_VHT_MIMO_CTRL_NG_M		0x000300
#define IEEE80211_VHT_MIMO_CTRL_NG_S		8
#define IEEE80211_VHT_MIMO_CTRL_CODEBOOK_M	0x000400
#define IEEE80211_VHT_MIMO_CTRL_CODEBOOK_S	10
#define IEEE80211_VHT_MIMO_CTRL_FBTYPE_M	0x000800
#define IEEE80211_VHT_MIMO_CTRL_FBTYPE_S	11
#define IEEE80211_VHT_MIMO_CTRL_R_FB_M          0x007000
#define IEEE80211_VHT_MIMO_CTRL_R_FB_S          12
#define IEEE80211_VHT_MIMO_CTRL_FIRSTFB_M       0x008000
#define IEEE80211_VHT_MIMO_CTRL_FIRSTFB_S       15
#define IEEE80211_VHT_MIMO_CTRL_DTOKEN_M        0xFC0000
#define IEEE80211_VHT_MIMO_CTRL_DTOKEN_S        18

/* Block Ack actions */
#define IEEE80211_ACTION_BA_ADDBA_REQ		0	/* Add block ack request */
#define IEEE80211_ACTION_BA_ADDBA_RESP		1	/* Add block ack response */
#define IEEE80211_ACTION_BA_DELBA			2	/* delete block ack */

/* BA - Add block ack request */
struct ieee80211_action_ba_addba_req {
	struct ieee80211_action		am_header;
	uint8_t	am_dlg;
	uint16_t	am_ba_params;
	uint16_t	am_ba_to;
	uint16_t	am_ba_seq;
} __packed;

#define IEEE80211_A_BA_AMSDU_SUPPORTED		0x0001
#define IEEE80211_A_BA_IMMEDIATE			0x0002
#define IEEE80211_A_BA_DELAYED				0x0000
#define IEEE80211_A_BA_TID_M				0x003C
#define IEEE80211_A_BA_TID_S				2
#define IEEE80211_A_BA_BUFF_SIZE_M			0xFFC0
#define IEEE80211_A_BA_BUFF_SIZE_S			6
#define IEEE80211_A_BA_FRAG_M				0x000F
#define IEEE80211_A_BA_FRAG_S				0
#define IEEE80211_A_BA_SEQ_M				0xFFF0
#define IEEE80211_A_BA_SEQ_S				4
#define IEEE80211_IOT_INTEL_AGG_MAX_FRAMES_NUM          16

/* BA - Add block ack response */
struct ieee80211_action_ba_addba_resp {
	struct ieee80211_action		am_header;
	uint8_t	am_dlg;
	__le16		am_status;
	__le16		am_ba_params;
	__le16		am_ba_to;
} __packed;

/* BA - delete block ack request */
struct ieee80211_action_ba_delba {
	struct ieee80211_action		am_header;
	__le16		am_delba_params;
	__le16		am_reason;
}__packed;

#define IEEE80211_A_BA_INITIATOR			0x0800
#define IEEE80211_A_BA_INITIATOR_S			11
#define IEEE80211_A_BA_DELBA_TID			0xF000
#define IEEE80211_A_BA_DELBA_TID_S			12

/* Move to a .config file later. */
#define CONFIG_QHOP 1

#ifdef CONFIG_QHOP
#define QDRV_ACTION_TYPE_QHOP        0x19
#define QDRV_ACTION_QHOP_DFS_REPORT  0x1
#define QDRV_ACTION_QHOP_SCS_REPORT  0x2

struct qdrv_vendor_action_header {
	uint8_t category;
	uint8_t oui[3];
	uint8_t type;
	uint8_t action;
} __packed;

struct qdrv_vendor_action_qhop_dfs_data {
	uint8_t cur_chan;
} __packed;

#endif

#ifdef CONFIG_QVSP

/**
 * Structures for action frames used to set stream states and configure VSP.
 *
 * These structures are the ones that go over the medium, so must be packed.
 */
#define QVSP_ACTION_TYPE_VSP 0x1
#define QVSP_ACTION_STRM_CTRL 0x1
#define QVSP_ACTION_VSP_CTRL 0x2

/**
 * Common header for all VSP action frames.
 */
struct ieee80211_qvsp_act_header_s {
	uint8_t category;
	uint8_t oui[3];
	uint8_t type;
	uint8_t action;
} __packed;

struct ieee80211_qvsp_act_frm_dis_attr_s {
	uint32_t throt_policy;
	uint32_t throt_rate;
	uint32_t demote_rule;
	uint32_t demote_state;
} __packed;

/**
 * Stream control action frame.
 */
struct ieee80211_qvsp_act_strm_ctrl_s {
	struct ieee80211_qvsp_act_header_s header;
	uint8_t strm_state;
	uint8_t count;
	struct ieee80211_qvsp_act_frm_dis_attr_s dis_attr;
	struct ieee80211_qvsp_strm_id strm_items[0]; /* One or more of these entries */
} __packed;

/**
 * Single VSP control item - set a parameter remotely.
 */
struct ieee80211_qvsp_act_vsp_ctrl_item_s {
	uint32_t index;
	uint32_t value;
} __packed;

/**
 * VSP configuration/control action frame.
 */
struct ieee80211_qvsp_act_vsp_ctrl_s {
	struct ieee80211_qvsp_act_header_s header;
	uint8_t count;
	uint8_t pad[3]; /* Pad for 32-bit alignment */
	struct ieee80211_qvsp_act_vsp_ctrl_item_s ctrl_items[0]; /* One or more of these entries */
} __packed;

#endif

/*
 * 802.11w / PMF SA Query Action Frame
 */
#define IEEE80211_ACTION_W_SA_QUERY_REQ		0
#define IEEE80211_ACTION_W_SA_QUERY_RESP	1

struct ieee80211_action_sa_query {
	struct ieee80211_action		at_header;
	u_int16_t			at_tid;
} __packed;

/*
 * Control frames.
 */
struct ieee80211_frame_min {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

/*
 * BAR frame format
 */
#define IEEE80211_BAR_CTL_TID		0xF000      /* tid mask             */
#define IEEE80211_BAR_CTL_TID_S         12      /* tid shift            */
#define IEEE80211_BAR_CTL_NOACK		0x0001      /* no-ack policy        */
#define IEEE80211_BAR_CTL_COMBA		0x0004      /* compressed block-ack */
#define IEEE80211_BAR_CTL_MULTIBA	0x0006		/* Multi TID Block Ack */
#define IEEE80211_BAR_INFO_FRAG_M	0x000F		/* fragment mask */
#define IEEE80211_BAR_CTL_FRAG_S	0			/* fragment shift */
#define IEEE80211_BAR_CTL_SEQ		0xFFF0		/* sequence number mask */
#define IEEE80211_BAR_CTL_SEQ_S		4			/* sequence number shift */


struct ieee80211_frame_bar {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_ra[IEEE80211_ADDR_LEN];
	uint8_t	i_ta[IEEE80211_ADDR_LEN];
	uint16_t	i_ctl;
	uint8_t	i_info[0];						/* variable length */
	/* FCS */
} __packed;

struct ieee80211_frame_bar_info_simple {
	uint16_t	i_seq;
} __packed;

struct ieee80211_frame_bar_info_tid {
	uint16_t	i_tid;
	uint16_t	i_seq;
} __packed;

#define IEEE80211_BAR_HDR_LEN		16
#define IEEE80211_BAR_COMPRESSED_LEN	(sizeof(struct ieee80211_frame_bar) + \
						sizeof(struct ieee80211_frame_bar_info_simple))

/*
 * BA frame format
 */
struct ieee80211_frame_ba {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_ra[IEEE80211_ADDR_LEN];
	uint8_t	i_ta[IEEE80211_ADDR_LEN];
	uint16_t	i_ctl;
	uint8_t	i_info[0];						/* variable length */
	/* FCS */
} __packed;

struct ieee80211_frame_ba_simple {
	uint16_t	i_seq;
	uint8_t	i_bm[128];
} __packed;

struct ieee80211_frame_ba_comp {
	uint16_t	i_seq;
	uint8_t	i_bm[8];
} __packed;

struct ieee80211_frame_ba_tid {
	uint16_t	i_tid;
	uint16_t	i_seq;
	uint8_t	i_bm[8];
} __packed;

struct ieee80211_frame_rts {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_ra[IEEE80211_ADDR_LEN];
	uint8_t i_ta[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_cts {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_ra[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_ack {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_ra[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_pspoll {
	uint8_t i_fc[2];
	uint8_t i_aid[2];
	uint8_t i_bssid[IEEE80211_ADDR_LEN];
	uint8_t i_ta[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_cfend {		/* NB: also CF-End+CF-Ack */
	uint8_t i_fc[2];
	uint8_t i_dur[2];	/* should be zero */
	uint8_t i_ra[IEEE80211_ADDR_LEN];
	uint8_t i_bssid[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_cw {
	uint8_t	i_fc[2];
	uint8_t	i_dur[2];
	uint8_t	i_ra[IEEE80211_ADDR_LEN];
	uint8_t	i_cfc[2]; /* carried frame control */
	/* variable control frame */
	/* FCS */
} __packed;

/* 802.11 Management over Ethernet Payload Types (Annex U.1) */
#define IEEE80211_SNAP_TYPE_REMOTE             1   /* Remote request/response */
#define IEEE80211_SNAP_TYPE_TDLS               2   /* TDLS */

#define IEEE80211_FCS_LEN		4
#define IEEE80211_ENCR_HDR_AES_LEN	16

/*
 * BEACON management packets
 *
 *	octet timestamp[8]
 *	octet beacon interval[2]
 *	octet capability information[2]
 *	information element
 *		octet elemid
 *		octet length
 *		octet information[length]
 */

typedef uint8_t *ieee80211_mgt_beacon_t;

#define	IEEE80211_BEACON_INTERVAL(beacon) \
	((beacon)[8] | ((beacon)[9] << 8))
#define	IEEE80211_BEACON_CAPABILITY(beacon) \
	((beacon)[10] | ((beacon)[11] << 8))

#define	IEEE80211_CAPINFO_ESS			0x0001
#define	IEEE80211_CAPINFO_IBSS			0x0002
#define	IEEE80211_CAPINFO_CF_POLLABLE		0x0004
#define	IEEE80211_CAPINFO_CF_POLLREQ		0x0008
#define	IEEE80211_CAPINFO_PRIVACY		0x0010
#define	IEEE80211_CAPINFO_SHORT_PREAMBLE	0x0020
#define	IEEE80211_CAPINFO_PBCC			0x0040
#define	IEEE80211_CAPINFO_CHNL_AGILITY		0x0080
/* bits 8-9 are reserved (8 now for spectrum management) */
#define IEEE80211_CAPINFO_SPECTRUM_MGMT		0x0100
#define	IEEE80211_CAPINFO_WME				0x0200
#define	IEEE80211_CAPINFO_SHORT_SLOTTIME	0x0400
#define	IEEE80211_CAPINFO_RSN			0x0800
/* bit 12 is reserved */
#define	IEEE80211_CAPINFO_DSSSOFDM		0x2000
/* bits 14-15 are reserved */

/* Extended Capabilities element (8.4.2.29) - bits 0 to 31 */
#define IEEE80211_EXTCAP1_TDLS_UAPSD		0x10000000UL	/* TDLS peer U-APSD buf STA support */
#define IEEE80211_EXTCAP1_TDLS_PSM		0x20000000UL	/* Peer PSM Support */
#define IEEE80211_EXTCAP1_TDLS_CS		0x40000000UL	/* channel switching */

/* Extended Capabilities element (8.4.2.29) - bits 32 to 63 */
#define IEEE80211_EXTCAP2_TDLS			0x00000020UL	/* TDLS supported */
#define IEEE80211_EXTCAP2_TDLS_PROHIB		0x00000040UL	/* TDLS prohibited */
#define IEEE80211_EXTCAP2_TDLS_CS_PROHIB	0x00000080UL	/* TDLS channel switch prohibited */

#define IEEE8211_EXTCAP_LENGTH	8	/* Extended capabilities element length */

/*
 * 802.11i/WPA information element (maximally sized).
 */
struct ieee80211_ie_wpa {
	uint8_t wpa_id;			/* IEEE80211_ELEMID_VENDOR */
	uint8_t wpa_len;		/* length in bytes */
	uint8_t wpa_oui[3];		/* 0x00, 0x50, 0xf2 */
	uint8_t wpa_type;		/* OUI type */
	uint16_t wpa_version;		/* spec revision */
	uint32_t wpa_mcipher[1];	/* multicast/group key cipher */
	uint16_t wpa_uciphercnt;	/* # pairwise key ciphers */
	uint32_t wpa_uciphers[8];	/* ciphers */
	uint16_t wpa_authselcnt;	/* authentication selector cnt*/
	uint32_t wpa_authsels[8];	/* selectors */
	uint16_t wpa_caps;		/* 802.11i capabilities */
	uint16_t wpa_pmkidcnt;		/* 802.11i pmkid count */
	uint16_t wpa_pmkids[8];		/* 802.11i pmkids */
} __packed;

/* TDLS Link Identifier element (7.3.2.62) */
struct ieee80211_tdls_link_id {
	uint8_t        id;                             /* IEEE80211_ELEMID_TDLS_LINK_ID */
	uint8_t        len;                            /* 20 */
	uint8_t        bssid[IEEE80211_ADDR_LEN];      /* BSSID */
	uint8_t        init_sa[IEEE80211_ADDR_LEN];    /* Initiator STA MAC address */
	uint8_t        resp_sa[IEEE80211_ADDR_LEN];    /* Responder STA MAC address */
} __packed;

/* TDLS Wakeup Schedule information element (7.3.2.63) */
struct ieee80211_tdls_wkup_sched {
	uint8_t        id;             /* IEEE80211_ELEMID_TDLS_WKUP_SCHED */
	uint8_t        len;            /* 20 */
	uint32_t       offset;         /* Offset from TSF 0 */
	uint32_t       interval;       /* Microsecs between awake windows */
	uint32_t       awake_slots;    /* Awake window slots */
	uint32_t       awake_dur;      /* Max Awake Window Duration */
	uint16_t       idle_count;     /* Idle Count */
} __packed;

/* Extender Role IE */
struct ieee80211_ie_qtn_extender {
	uint8_t id;		/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;		/* 5 */
	uint8_t qtn_ie_oui[3];	/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t qtn_ie_type;	/* QTN_OUI_EXTENDER_ROLE */
	uint8_t role;		/* extender device role */
} __packed;

/* TDLS IE */
struct ieee80211_ie_qtn_tdls_sta_info {
	uint8_t id;		/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;		/* 6 */
	uint8_t qtn_ie_oui[3];	/* QTN_OUI - 0x00, 0x26, 0x86 */
	uint8_t qtn_ie_type;	/* QTN_OUI_TDLS */
	uint16_t sta_associd;	/* station's AID, unique value at BSS */
} __packed;

/* TDLS Channel Switch Timing element (7.3.2.64) */
struct ieee80211_tdls_cs_timing {
	uint8_t        id;             /* IEEE80211_ELEMID_TDLS_CS_TIMING */
	uint8_t        len;            /* 6 */
	uint16_t       switch_time;    /* Microsecs to switch channels */
	uint16_t       switch_timeout; /* Microsecs to timeout channel switch */
} __packed;

/* TDLS PTI Control element (7.3.2.65) */
struct ieee80211_tdls_pti_ctrl {
	uint8_t        id;             /* IEEE80211_ELEMID_TDLS_PTI_CTRL */
	uint8_t        len;            /* 5 */
	uint16_t       tid;            /* TID in last mpdu to pu sleep sta */
	uint16_t       seq_ctrl;       /* Seq ctrl in last mpdu to sleep sta */
} __packed;

/* TDLS PU Buffer Status element (7.3.2.66) */
struct ieee80211_tdls_pu_buf_stat {
	uint8_t        id;             /* IEEE80211_ELEMID_TDLS_PU_BUF_STAT */
	uint8_t        len;            /* 3 */
	uint8_t        pu_buf_stat;    /* PU buffer status flags */
} __packed;

/* Extender Role IE */
struct ieee80211_qtn_ext_role {
	uint8_t id;				/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;				/* 5 */
	uint8_t qtn_ie_oui[3];			/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t qtn_ie_type;			/* QTN_OUI_EXTENDER_ROLE */
	uint8_t role;				/* extender device role: MBS, RBS, NONE */
} __packed;

#define QTN_MAX_RBS_NUM		8
struct ieee80211_qtn_ext_bssid {
	uint8_t id;				/* IEEE80211_ELEMID_VENDOR */
	uint8_t len;				/* 59 */
	uint8_t qtn_ie_oui[3];			/* QTN_OUI - 0x00, 0x26, 0x86*/
	uint8_t qtn_ie_type;			/* QTN_OUI_EXTENDER_BSSID */
	uint8_t mbs_bssid[IEEE80211_ADDR_LEN];	/* BSSID of mbs */
	uint8_t rbs_num;
	uint8_t rbs_bssid[QTN_MAX_RBS_NUM][IEEE80211_ADDR_LEN]; /* BSSID of rbs */
} __packed;

/*
 * 802.11n AMPDU delimiters and frame structure
 */

/* XXX - Endianness?  */
struct ieee80211_ampdu_delim {
	uint8_t	dl_mpdulen[2];		/* only 12 bits */
	uint8_t	dl_crc;
	uint8_t	dl_uniquepat;
} __packed;

#define IEEE80211_AMPDU_DLPAT		0x4E	/* ASCII for char 'N' */
#define	IEEE80211_AMPDU_PADMAX		3

/*
 * 802.11n HT Capability IE
 */
struct ieee80211_ie_htcap {
	uint8_t	hc_id;			/* element ID */
	uint8_t	hc_len;			/* length in bytes */
	uint8_t	hc_cap[2];			/* HT capabilities */
	uint8_t	hc_ampdu;		/* A-MPDU parameters */
	uint8_t	hc_mcsset[16];		/* supported MCS set */
	uint8_t	hc_extcap[2];		/* extended HT capabilities */
	uint8_t	hc_txbf[4];		/* txbf capabilities */
	uint8_t	hc_antenna;		/* antenna capabilities */
} __packed;


/* HT capability flags */
#define	IEEE80211_HTCAP_C_LDPCCODING		0x0001
#define	IEEE80211_HTCAP_C_CHWIDTH40		0x0002
#define	IEEE80211_HTCAP_C_GREENFIELD		0x0010
#define IEEE80211_HTCAP_C_SHORTGI20		0x0020
#define IEEE80211_HTCAP_C_SHORTGI40		0x0040
#define IEEE80211_HTCAP_C_TXSTBC		0x0080
#define IEEE80211_HTCAP_C_RXSTBC		0x0100
#define IEEE80211_HTCAP_C_DELAYEDBLKACK		0x0400
#define IEEE80211_HTCAP_C_MAXAMSDUSIZE_8K	0x0800  /* 1 = 8K, 0 = 3839 bytes */
#define IEEE80211_HTCAP_C_DSSSCCK40		0x1000
#define IEEE80211_HTCAP_C_PSMP			0x2000
#define IEEE80211_HTCAP_C_40_INTOLERANT		0x4000
#define IEEE80211_HTCAP_C_LSIGTXOPPROT		0x8000

/* STBC defines */
#define IEEE80211_MAX_TX_STBC_SS		2

/* MCS set flags */
#define IEEE80211_HTCAP_MCS_TX_SET_DEFINED	0x01
#define IEEE80211_HTCAP_MCS_TX_RX_SET_NEQ	0x02
#define IEEE80211_HTCAP_MCS_TX_UNEQ_MOD		0x10

/* Maximum MSDU sizes */
#define	IEEE80211_MSDU_SIZE_7935			7935
#define	IEEE80211_MSDU_SIZE_3839			3839


#define IEEE80211_HT_MCS_SET_BPSK_CR_HALF		0x01
#define IEEE80211_HT_MCS_SET_QPSK_CR_HALF		0x02
#define IEEE80211_HT_MCS_SET_QPSK_CR_THREEFORTH	0x04
#define IEEE80211_HT_MCS_SET_16QAM_CR_HALF		0x08
#define IEEE80211_HT_MCS_SET_16QAM_CR_THREEFORTH	0x10
#define IEEE80211_HT_MCS_SET_64QAM_CR_TWOTHIRD	0x20
#define IEEE80211_HT_MCS_SET_64QAM_CR_THREEFORTH	0x40
#define IEEE80211_HT_MCS_SET_64QAM_CR_FIVESIXTH	0x80

/* Extended capabilities flags */
#define IEEE80211_HTCAP_E_PCO				0x0001
#define IEEE80211_HTCAP_E_PLUS_HTC			0x0400
#define IEEE80211_HTCAP_E_RD_RESPONSE		0x0800

/* Tx Beamforming flags */
#define IEEE80211_HTCAP_B_IMP_TXBF_RX		0x00000001
#define IEEE80211_HTCAP_B_STAG_SOUNDING_RX	0x00000002
#define IEEE80211_HTCAP_B_STAG_SOUNDING_TX	0x00000004
#define IEEE80211_HTCAP_B_NDP_RX			0x00000008
#define IEEE80211_HTCAP_B_NDP_TX			0x00000010
#define IEEE80211_HTCAP_B_IMP_TXBF_TX		0x00000020
#define IEEE80211_HTCAP_B_EXP_CSI_TXBF		0x00000100
#define IEEE80211_HTCAP_B_EXP_NCOMP_STEER	0x00000200
#define IEEE80211_HTCAP_B_EXP_COMP_STEER	0x00000400

/* Antenna selection flags */
#define IEEE80211_HTCAP_A_ASEL_CAPABLE		0x01
#define IEEE80211_HTCAP_A_EXP_CSI_FB_ASEL	0x02
#define IEEE80211_HTCAP_A_ANT_IND_FB_ASEL	0x04
#define IEEE80211_HTCAP_A_EXP_CSI_FB		0x08
#define IEEE80211_HTCAP_A_ANT_IND_FB		0x10
#define IEEE80211_HTCAP_A_RX_ASEL			0x20
#define IEEE80211_HTCAP_A_TX_SOUNDING_PPDU	0x40

/* 11 AC related defines */
#define IEEE80211_11AC_MCS_VAL_ERR		-1
#define IEEE80211_HT_EQUAL_MCS_START		0
#define IEEE80211_HT_EQUAL_MCS_2SS_MAX		15
#define IEEE80211_EQUAL_MCS_32			32
#define IEEE80211_UNEQUAL_MCS_START		33
#define IEEE80211_HT_UNEQUAL_MCS_2SS_MAX	38
#define IEEE80211_UNEQUAL_MCS_MAX		76
#define IEEE80211_UNEQUAL_MCS_BIT		0x40
#define IEEE80211_AC_MCS_MASK			0xFF
#define IEEE80211_AC_MCS_SHIFT			8
#define IEEE80211_AC_MCS_VAL_MASK		0x0F
#define IEEE80211_AC_MCS_NSS_MASK		0xF0
#define IEEE80211_11AC_MCS_NSS_SHIFT		4
#define IEEE80211_AC_MCS_MAX			10
#define IEEE80211_AC_MCS_NSS_MAX		4

/* B0-1 maximum rx A-MPDU factor 2^(13+Max Rx A-MPDU Factor) - 1 */
enum {
	IEEE80211_HTCAP_MAXRXAMPDU_8191,	/* (2 ^ 13) - 1*/
	IEEE80211_HTCAP_MAXRXAMPDU_16383,   /* (2 ^ 14) - 1 */
	IEEE80211_HTCAP_MAXRXAMPDU_32767,   /* (2 ^ 15) - 1*/
	IEEE80211_HTCAP_MAXRXAMPDU_65535,   /* (2 ^ 16) - 1*/
};

/* B2-4 MPDU spacing (usec) */
enum {
	IEEE80211_HTCAP_MPDUSPACING_NA,		/* No time restriction */
	IEEE80211_HTCAP_MPDUSPACING_0_25,   /* 1/4 usec */
	IEEE80211_HTCAP_MPDUSPACING_0_5,    /* 1/2 usec */
	IEEE80211_HTCAP_MPDUSPACING_1,      /* 1 usec */
	IEEE80211_HTCAP_MPDUSPACING_2,      /* 2 usec */
	IEEE80211_HTCAP_MPDUSPACING_4,      /* 4 usec */
	IEEE80211_HTCAP_MPDUSPACING_8,      /* 8 usec */
	IEEE80211_HTCAP_MPDUSPACING_16,     /* 16 usec */
};

/*
 * Rx MCS set
 * # Supported rates IE is a 10 octet bitmap - also see mcs_stream_map[]
 * Octet: 0        1        2        3        4 UEQM1  5 UEQM2  6 UEQM3  7 UEQM4  8 UEQM5  9 UEQM6
 * NSS:   11111111 22222222 33333333 44444444 02222223 33333333 33333444 44444444 44444444 44444...
 * MCS:   0        8        16       24       32       40       48       56       64       72  76
 */
enum {
	IEEE80211_HT_MCSSET_20_40_NSS1,		/* CBW = 20/40 MHz, Nss = 1, Nes = 1, EQM/ No EQM */
	IEEE80211_HT_MCSSET_20_40_NSS2,		/* CBW = 20/40 MHz, Nss = 2, Nes = 1, EQM */
	IEEE80211_HT_MCSSET_20_40_NSS3,		/* CBW = 20/40 MHz, Nss = 3, Nes = 1, EQM */
	IEEE80211_HT_MCSSET_20_40_NSS4,		/* CBW = 20/40 MHz, Nss = 4, Nes = 1, EQM */
	IEEE80211_HT_MCSSET_20_40_UEQM1,	/* MCS 32 and UEQM MCSs 33 - 39 */
	IEEE80211_HT_MCSSET_20_40_UEQM2,	/* UEQM MCSs 40 - 47 */
	IEEE80211_HT_MCSSET_20_40_UEQM3,        /* UEQM MCSs 48 - 55 */
	IEEE80211_HT_MCSSET_20_40_UEQM4,        /* UEQM MCSs 56 - 63 */
	IEEE80211_HT_MCSSET_20_40_UEQM5,        /* UEQM MCSs 64 - 71 */
	IEEE80211_HT_MCSSET_20_40_UEQM6,        /* UEQM MCSs 72 - 76 plus 3 reserved bits */
};

#define IEEE80211_HT_MCSSET_20_40_UEQM1_2SS	0x7E

#define IEEE80211_HT_MCSSET_20_40_UEQM1_3SS	0x80
#define IEEE80211_HT_MCSSET_20_40_UEQM2_3SS	0xFF
#define IEEE80211_HT_MCSSET_20_40_UEQM3_3SS	0x1F

#define IEEE80211_HT_MCSSET_20_40_UEQM3_4SS	0xE0
#define IEEE80211_HT_MCSSET_20_40_UEQM4_4SS	0xFF
#define IEEE80211_HT_MCSSET_20_40_UEQM5_4SS	0xFF
#define IEEE80211_HT_MCSSET_20_40_UEQM6_4SS	0x1F

#define IEEE80211_HT_HAS_2SS_UEQM_MCS(mcsset) \
		(mcsset[IEEE80211_HT_MCSSET_20_40_UEQM1] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM1_2SS)

#define IEEE80211_HT_HAS_3SS_UEQM_MCS(mcsset) \
		((mcsset[IEEE80211_HT_MCSSET_20_40_UEQM1] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM1_3SS) ||	\
		 (mcsset[IEEE80211_HT_MCSSET_20_40_UEQM2] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM2_3SS) ||	\
		 (mcsset[IEEE80211_HT_MCSSET_20_40_UEQM3] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM3_3SS))

#define IEEE80211_HT_HAS_4SS_UEQM_MCS(mcsset) \
		((mcsset[IEEE80211_HT_MCSSET_20_40_UEQM3] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM3_4SS) ||	\
		 (mcsset[IEEE80211_HT_MCSSET_20_40_UEQM4] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM4_4SS) ||	\
		 (mcsset[IEEE80211_HT_MCSSET_20_40_UEQM5] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM5_4SS) ||	\
		 (mcsset[IEEE80211_HT_MCSSET_20_40_UEQM6] &	\
			IEEE80211_HT_MCSSET_20_40_UEQM6_4SS))

#define IEEE80211_HT_IS_1SS_NODE(mcsset) \
		((mcsset[IEEE80211_HT_MCSSET_20_40_NSS1] != 0) && \
		(mcsset[IEEE80211_HT_MCSSET_20_40_NSS2] == 0))

#define IEEE80211_HT_IS_2SS_NODE(mcsset) \
		((mcsset[IEEE80211_HT_MCSSET_20_40_NSS2] != 0) && \
		(mcsset[IEEE80211_HT_MCSSET_20_40_NSS3] == 0))

#define IEEE80211_HT_IS_3SS_NODE(mcsset) \
		((mcsset[IEEE80211_HT_MCSSET_20_40_NSS3] != 0) && \
		(mcsset[IEEE80211_HT_MCSSET_20_40_NSS4] == 0))

#define IEEE80211_HT_IS_4SS_NODE(mcsset) \
		(mcsset[IEEE80211_HT_MCSSET_20_40_NSS4] != 0)

/* B2-3 Maximum Tx spatial streams */
enum {
	IEEE80211_HTCAP_MCS_ONE_TX_SS,		/* One spatial stream */
	IEEE80211_HTCAP_MCS_TWO_TX_SS,		/* Two spatial streams */
	IEEE80211_HTCAP_MCS_THREE_TX_SS,	/* Three spatial streams */
	IEEE80211_HTCAP_MCS_FOUR_TX_SS		/* Four spatial streams */
};

/* B2-3 power save mode */
enum {
	IEEE80211_HTCAP_C_MIMOPWRSAVE_STATIC = 0,	/* No MIMO (static mode) */
	IEEE80211_HTCAP_C_MIMOPWRSAVE_DYNAMIC,		/* Precede MIMO with RTS */
	IEEE80211_HTCAP_C_MIMOPWRSAVE_NA,		/* Not applicable        */
	IEEE80211_HTCAP_C_MIMOPWRSAVE_NONE		/* No limitation on MIMO (SM power save disabled) */
};

/* B8-9 Rx STBC Mode */
enum {
	IEEE80211_HTCAP_C_RXSTBC_NONE,			/* No STBC SS */
	IEEE80211_HTCAP_C_RXSTBC_ONE_SS,		/* One STBC SS */
	IEEE80211_HTCAP_C_RXSTBC_TWO_SS,		/* Two STBC SS */
	IEEE80211_HTCAP_C_RXSTBC_THREE_SS		/* Three STBC SS */
};

/* B1-2 PCO transition time */
enum {
	IEEE80211_HTCAP_E_PCO_NONE,				/* No transition */
	IEEE80211_HTCAP_E_PCO_FOUR_HUNDRED_US,	/* 400 us */
	IEEE80211_HTCAP_E_PCO_ONE_HALF_MS,		/* 1.5 ms */
	IEEE80211_HTCAP_E_PCO_FIVE_MS			/* 5 ms */
};

/* B8-9 MCS feedback */
enum {
	IEEE80211_HTCAP_E_MCS_FB_NONE,			/* No feedback */
	IEEE80211_HTCAP_E_MCS_FB_NA,			/* Reserved */
	IEEE80211_HTCAP_E_MCS_FB_UNSOLICITED,	/* Unsolicited feedback only*/
	IEEE80211_HTCAP_E_MCS_FB_SOLICITED		/* Solicited and unsolicited feedback */
};

/* B6-7 Calibration */
enum {
	IEEE80211_HTCAP_B_CALIBRATION_NONE,			/* No support */
	IEEE80211_HTCAP_B_CALIBRATION_RESP_ONLY,	/* Response only */
	IEEE80211_HTCAP_B_CALIBRATION_NA,			/* Reserved */
	IEEE80211_HTCAP_B_CALIBRATION_REQ_RESP		/* Request and response */
};

/* B11-12 explicit CSI TxBF feedback, B13-14 explicit non compressed TxBF,
 * B15-16 explicit compressed TxBF
 */
enum {
	IEEE80211_HTCAP_B_CAPABLE_NONE,			/* No support */
	IEEE80211_HTCAP_B_CAPABLE_DELAYED,		/* delayed response only */
	IEEE80211_HTCAP_B_CAPABLE_IMMEDIATE,	/* immediate response only */
	IEEE80211_HTCAP_B_CAPABLE_BOTH			/* both delayed and immediate response */
};

/* B17-18 Grouping */
enum {
	IEEE80211_HTCAP_B_GROUPING_NONE,		/* No support */
	IEEE80211_HTCAP_B_GROUPING_ONE_TWO,		/* groups 1 and 2 */
	IEEE80211_HTCAP_B_GROUPING_ONE_FOUR,	/* groups 1 and 4 */
	IEEE80211_HTCAP_B_GROUPING_ONE_TWO_FOUR	/* groups 1, 2 and 4 */
};

/* B19-20 CSI number of beamforming antennas, B21-22 non compressed number of beamforming
 * antennas, B23-24 compressed number of beamforming antennas
 */
enum {
	IEEE80211_HTCAP_B_ANTENNAS_ONE,		/* Single antenna sounding */
	IEEE80211_HTCAP_B_ANTENNAS_TWO,		/* 2 antenna sounding */
	IEEE80211_HTCAP_B_ANTENNAS_THREE,	/* 3 antenna sounding */
	IEEE80211_HTCAP_B_ANTENNAS_FOUR		/* 4 antenna sounding */
};

/* B25-26 CSI Max number of beamformer rows */
enum {
	IEEE80211_HTCAP_B_CSI_ONE_ROW,
	IEEE80211_HTCAP_B_CSI_TWO_ROWS,
	IEEE80211_HTCAP_B_CSI_THREE_ROWS,
	IEEE80211_HTCAP_B_CSI_FOUR_ROWS
};

/* B27-28 channel estimation capability */
enum {
	IEEE80211_HTCAP_B_ST_STREAM_ONE,	/* one space time stream */
	IEEE80211_HTCAP_B_ST_STREAM_TWO,	/* two space time streams */
	IEEE80211_HTCAP_B_ST_STREAM_THREE,	/* three space time streams */
	IEEE80211_HTCAP_B_ST_STREAM_FOUR	/* four space time streams */
};

/* HT NSS */
enum ieee80211_ht_nss {
	IEEE80211_HT_NSS1 = 1,
	IEEE80211_HT_NSS2 = 2,
	IEEE80211_HT_NSS3 = 3,
	IEEE80211_HT_NSS4 = 4
};

/* HT capability macros */

/* get macros */
/* A-MPDU spacing  B2-B4 */
#define IEEE80211_HTCAP_MIN_AMPDU_SPACING(htcap) \
	(((htcap)->hc_ampdu & 0x1c) >> 2)
/* max RX A-MPDU length  B0-B1 */
#define IEEE80211_HTCAP_MAX_AMPDU_LEN(htcap) \
	(((htcap)->hc_ampdu & 0x03))
/* highest supported data rate, B0-B7 in set 10, B0-B1 in set 11 */
#define IEEE80211_HTCAP_HIGHEST_DATA_RATE(htcap) \
	(((htcap)->hc_mcsset[10]) | (((htcap)->hc_mcsset[11] & 0x3) << 8))
/* MCS parameters (all bits)*/
#define IEEE80211_HTCAP_MCS_PARAMS(htcap) \
	((htcap)->hc_mcsset[12] & 0x1F)
/* MCS maximum spatial streams, B2-B3 in set 12 */
#define IEEE80211_HTCAP_MCS_STREAMS(htcap) \
	(((htcap)->hc_mcsset[12] & 0xC) >> 2)
/* MCS set value (all bits) */
#define IEEE80211_HTCAP_MCS_VALUE(htcap,_set) \
	((htcap)->hc_mcsset[_set])
/* HT capabilities (all bits) */
#define IEEE80211_HTCAP_CAPABILITIES(htcap) \
	(((htcap)->hc_cap[0]) | ((htcap)->hc_cap[1] << 8))
/* B3-4 power save mode */
#define IEEE80211_HTCAP_PWRSAVE_MODE(htcap) \
	(((htcap)->hc_cap[0] & 0x0C) >> 2)
/* B8-9 Rx STBC MODE */
#define IEEE80211_HTCAP_RX_STBC_MODE(htcap) \
	((htcap)->hc_cap[1] & 0x3)
/* HT extended capabilities (all bits) */
#define IEEE80211_HTCAP_EXT_CAPABILITIES(htcap) \
	((htcap)->hc_extcap)
/* B1-2 PCO transition time */
#define IEEE80211_HTCAP_PCO_TRANSITION(htcap) \
	(((htcap)->hc_extcap & 0x6) >> 1)
/* B8-9 MCS feedback type */
#define IEEE80211_HTCAP_MCS_FEEDBACK_TYPE(htcap) \
	(((htcap)->hc_extcap & 0x300) >> 8)
/* HT TxBeamForming (bits 0-13) */
#define IEEE80211_HTCAP_TXBF_CAPABILITIES(htcap) \
	((htcap)->hc_txbf[0] | ((htcap)->hc_txbf[1] << 8))
/* HT TxBeamForming (bits 14-31) */
#define IEEE80211_HTCAP_TXBF_CAPABILITIES_EXTN(htcap) \
	((htcap)->hc_txbf[2] | ((htcap)->hc_txbf[3] << 8))
/* B6-7 Calibration */
#define IEEE80211_HTCAP_CALIBRATION(htcap) \
	(((htcap)->hc_txbf[0] & 0xC0) >> 6)
/* B11-12 explicit CSI TxBF feedback*/
#define IEEE80211_HTCAP_EXP_CSI_TXBF(htcap) \
	(((htcap)->hc_txbf[1] & 0x18) >> 3)
/* B13-14 explicit non compressed TxBF */
#define IEEE80211_HTCAP_EXP_NCOMP_TXBF(htcap) \
	(((htcap)->hc_txbf[1] & 0x60) >> 5)
/* B15-16 explicit compressed TxBF */
#define IEEE80211_HTCAP_EXP_COMP_TXBF(htcap) \
	((((htcap)->hc_txbf[1] & 0x80) >> 7) | (((htcap)->hc_txbf[2] & 0x01) << 1))
/* B17-18 Grouping */
#define IEEE80211_HTCAP_GROUPING(htcap) \
	(((htcap)->hc_txbf[2] & 0x6) >> 1)
/* B19-20 CSI number of beamforming antennas */
#define IEEE80211_HTCAP_CSI_NUM_BF(htcap) \
	(((htcap)->hc_txbf[2] & 0x18) >> 3)
/* B21-22 non compressed number of beamforming antennas */
#define IEEE80211_HTCAP_NCOM_NUM_BF(htcap) \
	(((htcap)->hc_txbf[2] & 0x60) >> 5)
/* B23-24 compressed number of beamforming antennas */
#define IEEE80211_HTCAP_COMP_NUM_BF(htcap) \
	((((htcap)->hc_txbf[2] & 0x80) >> 7) | (((htcap)->hc_txbf[3] & 0x01) << 1))
/* B25-26 CSI Max number of beamformer rows */
#define IEEE80211_HTCAP_CSI_BF_ROWS(htcap) \
	(((htcap)->hc_txbf[3] & 0x6) >> 1)
/* B27-28 channel estimation capability */
#define IEEE80211_HTCAP_CHAN_EST(htcap) \
	(((htcap)->hc_txbf[3] & 0x18) >> 3)

/* set macros */
/* A-MPDU spacing  B2-B4 */
#define IEEE80211_HTCAP_SET_AMPDU_SPACING(htcap,_d) \
	((htcap)->hc_ampdu = (((htcap)->hc_ampdu & ~0x1c)  | ((_d) << 2)))
/* max RX A-MPDU length  B0-B1 */
#define IEEE80211_HTCAP_SET_AMPDU_LEN(htcap,_f)	\
	((htcap)->hc_ampdu = (((htcap)->hc_ampdu & ~0x03)  | (_f)))
/* highest supported data rate, B0-B7 in set 10, B0-B1 in set 11) */
#define IEEE80211_HTCAP_SET_HIGHEST_DATA_RATE(htcap,_r) \
	((htcap)->hc_mcsset[10] = ((_r) & 0xFF)); \
	((htcap)->hc_mcsset[11] = ((_r) & 0x3FF) >> 8)
/* MCS set parameters (all bits) */
#define IEEE80211_HTCAP_SET_MCS_PARAMS(htcap,_p) \
	((htcap)->hc_mcsset[12] = (_p & 0x1F))
/* MCS maximum spatial streams, B2-B3 in set 12 */
#define IEEE80211_HTCAP_SET_MCS_STREAMS(htcap,_s) \
	((htcap)->hc_mcsset[12] = ((htcap)->hc_mcsset[12] & ~0xC)| (_s << 2))
/* MCS set value (all bits) */
#define IEEE80211_HTCAP_SET_MCS_VALUE(htcap,_set,_value) \
	((htcap)->hc_mcsset[_set] = (_value & 0xFF))
/* HT capabilities (all bits) */
#define IEEE80211_HTCAP_SET_CAPABILITIES(htcap,_cap) \
	(htcap)->hc_cap[0] = (_cap & 0x00FF); \
	(htcap)->hc_cap[1] = ((_cap & 0xFF00) >> 8)
/* B2-B3 power save mode */
#define IEEE80211_HTCAP_SET_PWRSAVE_MODE(htcap,_m) \
	((htcap)->hc_cap[0] = (((htcap)->hc_cap[0] & ~0xC) | ((_m) << 2)))
/* B8-9 Rx STBC MODE */
#define IEEE80211_HTCAP_SET_RX_STBC_MODE(htcap,_m) \
	((htcap)->hc_cap[1] = (((htcap)->hc_cap[1] & ~0x3) | (_m) ))
/* HT extended capabilities (all bits) */
#define IEEE80211_HTCAP_SET_EXT_CAPABILITIES(htcap,_cap) \
	((htcap)->hc_extcap = (_cap & 0xFFFF))
/* B1-2 PCO transition time */
#define IEEE80211_HTCAP_SET_PCO_TRANSITION(htcap,_t) \
	((htcap)->hc_extcap = (((htcap)->hc_extcap & ~0x6) | ((_t) << 1)))
/* B8-9 MCS feedback type */
#define IEEE80211_HTCAP_SET_MCS_FEEDBACK_TYPE(htcap,_t) \
	((htcap)->hc_extcap = (((htcap)->hc_extcap & ~0x300) | ((_t) << 8)))
/* HT TxBeamForming (all bits ) */
#define IEEE80211_HTCAP_SET_TXBF_CAPABILITIES(htcap,_cap) \
	(htcap)->hc_txbf[0] = ((_cap) & 0x00FF); \
	(htcap)->hc_txbf[1] = (((_cap) & 0xFF00) >> 8)
/* B6-7 Calibration */
#define IEEE80211_HTCAP_SET_CALIBRATION(htcap,_t) \
	((htcap)->hc_txbf[0] = (((htcap)->hc_txbf[0] & ~0xC0) | ((_t) << 6)))
/* B11-12 explicit CSI TxBF feedback*/
#define IEEE80211_HTCAP_SET_EXP_CSI_TXBF(htcap,_t) \
	((htcap)->hc_txbf[1] = (((htcap)->hc_txbf[1] & ~0x18) | ((_t) << 3)))
/* B13-14 explicit non compressed TxBF */
#define IEEE80211_HTCAP_SET_EXP_NCOMP_TXBF(htcap,_t) \
	((htcap)->hc_txbf[1] = (((htcap)->hc_txbf[1] & ~0x60) | ((_t) << 5)))
/* B15-16 explicit compressed TxBF */
#define IEEE80211_HTCAP_SET_EXP_COMP_TXBF(htcap,_t) \
	(htcap)->hc_txbf[1] = (((htcap)->hc_txbf[1] & ~0x80) | ((((_t) & 0x01) << 7))); \
	(htcap)->hc_txbf[2] = (((htcap)->hc_txbf[2] & ~0x01) | ((_t) >> 1))
/* B17-18 Grouping */
#define IEEE80211_HTCAP_SET_GROUPING(htcap,_t) \
	((htcap)->hc_txbf[2] = (((htcap)->hc_txbf[2] & ~0x6) | ((_t) << 1)))
/* B19-20 CSI number of beamforming antennas */
#define IEEE80211_HTCAP_SET_CSI_NUM_BF(htcap,_t) \
	((htcap)->hc_txbf[2] = (((htcap)->hc_txbf[2] & ~0x18) | ((_t) << 3)))
/* B21-22 non compressed number of beamforming antennas */
#define IEEE80211_HTCAP_SET_NCOMP_NUM_BF(htcap,_t) \
	((htcap)->hc_txbf[2] = (((htcap)->hc_txbf[2] & ~0x60) | ((_t) << 5)))
/* B23-24 compressed number of beamforming antennas */
#define IEEE80211_HTCAP_SET_COMP_NUM_BF(htcap,_t) \
	(htcap)->hc_txbf[2] = (((htcap)->hc_txbf[2] & ~0x80) | (((_t) & 0x01) << 7)); \
	(htcap)->hc_txbf[3] = (((htcap)->hc_txbf[3] & ~0x01) | ((_t) >> 1))
/* B25-26 CSI Max number of beamformer rows */
#define IEEE80211_HTCAP_SET_CSI_BF_ROWS(htcap,_t) \
	((htcap)->hc_txbf[3] = (((htcap)->hc_txbf[3] & ~0x6) | ((_t) << 1)))
/* B27-28 channel estimation capability */
#define IEEE80211_HTCAP_SET_CHAN_EST(htcap,_t) \
	((htcap)->hc_txbf[3] = (((htcap)->hc_txbf[3] & ~0x18) | ((_t) << 3)))

/*
 * 802.11n HT Information IE
 */
struct ieee80211_ie_htinfo {
	uint8_t	hi_id;			/* element ID */
	uint8_t	hi_len;			/* length in bytes */
	uint8_t	hi_ctrlchannel;	/* control channel */
	uint8_t	hi_byte1;		/* ht ie byte 1 */
	uint8_t	hi_byte2;		/* ht ie byte 2 */
	uint8_t	hi_byte3;		/* ht ie byte 3 */
	uint8_t	hi_byte4;		/* ht ie byte 4 */
	uint8_t	hi_byte5;		/* ht ie byte 5 */
	uint8_t	hi_basicmcsset[16];	/* basic MCS set */
} __packed;

#define IEEE80211_HTINFO_B1_REC_TXCHWIDTH_40	0x04
#define IEEE80211_HTINFO_B1_RIFS_MODE			0x08
#define IEEE80211_HTINFO_B1_CONTROLLED_ACCESS	0x10
#define IEEE80211_HTINFO_B2_NON_GF_PRESENT		0x04
#define IEEE80211_HTINFO_B2_OBSS_PROT			0x10
#define IEEE80211_HTINFO_B4_DUAL_BEACON			0x40
#define IEEE80211_HTINFO_B4_DUAL_CTS			0x80
#define IEEE80211_HTINFO_B5_STBC_BEACON			0x01
#define IEEE80211_HTINFO_B5_LSIGTXOPPROT		0x02
#define IEEE80211_HTINFO_B5_PCO_ACTIVE			0x04
#define IEEE80211_HTINFO_B5_40MHZPHASE			0x08


/* get macros */
/* control channel (all bits) */
#define IEEE80211_HTINFO_PRIMARY_CHANNEL(htie) \
	(htie->hi_ctrlchannel)
/* byte 1 (all bits) */
#define IEEE80211_HTINFO_BYTE_ONE(htie) \
	(htie->hi_byte1)
/* byte 2 (all bits) */
#define IEEE80211_HTINFO_BYTE_TWO(htie) \
	(htie->hi_byte2)
/* byte 3 (all bits) */
#define IEEE80211_HTINFO_BYTE_THREE(htie) \
	(htie->hi_byte3)
/* byte 4 (all bits) */
#define IEEE80211_HTINFO_BYTE_FOUR(htie) \
	(htie->hi_byte4)
/* byte 5 (all bits) */
#define IEEE80211_HTINFO_BYTE_FIVE(htie) \
	(htie->hi_byte5)
/* B5-B7, byte 1 */
#define IEEE80211_HTINFO_B1_SIGRANULARITY(htie) \
	(((htie)->hi_byte1 & 0xe0) >> 5)
/* B0-B1, byte 1 */
#define IEEE80211_HTINFO_B1_EXT_CHOFFSET(htie) \
	(((htie)->hi_byte1 & 0x3))
/* B0-B1, byte 2 */
#define IEEE80211_HTINFO_B2_OP_MODE(htie) \
	(((htie)->hi_byte2 & 0x3))
/* MCS set value (all bits) */
#define IEEE80211_HTINFO_BASIC_MCS_VALUE(htie,_set) \
	((htie)->hi_basicmcsset[_set])

/* set macros */
/* control channel (all bits) */
#define IEEE80211_HTINFO_SET_PRIMARY_CHANNEL(htie,_c) \
	(htie->hi_ctrlchannel = _c)
/* byte 1 (all bits) */
#define IEEE80211_HTINFO_SET_BYTE_ONE(htie,_b) \
	(htie->hi_byte1 = _b)
/* byte 2 (all bits) */
#define IEEE80211_HTINFO_SET_BYTE_TWO(htie,_b) \
	(htie->hi_byte2 = _b)
/* byte 3 (all bits) */
#define IEEE80211_HTINFO_SET_BYTE_THREE(htie,_b) \
	(htie->hi_byte3 = _b)
/* byte 4 (all bits) */
#define IEEE80211_HTINFO_SET_BYTE_FOUR(htie,_b) \
	(htie->hi_byte4 = _b)
/* byte 5 (all bits) */
#define IEEE80211_HTINFO_SET_BYTE_FIVE(htie,_b) \
	(htie->hi_byte5 = _b)
/* B5-B7, byte 1 */
#define IEEE80211_HTINFO_B1_SET_SIGRANULARITY(htie,_g)			\
	((htie)->hi_byte1 = (((htie)->hi_byte1 & ~0xe0)  |((_g) << 5) ))
/* B0-B1, byte 1 */
#define IEEE80211_HTINFO_B1_SET_EXT_CHOFFSET(htie,_off)					\
	((htie)->hi_byte1 = (((htie)->hi_byte1 & ~0x03)  |(_off)))
/* B0-B1, byte 2 */
#define IEEE80211_HTINFO_B2_SET_OP_MODE(htie,_m)									\
	((htie)->hi_byte2 = (((htie)->hi_byte2 & ~0x3) | ((_m) )))
/* Basic MCS set value (all bits) */
#define IEEE80211_HTINFO_SET_BASIC_MCS_VALUE(htie,_set,_value) \
	((htie)->hi_basicmcsset[_set] = (_value & 0xFF))


/* extension channel offset (2 bit signed number) */
enum {
	IEEE80211_HTINFO_EXTOFFSET_NA	 = 0,	/* 0  no extension channel is present */
	IEEE80211_HTINFO_EXTOFFSET_ABOVE = 1,   /* +1 extension channel above control channel */
	IEEE80211_HTINFO_EXTOFFSET_UNDEF = 2,   /* -2 undefined */
	IEEE80211_HTINFO_EXTOFFSET_BELOW = 3	/* -1 extension channel below control channel*/
};

/* operating mode */
enum {
	IEEE80211_HTINFO_OPMODE_NO_PROT,			/* no protection */
	IEEE80211_HTINFO_OPMODE_HT_PROT_NON_MEM,	/* protection required (Legacy device present in other BSS) */
	IEEE80211_HTINFO_OPMODE_HT_PROT_20_ONLY,	/* protection required ( One 20 MHZ only HT device is present in 20/40 BSS) */
	IEEE80211_HTINFO_OPMODE_HT_PROT_MIXED,		/* protection required (Legacy device is present in this BSS) */
};

/* signal granularity */
enum {
	IEEE80211_HTINFO_SIGRANULARITY_5,	/* 5 ms */
	IEEE80211_HTINFO_SIGRANULARITY_10,	/* 10 ms */
	IEEE80211_HTINFO_SIGRANULARITY_15,	/* 15 ms */
	IEEE80211_HTINFO_SIGRANULARITY_20,	/* 20 ms */
	IEEE80211_HTINFO_SIGRANULARITY_25,	/* 25 ms */
	IEEE80211_HTINFO_SIGRANULARITY_30,	/* 30 ms */
	IEEE80211_HTINFO_SIGRANULARITY_35,	/* 35 ms */
	IEEE80211_HTINFO_SIGRANULARITY_40,	/* 40 ms */
};

/*
 * Management information element payloads.
 */

enum {
	IEEE80211_ELEMID_SSID		= 0,
	IEEE80211_ELEMID_RATES		= 1,
	IEEE80211_ELEMID_FHPARMS	= 2,
	IEEE80211_ELEMID_DSPARMS	= 3,
	IEEE80211_ELEMID_CFPARMS	= 4,
	IEEE80211_ELEMID_TIM		= 5,
	IEEE80211_ELEMID_IBSSPARMS	= 6,
	IEEE80211_ELEMID_COUNTRY	= 7,
	IEEE80211_ELEMID_REQINFO	= 10,
	IEEE80211_ELEMID_BSS_LOAD	= 11,
	IEEE80211_ELEMID_EDCA		= 12,
	IEEE80211_ELEMID_CHALLENGE	= 16,
	/* 17-31 reserved for challenge text extension */
	IEEE80211_ELEMID_PWRCNSTR	= 32,
	IEEE80211_ELEMID_PWRCAP		= 33,
	IEEE80211_ELEMID_TPCREQ		= 34,
	IEEE80211_ELEMID_TPCREP		= 35,
	IEEE80211_ELEMID_SUPPCHAN	= 36,
	IEEE80211_ELEMID_CHANSWITCHANN	= 37,
	IEEE80211_ELEMID_MEASREQ	= 38,
	IEEE80211_ELEMID_MEASREP	= 39,
	IEEE80211_ELEMID_QUIET		= 40,
	IEEE80211_ELEMID_IBSSDFS	= 41,
	IEEE80211_ELEMID_ERP		= 42,
	IEEE80211_ELEMID_HTCAP		= 45,
	IEEE80211_ELEMID_QOSCAP		= 46,
	IEEE80211_ELEMID_RSN		= 48,
	IEEE80211_ELEMID_XRATES		= 50,
	IEEE80211_ELEMID_NEIGHBOR_REP	= 52,
	IEEE80211_ELEMID_FTIE		= 55,
	IEEE80211_ELEMID_TIMEOUT_INT	= 56,
	IEEE80211_ELEMID_REG_CLASSES	= 59,
	IEEE80211_ELEMID_HTINFO		= 61,
	IEEE80211_ELEMID_SEC_CHAN_OFF	= 62,	/* Secondary Channel Offset */
	IEEE80211_ELEMID_20_40_BSS_COEX = 72,	/* 20/40 BSS Coexistence */
	IEEE80211_ELEMID_TDLS_LINK_ID	   = 101, /* TDLS Link Identifier */
	IEEE80211_ELEMID_TDLS_WKUP_SCHED   = 102, /* TDLS Wakeup Schedule */
	IEEE80211_ELEMID_TDLS_CS_TIMING    = 104, /* TDLS Channel Switch Timing */
	IEEE80211_ELEMID_TDLS_PTI_CTRL	   = 105, /* TDLS PTI Control */
	IEEE80211_ELEMID_TDLS_PU_BUF_STAT  = 106, /* TDLS PU Buffer Status */
	IEEE80211_ELEMID_INTERWORKING	= 107,
	IEEE80211_ELEMID_EXTCAP		= 127,
	/* 128-129 proprietary elements used by Agere chipsets */
	IEEE80211_ELEMID_AGERE1		= 128,
	IEEE80211_ELEMID_AGERE2		= 129,
	IEEE80211_ELEMID_TPC		= 150,
	IEEE80211_ELEMID_CCKM		= 156,
	/* 191-199 Table 8-54-Element IDs in Std 802.11ac-2013 */
	IEEE80211_ELEMID_VHTCAP		= 191,
	IEEE80211_ELEMID_VHTOP		= 192,
	IEEE80211_ELEMID_EXTBSSLOAD	= 193,
	IEEE80211_ELEMID_WBWCHANSWITCH	= 194,
	IEEE80211_ELEMID_VHTXMTPWRENVLP	= 195,
	IEEE80211_ELEMID_CHANSWITCHWRP	= 196,
	IEEE80211_ELEMID_AID		= 197,
	IEEE80211_ELEMID_QUIETCHAN	= 198,
	IEEE80211_ELEMID_OPMOD_NOTIF	= 199,
	/* Vendor Specific */
	IEEE80211_ELEMID_VENDOR		= 221,	/* vendor private */
};

#define IEEE80211_2040BSSCOEX_INFO_REQ	0x01
#define IEEE80211_2040BSSCOEX_40_intol	0x02
#define IEEE80211_2040BSSCOEX_20_REQ	0x04
#define IEEE80211_2040BSSCOEX_SCAN_EXEP_REQ	0x08
#define IEEE80211_2040BSSCOEX_SCAN_EXEP_GRA	0x10

#define IEEE80211_CHANSWITCHANN_BYTES 5
#define QTN_CHANSWITCHANN_TSF_BYTES 10
#define IEEE80211_CSA_LEN	7
#define IEEE80211_CSA_TSF_LEN	(IEEE80211_CSA_LEN + 10)
#define IEEE80211_SEC_CHAN_OFF_IE_LEN 3
#define IEEE80211_WBAND_CHANSWITCH_IE_LEN 5
#define IEEE80211_NCW_ACT_LEN   3      /* Notify Channel Width Action size */
#define IEEE80211_MU_GRP_ID_ACT_LEN 26 /* MU grp id mgmt action size */

#define IEEE80211_NODE_IDX_UNMAP(x)	(BR_SUBPORT_UNMAP(x))
#define IEEE80211_NODE_IDX_MAP(x)	(BR_SUBPORT_MAP(x))
#define IEEE80211_NODE_IDX_VALID(x)	((x) & 0x8000)
#define IEEE80211_NODE_IDX_INVALID(x)	(!IEEE80211_NODE_IDX_VALID(x))

/*
 * The 802.11 spec says at most 2007 stations may be
 * associated at once.  For most AP's this is way more
 * than is feasible so we use a default of 128.  This
 * number may be overridden by the driver and/or by
 * user configuration.
 */
#define	IEEE80211_AID_MAX		2007
#define	IEEE80211_AID_DEF		128

#define	IEEE80211_AID(b)	((b) &~ 0xc000)

struct ieee80211_tim_ie {
	uint8_t	tim_ie;			/* IEEE80211_ELEMID_TIM */
	uint8_t	tim_len;
	uint8_t	tim_count;		/* DTIM count */
	uint8_t	tim_period;		/* DTIM period */
	uint8_t	tim_bitctl;		/* bitmap control */
	uint8_t	tim_bitmap[IEEE80211_AID_DEF / NBBY];		/* variable-length bitmap */
} __packed;

struct ieee80211_ie_sec_chan_off {
	uint8_t	sco_id;			/* IEEE80211_ELEMID_SEC_CHAN_OFF */
	uint8_t	sco_len;
	uint8_t	sco_off;		/* offset */
} __packed;

struct ieee80211_country_ie {
	uint8_t	ie;			/* IEEE80211_ELEMID_COUNTRY */
	uint8_t	len;
	uint8_t	cc[3];			/* ISO CC+(I)ndoor/(O)utdoor */
	struct {
		uint8_t schan;			/* starting channel */
		uint8_t nchan;			/* number channels */
		uint8_t maxtxpwr;		/* tx power cap */
	} __packed band[4];			/* up to 4 sub bands */
} __packed;

#define IEEE80211_CHALLENGE_LEN		128

#define IEEE80211_SUPPCHAN_LEN		26

#define	IEEE80211_RATE_BASIC		0x80
#define	IEEE80211_RATE_VAL			0x7f
#define IEEE80211_BSS_MEMBERSHIP_SELECTOR	0x7F

/* EPR information element flags */
#define	IEEE80211_ERP_NON_ERP_PRESENT	0x01
#define	IEEE80211_ERP_USE_PROTECTION	0x02
#define	IEEE80211_ERP_LONG_PREAMBLE	0x04

/* Atheros private advanced capabilities info */
#define	ATHEROS_CAP_TURBO_PRIME		0x01
#define	ATHEROS_CAP_COMPRESSION		0x02
#define	ATHEROS_CAP_FAST_FRAME		0x04
/* bits 3-6 reserved */
#define	ATHEROS_CAP_BOOST		0x80

#define IEEE80211_OUI_LEN	3

#define	ATH_OUI			0x7f0300	/* Atheros OUI */
#define	ATH_OUI_TYPE		0x01
#define	ATH_OUI_SUBTYPE		0x01
#define ATH_OUI_VERSION		0x00
#define	ATH_OUI_TYPE_XR		0x03
#define	ATH_OUI_VER_XR		0x01

#define	QTN_OUI			0x862600	/* Quantenna OUI */
#define	QTN_OUI_CFG		0x01
#define QTN_OUI_PAIRING		0x02		/* Pairing Protection */
#define	QTN_OUI_VSP_CTRL	0x03		/* VSP configuration */
#define	QTN_OUI_TDLS_BRMACS	0x04		/* TDLS */
#define QTN_OUI_TDLS		0x05		/* TDLS Information */
#define	QTN_OUI_RM_SPCIAL	0x10		/* Radio measurement special group */
#define	QTN_OUI_RM_ALL		0x11		/* Radio measurement all group */
#define QTN_OUI_SCS             0x12            /* SCS status report and control */
#define QTN_OUI_QWME            0x13            /* WME IE between QSTA */
#define QTN_OUI_EXTENDER_ROLE	0x14		/* WDS Extender Role */

#define QTN_OUI_EXTENDER_BSSID	0x15	/* Extender BSSID */

#define QTN_OUI_EXTENDER_ROLE_NONE	0x00	/* NONE Role */
#define QTN_OUI_EXTENDER_ROLE_MBS	0x01	/* MBS Role */
#define QTN_OUI_EXTENDER_ROLE_RBS	0x02	/* RBS Role */

#define QTN_QWME_IE_VERSION	1

#define	WPA_OUI			0xf25000
#define	WPA_OUI_TYPE		0x01
#define	WSC_OUI_TYPE		0x04
#define	WPA_VERSION		1		/* current supported version */

#define	WPA_CSE_NULL		0x00
#define	WPA_CSE_WEP40		0x01
#define	WPA_CSE_TKIP		0x02
#define	WPA_CSE_CCMP		0x04
#define	WPA_CSE_WEP104		0x05
#define	RSN_CSE_GROUP_NOT_ALLOW 0x07  /* Group addressed traffic not allowed */

#define	WPA_ASE_NONE		0x00
#define	WPA_ASE_8021X_UNSPEC	0x01
#define	WPA_ASE_8021X_PSK	0x02
#define	IEEE80211_RSN_ASE_TPK	0x07  /* TDLS TPK Handshake */

#define	RSN_OUI			0xac0f00
#define	RSN_VERSION		1		/* current supported version */

#define	BCM_OUI			0x4C9000	/* Apple Products */
#define	BCM_OUI_TYPE		0x01

#define	BCM_OUI_2		0x181000	/* iPad */
#define	BCM_OUI_2_TYPE		0x02


#define	RSN_CSE_NULL		0x00
#define	RSN_CSE_WEP40		0x01
#define	RSN_CSE_TKIP		0x02
#define	RSN_CSE_WRAP		0x03
#define	RSN_CSE_CCMP		0x04
#define	RSN_CSE_WEP104		0x05
#define	RSN_CSE_BIP		0x06

#define	RSN_ASE_NONE		0x00
#define	RSN_ASE_8021X_UNSPEC	0x01
#define	RSN_ASE_8021X_PSK	0x02
#define	RSN_ASE_8021X_SHA256	0x05
#define	RSN_ASE_8021X_PSK_SHA256 0x06

#define	RSN_CAP_PREAUTH		0x01
#define	RSN_CAP_MFP_REQ		0x0040
#define	RSN_CAP_MFP_CAP		0x0080
#define	RSN_CAP_SPP_CAP		0x0400
#define	RSN_CAP_SPP_REQ		0x0800

#define RSN_IS_MFP(_rsn_caps) (((_rsn_caps) & RSN_CAP_MFP_REQ) || ((_rsn_caps) & RSN_CAP_MFP_CAP))

#define	WME_OUI			0xf25000
#define	WME_OUI_TYPE		0x02
#define	WME_INFO_OUI_SUBTYPE	0x00
#define	WME_PARAM_OUI_SUBTYPE	0x01
#define	WME_VERSION		1
#define	WME_UAPSD_MASK		0x0f

#define RLNK_OUI		0x430C00	/* Ralink OUI */

#define RTK_OUI			0x4ce000	/* Realtek OUI */
#define EDIMAX_OUI		0x021f80	/* Edimax OUI */

#define PEER_VENDOR_NONE	0x00
#define PEER_VENDOR_QTN		0x01
#define PEER_VENDOR_BRCM	0x02
#define PEER_VENDOR_ATH		0x04
#define PEER_VENDOR_RLNK	0x08
#define PEER_VENDOR_RTK		0x10
#define PEER_VENDOR_INTEL	0x20

/*
 * 802.11ac VHT Capabilities element
 */
struct ieee80211_ie_vhtcap {
	u_int8_t	vht_id;			/* element ID */
	u_int8_t	vht_len;		/* length in bytes */
	u_int8_t	vht_cap[4];		/* VHT capabilities info */
	u_int8_t	vht_mcs_nss_set[8];	/* supported MSC and NSS set */
} __packed;

/* VHT capabilities flags */
#define IEEE80211_VHTCAP_C_RX_LDPC			0x00000010
#define IEEE80211_VHTCAP_C_SHORT_GI_80			0x00000020
#define IEEE80211_VHTCAP_C_SHORT_GI_160			0x00000040
#define IEEE80211_VHTCAP_C_TX_STBC			0x00000080
#define IEEE80211_VHTCAP_C_SU_BEAM_FORMER_CAP		0x00000800
#define IEEE80211_VHTCAP_C_SU_BEAM_FORMEE_CAP		0x00001000
#define IEEE80211_VHTCAP_C_MU_BEAM_FORMER_CAP		0x00080000
#define IEEE80211_VHTCAP_C_MU_BEAM_FORMEE_CAP		0x00100000
#define IEEE80211_VHTCAP_C_VHT_TXOP_PS			0x00200000
#define IEEE80211_VHTCAP_C_PLUS_HTC_MINUS_VHT_CAP	0x00400000
#define IEEE80211_VHTCAP_C_RX_ATN_PATTERN_CONSISTNCY	0x10000000
#define IEEE80211_VHTCAP_C_TX_ATN_PATTERN_CONSISTNCY	0x20000000

#define IEEE80211_VHTCAP_C_MU_BEAM_FORMXX_CAP_MASK	(IEEE80211_VHTCAP_C_MU_BEAM_FORMER_CAP | \
							 IEEE80211_VHTCAP_C_MU_BEAM_FORMEE_CAP)

/* VHT mcs info extras */
#define IEEE80211_VHTCAP_MCS_MAX			8
#define IEEE80211_VHTCAP_MCS_DISABLED			0x03

/* VHT capability macro */
/* get macros */
/* VHT capabilities (all bits) */
#define IEEE80211_VHTCAP_GET_CAPFLAGS(vhtcap) \
	(u_int32_t)((vhtcap)->vht_cap[0] | \
	((vhtcap)->vht_cap[1] << 8) | \
	((vhtcap)->vht_cap[2] << 16) | \
	((vhtcap)->vht_cap[3] << 24))

/* B0-1 Max. MPDU Length */
#define IEEE80211_VHTCAP_GET_MAXMPDU(vhtcap) \
	(enum ieee80211_vht_maxmpdu)((vhtcap)->vht_cap[0] & 0x03)

/* B2-3 Supported channel width */
#define IEEE80211_VHTCAP_GET_CHANWIDTH(vhtcap) \
	(enum ieee80211_vht_chanwidth)(((vhtcap)->vht_cap[0] & 0x0C) >> 2)

/* B4 RX LDPC support */
#define IEEE80211_VHTCAP_GET_RXLDPC(vhtcap) \
	(((vhtcap)->vht_cap[0] & 0x10) >> 4)

/* B5 Short GI for 80MHz support */
#define IEEE80211_VHTCAP_GET_SGI_80MHZ(vhtcap) \
	(((vhtcap)->vht_cap[0] & 0x20) >> 5)

/* B7 TX STBC */
#define IEEE80211_VHTCAP_GET_TXSTBC(vhtcap) \
	(((vhtcap)->vht_cap[0] & 0x80) >> 7)

/* B8-10 RX STBC */
#define IEEE80211_VHTCAP_GET_RXSTBC(vhtcap) \
	(enum ieee80211_vht_rxstbc)((vhtcap)->vht_cap[1] & 0x07)

/* B11 SU Beam-former */
#define IEEE80211_VHTCAP_GET_SU_BEAMFORMER(vhtcap) \
	(((vhtcap)->vht_cap[1] & 0x08) >> 3)

/* B12 SU Beam-formee */
#define IEEE80211_VHTCAP_GET_SU_BEAMFORMEE(vhtcap) \
	(((vhtcap)->vht_cap[1] & 0x10) >> 4)

/* B13-15 Beamformee STS capability */
#define IEEE80211_VHTCAP_GET_BFSTSCAP(vhtcap) \
	(u_int8_t)(((vhtcap)->vht_cap[1] & 0xE0) >> 5)

/* B16-18 Number of sounding Dimensions */
#define IEEE80211_VHTCAP_GET_NUMSOUND(vhtcap) \
	(u_int8_t)((vhtcap)->vht_cap[2] & 0x07)

/* B19 MU Beam-formee VHT capability */
#define IEEE80211_VHTCAP_GET_MU_BEAMFORMER(vhtcap) \
	(((vhtcap)->vht_cap[2] & 0x08) >> 3)

/* B20 MU Beam-former VHT capability */
#define IEEE80211_VHTCAP_GET_MU_BEAMFORMEE(vhtcap) \
	(((vhtcap)->vht_cap[2] & 0x10) >> 4)

/* B22 VHT variant HT control field */
#define IEEE80211_VHTCAP_GET_HTC_VHT(vhtcap) \
	(((vhtcap)->vht_cap[2] & 0x40) >> 6)

/* B23-25 Max. A-MPDU Length Exponent */
#define IEEE80211_VHTCAP_GET_MAXAMPDUEXP(vhtcap) \
	(enum ieee80211_vht_maxampduexp)((((vhtcap)->vht_cap[2] & 0x80) >> 7) | \
	(((vhtcap)->vht_cap[3] & 0x03) << 1))

/* B26-27 VHT Link Adaptation capable */
#define IEEE80211_VHTCAP_GET_LNKADPTCAP(vhtcap) \
	(enum ieee80211_vht_lnkadptcap)(((vhtcap)->vht_cap[3] & 0x0C) >> 2)

/* B28 Rx Antenna pattern consistency */
#define IEEE80211_VHTCAP_GET_RXANTPAT(vhtcap) \
	(((vhtcap)->vht_cap[3] & 0x10) >> 4)

/* B29 Tx Antenna pattern consistency */
#define IEEE80211_VHTCAP_GET_TXANTPAT(vhtcap) \
	(((vhtcap)->vht_cap[3] & 0x20) >> 5)

/* B0-B15 RX VHT-MCS MAP for Spatial streams 1-8 */
#define IEEE80211_VHTCAP_GET_RX_MCS_NSS(vhtcap) \
	(((vhtcap)->vht_mcs_nss_set[1] << 8) | \
	((vhtcap)->vht_mcs_nss_set[0]))

/* B32-B47 TX VHT-MCS MAP for Spatial streams 1-8 */
#define IEEE80211_VHTCAP_GET_TX_MCS_NSS(vhtcap) \
	(((vhtcap)->vht_mcs_nss_set[5] << 8) | \
	((vhtcap)->vht_mcs_nss_set[4]))

/* VHT-MCS MAP entry for RX or TX MAP */
#define IEEE80211_VHTCAP_GET_MCS_MAP_ENTRY(mcsmap, idx) \
	((mcsmap >> (idx * 2)) & 0x3)

/* B16-B28 RX Highest supported Long GI data rates */
#define IEEE80211_VHTCAP_GET_RX_LGIMAXRATE(vhtcap) \
	(u_int16_t)(((vhtcap)->vht_mcs_nss_set[2]) | \
	((vhtcap)->vht_mcs_nss_set[3] << 8))

/* B48-B60 TX Highest supported Long GI data rates */
#define IEEE80211_VHTCAP_GET_TX_LGIMAXRATE(vhtcap) \
	(u_int16_t)(((vhtcap)->vht_mcs_nss_set[6]) | \
	((vhtcap)->vht_mcs_nss_set[7] << 8))

/* set macros */
/* VHT capabilities (all bits) */
#define IEEE80211_VHTCAP_SET_CAPFLAGS(vhtcap, _cap) \
	(vhtcap)->vht_cap[0] = ((_cap) & 0x000000FF); \
	(vhtcap)->vht_cap[1] = (((_cap) & 0x0000FF00) >> 8); \
	(vhtcap)->vht_cap[2] = (((_cap) & 0x00FF0000) >> 16); \
	(vhtcap)->vht_cap[3] = (((_cap) & 0xFF000000) >> 24)

/* B0-1 Max. MPDU Length */
#define IEEE80211_VHTCAP_SET_MAXMPDU(vhtcap, _m) \
	(vhtcap)->vht_cap[0] = (((vhtcap)->vht_cap[0] & ~0x03) | ((_m) & 0x03))

/* B2-3 Supported channel width */
#define IEEE80211_VHTCAP_SET_CHANWIDTH(vhtcap, _m) \
	(vhtcap)->vht_cap[0] = (((vhtcap)->vht_cap[0] & ~0x0C) | ((_m) & 0x03) << 2)

/* B8-10 RX STBC */
#define IEEE80211_VHTCAP_SET_RXSTBC(vhtcap, _m) \
	(vhtcap)->vht_cap[1] = (((vhtcap)->vht_cap[1] & ~0x07) | ((_m) & 0x07))

/* B13-15 Beamformee STS capability */
#define IEEE80211_VHTCAP_SET_BFSTSCAP(vhtcap, _m) \
	(vhtcap)->vht_cap[1] = (((vhtcap)->vht_cap[1] & ~0xE0) | ((_m) & 0x07) << 5)

/* B16-18 Number of sounding Dimensions */
#define IEEE80211_VHTCAP_SET_NUMSOUND(vhtcap, _m) \
	(vhtcap)->vht_cap[2] = (((vhtcap)->vht_cap[2] & ~0x07) | ((_m) & 0x07))

/* B23-25 Max. A-MPDU Length Exponent */
#define IEEE80211_VHTCAP_SET_MAXAMPDUEXP(vhtcap, _m) \
	(vhtcap)->vht_cap[2] = (((vhtcap)->vht_cap[2] & ~0x80) | ((_m) & 0x01) << 7); \
	(vhtcap)->vht_cap[3] = (((vhtcap)->vht_cap[3] & ~0x03) | ((_m) & 0x06) >> 1)

/* B26-27 VHT Link Adaptation capable */
#define IEEE80211_VHTCAP_SET_LNKADPTCAP(vhtcap, _m) \
	(vhtcap)->vht_cap[3] = (((vhtcap)->vht_cap[3] & ~0x0C) | ((_m) & 0x03) << 2)

/* B0-B15 RX VHT-MCS MAP for Spatial streams 1-8 */
#define IEEE80211_VHTCAP_SET_RX_MCS_NSS(vhtcap, _m) \
	(vhtcap)->vht_mcs_nss_set[1] = (((_m) & 0xFF00) >> 8); \
	(vhtcap)->vht_mcs_nss_set[0] = ((_m) & 0x00FF)

/* B16-B28 RX Highest supported Long GI data rates */
#define IEEE80211_VHTCAP_SET_RX_LGIMAXRATE(vhtcap, _m) \
	(vhtcap)->vht_mcs_nss_set[2] = ((_m) & 0x00FF); \
	(vhtcap)->vht_mcs_nss_set[3] = (((_m) & 0x1F00) >> 8)

/* B32-B47 TX VHT-MCS MAP for Spatial streams 1-8 */
#define IEEE80211_VHTCAP_SET_TX_MCS_NSS(vhtcap, _m) \
	(vhtcap)->vht_mcs_nss_set[5] = (((_m) & 0xFF00) >> 8); \
	(vhtcap)->vht_mcs_nss_set[4] = ((_m) & 0x00FF)

/* B48-B60 TX Highest supported Long GI data rates */
#define IEEE80211_VHTCAP_SET_TX_LGIMAXRATE(vhtcap, _m) \
	(vhtcap)->vht_mcs_nss_set[6] = ((_m) & 0x00FF); \
	(vhtcap)->vht_mcs_nss_set[7] = (((_m) & 0x1F00) >> 8)

/* VHT MCS MAP */
#define	IEEE80211_VHTMCS_ALL_DISABLE	(0xFFFF)

/* VHT capabilities options */
/* Defined in _ieee80211.h file */
/*
 * 802.11ac VHT Operation element
 */
struct ieee80211_ie_vhtop {
	u_int8_t	vhtop_id;		/* element ID */
	u_int8_t	vhtop_len;		/* length in bytes */
	u_int8_t	vhtop_info[3];		/* VHT Operation info */
	u_int8_t	vhtop_bvhtmcs[2];	/* basic VHT MSC and NSS set */
} __packed;

/* VHT Operation Information */
/* Channel width Octet 1 */
#define IEEE80211_VHTOP_SET_CHANWIDTH(vhtop, _m) \
	(vhtop)->vhtop_info[0] = (_m)

/* Channel Center Frequency Segment 0 */
#define IEEE80211_VHTOP_SET_CENTERFREQ0(vhtop, _m) \
	(vhtop)->vhtop_info[1] = (_m)

/* Channel Center Frequency Segment 1 */
#define IEEE80211_VHTOP_SET_CENTERFREQ1(vhtop, _m) \
	(vhtop)->vhtop_info[2] = (_m)

/* Basic VHT-MCS and NSS Set  */
#define IEEE80211_VHTOP_SET_BASIC_MCS_NSS(vhtop, _m) \
	(vhtop)->vhtop_bvhtmcs[0] = ((_m) & 0xFF00) >> 8; \
	(vhtop)->vhtop_bvhtmcs[1] = ((_m) & 0x00FF)

/* Get macros */
/* Channel width Octet 1 */
#define IEEE80211_VHTOP_GET_CHANWIDTH(vhtop) \
	(vhtop)->vhtop_info[0]

/* Channel Center Frequency Segment 0 */
#define IEEE80211_VHTOP_GET_CENTERFREQ0(vhtop) \
	(vhtop)->vhtop_info[1]

/* Channel Center Frequency Segment 1 */
#define IEEE80211_VHTOP_GET_CENTERFREQ1(vhtop) \
	(vhtop)->vhtop_info[2]

/* Basic VHT-MCS and NSS Set  */
#define IEEE80211_VHTOP_GET_BASIC_MCS_NSS(vhtop) \
	(((vhtop)->vhtop_bvhtmcs[0] << 8) | \
	((vhtop)->vhtop_bvhtmcs[1]))

/*
 * 802.11ac VHT Operating mode notification element
 */
struct ieee80211_ie_vhtop_notif {
	uint8_t	id;
	uint8_t	len;
	uint8_t	vhtop_notif_mode;
} __packed;

/*
 * 802.11ac Extended BSS Load element
 */
struct ieee80211_ie_ebssload {
	u_int8_t	ebl_id;			/* element ID */
	u_int8_t	ebl_len;		/* length in bytes */
	u_int8_t	ebl_mumimo_cnt[2];	/* MU-MIMO Capable station count */
	u_int8_t	ebl_ss_underuse;	/* Spatial Stream Underutilization */
	u_int8_t	ebl_20mhz_use;		/* Observable Secondary 20Mhz use */
	u_int8_t	ebl_40mhz_use;		/* Observable Secondary 40Mhz use */
	u_int8_t	ebl_80mhz_use;		/* Observable Secondary 80Mhz use */
} __packed;


/*
 * 802.11ac Wide Bandwidth Channel Switch element
 */
struct ieee80211_ie_wbchansw {
	u_int8_t	wbcs_id;		/* element ID */
	u_int8_t	wbcs_len;		/* length in bytes */
	u_int8_t	wbcs_newchanw;		/* New Channel Width */
	u_int8_t	wbcs_newchancf0;	/* New Channel Center Freq 0 */
	u_int8_t	wbcs_newchancf1;	/* New Channel Center Freq 1 */
} __packed;


/*
 * 802.11ac VHT Transmit Power Envelope element
 */
enum {
	IEEE80211_TX_POW_FOR_20MHZ,
	IEEE80211_TX_POW_FOR_40MHZ,
	IEEE80211_TX_POW_FOR_80MHZ,
	IEEE80211_TX_POW_FOR_160MHZ
};

struct ieee80211_ie_vtxpwren {
	u_int8_t	vtxpwren_id;		/* element ID */
	u_int8_t	vtxpwren_len;		/* length in byte */
	u_int8_t	vtxpwren_txpwr_info;	/* tx power info */
	u_int8_t	vtxpwren_tp20;		/* local max tx power for 20Mhz */
	u_int8_t	vtxpwren_tp40;		/* local max tx power for 40Mhz */
	u_int8_t	vtxpwren_tp80;		/* local max tx power for 80Mhz */
	u_int8_t	vtxpwren_tp160;		/* local max tx power for 160Mhz */
} __packed;

/*
 * 802.11ac Channel Switch Wrapper element
 */
struct ieee80211_ie_chsw_wrapper {
	u_int8_t			chsw_id;		/* element ID */
	u_int8_t			chsw_len;		/* length in byte */
} __packed;

/*
 * 802.11ac AID element
 */
struct ieee80211_ie_aid {
	u_int8_t	aid_id;		/* element ID */
	u_int8_t	aid_len;	/* length in byte */
	u_int16_t	aid;		/* aid */
} __packed;

/*
 * 802.11ac Quiet Channel element
 */
struct ieee80211_ie_quietchan {
	u_int8_t	qc_id;		/* element ID */
	u_int8_t	qc_len;		/* length in byte */
	u_int8_t	qc_qmode;	/* AP Quite Mode */
	u_int8_t	qc_qcnt;	/* AP Quite Count */
	u_int8_t	qc_qperiod;	/* AP Quite Period */
	u_int8_t	qc_qduration;	/* AP Quite Duration */
	u_int8_t	qc_qoffset;	/* AP Quite Offset */
} __packed;


/*
 * 802.11ac Operating Mode Notification element
 */
struct ieee80211_ie_opmodenotice {
	u_int8_t	omn_id;		/* element ID */
	u_int8_t	omn_len;	/* length in byte */
	u_int8_t	opn_opmode;	/* Op Mode */
} __packed;

enum {
	IEEE80211_TIMEOUT_REASSOC_DEADLINE		= 1,
	IEEE80211_TIMEOUT_KEY_LIFETIME			= 2,
	IEEE80211_TIMEOUT_ASSOC_COMEBACK		= 3,
};

#define IEEE80211_W_ASSOC_COMEBACK_TO		1000

/*
 * 802.11w timeout information IE
 */
struct ieee80211_timout_int_ie {
	u_int8_t	timout_int_ie;			/* IEEE80211_ELEMID_TIMEOUT_INT */
	u_int8_t	timout_int_len;
	u_int8_t	timout_int_type;		/* Timeout Interval Type */
	u_int32_t	timout_int_value;		/* in tus */
} __packed;

/*
 * Add the Quantenna OUI to a frame
 */
uint8_t ieee80211_oui_add_qtn(uint8_t *oui);

/*
 * AUTH management packets
 *
 *	octet algo[2]
 *	octet seq[2]
 *	octet status[2]
 *	octet chal.id
 *	octet chal.length
 *	octet chal.text[253]
 */

typedef uint8_t *ieee80211_mgt_auth_t;

#define	IEEE80211_AUTH_ALGORITHM(auth) \
	((auth)[0] | ((auth)[1] << 8))
#define	IEEE80211_AUTH_TRANSACTION(auth) \
	((auth)[2] | ((auth)[3] << 8))
#define	IEEE80211_AUTH_STATUS(auth) \
	((auth)[4] | ((auth)[5] << 8))

#define	IEEE80211_AUTH_ALG_OPEN		0x0000
#define	IEEE80211_AUTH_ALG_SHARED	0x0001
#define	IEEE80211_AUTH_ALG_LEAP		0x0080

enum {
	IEEE80211_AUTH_OPEN_REQUEST		= 1,
	IEEE80211_AUTH_OPEN_RESPONSE		= 2,
};

enum {
	IEEE80211_AUTH_SHARED_REQUEST		= 1,
	IEEE80211_AUTH_SHARED_CHALLENGE		= 2,
	IEEE80211_AUTH_SHARED_RESPONSE		= 3,
	IEEE80211_AUTH_SHARED_PASS		= 4,
};

/*
 * Reason codes
 *
 * Unlisted codes are reserved
 */

enum {
	IEEE80211_REASON_UNSPECIFIED			= 1,
	IEEE80211_REASON_AUTH_EXPIRE			= 2,
	IEEE80211_REASON_AUTH_LEAVE			= 3,
	IEEE80211_REASON_ASSOC_EXPIRE			= 4,
	IEEE80211_REASON_ASSOC_TOOMANY			= 5,
	IEEE80211_REASON_NOT_AUTHED			= 6,
	IEEE80211_REASON_NOT_ASSOCED			= 7,
	IEEE80211_REASON_ASSOC_LEAVE			= 8,
	IEEE80211_REASON_ASSOC_NOT_AUTHED		= 9,
	IEEE80211_REASON_DISASSOC_BAD_POWER		= 10,
	IEEE80211_REASON_DISASSOC_BAD_SUPP_CHAN		= 11,
	IEEE80211_REASON_IE_INVALID			= 13,
	IEEE80211_REASON_MIC_FAILURE			= 14,
	IEEE80211_REASON_4WAY_HANDSHAKE_TIMEOUT		= 15,
	IEEE80211_REASON_GROUP_KEY_HANDSHAKE_TIMEOUT	= 16,
	IEEE80211_REASON_IE_DIFFERENT			= 17,
	IEEE80211_REASON_INVALID_GROUP_CIPHER		= 18,
	IEEE80211_REASON_INVALID_PAIRWISE_CIPHER	= 19,
	IEEE80211_REASON_INVALID_AKMP			= 20,
	IEEE80211_REASON_UNSUPP_RSN_VERSION		= 21,
	IEEE80211_REASON_INVALID_RSN_IE_CAP		= 22,
	IEEE80211_REASON_IEEE8021X_FAILED		= 23,
	IEEE80211_REASON_CIPHER_SUITE_REJECTED		= 24,
	IEEE80211_REASON_TDLS_UNREACH			= 25, /* TDLS teardown due to peer unreachable */
	IEEE80211_REASON_TDLS_UNSPEC			= 26, /* TDLS teardown for unspecified reason */
	IEEE80211_REASON_DISASSOC_UNSPECIFIED_QOS	= 32,
	IEEE80211_REASON_DISASSOC_QOS_AP_NO_BANDWIDTH	= 33,
	IEEE80211_REASON_DISASSOC_LOW_ACK		= 34,
	IEEE80211_REASON_DISASSOC_STA_EXCEED_TXOP	= 35,
	IEEE80211_REASON_STA_LEAVE_BSS			= 36,
	IEEE80211_REASON_STA_NOT_USE			= 37,
	IEEE80211_REASON_STA_REQUIRE_SETUP		= 38,
	IEEE80211_REASON_STA_TIMEOUT			= 39,
	IEEE80211_REASON_STA_CIPHER_NOT_SUPP		= 45,


	IEEE80211_STATUS_SUCCESS		= 0,
	IEEE80211_STATUS_UNSPECIFIED		= 1,
	IEEE80211_STATUS_TDLS_WKUP_REJ_ALT	= 2,  /* Wakeup sched rejected/alternative */
	IEEE80211_STATUS_TDLS_WKUP_REJ		= 3,  /* Wakeup sched rejected */
	IEEE80211_STATUS_SEC_DIS		= 5,  /* Security disabled */
	IEEE80211_STATUS_LIFETIME_NOTOK		= 6,  /* Unacceptable lifetime */
	IEEE80211_STATUS_BSS_INVALID		= 7,  /* Not in same BSS */
	IEEE80211_STATUS_CAPINFO		= 10,
	IEEE80211_STATUS_NOT_ASSOCED		= 11,
	IEEE80211_STATUS_OTHER			= 12,
	IEEE80211_STATUS_ALG			= 13,
	IEEE80211_STATUS_SEQUENCE		= 14,
	IEEE80211_STATUS_CHALLENGE		= 15,
	IEEE80211_STATUS_TIMEOUT		= 16,
	IEEE80211_STATUS_TOOMANY		= 17,
	IEEE80211_STATUS_BASIC_RATE		= 18,
	IEEE80211_STATUS_SP_REQUIRED		= 19,
	IEEE80211_STATUS_PBCC_REQUIRED		= 20,
	IEEE80211_STATUS_CA_REQUIRED		= 21,
	IEEE80211_STATUS_TOO_MANY_STATIONS	= 22,
	IEEE80211_STATUS_RATES			= 23,
	IEEE80211_STATUS_SHORTSLOT_REQUIRED	= 25,
	IEEE80211_STATUS_DSSSOFDM_REQUIRED	= 26,
	IEEE80211_STATUS_HT_FEATURE		= 27,
	IEEE80211_STATUS_PMF_REJECT_RETRY		= 30,
	IEEE80211_STATUS_PMF_VIOLATION		= 31,
	IEEE80211_STATUS_PEER_MECHANISM_REJECT	= 37,
	IEEE80211_STATUS_TDLS_RSNIE_INVALID	= 72, /* Invalid contents of RSNIE */

	/* Quantenna */
	IEEE80211_STATUS_DENIED			= 100,
};

#define	IEEE80211_WEP_KEYLEN		5	/* 40bit */
#define	IEEE80211_WEP_IVLEN		3	/* 24bit */
#define	IEEE80211_WEP_KIDLEN		1	/* 1 octet */
#define	IEEE80211_WEP_CRCLEN		4	/* CRC-32 */
#define	IEEE80211_WEP_NKID		4	/* number of key ids */

/*
 * 802.11i defines an extended IV for use with non-WEP ciphers.
 * When the EXTIV bit is set in the key id byte an additional
 * 4 bytes immediately follow the IV for TKIP.  For CCMP the
 * EXTIV bit is likewise set but the 8 bytes represent the
 * CCMP header rather than IV+extended-IV.
 */
#define	IEEE80211_WEP_EXTIV		0x20
#define	IEEE80211_WEP_EXTIVLEN		4	/* extended IV length */
#define	IEEE80211_WEP_CCMPLEN		8	/* CCMP header */
#define	IEEE80211_WEP_MICLEN		8	/* trailing MIC */
#define	IEEE80211_WEP_ICVLEN		4	/* ICV */

#define	IEEE80211_CRC_LEN		4
#define IEEE80211_MAX_IE_LEN		257

/*
 * Maximum acceptable MTU is:
 *	IEEE80211_MAX_LEN - WEP overhead - CRC -
 *		QoS overhead - RSN/WPA overhead
 * Min is arbitrarily chosen > IEEE80211_MIN_LEN.  The default
 * mtu is Ethernet-compatible; it's set by ether_ifattach.
 */
#define	IEEE80211_MTU_MAX		3500
#define	IEEE80211_MTU_MIN		32

#define	IEEE80211_MAX_LEN		(2300 + IEEE80211_CRC_LEN + \
	(IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN + IEEE80211_WEP_CRCLEN))
#define	IEEE80211_ACK_LEN \
	(sizeof(struct ieee80211_frame_ack) + IEEE80211_CRC_LEN)
#define	IEEE80211_MIN_LEN \
	(sizeof(struct ieee80211_frame_min) + IEEE80211_CRC_LEN)

/*
 * RTS frame length parameters.  The default is specified in
 * the 802.11 spec.  The max may be wrong for jumbo frames.
 */
#define	IEEE80211_RTS_DEFAULT		512
#define	IEEE80211_RTS_MIN		0
#define	IEEE80211_RTS_MAX		65536
#define	IEEE80211_RTS_THRESH_OFF	(IEEE80211_RTS_MAX + 1)

/*
 * Regulatory extension identifier for country IE.
 */
#define IEEE80211_REG_EXT_ID		201

/*
 * IEEE 802.11 timer synchronization function (TSF) timestamp length
 */
#define IEEE80211_TSF_LEN		8
/*
 * 802.11n defines
 */
#define IEEE80211_11N_BAWLEN		64
#define IEEE80211_11N_QLENLIM		(64*1024)

#define IEEE80211_11N_SEQINORDER_BAW(seq_front, seq_back)       \
        IEEE80211_SEQ_INORDER_LAG((seq_front), (seq_back), IEEE80211_11N_BAWLEN)

struct wmm_params {
	uint8_t wmm_acm;		/* ACM parameter */
	uint8_t wmm_aifsn;		/* AIFSN parameters */
	uint8_t wmm_logcwmin;		/* cwmin in exponential form */
	uint8_t wmm_logcwmax;		/* cwmax in exponential form */
	uint16_t wmm_txopLimit;		/* txopLimit */
	uint8_t wmm_noackPolicy;	/* No-Ack Policy: 0=ack, 1=no-ack */
};

#define IEEE80211_DEFAULT_BA_WINSIZE	64	/* use for explicit BA establishing, size and throughput is moderate */
#define IEEE80211_DEFAULT_BA_WINSIZE_H	256	/* use for implicit BA, large size to support large aggregates and high throughput */
#define IEEE80211_MAX_BA_WINSIZE	0x3FF

/*value assignment for the little-endian*/
#define	ADDINT16LE(frm, v) do {			\
	frm[0] = (v) & 0xff;				\
	frm[1] = ((v) >> 8) & 0xff;			\
	frm += 2;							\
} while (0)
/* 32 bits to 32 bits */
#define	ADDINT32LE(frm, v) do {			\
	frm[0] = (v) & 0xff;				\
	frm[1] = ((v) >> 8) & 0xff;			\
	frm[2] = ((v) >> 16) & 0xff;		\
	frm[3] = ((v) >> 24) & 0xff;		\
	frm += 4;							\
} while (0)


/* value assignment */
/* 16 bits to 16 bits */
#define	ADDINT16(frm, v) do {			\
	frm[1] = (v) & 0xff;				\
	frm[0] = ((v) >> 8) & 0xff;			\
	frm += 2;							\
} while (0)
/* 32 bits to 32 bits */
#define	ADDINT32(frm, v) do {			\
	frm[3] = (v) & 0xff;				\
	frm[2] = ((v) >> 8) & 0xff;			\
	frm[1] = ((v) >> 16) & 0xff;		\
	frm[0] = ((v) >> 24) & 0xff;		\
	frm += 4;							\
} while (0)
/* 8 bits to 32 bits */
#define	ADDINT8TO32(frm, v) do {			\
	frm[3] = (v) & 0xff;				\
	frm[2] = 0;					\
	frm[1] = 0;					\
	frm[0] = 0;					\
	frm += 4;							\
} while (0)
/* 16 bits to 32 bits */
#define	ADDINT16TO32(frm, v) do {			\
	frm[3] = (v) & 0xff;				\
	frm[2] = ((v) >> 8) & 0xff;			\
	frm[1] = 0;					\
	frm[0] = 0;					\
	frm += 4;							\
} while (0)
/* 32 bits to 64 bits */
#define	ADDINT32TO64(frm, v) do {			\
	frm[7] = (v) & 0xff;				\
	frm[6] = ((v) >> 8) & 0xff;			\
	frm[5] = ((v) >> 16) & 0xff;		\
	frm[4] = ((v) >> 24) & 0xff;		\
	frm[3] = 0;							\
	frm[2] = 0;							\
	frm[1] = 0;							\
	frm[0] = 0;							\
	frm += 8;							\
} while (0)

#define IEEE80211_IE_LEADER_STR_VHTCAP	"vhtcap_ie="
#define IEEE80211_IE_LEADER_STR_HTCAP	"htcap_ie="
#define IEEE80211_IE_LEADER_STR_RSN	"rsn_ie="
#define IEEE80211_IE_LEADER_STR_WPA	"wpa_ie="
#define IEEE80211_IE_LEADER_STR_WME	"wme_ie="
#define IEEE80211_IE_LEADER_STR_ATH	"ath_ie="
#define IEEE80211_IE_LEADER_STR_EXT_ROLE	"qtn_extender_role="

#ifndef DSP_BUILD
static __inline__ int ieee80211_is_bcst(const void *p)
{
	const uint16_t *p16 = p;

	return (p16[0] == 0xFFFF) && (p16[1] == 0xFFFF) && (p16[2] == 0xFFFF);
}

#ifdef TOPAZ_PLATFORM
/*
 * IEEE802.11w spec - Table 8-38 and section 11.1.7
 */
static __inline__ int ieee80211_mgmt_is_robust(const struct ieee80211_frame *wh) {

	int is_robust_mgmt = 0;
	const uint8_t subtype = wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK;

	switch (subtype){
		case IEEE80211_FC0_SUBTYPE_DEAUTH:
		case IEEE80211_FC0_SUBTYPE_DISASSOC:
			is_robust_mgmt = 1;
			break;
		case IEEE80211_FC0_SUBTYPE_ACTION:
		{
			struct ieee80211_action *ia;
			ia = (struct ieee80211_action *) (void*)&wh[1];

			switch (ia->ia_category) {
				case IEEE80211_ACTION_CAT_SPEC_MGMT:
				case IEEE80211_ACTION_CAT_QOS:
				case IEEE80211_ACTION_CAT_DLS:
				case IEEE80211_ACTION_CAT_BA:
				case IEEE80211_ACTION_CAT_RM:
				case IEEE80211_ACTION_CAT_FBSS:
				case IEEE80211_ACTION_CAT_SA_QUERY:
				case IEEE80211_ACTION_CAT_PROT_DUAL_PA:
				case IEEE80211_ACTION_CAT_WNM:
				case IEEE80211_ACTION_CAT_MESH:
				case IEEE80211_ACTION_CAT_MULTIHOP:
				case IEEE80211_ACTION_CAT_VEND_PROT:
					is_robust_mgmt = 1;
					break;
				default:
					is_robust_mgmt = 0;
					break;
			}
			break;
		}
		default:
			break;

	}

	return is_robust_mgmt;
}
#endif
#endif

#define IEEE80211_N_RATE_PREFIX 0x7F000000
#define IEEE80211_AC_RATE_PREFIX 0x7E000000
#define IEEE80211_RATE_PREFIX_MASK 0xFF000000

#define IEEE80211U_PARAM_IPV4ADDRTYPE_MIN	0
#define IEEE80211U_PARAM_IPV4ADDRTYPE_MAX	7
#define IEEE80211U_PARAM_IPV6ADDRTYPE_MIN	0
#define IEEE80211U_PARAM_IPV6ADDRTYPE_MAX	2
#define IEEE80211U_PARAM_IP_STATUS_MAX		2
/* MU MIMO */
#define IEEE80211_MU_GRP_VALID(_grp)		\
	(((_grp) > 0) && ((_grp) < (IEEE80211_VHT_GRP_MAX_BIT_OFFSET+1)))

#define IEEE80211_MU_POS_VALID(_pos) ((_pos) < 4)

#define IEEE80211_MU_DEL_GRP(mu_grp, _grp) do {		\
	(mu_grp).member[(_grp) >> 3] &= ~(1 << ((_grp) & 0x7)); \
} while (0)

#define IEEE80211_MU_ADD_GRP(mu_grp, _grp, _pos) do {	\
	(mu_grp).member[(_grp) >> 3] |= (1 << ((_grp) & 0x7)); \
	(mu_grp).pos[(_grp) >> 2] &= ~((0x03 << (((_grp) & 0x3) << 1))); \
	(mu_grp).pos[(_grp) >> 2] |= (((_pos) << (((_grp) & 0x3) << 1))); \
} while (0)

#define IEEE80211_MU_IS_GRP_MBR(mu_grp, _grp)	\
	((mu_grp).member[(_grp) >> 3] & (1 << ((_grp) & 0x7)))

#define IEEE80211_MU_GRP_POS(mu_grp, _grp)	\
	(((mu_grp).pos[(_grp) >> 2] >> (((_grp) & 0x3) << 1)) & 0x3)

#endif /* _NET80211_IEEE80211_H_ */

