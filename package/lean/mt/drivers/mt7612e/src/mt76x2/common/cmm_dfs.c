/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_dfs.c

    Abstract:
    Support DFS function.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/

#include "rt_config.h"

#ifdef DFS_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#ifdef DFS_ATP_SUPPORT
extern INT Set_AP_SSID_Proc(IN  PRTMP_ADAPTER, IN  PSTRING);
#endif /* DFS_ATP_SUPPORT */

NewDFSValidRadar NewDFSValidTable[] = 
{
	/* FCC-1  && (Japan W53 Radar 1 / W56 Radar 2)*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP | NEW_DFS_JAP_W53),
	7,
	10, 1000,
	0,
	4,
	0, 0, 
	28570 - 70,
	150
	},
	/* FCC-2*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	7,
	13, 1000,
	0,
	1,
	3000, 4600 - 20,
	0,
	25
	},
	/* FCC-3 & FCC-4*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	7,
	/*120, 200, FCC-3 */
	/*220, 400, FCC-4*/
	100, 1500, 
	0,
	1,
	4000, 10000 - 40, 
	0,
	60
	},
	/* FCC-6*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	7,
	12, 1000,
	0,
	1,
	0, 0, 
	6660-10,
	35
	},
	/* Japan W53 Radar 2*/
	{
	NEW_DFS_JAP_W53,
	7,
	40, 1000, 
	0,
	1,
	0, 0, 
	76923 - 30,
	180
	},
	/* Japan W56 Radar 1*/
	{
	NEW_DFS_JAP,
	7,
	5, 500, 
	0,
	2,
	0, 0, 
	27777 - 30,
	70
	},
	/* Japan W56 Radar 3*/
	{
	NEW_DFS_JAP,
	7,
	30, 1000, 
	0,
	1,
	0, 0, 
	80000 - 50,
	200
	},

/* CE Staggered radar*/

	{
	/*	EN-1*/
	/*	width	0.8 - 5 us*/
	/*	PRF		200 - 1000 Hz*/
	/*	PRI		5000 - 1000 us	(T: 20000 - 100000)*/
	/*	*/
	NEW_DFS_EU,
	0xf,
	10, 1000, 
	0,
	1,
	20000-15, 100000-70, 
	0,
	120
	},
	/*	EN-2*/
	/*	width	0.8 - 15 us*/
	/*	PRF		200 - 1600 Hz*/
	/*	PRI		5000 - 625 us	(T: 12500 - 100000)*/
	{
	NEW_DFS_EU,
	0xf,
	10, 2000, 
	0,
	1,
	12500 - 10, 100000 - 70, 
	0,
	120
	},
	
	/*	EN-3*/
	/*	width	0.8 - 15 us*/
	/*	PRF		2300 - 4000 Hz*/
	/*	PRI		434 - 250 us	(T: 5000 - 8695)*/
	{
	NEW_DFS_EU,
	0xf,
	21, 2000, 
	0,
	1,
	5000 - 4, 8695 - 7, 
	0,
	50
	},
	/*	EN-4*/
	/*	width	20 - 30 us*/
	/*	PRF		2000 - 4000 Hz*/
	/*	PRI		500 - 250 us	(T: 5000 - 10000)*/
	/*	Note : with Chirp Modulation +- 2,5Mhz*/
	{
	NEW_DFS_EU,
	0xf,
	380, 3000, 
	0,
	4,
	5000 - 4, 10000 - 8, 
	0,
	60
	},
	/*	EN-5*/
	/*	width	0.8 - 2 us*/
	/*	PRF		300 - 400 Hz*/
	/*	PRI		3333 - 2500 us	(T: 50000 - 66666)*/
	/*	Staggered PRF, 20 - 50 pps*/
	{
	NEW_DFS_EU,
	0xf,
	10, 800, 
	0,
	1,
	50000 - 35, 66666 + 50,
	0,
	30
	},
	/*	EN-6*/
	/*	width	0.8 - 2 us*/
	/*	PRF		400 - 1200 Hz*/
	/*	PRI		2500 - 833 us	(T: 16666 - 50000)*/
	/*	Staggered PRF, 80 - 400 pps*/
	{
	NEW_DFS_EU,
	0xf,
	10, 800, 
	0,
	1,
	16666 - 13, 50000 + 35,
	0,
	30
	},
	
	{
	NEW_DFS_END,
	0,
	0, 0, 
	0,
	0,
	0, 0, 
	0,
	0,
	},
};

static NewDFSTable NewDFSTable1[] = 
{
	{
		/* ch, mode(0~7), M(~511), el, eh(~4095), wl, wh(~4095), err_w, tl, th, err_t, bl, bh, ee, pj*/
		NEW_DFS_FCC,
		{
		{0, 0,  10,   8,  16,   6, 2000,  5, 2900, 29000,  5, 0, 0, 0x1000000, 0},
		{1, 0,  70,  42, 126,  20, 5000,  5, 2900, 29000, 10, 0, 0, 0x1000000, 0},
		{2, 0, 100,  42, 160,  20, 5000, 25, 2900, 10100, 20, 0, 0, 0x1000000, 0},
		{3, 2, 200,  20, 150, 900, 2200, 50, 1000, 999999999, 200, 0, 999999999, 0x10000000, 0},
		}
	},

    {
        NEW_DFS_EU,
        {
            {0, 0,  10,  10,  18,   4, 4095,   5,  7800, 101000,   5, 0, 0, 0x1000000, 0},
            {1, 0,  70,  42,  90,  20, 4095,   3,  4900, 101000,  10, 0, 0, 0x1000000, 0},
            {2, 0, 100,  42, 160,  20, 4095,   5,  4900, 101000,  20, 0, 0, 0x1000000, 0},
            {3, 3, 200,  20, 150, 200, 4095, 100,  4900,  11000, 200, 0, 0, 0x1000000, 0},      
            {4, 8,   0,   8,  17,  7,   70,   2, 32500, 200500,  10, 0, 0, 0x1000000, 0},
            /*{5, 1,   0,  12,  16,   8,  700,  5, 33000, 135000,    100, 0, 0, 0x1000000, 0},*/
        }
    },

	{
		NEW_DFS_JAP,
		{
		{0, 0,  10,   8,  16,   4, 2000,  5, 2900, 85000,  5, 0, 0, 0x1000000, 0},
		{1, 0, 70,  48, 126,   20, 5000,  5,  2900, 85000,  10,  0,   0, 0x1000000, 0},
		{2, 0, 100, 48, 160,   20, 5000, 25,  2900, 85000,  20,  0,   0, 0x1000000, 0},
		{3, 2, 200, 20, 150,  900, 2200, 50,  1000, 999999999,  200,  0, 999999999, 0x10000000, 0},
		}
	},

	{
		NEW_DFS_JAP_W53,
		{
		{0, 0,  10,   8,  16,   8,  2000,  5, 28000, 85000, 10, 0x1000000, 0},
		{1, 0, 32,  24,  64,   20, 2000, 5,  28000, 85000, 10, 0x1000000, 0},
		{2, 0, 100,  42, 160,   20, 2000, 25,  28000, 85000, 10, 0x1000000, 0},
		/*{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200},*/
		}
	},
};

/* 7610/7662 DFS table */
static NewDFSTable NewDFSTable2_BW20[] = {
	{
		NEW_DFS_FCC,
		{
			{0, 0, 8, 2, 7, 106, 150,  5, 2900,  80100,  5,  0, 2147483647, 0xfe808, 0x13dc},
			{1, 0, 8, 2, 7, 106, 140,  5,27600,  27900,  5,  0, 2147483647, 0xfe808, 0x19dd},
			{2, 0,40, 4,44,  96, 480,150, 2900,  80100, 40,  0, 2147483647, 0xfe808, 0x12cc},
			{3, 2,60,15,48, 640,2080, 32,19600,  40200, 32,  0, 60000000,   0x57BCF00, 0x1289},
		}
	},

	{
		NEW_DFS_EU,
		{
			{0, 0, 8, 2,  9, 106,  150, 10, 4900, 100096, 10, 0, 2147483647, 0x155cc0, 0x19cc},
			{1, 0,40, 4, 44,  96,  380,150, 4900, 100096, 40, 0, 2147483647, 0x155cc0, 0x19cc},
			{2, 3,60,20, 46, 300,  640, 80, 4900,  10100, 80, 0, 2147483647,  0x155cc0, 0x19dd},
			{3, 8, 8, 2,  9, 106,  150, 32, 4900, 296704, 32, 0, 2147483647,  0x2191c0, 0x15cc},
		}
	},

	{
		NEW_DFS_JAP_W53,
		{
			{0, 0, 8, 2, 9, 106, 150, 20,28400,  77000, 20,  0, 2147483647,  0x14c080, 0x16cc},
			{1, 0, 0, 0, 0,   0,   0,  0,    0,      0,  0,  0,          0,         0,      0},
			{2, 0,40, 4,44,  96, 200,150,28400,  77000, 60,  0, 2147483647,  0x14c080, 0x16cc},
			{3, 0, 0, 0, 0,   0,   0,  0,    0,      0,  0,  0,          0,         0,      0},
		}
	},

	{
		NEW_DFS_JAP,
		{
			{0, 0, 8, 2, 7, 106, 150,  5, 2900,  80100,  5,  0, 2147483647,  0x14c080, 0x13dc},
			{1, 0, 8, 2, 7, 106, 140,  5,27600,  27900,  5,  0, 2147483647,  0x14c080, 0x19dd},
			{2, 0,40, 4,44,  96, 480,150, 2900,  80100, 40,  0, 2147483647,  0x14c080, 0x12cc},
			{3, 2,60,15,48, 940,2080, 32,19600,  40200, 32,  0,   60000000, 0x57BCF00, 0x1289},
		}
	},
};

static NewDFSTable NewDFSTable2_BW40[] = {
	{
		NEW_DFS_FCC,
		{
			{0, 0, 8, 2, 7, 106, 150,  5, 2900,  80100,  5,  0, 2147483647, 0xfe808, 0x13dc},
			{1, 0, 8, 2, 7, 106, 140,  5,27600,  27900,  5,  0, 2147483647, 0xfe808, 0x19dd},
			{2, 0,40, 4,44,  96, 480,150, 2900,  80100, 40,  0, 2147483647, 0xfe808, 0x12cc},
			{3, 2,60,15,48, 640,2080, 32,19600,  40200, 32,  0, 60000000,   0x57BCF00, 0x1289},
		}
	},

	{
		NEW_DFS_EU,
		{
			{0, 0, 8, 2,  9, 106,  150, 10, 4900, 100096, 10, 0, 2147483647, 0x155cc0, 0x19cc},
			{1, 0,40, 4, 44,  96,  380,150, 4900, 100096, 40, 0, 2147483647, 0x155cc0, 0x19cc},
			{2, 3,60,20, 46, 300,  640, 80, 4900,  10100, 80, 0, 2147483647, 0x155cc0, 0x19dd},
			{3, 8, 8, 2,  9, 106,  150, 32, 4900, 296704, 32, 0, 2147483647, 0x2191c0, 0x15cc},
		}
	},

	{
		NEW_DFS_JAP_W53,
		{
			{0, 0, 8, 2, 9, 106, 150, 20,28400,  77000, 20,  0, 2147483647, 0x14c080, 0x16cc},
			{1, 0, 0, 0, 0,   0,   0,  0,    0,      0,  0,  0,          0,         0,      0},
			{2, 0,40, 4,44,  96, 200,150,28400,  77000, 60,  0, 2147483647, 0x14c080, 0x16cc},
			{3, 0, 0, 0, 0,   0,   0,  0,    0,      0,  0,  0,          0,         0,      0},
		}
	},

	{
		NEW_DFS_JAP,
		{
			{0, 0, 8, 2, 7, 106, 150, 5,2900,  80100, 5,  0, 2147483647, 0x14c080, 0x13dc},
			{1, 0, 8, 2, 7, 106, 140,  5,27600,  27900,  5,  0, 2147483647, 0x14c080, 0x19dd},
			{2, 0,40, 4,44,  96, 480,150, 2900,  80100, 40,  0, 2147483647, 0x14c080, 0x12cc},
			{3, 2,60,15,48, 940,2080, 32,19600,  40200, 32,  0,   60000000, 0x57BCF00, 0x1289},
		}
	},
};

static NewDFSTable NewDFSTable2_BW80[] = {
	{
		NEW_DFS_FCC,
		{
			{0, 0, 8, 2, 9, 106, 150, 15, 2900,  80100, 15,  0, 2147483647,   0xfe808, 0x16cc},
			{1, 0, 8, 2, 7, 106, 140,  5,27600,  27900,  5,  0, 2147483647,   0xfe808, 0x19dd},
			{2, 0,40, 4,44,  96, 480,150, 2900,  80100, 40,  0, 2147483647,   0xfe808, 0x12cc},
			{3, 2,60,15,48, 640,2080, 32,19600,  40200, 32,  0, 60000000,   0x57BCF00, 0x1289},
		}
	},

	{
		NEW_DFS_EU,
		{
			{0, 0, 8, 2,  9, 106,  150, 10, 4900, 100096, 10, 0, 2147483647,  0x155cc0, 0x19cc},
			{1, 0,40, 4, 44,  96,  380,150, 4900, 100096, 40, 0, 2147483647,  0x155cc0, 0x19cc},
			{2, 3,60,20, 46, 300,  640, 80, 4900,  10100, 80, 0, 2147483647,  0x155cc0, 0x19dd},
			{3, 8, 8, 2,  9, 106,  150, 32, 4900, 296704, 32, 0, 2147483647,  0x2191c0, 0x15cc},
		}
	},

	{
		NEW_DFS_JAP_W53,
		{
			{0, 0, 8, 2, 9, 106, 150, 20,28400,  77000, 20,  0, 2147483647,  0x14c080, 0x16cc},
			{1, 0, 0, 0, 0,   0,   0,  0,    0,      0,  0,  0,          0,         0,      0},
			{2, 0,40, 4,44,  96, 200,150,28400,  77000, 60,  0, 2147483647,  0x14c080, 0x16cc},
			{3, 0, 0, 0, 0,   0,   0,  0,    0,      0,  0,  0,          0,         0,      0},
		}
	},

	{
		NEW_DFS_JAP,
		{
			{0, 0, 8, 2, 9, 106, 150, 15, 2900,  80100, 15,  0, 2147483647,  0x14c080, 0x16cc},
			{1, 0, 8, 2, 7, 106, 140,  5,27600,  27900,  5,  0, 2147483647,  0x14c080, 0x19dd},
			{2, 0,40, 4,44,  96, 480,150, 2900,  80100, 40,  0, 2147483647,  0x14c080, 0x12cc},
			{3, 2,60,15,48, 940,2080, 32,19600,  40200, 32,  0,   60000000, 0x57BCF00, 0x1289},
		}
	},
};

