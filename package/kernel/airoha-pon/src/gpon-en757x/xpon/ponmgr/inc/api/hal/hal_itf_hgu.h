#ifndef _HAL_ITF_HGU_H_
#define _HAL_ITF_HGU_H_
/*
History:
  2012.10.01 Created By Lou Shengyong
*/

#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <netinet/in.h>

#include "hal_type_flow.h"
#include "hal_type_ethernet.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
    PBM_VID,
    PBM_GEM_VID,
    PBM_VID_PBIT,
    PBM_GEM_VID_PBIT
}PktBasedMethod;

/* downstream vlan action per port */
typedef struct
{
    hal_header_t halHeader;
    unsigned int flowId;
    unsigned int portId;
    unsigned int vlanId;
} hal_vlan_action_per_port_t;

/* vlan action. */
typedef struct
{
    hal_header_t halHeader;
    unsigned int vlanEntryId;
    hal_vlan_tag_action_t type;
    unsigned short tagMode;
    unsigned short innerVId;
    unsigned int innerPbit;
    unsigned short outerVId;
    unsigned int outerPbit;
} hal_vlan_action_t;

typedef struct
{
    hal_header_t halHeader;
    unsigned int vlanId;
    unsigned int rateControlId;
    unsigned int ethFlowId;
    unsigned int gemFlowId;
} hal_ds_dmm_eth_flow_policer_t;


/**** the following functions are also used in HGU mode, so export them ****/
int hal_vlan_action_per_port_handle(hal_vlan_action_per_port_t* vlanActionPerPort);
int hal_vlan_action_handle(hal_vlan_action_t* vlanAction);

int hal_eth_flow_handle(hal_eth_flow_t* ethFlow);

int hal_ds_dmm_map_ethflow_to_rateController_handle(hal_ds_dmm_eth_flow_policer_t *dsVid2RateCtrl);
int hal_ds_remove_rateController_handle(hal_ds_dmm_eth_flow_policer_t *dsDmmRateCtrl);
/**** ***************************************************************** ****/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif



