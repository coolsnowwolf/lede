#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include "hal_itf_gpon.h"
#include <arpa/inet.h>
#include "api/mgr_api.h"
#if 0
#include "hal_itf_gpon_mtk.h"
#endif
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if 0
#include "hal_crc32.h"
#include "mgr_flow.h"
#endif

/********************************************************************/
#define DEFAULT_OMCC_TCONT     0
#define CONFIG_GPON_MAX_TCONT  16
#define DEFAULT_ANI_INTERFACE  1
#define PHY_TX_ROGUE_MODE	   1

/********************************************************************/
int halOmciSocket = -1 ;
int g_TcontID_Allocid_Map[CONFIG_GPON_MAX_TCONT] = {0};
struct sockaddr_ll halSocketAddr ;

/********************************************************************/
typedef void (*hal_omci_callback)(void* pv_data, unsigned int length);
static hal_omci_callback  OmciCallbackFunc = NULL;

/********************************************************************/
#if 0
extern unsigned short g_bcast_gempot;
#endif
unsigned short g_bcast_gempot;
void* omciTxRx_thread(void* arg);
void* ploamTxRx_thread(void* arg);
/********************************************************************/
#if 0
static long XMCS_IOCTL_SDI(unsigned int cmd, unsigned long arg)
{
	int fd = -1;
	
	fd = open("/dev/pon", O_RDONLY);
	if (fd < 0)
	{
		printf("Couldn't open %s, fd = %d.\n", "/dev/pon", fd);
		return HAL_RET_FAILED;
	}
	
	if (ioctl(fd, cmd, arg) < 0) 
	{
		printf("do XMCS_IOCTL_SDI failed.Reason is %s\n",strerror(errno));
		close(fd);
		return HAL_RET_FAILED;
	}
	
	close(fd);
	return HAL_RET_SUCCESS;	
}
#endif

/**
* To initialize the hw 
* @param  hw_info the hardware structure about support LAG, outside switch
* @see hal_onu_hw_mode
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @post after this function is compeleted, the client should be able to start operation 
* on the hardware such as to configure  the pon link etc.
*/
hal_ret_t hal_gpon_init(hal_onu_hw_mode* hw_info)
{    
	return HAL_RET_SUCCESS;
}

/* TC related 
***********************************************************/


/**
* To activate the pon link 
* @param  ponLink 
* @see  hal_pon_link_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_activate_gponlink(hal_pon_link_t* ponLink)
{
    int ret = HAL_RET_SUCCESS;  

    if(HAL_RET_SUCCESS != XMCS_IOCTL_SDI(IO_IOS_WAN_DETECTION_MODE,XMCS_IF_WAN_DETECT_MODE_GPON)){
        return HAL_RET_FAILED;
    }
    ret = XMCS_IOCTL_SDI(IO_IOS_WAN_LINK_START, XPON_ENABLE);
    if(HAL_RET_SUCCESS != ret)
    {       
        return HAL_RET_FAILED;
    }
    return HAL_RET_SUCCESS;
}


/**
*First call this api to set onu be into O7 state inf the /config/pon_disable file exists
*Then call hal_gpon_activate_gponlink to be effective
*/
hal_ret_t hal_gpon_set_emergency_state(void)
{
	int ret = HAL_RET_SUCCESS;

	ret = XMCS_IOCTL_SDI(GPON_IOS_EMERGENCY_STATE, XPON_ENABLE);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}
	
	return HAL_RET_SUCCESS;
}

/**
* To deactivate the pon link 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_deactivate_gponlink(void)
{
	int ret = HAL_RET_SUCCESS;

	ret = XMCS_IOCTL_SDI(IO_IOS_WAN_LINK_START, XPON_DISABLE);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}

	return HAL_RET_SUCCESS;
}

/**
* To modify the pon link password (SLID) 
* @param  password
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @note password can be modified after ponlink is activated
*/
hal_ret_t hal_gpon_modify_gponlink_pwd(unsigned char* password)
{
	int ret = HAL_RET_SUCCESS;
	struct XMCS_GponSnPasswd_S gponSnPasswd ;
	memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
	
	if((NULL == password) || (GPON_PASSWD_LENS < strlen((const char*)password)))
	{
		return HAL_RET_FAILED;
	}
	
	memcpy(gponSnPasswd.passwd, password, GPON_PASSWD_LENS);
	ret = XMCS_IOCTL_SDI(GPON_IOS_PASSWD, &gponSnPasswd);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}
	
	return HAL_RET_SUCCESS;
}

