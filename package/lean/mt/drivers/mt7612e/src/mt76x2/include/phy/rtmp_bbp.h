/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_bbp.h

	Abstract:
	Ralink Wireless Chip PHY(BBP) related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RTMP_BBP_H__
#define __RTMP_BBP_H__


/*
	BBP sections
*/
#define BBP_R0			0  /* version */
#define BBP_R1			1  /* TSSI */
#define BBP_R2			2  /* TX configure */
#define BBP_R3			3
#define BBP_R4			4
#define BBP_R5			5
#define BBP_R6			6
#define BBP_R10			10 /* Rate report */
#define BBP_R11			11
#define BBP_R12			12
#define BBP_R13			13
#define BBP_R14			14 /* RX configure */
#define BBP_R15			15
#define BBP_R16			16
#define BBP_R17			17 /* RX sensibility */
#define BBP_R18			18
#define BBP_R19			19
#define BBP_R20			20
#define BBP_R21			21
#define BBP_R22			22
#define BBP_R23			23
#define BBP_R24			24
#define BBP_R25			25
#define BBP_R26			26
#define BBP_R27			27
#define BBP_R28			28
#define BBP_R30			30
#define BBP_R31			31
#define BBP_R47			47
#define BBP_R49			49 /*TSSI */
#define BBP_R50			50
#define BBP_R51			51
#define BBP_R52			52
#define BBP_R53			53
#define BBP_R54			54
#define BBP_R55			55
#define BBP_R58			58
#define BBP_R60			60
#define BBP_R57			57
#define BBP_R61			61
#define BBP_R62			62 /* Rx SQ0 Threshold HIGH */
#define BBP_R63			63
#define BBP_R64			64
#define BBP_R65			65
#define BBP_R66			66
#define BBP_R67			67
#define BBP_R68			68
#define BBP_R69			69
#define BBP_R70			70 /* Rx AGC SQ CCK Xcorr threshold */
#define BBP_R73			73
#define BBP_R74			74
#define BBP_R75			75
#define BBP_R76			76
#define BBP_R77			77
#define BBP_R78			78
#define BBP_R79			79
#define BBP_R80			80
#define BBP_R81			81
#define BBP_R82			82
#define BBP_R83			83
#define BBP_R84			84
#define BBP_R86			86
#define BBP_R87			87
#define BBP_R88			88
#define BBP_R91			91
#define BBP_R92			92
#define BBP_R93			93
#define BBP_R94			94 /* Tx Gain Control */
#define BBP_R95			95
#define BBP_R96			96
#define BBP_R98			98
#define BBP_R99			99
#define BBP_R101		101
#define BBP_R103		103
#define BBP_R104		104
#define BBP_R105		105
#define BBP_R106		106
#define BBP_R107		107
#define BBP_R108		108
#define BBP_R109		109
#define BBP_R110		110
#define BBP_R113		113
#define BBP_R114		114
#define BBP_R115		115
#define BBP_R116		116
#define BBP_R117		117
#define BBP_R118		118
#define BBP_R119		119
#define BBP_R120		120
#define BBP_R121		121
#define BBP_R122		122
#define BBP_R123		123
#define BBP_R126		126
#define BBP_R127		127
#define BBP_R128		128
#define BBP_R129		129
#define BBP_R130		130
#define BBP_R131		131
#define BBP_R132		132
#define BBP_R133		133
#define BBP_R134		134
#define BBP_R135		135
#define BBP_R137		137
#define BBP_R138		138 /* add by johnli, RF power sequence setup, ADC dynamic on/off control */
#define BBP_R139		139
#define BBP_R140		140
#define BBP_R141		141
#define BBP_R142		142
#define BBP_R143		143
#define BBP_R144		144
#define BBP_R145		145
#define BBP_R146		146
#define BBP_R147		147
#define BBP_R148		148
#define BBP_R149		149
#define BBP_R150		150
#define BBP_R151		151
#define BBP_R152		152
#define BBP_R153		153
#define BBP_R154		154
#define BBP_R155		155
#define BBP_R156		156
#define BBP_R157		157
#define BBP_R158		158 /* Calibration register are accessed through R158 and R159 */
#define BBP_R159		159
#define BBP_R160		160 /* Tx BF control */
#define BBP_R161		161
#define BBP_R162		162
#define BBP_R163		163
#define BBP_R164		164
#define BBP_R165		165
#define BBP_R166		166
#define BBP_R167		167
#define BBP_R168		168
#define BBP_R169		169
#define BBP_R170		170
#define BBP_R171		171
#define BBP_R172		172
#define BBP_R173		173
#define BBP_R174		174
#define BBP_R175		175
#define BBP_R176		176
#define BBP_R177		177
#define BBP_R178		178
#define BBP_R179		179
#define BBP_R180		180
#define BBP_R181		181
#define BBP_R182		182
#define BBP_R183		183
#define BBP_R184		184
#define BBP_R185		185
#define BBP_R186		186
#define BBP_R187		187
#define BBP_R188		188
#define BBP_R189		189
#define BBP_R190		190
#define BBP_R191		191
#define BBP_R195		195
#define BBP_R196		196
#define BBP_R200		200
#define BBP_R201		201
#define BBP_R202		202
#define BBP_R203		203
#define BBP_R204		204
#define BBP_R205		205
#define BBP_R206		206
#define BBP_R207		207
#define BBP_R208		208
#define BBP_R209		209
#define BBP_R210		210
#define BBP_R211		211

