/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	omci.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/5	Create
*/
#ifndef _PMMGR_DEFINE_H_
#define _PMMGR_DEFINE_H_

#define PMMGR_DEBUG_NO_MSG 0
#define PMMGR_DEBUG_ERROR  	1
#define PMMGR_DEBUG_WARN	2
#define PMMGR_DEBUG_TRACE	3

#ifndef _int8_defined
#define _int8_defined
typedef signed char 			int8;
#endif

#ifndef _uint8_defined
#define _uint8_defined
typedef unsigned char			uint8;
#endif

#ifndef _int16_defined
#define _int16_defined
typedef signed short 			int16;
#endif

#ifndef _uint16_defined
#define _uint16_defined
typedef unsigned short			uint16;
#endif

#ifndef _int32_defined
#define _int32_defined
typedef int					int32;
#endif

#ifndef _uint32_defined
#define _uint32_defined
typedef unsigned int			uint32;
#endif

#ifndef _int64_defined
#define _int64_defined
typedef long long 				int64;
#endif

#ifndef _uint64_defined
#define _uint64_defined
typedef unsigned long long		uint64;
#endif

#define GPON_MODE 1
#define EPON_MODE 2
#define TIMER_PTHREAD_NO_EXIST 	0
#define TIMER_PTHREAD_EXIST		1


//omci tca id macro define
#define OMCI_CLASS_ID_FEC_PM 			312
#define NO_TCA		0
//#define OMCI_TCA_ID_FEC_PM_CORRECTEDBYTES                         		((OMCI_CLASS_ID_FEC_PM<<16) | 0x1)
//#define OMCI_TCA_ID_FEC_PM_CORRECTEDODEWORDS   				((OMCI_CLASS_ID_FEC_PM<<16) | 0x2)
//#define OMCI_TCA_ID_FEC_PM_UNCORRECTABLECODEWORDS		  	((OMCI_CLASS_ID_FEC_PM<<16) | 0x3)
//#define OMCI_TCA_ID_FEC_PM_FECSECONDS					  	((OMCI_CLASS_ID_FEC_PM<<16) | 0x4)

#define IN 
#define OUT

#define ENABLE 1
#define DISABLE 0

#ifdef FAIL
#undef FAIL
#endif
#define FAIL (-1)

#ifdef TCSUPPORT_CUC
    #define ALARM_SET 1
    #define ALARM_CLR 0
#else
    #define ALARM_SET 0
    #define ALARM_CLR 1
#endif

#define DEFAULT_CYCLE_TIME 900

#define ETH_LAN_PORT_NUM 4


// PONIF Alarm / Warning  define
#define ALARM_ID_PONIF_HIGH_RXPOWER				0x0101
#define ALARM_ID_PONIF_LOW_RXPOWER				0x0102
#define ALARM_ID_PONIF_HIGH_TXPOWER				0x0103
#define ALARM_ID_PONIF_LOW_TXPOWER				0x0104
#define ALARM_ID_PONIF_HIGH_TXCURRENT			0x0105
#define ALARM_ID_PONIF_LOW_TXCURRENT			0x0106
#define ALARM_ID_PONIF_HIGH_SUPPLYVOLTAGE		0x0107
#define ALARM_ID_PONIF_LOW_SUPPLYVOLTAGE		0x0108
#define ALARM_ID_PONIF_HIGH_TEMPRATURE			0x0109
#define ALARM_ID_PONIF_LOW_TEMPRATURE			0x010A

#define WARNING_ID_PONIF_HIGH_RXPOWER			0x010B
#define WARNING_ID_PONIF_LOW_RXPOWER			0x010C
#define WARNING_ID_PONIF_HIGH_TXPOWER			0x010D
#define WARNING_ID_PONIF_LOW_TXPOWER			0x010E
#define WARNING_ID_PONIF_HIGH_TXCURRENT			0x010F
#define WARNING_ID_PONIF_LOW_TXCURRENT			0x0110
#define WARNING_ID_PONIF_HIGH_SUPPLYVOLTAGE	0x0111
#define WARNING_ID_PONIF_LOW_SUPPLYVOLTAGE		0x0112
#define WARNING_ID_PONIF_HIGH_TEMPRATURE		0x0113
#define WARNING_ID_PONIF_LOW_TEMPRATURE		0x0114


