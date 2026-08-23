#ifndef TC_VLANFILTER_H_
#define TC_VLANFILTER_H_

#include <linux/list.h>

#define VLAN_FILTER_DEVNAME			"vlanfilter"
#define VLAN_FILTER_MAJOR			(238)

#define VLAN_FILTER_LIST_ADD 0
#define VLAN_FILTER_LIST_DELETE 1

#define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */
#define ETH_P_QinQ_88a8	0x88a8          /*  VLAN Extended Header  */
#define ETH_P_QinQ_9100	0x9100          /*  VLAN Extended Header  */

enum vlanFilterListResult
{
	OP_SUCCESS,
	OP_FAIL,
	ADD_ALREADY_EXIST,
	DELETE_NOT_FOUND,
};

typedef struct vlanfilter_args_s 
{
	unsigned short vid;
	enum vlanFilterListResult result;
}vlanfilter_args_t;

typedef struct vlanfilter_whitelist_s
{
	unsigned short vid;
	struct list_head list;
}vlanfilter_whitelist_t;
#endif