#define BBP_R241		241
#define BBP_R242		242
#define BBP_R244		244

#define BBP_R250		250
#define BBP_R253		253
#define BBP_R254		254
#define BBP_R255		255

#define BBPR94_DEFAULT	0x06 /* Add 1 value will gain 1db */


#ifdef RT_BIG_ENDIAN
typedef union _BBP_R47_STRUC {
	struct
	{
		UCHAR	Adc6On:1;
		UCHAR	Reserved:2; 
		UCHAR	TssiMode:2;
		UCHAR	TssiUpdateReq:1;
		UCHAR	TssiReportSel:2;
	} field;

	UCHAR		byte;
} BBP_R47_STRUC, *PBBP_R47_STRUC;
#else
typedef union _BBP_R47_STRUC {
	struct
	{
		UCHAR	TssiReportSel:2;
		UCHAR	TssiUpdateReq:1;
		UCHAR	TssiMode:2;
		UCHAR	Reserved:2; 
		UCHAR	Adc6On:1;
	} field;
	
	UCHAR		byte;
} BBP_R47_STRUC, *PBBP_R47_STRUC;
#endif


/* */
/* BBP R105 (FEQ control, MLD control and SIG remodulation) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R105_STRUC {
	struct
	{
		UCHAR	Reserve1:4; /* Reserved field */
		UCHAR	EnableSIGRemodulation:1; /* Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols. */
		UCHAR	MLDFor2Stream:1; /* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX) */
		UCHAR	IndependentFeedForwardCompensation:1; /* Apply independent feed-forward compensation for independent stream. */
		UCHAR	DetectSIGOnPrimaryChannelOnly:1; /* Under 40 MHz band, detect SIG on primary channel only. */
	} field;

	UCHAR		byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#else
