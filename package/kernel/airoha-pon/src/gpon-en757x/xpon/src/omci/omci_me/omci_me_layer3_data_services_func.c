

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_layer3_data_services.h"
#include "omci_me_mac_bridge.h"
#include "omci_me_vlan.h"

/*******************************************************************************************************************************
									C O N S T A N T S
										
********************************************************************************************************************************/

#define IP_OPTION_DHCP_MASK		(1<<0)
#define IP_OPTION_ENABLE_PING_RESPONSE_MASK (1<<1)
#define IP_OPTION_ENABLE_TRACERT_RESPONSE_MASK (1<<2)
#define IP_OPTION_ENABLE_IPSTACK_MASK (1<<3)
#define IP_HOST_MAC_PATH "/etc/mac.conf"

#define IP_HOST_PING_TEST			0x1
#define IP_HOST_TRACEROUTE_TEST	0x2
#define IP_HOST_VENDOR_TEST 		0x8
#define IP_HOST_TEST_PING_PATH		"/tmp/omci/ipHostPingTest_stat"
#define IP_HOST_TEST_TRACEROUTE_PATH	"/tmp/omci/ipHostTracerouteTest_stat"
#define MAX_BUF_LEN 		1024
#define IMCOMPLETE_STR		"0 packets received"
#define ICMP_SEQ0			"icmp_seq=0"
#define ICMP_SEQ1			"icmp_seq=1"
#define ICMP_SEQ2			"icmp_seq=2"
#define ICMP_SEQ3			"icmp_seq=3"

#define MAX_ARG_NUM	3
#define MAX_NODE_NAME 16
#define WAN	"Wan"
#define WAN_PVC "PVC"
#define IPHOST_NODE "XPON_IPHost%d"
#define COMMIT_WAN_ATTR "commitWan"
#if !defined(TCSUPPORT_CMCC) 
#define PVC_NUM 8
#endif

#define TMP_IF_PATH "/tmp/wanInfo"
#define MAX_SMUX_NUM 8
#define WAN_INTERFACE "Entry"

#if !defined(TCSUPPORT_CT) 
#define WANNODE_FORMAT "Wan_PVC%d"
#define IFNAME_FORMAT "nas%d"
#define WANDEV_INFO_FORMAT "DeviceInfo_PVC%d"
#endif

#define OMCI_INVALID_IPHOST_ID "0xFF"
omciIPhostWanInfo_t omciIPHostWanInfo[2]; 
omciIPhostWanInfo_t omciOldTR069Info; 
omciIPhostWanInfo_t omciOldVOICEInfo; 
/*******************************************************************************************************************************
									FUNCTIONS
										
********************************************************************************************************************************/

int checkInternalClientIp(char *st);
int checkSubnetLegal(char *mask);
int omciInternalCreatActionForIPHostConfigData(uint16 classId, uint16 instanceId);
int omciTestActionForIPHostConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciIPHostCfgDataTestRstMsgHandle(pthreadIPHostArgType *pthreadArg);
int omciIPHostCfgDataTestRstMsgHandle_Extended(pthreadIPHostArgType *pthreadArg);
void startTestForIPHost(void* arg);
void fileRead(char *path, char *buf, int size);
int get_profile_str(char *keyname,char *str_return, int size, int type, char *path);
int32 getIPHostWanInterface(char* ifName, uint16 IPHostIstId, char noCreate);

extern int unlink(const char *pathname);
extern uint32 aton(char *s);
extern int commitNodeTimeId;
int omciWanType [2] = {WAN_SERVICELIST_MAX,WAN_SERVICELIST_MAX};

/*******************************************************************************************************************************
9.4.1: IP host config data

********************************************************************************************************************************/
int omciMeInitForIPHostConfigData(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForIPHostConfigData \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_TEST] = omciTestActionForIPHostConfigData;
	
	return 0;
}

int omciInternalCreatActionForIPHostConfigData(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 buffer[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	uint16 attriMask = 0;
	uint8 value[6] = {0};

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	//tmpomciMeInst_p->deviceId = instanceId;//for receive message
 
	/*meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;
	
	getIPHostConfigDataMACAddr(value, tmpomciMeInst_p, &tmpOmciManageEntity_p->omciAttriDescriptList[2], 0);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreatActionForIPHostConfigData: mac = %02x%02x%02x%02x%02x%02x\n", value[0], value[1], value[2], value[3], value[4], value[5]);

	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, 6);

	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 25);

	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<6;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<4;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<3;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);

	attriMask = 1<<2;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 25);

	attriMask = 1<<1;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 25);

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	return 0;	
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;	
}

static void omciPreSetIPhostIstId(uint16 * istId)
{
    if(isFiberhome_551601){
        *istId += 0x1;
    }
}


static void omciCheckIPhostIstId(uint16 * istId)
{
    if(isFiberhome_551601 && (* istId > 0)){
        *istId -= 0x1;
    }
}

int omciInitInstForIPHostConfigData(void)
{
	int ret = 0;
	uint16 instanceId = 0;
	int i = 0;
    const uint8 timerValid = 1;
	
	omciPreSetIPhostIstId(&instanceId);
    
#if defined(TCSUPPORT_PON_IP_HOST)
    initIPHostWanInterface(timerValid);
#endif


#if defined(TCSUPPORT_PON_IP_HOST)
	for( i = instanceId; i <= instanceId+1; i++){
		ret = omciInternalCreatActionForIPHostConfigData(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA, i);
	        if (ret == -1)		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create omciInitInstForIPHostConfigData fail\n");
	}
#endif

	return ret;
}

int omciTestActionForIPHostConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint8 *contentPtr = NULL;
	pthreadIPHostArgType *pthreadArg = NULL;
	pthread_t thread_id;
	pthread_attr_t thread_attr;
	uint32 addr = 0;
	uint8 resultAndReason = 0;
	uint8 testType = 0;
	uint8 devId = 0;
	omciBaselinePayload_t *omciBaselinePayload=NULL;
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
	omciExtendedPayload_t *omciExtendedPayload=NULL;
#endif

	if(omciPayLoad == NULL)
		return OMCI_CMD_ERROR;

	devId = omciPayLoad->devId;
	if(devId == OMCI_BASELINE_MSG)
	{
		omciBaselinePayload = (omciBaselinePayload_t *)omciPayLoad;
		contentPtr = (uint8 *)(omciBaselinePayload->msgContent);
	}
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
	else	if(devId == OMCI_EXTENED_MSG)
	{
		omciExtendedPayload = (omciExtendedPayload_t *)omciPayLoad;
		contentPtr = (uint8 *)(omciExtendedPayload->msgContent);
	}	
#endif
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "devId is invalid!\n");							
		return -1;
	}	
	
	pthreadArg = osMalloc(sizeof(pthreadIPHostArgType));
	if(pthreadArg == NULL)
		return -1;
	memset(pthreadArg, 0, sizeof(pthreadIPHostArgType));
	pthreadArg->msgFormat = devId;
	testType = (*contentPtr) & 0xF;
	memcpy(&addr, contentPtr+1, 4);
	if ((testType == IP_HOST_PING_TEST) || (testType == IP_HOST_TRACEROUTE_TEST) || (testType >= IP_HOST_VENDOR_TEST))
	{
		pthreadArg->tranCorrelatId = omciPayLoad->tranCorrelatId;
		pthreadArg->testType= testType;
		pthreadArg->meClassId = meClassId;
		pthreadArg->addr = addr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciTestAction: pthreadArg.tranCorrelatId = %x\n", pthreadArg->tranCorrelatId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciTestAction: pthreadArg.testType = %x\n", pthreadArg->testType);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciTestAction: pthreadArg.meClassId = %x \n", pthreadArg->meClassId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciTestAction: pthreadArg.addr = %x \n", pthreadArg->addr);
	}
	else/*reserved for future use*/
	{
		resultAndReason = OMCI_CMD_NOT_SUPPORT;
		if(pthreadArg != NULL){
			osFree(pthreadArg);
			pthreadArg = NULL;
		}
		goto pktSend;
	}

	pthread_attr_init(&thread_attr);
	pthread_attr_setstacksize(&thread_attr, MAX_STACK_SIZE);
	pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED); 

	if( pthread_create(&thread_id, &thread_attr, (void *)startTestForIPHost, (void*)pthreadArg) != 0 ){
		fprintf(stderr, "pthread_create error!!\n");
		if(pthreadArg != NULL){
			osFree(pthreadArg);
			pthreadArg = NULL;
		}
		exit(0);
	}
pktSend:

	/*MessageType response AR = 0; AK =1*/
	responseMessageTypeHandle((uint8 *)&omciPayLoad->msgType);

	if(devId == OMCI_BASELINE_MSG)
	{
		/*clear the Content to 0*/
		memset(contentPtr, 0, OMCI_BASELINE_CONT_LEN);
	}
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
	else	
	{
		/*clear the Content to 0*/
		memset(contentPtr, 0, omciExtendedPayload->msgContentLength+OMCI_EXEENTED_SIZE_OF_MIC);	
		omciExtendedPayload->msgContentLength = 1;
	}
#endif	

	/*write reason and result*/
	writeResultAndReasonToOmciPayLoad(resultAndReason, contentPtr);
	//	omciDumpPkt(omciPayLoad);

	omciPktSend(omciPayLoad, TRUE);	

	pthread_attr_destroy(&thread_attr);
	
	return 0;
}

void fileRead(char *path, char *buf, int size){
	int  fd=0, ret=0;

	memset(buf,0, size);
	fd = open(path,O_RDONLY);
	if(fd == -1){
		return;
	}

	ret = read(fd, buf, size);
	if(ret <= 0){
		close(fd);
		return;
	}

	close(fd);
	
	return;
}

int get_profile_str(char *keyname,char *str_return, int size, int type, char *path)
{

	FILE *fp = NULL;
	char *str_key = NULL;
	char stream[128] = {0};
	int enterOffSet = 1;
	int qmarkLength = 0;
	int skipQmark = 0;
	int totalLength = 0;

	fp = fopen(path,"r");
	if(fp == NULL){
		fprintf(stderr,"Can't open %s\n",path);
		return -1;
	}

	memset(str_return, 0, size);
	fseek(fp, 0, 0);
	if(type == 1){
		qmarkLength = 2;
		skipQmark = 1;
	}
	else if(type == 0){
		qmarkLength = 0;
		skipQmark = 0;
	}
	else{
		fprintf(stderr, "The input qmark type of get_profile_str is wrong \n");
		fclose(fp);
		return -1;
	}

	while(fgets(stream, 128, fp) != NULL){
		//str_enter = strrchr(stream,'\n');
		if(strrchr(stream,'\n') == NULL){
			enterOffSet = 0;
		}
		else{
			enterOffSet = 1;
		}

		str_key = strstr(stream,keyname);
		if(str_key == NULL || str_key != stream){
			continue;
		}

		totalLength = strlen(stream) - strlen(keyname) - enterOffSet - qmarkLength;
		if(size < totalLength+1){/*total length + '\0' should not less than buffer*/
			fprintf(stderr, "Too small buffer to catch the %s frome get_profile_str\n", keyname);
			fclose(fp);
			return -1;
		}
		else if(totalLength < 0) {/*can't get a negative length string*/
			fprintf(stderr, "No profile string can get\n");
			fclose(fp);
			return -1;
		}
		else{
			strncpy(str_return, stream+strlen(keyname)+skipQmark, totalLength);
			str_return[totalLength] = '\0';
			fclose(fp);
			return strlen(str_return);
		}
		
		memset(stream, 0, 128);
	}
	fclose(fp);
	fprintf(stderr,"File %s content %s is worng\n",path,keyname);
	
	return -1;
}