static void dfs_sw_init(
		IN PRTMP_ADAPTER pAd);

static BOOLEAN StagerRadarCheck(
		IN PRTMP_ADAPTER pAd,
		UINT8 dfs_channel);

static BOOLEAN ChirpRadarCheck(
		IN PRTMP_ADAPTER pAd);

#ifdef RLT_BBP
static BOOLEAN MT7650DfsEventDataFetch(
		IN PRTMP_ADAPTER pAd,
		IN PRADAR_DETECT_STRUCT pRadarDetect,
		OUT PDFS_EVENT pDfsEvent);
#else
static BOOLEAN DfsEventDataFetch(
		IN PRTMP_ADAPTER pAd,
		IN PRADAR_DETECT_STRUCT pRadarDetect,
		OUT PDFS_EVENT pDfsEvent);
#endif /* MT76x0 */
static BOOLEAN DfsChannelCheck(
		IN PRTMP_ADAPTER pAd,
		IN UINT8 DfsChannel);

static VOID ChannelSelectOnRadarDetection(
		IN PRTMP_ADAPTER pAd);

static BOOLEAN DfsEventDrop(
		IN PRTMP_ADAPTER pAd,
		IN PDFS_EVENT pDfsEvent);

static inline BOOLEAN NewRadarDetectionMcuStart(PRTMP_ADAPTER pAd)
{
	/*
	   8051 firmware don't care parameter Token, Arg0 and Arg1
	 */
	return AsicSendCommandToMcu(pAd, DFS_ONOFF_MCU_CMD, 0xff, 0x01, 0x01, FALSE);
}

static inline BOOLEAN NewRadarDetectionMcuStop(PRTMP_ADAPTER pAd)
{
	/*
		8051 firmware don't care parameter Token, Arg0 and Arg1
	*/
	return AsicSendCommandToMcu(pAd, DFS_ONOFF_MCU_CMD, 0xff, 0x01, 0x00, FALSE);
}


static inline VOID DfsHwDetectionStatusGet(PRTMP_ADAPTER pAd,
											PUINT8 pDetectedChannels)
{
#ifdef RLT_BBP
	RTMP_BBP_IO_READ32(pAd, DFS_R1, pDetectedChannels);
#else
	RTMP_DFS_IO_READ8(pAd, 0x2, pDetectedChannels);
#endif /* MT76x0 */
}

static inline VOID DfsResetHwDetectionStatus(PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

#ifdef RLT_BBP
	RTMP_BBP_IO_WRITE32(pAd, DFS_R1, pRadarDetect->EnabledChMask);
#else
	RTMP_DFS_IO_WRITE8(pAd, 0x2, pRadarDetect->EnabledChMask);
#endif /* MT76x0 */
}

static inline VOID DfsCaptureModeControl(PRTMP_ADAPTER pAd,
											BOOLEAN	bEnable,
											BOOLEAN bPowerUp,
											BOOLEAN bCapLastEvents)
{
	UINT8 bbp_val = 0;

	bbp_val =  (bEnable == TRUE) | 
			((bPowerUp == TRUE) <<1) |
			((bCapLastEvents == TRUE) << 2);

#ifdef RLT_BBP
	RTMP_BBP_IO_WRITE32(pAd, DFS_R36, (UINT32) bbp_val);
#else
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, bbp_val);
#endif /* MT76x0 */
}

static inline VOID DfsDetectionEnable(PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

#ifdef RLT_BBP
	RTMP_BBP_IO_WRITE32(pAd, DFS_R0, (pRadarDetect->EnabledChMask << 16));
#else
	RTMP_DFS_IO_WRITE8(pAd, pRadarDetect->EnabledChMask, 1);
#endif /* MT76x0 */
}

static inline VOID DfsDetectionDisable(PRTMP_ADAPTER pAd)
{
#ifdef RLT_BBP
	RTMP_BBP_IO_WRITE32(pAd, DFS_R0, 0);
#else
	RTMP_DFS_IO_WRITE8(pAd, 0x1, 0);
#endif /* MT76x0 */
}

static inline VOID StagerRadarGetPRIs(PRTMP_ADAPTER pAd,
									UINT8 dfs_channel,
									PUINT32 pT1, 
									PUINT32 pT2, 
									PUINT32 pT3)
{
	UINT32 T_all = 0;
#ifdef RLT_BBP
	UINT32 bbp_val = 0;
	PDFS_PROGRAM_PARAM pDfsProgramParam = \
					&pAd->CommonCfg.RadarDetect.DfsProgramParam;
#else
	UINT8 bbp_val = 0;
#endif /* MT76x0 */

#ifdef RLT_BBP
	/* select channel */
	bbp_val = (pDfsProgramParam->ChEnable << 16) | dfs_channel;
	RTMP_BBP_IO_WRITE32(pAd, DFS_R0, bbp_val);

	RTMP_BBP_IO_READ32(pAd, DFS_R19, &T_all);
	RTMP_BBP_IO_READ32(pAd, DFS_R22, pT1);
	RTMP_BBP_IO_READ32(pAd, DFS_R25, pT2);
#else
	/* select channel */
	RTMP_DFS_IO_WRITE8(pAd, 0x0, dfs_channel);
	
	/*0.	Select dfs channel 4 and read out T1/T2/T3, 50 ns unit; 32bits */	
	RTMP_DFS_IO_READ8(pAd, 0x2d, &bbp_val);
	T_all += bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x2e, &bbp_val);	
	T_all += bbp_val<<8;
	RTMP_DFS_IO_READ8(pAd, 0x2f, &bbp_val);	
	T_all += bbp_val<<16;
	RTMP_DFS_IO_READ8(pAd, 0x30, &bbp_val);	
	T_all += bbp_val<<24;

	RTMP_DFS_IO_READ8(pAd, 0x33, &bbp_val);
	*pT1 += bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x34, &bbp_val);	
	*pT1 += bbp_val<<8;
	RTMP_DFS_IO_READ8(pAd, 0x35, &bbp_val);	
	*pT1 += bbp_val<<16;
	RTMP_DFS_IO_READ8(pAd, 0x36, &bbp_val);	
	*pT1 += bbp_val<<24;

	RTMP_DFS_IO_READ8(pAd, 0x3d, &bbp_val);
	*pT2 += bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x3e, &bbp_val);	
	*pT2 += bbp_val<<8;
	RTMP_DFS_IO_READ8(pAd, 0x3f, &bbp_val);	
	*pT2 += bbp_val<<16;
	RTMP_DFS_IO_READ8(pAd, 0x40, &bbp_val);	
	*pT2 += bbp_val<<24;
#endif /* MT76x0 */

	*pT3 = T_all - *pT1 -*pT2;
}

static inline VOID DfsHwDetectionGetPulseInfo(PRTMP_ADAPTER pAd,
										UINT8	ChannelIndex,
										PUINT32 pPeriod,
										PUINT32 pWidth1,
										PUINT32 pWidth2,
										PUINT32 pBurst)
{
#ifdef RLT_BBP
	UINT32 bbp_val = 0;
	PDFS_PROGRAM_PARAM pDfsProgramParam = \
					&pAd->CommonCfg.RadarDetect.DfsProgramParam;
#else
	UINT8 bbp_val = 0;
#endif /* MT76x0 */

	*pPeriod = *pWidth1 = *pWidth2 = *pBurst = 0;

#ifdef RLT_BBP
	/* select channel */
	bbp_val = (pDfsProgramParam->ChEnable << 16) | ChannelIndex;
	RTMP_BBP_IO_WRITE32(pAd, DFS_R0, bbp_val);

	/* Read reports - Period */
	RTMP_BBP_IO_READ32(pAd, DFS_R19, pPeriod);

	/* Read reports - Width */
	RTMP_BBP_IO_READ32(pAd, DFS_R20, pWidth1);
	RTMP_BBP_IO_READ32(pAd, DFS_R23, pWidth2);

	/* Read reports - Burst Number */
	RTMP_BBP_IO_READ32(pAd, DFS_R22, pBurst);	
#else
	/* select channel*/
	RTMP_DFS_IO_WRITE8(pAd, 0x0, ChannelIndex);

	/* Read reports - Period */
	RTMP_DFS_IO_READ8(pAd, 0x2d, &bbp_val);
	*pPeriod = bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x2e, &bbp_val);
	*pPeriod += (bbp_val << 8);
	RTMP_DFS_IO_READ8(pAd, 0x2f, &bbp_val);
	*pPeriod += (bbp_val << 16);
	RTMP_DFS_IO_READ8(pAd, 0x30, &bbp_val);
	*pPeriod += (bbp_val << 24);

	/* Read reports - Width */
	RTMP_DFS_IO_READ8(pAd, 0x31, &bbp_val);
	*pWidth1 = bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x32, &bbp_val);
	*pWidth1 += (bbp_val << 8);

	RTMP_DFS_IO_READ8(pAd, 0x37, &bbp_val);
	*pWidth2 = bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x38, &bbp_val);
	*pWidth2 += (bbp_val << 8);

	/* Read reports - Burst Number */
	RTMP_DFS_IO_READ8(pAd, 0x33, &bbp_val);
	*pBurst = bbp_val;
	RTMP_DFS_IO_READ8(pAd, 0x34, &bbp_val);
	*pBurst += (bbp_val << 8);
	RTMP_DFS_IO_READ8(pAd, 0x35, &bbp_val);
	*pBurst += (bbp_val << 16);
	RTMP_DFS_IO_READ8(pAd, 0x36, &bbp_val);
	*pBurst += (bbp_val << 24);
#endif /* MT76x0 */
}

static inline VOID DfsProgramBbpValues(PRTMP_ADAPTER pAd,
										pNewDFSTable pDfsTable)
{
	UINT8   idx, DfsInputControl;
	UINT8  DfsEngineNum = pAd->chipCap.DfsEngineNum;	
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	
#ifdef RLT_BBP
	{
		UINT32 bbp_val = 0;
		
		DfsInputControl = pDfsProgramParam->Symmetric_Round << 4;

		/* Delta Delay*/
		DfsInputControl |= (pDfsProgramParam->DeltaDelay & 0xf);
		DBGPRINT(RT_DEBUG_TRACE,("R3 = 0x%x\n", DfsInputControl));
		
		/* VGA Mask*/
		DBGPRINT(RT_DEBUG_TRACE,("VGA_Mask = 0x%x\n", pDfsProgramParam->VGA_Mask));

		/* Input Control 0 */	
		bbp_val = (pDfsProgramParam->VGA_Mask << 16) | (DfsInputControl);

		bbp_val |= (pDfsProgramParam->PwrDown_Hold_Time << 8);
		bbp_val |= (pDfsProgramParam->PwrGain_Offset << 12);

		RTMP_BBP_IO_WRITE32(pAd, DFS_R2, bbp_val);
		
		/* packet end Mask*/
		DBGPRINT(RT_DEBUG_TRACE,("Packet_End_Mask = 0x%x\n", pDfsProgramParam->Packet_End_Mask));
		
		/* Rx PE Mask*/
		DBGPRINT(RT_DEBUG_TRACE,("Rx_PE_Mask = 0x%x\n", pDfsProgramParam->Rx_PE_Mask));

		/* Input Control 1 */
		bbp_val = (pDfsProgramParam->Rx_PE_Mask << 16) | (pDfsProgramParam->Packet_End_Mask);
		RTMP_BBP_IO_WRITE32(pAd, DFS_R3, bbp_val);

		/* AGC Input Control */
		RTMP_BBP_IO_WRITE32(pAd, DFS_R32, 0x00040071);
		MT7650_ADJUST_AGC(pAd);
		MT7650_ADJUST_DFS_AGC(pAd);
		/* program each channel*/
		for (idx = 0; idx < DfsEngineNum; idx++)
		{
			/* select channel*/
			RTMP_BBP_IO_WRITE32(pAd, DFS_R0, idx);

			DBGPRINT(RT_DEBUG_TRACE, ("write DFS Channle[%d] configuration \n", idx));

			/* Detection Mode */
			bbp_val = (pDfsTable->entry[idx].mode & 0xf);

			if (pAd->chipCap.DfsEngineNum > 4 && (idx==4 || idx==5))
				bbp_val |= ((pDfsTable->entry[idx].avgLen & 0x3) << 28);
			else
				bbp_val |= ((pDfsTable->entry[idx].avgLen & 0x1ff) << 16);

			/* DFS Mode */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R4, bbp_val);

			/* DFS Energy */
			bbp_val = ((pDfsTable->entry[idx].EHigh & 0x0fff) << 16) | (pDfsTable->entry[idx].ELow & 0x0fff);
			RTMP_BBP_IO_WRITE32(pAd, DFS_R5, bbp_val);

			/* DFS Period Low */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R7, pDfsTable->entry[idx].TLow);

			/* DFS Period High */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R9, pDfsTable->entry[idx].THigh);

			/* DFS Burst Low */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R11, pDfsTable->entry[idx].BLow);
			
			/* DFS Burst High */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R13, pDfsTable->entry[idx].BHigh);
			
			/* DFS Width */
			bbp_val = ((pDfsTable->entry[idx].WHigh & 0x0fff) << 16) | (pDfsTable->entry[idx].WLow & 0x0fff);
			RTMP_BBP_IO_WRITE32(pAd, DFS_R14, bbp_val);


			/* DFS Measurement Uncertainty */
			bbp_val = (pDfsTable->entry[idx].EpsilonW << 16) | (pDfsTable->entry[idx].EpsilonT);
			RTMP_BBP_IO_WRITE32(pAd, DFS_R15, bbp_val);

			/* DFS Event Expiration */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R17, pDfsTable->entry[idx].EventExpiration);

			/* DFS Power Jump */
			RTMP_BBP_IO_WRITE32(pAd, DFS_R30, pDfsTable->entry[idx].PwrJump);
		}	

		/* Reset status */
		RTMP_BBP_IO_WRITE32(pAd, DFS_R1, pRadarDetect->EnabledChMask);

		if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
			RTMP_BBP_IO_WRITE32(pAd, DFS_R36, 0x00000003);
		
		/* Enable detection*/
		bbp_val = (pDfsProgramParam->ChEnable << 16);
		RTMP_BBP_IO_WRITE32(pAd, DFS_R0, bbp_val);
		RTMP_IO_WRITE32(pAd, 0x212C, 0x0c350001);
	}