typedef union _BBP_R105_STRUC {
	struct
	{
		UCHAR	DetectSIGOnPrimaryChannelOnly:1; /* Under 40 MHz band, detect SIG on primary channel only. */
		UCHAR	IndependentFeedForwardCompensation:1; /* Apply independent feed-forward compensation for independent stream. */
		UCHAR	MLDFor2Stream:1; /* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX) */
		UCHAR	EnableSIGRemodulation:1; /* Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols. */
		UCHAR	Reserve1:4; /* Reserved field */
	} field;
	
	UCHAR		byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#endif

/* */
/* BBP R106 (GI remover) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R106_STRUC {
	struct
	{
		UCHAR	EnableLowPowerFSD:1; /* enable/disable the low power FSD */
		UCHAR	ShortGI_Offset40:4; /* Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate) */
		UCHAR	ShortGI_Offset20:3; /* Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate) */
	} field;

	UCHAR		byte;
} BBP_R106_STRUC, *PBBP_R106_STRUC;
#else
typedef union _BBP_R106_STRUC {
	struct
	{
		UCHAR	ShortGI_Offset20:3; /* Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate) */
		UCHAR	ShortGI_Offset40:4; /* Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate) */
		UCHAR	EnableLowPowerFSD:1; /* enable/disable the low power FSD */
	} field;
	
	UCHAR		byte;
} BBP_R106_STRUC, *PBBP_R106_STRUC;
#endif

/* */
/* BBP R109 (Tx power control in 0.1dB step) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R109_STRUC {
	struct
	{
		UCHAR	Tx1PowerCtrl:4; /* Tx1 power control in 0.1dB step (valid: 0~10) */
		UCHAR	Tx0PowerCtrl:4; /* Tx0 power control in 0.1dB step (valid: 0~10) */
	} field;

	UCHAR		byte;
} BBP_R109_STRUC, *PBBP_R109_STRUC;
#else
typedef union _BBP_R109_STRUC {
	struct
	{
		UCHAR	Tx0PowerCtrl:4; /* Tx0 power control in 0.1dB step (valid: 0~10) */
		UCHAR	Tx1PowerCtrl:4; /* Tx0 power control in 0.1dB step (valid: 0~10) */
	} field;
	
	UCHAR		byte;
} BBP_R109_STRUC, *PBBP_R109_STRUC;
#endif

/* */
/* BBP R110 (Tx power control in 0.1dB step) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R110_STRUC {
	struct
	{
		UCHAR	Tx2PowerCtrl:4; /* Tx2 power control in 0.1dB step (valid: 0~10) */
		UCHAR	AllTxPowerCtrl:4; /* All transmitters' fine power control in 0.1dB (valid: 0~10) */
	} field;

	UCHAR		byte;
} BBP_R110_STRUC, *PBBP_R110_STRUC;
#else
typedef union _BBP_R110_STRUC {
	struct
	{
		UCHAR	AllTxPowerCtrl:4; /* All transmitters' fine power control in 0.1dB (valid: 0~10) */
		UCHAR	Tx2PowerCtrl:4; /* Tx2 power control in 0.1dB step (valid: 0~10) */
	} field;
	
	UCHAR		byte;
} BBP_R110_STRUC, *PBBP_R110_STRUC;
#endif

/* */
/* BBP R179 (Test config #1) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R179_STRUC {
	struct
	{
		UCHAR	DataIndex1:8; /* Data index #1 */
	} field;

	UCHAR		byte;
} BBP_R179_STRUC, *PBBP_R179_STRUC;
#else
typedef union _BBP_R179_STRUC {
	struct
	{
		UCHAR	DataIndex1:8; /* Data index #1 */
	} field;
	
	UCHAR		byte;
} BBP_R179_STRUC, *PBBP_R179_STRUC;
#endif /* RT_BIG_ENDIAN */

/* */
/* BBP R180 (Test config #2) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R180_STRUC {
	struct
	{
		UCHAR	DataIndex2:8; /* Data index #2 */
	} field;

	UCHAR		byte;
} BBP_R180_STRUC, *PBBP_R180_STRUC;
#else
typedef union _BBP_R180_STRUC {
	struct
	{
		UCHAR	DataIndex2:8; /* Data index #2 */
	} field;
	
	UCHAR		byte;
} BBP_R180_STRUC, *PBBP_R180_STRUC;
#endif /* RT_BIG_ENDIAN */

