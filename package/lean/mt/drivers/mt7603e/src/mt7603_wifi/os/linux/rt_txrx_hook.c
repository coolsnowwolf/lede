#include <os/rt_linux_txrx_hook.h>
#include "rt_config.h"


/* Registration hooks for tx/rx path. */
struct list_head hookHead;

void RtmpOsTxRxHookInit(VOID)
{
	INIT_LIST_HEAD(&hookHead);
}


int RtmpOsTxRxHookCall(unsigned short hook,struct sk_buff *skb,unsigned char queIdx, void *priv)
{
	struct mt_wlanTxRxHookOps *entry;
	struct list_head *next,*cur;

	list_for_each_safe(cur,next,&hookHead)
	{
		entry = list_entry(cur,struct mt_wlanTxRxHookOps,list);
		if(entry->hooks & 1 << hook)
		{
			entry->hookfun(hook,skb,queIdx,priv);
		}
	}
	return 0;
}


int RtmpOsTxRxHookRegister(struct mt_wlanTxRxHookOps *ops)
{
	struct mt_wlanTxRxHookOps *entry;
	struct list_head *next,*cur;

	if(!ops)
	{
		return -1;
	}
	
	list_for_each_safe(cur,next,&hookHead)
	{
		entry = list_entry(cur,struct mt_wlanTxRxHookOps,list);
		if(entry && ops->priority > entry->priority)
		{
			(ops->list).next = next;
			cur->next =& ops->list;
			return 0;
		}
	}

	/*add first entry*/
	list_add(&ops->list,&hookHead);

	return 0;
}


int RtmpOsTxRxHookUnRegister(struct mt_wlanTxRxHookOps *ops)
{
	struct mt_wlanTxRxHookOps *entry;
	struct list_head *next,*cur;

	if(!ops)
	{
		return -1;
	}
	
	list_for_each_safe(cur,next,&hookHead)
	{
		entry = list_entry(cur,struct mt_wlanTxRxHookOps,list);
		if(ops==entry)
		{
			list_del(&entry->list);
			return 0;
		}
	}
	return -1;
}


#ifdef MT_MAC
/*Export for modules*/
INT32 RtmpOsRfRead(RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value)
{
	INT32 ret;
	
	ret =  CmdRFRegAccessRead(pAd, RFIdx, Offset, Value);
	return ret;
}

EXPORT_SYMBOL(RtmpOsRfRead);


INT32 RtmpOsMemIORead(RTMP_ADAPTER *pAd,UINT32 addr,UINT32 *value)
{
	INT32 ret=0;
	RTMP_IO_READ32(pAd,addr,value);
	return ret;
}

EXPORT_SYMBOL(RtmpOsMemIORead);


INT32 RtmpOsRfTest(RTMP_ADAPTER *pAd, UINT8 Action, UINT8 Mode, UINT8 CalItem)
{
   	INT32 ret;
   	ret = CmdRfTest(pAd, Action, Mode, CalItem);
	return ret;

}
EXPORT_SYMBOL(RtmpOsRfTest);
EXPORT_SYMBOL(RtmpOsGetNetDevPriv);
EXPORT_SYMBOL(AsicSwitchChannel);
#endif /*MT_MAC*/


EXPORT_SYMBOL(RtmpOsTxRxHookRegister);
EXPORT_SYMBOL(RtmpOsTxRxHookUnRegister);
