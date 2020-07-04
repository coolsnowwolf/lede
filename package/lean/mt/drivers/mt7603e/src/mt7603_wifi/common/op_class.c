#ifdef DOT11_N_SUPPORT
#include "rt_config.h"

typedef enum {
	Reserved,
	FREQ_2G407,
	FREQ_2G414,
	FREQ_3G00,
	FREQ_3G0025,
	FREQ_4G00,
	FREQ_4G0025,
	FREQ_4G85,
	FREQ_4G89,
	FREQ_4G9375,
	FREQ_5G00,
	FREQ_5G0025
} START_FREQ;
/*definition for Behavior limits set Reference 11n/11ac spec Table D-2 */
typedef enum {
	NOMADICBEHAVIOR = (1 << 1),
	LICENSEEXEMPTBEHAVIOR = (1 << 10),
	PRIMARYCHANNELLOWERBEHAVIOR = (1 << 13),
	PRIMARYCHANNELUPPERBEHAVIOR = (1 << 14),
	CCA_EDBEHAVIOR = (1 << 15),
	DFS_50_100_BEHAVIOR = (1 << 16),
	ITS_NONMOBILE_OPERATIONS = (1 << 17),
	ITS_MOBILE_OPERATIONS = (1 << 18),
	PLUS_80 = (1 << 19),
	USEEIRPFORVHTTXPOWENV = (1 << 20),
	COMMON = 0xffffff
} BEHAVIOR_LIMITS_SET;

/* definition for Behavior limits set Reference 11n/11ac spec Table D-2 */

typedef struct {
	UCHAR		reg_class;			/* regulatory class */
	UCHAR		global_class;		/* Global operating class */
	START_FREQ	start_freq;			/* Channel starting frequency*/
	UCHAR		spacing;			/* 0: 20Mhz, 1: 40Mhz */
	UCHAR		channelset[16];	/* max 15 channels, use 0 as terminator */
	BEHAVIOR_LIMITS_SET behavior_limit_set; /* Behavior limits set */
} REG_CLASS, *PREG_CLASS;