/* */
/* BBP R182 (Test data port) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R182_STRUC {
	struct
	{
		UCHAR	DataArray:8; /* Data array indexed by BBP R179 and R180 */
	} field;

	UCHAR		byte;
} BBP_R182_STRUC, *PBBP_R182_STRUC;
#else
typedef union _BBP_R182_STRUC {
	struct
	{
		UCHAR	DataArray:8; /* Data array indexed by BBP R179 and R180 */
	} field;
	
	UCHAR		byte;
} BBP_R182_STRUC, *PBBP_R182_STRUC;
#endif /* RT_BIG_ENDIAN */

#ifdef RTMP_RBUS_SUPPORT
/* TODO: for this definition, need to modify it!! */
	/*#define MAX_BBP_ID	255 */
	#define MAX_BBP_ID	200

#else
	#define MAX_BBP_ID	136

#endif /* RTMP_RBUS_SUPPORT */


#if defined(RT5572) || defined(RT6352)
#undef MAX_BBP_ID
#define MAX_BBP_ID	248
#undef MAX_BBP_MSG_SIZE
#define MAX_BBP_MSG_SIZE 4096
#endif /* defined(RT5572) || defined(RT6352) */


/* */
/* BBP & RF are using indirect access. Before write any value into it. */
/* We have to make sure there is no outstanding command pending via checking busy bit. */
/* */
#define MAX_BUSY_COUNT  100         /* Number of retry before failing access BBP & RF indirect register */
#define MAX_BUSY_COUNT_US 2000      /* Number of retry before failing access BBP & RF indirect register */

/*#define PHY_TR_SWITCH_TIME          5  // usec */

/*#define BBP_R17_LOW_SENSIBILITY     0x50 */
/*#define BBP_R17_MID_SENSIBILITY     0x41 */
/*#define BBP_R17_DYNAMIC_UP_BOUND    0x40 */

#define RSSI_FOR_VERY_LOW_SENSIBILITY   -35
#define RSSI_FOR_LOW_SENSIBILITY		-58
#define RSSI_FOR_MID_LOW_SENSIBILITY	-65 /*-80*/
#define RSSI_FOR_MID_SENSIBILITY		-90

/*****************************************************************************
	RF register Read/Write marco definition
 *****************************************************************************/
#ifdef RTMP_MAC_PCI
#define RTMP_RF_IO_WRITE32(_A, _V)                  \
{                                                   					\
	if ((_A)->bPCIclkOff == FALSE) 	                \
	{												\
		RF_CSR_CFG0_STRUC  _value;                          \
		ULONG           _busyCnt = 0;                    \
											\
		do {                                            \
			RTMP_IO_READ32((_A), RF_CSR_CFG0, &_value.word);  \
			if (_value.field.Busy == IDLE)               \
				break;                                  \
			_busyCnt++;                                  \
		}while (_busyCnt < MAX_BUSY_COUNT);			\
		if(_busyCnt < MAX_BUSY_COUNT)                   \
		{                                               \
			RTMP_IO_WRITE32((_A), RF_CSR_CFG0, (_V));          \
    		}                                               \
    	}								\
    if ((_A)->ShowRf)					\
    {									\
    	printk("RF:%x\n", _V);			\
	}									\
}
#endif /* RTMP_MAC_PCI */



/*****************************************************************************
	BBP register Read/Write marco definitions.
	we read/write the bbp value by register's ID. 
	Generate PER to test BA
 *****************************************************************************/
