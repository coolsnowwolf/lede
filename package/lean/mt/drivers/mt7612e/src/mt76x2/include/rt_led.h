/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_led.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#ifndef __RT_LED_H__
#define __RT_LED_H__

/* LED MCU command */
#define MCU_SET_LED_MODE				0x50
#define MCU_SET_LED_GPIO_SIGNAL_CFG		0x51
#define MCU_SET_LED_AG_CFG 				0x52
#define MCU_SET_LED_ACT_CFG 			0x53
#define MCU_SET_LED_POLARITY			0x54
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
#define MCU_SET_WPS_LED_MODE			0x55 /* Set WPS LED mode (based on WPS specification V1.0). */
#define MCU_SET_ANT_DIVERSITY			0x73
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

/* LED Mode */
#define LED_MODE(pAd) ((pAd)->LedCntl.MCULedCntl.field.LedMode & 0x7F)
#define LED_HW_CONTROL					19	/* set LED to controll by MAC registers instead of by firmware */
#define LED_MODE_DEFAULT            	0	/* value domain of pAd->LedCntl.LedMode and E2PROM */
#define LED_MODE_TWO_LED				1
#define LED_MODE_8SEC_SCAN				2	/* Same as LED mode 1; except that fast blink for 8sec when doing scanning. */
#define LED_MODE_SITE_SURVEY_SLOW_BLINK	3	/* Same as LED mode 1; except that make ACT slow blinking during site survey period and blink once at power-up. */
#define LED_MODE_WPS_LOW_POLARITY		4	/* Same as LED mode 1; except that make ACT steady on during WPS period */
#define LED_MODE_WPS_HIGH_POLARITY		5	/* Same as LED mode 1; except that make ACT steady on during WPS period */
/*#define LED_MODE_SIGNAL_STREGTH		8   // EEPROM define =8 */
#define LED_MODE_SIGNAL_STREGTH			0x40 /* EEPROM define = 64 */
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
#define LED_WPS_MODE10_TURN_ON			16 	/* For Dlink WPS LED, turn the WPS LED on */
#define LED_WPS_MODE10_FLASH			17	/* For Dlink WPS LED, let the WPS LED flash */
#define LED_WPS_MODE10_TURN_OFF			18	/* For Dlink WPS LED, turn the WPS LED off */
#define WPS_LED_MODE_7					0x07
#define WPS_LED_MODE_8					0x08
#define WPS_LED_MODE_9					0x09
#define WPS_LED_MODE_10					0x0a
/* */
/* Chungwa Telecom (WPS LED and SD/HD LEDs) use WPS LED mode #11 and it is based on mode #7 */
/* */
/* GPIO #1: WPS LED */
/* GPIO #2: SD (Standard Definition) LED */
/* GPIO #4: HD (High Definition) LED */
/* */
#define WPS_LED_MODE_11				0x0b
#define WPS_LED_MODE_12				0x0c
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

/* Driver LED Status */
#define LED_LINK_DOWN			0
#define LED_LINK_UP				1
#define LED_RADIO_OFF			2
#define LED_RADIO_ON			3
#define LED_HALT				4
#define LED_WPS					5
#define LED_ON_SITE_SURVEY		6
#define LED_POWER_UP			7
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/* For LED mode = 7 only */
#define LED_WPS_IN_PROCESS						8  /* The protocol is searching for a partner, connecting, or exchanging network parameters. */
#define LED_WPS_ERROR							9  /* Some error occurred which was not related to security, such as failed to find any partner or protocol prematurely aborted. */
#define LED_WPS_SESSION_OVERLAP_DETECTED		10 /* The Protocol detected overlapping operation (more than one Registrar in PBC mode): could be a security risk. */
#define LED_WPS_SUCCESS							11 /* The protocol is finished: no uncorrectable errors occured. Normally after guard time period. */
#define LED_WPS_TURN_LED_OFF					12 /* Turn the WPS LEDs off. */
#define LED_WPS_TURN_ON_BLUE_LED				13 /* Turn on the WPS blue LED. */
#define LED_NORMAL_CONNECTION_WITHOUT_SECURITY	14 /* Successful connection with an AP using OPEN-NONE. */
#define LED_NORMAL_CONNECTION_WITH_SECURITY		15 /* Successful connection with an AP using an encryption algorithm. */
/* For LED Share Mode */
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