int omciIPHostCfgDataTestRstMsgHandle(pthreadIPHostArgType *pthreadArg)
{ 
	omciPayload_t tstPkt;
	omciBaselinePayload_t *tstPkt_baseline=NULL;
	uint8 *tmp = NULL; 
	char desAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char cmdbuf[120] = {0};
	char *tmpbuf = NULL;
	char *tmpPtr = NULL;
	char tmpValue[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int i = 0, j = 0, cnt = 0;
	uint32 tmpAddr = 0;
	char indexStr[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
	struct in_addr address;
	
	if (pthreadArg == NULL)
		return -1;
	
	memset(&tstPkt, 0, sizeof(omciPayload_t));
	tstPkt_baseline = (omciBaselinePayload_t *)(&tstPkt);
	
	tstPkt_baseline->tranCorrelatId = pthreadArg->tranCorrelatId;
	tmp = (uint8*)&tstPkt_baseline->meId;
	put16(tmp, OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	put16(tmp+2, 0);
	tstPkt_baseline->msgType = MT_OMCI_MSG_TYPE_TEST_RST;
	tstPkt_baseline->devId = OMCI_BASELINE_MSG;
 
	tmpbuf = malloc(MAX_BUF_LEN);
	if(tmpbuf == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "malloc fail!");
		return -1;
	}
	memset(tmpbuf, 0, sizeof(tmpbuf));

	if(pthreadArg->testType == 1)//ping
	{
		//do ping
		memset(desAddr, 0, sizeof(desAddr));
		memset(&address, 0, sizeof(address));
		address.s_addr = pthreadArg->addr;
		//printf("\r\n debug ping:pthreadArg->addr=%x", pthreadArg->addr);
		strcpy(desAddr, (char *)inet_ntoa(address));
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n ping:desAddr=%s", desAddr);
		
		memset(cmdbuf, 0, sizeof(cmdbuf));
		sprintf(cmdbuf,"ping -c %d %s > %s 2>/dev/null", 4, desAddr, IP_HOST_TEST_PING_PATH);
		system(cmdbuf);
		
		//read the ping result
		fileRead(IP_HOST_TEST_PING_PATH, tmpbuf, MAX_BUF_LEN);
		if(strstr(tmpbuf, IMCOMPLETE_STR) || 
			((!strstr(tmpbuf, ICMP_SEQ0)) && (!strstr(tmpbuf, ICMP_SEQ1)) && (!strstr(tmpbuf, ICMP_SEQ2)) &&(!strstr(tmpbuf, ICMP_SEQ3))))
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n ping result:no response \n");
			/*send test result packets*/
			//test result: 000, time out, no response
			tmp = (uint8 *)(tstPkt_baseline->msgContent);
			*tmp = 0;
			unlink(IP_HOST_TEST_PING_PATH);
		}
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n ping result:001, ICMP echo response attached\n");
			//test result: 001, ICMP echo response attached
			tmp = (uint8 *)(tstPkt_baseline->msgContent);
			*tmp = 1;
			tmp++;
			*tmp = 0x8;
			tmp++;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ0)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			tmp += 2;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ1)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			tmp += 2;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ2)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			tmp += 2;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ3)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			unlink(IP_HOST_TEST_PING_PATH);
		}
	}
	else if(pthreadArg->testType == 2)//traceroute
	{
		//do traceroute
		//test result: 010, ICMP time exceeded response attached
		tmp = (uint8 *)(tstPkt_baseline->msgContent);
		*tmp = 2;
		tmp++;
		
		memset(desAddr, 0, sizeof(desAddr));
		memset(&address, 0, sizeof(address));
		address.s_addr = pthreadArg->addr;
		//printf("\r\n debug traceroute:pthreadArg->addr=%x", pthreadArg->addr);
		strcpy(desAddr, (char *)inet_ntoa(address));
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n traceroute:desAddr=%s", desAddr);
		
		memset(cmdbuf, 0, sizeof(cmdbuf));
		sprintf(cmdbuf,"/usr/bin/traceroute -m 7 -q 1 -n %s > %s 2>/dev/null", desAddr, IP_HOST_TEST_TRACEROUTE_PATH);
		system(cmdbuf);
		
		//read the traceroute result
		fileRead(IP_HOST_TEST_TRACEROUTE_PATH, tmpbuf, MAX_BUF_LEN);
		if((tmpPtr = strstr(tmpbuf, " 1  ")) == NULL)
		{
			*tmp = 0;//cnt = 0
		}
		else
		{
			tmp++;
			for(j=1; j<8; j++)
			{
				memset(indexStr, 0, sizeof(indexStr));
				sprintf(indexStr, " %d  ", j);
				if((tmpPtr = strstr(tmpbuf, indexStr)) == NULL)
					continue;
				tmpPtr += 4;
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				if((*tmpPtr) != '*'){
					while((*tmpPtr) != ' '){
						tmpValue[i++] = *tmpPtr;
						tmpPtr++;
					}
					tmpValue[i] = '\0';
					//printf("\r\n debug traceroute:tmpValue=%s", tmpValue);
					tmpAddr = aton(tmpValue);
					//printf("\r\n debug traceroute:tmpValue=%02X", tmpAddr);
					put32(tmp, tmpAddr);
					tmp += 4;
					cnt++;
				}
			}
			tmp = (uint8 *)(tstPkt_baseline->msgContent);
			tmp++;
			*tmp = 4*cnt;
		}
		//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n traceroute: cnt=%d", cnt);
		unlink(IP_HOST_TEST_TRACEROUTE_PATH);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "testType error!");
		if(tmpbuf != NULL){
			free(tmpbuf);
			tmpbuf = NULL;
		}
		return -1;
	}

	if(tmpbuf != NULL){
		free(tmpbuf);
		tmpbuf = NULL;
	}

	return	omciInternalNotificationPktSend(&tstPkt);
}

#ifdef TCSUPPORT_OMCI_EXTENDED_MSG	
int omciIPHostCfgDataTestRstMsgHandle_Extended(pthreadIPHostArgType *pthreadArg)
{
	omciPayload_t tstPkt;
	omciExtendedPayload_t *tstPkt_extended=NULL;	
	uint8 *tmp = NULL; 
	char desAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char cmdbuf[120] = {0};
	char *tmpbuf = NULL;
	char *tmpPtr = NULL;
	char tmpValue[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int i = 0, j = 0, cnt = 0;
	uint32 tmpAddr = 0;
	char indexStr[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
	struct in_addr address;
	
	if (pthreadArg == NULL)
		return -1;
	
	memset(&tstPkt, 0, sizeof(tstPkt));
	tstPkt_extended = (omciExtendedPayload_t *)(&tstPkt);
	
	tstPkt_extended->tranCorrelatId = (uint16)pthreadArg->tranCorrelatId;
	tmp = (uint8*)(&(tstPkt_extended->meId));
	put16(tmp, OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	put16(tmp+2, 0);
	tstPkt_extended->msgType = MT_OMCI_MSG_TYPE_TEST_RST;
	tstPkt_extended->devId = OMCI_EXTENED_MSG;
 
	tmpbuf = malloc(MAX_BUF_LEN);
	if(tmpbuf == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "malloc fail!");
		return -1;
	}
	memset(tmpbuf, 0, sizeof(tmpbuf));

	if(pthreadArg->testType == 1)//ping
	{
		//do ping
		memset(desAddr, 0, sizeof(desAddr));
		memset(&address, 0, sizeof(address));
		address.s_addr = pthreadArg->addr;
		strcpy(desAddr, (char *)inet_ntoa(address));
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n ping:desAddr=%s", desAddr);
		
		memset(cmdbuf, 0, sizeof(cmdbuf));
		sprintf(cmdbuf,"ping -c %d %s > %s 2>/dev/null", 4, desAddr, IP_HOST_TEST_PING_PATH);
		system(cmdbuf);
		
		//read the ping result
		fileRead(IP_HOST_TEST_PING_PATH, tmpbuf, MAX_BUF_LEN);
		if(strstr(tmpbuf, IMCOMPLETE_STR) || 
			((!strstr(tmpbuf, ICMP_SEQ0)) && (!strstr(tmpbuf, ICMP_SEQ1)) && (!strstr(tmpbuf, ICMP_SEQ2)) &&(!strstr(tmpbuf, ICMP_SEQ3))))
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n ping result:no response \n");
			/*send test result packets*/
			//test result: 000, time out, no response
			tstPkt_extended->msgContentLength = 0x1; 
			unlink(IP_HOST_TEST_PING_PATH);
		}
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n ping result:001, ICMP echo response attached\n");
			//test result: 001, ICMP echo response attached
			tstPkt_extended->msgContentLength = 0x9; 
			tmp = tstPkt_extended->msgContent;
			*tmp = 1;
			tmp++; 
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ0)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			tmp += 2;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ1)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			tmp += 2;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ2)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			tmp += 2;
			if((tmpPtr = strstr(tmpbuf, ICMP_SEQ3)) == NULL)
			{
				put16(tmp, 0xFFFF);
			}
			else
			{
				tmpPtr = strstr(tmpbuf, "time=");
				tmpPtr += 5;//format as: 64 bytes from X.X.X.X: icmp_seq=0 ttl=64 time=X.X ms
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				while((*tmpPtr) != ' '){
					tmpValue[i++] = *tmpPtr;
					tmpPtr++;
				}
				tmpValue[i] = '\0';
				//printf("\r\n debug tmpValue = %s \r\n", tmpValue);
				//printf("\r\n debug tmpTime = %d \r\n", atoi(tmpValue));
				put16(tmp, atoi(tmpValue));
			}
			unlink(IP_HOST_TEST_PING_PATH);
		}
	}
	else if(pthreadArg->testType == 2)//traceroute
	{
		//do traceroute
		//test result: 010, ICMP time exceeded response attached
		tmp = tstPkt_extended->msgContent;
		*tmp = 2;
		tmp++;
		
		memset(desAddr, 0, sizeof(desAddr));
		memset(&address, 0, sizeof(address));
		address.s_addr = pthreadArg->addr;
		//printf("\r\n debug traceroute:pthreadArg->addr=%x", pthreadArg->addr);
		strcpy(desAddr, (char *)inet_ntoa(address));
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n traceroute:desAddr=%s", desAddr);
		
		memset(cmdbuf, 0, sizeof(cmdbuf));
		sprintf(cmdbuf,"/usr/bin/traceroute -m 7 -q 1 -n %s > %s 2>/dev/null", desAddr, IP_HOST_TEST_TRACEROUTE_PATH);
		system(cmdbuf);
		
		//read the traceroute result
		fileRead(IP_HOST_TEST_TRACEROUTE_PATH, tmpbuf, MAX_BUF_LEN);
		if((tmpPtr = strstr(tmpbuf, " 1  ")) == NULL)
		{
			//cnt = 0
			tstPkt_extended->msgContentLength = 1; 
		}
		else
		{
			for(j=1; j<8; j++)
			{
				memset(indexStr, 0, sizeof(indexStr));
				sprintf(indexStr, " %d  ", j);
				if((tmpPtr = strstr(tmpbuf, indexStr)) == NULL)
					continue;
				tmpPtr += 4;
				i = 0;
				memset(tmpValue, 0, sizeof(tmpValue));
				if((*tmpPtr) != '*'){
					while((*tmpPtr) != ' '){
						tmpValue[i++] = *tmpPtr;
						tmpPtr++;
					}
					tmpValue[i] = '\0';
					//printf("\r\n debug traceroute:tmpValue=%s", tmpValue);
					tmpAddr = aton(tmpValue);
					//printf("\r\n debug traceroute:tmpValue=%02X", tmpAddr);
					put32(tmp, tmpAddr);
					tmp += 4; 
					cnt++;
				}
			}
			tstPkt_extended->msgContentLength = 4*cnt+1;
		}  
		unlink(IP_HOST_TEST_TRACEROUTE_PATH);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "testType error!");
		if(tmpbuf != NULL){
			free(tmpbuf);
			tmpbuf = NULL;
		}
		return -1;
	}

	if(tmpbuf != NULL){
		free(tmpbuf);
		tmpbuf = NULL;
	}

	return	omciInternalNotificationPktSend(&tstPkt);
}
#endif

void startTestForIPHost(void* arg)
{
	pthreadIPHostArgType *pthreadArg = NULL;
	int ret = 0;

	if (arg == NULL)
		return;
	
	pthreadArg = (pthreadIPHostArgType*)arg;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "startTest:pthreadArg->tranCorrelatId = %x\n", pthreadArg->tranCorrelatId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "startTest:pthreadArg->testType = %x\n", pthreadArg->testType);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "startTest:pthreadArg->meClassId = %x\n", pthreadArg->meClassId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "startTest:pthreadArg->addr = %x\n", pthreadArg->addr);

	//do ME test
	if (pthreadArg->meClassId == OMCI_CLASS_ID_IP_HOST_CONFIG_DATA)
	{
		if(pthreadArg->msgFormat == OMCI_BASELINE_MSG)
		{
			ret = omciIPHostCfgDataTestRstMsgHandle(pthreadArg);
			if (ret != 0)
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "startTestForIPHost:omciIPHostCfgDataTestRstMsgHandle fail\n");		
		}
		#ifdef TCSUPPORT_OMCI_EXTENDED_MSG			
		else if(pthreadArg->msgFormat == OMCI_EXTENED_MSG)
		{
			ret = omciIPHostCfgDataTestRstMsgHandle_Extended(pthreadArg);
			if (ret != 0)
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "startTestForIPHost:omciIPHostCfgDataTestRstMsgHandle fail\n");			
		}
		#endif			
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "message format is invalid!\n");
		}
	}
	else 
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "startTestForIPHost: unknown me Class Id\n");
	}

	osFree(pthreadArg);
}