#else
	DfsInputControl = pDfsProgramParam->Symmetric_Round << 4;

	/* Full 40Mhz*/
	if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
	{
		/* BW 40*/
		DfsInputControl |= 0x80; 
	}

	/* Delta Delay*/
	DfsInputControl |= (pDfsProgramParam->DeltaDelay & 0xf);
	RTMP_DFS_IO_WRITE8(pAd, 0x3, DfsInputControl);
	DBGPRINT(RT_DEBUG_TRACE,("R3 = 0x%x\n", DfsInputControl));
	
	/* VGA Mask*/
	RTMP_DFS_IO_WRITE8(pAd, 0x4, pDfsProgramParam->VGA_Mask);
	DBGPRINT(RT_DEBUG_TRACE,("VGA_Mask = 0x%x\n", pDfsProgramParam->VGA_Mask));
	
	/* packet end Mask*/
	RTMP_DFS_IO_WRITE8(pAd, 0x5, pDfsProgramParam->Packet_End_Mask);
	DBGPRINT(RT_DEBUG_TRACE,("Packet_End_Mask = 0x%x\n", pDfsProgramParam->Packet_End_Mask));
	
	/* Rx PE Mask*/
	RTMP_DFS_IO_WRITE8(pAd, 0x6, pDfsProgramParam->Rx_PE_Mask);
	DBGPRINT(RT_DEBUG_TRACE,("Rx_PE_Mask = 0x%x\n", pDfsProgramParam->Rx_PE_Mask));
	
	/* program each channel*/
	for (idx = 0; idx < DfsEngineNum; idx++)
	{
		/* select channel*/
		RTMP_DFS_IO_WRITE8(pAd, 0x0, idx);

		DBGPRINT(RT_DEBUG_TRACE, ("write DFS Channle[%d] configuration \n",idx));
		/* start programing*/

		/* reg 0x10, Detection Mode[2:0]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x10, (pDfsTable->entry[idx].mode & 0xf));
		
		/* reg 0x11~0x12, M[7:0] & M[8]*/

		if (pAd->chipCap.DfsEngineNum > 4 && 
			(idx==4 || idx==5))
		{
			/* Ch 4 and Ch5 use indirect M which taking the M value of another channel (one of 0~3) */
			RTMP_DFS_IO_WRITE8(pAd, 0x12, ((pDfsTable->entry[idx].avgLen << 4) & 0x30));
		}
		else
		{
			RTMP_DFS_IO_WRITE8(pAd, 0x11, (pDfsTable->entry[idx].avgLen & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x12, ((pDfsTable->entry[idx].avgLen >> 8) & 0x1));
		}

		/* reg 0x13~0x14, Energy Low[7:0] & Energy Low[11:8]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x13, (pDfsTable->entry[idx].ELow & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x14, ((pDfsTable->entry[idx].ELow >> 8) & 0xf));

		
		/* reg 0x15~0x16, Energy High[7:0] & Energy High[11:8]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x15, (pDfsTable->entry[idx].EHigh & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x16, ((pDfsTable->entry[idx].EHigh >> 8) & 0xf));

		
		/* reg 0x28~0x29, Width Low[7:0] & Width Low[11:8]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x28, (pDfsTable->entry[idx].WLow & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x29, ((pDfsTable->entry[idx].WLow >> 8) & 0xf));

		/* reg 0x2a~0x2b, Width High[7:0] & Width High[11:8]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x2a, (pDfsTable->entry[idx].WHigh & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x2b, ((pDfsTable->entry[idx].WHigh >> 8) & 0xf));

		/* reg 0x2c, Width Delta[7:0], (Width Measurement Uncertainty)*/
		RTMP_DFS_IO_WRITE8(pAd, 0x2c, (pDfsTable->entry[idx].EpsilonW & 0xff));

		/* reg 0x17~0x1a, Period Low[7:0] & Period Low[15:8] & Period Low[23:16] & Period Low[31:24]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x17, (pDfsTable->entry[idx].TLow & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x18, ((pDfsTable->entry[idx].TLow >> 8) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x19, ((pDfsTable->entry[idx].TLow >> 16) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1a, ((pDfsTable->entry[idx].TLow >> 24) & 0xff));

		/* reg 0x1b~0x1e, Period High[7:0] & Period High[15:8] & Period High[23:16] & Period High[31:24]*/
		RTMP_DFS_IO_WRITE8(pAd, 0x1b, (pDfsTable->entry[idx].THigh & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1c, ((pDfsTable->entry[idx].THigh >> 8) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1d, ((pDfsTable->entry[idx].THigh >> 16) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1e, ((pDfsTable->entry[idx].THigh >> 24) & 0xff));

		/* reg 0x27, Period Delt[7:0], (Period Measurement Uncertainty)*/
		RTMP_DFS_IO_WRITE8(pAd, 0x27, (pDfsTable->entry[idx].EpsilonT & 0xff));
		
		if (pDfsProgramParam->RadarEventExpire[idx] != 0)
		{
			RTMP_DFS_IO_WRITE8(pAd,0x39, (pDfsTable->entry[idx].EventExpiration & 0xff));
			RTMP_DFS_IO_WRITE8(pAd,0x3a, ((pDfsTable->entry[idx].EventExpiration >> 8) & 0xff) );
			RTMP_DFS_IO_WRITE8(pAd,0x3b, ((pDfsTable->entry[idx].EventExpiration >> 16) & 0xff));
			RTMP_DFS_IO_WRITE8(pAd,0x3c, ((pDfsTable->entry[idx].EventExpiration >> 24) & 0xff));
		}
		
	}	

	/* reset status */
	RTMP_DFS_IO_WRITE8(pAd, 0x2, pRadarDetect->EnabledChMask);
	/* Enable detection*/
	RTMP_DFS_IO_WRITE8(pAd, 0x1, (pDfsProgramParam->ChEnable));
#endif /* MT76x0 */
}

#ifdef RTMP_MAC_PCI
static VOID SwCheckDfsEvent(
		IN PRTMP_ADAPTER pAd);
#endif /* RTMP_MAC_PCI  */


/*
    ========================================================================
    Routine Description:
        Radar wave detection. The API should be invoke each second.
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID ApRadarDetectPeriodic(
	IN PRTMP_ADAPTER pAd)
{
	INT	i;

	if (ScanRunning(pAd) == TRUE)
		return;

	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
		pAd->Dot11_H.InServiceMonitorCount++;

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].RemainingTimeForUse > 0)
		{
			pAd->ChannelList[i].RemainingTimeForUse --;
			if ((pAd->Mlme.PeriodicRound%5) == 0)
			{
				DBGPRINT(RT_DEBUG_INFO, ("RadarDetectPeriodic - ch=%d, RemainingTimeForUse=%d\n",
					pAd->ChannelList[i].Channel, pAd->ChannelList[i].RemainingTimeForUse));
			}
		}
	}
	/*radar detect*/
	if ((pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
	{
		RadarDetectPeriodic(pAd);
	}
	return;
}


/* 	0 = Switch Channel when Radar Hit (Normal mode) 
	1 = Don't Switch Channel when Radar Hit */
INT	Set_RadarDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	pRadarDetect->McuRadarDebug = simple_strtol(arg, 0, 16);

	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_SHOW_RAW_EVENT)
		printk("Show raw data of the event buffer.\n");

	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_EVENT)
	{
		if (pRadarDetect->bDfsSwDisable == 1)
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): Warning!! DfsSwDisable is 1\n", __FUNCTION__));
		printk("Show effective event\n");
	}

	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_SILENCE)
		printk("Silence\n");

	if (pRadarDetect->McuRadarDebug & RADAR_DONT_SWITCH)
		printk("Dont Switch Channel\n");
	
	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_DONT_CHECK_BUSY)
		printk("Dont Check Channel Busy\n");
	
	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_DONT_CHECK_RSSI)
		printk("Dont Check RSSI\n");
	
	if (pRadarDetect->McuRadarDebug & RADAR_SIMULATE)
		printk("Simulate a radar detection\n");
	
	return TRUE;
}

INT	Set_ResetRadarHwDetect_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DfsResetHwDetectionStatus(pAd);
	return TRUE;
}

INT Set_DfsSwDisable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	
	pRadarDetect->bDfsSwDisable = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("pRadarDetect->bDfsSwDisable = %u\n", pRadarDetect->bDfsSwDisable));
	return TRUE;
}

INT Set_DfsEnvtDropAdjTime_Proc(
	IN PRTMP_ADAPTER   pAd, 
	IN PSTRING  arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->EvtDropAdjTime = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("EventDropAdjTime = %u\n", pDfsSwParam->EvtDropAdjTime));
	return TRUE;
}

INT	Set_RadarStart_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;
	
	if (simple_strtol(arg, 0, 10) == 0)
	{
		NewRadarDetectionStart(pAd);
	}
	else if ((simple_strtol(arg, 0, 10) >= 1) && (simple_strtol(arg, 0, 10) <= pAd->CommonCfg.RadarDetect.EnabledChMask))
	{
		pDfsProgramParam->ChEnable = simple_strtol(arg, 0, 10);
		printk("Ch Enable == 0x%x\n", pDfsProgramParam->ChEnable);
		NewRadarDetectionStart(pAd);
	}
	
	return TRUE;
}

INT	Set_RadarStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	NewRadarDetectionStop(pAd);
	return TRUE;
}

#ifdef DFS_ATP_SUPPORT
INT Set_DfsAtpStart_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg)
{
	INT ret;

	if (simple_strtol(arg, 0, 10) == 1) {
		/* To set the HtBw... */
		ret = Set_AP_SSID_Proc(pAd, pAd->CommonCfg.Ssid);
		/* reset the flag */
		pAd->CommonCfg.RadarDetect.atp_radar_detect = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: reset atp_radar_detect\n", __FUNCTION__));
		/* set not switch channel */
		pAd->CommonCfg.RadarDetect.McuRadarDebug = RADAR_DONT_SWITCH;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: channel do not switch\n", __FUNCTION__));
		if (ret) {
			printk ("AP ATP DFS ready\n");
		}
	}
	else if (simple_strtol(arg, 0, 10) == 0) {
		/* reset the flag */
		pAd->CommonCfg.RadarDetect.atp_radar_detect = FALSE;
		pAd->CommonCfg.RadarDetect.atp_set_ht_bw = FALSE;
		pAd->CommonCfg.RadarDetect.atp_set_vht_bw = FALSE;
		pAd->CommonCfg.RadarDetect.atp_set_channel_ready = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: reset atp_radar_detect\n", __FUNCTION__));
		pAd->CommonCfg.RadarDetect.McuRadarDebug = 0x0;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: channel do not switch\n", __FUNCTION__));
	}
	else {
		printk ("%s: wrong arg. 1->start, 0->stop\n", __FUNCTION__);
	}

	return TRUE;
}

INT Set_DfsAtpReset_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg)
{
	if (simple_strtol(arg, 0, 10) == 1) {
		/* reset the flag */
		pAd->CommonCfg.RadarDetect.atp_radar_detect = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: reset atp_radar_detect\n", __FUNCTION__));
		printk ("AP ATP DFS ready\n");
	}

	return TRUE;
}

INT Set_DfsAtpReport_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg)
{
	printk ("%s\n", __FUNCTION__);	
	if (pAd->CommonCfg.RadarDetect.atp_radar_detect == TRUE) {
		printk ("DfsAtpDetected\n");
	}
	else {
		printk ("DfsAtp Not Detected\n");
	}
	return TRUE;
}
#endif /* DFS_ATP_SUPPORT */

INT	Set_RadarSetTbl1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PUCHAR p2 = arg;
	ULONG idx, value;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;

	while((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}

	if (*p2 == ':')
	{
		A2Dec(idx, arg);
		A2Dec(value, p2+ 1);
		if (idx == 0)
		{
			pDfsProgramParam->DeltaDelay = value;
			printk("Delta_Delay = %d\n", pDfsProgramParam->DeltaDelay);
		}
		else
			modify_table1(pAd, idx, value);

		if (pAd->CommonCfg.RadarDetect.pDFSTable)
			NewRadarDetectionProgram(pAd, pAd->CommonCfg.RadarDetect.pDFSTable);
		else
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): Table not initialized.\n", __FUNCTION__));
	}
	else
		printk("please enter iwpriv ra0 set RadarT1=xxx:yyy\n");

	return TRUE;
}

INT	Set_RadarSetTbl2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PUCHAR p2 = arg;
	ULONG idx, value;
	
	while((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Dec(idx, arg);
		A2Dec(value, p2+ 1);
		modify_table2(pAd, idx, value);
	}
	else
		printk("please enter iwpriv ra0 set RadarT2=xxx:yyy\n");
	
	return TRUE;
}

INT	Set_PollTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->PollTime = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_PrintBusyIdle_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->print_ch_busy_sta = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_BusyIdleRatio_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->ch_busy_idle_ratio = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsRssiHigh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->DfsRssiHigh = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsRssiLow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->DfsRssiLow = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Show_BlockCh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i; 

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].RemainingTimeForUse != 0)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Ch%d: RemainingTimeForUse:%d sec;\n",
				pAd->ChannelList[i].Channel, pAd->ChannelList[i].RemainingTimeForUse));
		}
	}
	return TRUE;
}


VOID DFSInit(PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	
	pRadarDetect->ch_busy_countdown = -1;
	pRadarDetect->EnabledChMask = ((1 << pAd->chipCap.DfsEngineNum) -1);
#ifdef RLT_BBP
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		/*
			Do NOT enable Radar Channel 1. (MT7650 DFS programming guide_v2_20121012.docx)
		*/
		pRadarDetect->EnabledChMask = 0xF;
		pRadarDetect->bAdjustDfsAgc = FALSE;
		pDfsProgramParam->PwrDown_Hold_Time = 0xf;
		pDfsProgramParam->PwrGain_Offset = 0x3;
	}
#endif /* MT76x0 */
	pRadarDetect->PollTime = 3;
	pRadarDetect->DfsRssiHigh = -30;
	pRadarDetect->DfsRssiLow = -90;
	pRadarDetect->use_tasklet = 1;
	pRadarDetect->McuRadarDebug = 0;
	pRadarDetect->radarDeclared = 0;
	pDfsProgramParam->ChEnable = pRadarDetect->EnabledChMask;

#ifdef RLT_BBP
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		pDfsProgramParam->VGA_Mask = 0;
		pDfsProgramParam->Packet_End_Mask = 0;
		pDfsProgramParam->Rx_PE_Mask = (IS_MT76x2(pAd)) ? 0xFF : 0x0;
		pDfsProgramParam->DeltaDelay = 0x2;
		pDfsProgramParam->Symmetric_Round = 0;
	}
	else