/* MCU Led Link Status */
#define LINK_STATUS_LINK_DOWN		0x20
#define LINK_STATUS_ABAND_LINK_UP	0xa0
#define LINK_STATUS_GBAND_LINK_UP	0x60
#define LINK_STATUS_RADIO_ON		0x20
#define LINK_STATUS_RADIO_OFF		0x00
#define LINK_STATUS_WPS				0x10
#define LINK_STATUS_ON_SITE_SURVEY	0x08
#define LINK_STATUS_POWER_UP		0x04
#define LINK_STATUS_HW_CONTROL		0x00
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/* LED mode = 7 and 8 only */
#define LINK_STATUS_WPS_IN_PROCESS						0x00 /* The protocol is searching for a partner, connecting, or exchanging network parameters. */
#define LINK_STATUS_WPS_SUCCESS_WITH_SECURITY			0x01 /* The protocol is finished (with security): no uncorrectable errors occured. Normally after guard time period. */
#define LINK_STATUS_WPS_ERROR							0x02 /* Some error occurred which was not related to security, such as failed to find any partner or protocol prematurely aborted. */
#define LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED		0x03 /* The Protocol detected overlapping operation (more than one Registrar in PBC mode): could be a security risk. */
#define LINK_STATUS_WPS_TURN_LED_OFF					0x04 /* Turn the WPS LEDs off. */
#define LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY		0X05 /* The protocol is finished (without security): no uncorrectable errors occured. Normally after guard time period. */
#define LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY	0x06 /* Successful connection with an AP using OPEN-NONE. */
#define LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY		0x0E /* Successful connection with an AP using an encryption algorithm. */
#define LINK_STATUS_WPS_BLUE_LED						0x01 /* WPS blue LED. */
/* LED moe = 10 */
#define LINK_STATUS_WPS_MODE10_TURN_ON					0x00	/*Use only on Dlink WPS LED (mode 10), turn the WPS LED on. */
#define LINK_STATUS_WPS_MODE10_FLASH					0x01	/*Use only on Dlink WPS LED (mode 10), let the WPS LED flash, three times persecond. */
#define LINK_STATUS_WPS_MODE10_TURN_OFF					0x02	/*Use only on Dlink WPS LED (mode 10), turn the WPS LED off. */
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef CONFIG_SWMCU_SUPPORT
/* SWMCU Led Status */
/* LED G*/
#define MCU_LED_G_OFF			0
#define MCU_LED_G_SOLID_ON		1
#define MCU_LED_G_SLOW_BLINK	2
#define MCU_LED_G_FAST_BLINK	3
/* LED A */
#define MCU_LED_A_OFF			0
#define MCU_LED_A_SOLID_ON		1
#define MCU_LED_A_SLOW_BLINK	2
#define MCU_LED_A_FAST_BLINK	3
/* LED ACT */
#define MCU_LED_ACT_OFF											0
#define MCU_LED_ACT_SOLID_ON									1
#define MCU_LED_ACT_BLINK_UPON_TX_DATA_MNG						2
#define MCU_LED_ACT_BLINK_UPON_TX_DATA_MNG_BEN					3
#define MCU_LED_ACT_WPS_IN_PROCESS								6
#define MCU_LED_ACT_WPS_ERROR									7
#define MCU_LED_ACT_WPS_SESSION_OVERLAP_DETECTED				8
#define MCU_LED_ACT_WPS_SUCCESS_WITH_SECURITY					9
#define MCU_LED_ACT_WPS_SUCCESS_WITHOUT_SECURITY				10
#define MCU_LED_ACT_WPS_TURN_LED_OFF							11
#define MCU_LED_ACT_WPS_NORMAL_CONNECTION_WITHOUT_SECURITY		12
#define	MCU_LED_ACT_WPS_NORMAL_CONNECTION_WITH_SECURITY			13
#endif /* CONFIG_SWMCU_SUPPORT */

#define ACTIVE_LOW 	0
#define ACTIVE_HIGH 1

/* */
/* MCU_LEDCS: MCU LED Control Setting. */
/* */
typedef union  _MCU_LEDCS_STRUC {
	struct	{
#ifdef RT_BIG_ENDIAN
		UCHAR		Polarity:1;
		UCHAR		LedMode:7;
#else
		UCHAR		LedMode:7;		
		UCHAR		Polarity:1;
#endif /* RT_BIG_ENDIAN */
	} field;
	UCHAR				word;
} MCU_LEDCS_STRUC, *PMCU_LEDCS_STRUC;

void RTMPGetLEDSetting(IN RTMP_ADAPTER *pAd);
void RTMPInitLEDMode(IN RTMP_ADAPTER *pAd);
void RTMPExitLEDMode(IN RTMP_ADAPTER *pAd);

VOID RTMPSetLEDStatus(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR			Status);


