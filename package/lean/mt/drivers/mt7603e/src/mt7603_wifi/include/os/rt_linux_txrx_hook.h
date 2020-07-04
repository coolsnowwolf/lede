#ifndef _RT_LINUX_TXRX_HOOK_H_
#define _RT_LINUX_TXRX_HOOK_H_

#include <linux/list.h>
#include <linux/skbuff.h>
#define MT_WLAN_TXRXHOOK_NAME_MAX 128

/*define hook point, if you want to add a new point, please define it in here*/
typedef enum {

	WLAN_TX_IN=0,
	WLAN_TX_ENQUEUE,
	WLAN_TX_DROP,
	WLAN_TX_DEQUEUE,
	WLAN_TX_DMA_DONE,
	WLAN_TX_PRETBTT_INTR,
	WLAN_TX_DEQUEUE_PROGRESS,
	WLAN_TX_ENQUEUE_PROGRESS,
	WLAN_TX_MLME_PERIOD,
	WLAN_CALIB_TEST_RSP,
	
} WLAN_HOOK_PT_T ;


/*hook link list will handle the order depend on below priority in the same hook point.
   0: priority low ~ 100: priority high
*/
typedef enum {
	WLAN_HOOK_PRI_LOWEST=0,
	WLAN_HOOK_PRI_SOFTQ_STAT=WLAN_HOOK_PRI_LOWEST,
	WLAN_HOOK_PRI_HIGHEST=100,
} WLAN_HOOK_PRI_T;


struct mt_wlanTxRxHookOps {
	
	struct list_head list;
	const char name[MT_WLAN_TXRXHOOK_NAME_MAX];
	unsigned int (*hookfun)(unsigned short hook, struct sk_buff *skb,unsigned char queIdx,void *priv);
	struct module *me;
	unsigned short priority;
	unsigned short hooks;
};


void RtmpOsTxRxHookInit(void);
int RtmpOsTxRxHookCall(unsigned short hook,struct sk_buff *skb, unsigned char queldx,void *priv);
int RtmpOsTxRxHookRegister(struct mt_wlanTxRxHookOps *ops);
int RtmpOsTxRxHookUnRegister(struct mt_wlanTxRxHookOps *ops);

#endif
