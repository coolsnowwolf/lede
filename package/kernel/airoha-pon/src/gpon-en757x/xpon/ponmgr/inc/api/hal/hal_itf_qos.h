/**
*@file
*@brief This file defines qos related interfaces that should be implemented by chip vendors 
*@author Shen Jingguo
*@date     2013.3.10
*@copyright 
*******************************************************************
* Copyright (c) 2013 Alcatel Lucent Shanghai Bell Software, Inc.
*             All Rights Reserved
*
* This source is confidential and proprietary and may not
* be used without the written permission of Alcatel Lucent
* Shanghai Bell, Inc.
********************************************************************
*@par Module History:
*/

#ifndef _HAL_QOS_H_
#define _HAL_QOS_H_

#include "hal_type_qos.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C"{
#endif

/**
* To get us priority level of a special priory queue when connect with OLT7360
* @param  pqid
* @return priority level
*/
extern char hal_qos_get_us_prilevel_from_pqid(unsigned int pqid);

/**
* To get us priority level of a special gemport
* @param  pbit the pbit that gemport used for
* @param  gemport_array[] a group of gemports
* @return priority level
*/
unsigned char hal_qos_get_us_prilevel(unsigned int pbit, unsigned short gemport_array[]);

/**
* To update priority level of a special  priority queue
* @param  pqid the priority queue id
* @param  pri_level priority level of the queue  
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_update_queue_prilevel(unsigned short pqid, unsigned short pri_level);

/**
* To init downstream emac queue
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_ds_init_emac_queue(void);

/**
* To init downstream something about configures
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_ds_init_cfgs(void);

/**
* To init upstream queue which is used to upload to olt 
* @param  index index of queue
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_queue_t* hal_qos_init_us_queue(unsigned short index);

/**
* To init downstream queue which is used to upload to olt 
* @param  index index of queue
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_queue_t* hal_qos_init_ds_queue(unsigned short index);

/**
* To reset queue 
* @param  none
* @return none
*/
void hal_qos_reset_queue(void);

/**
* To get us queue info 
* @param  pq id
* @see  pq_id
* @return return NULL if failed, return queue info if success
*/
hal_queue_t*  hal_qos_get_us_pq_info(unsigned short  pq_id);

/**
* To update queue info 
* @param  pq_info such as pq id, tschedPtr, weight and so on
* @see  hal_queue_t
* @return return 0 if failed, return 1 if success
*/
hal_ret_t hal_qos_update_pq_info(hal_queue_t pq_info);


/**
* To get pbit and vlan id  from gemport
* @param  us_ds zero for upstream, one for downstream
* @param  flow_id ethernet flow identifier,
* @param  gemport_array[] a group of gemports
* @param  vlan_id  the vlan id get by the API
* @param  pbit the pbit get by the API
* @param  remark_flag the remark flag get by API
* @return return none
* @deprecated move to internal function
*/
void hal_qos_get_vlan_id_from_gemport(unsigned int us_ds, unsigned int flow_id, unsigned short gemport_array[], unsigned short *vlan_id, unsigned char *pbit, hal_bool_t *remark_flag);

/**
* To create a qos policer
* @param  policer_info such as policer_id cir and pir
* @see  hal_qos_policer_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_create_policer(hal_qos_policer_t policer_info);

/**
* To modify a qos policer
* @param  policer_info such as policer_id cir and pir
* @see  hal_qos_policer_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_modify_policer(hal_qos_policer_t policer_info);

/**
* To delete a qos policer 
* @param  policer_id policer identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_delete_policer(unsigned int policer_id);

/**
* To map ethflow to policer sfu  
* @param  dir   direction
* @param  flow_id   app flowid identifier
* @param  policer_id   policer identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_map_ethflow_to_policer_sfu(hal_direction_t dir, unsigned int flow_id, unsigned int policer_id);

/**
* To unmap ethflow to policer sfu  
* @param  dir   direction
* @param  flow_id   app flowid identifier
* @param  policer_id   policer identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_unmap_ethflow_to_policer_sfu(hal_direction_t dir, unsigned int flow_id, unsigned int policer_id);

/**
* To map uni ethflow to policer sfu  
* @param  hal_port_info   uni port info
* @param  policer_id   policer identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_map_us_uni_to_policer_sfu(hal_port_info_t hal_port_info,unsigned int policer_id);

/**
* To unmap uni ethflow to policer sfu  
* @param  hal_port_info   uni port info
* @param  policer_id   policer identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_unmap_us_uni_to_policer_sfu(hal_port_info_t hal_port_info,unsigned int policer_id);

/**
* To config ds uni rate limit 
* @param  hal_port_info   uni port info
* @param  af_rate  ds uni ratelimit value
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo rename
*/
hal_ret_t  hal_qos_set_ds_unirate(hal_port_info_t hal_port_info,unsigned int af_rate);

/**
* To create gemport to policer info
* @param  gemport_info such as gemport id, tcont upstream policer identifier 
*         and downstream policer identifier
* @see  hal_qos_gemport_info_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @deprecated
*/
hal_ret_t hal_qos_create_gemport_to_policer_info(hal_qos_gemport_info_t  gemport_info);

/**
* To unmap upstream gemport to policer
* @param  gemport_id each gemport
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_delete_gemport_to_policer_info(unsigned short gemport_id);

/**
* To check if necessary delete some resouce (usServiceFlow, scheduler...) before delete tcont
* @param  tcontId
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo ,local function
*/
hal_ret_t hal_qos_prepare_delete_tcont( unsigned int tcont_id);