#ifdef RTMP_MAC_PCI
#ifdef LED_CONTROL_SUPPORT
#define RTMPSetLED(pAd, Status)	RTMPSetLEDStatus(pAd, Status)
#else
#define RTMPSetLED(pAd, Status)
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_MAC_PCI */

VOID RTMPSetSignalLED(
	IN PRTMP_ADAPTER 	pAd, 
	IN NDIS_802_11_RSSI Dbm);

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
VOID LEDConnectionStart(
	IN PRTMP_ADAPTER pAd);

VOID LEDConnectionCompletion(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN bSuccess);
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */

#ifdef CONFIG_SWMCU_SUPPORT
/*************************************************************************
  *
  *	LED related data type and MARCO definitions.
  *
  ************************************************************************/
#define GetBitX(x, y)			(	((x) & (1 << (y)))>>y		)
#define GetBit(x, y)			(	((x) & (1 << (y)))		)
#define GetBitX(x, y)			(	((x) & (1 << (y)))>>y		)
#define SetBit(x, y)			(	(x) = (x) | (1 << (y))	)
#define ClearBit(x, y)			{	(x) &= (~(1 << (y)));	}
#define DSetBit(x, y)			(	(x) = (1 << (y))		)
#define MakeWord(HI, LO)		(	((WORD)(LO)) | (((WORD)(HI))<<8) )	/* Note! LO must be read first */
#define HiByte(W)				(	(BYTE)(((WORD)W) >> 8)	)
#define LoByte(W)				(	(BYTE)(W)				)
#define FireInterruptToHost()	{	IntToHost = 0xff;		}

#define LED_CHECK_INTERVAL 	50 /* 50ms */

#define LED_G			0
#define LED_A			1
#define LED_ACT			2

#define MCU_INT_STATUS	0x0414
#define GPIO_DAT		0x0228
#define GPIO_DIR		0x0229
#define MAC_LED_CFG		0x102c

typedef unsigned char	BYTE;

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/* SWMCU LED Current Operation Mode */
typedef union _LED_OPERATION_MODE
{
	struct {
		UINT32 :20;
		UINT32 LedGMode:2;		/* 
								 * 0: MCU_LED_G_OFF
								 * 1: MCU_LED_G_SOLID_ON, 
								 * 2: MCU_LED_G_SLOW_BLINK 
								 * 3: MCU_LED_G_FAST_BLINK 
								 */
								 
		UINT32 LedAMode:2;		/* 
								 * 0: MCU_LED_A_OFF
								 * 1: MCU_LED_A_SOLID_ON, 
								 * 2: MCU_LED_A_SLOW_BLINK 
								 * 3: MCU_LED_A_FAST_BLINK. 
								 */
								 
		UINT32 LedActMode:4;	/*
								 * 0: MCU_LED_ACT_OFF
								 * 1: MCU_LED_ACT_SOLID_ON
								 * 2: MCU_LED_ACT_BLINK_UPON_TX_DATA_MNG
								 * 3: MCU_LED_ACT_BLINK_UPON_TX_DATA_MNG_BEN 
								 * 6: MCU_LED_ACT_WPS_IN_PROCESS,
								 * 7: MCU_LED_ACT_WPS_ERROR
								 * 8: MCU_LED_ACT_WPS_SESSION_OVERLAP_DETECTED
								 * 9: MCU_LED_ACT_WPS_SUCCESS_WITH_SECURITY
								 * 10:MCU_LED_ACT_WPS_SUCCESS_WITHOUT_SECURITY
								 * 11:MCU_LED_ACT_WPS_TURN_LED_OFF
								 * 12:MCU_LED_ACT_NORMAL_CONNECTION_WITHOUT_SECURITY
								 * 13:MCU_LED_ACT_NORMAL_CONNECTION_WITH_SECURITY
								 * 14:MCU_LED_ACT_WPS_PRE_STAGE
								 * 15:MCU_LED_ACT_WPS_POST_STAGE
								 */
		UINT32 LedActModeNoTx:1;
		UINT32 LedGPolarity:1;
		UINT32 LedAPolarity:1;
		UINT32 LedActPolarity:1;
	} field;
	UINT word;
} LED_OPERATION_MODE, *PLED_OPERATION_MODE;