#define ALARM_ID_PONIF_DS_DROPEVENTS                   0x0115
#define ALARM_ID_PONIF_US_DROPEVENTS                   0x0116
#define ALARM_ID_PONIF_DS_CRCERRORFRAMES        0x0117
#define ALARM_ID_PONIF_US_CRCERRORFRAMES        0x0118
#define ALARM_ID_PONIF_DS_UNDERSIZEFRAMES       0x0119
#define ALARM_ID_PONIF_US_UNDERSIZEFRAMES      0x011A
#define ALARM_ID_PONIF_DS_OVERSIZEFRAMES          0x011B
#define ALARM_ID_PONIF_US_OVERSIZEFRAMES          0x011C
#define ALARM_ID_PONIF_DS_FRAGMENTS                     0x011D
#define ALARM_ID_PONIF_US_FRAGMENTS                    0x011E
#define ALARM_ID_PONIF_DS_JABBERS                             0x011F
#define ALARM_ID_PONIF_US_JABBERS                             0x0120
#define ALARM_ID_PONIF_DS_DISCARDS                          0x0121
#define ALARM_ID_PONIF_US_DISCARDS                          0x0122
#define ALARM_ID_PONIF_DS_ERRORS                               0x0123
#define ALARM_ID_PONIF_US_ERRORS                              0x0124

#define WARNING_ID_PONIF_DS_DROPEVENTS             0x0125
#define WARNING_ID_PONIF_US_DROPEVENTS             0x0126
#define WARNING_ID_PONIF_DS_CRCERRORFRAMES 0x0127
#define WARNING_ID_PONIF_US_CRCERRORFRAMES                 0x0128
#define WARNING_ID_PONIF_DS_UNDERSIZEFRAMES                0x0129
#define WARNING_ID_PONIF_US_UNDERSIZEFRAMES                0x012A
#define WARNING_ID_PONIF_DS_OVERSIZEFRAMES    0x012B
#define WARNING_ID_PONIF_US_OVERSIZEFRAMES   0x012C
#define WARNING_ID_PONIF_DS_FRAGMENTS              0x012D
#define WARNING_ID_PONIF_US_FRAGMENTS              0x012E
#define WARNING_ID_PONIF_DS_JABBERS                       0x012F
#define WARNING_ID_PONIF_US_JABBERS                       0x0130
#define WARNING_ID_PONIF_DS_DISCARDS                    0x0131
#define WARNING_ID_PONIF_US_DISCARDS                    0x0132
#define WARNING_ID_PONIF_DS_ERRORS                        0x0133
#define WARNING_ID_PONIF_US_ERRORS                        0x0134

#define ALARM_ID_PORT_AUTONEGFAILURE  0x0301
#define ALARM_ID_PORT_LOS			  0x0302
#define ALARM_ID_PORT_FAILURE 		  0x0303
#define ALARM_ID_PORT_LOOPBACK		  0x0304
#define ALARM_ID_PORT_CONGESTION      0x0305

// ETHER PORT Alarm / Warning  define
#define ALARM_ID_PORT_DS_DROPEVENTS                     0x0306
#define ALARM_ID_PORT_US_DROPEVENTS                     0x0307
#define ALARM_ID_PORT_DS_CRCERRORFRAMES          0x0308
#define ALARM_ID_PORT_US_CRCERRORFRAMES          0x0309
#define ALARM_ID_PORT_DS_UNDERSIZEFRAMES         0x030A
#define ALARM_ID_PORT_US_UNDERSIZEFRAMES        0x030B
#define ALARM_ID_PORT_DS_OVERSIZEFRAMES            0x030C
#define ALARM_ID_PORT_US_OVERSIZEFRAMES            0x030D
#define ALARM_ID_PORT_DS_FRAGMENTS                      0x030E
#define ALARM_ID_PORT_US_FRAGMENTS                      0x030F
#define ALARM_ID_PORT_DS_JABBERS                               0x0310
#define ALARM_ID_PORT_US_JABBERS                               0x0311
#define ALARM_ID_PORT_DS_DISCARDS                            0x0312
#define ALARM_ID_PORT_US_DISCARDS                            0x0313
#define ALARM_ID_PORT_DS_ERRORS                                 0x0314
#define ALARM_ID_PORT_US_ERRORS                                0x0315
#define ALARM_ID_PORT_STATUSCHANGETIMES           0x0316