#ifdef RTMP_MAC_PCI
/*
	basic marco for BBP read operation. 
	_pAd: the data structure pointer of RTMP_ADAPTER
	_bbpID : the bbp register ID
	_pV: data pointer used to save the value of queried bbp register.
	_bViaMCU: if we need access the bbp via the MCU.
*/
#ifdef RELASE_INCLUDE
/*
	The RTMP_PCIE_PS_L3_BBP_IO_READ8 is used to support PCIE power-saving solution3.
	"brc =AsicSendCommandToMcuBBP" is used to avoid any muc command is executed during
	RF_OFF command.
*/
#endif /* RELASE_INCLUDE */

	
#ifdef CONFIG_STA_SUPPORT
#define IS_SUPPORT_PCIE_PS_L3(_pAd) (((_pAd)->OpMode == OPMODE_STA) &&\
	(IS_RT3090((_pAd)) || IS_RT3572((_pAd)) || IS_RT3390((_pAd)) || IS_RT3593((_pAd)) || IS_RT5390((_pAd)) || IS_RT5392((_pAd)) || IS_RT3290((_pAd))) && \
	((_pAd)->StaCfg.PSControl.field.rt30xxPowerMode == 3)&& \
	((_pAd)->StaCfg.PSControl.field.EnableNewPS == TRUE)) 
	
#define RTMP_PCIE_PS_L3_BBP_IO_READ8(_pAd, _bbpID, _pV, _bViaMCU)			\
	do{															\
	BBP_CSR_CFG_STRUC	BbpCsr;									\
	int					_busyCnt, _secCnt, _regID;					\
	BOOLEAN					brc;									\
	ULONG __IrqFlags = 0;										\
																\
	if ((_bViaMCU) == TRUE) \
	RTMP_MAC_SHR_MSEL_PROTECT_LOCK(_pAd, __IrqFlags);			\
	_regID = ((_bViaMCU) == TRUE ? H2M_BBP_AGENT : BBP_CSR_CFG);	\
	BbpCsr.field.Busy = IDLE;										\
	if (((_pAd)->bPCIclkOff == FALSE)								\
		&& (((_pAd)->LastMCUCmd== WAKE_MCU_CMD) || ((_pAd)->LastMCUCmd==0x72))\
		&& ((_pAd)->brt30xxBanMcuCmd == FALSE))					\
	{															\
		for (_busyCnt=0; _busyCnt<MAX_BUSY_COUNT; _busyCnt++)	\
		{														\
			RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word);			\
			if (BbpCsr.field.Busy == BUSY)                 					\
				continue;                                               				\
			BbpCsr.word = 0;										\
			BbpCsr.field.fRead = 1;									\
			BbpCsr.field.BBP_RW_MODE = 1;							\
			BbpCsr.field.Busy = 1;									\
			BbpCsr.field.RegNum = _bbpID;                       			\
			RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);			\
			if ((_bViaMCU) == TRUE)								\
			{													\
				brc =AsicSendCommandToMcuBBP(_pAd, 0x80, 0xff, 0x0, 0x0, FALSE); \
				/*RtmpusecDelay(1000);*/							\
			}							\
	               if (brc == TRUE) 										\
			{                                                 								\
				for (_secCnt=0; _secCnt<MAX_BUSY_COUNT; _secCnt++)       	\
				{														\
					RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word); 	\
					if (BbpCsr.field.Busy == IDLE)							\
						break;											\
				}														\
				if ((BbpCsr.field.Busy == IDLE) &&							\
				(BbpCsr.field.RegNum == _bbpID))                					\
				{																\
					*(_pV) = (UCHAR)BbpCsr.field.Value;							\
					break;														\
				}																\
			}																\
			else 																\
			{																\
				BbpCsr.field.Busy = 0;											\
				RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);				\
			}																\
		}																	\
	}	\
	if ((BbpCsr.field.Busy == BUSY) || ((_pAd)->bPCIclkOff == TRUE))				\
	{																	\
	                DBGPRINT_ERR(("RTMP_PCIE_PS_L3_BBP_IO_READ8(viaMCU=%d) read R%d fail(reason:clk=%d,busy=%x)\n", (_bViaMCU), _bbpID,(_pAd)->bPCIclkOff ,BbpCsr.field.Busy));      \
			*(_pV) = (_pAd)->BbpWriteLatch[_bbpID];               \
	}																	\
	if ((_bViaMCU) == TRUE) \
	RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(_pAd, __IrqFlags);	\
}while(0)
#else
#define IS_SUPPORT_PCIE_PS_L3(_pAd) FALSE
#define RTMP_PCIE_PS_L3_BBP_IO_READ8(_pAd, _bbpID, _pV, _bViaMCU)
#endif /* CONFIG_STA_SUPPORT */