/* MCU_LEDCS: MCU LED A/G Configure Setting */
typedef union _LED_AG_CFG
{
	struct {
#ifdef RT_BIG_ENDIAN
		UINT16 LedAMode_RadioOnLinkA:2;
		UINT16 LedGMode_RadioOnLinkA:2;
		UINT16 LedAMode_RadioOnLinkG:2;
		UINT16 LedGMode_RadioOnLinkG:2;
		UINT16 LedAMode_RadioOnLinkDown:2;
		UINT16 LedGMode_RadioOnLinkDown:2;
		UINT16 LedAMode_RadioOff:2;
		UINT16 LedGMode_RadioOff:2;
#else
		UINT16 LedGMode_RadioOff:2;
		UINT16 LedAMode_RadioOff:2;
		UINT16 LedGMode_RadioOnLinkDown:2;
		UINT16 LedAMode_RadioOnLinkDown:2;
		UINT16 LedGMode_RadioOnLinkG:2;
		UINT16 LedAMode_RadioOnLinkG:2;
		UINT16 LedGMode_RadioOnLinkA:2;
		UINT16 LedAMode_RadioOnLinkA:2;
#endif
	} field;
	UINT16 word;
} LED_AG_CFG, *PLED_AG_CFG;

/* MCU_LEDCS: MCU LED ACT Configure Setting */
typedef union _LED_ACT_CFG
{
	struct {
#ifdef RT_BIG_ENDIAN
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOnLinkA:1;
		UINT16 LedActMode_RadioOnLinkA:2;
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOnLinkG:1;
		UINT16 LedActMode_RadioOnLinkG:2;
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOnLinkDown:1;
		UINT16 LedActMode_RadioOnLinkDown:2;
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOff:1;
		UINT16 LedActMode_RadioOff:2;
#else
		UINT16 LedActMode_RadioOff:2;
		UINT16 LedActModeNoTx_RadioOff:1;
		UINT16 :1;
		UINT16 LedActMode_RadioOnLinkDown:2;
		UINT16 LedActModeNoTx_RadioOnLinkDown:1;
		UINT16 :1;
		UINT16 LedActMode_RadioOnLinkG:2;
		UINT16 LedActModeNoTx_RadioOnLinkG:1;
		UINT16 :1;
		UINT16 LedActMode_RadioOnLinkA:2;
		UINT16 LedActModeNoTx_RadioOnLinkA:1;
		UINT16 :1;
#endif
	} field;
	UINT16 word;
} LED_ACT_CFG, *PLED_ACT_CFG;

/* MCU_LEDCS: MCU LED POLARITY Configure Setting. */
typedef union _LED_POLARITY
{
	struct {
#ifdef RT_BIG_ENDIAN
		UINT16 :1;
		UINT16 LedActPolarity_RadioOnLinkA:1;
		UINT16 LedAPolarity_RadioOnLinkA:1;
		UINT16 LedGPolarity_RadioOnLinkA:1;
		UINT16 :1;
		UINT16 LedActPolarity_RadioOnLinkG:1;
		UINT16 LedAPolarity_RadioOnLinkG:1;
		UINT16 LedGPolarity_RadioOnLinkG:1;
		UINT16 :1;
		UINT16 LedActPolarity_RadioOnLinkDown:1;
		UINT16 LedAPolarity_RadioOnLinkDown:1;
		UINT16 LedGPolarity_RadioOnLinkDown:1;
		UINT16 :1;
		UINT16 LedActPolarity_RadioOff:1;
		UINT16 LedAPolarity_RadioOff:1;
		UINT16 LedGPolarity_RadioOff:1;
#else
		UINT16 LedGPolarity_RadioOff:1;
		UINT16 LedAPolarity_RadioOff:1;
		UINT16 LedActPolarity_RadioOff:1;
		UINT16 :1;
		UINT16 LedGPolarity_RadioOnLinkDown:1;
		UINT16 LedAPolarity_RadioOnLinkDown:1;
		UINT16 LedActPolarity_RadioOnLinkDown:1;
		UINT16 :1;
		UINT16 LedGPolarity_RadioOnLinkG:1;
		UINT16 LedAPolarity_RadioOnLinkG:1;
		UINT16 LedActPolarity_RadioOnLinkG:1;
		UINT16 :1;
		UINT16 LedGPolarity_RadioOnLinkA:1;
		UINT16 LedAPolarity_RadioOnLinkA:1;
		UINT16 LedActPolarity_RadioOnLinkA:1;
		UINT16 :1;
#endif
	} field;
	UINT16 word;
} LED_POLARITY, *PLED_POLARITY;

/* SWMCU LED Current Parameters */
typedef struct _ULED_PARAMETER
{
	LED_AG_CFG LedAgCfg;
	LED_ACT_CFG LedActCfg;
	LED_POLARITY LedPolarityCfg;
	UINT8 LedMode;
}ULED_PARAMETER, *PULED_PARAMETER;

