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
	oam_ctc_alarm.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_ALARM_NODE_H_
#define OAM_ALARM_NODE_H_
#include "epon_oam_types.h"
#include "ctc/oam_ctc_event.h"

#define OAM_MAX_ALARM_NAME_LEN 64

/* Alarm or warning for ONU */
#define ALARM_ID_EQUIPMENT            	0x0001
#define ALARM_ID_POWER                	0x0002
#define ALARM_ID_BATTERYMISSING       	0x0003
#define ALARM_ID_BATTERYFAILURE       	0x0004
#define ALARM_ID_BATTERYVOLTLOW       	0x0005
#define ALARM_ID_PHYSICALINTRUSION    	0x0006
#define ALARM_ID_ONUSELFTESTFAILURE   	0x0007
#define ALARM_ID_ONUTEMPHIGH          	0x0009
#define ALARM_ID_ONUTEMPLOW           	0x000A
#define ALARM_ID_IADCONNECTIONFAILURE 	0x000B
#define ALARM_ID_PON_IFSWITCH         	0x000C
#define ALARM_ID_SLEEPSTATUSUPDATE    	0x000D

#define ALARM_ID_ONU_USED_MAX           0x000E
#define ALARM_ID_ONU_MAX                0x00FF

/* Alarm or warning for PON IF */
#define ALARM_ID_RXPOWERHIGH          	0x0101
#define ALARM_ID_RXPOWERLOW           	0x0102
#define ALARM_ID_TXPOWERHIGH          	0x0103
#define ALARM_ID_TXPOWERLOW           	0x0104
#define ALARM_ID_TXBIASHIGH           	0x0105
#define ALARM_ID_TXBIASLOW            	0x0106
#define ALARM_ID_VCCHIGH              	0x0107
#define ALARM_ID_VCCLOW               	0x0108
#define ALARM_ID_TEMPHIGH             	0x0109
#define ALARM_ID_TEMPLOW              	0x010a
#define WARING_ID_RXPOWERHIGH         	0x010B
#define WARING_ID_RXPOWERLOW          	0x010C
#define WARING_ID_TXPOWERHIGH         	0x010D
#define WARING_ID_TXPOWERLOW          	0x010E
#define WARING_ID_TXBIASHIGH          	0x010F
#define WARING_ID_TXBIASLOW           	0x0110
#define WARING_ID_VCCHIGH             	0x0111
#define WARING_ID_VCCLOW              	0x0112
#define WARING_ID_TEMPHIGH            	0x0113
#define WARING_ID_TEMPLOW             	0x0114

#define ALARM_ID_PONIF_DS_DROPEVENTS     	0x0115
#define ALARM_ID_PONIF_US_DROPEVENTS     	0x0116
#define ALARM_ID_PONIF_DS_CRCERRORFRAMES 	0x0117
#define ALARM_ID_PONIF_US_CRCERRORFRAMES 	0x0118
#define ALARM_ID_PONIF_DS_UNDERSIZEFRAMES	0x0119
#define ALARM_ID_PONIF_US_UNDERSIZEFRAMES	0x011A
#define ALARM_ID_PONIF_DS_OVERSIZEFRAMES 	0x011B
#define ALARM_ID_PONIF_US_OVERSIZEFRAMES 	0x011C
#define ALARM_ID_PONIF_DS_FRAGMENTS      	0x011D
#define ALARM_ID_PONIF_US_FRAGMENTS      	0x011E
#define ALARM_ID_PONIF_DS_JABBERS        	0x011F
#define ALARM_ID_PONIF_US_JABBERS        	0x0120
#define ALARM_ID_PONIF_DS_DISCARDS       	0x0121
#define ALARM_ID_PONIF_US_DISCARDS       	0x0122
#define ALARM_ID_PONIF_DS_ERRORS         	0x0123
#define ALARM_ID_PONIF_US_ERRORS         	0x0124