#endif /* MT76x0 */
	{
		pDfsProgramParam->VGA_Mask = 45;
		pDfsProgramParam->Packet_End_Mask = 45;
		pDfsProgramParam->Rx_PE_Mask = 45;
		pDfsProgramParam->DeltaDelay = 0x3;
		pDfsProgramParam->Symmetric_Round = 1;
	}	
	/*
		s/w detection needs event buffer.
	*/
	if (pAd->chipCap.DfsEngineNum > 4)
		pRadarDetect->bDfsSwDisable = TRUE; 	/* Default close s/w detection for new DFS*/
	
	dfs_sw_init(pAd);
}

void NewRadarDetectionStart(PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pNewDFSTable *ppDFSTable = &pRadarDetect->pDFSTable;

	pRadarDetect->bDfsInit = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("--->NewRadarDetectionStart()\n"));

	DFSInit(pAd);


	RTMP_CHIP_RADAR_GLRT_COMPENSATE(pAd);

	if ((pAd->CommonCfg.RDDurRegion == CE) && RESTRICTION_BAND_1(pAd))
		pAd->Dot11_H.ChMovingTime = 605;
	else
		pAd->Dot11_H.ChMovingTime = 65;

	if (pAd->CommonCfg.RDDurRegion == FCC)
	{
		if (pRadarDetect->ch_busy_idle_ratio == 0)
			pRadarDetect->ch_busy_idle_ratio = 2;
		
		*ppDFSTable = &NewDFSTable1[0];
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use FCC table\n"));
	}
	else if (pAd->CommonCfg.RDDurRegion == CE)
	{
		if (pRadarDetect->ch_busy_idle_ratio == 0)
			pRadarDetect->ch_busy_idle_ratio = 3;
		
		*ppDFSTable = &NewDFSTable1[1];
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use CE table\n"));
	}
	else /* JAP*/
	{
		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			*ppDFSTable = &NewDFSTable1[3];
			
			if (pRadarDetect->ch_busy_idle_ratio == 0)
				pRadarDetect->ch_busy_idle_ratio = 3;
		}
		else
		{
			*ppDFSTable = &NewDFSTable1[2];

			if (pRadarDetect->ch_busy_idle_ratio == 0)
				pRadarDetect->ch_busy_idle_ratio = 2;
		}
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use JAP table\n"));
	}	

#ifdef RLT_BBP
	if (IS_MT7610E(pAd) || IS_MT76x2(pAd))
	{
		if (pAd->CommonCfg.BBPCurrentBW == BW_80) {
			// New update: 20130606
			if (pAd->CommonCfg.RDDurRegion == CE) {
				*ppDFSTable = &NewDFSTable2_BW80[1];
			}
			else if (pAd->CommonCfg.RDDurRegion == FCC) {
				*ppDFSTable = &NewDFSTable2_BW80[0];
			}
			else if ((pAd->CommonCfg.Channel >= 52) 
					&& (pAd->CommonCfg.Channel <= 64)) { 	
				/* JAP_W53 */
				*ppDFSTable = &NewDFSTable2_BW80[2];
			}
			else {
				/* JAP_W56 */
				*ppDFSTable = &NewDFSTable2_BW80[3];
			}
		}
		else if (pAd->CommonCfg.BBPCurrentBW == BW_40) {
			// New update: 20130606
			if (pAd->CommonCfg.RDDurRegion == CE) {
				*ppDFSTable = &NewDFSTable2_BW40[1];
			}
			else if (pAd->CommonCfg.RDDurRegion == FCC) {
				*ppDFSTable = &NewDFSTable2_BW40[0];
			}
			else if ((pAd->CommonCfg.Channel >= 52) 
					&& (pAd->CommonCfg.Channel <= 64)) { 	
				/* JAP_W53 */
				*ppDFSTable = &NewDFSTable2_BW40[2];
			}
			else {
				/* JAP_W56 */
				*ppDFSTable = &NewDFSTable2_BW40[3];
			}
		}
		else if (pAd->CommonCfg.BBPCurrentBW == BW_20){
			if (pAd->CommonCfg.RDDurRegion == CE) {
				*ppDFSTable = &NewDFSTable2_BW20[1];
			}
			else if (pAd->CommonCfg.RDDurRegion == FCC) {
				*ppDFSTable = &NewDFSTable2_BW20[0];
			}
			else if ((pAd->CommonCfg.Channel >= 52) 
					&& (pAd->CommonCfg.Channel <= 64)) { 	
				/* JAP_W53 */
				*ppDFSTable = &NewDFSTable2_BW20[2];
			}
			else {
				/* JAP_W56 */
				*ppDFSTable = &NewDFSTable2_BW20[3];
		}
	}
		else
		{}
	}
#endif /* MT76x0 */

	NewRadarDetectionProgram(pAd, (*ppDFSTable));

#ifdef RTMP_MAC_PCI
	/* enable debug mode*/
	DfsCaptureModeControl(pAd, TRUE, TRUE, FALSE);

	RTMP_HW_TIMER_INT_SET(pAd, HW_TIMER_INTERVAL); /* 1ms Timer interrupt */
	RTMP_HW_TIMER_INT_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */
	pRadarDetect->bDfsInit = TRUE;
	DBGPRINT(RT_DEBUG_TRACE,("Poll Time=%d\n", pRadarDetect->PollTime));
}

VOID NewRadarDetectionStop(
	IN PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStop\n"));

	/* set init bit = false to prevent dfs rotines */
	pRadarDetect->bDfsInit = FALSE;
	pRadarDetect->radarDeclared = 0;
	
	/* Disable detection*/
	DfsDetectionDisable(pAd);

	/* Clear status */
	DfsResetHwDetectionStatus(pAd);
	
#ifdef RTMP_MAC_PCI
	RTMP_HW_TIMER_INT_SET(pAd, 0);
	RTMP_HW_TIMER_INT_DISABLE(pAd);
#endif /* RTMP_MAC_PCI */
#ifdef CARRIER_DETECTION_SUPPORT
	if ((pAd->chipCap.carrier_func == TONE_RADAR_V3) &&
		pAd->CommonCfg.CarrierDetect.Enable == 0)
#endif
	{
		RTMP_IO_WRITE32(pAd, 0x212C, 0x00);
	}
}


/* the debug port have timestamp 22 digit, the max number is 0x3fffff, each unit is 25ns for 40Mhz mode and 50ns for 20Mhz mode*/
/* so a round of timestamp is about 25 * 0x3fffff / 1000 = 104857us (about 100ms) or*/
/* 50 * 0x3fffff / 1000 = 209715us (about 200ms) in 20Mhz mode*/
/* 3ms = 3000,000 ns / 25ns = 120000 -- a unit */
/* 0x3fffff/120000 = 34.9 ~= 35*/
/* CE Staggered radar check*/
/* At beginning, the goal is to detect staggered radar, now, we also detect regular radar with this function.*/