/* Mac LED_CFG macro and definition */
#define MAC_LED_OFF				0
#define MAC_LED_BLINK_UPON_TX	1
#define MAC_LED_SLOW_BLINK		2
#define MAC_LED_ON				3

typedef union _LED_CFG_T
{
	struct{
#ifdef RT_BIG_ENDIAN
		UINT32 :1;
		UINT32 LED_POL:1;		/* 0: active low, 1:active high. */
		UINT32 Y_LED_MODE:2;	/* 0: off, 1: blinking upon Tx,
									2:Periodic slow blinking, 3:sloid on. */
		UINT32 G_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 R_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 :2;
		UINT32 SLOW_BLK_TIME:6;	/* slow blinking period (uint: 1sec). */
		UINT32 LED_OFF_TIIME:8;	/* Tx blinking off period (unit: 1ms). */
		UINT32 LED_ON_TIME:8; 	/* Tx blinking on period (uint: 1ms). */
#else
		UINT32 LED_ON_TIME:8; 	/* Tx blinking on period (uint: 1ms). */
		UINT32 LED_OFF_TIME:8;	/* Tx blinking off period (unit: 1ms). */
		UINT32 SLOW_BLK_TIME:6;	/* slow blinking period (uint: 1sec). */
		UINT32 :2;
		UINT32 R_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 G_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 Y_LED_MODE:2;	/* 0: off, 1: blinking upon Tx,
									2:Periodic slow blinking, 3:sloid on. */
		UINT32 LED_POL:1;		/* 0: active low, 1:active high. */
		UINT32 :1;
#endif
	} field;
	UINT32 word;
} LED_CFG_T, *PLED_CFG_T;

#define INFINITE_PERIOD (-1)
/* 
 * WPS User Feedback Time Unit
 * Time unit is LED_CHECK_INTERVAL
 */
typedef struct _WPS_LED_TIME_UNIT {
	INT32 OnPeriod, LeftOnPeriod;
	INT32 OffPeriod, LeftOffPeriod;
	INT32 BlinkPeriod, LeftBlinkPeriod;
	INT32 RestPeriod, LeftRestPeriod;
}WPS_LED_TIME_UNIT, *PWPS_LED_TIME_UNIT;

typedef struct _SWMCU_LED_CONTROL {
	RALINK_TIMER_STRUCT LedCheckTimer;
	ULED_PARAMETER LedParameter;
	LED_OPERATION_MODE CurrentLedCfg;
	unsigned char LinkStatus;
	unsigned char GPIOPolarity;
	unsigned char SignalStrength;
	unsigned char LedBlinkTimer;
	unsigned long BlinkFor8sTimer;
	BOOLEAN bWlanLed;
	WPS_LED_TIME_UNIT WPSLedTimeUnit;
}SWMCU_LED_CONTROL, *PSWMCU_LED_CONTROL;

INT Show_LedCfg_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

INT Set_LedCfg_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

INT Set_LedCheck_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

VOID SetLedCfg(
	IN PRTMP_ADAPTER pAd,
	IN INT ledPolarity,
	IN INT ledOnTime,
	IN INT ledOffTime,
	IN INT slowBlkTime);

VOID SetLedMode(
	IN PRTMP_ADAPTER pAd,
	IN INT LedSel,
	IN INT LedMode);

void SWMCULedCtrlMain(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(SWMCULedCtrlMain);

void SetLedLinkStatus(
	IN PRTMP_ADAPTER pAd);


#endif /* CONFIG_SWMCU_SUPPORT */

typedef struct _LED_CONTROL
{
	MCU_LEDCS_STRUC		MCULedCntl; /* LED Mode EEPROM 0x3b */
	USHORT				LedAGCfg;	/* LED A/G Configuration EEPROM 0x3c */
	USHORT				LedACTCfg;	/* LED ACT Configuration EEPROM 0x3e */
	USHORT				LedPolarity;/* LED A/G/ACT polarity EEPROM 0x40 */
	UCHAR				LedIndicatorStrength;
	UCHAR				RssiSingalstrengthOffet;
	BOOLEAN				bLedOnScanning;
	UCHAR				LedStatus;
#ifdef CONFIG_SWMCU_SUPPORT
	SWMCU_LED_CONTROL SWMCULedCntl;
#endif /* CONFIG_SWMCU_SUPPORT */
}LED_CONTROL, *PLED_CONTROL;

void RTMPStartLEDMode(IN RTMP_ADAPTER *pAd);

#endif /* __RT_LED_H__ */