#define WARNING_ID_PONIF_DS_DROPEVENTS     	0x0125
#define WARNING_ID_PONIF_US_DROPEVENTS     	0x0126
#define WARNING_ID_PONIF_DS_CRCERRORFRAMES 	0x0127
#define WARNING_ID_PONIF_US_CRCERRORFRAMES 	0x0128
#define WARNING_ID_PONIF_DS_UNDERSIZEFRAMES	0x0129
#define WARNING_ID_PONIF_US_UNDERSIZEFRAMES	0x012A
#define WARNING_ID_PONIF_DS_OVERSIZEFRAMES 	0x012B
#define WARNING_ID_PONIF_US_OVERSIZEFRAMES 	0x012C
#define WARNING_ID_PONIF_DS_FRAGMENTS      	0x012D
#define WARNING_ID_PONIF_US_FRAGMENTS      	0x012E
#define WARNING_ID_PONIF_DS_JABBERS        	0x012F
#define WARNING_ID_PONIF_US_JABBERS        	0x0130
#define WARNING_ID_PONIF_DS_DISCARDS       	0x0131
#define WARNING_ID_PONIF_US_DISCARDS       	0x0132
#define WARNING_ID_PONIF_DS_ERRORS         	0x0133
#define WARNING_ID_PONIF_US_ERRORS         	0x0134

#define ALARM_ID_PONIF_USED_MAX           0x0135
#define ALARM_ID_PONIF_MAX                0x01FF

/* Alarm or warning for Port */
#define ALARM_ID_ETHPORT_AUTONEGFAILURE	0x0301
#define ALARM_ID_ETHPORT_LOS           	0x0302
#define ALARM_ID_ETHPORT_FAILURE       	0x0303
#define ALARM_ID_ETHPORT_LOOPBACK      	0x0304
#define ALARM_ID_ETHPORT_CONGESTION    	0x0305

#define ALARM_ID_PORT_DS_DROPEVENTS     	0x0306
#define ALARM_ID_PORT_US_DROPEVENTS     	0x0307
#define ALARM_ID_PORT_DS_CRCERRORFRAMES 	0x0308
#define ALARM_ID_PORT_US_CRCERRORFRAMES 	0x0309
#define ALARM_ID_PORT_DS_UNDERSIZEFRAMES	0x030A
#define ALARM_ID_PORT_US_UNDERSIZEFRAMES	0x030B
#define ALARM_ID_PORT_DS_OVERSIZEFRAMES 	0x030C
#define ALARM_ID_PORT_US_OVERSIZEFRAMES 	0x030D
#define ALARM_ID_PORT_DS_FRAGMENTS      	0x030E
#define ALARM_ID_PORT_US_FRAGMENTS      	0x030F
#define ALARM_ID_PORT_DS_JABBERS        	0x0310
#define ALARM_ID_PORT_US_JABBERS        	0x0311
#define ALARM_ID_PORT_DS_DISCARDS       	0x0312
#define ALARM_ID_PORT_US_DISCARDS       	0x0313
#define ALARM_ID_PORT_DS_ERRORS         	0x0314
#define ALARM_ID_PORT_US_ERRORS         	0x0315
#define ALARM_ID_PORT_STATUSCHANGETIMES 	0x0316

#define WARNING_ID_PORT_DS_DROPEVENTS     	0x0317
#define WARNING_ID_PORT_US_DROPEVENTS     	0x0318
#define WARNING_ID_PORT_DS_CRCERRORFRAMES 	0x0319
#define WARNING_ID_PORT_US_CRCERRORFRAMES 	0x031A
#define WARNING_ID_PORT_DS_UNDERSIZEFRAMES	0x031B
#define WARNING_ID_PORT_US_UNDERSIZEFRAMES	0x031C
#define WARNING_ID_PORT_DS_OVERSIZEFRAMES 	0x031D
#define WARNING_ID_PORT_US_OVERSIZEFRAMES 	0x031E
#define WARNING_ID_PORT_DS_FRAGMENTS      	0x031F
#define WARNING_ID_PORT_US_FRAGMENTS      	0x0320
#define WARNING_ID_PORT_DS_JABBERS        	0x0321
#define WARNING_ID_PORT_US_JABBERS        	0x0322
#define WARNING_ID_PORT_DS_DISCARDS       	0x0323
#define WARNING_ID_PORT_US_DISCARDS       	0x0324
#define WARNING_ID_PORT_DS_ERRORS         	0x0325
#define WARNING_ID_PORT_US_ERRORS         	0x0326
#define WARNING_ID_PORT_STATUSCHANGETIMES 	0x0327

#define ALARM_ID_ETHPORT_USED_MAX           0x0328
#define ALARM_ID_PORT_MAX                0x03FF