/*
	This marco used for the BBP read operation which didn't need via MCU.
*/
#define BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)			\
	RTMP_BBP_IO_READ8((_A), (_I), (_pV), TRUE)


/*
	basic marco for BBP write operation. 
	_pAd: the data structure pointer of RTMP_ADAPTER
	_bbpID : the bbp register ID
	_pV: data used to save the value of queried bbp register.
	_bViaMCU: if we need access the bbp via the MCU.
*/
#ifdef CONFIG_STA_SUPPORT
#define RTMP_PCIE_PS_L3_BBP_IO_WRITE8(_pAd, _bbpID, _pV, _bViaMCU)				\
	do{											\
		BBP_CSR_CFG_STRUC  BbpCsr;							\
		int             k, _busyCnt=0, _regID;						\
		BOOLEAN					brc;					\
		ULONG				__IrqFlags = 0;						\
		if ((_bViaMCU) == TRUE) \
		RTMP_MAC_SHR_MSEL_PROTECT_LOCK(_pAd, __IrqFlags);		\
		_regID = ((_bViaMCU) == TRUE ? H2M_BBP_AGENT : BBP_CSR_CFG);			\
			if (((_pAd)->bPCIclkOff == FALSE)					\
			&& ((_pAd)->brt30xxBanMcuCmd == FALSE))								\
			{																	\
				if (_pAd->AccessBBPFailCount > 20)								\
				{																\
					AsicResetBBPAgent(_pAd);									\
					_pAd->AccessBBPFailCount = 0;								\
				}																\
				for (_busyCnt=1; _busyCnt<MAX_BUSY_COUNT; _busyCnt++)  \
				{                                                  						 \
					RTMP_IO_READ32((_pAd), _regID, &BbpCsr.word);     \
					if ((BbpCsr.field.Busy == BUSY) && (_busyCnt % 10 == 0))	\
					{															\
						BbpCsr.field.Busy = IDLE;								\
						RTMP_IO_WRITE32(_pAd, H2M_BBP_AGENT, BbpCsr.word);		\
						continue;                                   \
					}												\
					BbpCsr.word = 0;                                \
					BbpCsr.field.fRead = 0;                         \
					BbpCsr.field.BBP_RW_MODE = 1;                         \
					BbpCsr.field.Busy = 1;                          \
					BbpCsr.field.Value = _pV;                        \
					BbpCsr.field.RegNum = _bbpID;                       \
					RTMP_IO_WRITE32((_pAd), _regID, BbpCsr.word);     \
					if ((_bViaMCU) == TRUE)									\
					{														\
						brc =AsicSendCommandToMcuBBP(_pAd, 0x80, 0xff, 0x0, 0x0, FALSE);		\
						if ((_pAd)->OpMode == OPMODE_AP)						\
							RtmpusecDelay(1000);							\
					}														\
					if (brc == TRUE) 											\
					{														\
						for (k=0; k<MAX_BUSY_COUNT; k++)								\
						{																\
							BbpCsr.field.Busy = BUSY;											\
							RTMP_IO_READ32(_pAd, H2M_BBP_AGENT, &BbpCsr.word);			\
							if (BbpCsr.field.Busy == IDLE)								\
								break;													\
						}																\
						if ((BbpCsr.field.Busy == BUSY))										\
						{																	\
							DBGPRINT_ERR(("Check BBP write R%d=0x%x fail\n", _bbpID, BbpCsr.word));	\
						}						\
						(_pAd)->BbpWriteLatch[_bbpID] = _pV;                   		\
					}														\
					else 													\
					{														\
						BbpCsr.field.Busy = 0;									\
						RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);	\
					}								\
					break;													\
				}  	\
			}		\
			else 										\
			{																	\
				DBGPRINT_ERR(("  brt30xxBanMcuCmd = %d. Write BBP %d \n",  (_pAd)->brt30xxBanMcuCmd, (_regID)));	\
			}																	\
		if ((_busyCnt == MAX_BUSY_COUNT) || ((_pAd)->bPCIclkOff == TRUE))			\
			{																	\
				if (_busyCnt == MAX_BUSY_COUNT)					\
				(_pAd)->AccessBBPFailCount++;					\
				DBGPRINT_ERR(("BBP write R%d=0x%x fail. BusyCnt= %d.bPCIclkOff = %d. \n", _regID, BbpCsr.word, _busyCnt, (_pAd)->bPCIclkOff ));	\
			}																	\
		if ((_bViaMCU) == TRUE) \
		RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(_pAd, __IrqFlags);	\
	}while(0)
