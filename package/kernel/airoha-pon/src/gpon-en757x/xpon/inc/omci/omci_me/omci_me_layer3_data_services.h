
#ifndef _LAYER3_DATA_SERVICES_ME_H_
#define _LAYER3_DATA_SERVICES_ME_H_
#include "omci_types.h"
#include "omci_me.h"

#define  OMCI_LAYER3_STR_BUFF_SIZE   32
#define  OMCI_LAYER3_ATR_BUFF_SIZE   16
#define  OMCI_LAYER3_VAL_BUFF_SIZE   8


typedef struct __pthreadIPHostArgType{
		uint16 tranCorrelatId;
		uint16 meClassId;
		uint8 msgFormat;		//0x0A:baseline format   0X0B:extended format
		uint32 addr;
		uint8 testType;
}pthreadIPHostArgType;

enum wanIFType{
	WAN_IF_TYPE_IP_ADDR=0,
	WAN_IF_TYPE_NETMASK,
	WAN_IF_TYPE_GATEWAY,
	WAN_IF_TYPE_PRI_DNS,
	WAN_IF_TYPE_SEC_DNS,
	WAN_IF_TYPE_MAC_ADDR,
	WAN_IF_TYPE_ISP,
	WAN_IF_TYPE_VLANID,
	WAN_IF_TYPE_DOT1P,
	WAN_IF_TYPE_IPHOSTID,
};

enum wanServiceListType{
    WAN_SERVICELIST_TR069 = 0,
    WAN_SERVICELIST_VOICE,
    WAN_SERVICELIST_MAX
    
};

typedef struct omciIPhostWanInfo_s
{
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE];
	char netMask[OMCI_LAYER3_STR_BUFF_SIZE];
    char geteWay[OMCI_LAYER3_STR_BUFF_SIZE];
    char priDNS[OMCI_LAYER3_STR_BUFF_SIZE];
    char secDNS[OMCI_LAYER3_STR_BUFF_SIZE];
	char MACAdd[OMCI_LAYER3_STR_BUFF_SIZE];
	char isp[OMCI_LAYER3_VAL_BUFF_SIZE];
    char vlanId[OMCI_LAYER3_VAL_BUFF_SIZE];
    char pBit[OMCI_LAYER3_VAL_BUFF_SIZE];
    char serviceList[OMCI_LAYER3_STR_BUFF_SIZE];
    char ifNode[OMCI_LAYER3_STR_BUFF_SIZE];
    char iphostId[OMCI_LAYER3_VAL_BUFF_SIZE];
}omciIPhostWanInfo_t; 


typedef struct omciIPhostChkTimer_s
{
    int wanType;
    uint16 iphostId;
}omciIPhostChkTimer_t; 

typedef struct omciIPhostCheckWanInfo_s
{
    int wanType;
    char IPHostType[OMCI_LAYER3_STR_BUFF_SIZE];
    char IPHostWan[OMCI_LAYER3_STR_BUFF_SIZE];
    char ifName[OMCI_LAYER3_STR_BUFF_SIZE];
    uint16 IPHostIstId;
    char IPHostTypeNow[OMCI_LAYER3_STR_BUFF_SIZE];
    char wanIf[OMCI_LAYER3_STR_BUFF_SIZE];
    omciIPhostWanInfo_t * oldAttr;
    int  ifSetByOMCI;
    int  ifCreateByOMCI;
}omciIPhostCheckWanInfo_t;

//9.4.1: IP host config data
extern omciAttriDescript_t omciAttriDescriptListIPHostConfigData[];
int32 setIPHostConfigDataIPOptions(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataMACAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataIPAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setIPHostConfigDataIPAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataMask(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setIPHostConfigDataMask(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setIPHostConfigDataGateway(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setIPHostConfigDataPrimaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setIPHostConfigDataSecondaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataCurAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataCurMask(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataCurGateway(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataCurPrimaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataCurSecondaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataDomainName(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostConfigDataHostName(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciMeInitForIPHostConfigData(omciManageEntity_t *omciManageEntity_p);

//9.4.2: IP host performance monitoring history data
extern omciAttriDescript_t omciAttriDescriptListIPHostPM[];
int32 setIPHostPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostPMICMPErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostPMDNSErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostPMIPHostPMDHCPTimeout(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostPMIPHostPMIPConflict(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostPMOutOfMemory(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getIPHostPMInternalError(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciMeInitForIPHostPM(omciManageEntity_t *omciManageEntity_p);

//9.4.3: TCP/UDP config data
extern omciAttriDescript_t omciAttriDescriptListTcpUdpConfigData[];

int omciMeInitForTcpUdpConfigData(omciManageEntity_t *omciManageEntity_p);

int32 setTcpUdpDataIpHostPtrFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
//9.4.4: TCP/UDP performance monitoring history data
extern omciAttriDescript_t omciAttriDescriptListTcpUdpPM[];
int32 setTcpUdpPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTcpUdpPMSocketFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTcpUdpPMListenFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTcpUdpPMBindFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTcpUdpPMAcceptFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTcpUdpPMSelectFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciMeInitForTcpUdpPM(omciManageEntity_t *omciManageEntity_p);
int checkIPHostWanInterfaceTimer(int wanType, uint16 IPHostIstId);
int32 initIPHostWanInterface( uint8 timerValid );


#ifdef TCSUPPORT_VNPTT
int omciMeInitFor250ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor347ME(omciManageEntity_t *omciManageEntity_p);
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
int omciMeInitFor350ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor352ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor353ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor367ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor373ME(omciManageEntity_t *omciManageEntity_p);
#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
int omciMeInitFor370ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor65408ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor65414ME(omciManageEntity_t *omciManageEntity_p);
int omciMeInitFor65425ME(omciManageEntity_t *omciManageEntity_p);
#endif
#endif

#endif