/**
* To modify the pon link sequence number 
* @param  sn    
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @note sn can be modified after ponlink is activated
*/
hal_ret_t hal_gpon_modify_gponlink_sn(unsigned char* sn)
{
	int ret = HAL_RET_SUCCESS;
	unsigned int value = 0;
	struct XMCS_GponSnPasswd_S gponSnPasswd ;
	
	memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;	
	memcpy(gponSnPasswd.sn, sn, 8) ;    
	
	ret = XMCS_IOCTL_SDI(GPON_IOS_SN, &gponSnPasswd);
	if(HAL_RET_SUCCESS != ret)
	{	    
		return HAL_RET_FAILED;
	}
	return HAL_RET_SUCCESS;
}

/**
* To get current pon link information 
* @param  ponlinkinfo
* @see hal_pon_link_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_get_gponlink_info(hal_pon_link_t* ponlinkinfo)
{
	int ret = HAL_RET_SUCCESS;
	struct XMCS_GponOnuInfo_S gponOnuInfo ;
	memset(&gponOnuInfo, 0, sizeof(struct XMCS_GponOnuInfo_S));
	
	ret = XMCS_IOCTL_SDI(GPON_IOG_ONU_INFO, &gponOnuInfo);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}
	
	if(gponOnuInfo.EmergencyState)
	{
		ponlinkinfo->initState = HAL_LINK_OPERATION_STATE_EMERGENCY_STOP_O7_E;
	}
	else
	{
		ponlinkinfo->initState = HAL_LINK_OPERATION_STATE_IINIT_O1_E;
	}
	memcpy(ponlinkinfo->serialNo, gponOnuInfo.sn, GPON_SN_LENS);
	memcpy(ponlinkinfo->password, gponOnuInfo.passwd, GPON_PASSWD_LENS);

	switch (gponOnuInfo.state)
	{
		case ENUM_GPON_STATE_O2:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_STANDBY_O2_E;
			break;
		}
		case ENUM_GPON_STATE_O3:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_SN_O3_E;
			break;
		}
		case ENUM_GPON_STATE_O4:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_RANGING_O4_E;
			break;
		}
		case ENUM_GPON_STATE_O5:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_OPERATION_O5_E;
			break;
		}
		case ENUM_GPON_STATE_O6:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_POPUP_O6_E ;
			break;
		}
		case ENUM_GPON_STATE_O7:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_EMERGENCY_STOP_O7_E;
			break;
		}
		default:
		{
			ponlinkinfo->subStatus = HAL_LINK_OPERATION_STATE_IINIT_O1_E ;
			break;
		}
	}
	
	return HAL_RET_SUCCESS;
}

/**
* To set rogue state
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_set_rogue_state(void)
{
	int ret = HAL_RET_SUCCESS;

	//open  PHY_TX_ROGUE_MODE
	ret = XMCS_IOCTL_SDI(PHY_IOS_ROUGE_MODE, PHY_TX_ROGUE_MODE);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}
	
	return HAL_RET_SUCCESS;
}

/**
* To get rx and tx FEC status 
* @param  fecStatus the fect status get from chipset
* @see hal_pon_fec_status_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_get_gponlink_fec_status(hal_pon_fec_status_t* fecStatus)
{
	int ret = HAL_RET_SUCCESS;	
	struct XMCS_PhyTxRxFecStatus_S phyFecStatus;
	memset(&phyFecStatus,0,sizeof(struct XMCS_PhyTxRxFecStatus_S));
	
	ret = XMCS_IOCTL_SDI(PHY_IOG_TX_RX_FEC_STATUS, &phyFecStatus);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}

	fecStatus->rx = phyFecStatus.rx_status;
	fecStatus->tx = phyFecStatus.tx_status;
	
	return HAL_RET_SUCCESS;
}


/**
* To create a TCont  
* @param  TCont the id of the tcont need to be configured
* @param  AllocateID the allocate id of the tcont
* @param tcontPolicy the policy(SP or WRR) of the tcont
* @see hal_qos_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_create_tcont(int TCont,int AllocateID, hal_qos_policy_t tcontPolicy) 
{
	int ret = HAL_RET_SUCCESS;
	struct XMCS_TcontCfg_S tcontCfg;

	memset(&tcontCfg, 0, sizeof(tcontCfg));
	
	tcontCfg.tcontId     = TCont;
	tcontCfg.allocId     = AllocateID;
	tcontCfg.tcontPolicy = tcontPolicy;

	ret = XMCS_IOCTL_SDI(IF_IOS_TCONT_INFO_CREATE, &tcontCfg);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}
	
	//save tcont allocid map
	g_TcontID_Allocid_Map[TCont] = AllocateID;

	return HAL_RET_SUCCESS;
}

/**
* To create a hgu TCont  
* @param  TCont the id of the tcont need to be configured
* @param  AllocateID the allocate id of the tcont
* @param tcontPolicy the policy(SP or WRR) of the tcont
* @see hal_qos_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @deprecated
*/
//hal_ret_t hal_gpon_create_tcont_hgu(int TCont,int AllocateID, hal_qos_policy_t tcontPolicy); 