#else
#define RTMP_PCIE_PS_L3_BBP_IO_WRITE8(_pAd, _bbpID, _pV, _bViaMCU)
#endif /* CONFIG_STA_SUPPORT */


/*
	This marco used for the BBP write operation which didn't need via MCU.
*/
#define BBP_IO_WRITE8_BY_REG_ID(_A, _I, _pV)	RTMP_BBP_IO_WRITE8((_A), (_I), (_pV), FALSE)


/* 
	Hardware Periodic Timer interrupt setting.
	Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
*/
#define RTMP_HW_TIMER_INT_SET(_pAd, _V)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(_pAd, INT_TIMER_CFG, &temp);		\
		temp &= 0x0000ffff;								\
		temp |= _V << 20; 								\
		RTMP_IO_WRITE32(_pAd, INT_TIMER_CFG, temp);		\
	}

/* Enable Hardware Periodic Timer interrupt */
#define RTMP_HW_TIMER_INT_ENABLE(_pAd)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &temp);		\
		temp |=0x2;										\
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, temp);		\
	}

/* Disable Hardware Periodic Timer interrupt */
#define RTMP_HW_TIMER_INT_DISABLE(_pAd)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &temp);		\
		temp &=~(0x2);									\
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, temp);		\
	}
#endif /* RTMP_MAC_PCI */
	
#ifdef CARRIER_DETECTION_SUPPORT
/*TONE_RADAR_DETECT_V2*/
#define RTMP_CARRIER_IO_READ8(_A, _I, _V)               \
{                                                       \
	RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R184, _I);          \
	RTMP_BBP_IO_READ8_BY_REG_ID(_A, BBP_R185, _V);           \
}
#define RTMP_CARRIER_IO_WRITE8(_A, _I, _V)              \
{                                                       \
	RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R184, _I);          \
	RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R185, _V);          \
}
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DFS_SUPPORT
#define RTMP_DFS_IO_READ8(_A, _I, _V)                   \
{                                                       \
	RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R140, _I);          \
	RTMP_BBP_IO_READ8_BY_REG_ID(_A, BBP_R141, _V);           \
}

#define RTMP_DFS_IO_WRITE8(_A, _I, _V)                  \
{                                                       \
	RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R140, _I);          \
	RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R141, _V);          \
}
#endif /*DFS_SUPPORT*/





struct _RMTP_ADAPTER;

INT rtmp_phy_probe(struct _RTMP_ADAPTER *pAd);

VOID RTMP_BBP_IO_READ8(struct _RTMP_ADAPTER *pAd, UCHAR Offset, UINT8 *pVal, BOOLEAN FlgValidMCR);
VOID RTMP_BBP_IO_WRITE8(struct _RTMP_ADAPTER *pAd, UCHAR Offset, UINT8 Val, BOOLEAN FlgValidMCR);

NTSTATUS RTMP_BBP_IO_READ8_BY_REG_ID(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT8 *pVal);
NTSTATUS RTMP_BBP_IO_WRITE8_BY_REG_ID(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT8 Val);

#endif /* __RTMP_BBP_H__ */