int checkInternalClientIp(char *st)
{
	int i = 0, dot = 0;
	int i1 =0 ,i2 = 0 ,i3 = 0, i4 = 0;
	int len = 0;

	if(strcmp(st, "0.0.0.0") == 0)
		return 1;

	len = strlen(st);
	if(len < 7 || len > 15)
		return 0;
	
	for(i = 0; i<len; i++)
	{
		if(((*(st+i)) <= '9')&&((*(st+i)) >= '0'))
		{
			if(dot == 0){
				i1 = i1 *10 + (*(st+i)-'0');
			}
			else if(dot ==1) {
				i2 = i2 *10 + (*(st+i)-'0');
			}
			else if(dot ==2) {
				i3 = i3 *10 + (*(st+i)-'0');
			}		
			else if(dot ==3) {
				i4 = i4 *10 + (*(st+i)-'0');
			}
		}
		else if((*(st+i)) == '.')
			dot++;
		else if((*(st+i)) > '9')//error
			return 0;
		else if((*(st+i)) < '0')//error
			return 0;
	}
	
	//printf("i = %d, dot = %d, i1(%d), i2(%d), i3(%d), i4(%d)\n",i, dot, i1,i2,i3,i4);
	if (dot != 3)
		return 0;

	if(i1 == 0 || i4 == 255 || i1 == 127 )	
		return 0;

	if ((i1>223) || (i2>255) || (i3>255) || (i4>255))
		return 0;

	return 1;
}

int checkSubnetLegal(char *mask)
{	
	uint32 netmask = 0, broadcast = 0;
	int i = 0;

	if(mask == NULL)
		return 0;

	netmask = aton(mask);
	broadcast = aton("255.255.255.255");

	for(i = 0; i < 32; i++) {
		if(netmask & (1 << i))
			break;
		broadcast &= ~(1 << i);
	}

	if(broadcast != netmask) {
		return 0;
	}

	return 1;
}

int32 getIPHostWanInterfaceType(uint16 instanceId)
{
    omciMeInst_t * TCPUDPConfigDataList[OMCI_GET_INSTANC_BY_ATTR_MAX_NUM] = {0};
    omciMeInst_t * VEIPList[OMCI_GET_INSTANC_BY_ATTR_MAX_NUM] = {0};
    omciMeInst_t * SIPCfgList[OMCI_GET_INSTANC_BY_ATTR_MAX_NUM] = {0};
    omciMeInst_t * meInst_p = NULL;
    omciManageEntity_ptr currentME = NULL;
    omciMeInst_ptr meInstantCurPtr = NULL;
    char iphostWan[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char buff[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    uint16 istId = 0;
    int i = 0;
    int j = 0;
    
    if(omciWanType[instanceId] != WAN_SERVICELIST_MAX)
        return omciWanType[instanceId];

    istId = instanceId;
    omciPreSetIPhostIstId(&istId);

    /*1. ip_host <------- TCP/UDP config data <------ VEIP ----- TR069 management */

    /* get TCP/UDP config data which is point to this ip host*/
    omciGetInstByAttriValue(OMCI_CLASS_ID_TCP_UDP_CFGDATA , "ip host pointer",  &istId, TCPUDPConfigDataList, OMCI_GET_INSTANC_BY_ATTR_MAX_NUM);
    for (i = 0; i < OMCI_GET_INSTANC_BY_ATTR_MAX_NUM; i++)
    {
        meInst_p = TCPUDPConfigDataList[i];
        if (NULL == meInst_p)
            break;
        
        /*get the instance ID of ME:TCP/UDP config data*/
        istId = get16(meInst_p->attributeVlaue_ptr);

        /* get VEIP which is point to this TCP/UDP config data*/
        omciGetInstByAttriValue(OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT , "tcp/udp pointer",  &istId, VEIPList, OMCI_GET_INSTANC_BY_ATTR_MAX_NUM);
        for (j = 0; j < OMCI_GET_INSTANC_BY_ATTR_MAX_NUM; j++)
        {
            meInst_p = VEIPList[i];
            if (NULL == meInst_p)
                break;
            
            /*get the instance ID of ME:VEIP */
            istId = get16(meInst_p->attributeVlaue_ptr);

            /* get TR069 management by instance id */
            currentME = omciGetMeByClassId(OMCI_CLASS_ID_TR069_MANAGE_SERVER);
            meInstantCurPtr = omciGetInstanceByMeId(currentME , istId);

            if (NULL != meInstantCurPtr){
                omciWanType[instanceId] = WAN_SERVICELIST_TR069;
                return WAN_SERVICELIST_TR069;
            }
        }
    }
    
    istId = instanceId;
    omciPreSetIPhostIstId(&istId);
    /*2. ip_host <----- TCP/UDP config data <----- SIP agent config data*/
    /* get TCP/UDP config data which is point to this ip host*/
    omciGetInstByAttriValue(OMCI_CLASS_ID_TCP_UDP_CFGDATA , "ip host pointer",  &istId, TCPUDPConfigDataList, OMCI_GET_INSTANC_BY_ATTR_MAX_NUM);
    for (i = 0; i < OMCI_GET_INSTANC_BY_ATTR_MAX_NUM; i++)
    {
        meInst_p = TCPUDPConfigDataList[i];
        if (NULL == meInst_p)
            break;
        
        /*get the instance ID of ME:TCP/UDP config data*/
        istId = get16(meInst_p->attributeVlaue_ptr);

        /* get SIP agent config data which is point to this TCP/UDP config data*/
        omciGetInstByAttriValue(OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA , "TCP/UDP pointer",  &istId, SIPCfgList, OMCI_GET_INSTANC_BY_ATTR_MAX_NUM);
        if(NULL != SIPCfgList[0]){
            omciWanType[instanceId] = WAN_SERVICELIST_TR069;
            return WAN_SERVICELIST_VOICE;
        }
    }


/* not found !! for HW olt do not create  TCP/UDP config data for VEIP 0*/
#if defined(TCSUPPORT_PON_IP_HOST)
    istId = (0 == instanceId) ? 1 : 0;
    if (omciWanType[istId] == WAN_SERVICELIST_TR069)
        return WAN_SERVICELIST_VOICE;
    if (omciWanType[istId] == WAN_SERVICELIST_VOICE)
        return WAN_SERVICELIST_TR069;

    sprintf(iphostWan, "XPON_%s", "VOICE");
    if(0 == tcapi_get(iphostWan, "IPHostIstId", buff) && (0 != strcmp(buff,OMCI_INVALID_IPHOST_ID))
        && atoi(buff) == istId)
        return WAN_SERVICELIST_TR069;
    
    sprintf(iphostWan, "XPON_%s", "TR069");
    if(0 == tcapi_get(iphostWan, "IPHostIstId", buff) && (0 != strcmp(buff,OMCI_INVALID_IPHOST_ID))
        && atoi(buff) == istId)
        return WAN_SERVICELIST_VOICE;
        

    return WAN_SERVICELIST_TR069;   //default tr69 wan
    
#endif

    return -1;
}

int32 setWaninfoAttr(int type , char* ifName, char* buf)
{
	char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char attr[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char oldValue[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char xponIPHost[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char PVCName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    
    if(NULL == ifName || NULL == buf )
        return -1;
    
    if(ifNameToNodeName(ifName, nodeName,PVCName) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setIPHostConfigDataIPAddr ifName to nodeName error, fail\n");
        return -1;
    }
    switch(type){
        case WAN_IF_TYPE_ISP:
            strcpy(attr, "ISP");
            tcapi_set(nodeName, "LinkMode", "linkIP");
            break;
        case WAN_IF_TYPE_IP_ADDR:
            strcpy(attr, "IPADDR");
            break;
        case WAN_IF_TYPE_NETMASK:
            strcpy(attr, "NETMASK");
            break;

        case WAN_IF_TYPE_GATEWAY:
            strcpy(attr, "GATEWAY");
            break;

#if !defined(TCSUPPORT_CT) 
        case WAN_IF_TYPE_PRI_DNS:
            strcpy(attr, "PRIMARYDNS");
            break;
        case WAN_IF_TYPE_SEC_DNS:
            strcpy(attr, "SECONDARYDNS");
            break;
#endif
        case WAN_IF_TYPE_VLANID:
            strcpy(attr, "VLANID");
            tcapi_set(PVCName, "VLANID", buf);
            break;
        case WAN_IF_TYPE_DOT1P:
            strcpy(attr, "dot1pData");
            tcapi_set(nodeName, "DOT1P", buf);
            tcapi_set(PVCName, "DOT1P", buf);
            break;
        case WAN_IF_TYPE_IPHOSTID:
             tcapi_set(nodeName, "IPHostIstId", buf);
             return 0;
        default:
            return -1;
    }

    if(strcmp(attr, "ISP") == 0 && strcmp(buf, "0") == 0){/*DHCP mode*/
        tcapi_set(nodeName, "DHCPEnable", "1");
        tcapi_set(nodeName, attr, buf);
        tcapi_set(xponIPHost, COMMIT_WAN_ATTR, "1");
    }
    else{
        tcapi_get(nodeName, attr, oldValue);
        if(strcmp(oldValue, buf) != 0){ /* value change*/
            tcapi_set(nodeName, attr, buf);
            if(strcmp(attr, "ISP") != 0){
                memset(oldValue, 0, sizeof(oldValue));
                tcapi_get(nodeName, "ISP", oldValue);
                if((strcmp(oldValue, "1") == 0) || (strcmp(attr, "VLANID") == 0)){  /*static ip mode*/
                    tcapi_set(xponIPHost, COMMIT_WAN_ATTR, "1");
                }
            }       
        }
    }

    if(delayOmciCommitNode(PVCName) !=0){
        OMCI_ERROR("delayOmciCommitNode fail\n");
        return -1;
    }
}
int32 addIPHostWan(int newPvcIndex, int newEntryIndex,uint16 instanceId, int type)
{
	char entryName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char nasName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char wanPVCNode[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char vlanId[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char serviceList[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char DHCPEnable[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char xponWan[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char istId[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int i = 0;
	char cmd[128] = {0};
	int ret = 0;
    char IFIdx[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    int ifidx = 0;

    tcapi_get("Wan_Common", "LatestIFIdx", &IFIdx);
    ifidx = atoi(IFIdx) + 1;
    sprintf(IFIdx, "%d", ifidx);
    
	if (newPvcIndex == -1 || newEntryIndex == -1)
		return -1;
		
	if(WAN_SERVICELIST_TR069 == type)
	{
		sprintf(vlanId, "%d", 200);
		sprintf(serviceList, "%s", "TR069");
		sprintf(DHCPEnable, "%d", 1);
        sprintf(xponWan, "XPON_%s", "TR069");
	}else if(WAN_SERVICELIST_VOICE == type)
	{
		sprintf(vlanId, "%d", 99);
		sprintf(serviceList, "%s", "VOICE");
		sprintf(DHCPEnable, "%d", 0);
        sprintf(xponWan, "XPON_%s", "VOICE");
	}
    else
    {
		sprintf(vlanId, " ");
		sprintf(serviceList, " ");
		sprintf(DHCPEnable, "%d", 0);
	}

	sprintf(wanPVCNode, "Wan_PVC%d", newPvcIndex);
	sprintf(nasName,IFNAME_FORMAT,newPvcIndex, newEntryIndex);
	sprintf(entryName, WANNODE_FORMAT, newPvcIndex, newEntryIndex);
	sprintf(istId, "%d", instanceId);
    sprintf(xponWan, "XPON_%s", serviceList);
	omciNodeInfo_t wanParam_tbl[] = {
		{wanPVCNode,"GPONEnable", "Yes"},
		{wanPVCNode,"EPONEnable", "Yes"},	
		{wanPVCNode,"ATMEnable", "No"},
		{wanPVCNode,"PTMEnable", "No"},
		{wanPVCNode,"ENCAP", "1483 Bridged IP LLC"},
		{wanPVCNode,"VLANMode", "TAG"},
		{wanPVCNode,"VLANID", "200"},
		{wanPVCNode,"DOT1P", "0"},
		{wanPVCNode,"PVCACTIVE", "Yes"},
		{entryName,"ISP", "0"},
		{entryName, "Active", "Yes"},
		{entryName, "ServiceList",serviceList},
		{entryName, "BandActive","N/A"},
		{entryName, "WanMode","Route"},
		{entryName, "LinkMode","linkIP"},
		{entryName, "IPVERSION","IPv4"},
		{entryName, "DHCPOption60",""},
		{entryName, "dot1q","Yes"},
		{entryName, "dot1p","Yes"},
		{entryName, "VLANID", "200"},
		{entryName,"DOT1P", "0"},
		{entryName, "dot1pData", "0"},
		{entryName, "VLANMode", "TAG"},
		{entryName, "IGMPproxy","No"},
		{entryName, "NASName",nasName},
		{entryName, "IFName",nasName},
		{entryName, "DHCPRealy","Yes"},
		{entryName, "DHCPEnable", "1"},
		{entryName, "DHCPv6", "No"},
		{entryName, "DHCPv6PD", "No"},
		{entryName, "MTU", "1500"},
		{entryName, "MulticastVID", ""},
		{entryName, "IPHostIstId", istId},
		{entryName,"BridgeInterface", "No"},
		{entryName,"DsliteEnable","No"},
		{entryName,"PDOrigin", "None"},
		{entryName,"CreateByOMCI", "Yes"},
 		{xponWan, "valid", "Yes"},
		{xponWan, "IPHostIstId", istId},
		{xponWan, "WanIf", nasName},
	};
	for(i=0; i<(sizeof(wanParam_tbl)/sizeof(wanParam_tbl[0])); i++){
		if(TCAPI_SUCCESS != tcapi_set(wanParam_tbl[i].nodeName, wanParam_tbl[i].attrName, wanParam_tbl[i].value))
			OMCI_ERROR("addIPHostWan%s can't add a new attribute, fail\n",serviceList);		
	}
#ifdef TCSUPPORT_PON_IP_HOST
	if(WAN_SERVICELIST_TR069 == type) 
	{
		sprintf(cmd, "echo tr069 %s > /proc/tc3162/pon_wanIf", nasName);
		system(cmd);
	}
#endif

    ret = tcapi_set(entryName,"IFIdx",IFIdx);
    tcapi_set("Wan_Common", "LatestIFIdx", IFIdx);
    memcpy(omciIPHostWanInfo[instanceId].serviceList,serviceList,strlen(serviceList)+1);
    memcpy(omciIPHostWanInfo[instanceId].ifNode,nasName,strlen(nasName)+1);
	return ret;
}

int32 setIPHostWanServiceList(char * IPHosttype,char *ifName, uint16 IPHostIstId)
{
	char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int pvcIndex = -1;
    int entryIndex = -1;
    char istId[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};

    if(NULL == IPHosttype || NULL == ifName)
        return -1;

    sprintf(istId, "%d", IPHostIstId);
    
#if !defined(TCSUPPORT_CT) 
    sscanf(ifName, IFNAME_FORMAT, &pvcIndex);
    sprintf(nodeName, WANNODE_FORMAT, pvcIndex);
#endif

    tcapi_set(nodeName, "ServiceList", IPHosttype);

    sprintf(nodeName, "XPON_%s", IPHosttype);
    tcapi_set(nodeName,"WanIf",ifName);
    tcapi_set(nodeName,"IPHostIstId",istId);
    return 0;
}

int32 setIPHostWanInfo (omciIPhostWanInfo_t wanInfo,char *ifName,uint16 iphostId)
{
    char buff[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    if( NULL == ifName)
        return -1;

    if(iphostId == 0xFF)
        sprintf(buff, "%s", OMCI_INVALID_IPHOST_ID);
    else
        sprintf(buff, "%d", iphostId);
    
    setWaninfoAttr(WAN_IF_TYPE_IPHOSTID, ifName, buff);
    
    if(0 != strlen(wanInfo.ipAddr))
        setWaninfoAttr(WAN_IF_TYPE_IP_ADDR , ifName, wanInfo.ipAddr);
    if(0 != strlen(wanInfo.netMask))
        setWaninfoAttr(WAN_IF_TYPE_NETMASK , ifName, wanInfo.netMask);
    if(0 != strlen(wanInfo.geteWay))
        setWaninfoAttr(WAN_IF_TYPE_GATEWAY , ifName, wanInfo.geteWay);
    if(0 != strlen(wanInfo.priDNS))
        setWaninfoAttr(WAN_IF_TYPE_PRI_DNS , ifName, wanInfo.priDNS);
    if(0 != strlen(wanInfo.secDNS))
        setWaninfoAttr(WAN_IF_TYPE_SEC_DNS , ifName, wanInfo.secDNS);
    if(0 != strlen(wanInfo.MACAdd))
        setWaninfoAttr(WAN_IF_TYPE_MAC_ADDR , ifName, wanInfo.MACAdd);
    if(0 != strlen(wanInfo.isp))
        setWaninfoAttr(WAN_IF_TYPE_ISP , ifName, wanInfo.isp);
    if(0 != strlen(wanInfo.vlanId))
        setWaninfoAttr(WAN_IF_TYPE_VLANID , ifName, wanInfo.vlanId);
    if(0 != strlen(wanInfo.pBit))
        setWaninfoAttr(WAN_IF_TYPE_DOT1P , ifName, wanInfo.pBit);
    return 0;
}

static int getXPONIPHostWanInfo( uint16 istId , char* attr , char* value)
{
    char TR069Node[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char VOICENode[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    int ret = 0;
    int i = 0;
    char buff[OMCI_LAYER3_STR_BUFF_SIZE];
    char * ptr[2] = {TR069Node,VOICENode};

    sprintf(TR069Node, "XPON_%s", "TR069");
    sprintf(VOICENode, "XPON_%s", "VOICE");

    if(NULL == attr || NULL == value || istId > 2){
        return -1;
    }

    for(i = 0; i < 2 ; i ++)
    {
        if( 0 != tcapi_get(ptr[i], "IPHostIstId", buff)){
            return -1;
        }
        if(strlen(buff) > 1){
            continue;
        }
        if(istId == atoi(buff)){
            OMCI_WARN("==[%s][%d] buff = %s i = %d istId = %d ===\r\n",__FUNCTION__,__LINE__,buff,i,istId);
            break;
        }
    }

    memset(value, 0 ,OMCI_LAYER3_STR_BUFF_SIZE);
    if(i > 1){
        if(0 == strcmp(attr,"valid")){
            memcpy(value,"No",strlen("No"));
        }else {
            memcpy(value,"N/A",strlen("N/A"));
        }
        return 0;
    }

    if(0 == strcmp(attr,"valid")){
        memcpy(value,"Yes",strlen("Yes"));
        return 0;
    }
    
    if (0 == strcmp(attr,"type")){
        i == 0 ? (memcpy(value,"TR069",strlen("TR069")) )
            :(memcpy(value,"VOICE",strlen("VOICE")) );
        return 0;
    }

    if(0 == strcmp(attr, "WanIf")){
        return tcapi_get(ptr[i], attr, value);
        
    }

    return -1;
}

static void setXPONWanNode( char* node , char* ifname, uint16 istId)
{
    char buff[OMCI_LAYER3_STR_BUFF_SIZE] ={0};
    const uint16 invalidIstId = 0xFF;
    if(invalidIstId == istId){
        tcapi_set(node,"WanIf","N/A");
        tcapi_set(node,"IPHostIstId",OMCI_INVALID_IPHOST_ID);
        tcapi_set(node,"valid","No");
    }
    else
    {
        tcapi_set(node,"WanIf",ifname);
        sprintf(buff, "%d", istId);
        tcapi_set(node,"IPHostIstId",buff);
        tcapi_set(node,"valid","Yes");
    }
}

void unsetPVCNode(char * wanIf)
{
    int pvcIdx = 0;
    int entryIdx = 0;
    int i;
    char buf[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char WanService[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    if(NULL == wanIf)
        return ;

    tcapi_unset(wanIf);
    OMCI_WARN("==[%s][%d] unset Wan %s ==\r\n",__FUNCTION__,__LINE__,wanIf);
    sscanf(wanIf, "Wan_PVC%d_Entry%d", &pvcIdx, &entryIdx);

    for(i = 0; i < MAX_SMUX_NUM; i++)
    {
		sprintf(buf, "%s_%s%d_%s%d", WAN, WAN_PVC, pvcIdx, WAN_INTERFACE, i);
        if(tcapi_get(buf, "ServiceList", WanService) == 0)
            return ;
	}
    
    sprintf(buf, "%s_%s%d", WAN, WAN_PVC, pvcIdx);
    tcapi_unset(buf);
    return ;
}

static int initCheckIPHostWan (int wanType,uint16 istId,omciIPhostCheckWanInfo_t * infoIn, omciIPhostCheckWanInfo_t* infoAn)
{
    char buff[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    int pvcIndex = 0;
    int entryIndex = 0;

    memset(infoIn,0,sizeof(omciIPhostCheckWanInfo_t));
    memset(infoAn,0,sizeof(omciIPhostCheckWanInfo_t));
    
    infoIn->wanType = wanType;
    infoIn->IPHostIstId = istId;
    
    infoAn->IPHostIstId = (0 == infoIn->IPHostIstId) ? 1: 0;
    infoIn->oldAttr = (WAN_SERVICELIST_TR069 == infoIn->wanType) ? &omciOldVOICEInfo : &omciOldTR069Info;

    memset(infoIn->IPHostType, 0, OMCI_LAYER3_STR_BUFF_SIZE);
    memset(infoAn->IPHostType, 0, OMCI_LAYER3_STR_BUFF_SIZE);
    if(WAN_SERVICELIST_TR069 == infoIn->wanType){
        memcpy(infoIn->IPHostType,"TR069",strlen("TR069"));
        memcpy(infoAn->IPHostType,"VOICE",strlen("VOICE"));
    }else if (WAN_SERVICELIST_VOICE == infoIn->wanType){
        memcpy(infoIn->IPHostType,"VOICE",strlen("VOICE"));
        memcpy(infoAn->IPHostType,"TR069",strlen("TR069"));
    }else {
        return -1;
    }

    sprintf(infoIn->IPHostWan, "XPON_%s", infoIn->IPHostType);
    sprintf(infoAn->IPHostWan, "XPON_%s", infoAn->IPHostType);
    getXPONIPHostWanInfo(infoIn->IPHostIstId,"WanIf",infoIn->ifName);
    getXPONIPHostWanInfo(infoIn->IPHostIstId,"type",infoIn->IPHostTypeNow);
    getXPONIPHostWanInfo(infoAn->IPHostIstId,"WanIf",infoAn->ifName);
    getXPONIPHostWanInfo(infoAn->IPHostIstId,"type",infoAn->IPHostTypeNow);

#if !defined(TCSUPPORT_CT) 
    sscanf(infoIn->ifName, IFNAME_FORMAT, &pvcIndex);
    sprintf(infoIn->wanIf, WANNODE_FORMAT, pvcIndex);
#endif

    if((tcapi_get(infoIn->wanIf, "CreateByOMCI", buff) == 0) && 
                (0 == strcmp(buff,"Yes") ) ){
        infoIn->ifCreateByOMCI = 1;
    }
    sprintf(buff,"%d",OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096);
    if( 0 != strcmp(omciIPHostWanInfo[infoIn->IPHostIstId].vlanId,buff))
    {
        infoIn->ifSetByOMCI = 1;
    }

    if(0 != strlen(infoAn->ifName) ){
#if !defined(TCSUPPORT_CT) 
        sscanf(infoAn->ifName, IFNAME_FORMAT, &pvcIndex);
        sprintf(infoAn->wanIf, WANNODE_FORMAT, pvcIndex);
#endif
        if((tcapi_get(infoAn->wanIf, "CreateByOMCI", buff) == 0) && 
                    (0 == strcmp(buff,"Yes") ) ){
            infoAn->ifCreateByOMCI = 1;
        }
        sprintf(buff,"%d",OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096);
        if( 0 != strcmp(omciIPHostWanInfo[infoAn->IPHostIstId].vlanId,buff))
        {
            infoAn->ifSetByOMCI = 1;
        }
    }

    OMCI_WARN("==[%s][%d]IPHostIstId = %d==\r\n",__FUNCTION__,__LINE__,istId);
    OMCI_WARN("==[%s][%d]wanTypeIn = %d==\r\n",__FUNCTION__,__LINE__,wanType);
    OMCI_WARN("==[%s][%d]IPHostIstIdIn = %d==\r\n",__FUNCTION__,__LINE__,infoIn->IPHostIstId);
    OMCI_WARN("==[%s][%d]IPHostIstIdAn = %d==\r\n",__FUNCTION__,__LINE__,infoAn->IPHostIstId);
    OMCI_WARN("==[%s][%d]IPHostTypeIn = %s==\r\n",__FUNCTION__,__LINE__,infoIn->IPHostType);
    OMCI_WARN("==[%s][%d]IPHostTypeAn = %s==\r\n",__FUNCTION__,__LINE__,infoAn->IPHostType);
    OMCI_WARN("==[%s][%d]IPHostWanIn =%s ==\r\n",__FUNCTION__,__LINE__,infoIn->IPHostWan);
    OMCI_WARN("==[%s][%d]IPHostWanAn = %s==\r\n",__FUNCTION__,__LINE__,infoAn->IPHostWan);
    OMCI_WARN("==[%s][%d]ifNameIn = %s==\r\n",__FUNCTION__,__LINE__,infoIn->ifName);
    OMCI_WARN("==[%s][%d]ifNameAn = %s ==\r\n",__FUNCTION__,__LINE__,infoAn->ifName);
    OMCI_WARN("==[%s][%d]IPHostTypeNowIn = %s==\r\n",__FUNCTION__,__LINE__,infoIn->IPHostTypeNow);
    OMCI_WARN("==[%s][%d]IPHostTypeNowAn = %s==\r\n",__FUNCTION__,__LINE__,infoAn->IPHostTypeNow);
    OMCI_WARN("==[%s][%d]old attr vlanId = %s==\r\n",__FUNCTION__,__LINE__,infoIn->oldAttr->vlanId);
    OMCI_WARN("==[%s][%d]ifAnSetByOMCI = %d==\r\n",__FUNCTION__,__LINE__,infoAn->ifSetByOMCI);
    return 0;
}

int32 checkIPHostWanInterface(omciIPhostChkTimer_t * param)
{
    omciIPhostCheckWanInfo_t infoIn;
    omciIPhostCheckWanInfo_t infoAn;
    char buff[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char ifName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    const uint16 invalidIstId = 0xFF;

    if(NULL == param){
        return -1;
    }

    if (0 != initCheckIPHostWan(param->wanType,param->iphostId,&infoIn, &infoAn)){
        free(param);
        return -1;
    }
    
    free(param);

    /* type match*/
    if(0 == strcmp(infoIn.IPHostType,infoIn.IPHostTypeNow)){
        OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
        return 0;
    }
    
    /* type not match */
    /* create by omci */
    if(infoIn.ifCreateByOMCI)
    {
        OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
        /* this wan is valid */
        if(tcapi_get(infoIn.IPHostWan, "valid", buff) == 0 && 
            (0 == strcmp(buff,"Yes"))) {
            /* this wan is create by omci switch two wan type */
            if(infoAn.ifSetByOMCI ) {
                OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
                
                setXPONWanNode(infoIn.IPHostWan,infoIn.ifName,infoIn.IPHostIstId);
                setXPONWanNode(infoAn.IPHostWan,infoAn.ifName,infoAn.IPHostIstId);

                setIPHostWanServiceList(infoIn.IPHostType,infoIn.ifName,infoIn.IPHostIstId);
                setIPHostWanServiceList(infoAn.IPHostType,infoAn.ifName,infoAn.IPHostIstId);
            }
            /* not create by omci and this type wan used, change wan if and unset omci wan*/
            else {
                OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
                tcapi_get(infoIn.IPHostWan, "WanIf", ifName);
                setIPHostWanInfo(omciIPHostWanInfo[infoIn.IPHostIstId],ifName,infoIn.IPHostIstId);
                unsetPVCNode(infoIn.wanIf);

                setXPONWanNode(infoIn.IPHostWan,ifName,infoIn.IPHostIstId);
                setXPONWanNode(infoAn.IPHostWan,ifName,invalidIstId);
            }
        }
        /* this wan not used , just change service list*/
        else {
            OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
            setIPHostWanServiceList(infoIn.IPHostType,infoIn.ifName,infoIn.IPHostIstId);
            setXPONWanNode(infoAn.IPHostWan,infoAn.ifName,invalidIstId);
        }
    }
    /* not create by omci*/
    else
    {
        OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
        /* this Wan valid just switch the two wan if */
        if(tcapi_get(infoIn.IPHostWan, "valid", buff) == 0 && 
            (0 == strcmp(buff,"Yes")))  {
            OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
            tcapi_get(infoIn.IPHostWan, "WanIf", ifName);
            
            if(infoAn.ifSetByOMCI ) {
                OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
                setIPHostWanInfo(omciIPHostWanInfo[infoIn.IPHostIstId],infoAn.ifName,infoIn.IPHostIstId);
                setIPHostWanServiceList(infoIn.IPHostType,infoAn.ifName,infoAn.IPHostIstId);
                
                setIPHostWanInfo(omciIPHostWanInfo[infoAn.IPHostIstId],infoIn.ifName,infoAn.IPHostIstId);
                setXPONWanNode(infoIn.IPHostWan,ifName,infoIn.IPHostIstId);
                setXPONWanNode(infoAn.IPHostWan,infoAn.ifName,infoAn.IPHostIstId);
            }else{
                OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
                setIPHostWanInfo(*infoIn.oldAttr,infoIn.ifName,infoAn.IPHostIstId);
                setIPHostWanServiceList(infoAn.IPHostType,infoAn.ifName,infoAn.IPHostIstId);
                
                setIPHostWanInfo(omciIPHostWanInfo[infoIn.IPHostIstId],ifName,infoIn.IPHostIstId);
                setXPONWanNode(infoIn.IPHostWan,ifName,infoIn.IPHostIstId);
            }
        }
        /* Another IPHOST not used create new one and revert old one */
        else {
            OMCI_WARN("==[%s][%d]==\r\n",__FUNCTION__,__LINE__);
            setIPHostWanInfo(*infoIn.oldAttr,infoIn.ifName,invalidIstId);
            setXPONWanNode(infoAn.IPHostWan,infoAn.ifName,invalidIstId);

            omciPreSetIPhostIstId(&infoIn.IPHostIstId);
            getIPHostWanInterface(ifName, infoIn.IPHostIstId, 0);
            omciCheckIPhostIstId(&infoIn.IPHostIstId);
            setIPHostWanInfo(omciIPHostWanInfo[infoIn.IPHostIstId],ifName,infoIn.IPHostIstId);
            setXPONWanNode(infoIn.IPHostWan,ifName,infoIn.IPHostIstId);
        }
        
    }
    return 0;
}

int checkIPHostWanInterfaceTimer(int wanType, uint16 IPHostIstId)
{
    omciIPhostChkTimer_t * param;
    uint16 idx = IPHostIstId;

    omciCheckIPhostIstId(&idx);

    param = malloc(sizeof(omciIPhostChkTimer_t));
	if(param == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "malloc fail!");
		return -1;
	}
    
    param->wanType = wanType;
    param->iphostId = idx;

    return timerStartS(1000, (TimerCallback)checkIPHostWanInterface, param);
}

int32 initWaninfoByIPHost(omciIPhostWanInfo_t * WanInfo, char * node,char * IPHostNode)
{
    int idx = 0;
    char buffer[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    int pvcIndex = 0;
    int entryIndex = 0;

    tcapi_get(node, "ISP", buffer);
    strncpy(WanInfo->isp,buffer,OMCI_LAYER3_VAL_BUFF_SIZE);

    tcapi_get(node, "IPADDR", buffer);
    strncpy(WanInfo->ipAddr,buffer,OMCI_LAYER3_STR_BUFF_SIZE);
    
    tcapi_get(node, "NETMASK", buffer);
    strncpy(WanInfo->netMask,buffer,OMCI_LAYER3_STR_BUFF_SIZE);

    tcapi_get(node, "GATEWAY", buffer);
    strncpy(WanInfo->geteWay,buffer,OMCI_LAYER3_STR_BUFF_SIZE);
    
    tcapi_get(node, "DNS", buffer);
    strncpy(WanInfo->priDNS,buffer,OMCI_LAYER3_STR_BUFF_SIZE);
    
    tcapi_get(node, "SecDNS", buffer);
    strncpy(WanInfo->secDNS,buffer,OMCI_LAYER3_STR_BUFF_SIZE);

    tcapi_get(node, "VLANID", buffer);
    strncpy(WanInfo->vlanId,buffer,OMCI_LAYER3_VAL_BUFF_SIZE);
    
    tcapi_get(node, "dot1pData", buffer);
    strncpy(WanInfo->pBit,buffer,OMCI_LAYER3_VAL_BUFF_SIZE);
    
    tcapi_get(node, "ServiceList", buffer);
    strncpy(WanInfo->serviceList,buffer,OMCI_LAYER3_STR_BUFF_SIZE);
    strncpy(WanInfo->ifNode,node,OMCI_LAYER3_STR_BUFF_SIZE);

    if(tcapi_get(node, "IFName", buffer)== 0){
        tcapi_set(IPHostNode,"WanIf", buffer);
    }else{

#if !defined(TCSUPPORT_CT) 
        sscanf(node, WANNODE_FORMAT, &pvcIndex);
    	if(pvcIndex < 0 || pvcIndex >= PVC_NUM)
    		return -1;
    	
    	sprintf(buffer,IFNAME_FORMAT , pvcIndex);
#endif
        
        tcapi_set(IPHostNode,"WanIf", buffer);
    }

    tcapi_set(IPHostNode,"valid", "Yes");
    return 0;
}


int32 getWaninfoByIPHost(uint16 IPHostIstId, int type , char * buf)
{
    uint16 idx = IPHostIstId;
    
    if ( idx > 1 || NULL == buf){
        OMCI_ERROR("[%s][%d] param error \r\n",__FUNCTION__,__LINE__);
        return -1;
    }
    
    switch(type){
		case WAN_IF_TYPE_ISP:
            memcpy(buf,omciIPHostWanInfo[idx].isp,strlen(omciIPHostWanInfo[idx].isp)+1);
			break;
		case WAN_IF_TYPE_IP_ADDR:
            memcpy(buf,omciIPHostWanInfo[idx].ipAddr,strlen(omciIPHostWanInfo[idx].ipAddr)+1);
			break;
		case WAN_IF_TYPE_NETMASK:
            memcpy(buf,omciIPHostWanInfo[idx].netMask,strlen(omciIPHostWanInfo[idx].netMask)+1);
			break;
		case WAN_IF_TYPE_GATEWAY:
            memcpy(buf,omciIPHostWanInfo[idx].geteWay,strlen(omciIPHostWanInfo[idx].geteWay)+1);
			break;
		case WAN_IF_TYPE_PRI_DNS:
            memcpy(buf,omciIPHostWanInfo[idx].priDNS,strlen(omciIPHostWanInfo[idx].priDNS)+1);
			break;
		case WAN_IF_TYPE_SEC_DNS:
            memcpy(buf,omciIPHostWanInfo[idx].secDNS,strlen(omciIPHostWanInfo[idx].secDNS)+1);
			break;
		case WAN_IF_TYPE_VLANID:
            memcpy(buf,omciIPHostWanInfo[idx].vlanId,strlen(omciIPHostWanInfo[idx].vlanId)+1);
			break;
		case WAN_IF_TYPE_DOT1P:
            memcpy(buf,omciIPHostWanInfo[idx].pBit,strlen(omciIPHostWanInfo[idx].pBit)+1);
			break;
		default:
			return -1;
	}
    return 0;
}

int32 setWaninfoByIPHost(uint16 IPHostIstId, int type , char * buf)
{
    int idx = IPHostIstId;

    if (idx < 0 || idx > 1 || NULL == buf){
        OMCI_ERROR("[%s][%d] param error \r\n",__FUNCTION__,__LINE__);
        return -1;
    }
    
    switch(type){
		case WAN_IF_TYPE_ISP:
            memcpy(omciIPHostWanInfo[idx].isp,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_IP_ADDR:
            sprintf(omciIPHostWanInfo[idx].ipAddr,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_NETMASK:
            sprintf(omciIPHostWanInfo[idx].netMask,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_GATEWAY:
            sprintf(omciIPHostWanInfo[idx].geteWay,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_PRI_DNS:
            sprintf(omciIPHostWanInfo[idx].priDNS,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_SEC_DNS:
            sprintf(omciIPHostWanInfo[idx].secDNS,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_VLANID:
            sprintf(omciIPHostWanInfo[idx].vlanId,buf,strlen(buf)+1);
			break;
		case WAN_IF_TYPE_DOT1P:
            sprintf(omciIPHostWanInfo[idx].pBit,buf,strlen(buf)+1);
			break;
		default:
			return -1;
	}
    return 0;
}

char unsetNodeName[OMCI_LAYER3_STR_BUFF_SIZE];
int32 initIPHostWanInterface( uint8 timerValid )
{
    char IPHostNode[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char buffer[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    int pvcIndex = 0;
    int entryIndex = 0;
    char wanMode[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
    char wanServiceList[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
    char wanActive[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
    int i = 0;
    int waninfoId = 0;
    omciIPhostWanInfo_t * wanInfoArr[2];
    memset(nodeName, 0, sizeof(nodeName));
    memset(buffer, 0, sizeof(buffer));
    wanInfoArr[0] = &omciOldTR069Info;
    wanInfoArr[1] = &omciOldVOICEInfo;
    
    OMCI_WARN("==[%s][%d ==\r\n",__FUNCTION__,__LINE__);
    for(i = 0; i <= 1 ; i ++)
    {
        memset(wanInfoArr[i],0,sizeof(omciIPhostWanInfo_t));
        sprintf(wanInfoArr[i]->vlanId,"%d",OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096);
        sprintf(wanInfoArr[i]->pBit,"%d",OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15);
        strcpy(wanInfoArr[i]->iphostId, OMCI_INVALID_IPHOST_ID);
        
        sprintf(omciIPHostWanInfo[i].vlanId,"%d",OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096);
        sprintf(omciIPHostWanInfo[i].pBit,"%d",OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15);
        sprintf(omciIPHostWanInfo[i].iphostId,"%d",i);

        omciWanType[i] = WAN_SERVICELIST_MAX;
    }

    strcpy(IPHostNode, "XPON_TR069");
    tcapi_set(IPHostNode,"WanIf", "N/A");
    tcapi_set(IPHostNode,"IPHostIstId", OMCI_INVALID_IPHOST_ID);
    tcapi_set(IPHostNode,"valid", "No");
    
    strcpy(IPHostNode, "XPON_VOICE");
    tcapi_set(IPHostNode,"WanIf", "N/A");
    tcapi_set(IPHostNode,"IPHostIstId", OMCI_INVALID_IPHOST_ID);
    tcapi_set(IPHostNode,"valid", "No");
    
    for(pvcIndex = 0; pvcIndex < PVC_NUM; pvcIndex++){
        for(entryIndex = 0; entryIndex < MAX_SMUX_NUM; entryIndex++){
            sprintf(nodeName, "%s_%s%d_%s%d", WAN, WAN_PVC, pvcIndex, WAN_INTERFACE, entryIndex);   
            if((tcapi_get(nodeName, "WanMode", wanMode) != 0) || (strcmp(wanMode, "Route") != 0 )) {
                continue;
            }
                    
            memset(buffer, 0, sizeof(buffer));
            memset(wanServiceList, 0 ,sizeof(wanServiceList));
            if(tcapi_get(nodeName, "ServiceList", wanServiceList)!= 0) {
                continue;
            }
            if(tcapi_get(nodeName, "CreateByOMCI", buffer)== 0 && (NULL != strstr(buffer, "Yes") )) {
                memset(unsetNodeName, 0, sizeof(unsetNodeName));
                strcpy(unsetNodeName, nodeName);
                if (timerValid){
                    if( 0 > timerStartS(1000, (TimerCallback)unsetPVCNode, unsetNodeName) )
                        OMCI_WARN("==[%s][%d] unset Wan error ==\r\n",__FUNCTION__,__LINE__);
                } else {
                    unsetPVCNode(unsetNodeName);
                    OMCI_WARN("==[%s][%d] unset Wan %s ==\r\n",__FUNCTION__,__LINE__,unsetNodeName);
                }
                continue;
            }
            
            if(NULL != strstr(wanServiceList, "TR069") ) {
                tcapi_set(nodeName, "IPHostIstId", OMCI_INVALID_IPHOST_ID);
                strcpy(IPHostNode, "XPON_TR069");
                OMCI_WARN("==[%s][%d] ==\r\n",__FUNCTION__,__LINE__);
                initWaninfoByIPHost(&omciOldTR069Info, nodeName,IPHostNode);
            }
            else if(NULL != strstr(wanServiceList, "VOICE") ) {
                tcapi_set(nodeName, "IPHostIstId", OMCI_INVALID_IPHOST_ID);
                strcpy(IPHostNode, "XPON_VOICE");
                OMCI_WARN("==[%s][%d] ==\r\n",__FUNCTION__,__LINE__);
                initWaninfoByIPHost(&omciOldVOICEInfo, nodeName,IPHostNode);
            }
            else{
                continue;
            }
            
        }
    }
    return 0;
}

int32 getIPHostWanInterface(char* ifName, uint16 IPHostIstId, char noCreate)
{
    char IPHostWanNode[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char buffer[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char valid[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int pvcIndex = 0;
	int entryIndex = 0;
	char wanMode[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
    char WanService[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char wanActive[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char wanIFName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char IPHostWan[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char wanType[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
    int serviceType = WAN_SERVICELIST_MAX;
	int newPvcIndex = -1, newEntryIndex = -1;
    uint16 vlanId = 0;
    uint8 pBit = 0;
    uint16 vlanIdIn = 0;
    uint8 pBitIn = 0;
    uint8 pvcMatch = 0;
    char iphostId[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
    static uint8 tr069Restored = 0;
    static uint8 voiceRestore = 0;
    uint8 * restroedPtr = NULL;
    omciIPhostWanInfo_t * attrPtr = NULL;

    omciCheckIPhostIstId(&IPHostIstId);
    if(IPHostIstId > 1)
        return -1;
    
    serviceType = getIPHostWanInterfaceType(IPHostIstId);
    
    if(WAN_SERVICELIST_TR069 == serviceType){
        sprintf(IPHostWan, "TR069");
        attrPtr = &omciOldTR069Info;
        restroedPtr = &tr069Restored;
        strcpy(IPHostWanNode, "XPON_TR069");
    }
    else if(WAN_SERVICELIST_VOICE == serviceType){
        sprintf(IPHostWan, "VOICE");
        attrPtr = &omciOldVOICEInfo;
        restroedPtr = &voiceRestore;
        strcpy(IPHostWanNode, "XPON_VOICE");
    }else{
        return -1;
    }
	memset(nodeName, 0, sizeof(nodeName));
	memset(buffer, 0, sizeof(buffer));
    memset(valid, 0, sizeof(valid));
    
    getWaninfoByIPHost(IPHostIstId,WAN_IF_TYPE_VLANID,buffer);
    vlanIdIn = atoi(buffer);
    getWaninfoByIPHost(IPHostIstId,WAN_IF_TYPE_DOT1P,buffer);
    pBitIn = atoi(buffer);

    sprintf(IPHostWanNode, "XPON_%s", IPHostWan);
    if(0 != tcapi_get(IPHostWanNode, "IPHostIstId", iphostId)){
        sprintf(iphostId, "%s", OMCI_INVALID_IPHOST_ID);
    }
    if(0 != tcapi_get(IPHostWanNode, "valid", valid)){
        sprintf(valid, "%s", "No");
    }
    if(0 != tcapi_get(IPHostWanNode, "WanIf", buffer)){
        sprintf(valid, "%s", "No");
    }
    
    OMCI_WARN("==[%s][%d]iphostId = %d type = %s ==\r\n",__FUNCTION__,__LINE__,IPHostIstId,IPHostWan);
    OMCI_WARN("==[%s][%d]current valid status is %s iphostId = %s WanIf = %s ==\r\n",__FUNCTION__,__LINE__,valid,iphostId,buffer);
    
	if((strlen(buffer) != 0 ) && (0 != strcmp(buffer,"N/A") ) && (0 == strcmp(valid,"Yes") ) 
        && ((0 == strcmp(iphostId,OMCI_INVALID_IPHOST_ID) ) || (atoi(iphostId) == IPHostIstId) ) )
    {

#if !defined(TCSUPPORT_CT) 
        sscanf(buffer,IFNAME_FORMAT,&pvcIndex);
        snprintf(nodeName, OMCI_LAYER3_STR_BUFF_SIZE, "%s_%s%d", WAN, WAN_PVC, pvcIndex);
#endif
        
        if (0 == strcmp(iphostId,OMCI_INVALID_IPHOST_ID) && noCreate){
                return -1;
        }
        if((0 == *restroedPtr) &&  (0 == noCreate)){
        initWaninfoByIPHost(attrPtr, nodeName,IPHostWanNode);
            *restroedPtr = 1;
        }
		OMCI_WARN("==[%s][%d]get WanIf %s\n",__FUNCTION__,__LINE__, buffer);
        if (0 == strcmp(iphostId,OMCI_INVALID_IPHOST_ID) ){
            sprintf(iphostId, "%d", IPHostIstId);
            OMCI_WARN("==[%s][%d]set nodeName %s iphostid = %s\n",__FUNCTION__,__LINE__, nodeName,iphostId);
            OMCI_WARN("==[%s][%d]set IPHostWanNode %s ifname = %s\n",__FUNCTION__,__LINE__, IPHostWanNode,buffer);
            tcapi_set(nodeName, "IPHostIstId", iphostId); 
            setXPONWanNode(IPHostWanNode,buffer,IPHostIstId);
        }
		strcpy(ifName, buffer);
		return 0;

	}

    if (noCreate)
        return -1;
    
#if defined(TCSUPPORT_PON_IP_HOST)//if not found wan, add new wan
    OMCI_WARN("==[%s][%d] Doing create ==\r\n",__FUNCTION__,__LINE__);
    memset(nodeName, 0, sizeof(nodeName));
#if !defined(TCSUPPORT_CT) 
	if(WAN_SERVICELIST_TR069 == serviceType){
        sprintf(IPHostWan, "TR069");
        serviceType = WAN_SERVICELIST_TR069;
		newPvcIndex = 0;
		newEntryIndex = 0;
    }
    else{
        sprintf(IPHostWan, "VOICE");
        serviceType = WAN_SERVICELIST_VOICE;
		newPvcIndex = 1;
		newEntryIndex = 0;
    }
#endif
	if(addIPHostWan(newPvcIndex, newEntryIndex,IPHostIstId,serviceType) == 0){	
		sprintf(ifName, IFNAME_FORMAT, newPvcIndex, newEntryIndex); 
		return 0;
	}
	else{
		OMCI_ERROR("addIPHostWan can't add a new wan, fail\n");
	}
#endif
	
	return -1;
}

int ifNameToNodeName(char *ifName, char *nodeName,char * PVCname){
#if !defined(TCSUPPORT_CT) 
	int pvcIndex = -1;

	sscanf(ifName, IFNAME_FORMAT, &pvcIndex);
	if(pvcIndex < 0 || pvcIndex >= PVC_NUM)
		return -1;

	sprintf(nodeName, WANNODE_FORMAT, pvcIndex);
	sprintf(PVCname, "Wan_PVC%d", pvcIndex);
#endif
	return 0;
}
	
int omciCommitIPHostConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr)
{
	char commitWan[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char xponIPHost[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char PVCName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    uint16 instanceId = (OMCI_CLASS_ID_IP_HOST_CONFIG_DATA == meClassId )? get16(meInstant_ptr->attributeVlaue_ptr):0xFFFF;
    uint16 idx;
	
	if (meInstant_ptr == NULL || 0xFFFF == instanceId)
		return -1;
    idx = instanceId;
    omciCheckIPhostIstId(&idx);
    sprintf(xponIPHost, IPHOST_NODE, idx);
    
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCommitIPHostConfigData\n");

	tcapi_get(xponIPHost, COMMIT_WAN_ATTR, commitWan);
	
	if(strcmp(commitWan, "0") != 0){
		if(getIPHostWanInterface(ifName,instanceId,0) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciCommitIPHostConfigData can't find ip host wanif, fail\n");
			return -1;
		}
		
		if(ifNameToNodeName(ifName, nodeName,PVCName) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciCommitIPHostConfigData ifNameToNodeName error, fail\n");
			return -1;
		}
 		if(delayOmciCommitNode(PVCName) !=0){
			OMCI_ERROR("addOmciCommitNodeList and start the Timer fail\n");
			return -1;  
		}	
		tcapi_set(xponIPHost, COMMIT_WAN_ATTR, "0");
	}
	
	return 0;
}

char* getWanInfo(int type, char* ifName, char* buf){
	FILE *fp = NULL;
	char tmpBuf[512];
	char cmd[64];
	char macInfo[2][18];
	char *pValue = NULL;
#if !defined(TCSUPPORT_CT) 
	char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char attr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int pvcIndex = -1;
#endif
	
	if(ifName == NULL || buf == NULL)
		return NULL;
	
	strcpy(buf, "");

	// get mac info
	if(type == WAN_IF_TYPE_MAC_ADDR){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,"ifconfig %s > %s", ifName, TMP_IF_PATH);
		system(cmd);
		
		fp = fopen(TMP_IF_PATH, "r");
		if(fp == NULL)
			return buf;
		
		memset(macInfo,0, sizeof(macInfo));
		while (fgets (tmpBuf, sizeof(tmpBuf), fp) != NULL){
			if(pValue = strstr(tmpBuf,"HWaddr"))
				sscanf(pValue,"%s %s", macInfo[0],macInfo[1]); //format: HWaddr 00:AA:BB:01:23:45
		}
		
		if(strlen(macInfo[1]) > 0)
			strcpy(buf, macInfo[1]);
	
		fclose(fp);
		unlink(TMP_IF_PATH);
		return buf;
	}

#if !defined(TCSUPPORT_CT) 

	switch(type){
		case WAN_IF_TYPE_IP_ADDR:
			strcat(attr, "WanIP");
			break;
		case WAN_IF_TYPE_NETMASK:
			strcat(attr, "WanSubMask");
			break;
		case WAN_IF_TYPE_GATEWAY:
			strcat(attr, "WanDefGW");
			break;
		case WAN_IF_TYPE_PRI_DNS:
			strcat(attr, "DNSIP");
			break;	
		case WAN_IF_TYPE_SEC_DNS:
			strcat(attr, "SECDNSIP");
			break;
		default:
			return buf;
	}

	sscanf(ifName, IFNAME_FORMAT, &pvcIndex);
	if(pvcIndex < 0 || pvcIndex >= PVC_NUM)
		return buf;

	sprintf(nodeName, WANDEV_INFO_FORMAT, pvcIndex);
	memset(tmpBuf, 0, sizeof(tmpBuf));
	tcapi_get(nodeName, attr, tmpBuf);

	if(strlen(tmpBuf) >= 7 && strlen(tmpBuf) <= 15) //ip addr format xxx.xxx.xx.xxx
		strcpy(buf, tmpBuf);
#endif

	return buf;
}

char tr69wan[OMCI_LAYER3_STR_BUFF_SIZE];

int setWanInfo(int type ,uint16 IPHostIstId, char* buf){

	char ifName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char nodeName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	char attr[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char oldValue[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char xponIPHost[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    char serviceType[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
    char PVCName[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
    uint16 idx = IPHostIstId;
	
	if(ifName == NULL || buf == NULL)
		return -1;

    omciCheckIPhostIstId(&idx);

    if ( idx > 1 ){
        OMCI_ERROR("[%s][%d] param error \r\n",__FUNCTION__,__LINE__);
        return -1;
    }

    if (setWaninfoByIPHost(idx, type , buf) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setIPHostConfigDataIPOptions set waninfo fail\n");
		return -1;
	}
    
	if(getIPHostWanInterface(ifName,IPHostIstId,0) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setIPHostConfigDataIPOptions can't find ip host wanif, fail\n");
		return -1;
	}

#if  defined(TCSUPPORT_PON_IP_HOST)
    getXPONIPHostWanInfo( idx , "type" , serviceType);
    if(strcmp(serviceType, "TR069")==0)
	    strcpy(tr69wan, nodeName);
#endif
	
    setWaninfoAttr(type,ifName,buf);
	return 0;
}

int32 setIPHostConfigDataIPOptions(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	
	uint8 ipOptions = 0;
	
	char isp[OMCI_LAYER3_VAL_BUFF_SIZE] = {0};
	uint16 length = 0;
	char cmd[256];
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	ipOptions = * (uint8 *)value;
	
	if(ipOptions & IP_OPTION_DHCP_MASK){ /*DHCP*/
		strcpy(isp, "0");
	}
	else {/*Static IP*/
		strcpy(isp, "1");
	}
	setWanInfo(WAN_IF_TYPE_ISP, instanceId,isp);

	if(getIPHostWanInterface(ifName,instanceId,0) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setIPHostConfigDataIPOptions can't find ip host wanif, fail\n");
		return -1;
	}

	if(ipOptions & IP_OPTION_ENABLE_PING_RESPONSE_MASK){/*ping response*/
		sprintf(cmd, "/usr/bin/iptables -D OUTPUT -p icmp --icmp-type 0 -o %s -j DROP", ifName);
		system(cmd);
	}
	else{//Static IP
		sprintf(cmd, "/usr/bin/iptables -D OUTPUT -p icmp --icmp-type 0 -o %s -j DROP", ifName);
		system(cmd);
		
		sprintf(cmd, "/usr/bin/iptables -A OUTPUT -p icmp --icmp-type 0 -o %s -j DROP", ifName);
		system(cmd);
	}

	if(ipOptions & IP_OPTION_ENABLE_TRACERT_RESPONSE_MASK){ /*trace route response*/
		sprintf(cmd, "/usr/bin/iptables -D OUTPUT -p icmp --icmp-type 11 -o %s -j DROP", ifName);
		system(cmd);
#if !defined(TCSUPPORT_CMCC)		
		sprintf(cmd, "/usr/bin/iptables -D INPUT -p udp --dport 32768: -i %s -j DROP", ifName);
		system(cmd);
#endif
	}
	else{/*Static IP*/
		sprintf(cmd, "/usr/bin/iptables -D OUTPUT -p icmp --icmp-type 11 -o %s -j DROP", ifName);
		system(cmd);
		
		sprintf(cmd, "/usr/bin/iptables -A OUTPUT -p icmp --icmp-type 11 -o %s -j DROP", ifName);
		system(cmd);
#if !defined(TCSUPPORT_CMCC)
		sprintf(cmd, "/usr/bin/iptables -D INPUT -p udp --dport 32768: -i %s -j DROP", ifName);
		system(cmd);
	
		sprintf(cmd, "/usr/bin/iptables -A INPUT -p udp --dport 32768: -i %s -j DROP", ifName);
		system(cmd);
#endif
	}

	/*set attr*/
	attributeValuePtr = omciGetInstAttriByName(meInstantPtr, omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}


int32 getIPHostConfigDataMACAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	int mac[8] = {0};
	char tmpMAC[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	int i = 0;
	uint16 length = 0;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

#ifdef TCSUPPORT_PON_IP_HOST
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
#endif

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
#ifdef TCSUPPORT_PON_IP_HOST
    if (getIPHostWanInterface(ifName,instanceId,1) == 0)
    {
        if(getWanInfo(WAN_IF_TYPE_MAC_ADDR, ifName, tmpMAC) == NULL){
    		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "get wanInfo mac fail\n");
    		return -1;
    	}
    }
    else
    {
        if(get_profile_str("LAN_MAC=", tmpMAC, sizeof(tmpMAC), 0, IP_HOST_MAC_PATH) == -1){
            return -1;
            }
    }
#else
	if(get_profile_str("LAN_MAC=", tmpMAC, sizeof(tmpMAC), 0, IP_HOST_MAC_PATH) == -1)
		return -1;
#endif
							
	sscanf(tmpMAC, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	memset(tmpMAC, 0, sizeof(tmpMAC));
	for(i=0; i<6; i++)
	{
		tmpMAC[i] = (unsigned char)mac[i];
	}
	

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, tmpMAC, length);


	return getTheValue(value, (char *)attributeValuePtr, 6, omciAttribute);
}

int32 getIPHostConfigDataIPAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	//int ret = 0;
	//char ipAddr[32] = {0};
	//struct in_addr address;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
/*
	ret = tcapi_get("Lan_Entry0", "IP", ipAddr);
	if(ret == -1)
		return ret;
	
	memset(&address, 0, sizeof(address));
	inet_aton(ipAddr, &address);
*/	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
//	put32(attributeValuePtr, address.s_addr);
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}

int32 setIPHostConfigDataIPAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
	uint32 tmpAddr = 0;
	uint16 length = 0;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpAddr = get32((uint8 *)value);
	memset(&address, 0, sizeof(address));
	address.s_addr = tmpAddr;
	strcpy(ipAddr, (char *)inet_ntoa(address));
	if(checkInternalClientIp(ipAddr) == 0)
		return -1;

#ifdef TCSUPPORT_PON_IP_HOST
	setWanInfo(WAN_IF_TYPE_IP_ADDR, instanceId,ipAddr);
#else
	//tcapi_set("Lan_Entry0", "IP", ipAddr);
	//tcapi_commit("Lan_Entry0");
	//tcapi_save();
#endif
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 getIPHostConfigDataMask(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	//int ret = 0;
	//char mask[32] = {0};
	//struct in_addr address;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
/*
	ret = tcapi_get("Lan_Entry0", "netmask", mask);
	if(ret == -1)
		return ret;
	
	memset(&address, 0, sizeof(address));
	inet_aton(mask, &address);
*/	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
//	put32(attributeValuePtr, address.s_addr);
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}

int32 setIPHostConfigDataMask(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	char mask[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
	uint32 tmpAddr = 0;
	uint16 length = 0;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpAddr = get32((uint8 *)value);
	memset(&address, 0, sizeof(address));
	address.s_addr = tmpAddr;
	strcpy(mask, (char *)inet_ntoa(address));
	if(checkSubnetLegal(mask) == 0)
		return -1;

#ifdef TCSUPPORT_PON_IP_HOST
	setWanInfo(WAN_IF_TYPE_NETMASK,instanceId,mask);
#else
	//tcapi_set("Lan_Entry0", "netmask", mask);
	//tcapi_commit("Lan_Entry0");
	//tcapi_save();
#endif
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 setIPHostConfigDataGateway(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	char addr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
	uint32 tmpAddr = 0;
	uint16 length = 0;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpAddr = get32((uint8 *)value);
	memset(&address, 0, sizeof(address));
	address.s_addr = tmpAddr;
	strcpy(addr, (char *)inet_ntoa(address));
	if(checkInternalClientIp(addr) == 0)
		return -1;
	
#ifdef TCSUPPORT_PON_IP_HOST
	setWanInfo(WAN_IF_TYPE_GATEWAY, instanceId,addr);
#endif

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 setIPHostConfigDataPrimaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	char addr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
	uint32 tmpAddr = 0;
	uint16 length = 0;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpAddr = get32((uint8 *)value);
	memset(&address, 0, sizeof(address));
	address.s_addr = tmpAddr;
	strcpy(addr, (char *)inet_ntoa(address));
	if(checkInternalClientIp(addr) == 0)
		return -1;
	
#ifdef TCSUPPORT_PON_IP_HOST
	setWanInfo(WAN_IF_TYPE_PRI_DNS, instanceId,addr);
#endif

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 setIPHostConfigDataSecondaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	char addr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
	uint32 tmpAddr = 0;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpAddr = get32((uint8 *)value);
	memset(&address, 0, sizeof(address));
	address.s_addr = tmpAddr;
	strcpy(addr, (char *)inet_ntoa(address));
	if(checkInternalClientIp(addr) == 0)
		return -1;
	
#ifdef TCSUPPORT_PON_IP_HOST
	setWanInfo(WAN_IF_TYPE_SEC_DNS, instanceId,addr);
#endif

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 getIPHostConfigDataCurAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;

#ifdef TCSUPPORT_PON_IP_HOST
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
#endif

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[1];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	if(((*attributeValuePtr)&IP_OPTION_DHCP_MASK) == 0)//DHCP disable, curAddr == addr
	{
		currentAttribute = &currentME->omciAttriDescriptList[4];
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	else//DHCP enable, curAddr is assigned by DHCP, if not,  undefined
	{
#ifdef TCSUPPORT_PON_IP_HOST
		if(getIPHostWanInterface(ifName,instanceId,1) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setIPHostConfigDataSecondaryDNS can't find ip host wanif, fail\n");
			return -1;
		}

		getWanInfo(WAN_IF_TYPE_IP_ADDR, ifName, ipAddr);

		memset(&address, 0, sizeof(address));
		inet_aton(ipAddr, &address);

		put32(value, address.s_addr);		
		return 0;
		
#else
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
#endif

	}
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}

int32 getIPHostConfigDataCurMask(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;

#ifdef TCSUPPORT_PON_IP_HOST
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
	uint32 temp;
#endif

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[1];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	if(((*attributeValuePtr)&IP_OPTION_DHCP_MASK) == 0)//DHCP disable, curAddr == addr
	{
		currentAttribute = &currentME->omciAttriDescriptList[5];
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;	
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	else//DHCP enable, curAddr is assigned by DHCP, if not,  undefined
	{
#ifdef TCSUPPORT_PON_IP_HOST
		if(getIPHostWanInterface(ifName,instanceId,1) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setIPHostConfigDataSecondaryDNS can't find ip host wanif, fail\n");
			return -1;
		}

		getWanInfo(WAN_IF_TYPE_NETMASK, ifName, ipAddr);
		
#if 0
		if(strlen(ipAddr) <= 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getIPHostConfigDataIPAddr can't find ip host wanif IP, fail\n");
			return -1;
		}
#endif		
		memset(&address, 0, sizeof(address));
		inet_aton(ipAddr, &address);
		
		put32(value, address.s_addr);
		
		return 0;
#else
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
#endif
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}

int32 getIPHostConfigDataCurGateway(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
#ifdef TCSUPPORT_PON_IP_HOST
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
#endif 

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[1];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	if(((*attributeValuePtr)&IP_OPTION_DHCP_MASK) == 0)//DHCP disable, curAddr == addr
	{
		currentAttribute = &currentME->omciAttriDescriptList[6];
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	else//DHCP enable, curAddr is assigned by DHCP, if not,  undefined
	{
#ifdef TCSUPPORT_PON_IP_HOST
		if(getIPHostWanInterface(ifName,instanceId,1) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setIPHostConfigDataSecondaryDNS can't find ip host wanif, fail\n");
			return -1;
		}
		
		getWanInfo(WAN_IF_TYPE_GATEWAY, ifName, ipAddr);
		
#if 0
		if(strlen(ipAddr) <= 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getIPHostConfigDataIPAddr can't find ip host wanif IP, fail\n");
			return -1;
		}
#endif	

		memset(&address, 0, sizeof(address));
		inet_aton(ipAddr, &address);
			
		put32(value, address.s_addr);
		return 0;
#else
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
#endif
		
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}

int32 getIPHostConfigDataCurPrimaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
#ifdef TCSUPPORT_PON_IP_HOST
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
#endif

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[1];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	if(((*attributeValuePtr)&IP_OPTION_DHCP_MASK) == 0)//DHCP disable, curAddr == addr
	{
		currentAttribute = &currentME->omciAttriDescriptList[7];
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	else//DHCP enable, curAddr is assigned by DHCP, if not,  undefined
	{
#ifdef TCSUPPORT_PON_IP_HOST
		if(getIPHostWanInterface(ifName,instanceId,1) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setIPHostConfigDataSecondaryDNS can't find ip host wanif, fail\n");
			return -1;
		}
		
		getWanInfo(WAN_IF_TYPE_PRI_DNS, ifName, ipAddr);
		
#if 0
		if(strlen(ipAddr) <= 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getIPHostConfigDataIPAddr can't find ip host wanif IP, fail\n");
			return -1;
		}
#endif		
		memset(&address, 0, sizeof(address));
		inet_aton(ipAddr, &address);
			
		put32(value, address.s_addr);
		return 0;
#else
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
#endif

		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}

int32 getIPHostConfigDataCurSecondaryDNS(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
#ifdef TCSUPPORT_PON_IP_HOST
	char ifName[OMCI_LAYER3_ATR_BUFF_SIZE] = {0};
	char ipAddr[OMCI_LAYER3_STR_BUFF_SIZE] = {0};
	struct in_addr address;
#endif

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[1];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	if(((*attributeValuePtr)&IP_OPTION_DHCP_MASK) == 0)//DHCP disable, curAddr == addr
	{
		currentAttribute = &currentME->omciAttriDescriptList[8];
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	else//DHCP enable, curAddr is assigned by DHCP, if not,  undefined
	{
	
#ifdef TCSUPPORT_PON_IP_HOST
		if(getIPHostWanInterface(ifName,instanceId,1) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setIPHostConfigDataSecondaryDNS can't find ip host wanif, fail\n");
			return -1;
		}
			
		getWanInfo(WAN_IF_TYPE_SEC_DNS, ifName, ipAddr);
#if 0
		if(strlen(ipAddr) <= 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getIPHostConfigDataIPAddr can't find ip host wanif IP, fail\n");
			return -1;
		}
#endif		
		memset(&address, 0, sizeof(address));
		inet_aton(ipAddr, &address);
				
		put32(value, address.s_addr);
		return 0;
#else
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(meInstantCurPtr == NULL)
			return -1;
#endif
	
		return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);
}


/*******************************************************************************************************************************
9.4.2: IP host performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForIPHostPM(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForIPHostPM \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	
	return 0;
}

int32 setIPHostPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
#if 0
	uint16 thresholdMeId = 0;
	int ret = 0;
	uint32 thresholdData = 0;
	int i  = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
#endif

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

#if 0
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);

	for(i=1; i<7; i++)
	{
		ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, i, &thresholdData);
		if (ret != 0)
			return -1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		sprintf(tempBuffer, "%x", thresholdData);
		ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, i, tempBuffer);
		if (ret != 0)
			return -1;
	}
#endif

	return 0;
}

int32 getIPHostPMICMPErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 1, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getIPHostPMDNSErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 2, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getIPHostPMIPHostPMDHCPTimeout(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getIPHostPMIPHostPMIPConflict(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getIPHostPMOutOfMemory(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getIPHostPMInternalError(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 6, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

/*******************************************************************************************************************************
9.4.3: TCP/UDP config data

********************************************************************************************************************************/
int omciMeInitForTcpUdpConfigData(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForTcpUdpConfigData \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int32 setTcpUdpDataIpHostPtrFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
    uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
    uint16 instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
    omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
    uint16 IPHostIstId;
    
    setGeneralValue(value, meInstantPtr, omciAttribute, flag);
    IPHostIstId = get16(value);
    OMCI_WARN("==[%s][%d] IPHostIstId = %d ==\r\n",__FUNCTION__,__LINE__,IPHostIstId);

    /* add for ALU olt , in alu olt TR069 management istId index to TCP/UDP istId */
    /*1. ip_host <------- TCP/UDP config data ------ TR069 management */
    currentME = omciGetMeByClassId(OMCI_CLASS_ID_TR069_MANAGE_SERVER);
    meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
    
#if defined(TCSUPPORT_PON_IP_HOST)
    /* there is a TR069 management istid = TCP/UDP cfg data istid */
    if (NULL != meInstantCurPtr){
        OMCI_WARN("==[%s][%d] set IPHostIstId %d to TR069 ==\r\n",__FUNCTION__,__LINE__,IPHostIstId);
        checkIPHostWanInterfaceTimer(WAN_SERVICELIST_TR069, IPHostIstId);
    }
#endif
    return 0;
}

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
#define MECONTENTLENGTH 28
#define REVERSEDMETABLELENGTH 20

static uint8 reserved_353_ME_content[MECONTENTLENGTH]={0x31,0x2e,0x33,0x2e,0x30,0x2e,0x30};
static uint8 reserved_65414_ME_content[MECONTENTLENGTH]={0x01,0x03,0x01,0x1e,0x06,0x48,0x57,0x2d,0x42,0x4f,0x42,0x2d,0x30,0x30,0x30,0x32,0x20,0x20,0x20,0x20,0x20,0x3a,0x0e,0x00,0x0a};

typedef struct omciReservedMe_s
{
	uint16 reservedMeClassId;
	uint16 reservedMeMask;
	uint8* content_ptr;
}omciReservedMe_t,*omciReservedMe_ptr;

omciReservedMe_t reversedMeTable[REVERSEDMETABLELENGTH]=
{
{OMCI_CLASS_ID_RESERVED_353_ME, 		0x0040, 		&reserved_353_ME_content},
{OMCI_CLASS_ID_RESERVED_65414_ME,		0x00ff, 		&reserved_65414_ME_content},
};

void contentReserveMe(uint16 meClassId, uint16 attributeMask, uint8 *responseContentPtr)
{
	int i;
	for (i=0; i<REVERSEDMETABLELENGTH; i++)
	{
		if (meClassId==reversedMeTable[i].reservedMeClassId && attributeMask==reversedMeTable[i].reservedMeMask)
		{
			memcpy(responseContentPtr, reversedMeTable[i].content_ptr,MECONTENTLENGTH*8);
		}
	}
}
#endif

int omciRsvMEGetAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint8 *contentPtr = NULL;
	uint16 attributeMask = 0;
	uint16 responseAttributeMask = 0;
	uint16 excuteAttributeMask = 0;
	uint16 optionalAttributeMask = 0;
	int i = 0;
	int testj = 0;
	uint8 *responseContentPtr = contentPtr + 3; /*the first attribute value start Pointer*/
	uint8 result = 0; 
	int return_value = 0;

	omciManageEntity_ptr currentME = NULL;
	omciAttriDescript_t * currentAttribute =NULL;
	

	int msgContentLength = 0;
	int attributeLength = 0;

	int tableAttriubteNum = 0;
	uint8 attributeNum = 0;
	uint8 devId = 0;
	omciBaselinePayload_t *omciBaselinePayload=NULL;

	char data[OMCI_BASELINE_CONT_LEN] = {0};
	devId = omciPayLoad->devId;
	if(devId == OMCI_BASELINE_MSG)
	{

		omciBaselinePayload = (omciBaselinePayload_t *)omciPayLoad;
		contentPtr = (uint8 *)(omciBaselinePayload->msgContent);
		responseContentPtr = contentPtr +3;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciGetAction:---devId is invalid!\n");	
		return -1;
	}

	attributeMask	= parseAttributeMask(contentPtr);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciGetAction:attributeMask = %x\n", attributeMask);

	responseAttributeMask = attributeMask;
	
#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA	
	contentReserveMe(meClassId,attributeMask,responseContentPtr);
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
	if (0x40 == attributeMask && OMCI_CLASS_ID_RESERVED_350_ME == meClassId)
	{
		memset(omciPayLoad->msgContent,0,OMCI_BASELINE_CONT_LEN);
		omciPayLoad->msgContent[1] = 2; 
	}
#endif
pktSend:

/*MessageType response AR = 0; AK =1*/
	responseMessageTypeHandle((uint8 *)(&omciPayLoad->msgType));
/*write reason and result*/
	responseContentPtr = contentPtr;
	writeResultAndReasonToOmciPayLoad(result, responseContentPtr);
/*write attribute mask*/
	responseContentPtr = contentPtr + 1;
	reparseAttributeMask(responseAttributeMask, responseContentPtr);
	
/*write optional attribute mask*/	
	responseContentPtr = contentPtr + 28;
	reparseAttributeMask(optionalAttributeMask, responseContentPtr);		/*or result reason = 0011*/
/*write excute attribute mask*/
	responseContentPtr = contentPtr + 30;
	reparseAttributeMask(excuteAttributeMask, responseContentPtr);

	//memcpy(omciPayLoad->msgContent,"0200000000000000",strlen("0200000000000000"));

	omciPktSend(omciPayLoad, TRUE); 
	return 0;		
}

#ifdef TCSUPPORT_VNPTT
int omciMeInitFor250ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor250ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor347ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor250ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}
#endif
/*******************************************************************************************************************************
temp add for reserve ME

********************************************************************************************************************************/

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME

int omciMeInitFor350ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor350ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor352ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor352ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor353ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor353ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}


int omciMeInitFor367ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor367ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor373ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor373ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
int omciMeInitFor370ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor370ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor65408ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor65408ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor65414ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor65414ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciMeInitFor65425ME(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitFor65425ME \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciRsvMEGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}
#endif
#endif

int omciMeInitForTcpUdpPM(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForTcpUdpPM \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	
	return 0;
}

/*******************************************************************************************************************************
9.4.4: TCP/UDP performance monitoring history data

********************************************************************************************************************************/

int32 setTcpUdpPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
#if 0
	uint16 thresholdMeId = 0;
	int ret = 0;
	uint32 thresholdData = 0;
	int i = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
#endif

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
#if 0
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);

	for(i=1; i<6; i++)
	{
		ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, i, &thresholdData);
		if (ret != 0)
			return -1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		sprintf(tempBuffer, "%x", thresholdData);
		ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, i, tempBuffer);
		if (ret != 0)
			return -1;
	}
#endif

	return 0;
}

int32 getTcpUdpPMSocketFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 1, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getTcpUdpPMListenFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 2, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getTcpUdpPMBindFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getTcpUdpPMAcceptFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}

int32 getTcpUdpPMSelectFailed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}