/**
* To modify a TCont  information
* @param  TCont the id of the  tcont need to be modified
* @param  AllocateID the allocate id of the tcont
* @param tcontPolicy the policy(SP or WRR) of the tcont
* @see hal_qos_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_modify_tcont(int TCont,int AllocateID,hal_qos_policy_t tcontPolicy)
{
	return HAL_RET_SUCCESS;
}

/**
* To delete a TCont
* @param  TCont the id of the  tcont need to be modified
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_delete_tcont(int TCont)
{
	int ret = HAL_RET_SUCCESS;
	unsigned short allocId = 0;
	
	allocId = g_TcontID_Allocid_Map[TCont];
	ret = XMCS_IOCTL_SDI(IF_IOS_TCONT_INFO_REMOVE, allocId);
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_RET_FAILED;
	}
	
	//update tcont allocid map
	g_TcontID_Allocid_Map[TCont] = 0;
	
	return HAL_RET_SUCCESS;
}


/**
* To create a gem port 
* @param  GemPortID the gemport number need to be configured
* @param  gemportType the type of the gemport(unicast, broadcast,multicast)
* @param TContId the tcont that the gemport point to
* @see hal_gemport_type_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_create_gemport(unsigned short GemPortID, hal_gemport_type_t gemportType,unsigned short TContId)
{
	int ret = HAL_RET_SUCCESS;
	struct XMCS_GemPortCreate_S gponGemCreate;
	struct XMCS_GemPortAssign_S gponGemAssign;	
	struct XMCS_OMCC_Info_S Omcc_Info;
	
	memset(&Omcc_Info, 0, sizeof(struct XMCS_OMCC_Info_S));	
	memset(&gponGemAssign, 0, sizeof(struct XMCS_GemPortAssign_S));
	memset(&gponGemCreate, 0, sizeof(struct XMCS_GemPortCreate_S));
	
	if(GEMPORT_TYPE_MULTICAST_E == gemportType)
	{
		gponGemCreate.gemType   = GPON_MULTICAST_GEM ;
		gponGemCreate.gemPortId = GemPortID ;
	}
	else
	{
		gponGemCreate.gemType	= GPON_UNICAST_GEM ;
		gponGemCreate.gemPortId = GemPortID;
                //boardcast traffic should forward to tcont 0 in ASB
		if(GEMPORT_TYPE_BROADCAST_E == gemportType)
		{
			if(DEFAULT_OMCC_TCONT == TContId)
			{
				ret = XMCS_IOCTL_SDI(IF_IOG_OMCC_INFO, &Omcc_Info);
				if(HAL_RET_SUCCESS != ret) 
				{
					return HAL_RET_FAILED;
				}
				gponGemCreate.allocId = Omcc_Info.allocId;
			}
			else
			{
				gponGemCreate.allocId = g_TcontID_Allocid_Map[TContId] ;
			}
		}
		else
		{
			gponGemCreate.allocId = g_TcontID_Allocid_Map[TContId] ;
		}
		
		if(GEMPORT_TYPE_BROADCAST_E == gemportType)
		{
#if 0
			createBcastFlows(GemPortID);
#endif
			g_bcast_gempot = GemPortID;
		}
	}

	ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_CREATE, &gponGemCreate) ;
	if(HAL_RET_SUCCESS != ret) 
	{
		return HAL_RET_FAILED;
	}
	
	gponGemAssign.entryNum = 1;
	gponGemAssign.gemPort[0].id  = GemPortID;
	gponGemAssign.gemPort[0].ani = DEFAULT_ANI_INTERFACE;
	ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_ASSIGN, &gponGemAssign) ;
	if(HAL_RET_SUCCESS != ret) 
	{
		return HAL_RET_FAILED;
	}

	return HAL_RET_SUCCESS;
}
/**
* To delete a gem port and related configuation
* @param  GemPortID the gemport need to be deleted
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_delete_gemport(unsigned short GemPortID)
{
	int ret = HAL_RET_SUCCESS;

	ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_REMOVE, GemPortID) ;
	if(HAL_RET_SUCCESS != ret) 
	{
		return HAL_RET_FAILED;
	}

    if((g_bcast_gempot != INVALID_GEMID) && 
        (GemPortID == g_bcast_gempot))
    {
    	#if 0
        deleteBcastFlows(g_bcast_gempot);
        #endif
        g_bcast_gempot = INVALID_GEMID;
    }

	return HAL_RET_SUCCESS;
}

/**
* To create a gem port  that used the remote debug function
* @param  gemPortId the gemport number need to be configured
* @param  sn the serial number of ont, it may need not be used now
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo rename
*/
hal_ret_t hal_gpon_config_debug_gemPort(unsigned short gemPortId, unsigned int sn)
{
	return HAL_RET_SUCCESS;
}


