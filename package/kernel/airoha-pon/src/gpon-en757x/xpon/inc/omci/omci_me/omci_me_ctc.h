#ifndef _CTC_ME_H_
#define _CTC_ME_H_
#include "omci_types.h"
#include "omci_me.h"
#include "omci_me_multicast.h"

typedef struct portList_s{
uint16 portNum;
uint16 port[4];
}portList_t,*portList_Ptr;
typedef struct omciIgmpLostGroupInfo_s{
uint16 vid;
uint32 srcIp;
uint8 multicastIp[16];
}omciIgmpLostGroupInfo_t,*omciIgmpLostGroupInfo_Ptr;

typedef struct igmpLostGroupInst_s{
struct igmpLostGroupInst_s *next;
omciIgmpLostGroupInfo_Ptr value;
}igmpLostGroupInst_t,*igmpLostGroupInst_Ptr;


#define OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH 				30
#define OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH 			120
#define OMCI_EMOP_ACL_TABLE_ENTRY_SET_CTRL_FILED    		14
#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_FILED     	12
#define OMCI_EMOP_ACL_TABLE_ENTRY_SET_CTRL_MASK		0xc000
#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_MASK		0x3000
#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_ID_MASK			0x1ff

#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_0		0
#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_1		1
#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_2		2
#define OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_3		3




#define MAX_UNI_PORT 4
#define IP_ADDR_LEN 16


int32 getOperatorIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag);
int32 getCtcLOIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag);
int32 getCtcPasswordValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag);
int setAuthStatusValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int setEMOPMulticastAclTableValueByType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, IN uint16 attrIndex);
int setEMOPMulticastDynamicAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setEMOPMulticastStaticAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int omciInitInstForOnuCapability(void);
int omciInitInstForLoidAuthen(void);

int omciMeInitForOnuCapability(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForLoidAuthen(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForExtendMulticastProfile(omciManageEntity_t *omciManageEntity_p);

extern omciAttriDescript_t omciAttriDescriptListOnuCapability[];
extern omciAttriDescript_t omciAttriDescriptListLoidAuthentication[];
extern omciAttriDescript_t omciAttriDescriptListExtMulticastProfile[];
#endif
