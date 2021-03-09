#ifndef __WIFI_SYS_NOTIFY_H__
#define __WIFI_SYS_NOTIFY_H__

struct notify_entry;

typedef INT(*notify_cb_t)(struct notify_entry *entry,
	INT action, VOID *data);

struct notify_entry {
	notify_cb_t notify_call;
	struct notify_entry *next;
	INT priority;
	void *priv;
};

struct notify_head {
	NDIS_SPIN_LOCK lock;
	struct notify_entry *head;
};

#define INIT_NOTIFY_HEAD(_ad, _name) do { \
	NdisAllocateSpinLock(_ad, &(_name)->lock); \
	(_name)->head = NULL; \
} while (0)


enum {
	NOTIFY_STAT_DONE,
	NOTIFY_STAT_OK,
	NOTIFY_STAT_STOP = 0x8000,
};

INT mt_notify_chain_register(struct notify_head *head, struct notify_entry *entry);
INT mt_notify_chain_unregister(struct notify_head *head, struct notify_entry *entry);
INT mt_notify_call_chain(struct notify_head *head, INT val, VOID *v);

#endif /*__WIFI_SYS_NOTIFY_H__*/