/**
* To get omci channel gemport id
* @param  pPortId the port id of gemport omci channel
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_ploam_get_OMCIPortID(unsigned short *pPortId)
{
	int ret = 0;
	struct XMCS_OMCC_Info_S Omcc_Info;
	
	memset(&Omcc_Info, 0, sizeof(struct XMCS_OMCC_Info_S));
	ret = XMCS_IOCTL_SDI(IF_IOG_OMCC_INFO, &Omcc_Info);
	if(HAL_RET_SUCCESS != ret) 
	{
		return HAL_RET_FAILED;
	}

	*pPortId = Omcc_Info.gemportid;

	return HAL_RET_SUCCESS;
}

/**
* To send OMCI messages
* @param writeOmci the omci message need to be sent out
* @see hal_write_omci_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_write_omci(hal_write_omci_t* writeOmci)
{
	int result = 0;
	
    writeOmci->Omcibuff[43] = 0x28;
	writeOmci->mesLen = 44;
	
	result = sendto(halOmciSocket, writeOmci->Omcibuff, writeOmci->mesLen, 0, (struct sockaddr*)&halSocketAddr, sizeof(halSocketAddr)) ;
	if(-1 == result) 
	{ 
		return HAL_RET_FAILED;
	}
	
	return HAL_RET_SUCCESS;
}

/**
* To configure a ether type filter to do ratelimit
* @param etyp_filter 
* @see hal_cfg_eth_type_filter_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo move to misc or flow
*/
hal_ret_t hal_gpon_cfg_ethype_filter(hal_cfg_eth_type_filter_t *etyp_filter)
{
	return HAL_RET_SUCCESS;
}