int SWRadarCheck(
	IN PRTMP_ADAPTER pAd, USHORT id)
{
	int i, j, start_idx, end_idx;
	pNewDFSDebugPort pCurrent, p1, pEnd;
	ULONG period;
	int radar_detected = 0;
	USHORT	widthsum;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;	
	/*ENTRY_PLUS could be replace by (pDfsSwParam->sw_idx[id]+1)%128*/
	USHORT	Total, SwIdxPlus = ENTRY_PLUS(pDfsSwParam->sw_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	UCHAR	CounterToCheck;	

#ifdef RLT_BBP
	/* only engine 0 and engine2 support event buffer */
	if (id == 1)
		return 0;
#endif /* MT76x0 */
	
	if (!DFS_CHECK_FLAGS(pAd, pRadarDetect) ||
		(SwIdxPlus == pDfsSwParam->hw_idx[id]))  /* no entry to process*/
		return 0;
	
	/* process how many entries?? total NEW_DFS_DBG_PORT_ENT_NUM*/
	if (pDfsSwParam->hw_idx[id] > SwIdxPlus)
		Total = pDfsSwParam->hw_idx[id] - SwIdxPlus;
	else
		Total = pDfsSwParam->hw_idx[id] + NEW_DFS_DBG_PORT_ENT_NUM - SwIdxPlus;
	
	if (Total > NEW_DFS_DBG_PORT_ENT_NUM)
		pDfsSwParam->pr_idx[id] = ENTRY_PLUS(pDfsSwParam->sw_idx[id], MAX_PROCESS_ENTRY, NEW_DFS_DBG_PORT_ENT_NUM);
	else
		pDfsSwParam->pr_idx[id] = ENTRY_PLUS(pDfsSwParam->sw_idx[id], Total, NEW_DFS_DBG_PORT_ENT_NUM);
	
	
	start_idx = ENTRY_PLUS(pDfsSwParam->pr_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	end_idx = pDfsSwParam->pr_idx[id];
	
	pEnd = &pDfsSwParam->DFS_W[id][end_idx];
	
	if (start_idx > end_idx)
		end_idx += NEW_DFS_DBG_PORT_ENT_NUM;
	
	
	pDfsSwParam->sw_idx[id] = pDfsSwParam->pr_idx[id];
	
	/* FCC && Japan*/

	if (pAd->CommonCfg.RDDurRegion != CE)
	{
		ULONG minPeriod = (3000 << 1);
		/* Calculate how many counters to check*/
		/* if pRadarDetect->PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz*/
		/* if pRadarDetect->PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz*/
		/* if pRadarDetect->PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz*/
		/* if pRadarDetect->PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz*/
		/* the max period to check for 40Mhz for FCC is 28650 * 2*/
		/* the max period to check for 40Mhz for Japan is 80000 * 2*/
		/* 0x40000 = 4194304 / 57129 = 73.xxx*/
		/* 0x40000 = 4194304 / 160000 = 26.2144*/
		/* 53/73 < 1 (1+1)*/
		/* 53/26.2144 = 2.02... (2+1)*/
		/* 27/26.2144 = 1.02... (1+1)*/
		/* 20M should use the same value as 40Mhz mode*/

		if (pRadarDetect->MCURadarRegion == NEW_DFS_JAP_W53)
		{
			minPeriod = 28500 << 1;
		}
		
		if (pAd->CommonCfg.RDDurRegion == FCC)
		{
			CounterToCheck = 1+1; 
		}
		else /* if (pAd->CommonCfg.RDDurRegion == JAP)*/
		{
			if (pRadarDetect->PollTime <= 2)
				CounterToCheck = 2+1;
			else
				CounterToCheck = 1+1;
		}

		/* First Loop for FCC/JAP*/
		for (i = end_idx; i > start_idx; i--)
		{
			pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			/* we only handle entries has same counter with the last one*/
			if (pCurrent->counter != pEnd->counter)
				break;
		
			pCurrent->start_idx = 0xffff;

			/* calculate if any two pulse become a valid period, add it in period table,*/
			for (j = i - 1; j > start_idx; j--)
			{
				p1 = &pDfsSwParam->DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
				
				/* check period, must within max period*/
				if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
				{
					if (p1->counter + CounterToCheck < pCurrent->counter)
						break;
            	
					widthsum = p1->width + pCurrent->width;
					if (id == 0)
					{
						if (widthsum < 600)
							pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
						else
							pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
					
					if ( (pAd->Dot11_H.RDMode == RD_SILENCE_MODE) ||
						 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
						
						if ((period >= (minPeriod - 2)) && (period <= pDfsSwParam->dfs_max_period))
						{
							/* add in period table*/
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
            	
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
							pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
							
							pDfsSwParam->dfs_t_idx[id]++;
							if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pDfsSwParam->dfs_t_idx[id] = 0;
						}
						else if (period > pDfsSwParam->dfs_max_period)
							break;
					}
				}
				else
				{
					if (p1->counter + CounterToCheck < pCurrent->counter)
						break;
					
					widthsum = p1->width + pCurrent->width;
					if (id == 0)
					{
						if (widthsum < 600)
							pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
						else
							pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;

            	
					if ( (pAd->Dot11_H.RDMode == RD_SILENCE_MODE) || 
						 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )
            	
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
            	
						if ((period >= ((minPeriod >> 1) - 2)) && (period <= (pDfsSwParam->dfs_max_period >> 1)))
						{
							/* add in period table*/
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
							
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
							pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
							
							pDfsSwParam->dfs_t_idx[id]++;
							if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pDfsSwParam->dfs_t_idx[id] = 0;
						}
						else if (period > (pDfsSwParam->dfs_max_period >> 1))
							break;
					}
				}
			} /* for (j = i - 1; j > start_idx; j--)*/
		} /* for (i = end_idx; i > start_idx; i--)*/

		/* Second Loop for FCC/JAP*/
		for (i = end_idx; i > start_idx; i--)
		{
			pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			/* we only handle entries has same counter with the last one*/
			if (pCurrent->counter != pEnd->counter)
				break;
			if (pCurrent->start_idx != 0xffff)
			{
				/*pNewDFSDebugPort	p2, p3, p4, p5, p6;*/
				pNewDFSDebugPort	p2, p3;
				pNewDFSMPeriod pCE_T;
				ULONG idx[10], T[10];

				for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
				{
					pCE_T = &pDfsSwParam->DFS_T[id][idx[0]];
				
					p2 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];
				
					if (p2->start_idx == 0xffff)
						continue;
				
					T[0] = pCE_T->period;

					for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
					{
						
						pCE_T = &pDfsSwParam->DFS_T[id][idx[1]];
					
						p3 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];

						if (idx[0] == idx[1])
							continue;
						
						if (p3->start_idx == 0xffff)
							continue;
					
						T[1] = pCE_T->period;
						
						if ( PERIOD_MATCH(T[0], T[1], pDfsSwParam->dfs_period_err))
						{
							if (id <= 2) /* && (id >= 0)*/
							{
								/*if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > minPeriod)) ||*/
								/*	((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > (minPeriod >> 1))) )*/
								{
									unsigned int loop, PeriodMatched = 0, idx1;
									for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
									{
										idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
										if (PERIOD_MATCH(pDfsSwParam->DFS_T[id][idx1].period, T[1], pDfsSwParam->dfs_period_err))
											PeriodMatched++;										
									}
								
									if (PeriodMatched > pDfsSwParam->dfs_declare_thres)
									{
										{
											pNewDFSValidRadar pDFSValidRadar;
											ULONG T1 = (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)? (T[1]>>1) : T[1];
											
											pDFSValidRadar = &NewDFSValidTable[0];
                    					
											while (pDFSValidRadar->type != NEW_DFS_END)
											{
												if ((pDFSValidRadar->type & pRadarDetect->MCURadarRegion) == 0)
												{
													pDFSValidRadar++;
													continue;
												}
												
												if (pDFSValidRadar->TLow)
												{
													if ( (T1 > (pDFSValidRadar->TLow - pDFSValidRadar->TMargin)) && 
													     (T1 < (pDFSValidRadar->THigh + pDFSValidRadar->TMargin)) )
													{
														radar_detected = 1;
													}
												}
												else
												{
													if ( (T1 > (pDFSValidRadar->T - pDFSValidRadar->TMargin)) &&
													     (T1 < (pDFSValidRadar->T + pDFSValidRadar->TMargin)) )
													{
														radar_detected = 1;
														break;
													}
												}												
												
												pDFSValidRadar++;
											}
											if (radar_detected == 1)
											{
												DBGPRINT(RT_DEBUG_TRACE, ("W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T1, (unsigned int)id, PeriodMatched));
												printk("SWRadarCheck: Radar Detected\n");
												return radar_detected;
											}
											else if (pRadarDetect->MCURadarRegion != NEW_DFS_JAP_W53)
												DBGPRINT(RT_DEBUG_TRACE, ("W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T1, (unsigned int)id, PeriodMatched));
										}
									}								
								}
							} /* if (id <= 2)  && (id >= 0)*/
						}
					} /* for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)*/

					/* increase FCC-1 detection*/
					if (id <= 2)
					{
						if (IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
							int loop, idx1, PeriodMatched_fcc1 = 0;
							for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
							{
								idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
								if ( IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pDfsSwParam->DFS_T[id][idx1].period) )
								{
									/*printk("%d %d %d\n", PeriodMatched_fcc1, pDfsSwParam->DFS_T[id][idx1].period, loop);*/
									PeriodMatched_fcc1++;
								}
							}
								
							if (PeriodMatched_fcc1 > 3)
							{
								DBGPRINT(RT_DEBUG_TRACE, ("PeriodMatched_fcc1 = %d (%d)\n", PeriodMatched_fcc1, id));
								radar_detected = 1;
								return radar_detected;
							}
						}
					}


					/* increase W56-3 detection*/
					if ((pRadarDetect->MCURadarRegion == NEW_DFS_JAP) && (id <= 2))
					{
						if (IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
							int loop, idx1, PeriodMatched_w56_3 = 0;
							for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
							{
								idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
								if ( IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pDfsSwParam->DFS_T[id][idx1].period) )
								{
									/*printk("%d %d %d\n", PeriodMatched_w56_3, pDfsSwParam->DFS_T[id][idx1].period, loop);*/
									PeriodMatched_w56_3++;
								}
							}
								
							if (PeriodMatched_w56_3 > 3)
							{
								DBGPRINT(RT_DEBUG_TRACE, ("PeriodMatched_w56_3 = %d (%d)\n", PeriodMatched_w56_3, id));
								radar_detected = 1;
								return radar_detected;
							}
						}
					}

					if ((pRadarDetect->MCURadarRegion == NEW_DFS_JAP_W53) && (id <= 2) && IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
					{
						int loop, idx1, PeriodMatched_W56_2 = 0;
						
						for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
						{
							idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
							if ( IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pDfsSwParam->DFS_T[id][idx1].period) )
							{
								/*printk("%d %d %d\n", PeriodMatched_W56_2, pDfsSwParam->DFS_T[id][idx1].period, loop);*/
								PeriodMatched_W56_2++;
							}
						}
						
						if (PeriodMatched_W56_2 >= 3)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("PeriodMatched_W56_2 = %d(%d)\n", PeriodMatched_W56_2, id));
							radar_detected = 1;
							return radar_detected;
						}
					}
				} /* for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)*/
			} /* if (pCurrent->start_idx != 0xffff)*/
		} /* for (i = end_idx; i > start_idx; i--)*/
		
		return radar_detected;
	}

	/* CE have staggered radar	*/
	
	/* Calculate how many counters to check*/
	/* if pRadarDetect->PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz*/
	/* if pRadarDetect->PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz*/
	/* if pRadarDetect->PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz*/
	/* if pRadarDetect->PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz*/
	/* if pRadarDetect->PollTime is 8ms, a round of timestamp is ?? for 20Mhz, 12 for 40Mhz*/
	/* the max period to check for 40Mhz is 133333 + 125000 + 117647 = 375980*/
	/* 0x40000 = 4194304 / 375980 = 11.1556*/
	/* 53/11.1556 = 4.75...*/
	/* 35/11.1556 = 3.1374, (4+1) is safe, (3+1) to save CPU power, but may lost some data*/
	/* 27/11.1556 = 2.42, (3+1) is OK*/
	/* 21/11.1556 = 1.88, (2+1) is OK*/
	/* 20M should use the same value as 40Mhz mode*/
	if (pRadarDetect->PollTime == 1)
		CounterToCheck = 5+1;
	else if (pRadarDetect->PollTime == 2)
		CounterToCheck = 4+1;
	else if (pRadarDetect->PollTime == 3)
		CounterToCheck = 3+1;
	else if (pRadarDetect->PollTime <= 8)
		CounterToCheck = 2+1;
	else
		CounterToCheck = 1+1;

	/* First Loop for CE*/
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		/* we only handle entries has same counter with the last one*/
		if (pCurrent->counter != pEnd->counter)
			break;
		
		pCurrent->start_idx = 0xffff;

		/* calculate if any two pulse become a valid period, add it in period table,*/
		for (j = i - 1; j > start_idx; j--)
		{
			p1 = &pDfsSwParam->DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
			

			/* check period, must within 16666 ~ 66666*/
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				if (p1->counter + CounterToCheck < pCurrent->counter)
						break;

				widthsum = p1->width + pCurrent->width;
				if (id == 0)
				{
					if (((p1->width > 310) && (pCurrent->width < 300)) || ((pCurrent->width > 310) && ((p1->width < 300))) )
						continue;
					if (widthsum < 620)
						pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_H;
					else
						pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
					
				}
				else if (id == 1)
					pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch1_Shift;
				else if (id == 2)
					pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
				
				if ( (pAd->Dot11_H.RDMode == RD_SILENCE_MODE) ||
					 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )
				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;
					
					/*if ((period >= (33333 - 20)) && (period <= (133333 + 20)))*/
					if ((period >= (33333 - 20)) && (period <= pDfsSwParam->dfs_max_period))
					//if ((period >= (10000 - 2)) && (period <= pDfsSwParam->dfs_max_period))
					{
						/* add in period table*/
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
        
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
						pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
						
						pDfsSwParam->dfs_t_idx[id]++;
						if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pDfsSwParam->dfs_t_idx[id] = 0;
					}
					else if (period > pDfsSwParam->dfs_max_period) /* to allow miss a pulse*/
						break;
				}
				
			}
			else
			{
				if (p1->counter + CounterToCheck < pCurrent->counter)
					break;
				
				widthsum = p1->width + pCurrent->width;
				if (id == 0)
				{
					if (((p1->width > 300) && (pCurrent->width < 300)) || ((pCurrent->width > 300) && ((p1->width < 300))) )
						continue;
					if (widthsum < 620)
						pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_H;
					else
						pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
				}
				else if (id == 1)
				{
					pDfsSwParam->dfs_width_diff = widthsum >> 3;  /* for 20M verified */
					//printk("dfs_width_diff = %u\n",pDfsSwParam->dfs_width_diff);
				}
				else if (id == 2)
					pDfsSwParam->dfs_width_diff = widthsum >> 6;

				if ( (pAd->Dot11_H.RDMode == RD_SILENCE_MODE) || 
					 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )

				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;

					//if ((period >= (5000 - 2)) && (period <= (pDfsSwParam->dfs_max_period >> 1)))
					if ((period >= (16666 - 20)) && (period <= (pDfsSwParam->dfs_max_period >> 1)))//neil modify for ce 5-1
					{
						/* add in period table*/
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
						
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
						pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
						
						pDfsSwParam->dfs_t_idx[id]++;
						if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pDfsSwParam->dfs_t_idx[id] = 0;
					}
					else if (period > (pDfsSwParam->dfs_max_period >> 1))
						break;
				}
			}
		} /* for (j = i - 1; j > start_idx; j--)*/
	}

	/* Second Loop for CE*/
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		/* we only handle entries has same counter with the last one*/
		if (pCurrent->counter != pEnd->counter)
			break;
		
		/* Check Staggered radar*/
		if (pCurrent->start_idx != 0xffff)
		{
			pNewDFSDebugPort	p2, p3;
			pNewDFSMPeriod pCE_T;
			ULONG idx[10], T[10];
			
			/*printk("pCurrent=%d, idx=%d~%d\n", pCurrent->timestamp, pCurrent->start_idx, pCurrent->end_idx);*/

			for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
			{
				pCE_T = &pDfsSwParam->DFS_T[id][idx[0]];
				
				p2 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];
				
				/*printk("idx[0]= %d, idx=%d p2=%d, idx=%d~%d\n", idx[0], pCE_T->idx2, p2->timestamp, p2->start_idx, p2->end_idx);*/
				
				if (p2->start_idx == 0xffff)
					continue;
				
				T[0] = pCE_T->period;


				for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
				{
					
					pCE_T = &pDfsSwParam->DFS_T[id][idx[1]];
					
					p3 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];
					
					/*printk("p3=%d, idx=%d~%d\n", p3->timestamp, p3->start_idx, p3->end_idx);*/

					if (idx[0] == idx[1])
						continue;
						
					if (p3->start_idx == 0xffff)
						continue;
					


					T[1] = pCE_T->period;

		
					if (PERIOD_MATCH(T[0], T[1], pDfsSwParam->dfs_period_err))
					{
						if (id <= 2) /* && (id >= 0)*/
						{

							
							if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > 66666)) ||
								((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > 33333)) )
							{
								unsigned int loop, PeriodMatched = 0, idx1;
								
								for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
								{
									idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
									if (PERIOD_MATCH(pDfsSwParam->DFS_T[id][idx1].period, T[1], pDfsSwParam->dfs_period_err))
									{
										/*printk("%d %d\n", loop, pDfsSwParam->DFS_T[id][idx[1]-loop].period);*/
										PeriodMatched++;
									}
								}
								
								if (PeriodMatched > pDfsSwParam->dfs_declare_thres)
								{
									printk("Radar Detected(CE), W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T[1], (unsigned int)id, PeriodMatched);

									if (PeriodMatched > (pDfsSwParam->dfs_declare_thres + 1))
 								      		radar_detected = 1;
									return radar_detected;
								}								
							}
						}
					}
				} /* for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)*/
			} /* for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)*/
		}
	} /* for (i = end_idx; i < start_idx; i--)*/
	
	return radar_detected;
}

/* 
    ==========================================================================
    Description:
		Recheck DFS radar of stager type.
	Arguments:
	    pAdapter                    Pointer to our adapter
	    dfs_channel                DFS detect channel
    Return Value:
        "TRUE" if check pass, "FALSE" otherwise.
    Note:
    ==========================================================================
 */
static BOOLEAN StagerRadarCheck(IN PRTMP_ADAPTER pAd, UINT8 dfs_channel)
{
	UINT T1=0, T2=0, T3=0, F1, F2, F3 = 0, Fmax = 0, freq_diff_min, freq_diff_max;
	UINT8  dfs_stg2=0;
	UINT F_MAX, F_MID, F_MIN;

	DBGPRINT(RT_DEBUG_TRACE, ("--->StagerRadarCheck()\n"));
	StagerRadarGetPRIs(pAd, dfs_channel, &T1, &T2, &T3);

	if (T3 < 5)
		T3 = 0;
	
	/*1.	Check radar stagger2 or stagger3*/
	if (T3 == 0 || ((T3 > (T1 + T2) ? (T3 - T1 - T2) : (T1 + T2 - T3)) < 25))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("stg2 confirmed\n"));
		dfs_stg2 =1;
		F1 = 20000000/T1; /*hz*/
		F2 = 20000000/T2;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("stg3 confirmed\n"));
		F1 = 20000000/T1; /*hz*/
		F2 = 20000000/T2;
		F3 = 20000000/T3;
	}

	F_MAX = (F1 > F2) ? ( (F1 > F3) ? F1 : F3 ) : ( (F2 > F3) ? F2 : F3 );
	F_MIN = (F1 < F2) ? ( (F1 < F3) ? F1 : F3 ) : ( (F2 < F3) ? F2 : F3 );  	
	F_MID = (F1 > F2) ? ((F1 < F3) ? F1 : ( (F2 > F3) ?  F2 : F3 )) : ( F2 < F3 ? F2 : ((F1 > F3) ? F1 :F3)  );
	
	DBGPRINT(RT_DEBUG_TRACE, ("F_MAX=%d F_MID=%d F_MIN=%d\n", F_MAX, F_MID, F_MIN));
	
	F1 = F_MAX;	
	F2 = F_MID;	
	F3 = F_MIN;	
         
	Fmax = F1;
	
	/*2.	Check radar type 5 or type6*/
	if (Fmax>295 && Fmax<=405)
	{	
		DBGPRINT(RT_DEBUG_TRACE, ("type5 confirmed\n"));
		freq_diff_min = 20;
		freq_diff_max = 50;
	}
	else if (Fmax>405 && Fmax<=1205) /* tolerate more range for looser type6 */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("type6 confirmed\n"));
		freq_diff_min = 80;
		freq_diff_max = 400;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck failed, T1=%d, T2=%d, T3=%d\n", T1, T2, T3));
		return FALSE;
	}
	
	/*3.	According to decision of stagger and type do period check */
	if (dfs_stg2 == 1)
	{
        UINT freq_diff = (F1 - F2);
        	
        DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck freq_diff_min=%d freq_diff_max=%d \n", freq_diff_min, freq_diff_max));	
        DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck dfs_stg2, dff=%d  \n", freq_diff));		
        	
		if ((freq_diff >= freq_diff_min) && (freq_diff <= freq_diff_max))
			return TRUE; /* S/W check success */
		else
		{			
            DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck failed, F1=%d, F2=%d\n", F1, F2));
            DBGPRINT(RT_DEBUG_TRACE, ("stg2 fail on S/W Freq confirmed\n"));
			return FALSE; 	/* S/W check fail */
		}
	}
	else /* dfs_stg3 */
	{
        UINT freq_diff_1 = (F1 - F2);
        UINT freq_diff_2 = (F2 - F3);
        
        	
        DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck freq_diff_min=%d freq_diff_max=%d \n", freq_diff_min, freq_diff_max));		
        DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck dfs_stg3, dff_1=%d, dff_2=%d \n", freq_diff_1, freq_diff_2));	
        	

		if( (freq_diff_1 >= freq_diff_min) && (freq_diff_1 <= freq_diff_max) && (freq_diff_2 >= freq_diff_min) && (freq_diff_2 <= freq_diff_max) )
			return TRUE; /* S/W check success */
		else
		{
            DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck failed, F1=%d, F2=%d, F3=%d\n", F1, F2, F3));
            DBGPRINT(RT_DEBUG_TRACE, ("stg3 fail on S/W Freq confirmed\n"));
			return FALSE;   /* S/W check fail */
		}
	}
	
    DBGPRINT(RT_DEBUG_TRACE, ("<---StagerRadarCheck()\n"));
}


