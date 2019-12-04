#ifndef _MT_WLAN_HOOK_H_
#define _MT_WLAN_HOOK_H_

#include "rt_config.h"

#define MT_WLAN_NAME_MAX 128


/*define hook point, if you want to add a new point, please define it in here*/
typedef enum {
	WLAN_HOOK_FIRST = 0,
	WLAN_HOOK_HIF_INIT = WLAN_HOOK_FIRST,
	WLAN_HOOK_HIF_EXIT,
	WLAN_HOOK_PEER_LINKUP,
	WLAN_HOOK_PEER_LINKDOWN,
	WLAN_HOOK_TX,
	WLAN_HOOK_SYS_UP,
	WLAN_HOOK_SYS_DOWN,
	WLAN_HOOK_ISR,
	WLAN_HOOK_DMA_SET,
	WLAN_HOOK_SER,
	WLAN_HOOK_END
} WLAN_HOOK_PT;


/*hook link list will handle the order depend on below priority in the same hook point.
   0: priority low ~ 100: priority high
*/
typedef enum {
	WLAN_HOOK_PRI_LOWEST = 0,
	WLAN_HOOK_PRI_WOE = WLAN_HOOK_PRI_LOWEST,
	WLAN_HOOK_PRI_HIGHEST = 100,
} WLAN_HOOK_PRI;


struct mt_wlan_hook_ops {

	struct _DL_LIST list;
	const char name[MT_WLAN_NAME_MAX];
	unsigned int (*fun)(unsigned short hook, void *ad, void *priv);
	unsigned short priority;
	unsigned short hooks;
};

struct mt_wlan_hook_ctrl {
	struct _DL_LIST hook_head;
};

void mt_wlan_hook_init(void);
int mt_wlan_hook_call(unsigned short hook, void *ad, void *priv);
int mt_wlan_hook_register(struct mt_wlan_hook_ops *ops);
int mt_wlan_hook_unregister(struct mt_wlan_hook_ops *ops);

#endif /*_MT_WLAN_HOOK_H_*/