#define WARNING_ID_PORT_DS_DROPEVENTS               0x0317
#define WARNING_ID_PORT_US_DROPEVENTS               0x0318
#define WARNING_ID_PORT_DS_CRCERRORFRAMES    0x0319
#define WARNING_ID_PORT_US_CRCERRORFRAMES   0x031A
#define WARNING_ID_PORT_DS_UNDERSIZEFRAMES  0x031B
#define WARNING_ID_PORT_US_UNDERSIZEFRAMES  0x031C
#define WARNING_ID_PORT_DS_OVERSIZEFRAMES      0x031D
#define WARNING_ID_PORT_US_OVERSIZEFRAMES     0x031E
#define WARNING_ID_PORT_DS_FRAGMENTS                0x031F
#define WARNING_ID_PORT_US_FRAGMENTS                0x0320
#define WARNING_ID_PORT_DS_JABBERS                        0x0321
#define WARNING_ID_PORT_US_JABBERS                        0x0322
#define WARNING_ID_PORT_DS_DISCARDS                      0x0323
#define WARNING_ID_PORT_US_DISCARDS                      0x0324
#define WARNING_ID_PORT_DS_ERRORS                         0x0325
#define WARNING_ID_PORT_US_ERRORS                         0x0326
#define WARNING_ID_PORT_STATUSCHANGETIMES    0x0327

enum EPON_ALARM_MASK
{
	MASK_downstreamDropEvents 		= 1<<0,
	MASK_upstreamDropEvents 			= 1<<1,                                        
      	MASK_downstreamCrcErroredFrames	= 1<<2,                                    
      	MASK_upstreamCrcErroredFrames	= 1<<3,                                        
      	MASK_downstreamUndersizeFrames	= 1<<4,                                    
      	MASK_upstreamUndersizeFrames		= 1<<5,                                                          
      	MASK_downstreamOversizeFrames	= 1<<6,                                                                
      	MASK_upstreamOversizeFrames		= 1<<7,                                                                         
      	MASK_downstreamFragments			= 1<<8,                                                                     
      	MASK_upstreamFragments			= 1<<9,                                                                         
      	MASK_downstreamJabbers			= 1<<10,                                                                                       
      	MASK_upstreamJabbers				= 1<<11,                                                                                                
      	MASK_downstreamDiscards			= 1<<12,                                                                         
      	MASK_upstreamDiscards				= 1<<13,                                                                               
      	MASK_downstreamErrors				= 1<<14,                                                                                    
      	MASK_upstreamErrors				= 1<<15,                                                                           
      	MASK_statusChangeTimes			= 1<<16,         
      	MASK_highRxPower					= 1<<17,
      	MASK_lowRxPower					= 1<<18,
      	MASK_highTxPower					= 1<<19,
      	MASK_lowTxPower					= 1<<20,
      	MASK_highTxCurrent					= 1<<21,
      	MASK_lowTxCurrent					= 1<<22,
      	MASK_highSupplyVoltage				= 1<<23,
      	MASK_lowSupplyVoltage				= 1<<24,
      	MASK_highTemprature				= 1<<25,
      	MASK_lowTemprature				= 1<<26,
      	
      	MASK_ETH_AutoNegFailure			= 1<<27,
      	MASK_ETH_LOS					= 1<<28,
      	MASK_ETH_Failure				= 1<<29,
      	MASK_ETH_Loopback				= 1<<30,
      	MASK_ETH_Congestion				= 1<<31,
      	
};

#define MAX_PORT_NUM 	5
#define ETHER_PORT_1	0
#define ETHER_PORT_2	1
#define ETHER_PORT_3	2
#define ETHER_PORT_4	3
#define PON_IF_PORT		4

#define MAX_PM_BUFFER_SIZE 				64
#define MAX_MAC_BRIDGE_PORT_ITEM_LEN  	8	
#define PMGR_MAC_BRIDGE_PORT_ENTRY          "OMCI_Entry"
#endif
