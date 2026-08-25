#ifndef _HAL_CTQOS_H_
#define _HAL_CTQOS_H_

#include "hal_type_qos.h"
#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"{
#endif

#define INVALID_ENTRY -1
#define DSCP_REMARK_ENABLE 1
#define PBIT_REMARK_ENABLE 2
#define PROTOCAL_ICMP 1
#define PROTOCAL_EXCLUDE 1
#define TR069_APP 1
#define VOIP_APP 2
#define LAN2 2
/* voice rtp port range: 49152(0xC000) ~ 49407(0xC0FF), sip signal port: 5060 */
#define VOIP_RTP_PORT_START 49152
#define VOIP_RTP_PORT_END 49407
#define VOIP_SIP_SIG_PORT 5060


typedef enum
{
   QOS_COMMAND_NONE     = 0,
   QOS_COMMAND_CONFIG   = 1,
   QOS_COMMAND_UNCONFIG = 2

}QosCommandType;

typedef struct
{
    unsigned int class_id;
    int enable;
    int order;
    //char lanif[32];
    int lan_port;
    char wanif[32];
    //struct in_addr dest_ip;
    char dest_ip[32];
    unsigned int dest_ip_mask;
    int dest_ip_exclude;
    //struct in_addr src_ip;
    char src_ip[32];
    unsigned int src_ip_mask;
    int src_ip_exclude;
    int proto;
    int proto_exclude;	
    int dport;
    unsigned int dport_max;
    int dport_exclude;
    int sport;
    unsigned int sport_max;
    int sport_exclude;
    char smac[6];
    unsigned int smac_mask;
    char dmac[6];
    unsigned int dmac_mask;
    int priority;
    int dscp;
    int dscp_max;	
    int tos;
    int ether_type;
    hal_tr069_voip_info_t tr069_voip_classify;
}classify_t; 


extern void hal_hgu_qos_init(void);

extern int hal_hgu_qos_create_classify(hal_qos_classify_t classify_info);

extern int hal_hgu_qos_delete_classify(int classify_id);

extern int hal_hgu_qos_create_policer(hal_qos_policer_t policer_info);

extern int hal_hgu_qos_delete_policer(int policer_id);

extern int hal_hgu_qos_create_flow(hal_qos_flow_t flow_info);

extern int hal_hgu_qos_delete_flow(int flow_id);

extern int hal_hgu_qos_config_overall_ratelimit(int bandwidth);

extern int hal_hgu_qos_config_queue_rate(int queueId, int cir);



extern int hal_hgu_qos_create_classify_v6(hal_v6_qos_classify_t classify_info);

extern int hal_hgu_qos_create_flow_v6(hal_qos_flow_t flow_info);

extern int hal_hgu_qos_delete_flow_v6(int flow_id);

extern void hal_hgu_qos_telmex_hsi_classify(void);

extern void hal_hgu_qos_telmex_igmp_classify(void);

extern void hal_hgu_qos_telmex_voip_classify(void);


#ifdef __cplusplus
}
#endif


#endif