/**
* To configure a dhcp filter
* @param dhcp_filter 
* @see hal_cfg_dhcp_filter_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo merge with ethtype filter, use ethtype or protocol or some reasons
*/
hal_ret_t hal_gpon_cfg_dhcp_filter(hal_cfg_dhcp_filter_t *dhcp_filter)
{
	return HAL_RET_SUCCESS;
}

/**
* To get the ethernet flow id by gemflow id
* @param gemflow  the gemflow id in hal
* @param ethflowId  the ethernet flow id in hal
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo move to internal
*/
hal_ret_t hal_gpon_get_ds_eth_flow(unsigned int gemflow, unsigned int* ethflowId)
{
	return HAL_RET_SUCCESS;
}

/**
* To init the process of receive omci message 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_receive_omci_mesg_init()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, omciTxRx_thread, NULL);
    return HAL_RET_SUCCESS;	
}



	

/**
* To register the call back function which is called when omci message is received
* @param pf_omcicallback  the point of call back function
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_omci_mesg_callback(void *pf_omcicallback)
{
	if(NULL== pf_omcicallback) 
	{
		return HAL_RET_FAILED ;
	}

	OmciCallbackFunc = (hal_omci_callback )pf_omcicallback ;
	
	return HAL_RET_SUCCESS;
}

/**
* To init the process of receive ploam message 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_receive_ploam_mesg_init()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, ploamTxRx_thread, NULL);
    return HAL_RET_SUCCESS;	
}

/**
* To register the call back function which is called when ploam state is changed
* @param pf_snstatecallback  the point of call back function
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo rename
*/
hal_ret_t hal_gpon_ploam_snState_update_callback(void *pf_snstatecallback)
{
	return HAL_RET_SUCCESS;
}

/**
* To set the trace level of gpon mac module in hal
* @param trc_level  the trace level
* @todo merge the trc level
*/
void hal_gpon_set_trc_level(unsigned int trc_level)
{
	return;
}

/**
* To check the gemport is configured in hal or not
* @param gemportId  the gemport Id nee to check
* @return return HAL_RET_FAILED if not configured , return HAL_RET_SUCCESS if configured
*/
hal_bool_t hal_gpon_is_gemport_configured(unsigned short gemportId)
{
	int   i = 0 ;
	int ret = HAL_RET_SUCCESS;
	struct XMCS_GemPortInfo_S gponGemInfo ;
	
	memset(&gponGemInfo, 0, sizeof(struct XMCS_GemPortInfo_S));
	ret = XMCS_IOCTL_SDI(IF_IOG_GEMPORT_INFO, &gponGemInfo) ;
	if(HAL_RET_SUCCESS != ret)
	{
		return HAL_FALSE_E;
	}

	if(gponGemInfo.entryNum > CONFIG_GPON_MAX_GEMPORT) 
	{
		return HAL_FALSE_E;
	} 

	for(i=0; i<gponGemInfo.entryNum; i++) 
	{
		if(gemportId == gponGemInfo.info[i].gemPortId)
		{
		   return HAL_TRUE_E;
		}
	}
	
	return HAL_FALSE_E;
}

/**
* To get the gemflow Id in hal by the gemport Id, used by special function. 
* @param gemportId  the gemport Id 
* @return return the gemflow Id.
*/

unsigned short hal_gpon_get_gemport_flow_id(unsigned short gemportId)
{
	return HAL_RET_SUCCESS;
}

/**
 * * To get the upstream gemflow Id in hal by the gemport Id, used by special function. 
 * * @param gemportId  the gemport Id 
 * * @return return the gemflow Id.
 * */

unsigned short  hal_gpon_get_gemport_us_flow_id(unsigned short gemportId)
{
	return HAL_RET_SUCCESS;
}

