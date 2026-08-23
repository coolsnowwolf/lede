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
	omci_me.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <stdlib.h>
#include "../../../version/tcversion.h"
#include "omci.h"
#include "omci_me.h"
#include "omci_me_mac_bridge.h"
#include "omci_me_multicast.h"
#include "omci_me_gemport_mapping.h"
#include "omci_me_vlan.h"
#include "omci_me_equipment_management.h"
#include "omci_me_ani_management.h"
#include "omci_me_general_purpose.h"
#include "omci_me_enhanced_security_ctl.h"
#ifdef TCSUPPORT_OMCI_CTC
#include "omci_me_ctc.h"
#endif
#include "omci_me_traffic_management.h"
#include "omci_me_layer3_data_services.h"
#include "omci_me_ethernet_services.h"
#include "omci_me_layer2_data_services.h"
#ifdef TCSUPPORT_VOIP
#include "omci_me_voip.h"
#endif

omciManageEntity_t omciMeTable[] =
{
/*
omciMeIndex, omciMeClassId, omciMeName[32], omciMeInitFunc, omciMeInitFunc, omciActionFunc, omciInternalCreateFunc, omciMeAttriNum, omciAttriDescriptList,needAlarm;omciInst;	
*/
{OMCI_CLASS_ID_ONU_DATA ,  				"ONU data", 		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOnuData, 			{NULL},	 omciAttriDescriptListOnuData, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL},
{OMCI_CLASS_ID_CARDHOLDER ,  			"Cardholder", 		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForCardholder, 		{NULL}, 	 omciAttriDescriptListCardholder, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL},
{OMCI_CLASS_ID_CIRCUIT_PACK , 			"Circuit pack", 	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForCircuitPack , 		{NULL}, 	omciAttriDescriptListCircuitPack , 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
#ifdef TCSUPPORT_VNPTT
{OMCI_CLASS_ID_OLT_G,					"OLT-G",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOltG,				{NULL}, 	omciAttriDescriptListOltG,				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD },
{OMCI_CLASS_ID_NETWORK_ADDRESS, 	"Network address", OMCI_ME_ACCESS_OLT_ONU, OMCI_ME_SUPPORT,  omciMeInitForNetworkAddress,	{NULL}, 	omciAttriDescriptListNetworkAddress,		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD },
{OMCI_CLASS_ID_AUTHENTICATION_METHOD,	"Authentication method",OMCI_ME_ACCESS_OLT_ONU, OMCI_ME_SUPPORT,	omciMeInitForAuthenticationMethod,{NULL},	omciAttriDescriptListAuthenticationMethod,{PTHREAD_MUTEX_INITIALIZER , NULL},NULL, UNNEED_MIB_UPLOAD },
{OMCI_CLASS_ID_LARGE_STRING,			"Large string", 	OMCI_ME_ACCESS_OLT_ONU, OMCI_ME_SUPPORT,	omciMeInitForLargeString,		{NULL}, 	omciAttriDescriptListLargeString,			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , UNNEED_MIB_UPLOAD},
{OMCI_CLASS_ID_ONU_POWER_SHEDDING,	"ONU power shedding",OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForOnuPowerShedding, {NULL},	omciAttriDescriptListOnuPowerShedding,	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD },
#else
{OMCI_CLASS_ID_OLT_G,					"OLT-G",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOltG,				{NULL}, 	omciAttriDescriptListOltG,				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_NETWORK_ADDRESS, 	"Network address", OMCI_ME_ACCESS_OLT_ONU, OMCI_ME_SUPPORT,  omciMeInitForNetworkAddress,	{NULL}, 	omciAttriDescriptListNetworkAddress,		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_AUTHENTICATION_METHOD,	"Authentication method",OMCI_ME_ACCESS_OLT_ONU, OMCI_ME_SUPPORT,	omciMeInitForAuthenticationMethod,{NULL},	omciAttriDescriptListAuthenticationMethod,{PTHREAD_MUTEX_INITIALIZER , NULL},NULL, NORMAL },
{OMCI_CLASS_ID_LARGE_STRING,			"Large string", 	OMCI_ME_ACCESS_OLT_ONU, OMCI_ME_SUPPORT,	omciMeInitForLargeString,		{NULL}, 	omciAttriDescriptListLargeString,			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL},
{OMCI_CLASS_ID_ONU_POWER_SHEDDING,	"ONU power shedding",OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForOnuPowerShedding, {NULL},	omciAttriDescriptListOnuPowerShedding,	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
#endif
{OMCI_CLASS_ID_PROTECTION_PROFILE,		"protection profile", OMCI_ME_ACCESS_ONU, OMCI_ME_UNSUPPORT,	omciMeInitForProtectionProfile, 	{NULL}, 	omciAttriDescriptListProtectionProfile, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_EXTENSION_PACKAGE,		"Extension package",OMCI_ME_ACCESS_ONU, OMCI_ME_UNSUPPORT,	omciMeInitForExtensionPackage, 	{NULL}, 	omciAttriDescriptListExtensionPackage, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_ONU_G ,  				"ONU-G",  		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOnuG,  			{NULL}, 	 omciAttriDescriptListOnuG,				{PTHREAD_MUTEX_INITIALIZER , NULL}, meMapTableOnuG, NORMAL},
{OMCI_CLASS_ID_ONU2_G ,  				"ONU2-G",  		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOnu2G,  			{NULL}, 	 omciAttriDescriptListOnu2G,				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_T_CONT, 				"T-CONT", 		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForTcont, 				{NULL}, 	 omciAttriDescriptListTcont, 				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_ANI_G, 					"ANI-G", 			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForAniG, 				{NULL},	 omciAttriDescriptListAniG, 				{PTHREAD_MUTEX_INITIALIZER , NULL}, meMapTableAniG, NORMAL },
{OMCI_CLASS_ID_UNI_G, 					"UNI-G", 			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForUniG, 				{NULL},	omciAttriDescriptListUniG, 				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_GEM_PORT_PM,			"GEM port PM", 	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForGemPortPM,		{NULL},  omciAttriDescriptListGemPortPm,		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, IS_CLASSICAL_PM_ME },
{OMCI_CLASS_ID_GEM_PORT_CTP, 			"GEM port CTP", 	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForGemPortCtp, 		{NULL},	 omciAttriDescriptListGemPortCtp, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_THRESHOLD_DATA1, 		"Threshold data 1", OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForThresholdData1, 	{NULL},	 omciAttriDescriptListThresholdData1, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_THRESHOLD_DATA2, 		"Threshold data 2", OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForThresholdData2 , 	{NULL}, 	omciAttriDescriptListThresholdData2, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL},
{OMCI_CLASS_ID_PROTECTION_DATA, 		"Protection data", 	OMCI_ME_ACCESS_ONU, OMCI_ME_UNSUPPORT,	omciMeInitForProtectionData  , 	{NULL}, 	 omciAttriDescriptListProtectionData, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL },
{OMCI_CLASS_ID_OMCI, 					"OMCI", 			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOmci, 				{NULL}, 	 omciAttriDescriptListOmci, 				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD},
{OMCI_CLASS_ID_MANAGED_ENTITY, 		"Managed Entity", 	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForManagedEntity, 	{NULL},	 omciAttriDescriptListManagedEntity, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD },
{OMCI_CLASS_ID_ATTRIBUTE	, 			"Attribute", 		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForAttribute, 			{NULL},	 omciAttriDescriptListAttribute, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD  },
{OMCI_CLASS_ID_PORT_MAPPING,			"Port mapping",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForPortMapping, 		{NULL}, 	omciAttriDescriptListPortMapping, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL  },
{OMCI_CLASS_ID_OCTET_STRING,			"Octet string",  	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForOctetString , 		{NULL},	omciAttriDescriptListOctetString, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL  },
{OMCI_CLASS_ID_GENERAL_BUFFER,			"General buffer",  	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForGeneralBuffer , 	{NULL}, 	omciAttriDescriptListGeneralBuffer, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD  },
{OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER,"File transfer controller",OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,omciMeInitForFileTransController, {NULL}, 	omciAttriDescriptListFileTransController, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL  },
{OMCI_CLASS_ID_GENERIC_STATUS,			"Generic status portal",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForGenericStatus, 		{NULL},	omciAttriDescriptListGenericStatus, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
{OMCI_CLASS_ID_ONU_REMOTE_DEBUG ,  	"ONU remote debug",  	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForOnuRemoteDebug,  	{NULL},	omciAttriDescriptListOunRemoteDebug,	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , UNNEED_MIB_UPLOAD },
{OMCI_CLASS_ID_SOFTWARE_IMGAE ,  		"Software image",  	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForSoftwareImage,  	{NULL},	omciAttriDescriptListSoftwareImage,	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
#ifdef TCSUPPORT_SNMP
{OMCI_CLASS_ID_SNMP_CONFIG_DATA,		"SNMP config data",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForSnmpConfigData, 	{NULL},	omciAttriDescriptListSnmpConfigData, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
#endif
#ifdef TCSUPPORT_CWMP
#ifdef TCSUPPORT_VNPTT
{OMCI_CLASS_ID_TR069_MANAGE_SERVER,	"TR-069 management server",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForTr069ManageServer, 	{NULL},	omciAttriDescriptListTr069ManageServer, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, UNNEED_MIB_UPLOAD  },
#else
{OMCI_CLASS_ID_TR069_MANAGE_SERVER,	"TR-069 management server",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForTr069ManageServer, 	{NULL},	omciAttriDescriptListTr069ManageServer, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, NORMAL  },
#endif
#endif
#ifdef TCSUPPORT_OMCI_CTC
{OMCI_CLASS_ID_CTC_ONU_CAPABILITY,		"ONU capability",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForOnuCapability, 			{NULL},	omciAttriDescriptListOnuCapability, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
{OMCI_CLASS_ID_CTC_LOID_AUTHEN,		"LOID authentication",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,		omciMeInitForLoidAuthen, 			{NULL},	omciAttriDescriptListLoidAuthentication, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , UNNEED_MIB_UPLOAD },
{OMCI_CLASS_ID_CTC_EXTENDED_MUTICAST_PROFILE,				"Extended multicast profiles",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,		omciMeInitForExtendMulticastProfile, 			{NULL},	omciAttriDescriptListExtMulticastProfile, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
#endif
{OMCI_CLASS_ID_ONU_E ,  				"ONU-E",  			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForOnuE,  				{NULL},	 omciAttriDescriptListOnuE,				{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
{OMCI_CLASS_ID_ONU_DYNAMIC_POWER ,  	"ONU dynamic power",  	OMCI_ME_ACCESS_ONU, OMCI_ME_UNSUPPORT,omciMeInitForOnuDynamicPower,  {NULL},	 omciAttriDescriptListOnuDynamicPower,	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL },
{OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL,				"Enhanced security control",			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,			omciMeInitForEnhancedSecurityCtl,			{NULL}, 	omciAttriDescriptListEnhancedSecurityCtl,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 	NULL, NORMAL },

{OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM,				"Ethernet frame extended PM",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,            omciMeInitForEthernetFrameExtendedPM ,      {NULL},	    omciAttriDescriptListMBEthnetFrameExtendedPM,       {PTHREAD_MUTEX_INITIALIZER , NULL},     NULL, IS_EXTENDED_PM_ME },


{OMCI_ME_CLASS_ID_MAC_BRIDGE_SERVICE_PROFILE ,	 		"MAC bridge service profile",  		OMCI_ME_ACCESS_OLT,	OMCI_ME_PARTIAL_SUPPORT,	omciMeInitForMACBridgeServiceProfile, 		{NULL},		omciAttriDescriptListMBServiceProfile, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_CON_DATA ,  					"MAC bridge configuration data",	OMCI_ME_ACCESS_ONU,	OMCI_ME_PARTIAL_SUPPORT, 	omciMeInitForMACBridgeConData, 			{NULL},		omciAttriDescriptListMBConnData, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PMHD ,  						"MAC bridge PMHD", 		  	OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForMACBridgePMHD, 				{NULL},	omciAttriDescriptListMBPMHD, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL ,IS_CLASSICAL_PM_ME},
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA ,  			"MAC bridge port config data",		OMCI_ME_ACCESS_OLT,	OMCI_ME_PARTIAL_SUPPORT, 	omciMeInitForMACBridgePortConData, 		{NULL}, 	omciAttriDescriptListMBPortConData, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_DESIGNATION_DATA ,  	"MAC bridge port designate data",	OMCI_ME_ACCESS_ONU,	OMCI_ME_PARTIAL_SUPPORT, 	omciMeInitForMACBridgePortDesignData, 		{NULL}, 	omciAttriDescriptListMBPortDesignData, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA ,  	"MAC bridge port filter data",		OMCI_ME_ACCESS_ONU,	OMCI_ME_SUPPORT, 			omciMeInitForMACBridgePortFilterData, 		{NULL}, 	omciAttriDescriptListMBPortFilterData, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL , NORMAL },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA,	"MAC bridge port filter assign",  	OMCI_ME_ACCESS_ONU,	OMCI_ME_SUPPORT, 			omciMeInitForMACBridgePortFilterPreData, 	{NULL}, 	omciAttriDescriptListMBPortFilterPreData, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL  },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_TABLE_DATA ,  			"MAC bridge port table data", 	OMCI_ME_ACCESS_ONU,	OMCI_ME_SUPPORT, 			omciMeInitForMACBridgePortData, 			{NULL}, 	omciAttriDescriptListMBPortData, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL   },
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD ,  				"MAC bridge port PMHD", 		OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForMACBridgePortPMHD, 			{NULL}, 	omciAttriDescriptListMBPortPMHD, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL,IS_CLASSICAL_PM_ME},
{OMCI_ME_CLASS_ID_VLAN_802_1P ,  							"802.1p mapper profile",  		OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitFor8021pMapperProfile, 			{NULL}, 	omciAttriDescriptList8021pMapperProfile, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL , NORMAL },
{OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA ,  					"vlan tag filter data",			OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForVlanTagFilterData, 			{NULL}, 	omciAttriDescriptListVlanTagFilterData, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL  },
{OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE ,  						"vlan tag operation", 				OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForVlanTagOperation, 			{NULL}, 	omciAttriDescriptListVlanTagOperation, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL },
{OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE ,  				"extend vlan tag operation", 		OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForVlanTagExtendOperation, 		{NULL}, 	omciAttriDescriptListVlanTagExtendOperation,	 	{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL  },
{OMCI_ME_CLASS_ID_MULTICAST_OPERATE_PROFILE ,  				"multicast profile", 				OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitFor_MulticastProfile, 				{NULL}, 	omciAttriDescriptListMulticastProfile,				{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL  },
{OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO ,  		"multicast subscriber cfg info",  	OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForMulticasSubscribertCfgInfo,	{NULL}, 	omciAttriDescriptListMulticasSubscribertCfgInfo, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL  },
{OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_MONITOR ,  		"multicast subscriber monitor", 	OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForMulticastSubscriberMonitor,	{NULL}, 	omciAttriDescriptListMulticastSubscriberMonitor, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL, NORMAL  },
{OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD,  				"ethernet frame up PMHD", 		OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForEthFrameUpPMHD,				{NULL}, 	omciAttriDescriptList_EthFrameUpPMHD, 			{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL,IS_CLASSICAL_PM_ME},
{OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD,  			"ethernet frame down PMHD",	OMCI_ME_ACCESS_OLT,	OMCI_ME_SUPPORT, 			omciMeInitForEthFrameDownPMHD, 			{NULL}, 	omciAttriDescriptListEthFrameDownPMHD, 		{PTHREAD_MUTEX_INITIALIZER , NULL}, 	NULL,IS_CLASSICAL_PM_ME},

{OMCI_CLASS_ID_PRIORITY_QUEUE,						"Priority queue",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForPriorityQueue,			{NULL}, omciAttriDescriptListPriorityQueue,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_TRAFFIC_SCHEDULER,					"Traffic scheduler",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForTrafficScheduler,			{NULL}, omciAttriDescriptListTrafficScheduler,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
#ifdef TCSUPPORT_CHS
{OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR,					"Traffic descriptor",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForTrafficDescriptor,			{NULL}, omciAttriDescriptListTrafficDescriptor,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, UNNEED_MIB_UPLOAD },
#else
{OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR,					"Traffic descriptor",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForTrafficDescriptor,			{NULL}, omciAttriDescriptListTrafficDescriptor,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
#endif

{OMCI_CLASS_ID_GEM_PORT_NETWORK_CTP_PM,			"GEM port network CTP PM",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForGemportNetworkCtpPM,			{NULL}, omciAttriDescriptListGemportNetworkCtpPm,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_ENERGY_CONSUMPTION_PM,			"Energy consumption PM",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForEnergyConsumPM,			{NULL}, omciAttriDescriptListEnergyConsumptionPm,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_GAL_ETHERNET_PROFILE,				"GAL ethernet profile",				OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForGALEthernetProfile,			{NULL}, omciAttriDescriptListGALEthernetProfile,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_GAL_ETHERNET_PM,					"GAL ethernet PM",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForGALEthernetPM,				{NULL}, omciAttriDescriptListGALEthernetPM,				{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_FEC_PM_DATA,						"FEC PM",						OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForFecPM,						{NULL}, omciAttriDescriptListFecPM,						{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_GEM_IWTP,							"GEM IWTP",						OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForGemIWTP,						{NULL}, omciAttriDescriptListGemIWTP,					{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_MULTICAST_GEM_IWTP,				"Multicast GEM IWTP",				OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForMulticastGemIWTP,				{NULL}, omciAttriDescriptListMulticastGemIWTP,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_DOT1_RATE_LIMITER,					"Dot1 rate limiter",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForDot1RateLimiter,				{NULL}, omciAttriDescriptListDot1RateLimiter,				{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
#ifdef TCSUPPORT_OMCI_DOT1AG
{OMCI_CLASS_ID_DOT1AG_MAINTENANCE_DOMAIN,		"Dot1ag maintenance domain",		OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForDot1agMaintenanceDomain,		{NULL}, omciAttriDescriptListDot1agMaintenanceDomain,	{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_DOT1AG_MAINTENANCE_ASSOCIATION,	"Dot1ag maintenance association",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForDot1agMaintenanceAssociation,	{NULL}, omciAttriDescriptListDot1agMaintenanceAssociation,{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
#endif
{OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6,			"MAC bridge port icmpv6",			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForMACBridgePortICMPv6Process,	{NULL}, omciAttriDescriptListMBPortICMPv6Process,	{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_PPTP_ETHERNET_UNI,					"PPTP Ethernet UNI",				OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT, 	omciMeInitForPPTPEthernetUNI,				{NULL},  omciAttriDescriptListPPTPEthernetUNI,			{PTHREAD_MUTEX_INITIALIZER , NULL},	alarmIdTablePPTPEthernetUNI, NORMAL },
{OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA,			"Ethernet PM history data",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForEthernetPMHistoryData,		{NULL}, omciAttriDescriptListEthernetPMHistoryData,		{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_2,		"Ethernet PM history data 2",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForEthernetPMHistoryData2,		{NULL}, omciAttriDescriptListEthernetPMHistoryData2,		{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3,		"Ethernet PM history data 3",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForEthernetPMHistoryData3,		{NULL}, omciAttriDescriptListEthernetPMHistoryData3,		{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME },
{OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT,		"Ethernet frame extended pm 64_bit",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForEthernetFrameExtendedPM64bit,		{NULL}, omciAttriDescriptListEthernetFrameExtendedPM64bit,		{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, IS_EXTENDED_PM_ME },
{OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT,	"Virtual ethernet iface point",			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForVirtualEthernetInterfacePoint,	{NULL}, omciAttriDescriptListVirtualEthernetInterfacePoint,	{PTHREAD_MUTEX_INITIALIZER , NULL},	 alarmIdTableVirtualEthernetInterfacePoint, NORMAL },
{OMCI_CLASS_ID_IP_HOST_CONFIG_DATA,				"IP host config data",				OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForIPHostConfigData,				{NULL}, omciAttriDescriptListIPHostConfigData,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_IP_HOST_PM,							"IP host PM",						OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForIPHostPM,						{NULL}, omciAttriDescriptListIPHostPM,					{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME },
{OMCI_CLASS_ID_TCP_UDP_CFGDATA,					"tcp udp config data",				OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForTcpUdpConfigData,				{NULL}, omciAttriDescriptListTcpUdpConfigData,			{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL, NORMAL },
{OMCI_CLASS_ID_TCP_UDP_PM,							"tcp udp PM",						OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForTcpUdpPM,					{NULL}, omciAttriDescriptListTcpUdpPM,					{PTHREAD_MUTEX_INITIALIZER , NULL},	 NULL,IS_CLASSICAL_PM_ME},
#ifdef TCSUPPORT_VNPTT
{OMCI_CLASS_ID_RESERVED_250_ME, 				"reserved 250 ME",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitFor250ME, 				{NULL}, omciAttriDescriptListReserved250,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_347_ME, 				"reserved 347 ME",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor347ME, 				{NULL}, omciAttriDescriptListReserved347,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
#endif
{OMCI_CLASS_ID_RESERVED_351_PM, 				"reserved 351 PM",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForTcpUdpPM,				{NULL}, omciAttriDescriptListReserved351PM, 			{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,IS_CLASSICAL_PM_ME},

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
{OMCI_CLASS_ID_RESERVED_350_ME, 				"reserved 350 ME",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor350ME,					{NULL}, omciAttriDescriptListReserved350, 				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_352_ME, 				"reserved 352 ME",					OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitFor352ME, 				{NULL}, omciAttriDescriptListReserved352,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,UNNEED_MIB_UPLOAD},
{OMCI_CLASS_ID_RESERVED_353_ME, 				"reserved 353 ME",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor353ME, 				{NULL}, omciAttriDescriptListReserved353,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_367_ME, 				"reserved 367 ME",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor367ME, 				{NULL}, omciAttriDescriptListReserved367,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_373_ME, 				"reserved 373 ME",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor373ME, 				{NULL}, omciAttriDescriptListReserved373,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
{OMCI_CLASS_ID_RESERVED_370_ME, 				"reserved 370 ME",					OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor370ME, 				{NULL}, omciAttriDescriptListReserved370,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_65408_ME, 				"reserved 65408 ME",				OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor65408ME, 				{NULL}, omciAttriDescriptListReserved65408,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_65414_ME, 				"reserved 65414 ME",				OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor65414ME, 				{NULL}, omciAttriDescriptListReserved65414,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
{OMCI_CLASS_ID_RESERVED_65425_ME, 				"reserved 65425 ME",				OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitFor65425ME, 				{NULL}, omciAttriDescriptListReserved65425,				{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
#endif
#endif


#ifdef TCSUPPORT_VOIP
{OMCI_CLASS_ID_SIP_USER_DATA, 		"sip user data",		OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForSipUserData,	{NULL}, omciAttriDescriptListSipUserData,	{PTHREAD_MUTEX_INITIALIZER , NULL},  alarmIdTableSipUserData, NORMAL },
{OMCI_CLASS_ID_VOIP_MEDIA_PROFILE,	"voip media profile",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForVoipMediaPro,	{NULL}, omciAttriDescriptListVoipMediaPro,	{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_VOIP_VOICE_CTP,		"voip voice_ctp",		OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForVoipVoiceCTP,	{NULL}, omciAttriDescriptListVoipVoiceCTP, {PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_MGC_CFG_DATA, 		"MGC config data",		OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForMgcCfgData,	{NULL}, omciAttriDescriptListMgcCfgData,	{PTHREAD_MUTEX_INITIALIZER , NULL},  alarmIdTableMgcCfgData, NORMAL },
{OMCI_CLASS_ID_VOIP_CFG_DATA, 		"voip config data",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForVoipCfgData,	{NULL}, omciAttriDescriptListVoipCfgData,	{PTHREAD_MUTEX_INITIALIZER , NULL},  alarmIdTableVoipCfgData, NORMAL },
{OMCI_CLASS_ID_NETWORK_DIAL_PLAN_TBL,  "network dial plan tbl", OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT, omciMeInitForNetDialPlanTbl, {NULL}, omciAttriDescriptListNetDialPlanTbl, {PTHREAD_MUTEX_INITIALIZER , NULL},	NULL, NORMAL },
{CID_RTP_PROFILE_DATA,              "RTP profile data",OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,    omciMeInitForRtpProfileData, {NULL}, omciAttriDescriptListRtpProfileData, {PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{CID_VOICE_SERVICE_PROFILE,         "Voice service profile",OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,    omciMeInitForVoiceServiceProfile, {NULL}, omciAttriDescriptListVoiceServiceProfile, {PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{CID_VOIP_APP_SVC_PROFILE,          "voip app svc profile",OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForVoipAppSvcProfile,	{NULL}, omciAttriDescriptListVoIPAppSvcPro,	{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_VOIP_LINE_STATUS, 	"VoIP line status",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForVoIPLineStatus,				{NULL}, omciAttriDescriptListVoIPLineStatus,			{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_RTP_PM_HISTORY_DATA,     "Rtp history data PM",                                      OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,  omciMeInitForRtpPMHistoryData,        {NULL},     omciAttriDescriptListRtpPMHistoryData,        {PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, IS_CLASSICAL_PM_ME },
{OMCI_CLASS_SIP_AGENT_PM_HISTORY,       "SIP agent performance monitoring history data",            OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,  omciMeInitForSipAgentPMHistoryData,   {NULL},     omciAttriDescriptListSipAgentPMHistoryData,   {PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, IS_CLASSICAL_PM_ME },
{OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY,   "SIP call initiation performance monitoring history data",  OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,  omciMeInitForSipCallInitPMHistoryData,{NULL},     omciAttriDescriptListSipCallInitPMHistoryData, {PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, IS_CLASSICAL_PM_ME },
{OMCI_ME_CLASS_ID_PPTP_POTS_UNI,	"PPTP POTS UNI", 		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForPptpPotsUNI,				{NULL}, omciAttriDescriptListPptpPotsUNI,			{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA,  "SIP agent config data", OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT, omciMeInitForSIPAgentConfigData, {NULL}, omciAttriDescriptListSIPAgentConfigData, {PTHREAD_MUTEX_INITIALIZER , NULL},  alarmIdTableSIPAgentConfigData, NORMAL },
{CID_CALL_CTRL_PM_HISTORY,          "Call Control PM History",  OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,  omciMeInitForCallCtrlPMHistory,   {NULL}, omciAttriDescriptListCallCtrlPMHistory,  {PTHREAD_MUTEX_INITIALIZER , NULL}, NULL, IS_CLASSICAL_PM_ME },
#ifdef TCSUPPORT_OMCI_ALCATEL
{OMCI_CLASS_ID_VOIP_CALL_STATISTICS,	"VoIP Call statistics", OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMEInitForVoIPCallStatistics,  {NULL}, omciAttriDescriptListVoIPCallStatistics,	{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,	 NORMAL},
#endif
#endif
#ifdef TCSUPPORT_OMCI_ALCATEL
{OMCI_CLASS_ID_CLOCK_DATA_SET,          "Clock Data Set",       OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,    omciMEInitForClockDataSet,        {NULL}, omciAttriDescriptListClockDataSet,        {PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,   NORMAL},
{OMCI_CLASS_ID_PTP_MASTER_CONFIG_DATA_PROFILE,"PTP Master Configure Data Profile",  OMCI_ME_ACCESS_OLT,OMCI_ME_SUPPORT, omciMEInitForPTPMasterConfigDataProfile,{NULL}, omciAttriDescriptListPTPMasterConfigDataProfile,{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,   NORMAL},
{OMCI_CLASS_ID_PTP_Port,                "PTP Port",  OMCI_ME_ACCESS_OLT,OMCI_ME_SUPPORT, omciMEInitForPTPPort,{NULL}, omciAttriDescriptListPTPPort,{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,   NORMAL},
{OMCI_CLASS_ID_TOTAL_GEM_PORT_PM,	"Total GEM port PM",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForTotalGemPortPM, 	{NULL},	omciAttriDescriptListTotalGemPortPm, 	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_ETHERNET_TRAFFIC_PM,	"Ethernet Traffic PM",	OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForEthernetTrafficPM,	{NULL}, omciAttriDescriptListEthernetTrafficPm,	{PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , IS_CLASSICAL_PM_ME},
{OMCI_CLASS_ID_VLAN_TAG_DOWNSTREAM_EGRESS_BEHAVIOR_SUPPLEMENTAL1, "Vlan Tag Downstream Egress Behavior Supplemental 1",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForVlanTagDownstreamSupplemental1, {NULL}, omciAttriDescriptListVlanTagDownstreamSupplemental1, {PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL},
#if defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
{OMCI_CLASS_ID_VLAN_TAG_UPSTREAM_POLICER, "Vlan Tag Upstream Policer",	OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForVlanTagUpstreamPolicer, {NULL}, omciAttriDescriptListVlanTagUpstreamPolicer, {PTHREAD_MUTEX_INITIALIZER , NULL}, NULL , NORMAL},
#endif
{OMCI_CLASS_ID_ONT_OPTICAL_SUPERVISION_STATUS, 	"ONT Optical Supervision Status",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForONTOpticalSupervisionStatus,	{NULL}, omciAttriDescriptListONTOpticalSupervisionStatus,	{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_UNI_SUPPLEMENTAL_1_V2,	"UNI Supplemental 1 V2",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForUNISupplemental1V2,	{NULL}, omciAttriDescriptListUNISupplemental1V2,	{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_ONT_GENERIC_V2,	"ONT Generic V2",		OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForONTGenericV2,	{NULL}, omciAttriDescriptListONTGenericV2,	{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL, NORMAL },
{OMCI_CLASS_ID_GEMPORT_PROTOCOL_MONITORING_HISTORY_DATA_PART_2,		"GEMPort Protocol Monitoring History Data - Part 2",			OMCI_ME_ACCESS_OLT, OMCI_ME_SUPPORT,	omciMeInitForGEMPortProtocolMonitoringHistoryDataPart2,		{NULL}, omciAttriDescriptListGEMPortProtocolMonitoringHistoryDataPart2,		{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,IS_CLASSICAL_PM_ME},
#endif
#ifdef TCSUPPORT_CUC
{OMCI_CLASS_ID_ONU_CAPABILITY,		"ONU Capability",			OMCI_ME_ACCESS_ONU, OMCI_ME_SUPPORT,	omciMeInitForCucOnuCapability,		{NULL}, omciAttriDescriptListCucOnuCapability,		{PTHREAD_MUTEX_INITIALIZER , NULL},  NULL,NORMAL},
#endif
};

omciInterCreateME_t interCreateMETab[MAX_OMCI_MB_INTER_ATTR_NUM] = {
{0, OMCI_ME_CLASS_ID_MAC_BRIDGE_SERVICE_PROFILE, OMCI_ME_CLASS_ID_MAC_BRIDGE_CON_DATA},/*9.3.2 mac bridge configuration data*/
{1, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_DESIGNATION_DATA},/*9.3.5 mac bridge port designation data*/
{2, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA},/*9.3.6 mac bridge port filter table table*/
{3, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA},/*9.3.7 mac bridge port filter preassign table*/
{4, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_TABLE_DATA},/*9.3.8 mac bridge port bridge table data*/
};
//{5, OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6},/*9.3.33 MAC bridge port ICMPv6 process preassign table*/
//{OMCI_INTERNAL_END_TALBE, 0, 0}






int omciGeMeAlarmSize(omciManageEntity_t *meTmp){
	int alarmSize = 0;
	alarm_id_map_entry_t *alarmEntry;
	
	if((meTmp == NULL) || (meTmp->alarmIdMapTable == NULL)){
			return 0;
	}
	
	
	alarmEntry = meTmp->alarmIdMapTable;
	while(alarmEntry->alarmId !=0){
		alarmEntry++;
		alarmSize++;
	}
	
	return alarmSize;
		
	
}



int omciGetMeTableSize(void){
	return sizeof(omciMeTable)/sizeof(omciManageEntity_t);
}

/*omciMeTableInit
description:
	init ME table call back function and var

*/
int omciMeTableInit(void){
	uint16 meIndex;
	omciManageEntity_t *tmpOmciManageEntity_p = omciMeTable;
	int ret;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n Enter omciMeTableInit max=%d", sizeof(omciMeTable)/sizeof(omciManageEntity_t));

	for(meIndex = 0;meIndex < omci.meTableSize ; meIndex++ ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n omciMeTableInit tmpOmciManageEntity_p =0x%x , meIndex= %d\r\n", (uint32)tmpOmciManageEntity_p, meIndex);
		if(tmpOmciManageEntity_p->omciMeInitFunc != NULL)
			tmpOmciManageEntity_p->omciMeInitFunc(tmpOmciManageEntity_p );
		ret = pthread_mutex_init(&(tmpOmciManageEntity_p->omciInst.omciInstLock), NULL);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciMeTableInit pthread_mutex_init fail! meIndex=%d", meIndex);
		}
		tmpOmciManageEntity_p++;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n omciMeTableInit finished\r\n");
	return 0;	
}

int omciInitCapability(void)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
	int8 allPortsNum = 0;
	char attrName[25] = {0};
	int i = 0;
	int list = 0;
	int portId[8] = {0};
	char serialNumber[8] = {0};
	int tmp = 0;
	pSystemCap = &omci.omciCap.systemCap;
	pOnuCap = &omci.omciCap.onuCap;
	pAniCap = &omci.omciCap.aniCap;
    const uint8 timerValid = 0;

	memset(&omci.omciCap, 0, sizeof(omci.omciCap));
/*system capability init */
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "MaxPorts", buffer);
	allPortsNum = (uint8)atoi(buffer);
	for (list = 1; allPortsNum > 0; allPortsNum -= MAP_PORTS_NUM, list ++)
	{
		memset(buffer, 0, sizeof(buffer));
		memset(attrName, 0, sizeof(attrName));
		sprintf(attrName, "PortList%d", list);
		tcapi_get("GPON_ONU", attrName, buffer);
		sscanf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d", &portId[0], &portId[1], &portId[2], &portId[3], &portId[4], &portId[5],&portId[6], &portId[7]);
			
		for (i = 0; i < MAP_PORTS_NUM; i++)
		{
			if (portId[i] == UNIT_TYPE_GPON24881244)
				pSystemCap->gponNum ++;
			else if (portId[i] == UNIT_TYPE_BASET_10100)
				pSystemCap->feNum ++;
			else if (portId[i] == UNIT_TYPE_BASET_101001000)
				pSystemCap->geNum ++;
			else if (portId[i] == UNIT_TYPE_VEIP)
				pSystemCap->veipNum ++;
			else if (portId[i] == UNIT_TYPE_POTS)
				pSystemCap->potsNum ++;
		}
	}

	/*onu type */
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "OnuType", buffer);
	pSystemCap->onuType = atoi(buffer);
	
	if (pSystemCap->onuType != 2){
		pSystemCap->veipNum = 0;
	}
#if !defined(TCSUPPORT_HGU_OMCI_MIB_UPLOAD_PPTP_ETH_UNI)
    else{
        pSystemCap->feNum = 0;
        pSystemCap->geNum = 0;
    }
#endif

/*onu capability init*/
	//sn
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "SerialNumber", buffer);
	if(strlen(buffer) == 16){
		for (i = 0; i < 8; i++)
		{
			memset(serialNumber, 0, sizeof(serialNumber));
			memcpy(serialNumber, buffer + 2 *i , 2);
			sscanf(serialNumber, "%x", &tmp);
			*(pOnuCap->sn + i) = (uint8) tmp;
		}
	}else{
		memcpy(pOnuCap->sn, buffer, 4);
		for (i = 0; i < 4; i++)
		{
			memset(serialNumber, 0, sizeof(serialNumber));
			memcpy(serialNumber, buffer + 4 + 2 *i , 2);
			sscanf(serialNumber, "%x", &tmp);
			*(pOnuCap->sn + 4 + i) = (uint8) tmp;
		}
	}

	//version
	memset(buffer, 0, sizeof(buffer));
#if !defined(TCSUPPORT_CUC) && !defined(TCSUPPORT_CT) 
	tcapi_get("GPON_ONU", "Version", buffer);
#endif
	memcpy(pOnuCap->version, buffer, OMCI_VERSION_LENS);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:version = %s\n", pOnuCap->version);

	memset(buffer, 0, sizeof(buffer));
	tcapi_get("XPON_LinkCfg", PON_LINK_CFG_ATTR_XPON_MODE, buffer);
	pOnuCap->linkMode= atoi(buffer);

	memset(buffer, 0, sizeof(buffer));
	tcapi_get("VoIPBasic_Common", "VoIPConfig", buffer);
	pOnuCap->VoIPbyTR069 = (0 == strcmp(buffer,"TR69")) ? 1 : 0;
        

	//traffic management option
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "TMOption", buffer);
	pOnuCap->tmOption = atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:TMOption = %d\n", pOnuCap->tmOption);

	//onu survival time
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "ONUSurvivalTime", buffer);
	pOnuCap->onuSurvivalTime = atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:ONUSurvivalTime = %d\n", pOnuCap->onuSurvivalTime);

	//extended tc layer
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "ExtendedTClayer", buffer);
	pOnuCap->extendedTcLayer = atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:ExtendedTClayer = %d\n", pOnuCap->extendedTcLayer);

	//equipment id
	memset(buffer, 0, sizeof(buffer));
#if !defined(TCSUPPORT_CUC) && !defined(TCSUPPORT_CT) 
	tcapi_get("GPON_ONU", "EquipmentId", buffer);
#endif

	memcpy(pOnuCap->equipmentId, buffer, OMCI_EQUIPMENTID_LENS);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:EquipmentId = %s\n", pOnuCap->equipmentId);

	//omcc version
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "OMCCVersion", buffer);
	pOnuCap->omccVersion = strtol(buffer, (char**)NULL, 16);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:OMCCVersion = %x\n", pOnuCap->omccVersion);

	//vendor product code
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "VendorProCode", buffer);
	pOnuCap->vendorProCode = atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:VendorProCode = %d\n", pOnuCap->vendorProCode);

	//security capability
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "SecurityCap", buffer);
	pOnuCap->securityCap = (uint8)strtol(buffer, (char**)NULL, 16);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:SecurityCap = %x\n", pOnuCap->securityCap);

	//total priority queue number
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "TotalPQNum", buffer);
	pOnuCap->totalPQNum = (uint16)atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:TotalPQNum = %d\n", pOnuCap->totalPQNum);

	//total traffic scheduler number
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "TotalTSNum", buffer);
	pOnuCap->totalTSNum = (uint8)atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:TotalTSNum = %d\n", pOnuCap->totalTSNum);

	//total gemport number
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "TotalGEMPortNum", buffer);
	pOnuCap->totalGEMPortNum = (uint16)atoi(buffer);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:TotalGEMPortNum = %d\n", pOnuCap->totalGEMPortNum);

	//connectivity capability
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "ConnectivityCap", buffer);
	pOnuCap->connectivityCap = (uint16)strtol(buffer, (char**)NULL, 16);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:TotalGEMPortNum = %x\n", pOnuCap->connectivityCap);

	//qos configure flex
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ONU", "QosConfFlex", buffer);
	pOnuCap->qoSConfFlex = (uint16)strtol(buffer, (char**)NULL, 16);	
	pOnuCap->qoSConfFlex &= 0x3F;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:QoSConfFlex = %x\n", pOnuCap->qoSConfFlex);

/*ani capability init*/
	//state report indication
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ANI", "SRIndication", buffer);
	pAniCap->srIndication = (uint8)atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:SRIndication = %x\n", pAniCap->srIndication);

	//total tcont number
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ANI", "TotalTCONTNum", buffer);
	pAniCap->totalTcontNum = (uint16)atoi(buffer);
    if((CONFIG_GPON_MAX_TCONT < pAniCap->totalTcontNum)||(0 >= pAniCap->totalTcontNum)){
        pAniCap->totalTcontNum = CONFIG_GPON_MAX_TCONT-1;
    }
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:TotalTCONTNum = %d\n", pAniCap->totalTcontNum);

	//gem block length
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ANI", "GEMBlockLength", buffer);
	pAniCap->gemBlockLength = (uint16)atoi(buffer);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:GEMBlockLength = %d\n", pAniCap->gemBlockLength);

	//piggback DBA
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ANI", "PiggybackDBA", buffer);
	pAniCap->piggybackDBA = atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:PiggybackDBA = %d\n", pAniCap->piggybackDBA);

	//onu response time
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("GPON_ANI", "ONUResponseTime", buffer);
	pAniCap->onuResponseTime = (uint16)atoi(buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitCapability:ONUResponseTime = %d\n", pAniCap->onuResponseTime);
    
#if defined(TCSUPPORT_PON_IP_HOST)
    initIPHostWanInterface(timerValid);
#endif
	return 0;
}

/*omciInitInst
description: auto create instance when omci module init
*/
int  omciInitInst(void){
	int ret=0;
	omci.omciAlarmSeq = 0;
	omci.omciDataSync = 0;

/*ONU automatically  create OMCI_CLASS_ID_ONU_G one instance  */
	omciInitInstForOnuG();

/*ONU automatically  create OMCI_CLASS_ID_ONU2_G one instance  */
	omciInitInstForOnu2G();

/*ONU automatically  create OMCI_CLASS_ID_ONU_DATA one instance  */
	omciInitInstForOnuData();

/*ONU automatically  create OMCI_CLASS_ID_CARDHOLDER 4 instances  */
	omciInitInstForCardholder();

/*ONU automatically  create OMCI_CLASS_ID_CIRCUIT_PACK 4 instance  */
	omciInitInstForCircuitPack();

/*ONU automatically  create OMCI_CLASS_ID_ONU_POWER_SHEDDING one instance  */
	omciInitInstForOnuPowerShedding();


/*ONU automatically  create OMCI_CLASS_ID_PORT_MAPPING one instance  */
//	omciInitInstForPortMapping();

	if (pSystemCap->gponNum != 0)
	{
/*ONU automatically  create OMCI_CLASS_ID_ONU2_G one instance  */
		omciInitInstForAniG();
	}

/*ONU automatically  create OMCI_CLASS_ID_T_CONT 15 instance  */
	omciInitInstForTcont();

/* each one instance for PPTP UNI or VEIP  VEIP meId != PPTP UNI meId*/	
/*when PPTP instance create, the UNI-G instance create*/
//	omciInitInstForUniG(2);

/*ONU automatically  create OMCI_CLASS_ID_OLT_G one instance  */
	omciInitInstForOltG();

#if 1
/*ONU automatically  create OMCI_CLASS_ID_ATTRIBUTE one instance  for each attribute*/
	omciInitInstForAttribute();	

/*ONU automatically  create OMCI_CLASS_ID_MANAGED_ENTITY one instance  for each ME*/
	omciInitInstForManagedEntity();

/*ONU automatically  create OMCI_CLASS_ID_OMCI one instance  */
	omciInitInstForOmci();

/*ONU automatically  create OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER one instance  */
	omciInitInstForFileTransController();
#endif

/*ONU automatically  create OMCI_CLASS_ID_ONU_REMOTE_DEBUG one instance  */
	ret = omciInitInstForOnuRemoteDebug();
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_ONU_REMOTE_DEBUG fail\n");

/*create software image instance 0 and 1*/
	ret = omciInitInstForSoftwareImage();
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_SOFTWARE_IMGAE fail\n");
	
#ifdef TCSUPPORT_OMCI_CTC
/*create CTC ONU Capabilityinstance*/
	ret = omciInitInstForOnuCapability();
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_CTC_ONU_CAPABILITY fail\n");
		
/*create CTC LOID authentication instance*/
	ret = omciInitInstForLoidAuthen();
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_CTC_LOID_AUTHEN fail\n");
		
#endif
#ifdef TCSUPPORT_VOIP
/*create 9.9.18 VoIP config data instance*/
	ret = omciInitInstForVoipCfgData();
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_VOIP_CFG_DATA fail\n");
#endif
#if 0
/*ONU automatically  create OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL one instance  */
	ret = omciInitInstForEnhancedSecurityCtl();
	if(ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL fail\n");
	else
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL success \n");
#endif	

	/*ONU automatically  create OMCI_CLASS_ID_PPTP_ETHERNET_UNI one instance  */
	if (pSystemCap->feNum != 0 || pSystemCap->geNum != 0)
	{
		ret = omciInitInstForPPTPEthernetUNI();
		if(ret == -1)
			OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_PPTP_ETHERNET_UNI fail\n");
	}

	if (pSystemCap->veipNum != 0)
	{
		/*ONU automatically  create OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT one instance  */
		ret = omciInitInstForVirtualEthernetInterfacePoint();
		if(ret == -1)
			OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT fail\n");

#ifdef TCSUPPORT_CWMP
	/*create TR069 management server instance*/
		ret = omciInitInstForTr069ManageServer();
		if (ret == -1)
			OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_TR069_MANAGE_SERVER fail\n");
#endif
	}

#if 0
	/*ONU automatically  create OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6_PROCESS one instance  */
	ret = omciInitInstForMACBridgePortICMPv6Process();
	if(ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6_PROCESS fail\n");
	else
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6_PROCESS success \n");
	
#endif
#ifdef TCSUPPORT_VOIP
	/*ONU automatically  create OMCI_ME_CLASS_ID_PPTP_POTS_UNI one instance	*/
	if(pSystemCap->potsNum != 0)
	{
		ret = omciInitInstForPPTPPotsUNI();
		if(ret == -1)
			OMCI_ERROR("omciInitInst create OMCI_ME_CLASS_ID_PPTP_POTS_UNI fail\n");
	}
#endif	

	/*ONU automatically  create OMCI_CLASS_ID_IP_HOST_CONFIG_DATA one instance  */
	ret = omciInitInstForIPHostConfigData();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_IP_HOST_CONFIG_DATA fail\n");

	ret = omciInitInstForPriorityQueue();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_PRIORITY_QUEUE fail\n");

/*ONU automatically  create OMCI_CLASS_ID_TRAFFIC_SCHEDULER one instance  */
	ret = omciInitInstForTrafficScheduler();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_TRAFFIC_SCHEDULER fail\n");
	
#ifdef TCSUPPORT_OMCI_ALCATEL
	/*ONU automatically  create OMCI_CLASS_ID_ONT_OPTICAL_SUPERVISION_STATUS one instance  */
	ret = omciInitInstForONTOpticalSupervisionStatus();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_ONT_OPTICAL_SUPERVISION_STATUS fail\n");

	/*ONU automatically  create OMCI_CLASS_ID_ONT_GENERIC_V2 one instance  */
	ret = omciInitInstForONTGenericV2();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_ONT_GENERIC_V2 fail\n");
#endif

#ifdef TCSUPPORT_CUC
    ret = omciInitInstForCucOnuCapability();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create OMCI_CLASS_ID_ONU_CAPABILITY fail\n");
#endif

#ifdef TCSUPPORT_VNPTT
    ret = omciInitInstForRsv347ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv347ME fail\n");
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
	ret = omciInitInstForRsv350ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv350ME fail\n");
	ret = omciInitInstForRsv352ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv352ME fail\n");

	ret = omciInitInstForRsv353ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv353ME fail\n");

	ret = omciInitInstForRsv367ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv367ME fail\n");

	ret = omciInitInstForRsv373ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv373ME fail\n");
	
#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
	ret = omciInitInstForRsv370ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv370ME fail\n");

	ret = omciInitInstForRsv65408ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv65408ME fail\n");
	
	ret = omciInitInstForRsv65414ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv65414ME fail\n");

	ret = omciInitInstForRsv65425ME();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create Rsv65425ME fail\n");
#endif
#endif

#ifdef TCSUPPORT_CHS
	ret = omciInitInstForTrafficDescriptor();
	if(ret == -1)
		OMCI_ERROR("omciInitInst create omciInitInstForTrafficDescriptor fail\n");

#endif


	return 0;
}
