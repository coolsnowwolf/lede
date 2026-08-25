#ifndef _GPON_HOTPLUG_H_
#define _GPON_HOTPLUG_H_
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/kobject.h>
/***********************************************
	GPON Hotplug parameter define
***********************************************/
struct fb_event {
	const char		*name;
	char			*action;

	struct sk_buff		*skb;
	struct work_struct	work;
};

#ifndef container_of
#define container_of(ptr, type, member) (			\
	(type *)( (char *)ptr - offsetof(type,member) ))
#endif

#define FH_SKB_SIZE	2048

extern u64 uevent_next_seqnum(void);

typedef enum {
	REMOVE_FIBER = 0,
	INSERT_FIBER
} Fiber_Hotplug_Type_t ;

int fh_event_add_var(struct fb_event *event, int argv, const char *format, ...);
int fiber_hotplug_fill_event(struct fb_event *event);
void fiber_hotplug_work(struct work_struct *work);
int fiber_hotplug_create_event(const char *name, Fiber_Hotplug_Type_t fiber_hotplug_type);
void fiber_hotplug_event(Fiber_Hotplug_Type_t fiber_hotplug_type);


#endif /* _GPON_HOTPLUG_H_ */