/**
* To print the gemport table information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_gemflow_table_info(unsigned short appFlowId)
{
	return;
}


/**
* To print the downstream gemflow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_ds_gem_info(unsigned short appFlowId)
{
	return;
}


/**
* To print the upstream gemflow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_us_gem_info(unsigned short appFlowId)
{
	return;
}


/**
* To print the downstream ani flow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_ds_ani_flow_info(unsigned short appFlowId)
{
	return;
}

/**
* To print the upstream ani flow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_us_ani_flow_info(unsigned short appFlowId)
{
	return;
}


/**
* when operation mode change from uni to nni or vers
* @param mode 
* @see hal_op_mode_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_config_operation_mode(unsigned int mode)
{
	return HAL_RET_SUCCESS;
}

void * omciTxRx_thread(void * arg)
{   
	int          ret = HAL_RET_SUCCESS;   
	int       pktLen = 0 ; 
	unsigned int crc = 0;
	char omci_pkt[HAL_OMCI_BUFF_LEN] = {0};
	struct ifreq ifstruct; 
#if 0
	bl_InitCrc32Table();
#endif
	halOmciSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(halOmciSocket == -1) 
	{ 
		return NULL;
	}

	memset(&ifstruct, 0, sizeof(struct ifreq));
	memset(&halSocketAddr, 0, sizeof(struct sockaddr_ll));
	halSocketAddr.sll_family   = AF_PACKET; 
	halSocketAddr.sll_protocol = htons(ETH_P_ALL);
	strcpy(ifstruct.ifr_name, "omci");
	while (1)
	{
		ret = ioctl(halOmciSocket, SIOCGIFINDEX, &ifstruct); 
		if (HAL_RET_SUCCESS != ret)
		{
			sleep(2);
		}
		else
		{
			break;
		}
	}
   
	halSocketAddr.sll_ifindex= ifstruct.ifr_ifindex;
	if(bind(halOmciSocket, (struct sockaddr *)&halSocketAddr, sizeof(halSocketAddr)) == -1)
	{ 
		close(halOmciSocket);
		halOmciSocket = -1;
		return NULL;
	}

	while(1)
	{      	 
		pktLen = recvfrom(halOmciSocket, omci_pkt, HAL_OMCI_BUFF_LEN, 0, NULL, NULL) ;
		if(pktLen <= 0)	   
		{
			continue ;
		}
#if 0
		crc = hal_gpon_generate_crc32((unsigned char*)omci_pkt,44);      
		memcpy(&omci_pkt[44], &crc, 4);
		pktLen = pktLen + 4;    
#endif
		OmciCallbackFunc(omci_pkt,pktLen);
		memset(omci_pkt, 0, sizeof(omci_pkt));
	}   
   return NULL;

}


void * ploamTxRx_thread(void * arg)
{
   struct XMCS_PonEventStatus_S ponEventStatus ;
   unsigned int i = 0 ;

   while(1) 
   {
		if(HAL_RET_SUCCESS != XMCS_IOCTL_SDI(FDET_IOG_WAITING_EVENT, &ponEventStatus))
		{
			continue ;
		}

		for(i=0 ; i<ponEventStatus.items ; i++)
		{
			if(ponEventStatus.event[i].type == XMCS_EVENT_TYPE_GPON)
			{
				switch(ponEventStatus.event[i].id)
				{
					case XMCS_EVENT_GPON_LOS:
						break;
					case XMCS_EVENT_GPON_PHY_READY:
						break;
					case XMCS_EVNET_GPON_DEACTIVATE:
						break;
					case XMCS_EVENT_GPON_ACTIVATE:
						break;
					case XMCS_EVENT_GPON_TCONT_ALLOCED:
						break;
					case XMCS_EVENT_GPON_MODE_CHANGE:
						break;
					case XMCS_EVENT_GPON_PHY_TRANS_ALARM:
						break;
					case XMCS_EVENT_GPON_MAX_ID:
						break;
					case XMCS_EVENT_GPON_DISABLE:
						break;
					case XMCS_EVENT_GPON_ENABLE:
						break;
					case XMCS_EVENT_GPON_STATE_CHANGE:
						break;
					default:
						break;
				}				
			}	
			memset(&ponEventStatus, 0, sizeof(struct XMCS_PonEventStatus_S));			
			continue ;
		}
   }
}