/* 
    ==========================================================================
    Description:
		Recheck DFS radar of chrp type.
	Arguments:
	    pAdapter                    Pointer to our adapter
	    dfs_channel                DFS detect channel
    Return Value:
        "TRUE" if check pass, "FALSE" otherwise.
    Note:
    ==========================================================================
 */
static BOOLEAN ChirpRadarCheck(IN PRTMP_ADAPTER pAd)
{
	UINT32 CurrentTime, delta;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	

	RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &CurrentTime);
	delta = CurrentTime - pRadarDetect->TimeStamp;
	pRadarDetect->TimeStamp = CurrentTime;
	
	/* ChirpCheck = 0 means the very first detection since start up*/
	if (pRadarDetect->ChirpCheck++ == 0)
		return FALSE;
	
	if (delta <= (12*(1<<20)))  /* 12 sec */
	{
		if (pRadarDetect->ChirpCheck >= 2)
		{
			/* Anouce the radar on any mutiple detection within 12 sec*/
			DBGPRINT(RT_DEBUG_TRACE, ("ChirpRadarCheck OK.\n"));
			return TRUE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ChirpRadarCheck failed, discard previous detection.\n"));
		pRadarDetect->ChirpCheck = 1;		
		return FALSE;
	}
	/* default */
	return FALSE;
}


static BOOLEAN DfsChannelCheck(
			IN PRTMP_ADAPTER pAd,
			IN UINT8 DfsChannel)
{
	pNewDFSTable pDFSTable;
	UINT8 i;
	UINT32 T, W, W2, B;
	BOOLEAN radarDeclared = 0;

	T = W = W2 = B = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("DFS HW check channel = 0x%x\n", DfsChannel));
	/*Select the DFS table based on radar country region*/
	if (pAd->CommonCfg.RDDurRegion == FCC)
		pDFSTable = &NewDFSTable1[0];
	else if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDFSTable = &NewDFSTable1[1];
	}
	else /* Japan*/
	{
		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFSTable = &NewDFSTable1[3];
		}
		else
		{
		pDFSTable = &NewDFSTable1[2];
		}
	}
	/*check which channe(0~3) is detecting radar signals*/
	for (i = 0; i < pAd->chipCap.DfsEngineNum; i++)
	{
		
		if (DfsChannel & (0x1 << i))
		{
			DfsHwDetectionGetPulseInfo(pAd, i, &T, &W, &W2, &B);

			if (DfsSwCheckOnHwDetection(pAd, pDFSTable, i, T, W) == FALSE)
				continue;
			
			DBGPRINT(RT_DEBUG_OFF, ("T = %u, W= %u detected by ch %d\n", T, W, i));
			
			/*set this variable to 1 for announcing that we find the radar signals.*/
			radarDeclared = 1;

			if ( ((i == 3) || (i == 2)) && (pDFSTable->entry[i].mode != 0) )
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Burst = %u(0x%x)\n", B, B));
				DBGPRINT(RT_DEBUG_TRACE, ("The second Width = %u(0x%x)\n", W2, W2));
			}
		}
	}
	return radarDeclared;
}


#ifdef RLT_BBP
static BOOLEAN MT7650DfsEventDataFetch(
		IN PRTMP_ADAPTER pAd,
		IN PRADAR_DETECT_STRUCT pRadarDetect,
		OUT PDFS_EVENT pDfsEvent)
{
	UINT8 idx;
	UINT32 EventBuff[DFS_EVENT_SIZE] = {0};
	UINT32 bbp_val;

	/* Read a event from event buffer */
	for (idx = 0; idx < DFS_EVENT_SIZE; idx++)
	{
		/* 
			1st read - 	DFS_R37[31] = 0(engine0)/1(engine2), if no events ==> DFS_R37[31:0] = 0xFFFF_FFFF
			2nd read -	DFS_R37[21:0] = pulse time
			3rd read -  DFS_R37[25:16] = phase, DFS_R37[11:0] =  pulse width
			4th read -  DFS_R37[21:16] = power stable counter
						DFS_R37[12:0] = current power
		*/
		RTMP_BBP_IO_READ32(pAd, DFS_R37, &bbp_val);
		EventBuff[idx] = bbp_val;
	}

	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_SHOW_RAW_EVENT)
	{
		MT7650_DFS_EVENT_BUFF_PRINT(0, EventBuff, DFS_EVENT_SIZE);
	}

	/* No events */
	if (EventBuff[0] == 0xFFFFFFFF)
		return FALSE;

	/*
		Total 64 events: 32 events for engine0 and 32 event2 for engine2.
	*/
	pDfsEvent->EngineId = (EventBuff[0] & 0x80000000) ? 2:0;
	pDfsEvent->TimeStamp = EventBuff[1] & 0x003FFFFF;
	pDfsEvent->Width = EventBuff[2] & 0x0FFF;
	pDfsEvent->phase = (EventBuff[2] >> 16) & 0x03FF;
	pDfsEvent->power_stable_counter = (EventBuff[3] >> 16) & 0x003F;
	pDfsEvent->current_power = EventBuff[3] & 0x1FFF;

	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_EVENT)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("EngineId = %d\n", pDfsEvent->EngineId));
		DBGPRINT(RT_DEBUG_TRACE, ("TimeStamp = 0x%x\n", pDfsEvent->TimeStamp));
		DBGPRINT(RT_DEBUG_TRACE, ("Width = 0x%x\n", pDfsEvent->Width));
		DBGPRINT(RT_DEBUG_TRACE, ("phase = 0x%x\n", pDfsEvent->phase));
		DBGPRINT(RT_DEBUG_TRACE, ("power_stable_counter = 0x%x\n", pDfsEvent->power_stable_counter));
		DBGPRINT(RT_DEBUG_TRACE, ("current_power = 0x%x\n\n", pDfsEvent->current_power));
	}
	
	/* Check if event is valid */
	if (!DFS_EVENT_SANITY_CHECK(pAd, *pDfsEvent))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: This event is invalid\n", __FUNCTION__));
		return FALSE;
	}

	return TRUE;
}
#else
static BOOLEAN DfsEventDataFetch(
		IN PRTMP_ADAPTER pAd,
		IN PRADAR_DETECT_STRUCT pRadarDetect,
		OUT PDFS_EVENT pDfsEvent)
{
	UINT8 idx;
	UINT8 EventBuff[DFS_EVENT_SIZE] = {0};
	UINT32 bbp_val;

	/* Read a event from event buffer */
	for (idx = 0; idx < DFS_EVENT_SIZE; idx++)
	{
		RTMP_BBP_IO_READ32(pAd, BBP_R127, &bbp_val);
		EventBuff[idx] = bbp_val;
	}

	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_SHOW_RAW_EVENT)
	{
		DFS_EVENT_BUFF_PRINT(0, EventBuff, DFS_EVENT_SIZE);
	}
		
	pDfsEvent->EngineId = EventBuff[0];
	if (pDfsEvent->EngineId == 0xff) /* end of event */
		return FALSE;
	
	pDfsEvent->TimeStamp = EventBuff[1];
	pDfsEvent->TimeStamp |= (EventBuff[2] << 8);
	pDfsEvent->TimeStamp |= (EventBuff[3] << 16);

	pDfsEvent->Width = EventBuff[4];
	pDfsEvent->Width |= (EventBuff[5] << 8);
	
	/* Check if event is valid */
	if (!DFS_EVENT_SANITY_CHECK(pAd, *pDfsEvent))
		 return FALSE;

	return TRUE;
}
#endif /* MT76x0 */

VOID NewRadarDetectionProgram(PRTMP_ADAPTER pAd, pNewDFSTable pDFSTable)
{
	UINT8 idx, TalbeIdx;
	UINT8 DfsEngineNum = pAd->chipCap.DfsEngineNum;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;

	pRadarDetect->bDfsInit = FALSE;

	/* Get Table index*/
	for (TalbeIdx = 0; !((1<<TalbeIdx) & pDFSTable->type); TalbeIdx++)
	{
		if (TalbeIdx > MAX_RD_REGION)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Table index out of range.\n"));
			return;
		}
	}

	for(idx = 0; idx<DfsEngineNum; idx++)
	{
		if ((pRadarDetect->DFSParamFromConfig & (0x1<<idx)) && pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].valid)
		{
			pDFSTable->entry[idx].mode = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].mode;
			pDFSTable->entry[idx].avgLen = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].avgLen;
			pDFSTable->entry[idx].ELow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].ELow;
			pDFSTable->entry[idx].EHigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EHigh;
			pDFSTable->entry[idx].WLow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].WLow;
			pDFSTable->entry[idx].WHigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].WHigh;
			pDFSTable->entry[idx].EpsilonW = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EpsilonW;
			pDFSTable->entry[idx].TLow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].TLow;
			pDFSTable->entry[idx].THigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].THigh;
			pDFSTable->entry[idx].EpsilonT = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EpsilonT;
			pDFSTable->entry[idx].BLow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].BLow;
			pDFSTable->entry[idx].BHigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].BHigh;
			pDFSTable->entry[idx].EventExpiration= pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EventExpiration;
			pDFSTable->entry[idx].PwrJump= pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].PwrJump;

			DBGPRINT(RT_DEBUG_TRACE, ("TalbeIdx = %d; idx = %d; DFSParam = %2d; %3d; %3d; %3d; %3d; %4d; %3d; %6lu; %7lu; %4d; %2lu; %2lu, %d, 0x%04x\n", TalbeIdx, idx,
					pDFSTable->entry[idx].mode, pDFSTable->entry[idx].avgLen, pDFSTable->entry[idx].ELow, 
					pDFSTable->entry[idx].EHigh, pDFSTable->entry[idx].WLow, pDFSTable->entry[idx].WHigh,
					pDFSTable->entry[idx].EpsilonW, pDFSTable->entry[idx].TLow, pDFSTable->entry[idx].THigh,
					pDFSTable->entry[idx].EpsilonT, pDFSTable->entry[idx].BLow, pDFSTable->entry[idx].BHigh,
					pDFSTable->entry[idx].EventExpiration, pDFSTable->entry[idx].PwrJump));
		}
	}
	
	/* Symmetric round*/
    if(pRadarDetect->SymRoundFromCfg != 0)
    {
        pDfsProgramParam->Symmetric_Round = pRadarDetect->SymRoundFromCfg;
        DBGPRINT(RT_DEBUG_TRACE, ("Symmetric_Round = %d\n", pDfsProgramParam->Symmetric_Round));
    }

    /* BusyIdleRatio*/
    if(pRadarDetect->BusyIdleFromCfg != 0)
    {
        pRadarDetect->ch_busy_idle_ratio = pRadarDetect->BusyIdleFromCfg;
        DBGPRINT(RT_DEBUG_TRACE, ("ch_busy_idle_ratio = %d\n", pRadarDetect->ch_busy_idle_ratio));
    }
    /* DfsRssiHigh*/
    if(pRadarDetect->DfsRssiHighFromCfg != 0)
    {
        pRadarDetect->DfsRssiHigh = pRadarDetect->DfsRssiHighFromCfg;
        DBGPRINT(RT_DEBUG_TRACE, ("DfsRssiHigh = %d\n", pRadarDetect->DfsRssiHigh));
    }
    /* DfsRssiLow*/
    if(pRadarDetect->DfsRssiLowFromCfg != 0)
    {
        pRadarDetect->DfsRssiLow = pRadarDetect->DfsRssiLowFromCfg;
        DBGPRINT(RT_DEBUG_TRACE, ("DfsRssiLow = %d\n", pRadarDetect->DfsRssiLow));
    }
	
	/*pRadarDetect->MCURadarRegion = pAd->CommonCfg.RDDurRegion;*/
	pRadarDetect->MCURadarRegion = pDFSTable->type;
	
	DfsProgramBbpValues(pAd, pDFSTable);

	pRadarDetect->bDfsInit = TRUE;

}