/**
* To map upstream gemport to policer
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_map_us_gemport_to_policer_sfu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To unmap upstream gemport to policer
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_unmap_us_gemport_to_policer_sfu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To unmap upstream gemport to policer
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_unmap_us_gemport_to_policer_hgu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To map upstream gemport to policer
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_map_us_gemport_to_policer_hgu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To map downstream ethflow to policer 
* @param  flow_id ethernet flow identifier
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_map_ds_ethflow_to_policer_sfu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To map downstream ethflow to policer
* @param  flow_id ethernet flow identifier
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_map_ds_ethflow_to_policer_hgu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To unmap downstream ethflow to policer 
* @param  flow_id ethernet flow identifier
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_unmap_ds_ethflow_to_policer_sfu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To unmap downstream ethflow to policer
* @param  flow_id ethernet flow identifier
* @param  a group of gemports
* @see  gemport_array
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_unmap_ds_ethflow_to_policer_hgu(unsigned int flow_id, unsigned short gemport_array[]);

/**
* To configure dscp to pbit mapper
* @param  hal_port_info  uni port the dscp to pbit mapper configure to such user port
* @param  dscpPbitMapping dscp to pbit map
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo rename
*/
hal_ret_t hal_qos_cfg_dscp_to_pbit(hal_port_info_t hal_port_info, unsigned char dscpPbitMapping[24]);

/**
* To modify emac mtu size 
* @param  mtu max trans unit per the system
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_modify_emac_mtu_size(unsigned short mtu);

/**
* To add  downstream L3 rule of GMP classifier,  classify the downstream by the IP+Port(or IP, port only),
* it is effective for the all downstream,
* @param  appClassifer_id the classifer id
* @param  keyMask which mask of the key words, indicate the classifier parameters,ip , port or both
* @param  destIp the destination ip address of frame
* @param  destPort the destination port of the frame
* @param  appFlowId if the packets not to cpu, the packets should be dealed with same as which traffic
* @param  isPacketToCPU the packets need to trap to cpu or not 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_classify_ds_add_l3_dest_rule(unsigned int appClassifer_id,unsigned int keyMask, unsigned int destIp, unsigned int destPort, unsigned int appFlowId,hal_bool_t isPacketToCPU);

/**
* To remove downstream L3 rule of GMP classifier
* @param  appClassifer_id the classifer id
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_qos_classify_ds_remove_l3_dest_rule(unsigned int appClassifer_id);


/**
* To init qos classifcation method when system is up
* @return none
*/
void hal_qos_init_tc_method(void);

/**
* To print qos classification info for debug
* @param  str_cmd debug command string: flow, classify, sub_classify, other
* @return none
*/
void hal_qos_dbg_dump_qos_data(char *str_cmd);

/**
* To get qos gemport info by gemport Id
* @param  gemport_id the information of the gemport need to get
* @return hal_qos_gemport_info_db_t
*/
hal_qos_gemport_info_db_t * hal_qos_get_gemport_info(unsigned int gemport_id);

/**
* To set the traceLevel of qos function
* @param  traceLevel the trace level need to set
* @return hal_qos_gemport_info_db_t
*/

void hal_qos_set_trace_level(unsigned int traceLevel);

/**
* To set the type of OLT  that ONU connect to, support two mode: 7342 and 7360
* @param  olt_type  the olt type
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_config_olt_info(unsigned int olt_type);
/**
* To get the type of OLT  that ONU connect to
* @return  olt type
*/
int hal_get_olt_info(void);

/**
* To set the traffic classify for downstream or upstream, but only used by HGU now
* @param  traClassify  the traffic classify information
* @see hal_qos_traffic_classify_t
* @return  0 is success, else failed
*/

int hal_qos_traffic_classify_handle(hal_qos_traffic_classify_t* traClassify);

/**
* To get the hal policy id by gemport id
* @param  gemport_id  the gemport id
* @return  policy id
*/
int  hal_qos_get_ds_policy_id_from_gemport(unsigned int gemport_id);

#ifdef __cplusplus
}
#endif

/**
* To configure wifi bridge mode of upstream
* @param  flow_id,  gemport_array[]
* @return return 0 if failed, return 1 if success
*/
extern int hal_qos_config_us_wifi_bridge_mode (unsigned int flow_id,  unsigned short gemport_array[]);
/**
* To remove wifi bridge mode of upstream
* @param  flow_id, lan_port, gemport_array[]
* @return return 0 if failed, return 1 if success
*/
extern int hal_qos_remove_us_wifi_bridge_mode (unsigned int flow_id, hal_port_info_t hal_port_info,  unsigned short gemport_array[]);

/**
* To print wifi bridge info for debug
* @param  none
* @return none
*/
extern void hal_qos_dump_us_wifi_bridge_info();

/**
* To init tsched 
* @param  none
* @return none
*/
extern void hal_qos_init_tsched(void);

/**
* To configure tsched info
* @param  tsched_info
* @return return 0 if failed, return 1 if success
*/
extern hal_ret_t hal_qos_config_tsched_info(hal_qos_tsched_info_t tsched_info);

/**
* To get tsched info
* @param  tsched_id
* @return return 0 if failed, return 1 if success
*/
extern hal_ret_t hal_qos_get_tsched_info(unsigned short tsched_id, hal_qos_tsched_info_t *tsched_info_p);

//ALU01888760
int hal_config_port_to_allow_all_packets(hal_port_info_t hal_port_info);
int hal_config_port_to_only_allow_802_1x_packets(hal_port_info_t hal_port_info);
int hal_is_sfu_system(void);

#endif

