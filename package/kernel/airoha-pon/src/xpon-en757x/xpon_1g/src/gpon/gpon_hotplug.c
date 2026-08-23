#include <linux/kernel.h>
#include <linux/string.h>

#include "common/drv_global.h"
#include "gpon/gpon.h"
#include "gpon/gpon_ploam.h"
#include "xmcs/xmcs_gpon.h"
#include "gpon/gpon_hotplug.h"

int fh_event_add_var(struct fb_event *event, int argv, const char *format, ...)
{
	static char buf[128];
	char *s;
	va_list args;
	int len;

	if (argv)
		return 0;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	if (len >= sizeof(buf)) {
		PON_MSG(MSG_ERR,"[%s][%d]buffer size is too small\n",__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	s = skb_put(event->skb, len + 1);
	strcpy(s, buf);
	
	PON_MSG(MSG_TRACE,"[%s][%d]ADD Param:added variable '%s'\n",__FUNCTION__,__LINE__,s);

	return 0;
}

int fiber_hotplug_fill_event(struct fb_event *event)
{
	int ret = 0;
		
	PON_MSG(MSG_TRACE,"[%s][%d]fiber hotplug start to fill event\n",__FUNCTION__,__LINE__);
	
	ret = fh_event_add_var(event, 0, "SUBSYSTEM=%s", "fiber");
	if (ret)
	{
		PON_MSG(MSG_ERR, "ERROR:add param SUBSYSTEM error!The ret is %d\n",ret);
		return ret;
	}

	ret = fh_event_add_var(event, 0, "ACTION=%s", event->action);	
	return ret;	
}

void fiber_hotplug_work(struct work_struct *work)
{
	struct fb_event *event = container_of(work, struct fb_event, work);
	int ret = 0;
	
	PON_MSG(MSG_TRACE, "[%s][%d]Init hotplug work start\n",__FUNCTION__,__LINE__) ;		

	event->skb = alloc_skb(FH_SKB_SIZE, GFP_ATOMIC);

	if (!event->skb)
		goto out_free_event;

	ret = fiber_hotplug_fill_event(event);
	if (ret)
		goto out_free_skb;
	
	NETLINK_CB(event->skb).dst_group = 1;
	
	PON_MSG(MSG_TRACE, "[%s][%d]broadcast event start\n",__FUNCTION__,__LINE__) ;		
	/*broadcast_uevent(event->skb, 0, 1, GFP_ATOMIC);*/ /*new openwrt need patch to support it*/
	
out_free_skb:
   if (ret) {
   	   PON_MSG(MSG_ERR, "work error!The ret is %d\n",ret);
	   kfree_skb(event->skb);
   }

out_free_event:
	   kfree(event);
}

int fiber_hotplug_create_event(const char *name, Fiber_Hotplug_Type_t fiber_hotplug_type)
{
	struct fb_event *event;

	PON_MSG(MSG_TRACE, "[%s][%d]create event:name = %s,fiber hottype = %d\n",__FUNCTION__,__LINE__,name, fiber_hotplug_type) ;		

	/*create the environment buffer*/
	event = kzalloc(sizeof(*event), GFP_ATOMIC);
	if (!event)
			return -ENOMEM;

	event->name = name;
	event->action = fiber_hotplug_type ? "insert" : "pull";

	PON_MSG(MSG_TRACE, "[%s][%d]event action is  = %s\n",__FUNCTION__,__LINE__, event->action) ;		

	INIT_WORK(&event->work, (void *)(void *)fiber_hotplug_work);

	PON_MSG(MSG_TRACE, "[%s][%d]schedule work ready to start\n",__FUNCTION__,__LINE__) ;		
	schedule_work(&event->work);
	return 0;
}

void fiber_hotplug_event(Fiber_Hotplug_Type_t fiber_hotplug_type)
{
	int ret = 0;

	if(!gpGponPriv->gTypeOfHotplug)
	{
		PON_MSG(MSG_TRACE, "[%s][%d]Hotplug func now is off!\n",__FUNCTION__,__LINE__) ;
		return;
	}
	else
	{
		if((fiber_hotplug_type != INSERT_FIBER) && (fiber_hotplug_type != REMOVE_FIBER))
		{
			return;
		}
		if(fiber_hotplug_type == INSERT_FIBER)
		{
			PON_MSG(MSG_TRACE, "[%s][%d]Receive fiber sw signal,the current type is %s\n",__FUNCTION__,__LINE__,"Insert_Fiber") ;
		}
		else
		{
			PON_MSG(MSG_TRACE, "[%s][%d]Receive fiber sw signal,the current type is %s\n",__FUNCTION__,__LINE__,"Remove_Fiber") ;		
		}
		ret = fiber_hotplug_create_event("fiber_status", fiber_hotplug_type);
		
		if(ret != 0)
		{
			PON_MSG(MSG_ERR, "[%s][%d]GPON Hotplug: fiber hotplug create event fail\n",__FUNCTION__,__LINE__) ;
		}
	}
	
}