/*
*	Table E-1—Operating classes in the United States  (11N)
*/
REG_CLASS reg_class_fcc[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																DFS_50_100_BEHAVIOR},
	{3,  124, FREQ_5G00,   BW_20, {149, 153, 157, 161, 0},															NOMADICBEHAVIOR},
	{4,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						DFS_50_100_BEHAVIOR},
	{5,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 0},						LICENSEEXEMPTBEHAVIOR},
	{6,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0},												COMMON},
	{7,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0},												COMMON},
	{8,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0},															COMMON},
	{9,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0},															COMMON},
	{10, 101, FREQ_4G85,   BW_20, {21, 25, 0},																		COMMON},
	{11, 101, FREQ_4G85,   BW_20, {21, 25, 0},																		COMMON},
	{12,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0},											LICENSEEXEMPTBEHAVIOR},
	{13,  94, FREQ_3G00,   BW_20, {133, 137, 0},																	CCA_EDBEHAVIOR},
	{14,  95, FREQ_3G00,   BW_10, {132, 134, 136, 138, 0},															CCA_EDBEHAVIOR},
	{15,  96, FREQ_3G0025, BW_5,  {131, 132, 133, 134, 135, 136, 137, 138, 0},										CCA_EDBEHAVIOR},
	{16,   0, FREQ_5G0025, BW_5,  {170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},	ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{18,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0},					ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{22, 116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{23, 119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{24, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{25, 126, FREQ_5G00,   BW_40, {149, 157, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{26, 126, FREQ_5G00,   BW_40, {149, 157, 0},																	LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{27, 117, FREQ_5G00,   BW_40, {40, 48, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{28, 120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{29, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},													NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{30, 127, FREQ_5G00,   BW_40, {153, 161, 0},																	NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{31, 127, FREQ_5G00,   BW_40, {153, 161, 0},																	LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{32,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{33,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{128, 128, FREQ_5G00,  BW_80, {0},																			COMMON},
	{129, 129, FREQ_5G00,  BW_80, {0},																			COMMON},
	{130, 130, FREQ_5G00,  BW_80, {0},																			COMMON},

	{0, 0, 0, 0, {0}, 0}			/* end */
};

/*
	Table E-2—Operating classes in Europe (11N)
*/
REG_CLASS reg_class_ce[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																NOMADICBEHAVIOR},
	{3,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						COMMON},
	{4,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0},									LICENSEEXEMPTBEHAVIOR},
	{5,  116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{6,  119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{7,  122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR},
	{8,  117, FREQ_5G00,   BW_40, {40, 48, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{9,  120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{10, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},													PRIMARYCHANNELUPPERBEHAVIOR}, /* 10 */
	{11,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{12,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{13,   0, FREQ_5G0025, BW_5,  {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{14,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{15,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0},					ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{16,   0, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17, 125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 0},												COMMON},
	{0, 0, 0, 0, {0}, 0}			/* end */
};

/*
	Table E-3—Operating classes in Japan (11N)
*/
REG_CLASS reg_class_jp[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {34, 36, 38, 40, 42, 44, 46, 48, 0},												COMMON},/* 1 */
	{2,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{3,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{4,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{5,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{6,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{7,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{8,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{9,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{10, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},/* 10 */
	{11, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{12, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{13, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{14, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{15, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{16, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{17, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{18, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{19, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{20, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{21, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{22, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{23, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{24, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{25, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{26, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{27, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{28, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{29, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{30,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0},									LICENSEEXEMPTBEHAVIOR},
	{31,  82, FREQ_2G414,  BW_25, {14, 0},																			LICENSEEXEMPTBEHAVIOR},
	{32, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																COMMON},
	{33, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																COMMON},
	{34, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						DFS_50_100_BEHAVIOR},
	{35, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						DFS_50_100_BEHAVIOR},
	{36, 116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{37, 119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{38, 119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{39, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{40, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{41, 117, FREQ_5G00,   BW_40, {40, 48, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{42, 120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{43, 120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{44, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},														PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{45, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},														PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{46, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{47, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{48, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{49, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{50, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{51, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{52, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{53, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{54, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{55, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{56,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{57,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{58, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						NOMADICBEHAVIOR | LICENSEEXEMPTBEHAVIOR},
	{0, 0, 0, 0, {0}, 0}			/* end */
};

REG_CLASS reg_class_cn[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0},																USEEIRPFORVHTTXPOWENV},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{3,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 0},						USEEIRPFORVHTTXPOWENV},
	{4,  116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{5,  119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{6,  126, FREQ_5G00,   BW_40, {149, 157, 0},																	PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{7,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0},									LICENSEEXEMPTBEHAVIOR},
	{8,   83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{9,   84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{0, 0, 0, 0, {0}, 0}			/* end */
};

BOOLEAN is_channel_in_channelset(UCHAR *ChannelSet, UCHAR Channel)
{
	int index = 0;

	while (ChannelSet[index] != 0) {
		/* TODO: Need to consider behavior limits set!! */
		if (ChannelSet[index] == Channel)
			return TRUE;

		index++;
	}

	return FALSE;
}

PVOID get_reg_table_by_country(UCHAR *CountryCode)
{
	UCHAR reg_region;
	PVOID reg_class_table = NULL;
#ifdef EXT_BUILD_CHANNEL_LIST
	PCH_REGION pChReg;

	pChReg = GetChRegion(CountryCode);
	reg_region = pChReg->DfsType;
#else
	PCOUNTRY_PROP pCountryProp;

	pCountryProp = GetCountryProp(CountryCode);
	reg_region = pCountryProp->DfsType;
#endif

	{
		switch (reg_region) {
		case CE:
			reg_class_table = (PVOID)reg_class_ce;
			break;

		case JAP:
			reg_class_table = (PVOID)reg_class_jp;
			break;

		case CHN:
			reg_class_table = (PVOID)reg_class_cn;
			break;

		case FCC:
		default:
			reg_class_table = (PVOID)reg_class_fcc;
			break;
		}
	}

	return reg_class_table;
}

PUCHAR get_channelset_by_reg_class(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 RegulatoryClass)
{
	int i = 1; /* skip Invlid entry */
	PUCHAR channelset = NULL;

	PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode);

	if (reg_class) {
		do {
			/* find  channelset */
			if ((reg_class[i].reg_class == RegulatoryClass)
				|| (reg_class[i].global_class == RegulatoryClass)) {
				channelset = reg_class[i].channelset;
				break;
			}

			i++;
		} while (reg_class[i].reg_class != 0);
	}

	return channelset;
}


UINT BW_VALUE[] = {20, 40, 80, 160, 10, 5, 162, 60, 25};
UCHAR get_regulatory_class(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	int i = 1; /* skip Invlid entry */
	UCHAR regclass = 0;
	UINT16 bw = BW_20;
	UCHAR cfg_ht_bw = pAd->CommonCfg.RegTransmitSetting.field.BW;
	UCHAR reg_domain = pAd->reg_domain;

	if (cfg_ht_bw)
		bw = BW_40;
	else
		bw = BW_20;

	if (Channel == 0)
		Channel = pAd->CommonCfg.Channel;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(): Channel=%d,HT_BW=%d bw=%d\n",
			  __func__,
			  Channel,
			  cfg_ht_bw,
			  bw));
	{
		PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode);

		if (reg_class) {
			do {
				if (BW_VALUE[reg_class[i].spacing] >= BW_VALUE[bw]) {
					/* find  channelset */
					if (is_channel_in_channelset(reg_class[i].channelset, Channel)) {
						regclass = (reg_domain == REG_LOCAL) ?
						reg_class[i].reg_class : reg_class[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class[i].reg_class != 0);
		}
	}

	return regclass;
}

INT Show_ChannelSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int i = 1; /* skip Invlid entry */
	int Channel = 0, Type = 0, IsAc = 0, OpClass = 0;
	RTMP_STRING *arg0_type = NULL, *arg1_channel = NULL, *arg2_is_ac = NULL;

	arg0_type = strsep(&arg, ":");
	arg1_channel = strsep(&arg, ":");
	arg2_is_ac = strsep(&arg, ":");

	if (arg0_type == NULL || arg1_channel == NULL || arg2_is_ac == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: TYPE:CHANNEL:ISAC\n", __func__));
		return FALSE;
	}

	Type = simple_strtol(arg0_type, 0, 10);
	Channel = simple_strtol(arg1_channel, 0, 10);
	IsAc = simple_strtol(arg2_is_ac, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Type = %d Channel = %d IsAc = %d\n", __func__, Type, Channel, IsAc));

	if (Type != 0) {
		OpClass = get_regulatory_class(pAd, Channel);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("OpClass(%d) Channel(%d)\n", OpClass, Channel));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\n%-8s%-8s%-16s\n", "Regclass", "Spacing", "Channelset/CenterFreq"));
		{
			PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode);

			do {
				int j = 0;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%-8d%-8d", reg_class[i].reg_class, reg_class[i].spacing));

				while (reg_class[i].channelset[j] != 0) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("%-2d,", reg_class[i].channelset[j]));
					j++;
				}

				i++;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			} while (reg_class[i].reg_class != 0);
		}
	}

	return TRUE;
}


#endif /* DOT11_N_SUPPORT */