#define ALARM_ID_POTSPORTFAILURE      	0x0401
#define ALARM_ID_E1PORTFAILURE        	0x0501
#define ALARM_ID_E1TIMINGUNLOCK       	0x0502
#define ALARM_ID_E1LOS                	0x0503

#define ALARM_ENABLE  1
#define ALARM_DISABLE 0

#define ALARM_STATE_SEND  (1<<8)

#define REQ_GET  0x00
#define REQ_SET  0x01

/* OAM Alarm Node struct, every alarm node has one item in oamAlarmTable */
typedef struct oamAlarmNode_s
{
	char	name[OAM_MAX_ALARM_NAME_LEN];
	u_short	alarmId;
	u_char   length;  // alarm info length
	int (*oamStatus)(
			OamEventStatusEntry_Ptr esep,
			u_char	req_type	/*GetRequest or SetRequest*/
		);
	int (*oamThreshold)(
			OamEventThresholdEntry_Ptr ethep,
			u_char	req_type
		);

}OamAlarmNode_t, *OamAlarmNode_Ptr;

void oamDisableAllAlarm();

OamAlarmNode_Ptr findAlarmNodeById(u_short alarmId);

/*  The perforemance counter use the default status&threshold function */
#ifdef TCSUPPORT_PMMGR
int oamStatusPMCounter(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdPMCounter(OamEventThresholdEntry_Ptr ethep, u_char req_type);
#endif

int oamStatusSleepStatusUpdate(OamEventStatusEntry_Ptr esep, u_char req_type);

int oamStatusEquipment(OamEventStatusEntry_Ptr esep, u_char req_type);

int oamStatusONUSelfTestFailure(OamEventStatusEntry_Ptr esep, u_char req_type);

int oamStatusONUTempHigh(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdONUTempHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusONUTempLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdONUTempLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);


int oamStatusPON_IFSwitch(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdPON_IFSwitch(OamEventThresholdEntry_Ptr ethep, u_char req_type);

/*
int oamStatusRXPowerHigh(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdRXPowerHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusRXPowerLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdRXPowerLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXPowerHigh(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXPowerHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXPowerLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXPowerLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXBiasHigh(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXBiasHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type);


int oamStatusTXBiasLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXBiasLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusVccHigh(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdVccHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusVccLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdVccLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTempHigh(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTempHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTempLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTempLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusRXPowerHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdRXPowerHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusRXPowerLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdRXPowerLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXPowerHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXPowerHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXPowerLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXPowerLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXBiasHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXBiasHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTXBiasLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTXBiasLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusVccHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdVccHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusVccLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdVccLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTempHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTempHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusTempLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdTempLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusEthPortAutoNegFailure(OamEventStatusEntry_Ptr esep, u_char req_type);

int oamStatusEthPortLOS(OamEventStatusEntry_Ptr esep, u_char req_type);

int oamStatusEthPortFailure(OamEventStatusEntry_Ptr esep, u_char req_type);

int oamStatusEthPortCongestion(OamEventStatusEntry_Ptr esep, u_char req_type);
*/

int oamStatusEthPortLoopback(OamEventStatusEntry_Ptr esep, u_char req_type);


// * ----------These alarms or warnings don't supported by our system.----------//
int oamStatusIADConnectionFailure(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdIADConnectionFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);

int oamStatusPower(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusBatteryMissing(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusBatteryFailure(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusBatteryVoltLow(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamThresholdBatteryVoltLow(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamStatusPhysicalIntrusion(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusPOTSPortFailure(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusE1PortFailure(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusE1TimingUnlock(OamEventStatusEntry_Ptr esep, u_char req_type);
int oamStatusE1LOS(OamEventStatusEntry_Ptr esep, u_char req_type);
// * /

/* ----------------These alarms or warnings have no threshold.----------------//
int oamThresholdEquipment(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdPower(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdBatteryMissing(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdBatteryFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdPhysicalIntrusion(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdONUSelfTestFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdEthPortAutoNegFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdEthPortLOS(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdEthPortFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdEthPortLoopback(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdEthPortCongestion(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdPOTSPortFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdE1PortFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdE1TimingUnlock(OamEventThresholdEntry_Ptr ethep, u_char req_type);
int oamThresholdE1LOS(OamEventThresholdEntry_Ptr ethep, u_char req_type);
*/

#endif