BOOLEAN DfsSwCheckOnHwDetection(
	 IN PRTMP_ADAPTER pAd,
	 IN pNewDFSTable pDFSTable,
	 IN UINT8 DfsChannel,
	 IN ULONG RadarPeriod,
	 IN ULONG RadarWidth)
{
	BOOLEAN bRadarCheck = TRUE;
	if (!RadarPeriod || !RadarWidth)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Block eception on zero RadarPeriod or RadarWidth\n"));
		return FALSE;
	}

	if (pDFSTable->type == NEW_DFS_JAP)
	{
		/* Double check on pusle Width and Period*/
		if (DfsChannel < 3)
		{
			/*check short pulse*/
			if (RadarWidth < 120) /* pulse width less than 6.4us is report is 6.4us */
			{
				/* block the illegal period */
				if ((RadarPeriod < 2900) ||
					(RadarPeriod > 4700 && RadarPeriod < 6400) ||
					(RadarPeriod > 6800 && RadarPeriod < 27560)||
					(RadarPeriod > 27960 && RadarPeriod < 28360) ||
					(RadarPeriod > 28700 && RadarPeriod < 79900) ||
					(RadarPeriod > 80100))
				{ 
					 /*(0~145), (235~320us), (340~1378us), (1398~1418), (1435~3995us) and (4005us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
			else if (RadarWidth < 130) /* 120~130 over lap range */
			{
				/* block the illegal period */
				if ((RadarPeriod < 2900) ||
					(RadarPeriod > 10100 && RadarPeriod < 27560) ||
					(RadarPeriod > 27960 && RadarPeriod < 28360) ||
					(RadarPeriod > 28700 && RadarPeriod < 79900) ||
					(RadarPeriod > 80100))
				{ 
					 /*(0~145), (505~1378us), (1398~1418), (1435~3995us) and (4005us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
			else if (RadarWidth >= 130)// pulse width over 6.4us is accuracy
			{
				if ((RadarPeriod<3900) || (RadarPeriod>10100))
				{ 
					 /* block the illegal period */
					 /*(0~195) and (505us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
		}
		else if (DfsChannel == 3)
		{
			bRadarCheck = ChirpRadarCheck(pAd);
		}
	}
	else if (pDFSTable->type == NEW_DFS_EU)
	{
		/* Double check on pusle Width and Period*/
		if (DfsChannel < 3)
		{
	
			/* block the illegal period */
			if ((RadarPeriod < 4900) ||
        		(RadarPeriod > 10200 && RadarPeriod < 12400) ||
		        (RadarPeriod > 100100))
			{ 
				/*(0~245), (510~620us), (5005us~) according to the spec*/
				DBGPRINT(RT_DEBUG_TRACE,
					("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
				bRadarCheck = FALSE;
			}
		}
		else if (DfsChannel == 4) /* to do: check dfs mode 8or9*/
		{
			if (StagerRadarCheck(pAd, DfsChannel) == FALSE)
				bRadarCheck = FALSE;
		}
	}
	else if (pDFSTable->type == NEW_DFS_FCC)
	{
		/* Double check on pusle Width and Period*/
		if (DfsChannel < 3)
		{
			/*check short pulse*/
			if (RadarWidth < 120) /* pulse width less than 6.4us is report is 6.4us */
			{
		        /* block the illegal period */
				 if ((RadarPeriod < 2900) ||
					(RadarPeriod > 4700 && RadarPeriod < 6400) ||
					(RadarPeriod > 6800 && RadarPeriod < 10200)||					
					(RadarPeriod > 61600))
				{ 
					 /*(0~145), (235~320us), (340~510), (3080us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
			else if (RadarWidth < 130) //120~130 over lap range
			{
				if ((RadarPeriod < 2900) ||
					(RadarPeriod > 61600))
				{ 
					 /* block the illegal period */
					 /*(0~145) and (3080us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
			else if (RadarWidth >= 130)
			{
				if ((RadarPeriod<3900) || (RadarPeriod>10100))
        		{ 
                	/* block the illegal period */
					 /*(0~195) and (505us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
                                ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
                	bRadarCheck = FALSE;
        		}                               
			}
		}               
		else if (DfsChannel == 3)
		{
        	bRadarCheck = ChirpRadarCheck(pAd);
		}
	}
	else if (pDFSTable->type == NEW_DFS_JAP_W53)
	{
			if (RadarWidth > 130) // pulse width less than 6.4us is report is 6.4us
			{
					 /*block W53 very large and short width*/
						DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 	bRadarCheck = FALSE;
			}
			else{
					if ((RadarPeriod < 28360) ||
					(RadarPeriod > 28700 && RadarPeriod < 76900) ||
					(RadarPeriod > 76940))
					{ 
					 /*(0~1418), (1435~3845us) and (3847us~) according to the spec*/
						DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 	bRadarCheck = FALSE;
					}
			}
	}

	return bRadarCheck;
}

static VOID ChannelSelectOnRadarDetection(
		IN PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;	
	UINT i;

	if (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE)
		return;

#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
	{
		for (i=0; i<pAd->ChannelListNum; i++)
		{
			if ((pAd->CommonCfg.Channel >= 52 && pAd->CommonCfg.Channel <= 64) &&
				(pAd->ChannelList[i].Channel >= 52 && pAd->ChannelList[i].Channel <= 64))
			{
				pAd->ChannelList[i].RemainingTimeForUse = 1800;
			}
			else if ((pAd->CommonCfg.Channel >= 100 && pAd->CommonCfg.Channel <= 112) &&
				(pAd->ChannelList[i].Channel >= 100 && pAd->ChannelList[i].Channel <= 112))
			{
				pAd->ChannelList[i].RemainingTimeForUse = 1800;
			}
			else if ((pAd->CommonCfg.Channel >= 116 && pAd->CommonCfg.Channel <= 128) &&
				(pAd->ChannelList[i].Channel >= 116 && pAd->ChannelList[i].Channel <= 128))
			{
				pAd->ChannelList[i].RemainingTimeForUse = 1800;
			}
		}
	}
#endif /* DOT11_VHT_AC */

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
		{
			if(pAd->ChannelList[i].RemainingTimeForUse != 0)
				DBGPRINT(RT_DEBUG_TRACE, ("From block channel = %u Jump!!!\n", pAd->ChannelList[i].Channel));
			else
				DBGPRINT(RT_DEBUG_TRACE, ("From unblock channel = %u Jump!!!\n", pAd->ChannelList[i].Channel));
		
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
			{
				if ((pAd->ChannelList[i].Channel >> 2) & 1)
				{
					if ((pAd->ChannelList[i+1].Channel - pAd->ChannelList[i].Channel) == 4 )
					{
						DBGPRINT(RT_DEBUG_TRACE, ("Find extend channel = %u\n", pAd->ChannelList[i+1].Channel));
						pAd->ChannelList[i+1].RemainingTimeForUse = 1800;
					}
				}
				else 
				{
					if ((pAd->ChannelList[i].Channel - pAd->ChannelList[i-1].Channel) == 4 )
					{
						DBGPRINT(RT_DEBUG_TRACE, ("Find extend channel = %u\n", pAd->ChannelList[i-1].Channel));
						pAd->ChannelList[i-1].RemainingTimeForUse = 1800;
					}
				}
			}
			else
				DBGPRINT(RT_DEBUG_TRACE, ("BW is not 40.\n"));
			
			pAd->ChannelList[i].RemainingTimeForUse = 1800;/*30 min = 1800 sec*/
			break;
		}
	}

	/*when find an radar, the ChMovingTime will be set to announce how many seconds to sending software radar detection time.*/
	if ((pAd->CommonCfg.RDDurRegion == CE) && RESTRICTION_BAND_1(pAd))
		pAd->Dot11_H.ChMovingTime = 605;
	else
		pAd->Dot11_H.ChMovingTime = 65;

	/*if the Radar country region is JAP, we need find a new clear channel */
	if (pAd->CommonCfg.RDDurRegion == JAP_W56)
	{
		for (i = 0; i < pAd->ChannelListNum ; i++)
		{
			pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
			if ((pAd->CommonCfg.Channel >= 100) && (pAd->CommonCfg.Channel <= 140))
				break;
		}
	}
	else if (pAd->CommonCfg.RDDurRegion == JAP_W53)
	{
		for (i = 0; i < pAd->ChannelListNum ; i++)
		{
			pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
			if ((pAd->CommonCfg.Channel >= 36) && (pAd->CommonCfg.Channel <= 60))
				break;
		}
	}
	else
		pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
		
#ifdef DOT11_N_SUPPORT
	N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
		{
			if(pAd->ChannelList[i].RemainingTimeForUse != 0)
				DBGPRINT(RT_DEBUG_TRACE, ("Channel Switch to block channel = %u \n", pAd->ChannelList[i].Channel));
			else
				DBGPRINT(RT_DEBUG_TRACE, ("Channel Switch to unblock channel = %u \n", pAd->ChannelList[i].Channel));
			break;
		}
	}

	/*ApSelectChannelCheck(pAd);*/
	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
	{
		pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
		/* Prepare a count-down for channel switching */
		pAd->Dot11_H.CSCount = 0;
	}
	else if (pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
	{
		pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
		/*set this flag to 1 and the AP will restart to switch into new channel */
		pRadarDetect->DFSAPRestart = 1;
		schedule_dfs_task(pAd);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Error! Unexpected radar state.\n", __FUNCTION__));
	}
		pRadarDetect->radarDeclared = 0;
}

static BOOLEAN DfsEventDrop(
		IN PRTMP_ADAPTER pAd,
		IN PDFS_EVENT pDfsEvent)
{
	UINT32 TimeDiff = 0;  /* unit: 50ns */
	UINT16 PreEnvtWidth = 0;
	BOOLEAN RetVal = FALSE;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	PDFS_EVENT pPreDfsEvent = &pDfsSwParam->PreDfsEvent;

	if (pDfsEvent->EngineId != pPreDfsEvent->EngineId)
	{
		/* update prevoius event record then leave */
		NdisCopyMemory(pPreDfsEvent, pDfsEvent, DFS_EVENT_SIZE);
		return FALSE;
	}

	if (pDfsEvent->EngineId == 0x01 || pDfsEvent->EngineId == 0x02)
	{
		if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
		{
			TimeDiff = ((pDfsEvent->TimeStamp - pPreDfsEvent->TimeStamp) >> 1);  /* 25ns to 50ns*/
			PreEnvtWidth = pPreDfsEvent->Width >> 1;
		}
		else
		{
			TimeDiff = (pDfsEvent->TimeStamp - pPreDfsEvent->TimeStamp);
			PreEnvtWidth = pPreDfsEvent->Width;
		}
		
		if (TimeDiff < pDfsSwParam->EvtDropAdjTime &&
			PreEnvtWidth >= 200)
		{
			DBGPRINT(RT_DEBUG_TRACE, 
					("%s(): EngineId = %x,  Width = %u, TimeStamp = %u\n",
					__FUNCTION__,
					pDfsEvent->EngineId,
					pDfsEvent->Width,
					pDfsEvent->TimeStamp));
			RetVal = TRUE;
		}
	}

	/* update prevoius event record */
	NdisCopyMemory(pPreDfsEvent, pDfsEvent, DFS_EVENT_SIZE);
	
	return RetVal;
}

static void dfs_sw_init(PRTMP_ADAPTER pAd)
{
	int j, k;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	pDfsSwParam->dfs_check_loop = DFS_SW_RADAR_CHECK_LOOP;
	pDfsSwParam->dfs_width_diff_ch1_Shift = DFS_SW_RADAR_CH1_SHIFT;	
	pDfsSwParam->dfs_width_diff_ch2_Shift = DFS_SW_RADAR_CH2_SHIFT;	
	pDfsSwParam->PreDfsEvent.EngineId = 0xff;
	pDfsSwParam->EvtDropAdjTime = 2000;

	pDfsSwParam->dfs_width_ch0_err_L = DFS_SW_RADAR_CH0_ERR;
	if (pAd->CommonCfg.RDDurRegion == CE) {
		pDfsSwParam->dfs_period_err = (DFS_SW_RADAR_PERIOD_ERR << 2);
		pDfsSwParam->dfs_width_ch0_err_H = CE_STAGGERED_RADAR_CH0_H_ERR;
		pDfsSwParam->dfs_declare_thres = CE_STAGGERED_RADAR_DECLARE_THRES;
		pDfsSwParam->dfs_max_period = CE_STAGGERED_RADAR_PERIOD_MAX;
	}
	else {		
		pDfsSwParam->dfs_period_err = DFS_SW_RADAR_PERIOD_ERR;
		if (pAd->CommonCfg.RDDurRegion == FCC) {
			pDfsSwParam->dfs_max_period = FCC_RADAR_PERIOD_MAX;
		}
		else if (pAd->CommonCfg.RDDurRegion == JAP) {
			pDfsSwParam->dfs_max_period = JAP_RADAR_PERIOD_MAX;
	}
	}
	
	pDfsSwParam->dfs_check_loop = DFS_SW_RADAR_CHECK_LOOP;
	pDfsSwParam->dfs_width_diff_ch1_Shift = DFS_SW_RADAR_CH1_SHIFT;
	pDfsSwParam->dfs_width_diff_ch2_Shift = DFS_SW_RADAR_CH2_SHIFT;
	pDfsSwParam->dfs_width_ch0_err_L = DFS_SW_RADAR_CH0_ERR;
	if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDfsSwParam->dfs_period_err = (DFS_SW_RADAR_PERIOD_ERR << 2);
		pDfsSwParam->dfs_width_ch0_err_H = CE_STAGGERED_RADAR_CH0_H_ERR;
		pDfsSwParam->dfs_declare_thres = CE_STAGGERED_RADAR_DECLARE_THRES;
		pDfsSwParam->dfs_max_period = CE_STAGGERED_RADAR_PERIOD_MAX;
	}
	else
	{
		pDfsSwParam->dfs_period_err = DFS_SW_RADAR_PERIOD_ERR;
		if (pAd->CommonCfg.RDDurRegion == FCC)
			pDfsSwParam->dfs_max_period = FCC_RADAR_PERIOD_MAX;
		else if (pAd->CommonCfg.RDDurRegion == JAP)
			pDfsSwParam->dfs_max_period = JAP_RADAR_PERIOD_MAX;
	}

	if (pRadarDetect->use_tasklet)
		pRadarDetect->PollTime = NEW_DFS_CHECK_TIME_TASKLET;
	else
		pRadarDetect->PollTime = NEW_DFS_CHECK_TIME;

	for (k = 0; k < pAd->chipCap.DfsEngineNum; k++)
	{
		for (j = 0; j < NEW_DFS_DBG_PORT_ENT_NUM; j++)
		{
			pDfsSwParam->DFS_W[k][j].start_idx = 0xffff;
		}
	}

	for (k = 0; k < pAd->chipCap.DfsEngineNum; k++)
	{
		pDfsSwParam->sw_idx[k] = NEW_DFS_DBG_PORT_ENT_NUM - 1;
		pDfsSwParam->hw_idx[k] = 0;
	}
		
	NdisZeroMemory(pDfsSwParam->DFS_T, sizeof(pDfsSwParam->DFS_T));
	NdisZeroMemory(pDfsSwParam->DFS_W, sizeof(pDfsSwParam->DFS_W));
}

void modify_table1(PRTMP_ADAPTER pAd, ULONG idx, ULONG value)
{
	pNewDFSTable pDFSTable;
	ULONG x, y;	
	UINT8 DfsEngineNum = pAd->chipCap.DfsEngineNum;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;

	if (pAd->CommonCfg.RDDurRegion == FCC)
		pDFSTable = &NewDFSTable1[0];
	else if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDFSTable = &NewDFSTable1[1];
	}
	else /* Japan*/
	{
		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFSTable = &NewDFSTable1[3];
		}
		else
		{
		pDFSTable = &NewDFSTable1[2];
		}
	}

#ifdef RLT_BBP
	if (IS_MT7610E(pAd) || IS_MT76x2(pAd))
	{
		if (pAd->CommonCfg.BBPCurrentBW == BW_80) {
			if (pAd->CommonCfg.RDDurRegion == CE) {
				pDFSTable = &NewDFSTable2_BW80[1];
			}
			else if (pAd->CommonCfg.RDDurRegion == FCC) {
				pDFSTable = &NewDFSTable2_BW80[0];
			}
			else if ((pAd->CommonCfg.Channel >= 52) 
					&& (pAd->CommonCfg.Channel <= 64)) { 	
				/* JAP_W53 */
				pDFSTable = &NewDFSTable2_BW80[2];
			}
			else {
				/* JAP_W56 */
				pDFSTable = &NewDFSTable2_BW80[3];
			}
		}
		else if (pAd->CommonCfg.BBPCurrentBW == BW_40) {
			// New update: 20130606
			if (pAd->CommonCfg.RDDurRegion == CE) {
				pDFSTable = &NewDFSTable2_BW40[1];
			}
			else if (pAd->CommonCfg.RDDurRegion == FCC) {
				pDFSTable = &NewDFSTable2_BW40[0];
			}
			else if ((pAd->CommonCfg.Channel >= 52) 
					&& (pAd->CommonCfg.Channel <= 64)) {
				/* JAP_W53 */
				pDFSTable = &NewDFSTable2_BW40[2];
		}
		else {
				/* JAP_W56 */
				pDFSTable = &NewDFSTable2_BW40[3];
			}
		}
		else if (pAd->CommonCfg.BBPCurrentBW == BW_20){
			if (pAd->CommonCfg.RDDurRegion == CE) {
				pDFSTable = &NewDFSTable2_BW20[1];
			}
			else if (pAd->CommonCfg.RDDurRegion == FCC) {
				pDFSTable = &NewDFSTable2_BW20[0];
			}
			else if ((pAd->CommonCfg.Channel >= 52) 
					&& (pAd->CommonCfg.Channel <= 64)) { 	
				/* JAP_W53 */
				pDFSTable = &NewDFSTable2_BW20[2];
			}
			else {
				/* JAP_W56 */
				pDFSTable = &NewDFSTable2_BW20[3];
			}
		}
		else
		{}
	}
#endif /* MT76x0 */

		if (idx == 0)
		{
			pDfsProgramParam->DeltaDelay = value;
		}
		else if (idx <= (DfsEngineNum*16))
		{	
			x = idx / 16;
			y = idx % 16;
			pRadarDetect->DFSParamFromConfig = 0; /* to prevent table be loaded from config file again */
			switch (y)
			{
				case 1:
					pDFSTable->entry[x].mode = (UCHAR)value;
					break;
				case 2:
					pDFSTable->entry[x].avgLen = (USHORT)value;
					break;
		case 3:
			pDFSTable->entry[x].ELow = (USHORT)value;
			break;
    	
		case 4:
			pDFSTable->entry[x].EHigh = (USHORT)value;
			break;
    	
		case 5:
			pDFSTable->entry[x].WLow = (USHORT)value;
			break;
    	
		case 6:
			pDFSTable->entry[x].WHigh = (USHORT)value;
			break;
    	
		case 7:
			pDFSTable->entry[x].EpsilonW = (UCHAR)value;
			break;
    	
		case 8:
			pDFSTable->entry[x].TLow = (ULONG)value;
			break;
    	
		case 9:
			pDFSTable->entry[x].THigh = (ULONG)value;
			break;
    	
		case 0xa:
			pDFSTable->entry[x].EpsilonT = (UCHAR)value;
			break;

		case 0xb:
			pDFSTable->entry[x].BLow= (ULONG)value;
			break;
			
		case 0xc:
			pDFSTable->entry[x].BHigh = (ULONG)value;
			break;

		case 0xd:
			pDFSTable->entry[x].EventExpiration= (UINT32)value;
			break;
			
		case 0xe:
			pDFSTable->entry[x].PwrJump= (UINT16)value;
			break;
			
		default:
			break;
		}
    	
	}
	else if (idx == (DfsEngineNum*16 +1))
	{
		pDfsProgramParam->Symmetric_Round = (UCHAR)value;
	}
	else if (idx == (DfsEngineNum*16 +2))
	{
		pDfsProgramParam->VGA_Mask = (UCHAR)value;
	}
	else if (idx == (DfsEngineNum*16 +3))
	{
		pDfsProgramParam->Packet_End_Mask = (UCHAR)value;
	}
	else if (idx == (DfsEngineNum*16 +4))
	{
		pDfsProgramParam->Rx_PE_Mask = (UCHAR)value;
	}

	printk("Delta_Delay(0) = %d\n", pDfsProgramParam->DeltaDelay);

	for (x = 0; x < DfsEngineNum; x++)
	{
		printk("Channel %lu\n", x);
		printk("\t\tmode(%02lu)=%d, M(%02lu)=%03d, EL(%02lu)=%03d EH(%02lu)=%03d, WL(%02lu)=%03d WH(%02lu)=%04d, eW(%02lu)=%02d\n"
			    "\t\tTL(%02lu)=%05u TH(%02lu)=%06u, eT(%02lu)=%03d, BL(%02lu)=%u, BH(%02lu)=%u\n"
			    "\t\tEE(%02lu) = %06u, PJ(%02lu) = 0x%02x\n",
				(x*16+0x1), (unsigned int)pDFSTable->entry[x].mode,
				(x*16+0x2), (unsigned int)pDFSTable->entry[x].avgLen,
				(x*16+0x3), (unsigned int)pDFSTable->entry[x].ELow,
				(x*16+0x4), (unsigned int)pDFSTable->entry[x].EHigh,
				(x*16+0x5), (unsigned int)pDFSTable->entry[x].WLow,
				(x*16+0x6), (unsigned int)pDFSTable->entry[x].WHigh,
				(x*16+0x7), (unsigned int)pDFSTable->entry[x].EpsilonW,
				(x*16+0x8), (unsigned int)pDFSTable->entry[x].TLow,
				(x*16+0x9), (unsigned int)pDFSTable->entry[x].THigh,
				(x*16+0xa), (unsigned int)pDFSTable->entry[x].EpsilonT,
				(x*16+0xb), (unsigned int)pDFSTable->entry[x].BLow,
				(x*16+0xc), (unsigned int)pDFSTable->entry[x].BHigh,
				(x*16+0xd), (unsigned int)pDFSTable->entry[x].EventExpiration,
				(x*16+0xe), (unsigned int)pDFSTable->entry[x].PwrJump	);
	}

	printk("Symmetric_Round(%02d) = %d\n", (DfsEngineNum*16 +1), pDfsProgramParam->Symmetric_Round);
	printk("VGA_Mask(%02d) = %d\n", (DfsEngineNum*16 +2), pDfsProgramParam->VGA_Mask);
	printk("Packet_End_Mask(%02d) = %d\n", (DfsEngineNum*16 +3), pDfsProgramParam->Packet_End_Mask);
	printk("Rx_PE_Mask(%02d) = %d\n", (DfsEngineNum*16 +4), pDfsProgramParam->Rx_PE_Mask);

}


void modify_table2(PRTMP_ADAPTER pAd, ULONG idx, ULONG value)
{
	pNewDFSValidRadar pDFSValidRadar;
	ULONG x, y;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	
	idx--;

	x = idx / 17;
	y = idx % 17;
	
	pDFSValidRadar = &NewDFSValidTable[0];
	
	while (pDFSValidRadar->type != NEW_DFS_END)
	{
		if (pDFSValidRadar->type & pRadarDetect->MCURadarRegion)
		{
			if (x == 0)
				break;
			else
			{
				x--;
				pDFSValidRadar++;
			}
		}
		else
			pDFSValidRadar++;
	}
	
	if (pDFSValidRadar->type == NEW_DFS_END)
	{
		printk("idx=%d exceed max number\n", (unsigned int)idx);
		return;
	}
	switch(y)
	{
	case 0:
		pDFSValidRadar->channel = value;
		break;
	case 1:
		pDFSValidRadar->WLow = value;
		break;
	case 2:
		pDFSValidRadar->WHigh = value;
		break;
	case 3:
		pDFSValidRadar->W = value;
		break;
	case 8:
		pDFSValidRadar->WMargin = value;
		break;
	case 9:
		pDFSValidRadar->TLow = value;
		break;
	case 10:
		pDFSValidRadar->THigh = value;
		break;
	case 11:
		pDFSValidRadar->T = value;
		break;
	case 16:
		pDFSValidRadar->TMargin = value;
		break;
	}
	
	pDFSValidRadar = &NewDFSValidTable[0];
	while (pDFSValidRadar->type != NEW_DFS_END)
	{
		if (pDFSValidRadar->type & pRadarDetect->MCURadarRegion)
		{
			printk("ch = %x  --- ", pDFSValidRadar->channel);
			printk("wl:wh = %d:%d  ", pDFSValidRadar->WLow, pDFSValidRadar->WHigh);
			printk("W = %u  --- ", pDFSValidRadar->W);
			printk("W Margin = %d\n", pDFSValidRadar->WMargin);
			printk("Tl:Th = %d:%d  ", (unsigned int)pDFSValidRadar->TLow, (unsigned int)pDFSValidRadar->THigh);
			printk("T = %lu  --- ", pDFSValidRadar->T);
			printk("T Margin = %d\n", pDFSValidRadar->TMargin);
		}
		pDFSValidRadar++;
	}

}

#ifdef RTMP_MAC_PCI
VOID NewTimerCB_Radar(
 	IN PRTMP_ADAPTER pAd)
{
	UCHAR channel=0;
	UCHAR radarDeclared = 0;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	if (!DFS_CHECK_FLAGS(pAd, pRadarDetect) ||
		(pRadarDetect->PollTime == 0))
		return;
	
	/* to prevent possible re-entries*/
	pRadarDetect->bDfsInit = FALSE;

	pRadarDetect->RadarTimeStampLow++;


	/*if ((pRadarDetect->McuRadarTick++ >= pRadarDetect->PollTime) &&*/
	if ((pRadarDetect->McuRadarTick++ >= 3) && 	/* 30ms */
		(!pRadarDetect->bDfsSwDisable))
	{
		SwCheckDfsEvent(pAd);
	}

	/*The following codes is used to check if the hardware find the Radar Signal
	*  Read the 0~3 channel which had detected radar signals
	*/
	DfsHwDetectionStatusGet(pAd, &channel);
	
		/*Check if any interrupt trigger by Radar Global Status(Radar Signals)*/
	if ((channel & pRadarDetect->EnabledChMask) 
			//&& (!pRadarDetect->ch_busy)
	   ) {
		radarDeclared = DfsChannelCheck(pAd, channel);
#ifdef DFS_ATP_SUPPORT
		if ((radarDeclared == TRUE) && 
				(pRadarDetect->atp_radar_detect == FALSE)) 
		{
			pRadarDetect->atp_radar_detect = TRUE;
		}
#endif /* DFS_ATP_SUPPORT */
	}

	/*reset the radar channel for new counting (Write clear) */
	if (channel & pRadarDetect->EnabledChMask)
		DfsResetHwDetectionStatus(pAd);
	
	if (pRadarDetect->McuRadarDebug & RADAR_SIMULATE)
	{
		radarDeclared = 1;
		pRadarDetect->McuRadarDebug &= ~RADAR_SIMULATE;
	}

	if ((radarDeclared || pRadarDetect->radarDeclared) 
			&& (pRadarDetect->ch_busy_countdown == -1)
	   ) {
		pRadarDetect->ch_busy_countdown = 5;
	}
	else if (pRadarDetect->ch_busy_countdown >= 0) {
		pRadarDetect->ch_busy_countdown--;
	}

	/*Now, find an Radar signal*/
	//if ((!pRadarDetect->ch_busy) && (pRadarDetect->ch_busy_countdown == 0))
	if ((pRadarDetect->ch_busy_countdown == 0))
	{	
		/* Radar found!!!*/
		pRadarDetect->ch_busy_countdown = -1;
		
		/*Announce that this channel could not use in 30 minutes if we need find a clear channel*/
		if (!(pRadarDetect->McuRadarDebug & RADAR_DONT_SWITCH))
			ChannelSelectOnRadarDetection(pAd);
		else
			pRadarDetect->radarDeclared = 0;			
	}
	pRadarDetect->bDfsInit = TRUE;
}

#ifdef RLT_BBP
#define DFS_EVENT_DATA_FETCH(__pAd, __pRadarDetect, __DfsEvent) \
	MT7650DfsEventDataFetch(__pAd, __pRadarDetect, &__DfsEvent)
#else
#define DFS_EVENT_DATA_FETCH(__pAd, __pRadarDetect, __DfsEvent) \
	DfsEventDataFetch(__pAd, __pRadarDetect, &__DfsEvent)
#endif /* MT76x0 */

static VOID SwCheckDfsEvent(
		IN PRTMP_ADAPTER pAd)
{
	INT k,  limit = 64;
	UCHAR id = 0;
	DFS_EVENT DfsEvent;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	pRadarDetect->McuRadarTick = 0;

	/* disable debug mode to read debug port */
	DfsCaptureModeControl(pAd, FALSE, TRUE, FALSE);

	pDfsSwParam->dfs_w_counter++;

	for (k = 0; k < limit; k++)
	{
		/* fetch event data */
		if (DFS_EVENT_DATA_FETCH(pAd, pRadarDetect, DfsEvent) == FALSE)
			break;

		if (DfsEventDrop(pAd, &DfsEvent) == TRUE)
			continue;

		if (pRadarDetect->use_tasklet)
		{
			id = DfsEvent.EngineId;
			
			if (id < pAd->chipCap.DfsEngineNum)
			{			
				pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].counter = pDfsSwParam->dfs_w_counter;
				pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].timestamp = DfsEvent.TimeStamp;
				pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].width = DfsEvent.Width;

				if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_EVENT)
				{
					printk("counter = %lu ", pDfsSwParam->dfs_w_counter);
					DFS_EVENT_PRINT(DfsEvent);
				}				

				pDfsSwParam->dfs_w_last_idx[id] = pDfsSwParam->dfs_w_idx[id];
				pDfsSwParam->dfs_w_idx[id]++;
				if (pDfsSwParam->dfs_w_idx[id] >= NEW_DFS_DBG_PORT_ENT_NUM)
					pDfsSwParam->dfs_w_idx[id] = 0;
			}
		}
	}

	if (pRadarDetect->use_tasklet)
	{
		/* set hw_idx*/
		pDfsSwParam->hw_idx[0] = pDfsSwParam->dfs_w_idx[0];
		pDfsSwParam->hw_idx[1] = pDfsSwParam->dfs_w_idx[1];
		pDfsSwParam->hw_idx[2] = pDfsSwParam->dfs_w_idx[2];
		pDfsSwParam->hw_idx[3] = pDfsSwParam->dfs_w_idx[3];
		/*dfs tasklet will call SWRadarCheck*/
		schedule_dfs_task(pAd);
	}

	/* enable debug mode*/
	DfsCaptureModeControl(pAd, TRUE, TRUE, FALSE);
}
#endif /* RTMP_MAC_PCI */

#endif /*CONFIG_AP_SUPPORT*/
#endif /* DFS_SUPPORT */

