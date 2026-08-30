// SPDX-License-Identifier: GPL-2.0
/*
 * LED trigger for network interfaces.
 *
 * - Aggregated per-family (lan/wan/wlan).
 * - Family and flags are taken from device tree properties:
 *     - "family" : simple family string "lan" | "wan" | "wlan"
 *     - "mode"   : any combination of "link", "tx", "rx" flags
 *   Priority/combination:
 *     * If "mode" present: flags come from "mode" and take precedence.
 *       Family is taken from "family" if present, otherwise from the LED name.
 *     * If only "family" present: use its family and default flags = link+tx+rx.
 *     * If neither present: fall back to LED device name parsing.
 *
 * - Suffix "-online" is valid ONLY in the LED name (label), e.g. "green:wlan-online".
 *   It indicates the online variant but is applied only when DT "mode" is absent.
 *
 * Behaviour:
 * - wlan (normal): blink/solid driven by throughput table
 * - lan/wan (normal): one-shot blink on TX/RX packet change
 * - *-online variants: steady ON while any interface of the family has carrier
 *
 * Interfaces are auto-tracked by name match (lan0, wan1, wlan2, phy0, wl1, ath0, ra0...).
 * Up to MAX_IFACES (16) interfaces per family.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/compiler.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include "../leds.h"

#define MAX_IFACES 16
#define DEFAULT_INTERVAL_MS 50
/* Work reschedule period = 2 * DEFAULT_INTERVAL_MS; named explicitly
 * to avoid silent divergence between the zero-iface and normal paths.
 */
#define WORK_INTERVAL_MS (2 * DEFAULT_INTERVAL_MS)

enum net_trig_type {
	NET_TRIG_LAN = 0,
	NET_TRIG_WAN,
	NET_TRIG_WLAN,
	NET_TRIG_TYPE_MAX,
};

static const char * const type_names[] = {
	[NET_TRIG_LAN] = "lan",
	[NET_TRIG_WAN] = "wan",
	[NET_TRIG_WLAN] = "wlan",
};

/* labels indexed by bitmask: (link<<2)|(tx<<1)|(rx<<0) */
static const char *const labels[] = {
	"",              /* 0 */
	"(rx)",          /* 1 */
	"(tx)",          /* 2 */
	"(tx rx)",       /* 3 */
	"(link)",        /* 4 */
	"(link rx)",     /* 5 */
	"(link tx)",     /* 6 */
	"(link tx rx)",  /* 7 */
};

/* wlan throughput table */
static const struct {
	u32 throughput;
	unsigned long on_ms;
	unsigned long off_ms;
} wlan_tpt_table[] = {
	{   64, 200, 800 },
	{  512, 200, 300 },
	{ 2048, 200, 150 },
	{10000, 200,  50 },
	{54000, 100,  50 },
};

struct net_mgr {
	enum net_trig_type type; /* family: lan/wan/wlan */

	struct mutex lock;
	struct notifier_block notifier;
	struct delayed_work work;

	struct net_device *devs[MAX_IFACES];
	int dev_slot_limit;

	u64 agg_tx_packets;
	u64 agg_rx_packets;
	u64 agg_tx_bytes;
	u64 agg_rx_bytes;

	struct list_head leds;
	atomic_t refcnt;
};

struct net_led {
	struct list_head node;
	struct led_classdev *led_cdev;
	struct net_mgr *mgr;

	u64 last_tx_packets;
	u64 last_rx_packets;
	u64 last_tx_bytes;
	u64 last_rx_bytes;

	bool link;
	bool tx;
	bool rx;
};

/*
 * Locking rules:
 * - managers_lock protects managers[] and entry->mgr reassignment.
 * - m->lock protects m->leds, m->devs[], agg_* and per-entry last_* while linked to m.
 * - Moving an entry between managers requires managers_lock and both manager locks.
 */
static DEFINE_MUTEX(managers_lock);
static struct net_mgr *managers[NET_TRIG_TYPE_MAX];

static ssize_t net_flag_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t net_flag_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count);
static ssize_t net_dev_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t net_dev_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count);

/* helpers */
static inline void led_set_off_full(struct led_classdev *led, bool on)
{
	led_stop_software_blink(led);
	led_set_brightness(led, on ? LED_FULL : LED_OFF);
}

static inline void led_set_oneshot_ms(struct led_classdev *led, unsigned long ms, bool invert)
{
	led_stop_software_blink(led);
	led_blink_set_oneshot(led, &ms, &ms, invert);
}

static inline void net_mgr_lock_pair(struct net_mgr *a, struct net_mgr *b)
{
	if (a == b) {
		mutex_lock(&a->lock);
		return;
	}

	if ((unsigned long)a < (unsigned long)b) {
		mutex_lock(&a->lock);
		mutex_lock(&b->lock);
	} else {
		mutex_lock(&b->lock);
		mutex_lock(&a->lock);
	}
}

static inline void net_mgr_unlock_pair(struct net_mgr *a, struct net_mgr *b)
{
	if (a == b) {
		mutex_unlock(&a->lock);
		return;
	}

	if ((unsigned long)a < (unsigned long)b) {
		mutex_unlock(&b->lock);
		mutex_unlock(&a->lock);
	} else {
		mutex_unlock(&a->lock);
		mutex_unlock(&b->lock);
	}
}

/* helper: check that next char after prefix is digit, '-' or NUL */
static inline bool next_char_ok(const char *s, size_t pos)
{
	char c = s[pos];
	return c == '\0' || c == '-' || (c >= '0' && c <= '9');
}

/* name matching: lan/wan; wlan matched by various wifi prefixes with restriction
 * additionally accept ath (Atheros) and ra/rai (Ralink/MediaTek) prefixes.
 * For lan and wan require next char to be digit/'-' or end to avoid accidental matches.
 */
static bool name_matches_type(const char *name, enum net_trig_type type)
{
	if (!name)
		return false;

	switch (type) {
	case NET_TRIG_LAN:
		/* accept "lan", "lanX", "lan-X" */
		if (!strncmp(name, "lan", 3))
			return next_char_ok(name, 3);
		return false;

	case NET_TRIG_WAN:
		/* accept "wan", "wanX", "wan-X" */
		if (!strncmp(name, "wan", 3))
			return next_char_ok(name, 3);
		return false;

	case NET_TRIG_WLAN:
		/* accept "phyX", "wlX"/"wl-..." or "wlan" and common driver prefixes */
		if (!strncmp(name, "phy", 3))
			return next_char_ok(name, 3);
		if (!strncmp(name, "wlan", 4))
			return next_char_ok(name, 4);
		if (!strncmp(name, "wl", 2))
			return next_char_ok(name, 2);
		/* Atheros (athX) */
		if (!strncmp(name, "ath", 3))
			return next_char_ok(name, 3);
		/* Ralink/MediaTek: check "rai" (e.g. rai0) first, then "ra" (ra0) */
		if (!strncmp(name, "rai", 3))
			return next_char_ok(name, 3);
		if (!strncmp(name, "ra", 2))
			return next_char_ok(name, 2);
		return false;

	default:
		return false;
	}
}

/* Extract family type (lan/wan/wlan) from a string.
 * If "online" is supplied, also detects and strips the "-online" suffix.
 * Returns enum net_trig_type or -1 on error.
 */
static int parse_family_token(const char *fn, bool *online)
{
	size_t len;
	const size_t sufflen = sizeof("-online") - 1;

	if (online)
		*online = false;

	if (!fn)
		return -1;

	len = strlen(fn);

	if (online && (len > sufflen) && !strcmp(fn + len - sufflen, "-online")) {
		*online = true;
		len -= sufflen;
	}

	if (len == 3 && !strncmp(fn, "lan", 3))
		return NET_TRIG_LAN;
	if (len == 3 && !strncmp(fn, "wan", 3))
		return NET_TRIG_WAN;
	if (len == 4 && !strncmp(fn, "wlan", 4))
		return NET_TRIG_WLAN;

	return -1;
}

/* Parse only flags from a string: recognized tokens are "link", "tx", "rx".
 * Any other token => -EINVAL. If no flags found, set all three true.
 * Suitable for DT "mode" (strict) or parsing flags from name when no mode present.
 */
static int parse_flags_from_string(const char *fn, bool *link, bool *tx, bool *rx)
{
	size_t len;
	const char *buf = NULL;
	size_t buflen = 0;
	size_t i;
	bool found = false;

	if (!fn || !link || !tx || !rx)
		return -EINVAL;

	*link = false;
	*tx = false;
	*rx = false;

	len = strlen(fn);
	for (i = 0; i < len; i++) {
		if (fn[i] == ' ' || fn[i] == '\t' || fn[i] == ',') {
			buf = NULL;
			buflen = 0;
			continue;
		}

		if (!buf) {
			buf = fn + i;
			buflen = 0;
		}
		buflen++;

		if ((i + 1 == len) || fn[i + 1] == ' ' || fn[i + 1] == '\t' || fn[i + 1] == ',') {
			if (buflen == 4 && !strncmp(buf, "link", 4)) {
				*link = true;
				found = true;
			} else if (buflen == 2 && !strncmp(buf, "tx", 2)) {
				*tx = true;
				found = true;
			} else if (buflen == 2 && !strncmp(buf, "rx", 2)) {
				*rx = true;
				found = true;
			} else {
				/* unknown token */
				return -EINVAL;
			}

			buf = NULL;
			buflen = 0;
		}
	}

	/* default: if no flags found, set all true */
	if (!found) {
		*link = true;
		*tx = true;
		*rx = true;
	}

	return 0;
}

/* safe stats read wrapper */
static void get_dev_stats_safe(struct net_device *dev, struct rtnl_link_stats64 *st)
{
	memset(st, 0, sizeof(*st));
	dev_get_stats(dev, st);
}

/* Update single LED according to manager aggregates and per-LED flags.
 * any_online indicates whether any tracked interface currently has carrier.
 * If LED is online-only (link && !tx && !rx) it is driven directly by any_online.
 */
static void update_led(struct net_led *e, struct net_mgr *m, bool any_online)
{
	unsigned long on_ms, off_ms;
	struct led_classdev *led = e->led_cdev;

	/* defensive: avoid deref if led unexpectedly NULL */
	if (!led)
		return;

	/* If LED requested online-only, reflect any_online */
	if (READ_ONCE(e->link) && !READ_ONCE(e->tx) && !READ_ONCE(e->rx)) {
		led_set_off_full(led, any_online);
		return;
	}

	/* For non-online-only LEDs: if no tracked interface has carrier,
	 * keep the LED off and reset history baseline to avoid spurious deltas
	 * when carrier later returns.
	 */
	if (!any_online) {
		led_set_off_full(led, false);

		e->last_tx_packets = m->agg_tx_packets;
		e->last_rx_packets = m->agg_rx_packets;
		e->last_tx_bytes = m->agg_tx_bytes;
		e->last_rx_bytes = m->agg_rx_bytes;
		return;
	}

	/* non-online-only behaviour depends on family */
	if (m->type == NET_TRIG_WLAN) {
		/* throughput-driven */
		u64 bytes_delta = 0;
		u64 kbps = 0;
		int idx = 0;
		int t;

		if (READ_ONCE(e->tx) && m->agg_tx_bytes >= e->last_tx_bytes)
			bytes_delta += m->agg_tx_bytes - e->last_tx_bytes;
		if (READ_ONCE(e->rx) && m->agg_rx_bytes >= e->last_rx_bytes)
			bytes_delta += m->agg_rx_bytes - e->last_rx_bytes;

		/* Avoid overflow when multiplying bytes_delta; cap to max u64. */
		if (bytes_delta > ULLONG_MAX / 8)
			kbps = ULLONG_MAX;
		else
			kbps = div64_u64(bytes_delta * 8, WORK_INTERVAL_MS);

		if (kbps == 0) {
			led_set_off_full(led, READ_ONCE(e->link));
		} else {
			for (t = 0; t < ARRAY_SIZE(wlan_tpt_table); t++) {
				if (kbps >= wlan_tpt_table[t].throughput)
					idx = t;
				else
					break;
			}

			on_ms = wlan_tpt_table[idx].on_ms;
			off_ms = wlan_tpt_table[idx].off_ms;
			if (READ_ONCE(e->link))
				led_blink_set(led, &on_ms, &off_ms);
			else
				led_blink_set(led, &off_ms, &on_ms);
		}

		e->last_tx_bytes = m->agg_tx_bytes;
		e->last_rx_bytes = m->agg_rx_bytes;
	} else {
		/* LAN/WAN: oneshot on packet-count change */
		u64 tx_sum = m->agg_tx_packets;
		u64 rx_sum = m->agg_rx_packets;

		if ((READ_ONCE(e->tx) && tx_sum != e->last_tx_packets) ||
		    (READ_ONCE(e->rx) && rx_sum != e->last_rx_packets)) {
			unsigned long ms = DEFAULT_INTERVAL_MS;

			led_set_oneshot_ms(led, ms, READ_ONCE(e->link));
		} else {
			led_set_off_full(led, READ_ONCE(e->link));
		}

		e->last_tx_packets = tx_sum;
		e->last_rx_packets = rx_sum;
	}
}

/* Three-stage work: snapshot devices, collect stats, update LEDs.
 * Stats are collected without m->lock to avoid lock inversion with the
 * network stack (dev_get_stats may acquire driver locks / send notifiers).
 */
static void net_mgr_work(struct work_struct *work)
{
	struct net_mgr *m = container_of(work, struct net_mgr, work.work);
	struct net_device *snap[MAX_IFACES];
	int snap_count = 0;
	u64 agg_tx_packets = 0, agg_rx_packets = 0;
	u64 agg_tx_bytes = 0, agg_rx_bytes = 0;
	bool any_online = false;
	struct net_led *e;
	int i;

	/* Stage 1: snapshot device references under m->lock. */
	mutex_lock(&m->lock);
	for (i = 0; i < m->dev_slot_limit; i++) {
		if (m->devs[i]) {
			snap[snap_count] = m->devs[i];
			dev_hold(snap[snap_count]);
			snap_count++;
		}
	}
	mutex_unlock(&m->lock);

	/* Stage 2: collect stats without m->lock.
	 * Devices are pinned by dev_hold from stage 1.
	 */
	for (i = 0; i < snap_count; i++) {
		struct net_device *dev = snap[i];
		struct rtnl_link_stats64 st;

		get_dev_stats_safe(dev, &st);
		agg_tx_packets += st.tx_packets;
		agg_rx_packets += st.rx_packets;
		agg_tx_bytes   += st.tx_bytes;
		agg_rx_bytes   += st.rx_bytes;

		if (netif_running(dev) && netif_carrier_ok(dev))
			any_online = true;

		dev_put(dev);
	}

	/* Stage 3: update aggregates and LEDs under m->lock.
	 * Note: any_online was sampled in stage 2 without m->lock, so it may
	 * lag reality by up to one WORK_INTERVAL_MS cycle. This is acceptable
	 * for LED visual purposes.
	 */
	mutex_lock(&m->lock);

	m->agg_tx_packets = agg_tx_packets;
	m->agg_rx_packets = agg_rx_packets;
	m->agg_tx_bytes   = agg_tx_bytes;
	m->agg_rx_bytes   = agg_rx_bytes;

	list_for_each_entry(e, &m->leds, node)
		update_led(e, m, any_online);

	mutex_unlock(&m->lock);

	schedule_delayed_work(&m->work, msecs_to_jiffies(WORK_INTERVAL_MS));
}

/* Remove device and compact trailing NULLs in devs[].
 * Caller must hold m->lock (or other serializing lock)!
 */
static void net_mgr_remove_dev(struct net_mgr *m, int hole)
{
	int last = m->dev_slot_limit - 1;

	/* Trim trailing NULL slots first */
	while (last > hole && !m->devs[last]) {
		last--;
		m->dev_slot_limit--;
	}

	if (last > hole) {
		/* Swap hole with last non-NULL slot */
		m->devs[hole] = m->devs[last];
		m->devs[last] = NULL;
	}
	/* last == hole: the hole is the last slot; just shrink */
	m->dev_slot_limit--;
}

/* notifier: manage tracked devices */
static int net_mgr_notify(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct netdev_notifier_info *info = ptr;
	struct net_device *dev = NULL;
	/* to_put collects any reference that must be dropped after mutex release */
	struct net_device *to_put = NULL;
	struct net_mgr *m = container_of(nb, struct net_mgr, notifier);
	int i, id = -1, newid;

	if (event != NETDEV_REGISTER && event != NETDEV_UNREGISTER &&
	    event != NETDEV_CHANGENAME)
		return NOTIFY_DONE;

	if (!info)
		return NOTIFY_DONE;

	dev = info->dev;
	if (!dev)
		return NOTIFY_DONE;

	mutex_lock(&m->lock);
	for (i = 0; i < m->dev_slot_limit; i++) {
		if (m->devs[i] == dev) {
			id = i;
			break;
		}
	}

	switch (event) {
	case NETDEV_UNREGISTER:
		if (id >= 0 && m->devs[id]) {
			to_put = m->devs[id];
			m->devs[id] = NULL;
			pr_info("%s - interface %s unregistered\n",
				type_names[m->type], dev->name);
			net_mgr_remove_dev(m, id);
		}
		break;

	case NETDEV_CHANGENAME:
		if (id >= 0 && !name_matches_type(dev->name, m->type)) {
			if (m->devs[id]) {
				to_put = m->devs[id];
				m->devs[id] = NULL;
				pr_info("%s - interface renamed to %s (no longer matches), untracked\n",
					type_names[m->type], dev->name);
				net_mgr_remove_dev(m, id);
			}
			break;
		}
		fallthrough;

	case NETDEV_REGISTER:
		if (id < 0 && name_matches_type(dev->name, m->type)) {
			newid = -1;
			for (i = 0; i < m->dev_slot_limit; i++) {
				if (!m->devs[i]) {
					newid = i;
					break;
				}
			}
			if (newid < 0 && m->dev_slot_limit < MAX_IFACES)
				newid = m->dev_slot_limit++;

			if (newid >= 0) {
				dev_hold(dev);
				m->devs[newid] = dev;
				pr_info("%s - interface %s registered\n",
					type_names[m->type], dev->name);
			}
		}
		break;
	}

	mutex_unlock(&m->lock);

	/* drop reference outside the lock to avoid lock inversion */
	if (to_put)
		dev_put(to_put);

	return NOTIFY_DONE;
}

static void net_mgr_put(struct net_mgr *m)
{
	int i;

	if (!m)
		return;

	if (!atomic_dec_and_mutex_lock(&m->refcnt, &managers_lock))
		return;

	if (managers[m->type] == m)
		managers[m->type] = NULL;
	mutex_unlock(&managers_lock);

	cancel_delayed_work_sync(&m->work);
	unregister_netdevice_notifier(&m->notifier);

	mutex_lock(&m->lock);
	for (i = 0; i < m->dev_slot_limit; i++) {
		if (m->devs[i])
			dev_put(m->devs[i]);
	}
	mutex_unlock(&m->lock);

	kfree(m);
}

/* find/create manager for base family */
static struct net_mgr *net_mgr_get(enum net_trig_type type)
{
	struct net_mgr *m, *existing = NULL;
	int i;

	if (type >= NET_TRIG_TYPE_MAX)
		return NULL;

	/* fast-path: if already exists, bump ref and return */
	mutex_lock(&managers_lock);
	m = managers[type];
	if (m) {
		atomic_inc(&m->refcnt);
		mutex_unlock(&managers_lock);
		return m;
	}
	mutex_unlock(&managers_lock);

	/* allocate and init (not yet published) */
	m = kzalloc(sizeof(*m), GFP_KERNEL);
	if (!m)
		return NULL;

	m->type = type;
	mutex_init(&m->lock);
	INIT_LIST_HEAD(&m->leds);
	atomic_set(&m->refcnt, 1);
	INIT_DELAYED_WORK(&m->work, net_mgr_work);

	m->notifier.notifier_call = net_mgr_notify;
	m->notifier.priority = 0;

	if (register_netdevice_notifier(&m->notifier)) {
		kfree(m);
		return NULL;
	}

	/* publish manager, handle rare race where another thread created it first */
	mutex_lock(&managers_lock);
	existing = managers[type];
	if (existing) {
		/* use existing one: increase refcount, drop our allocation & deregister our (not-used) notifier */
		atomic_inc(&existing->refcnt);
		mutex_unlock(&managers_lock);
		unregister_netdevice_notifier(&m->notifier); /* deregister only our own! */
		for (i = 0; i < m->dev_slot_limit; i++) {
			if (m->devs[i])
				dev_put(m->devs[i]);
		}
		kfree(m);
		return existing;
	}

	managers[type] = m;
	mutex_unlock(&managers_lock);

	/* start background work */
	schedule_delayed_work(&m->work, 0);

	return m;
}

static DEVICE_ATTR(link, 0644, net_flag_show, net_flag_store);
static DEVICE_ATTR(tx, 0644, net_flag_show, net_flag_store);
static DEVICE_ATTR(rx, 0644, net_flag_show, net_flag_store);
static DEVICE_ATTR(family, 0644, net_dev_show, net_dev_store);

static struct attribute *net_attrs[] = {
	&dev_attr_link.attr,
	&dev_attr_tx.attr,
	&dev_attr_rx.attr,
	&dev_attr_family.attr,
	NULL,
};

static const struct attribute_group net_attr_group = {
	.attrs = net_attrs,
};

static ssize_t net_flag_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct net_led *entry;
	int val;

	entry = led_trigger_get_drvdata(dev);
	if (!entry)
		return -ENODEV;

	if (attr == &dev_attr_link)
		val = READ_ONCE(entry->link);
	else if (attr == &dev_attr_tx)
		val = READ_ONCE(entry->tx);
	else if (attr == &dev_attr_rx)
		val = READ_ONCE(entry->rx);
	else
		return -EINVAL;

	return sysfs_emit(buf, "%d\n", val);
}

static ssize_t net_flag_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct net_led *entry;
	struct net_mgr *mgr;
	bool val;
	int ret;

	entry = led_trigger_get_drvdata(dev);
	if (!entry)
		return -ENODEV;

	ret = kstrtobool(buf, &val);
	if (ret)
		return ret;

	/* Pin mgr via refcnt to prevent UAF.
	 * Hold managers_lock across the flag write so that net_dev_store()
	 * cannot reassign entry->mgr concurrently.
	 */
	mutex_lock(&managers_lock);
	mgr = entry->mgr;
	if (!mgr || !atomic_inc_not_zero(&mgr->refcnt)) {
		mutex_unlock(&managers_lock);
		return -ENODEV;
	}

	mutex_lock(&mgr->lock);

	if (attr == &dev_attr_link)
		WRITE_ONCE(entry->link, val);
	else if (attr == &dev_attr_tx)
		WRITE_ONCE(entry->tx, val);
	else if (attr == &dev_attr_rx)
		WRITE_ONCE(entry->rx, val);
	else {
		mutex_unlock(&mgr->lock);
		mutex_unlock(&managers_lock);
		net_mgr_put(mgr);
		return -EINVAL;
	}

	ret = count;

	pr_info("LED %s - network trigger flags changed to %s%s%s\n",
		dev_name(entry->led_cdev->dev),
		READ_ONCE(entry->link) ? "link " : "",
		READ_ONCE(entry->tx) ? "tx " : "",
		READ_ONCE(entry->rx) ? "rx" : "");

	mutex_unlock(&mgr->lock);
	mutex_unlock(&managers_lock);

	schedule_delayed_work(&mgr->work, 0);
	net_mgr_put(mgr);

	return ret;
}

static ssize_t net_dev_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct net_led *entry;
	struct net_mgr *mgr;
	ssize_t ret;

	entry = led_trigger_get_drvdata(dev);
	if (!entry)
		return -ENODEV;

	/* Protect dereference with managers_lock and refcount */
	mutex_lock(&managers_lock);
	mgr = entry->mgr;
	if (mgr && !atomic_inc_not_zero(&mgr->refcnt))
		mgr = NULL;
	mutex_unlock(&managers_lock);

	if (!mgr)
		return -ENODEV;

	ret = sysfs_emit(buf, "%s\n", type_names[mgr->type]);
	net_mgr_put(mgr);
	return ret;
}

static ssize_t net_dev_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	char tmp[32];
	char *trimmed;
	size_t len = count;
	struct net_led *entry;
	int parsed;
	struct net_mgr *old_mgr = NULL, *new_mgr;
	size_t i;

	entry = led_trigger_get_drvdata(dev);
	if (!entry)
		return -ENODEV;

	/* Pin old_mgr via refcnt to prevent UAF */
	mutex_lock(&managers_lock);
	old_mgr = entry->mgr;
	if (old_mgr && !atomic_inc_not_zero(&old_mgr->refcnt))
		old_mgr = NULL;
	mutex_unlock(&managers_lock);
	if (!old_mgr)
		return -ENODEV;

	if (len == 0 || len >= sizeof(tmp)) {
		net_mgr_put(old_mgr);
		return -EINVAL;
	}

	/* copy raw input from sysfs buffer and ensure null-termination */
	memcpy(tmp, buf, len);
	tmp[len] = '\0';

	/* trim leading/trailing spaces, tabs, and newlines (\r, \n) */
	trimmed = strim(tmp);
	len = strlen(trimmed);

	/* reject internal spaces/tabs inside the token */
	for (i = 0; i < len; i++) {
		if (trimmed[i] == ' ' || trimmed[i] == '\t') {
			net_mgr_put(old_mgr);
			return -EINVAL;
		}
	}

	parsed = parse_family_token(trimmed, NULL);
	if (parsed < 0) {
		net_mgr_put(old_mgr);
		return -EINVAL;
	}

	if (old_mgr->type == parsed) {
		net_mgr_put(old_mgr);
		return count; /* no change */
	}

	/* get or create new manager (increments refcnt) */
	new_mgr = net_mgr_get(parsed);
	if (!new_mgr) {
		net_mgr_put(old_mgr);
		return -ENOMEM;
	}

	/* Move entry between manager lists.
	 * managers_lock serializes entry->mgr reassignment against all other paths.
	 * Both manager locks are required because both old_mgr->leds and new_mgr->leds
	 * are accessed by net_mgr_work() under the respective manager lock.
	 */
	mutex_lock(&managers_lock);
	net_mgr_lock_pair(old_mgr, new_mgr);

	/* Verify entry still belongs to old_mgr */
	if (entry->mgr != old_mgr) {
		net_mgr_unlock_pair(old_mgr, new_mgr);
		mutex_unlock(&managers_lock);
		net_mgr_put(new_mgr);
		net_mgr_put(old_mgr);
		return -ENODEV;
	}

	list_del(&entry->node);
	list_add_tail(&entry->node, &new_mgr->leds);
	entry->mgr = new_mgr;

	/* initialize history so future reads won't see a spurious delta */
	entry->last_tx_packets = new_mgr->agg_tx_packets;
	entry->last_rx_packets = new_mgr->agg_rx_packets;
	entry->last_tx_bytes = new_mgr->agg_tx_bytes;
	entry->last_rx_bytes = new_mgr->agg_rx_bytes;

	pr_info("LED %s - network trigger family changed to %s\n",
		dev_name(entry->led_cdev->dev), type_names[parsed]);

	net_mgr_unlock_pair(old_mgr, new_mgr);
	mutex_unlock(&managers_lock);

	schedule_delayed_work(&new_mgr->work, 0);

	/* Two puts for old_mgr:
	 * (1) the local pin acquired by atomic_inc_not_zero() at the top.
	 * (2) the structural ref that entry->mgr held for old_mgr,
	 *     which is now orphaned because entry->mgr was reassigned to new_mgr
	 *     (new_mgr's ref came from net_mgr_get() and was adopted by entry->mgr).
	 */
	net_mgr_put(old_mgr); /* (1) local pin */
	net_mgr_put(old_mgr); /* (2) structural ref from old entry->mgr */

	return count;
}

/* deactivate: detach led from manager */
static void net_deactivate(struct led_classdev *led_cdev)
{
	struct net_led *entry;
	struct net_mgr *m;

	entry = led_trigger_get_drvdata(led_cdev->dev);
	if (!entry)
		return;

	mutex_lock(&managers_lock);
	m = entry->mgr;
	if (!m || !atomic_inc_not_zero(&m->refcnt)) {
		mutex_unlock(&managers_lock);
		led_set_trigger_data(led_cdev, NULL);
		kfree(entry);
		return;
	}

	/* managers_lock is held continuously from above until after list_del,
	 * so entry->mgr cannot be reassigned by net_dev_store() concurrently.
	 * No re-verify needed.
	 */
	mutex_lock(&m->lock);
	list_del(&entry->node);
	entry->mgr = NULL;
	mutex_unlock(&m->lock);
	mutex_unlock(&managers_lock);

	led_set_off_full(led_cdev, false);
	led_set_trigger_data(led_cdev, NULL);

	pr_info("LED %s - trigger %s%s detached\n",
		dev_name(led_cdev->dev),
		type_names[m->type],
		labels[(READ_ONCE(entry->link) << 2) |
		       (READ_ONCE(entry->tx) << 1) |
		       (READ_ONCE(entry->rx) << 0)]);

	kfree(entry);
	net_mgr_put(m);		/* structural ref transferred from entry */
	net_mgr_put(m);		/* local pin taken above */
}

/* activate/deactivate: attach led to manager and remember flags */
static int net_activate(struct led_classdev *led_cdev)
{
	const char *fn = NULL;
	const char *dt_family = NULL;
	const char *dt_mode = NULL;
	int parsed = -1;
	bool link = false;
	bool tx = false;
	bool rx = false;
	bool online;
	struct net_mgr *m;
	struct net_led *entry;
	const char *name;
	const char *sep;
	int ret;

	if (!led_cdev) {
		pr_err("network: net_activate called with NULL led_cdev\n");
		return -EINVAL;
	}
	if (!led_cdev->dev) {
		pr_err("network: LED device is NULL, aborting activate\n");
		return -EINVAL;
	}

	name = dev_name(led_cdev->dev);
	if (!name || !*name) {
		pr_err("network: LED has no name, aborting activate\n");
		return -EINVAL;
	}

	if (led_trigger_get_drvdata(led_cdev->dev)) {
		pr_warn("network: LED %s already has trigger_data set, refusing attach\n",
			name);
		return -EBUSY;
	}

	if (led_cdev->dev->of_node) {
		of_property_read_string(led_cdev->dev->of_node, "family", &dt_family);
		of_property_read_string(led_cdev->dev->of_node, "mode", &dt_mode);
	}

	/* function part from name (after last ':') */
	sep = strrchr(name, ':');
	if (sep && sep[1] != '\0')
		fn = sep + 1;
	else
		fn = name;

	if (dt_mode) {
		/* DT mode present: strict parsing, empty string is invalid */
		if (*dt_mode == '\0')
			return -EINVAL;

		ret = parse_flags_from_string(dt_mode, &link, &tx, &rx);
		if (ret)
			return -EINVAL;

		/* family: from dt_family if present, otherwise from name */
		if (dt_family) {
			parsed = parse_family_token(dt_family, NULL);
			if (parsed < 0) {
				pr_info("network: invalid family '%s' for LED %s\n",
					dt_family, name);
				return -EINVAL;
			}
		} else {
			/* parse family from name; online flag parsed but not used here */
			parsed = parse_family_token(fn, &online);
			if (parsed < 0) {
				pr_info("network: no family in name and no family for LED %s\n",
					name);
				return -EINVAL;
			}
		}
	} else if (dt_family) {
		/* Only dt_family present: use its family. Flags depend on name "-online" */
		parsed = parse_family_token(dt_family, NULL);
		if (parsed < 0) {
			pr_info("network: invalid family '%s' for LED %s\n",
				dt_family, name);
			return -EINVAL;
		}

		/* test whether name contains "-online" */
		parse_family_token(fn, &online);
		if (online) {
			/* name indicated online variant and no mode -> online-only */
			link = true;
			tx = false;
			rx = false;
		} else {
			link = true;
			tx = true;
			rx = true;
		}
	} else {
		/* No DT properties: family and flags come from the LED name.
		 * If name had "-online" -> online-only.
		 */
		parsed = parse_family_token(fn, &online);
		if (parsed < 0) {
			pr_warn("network: cannot infer family from '%s' for LED %s, defaulting to 'wlan'\n",
				fn ?: "<NULL>", name);
			parsed = NET_TRIG_WLAN;
		}

		if (online) {
			/* online variant in name => online-only */
			link = true;
			tx = false;
			rx = false;
		} else {
			link = true;
			tx = true;
			rx = true;
		}
	}

	m = net_mgr_get(parsed);
	if (!m)
		return -ENOMEM;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry) {
		net_mgr_put(m);
		return -ENOMEM;
	}

	entry->led_cdev = led_cdev;
	entry->mgr = m;
	entry->last_tx_packets = 0;
	entry->last_rx_packets = 0;
	entry->last_tx_bytes = 0;
	entry->last_rx_bytes = 0;
	WRITE_ONCE(entry->link, link);
	WRITE_ONCE(entry->tx, tx);
	WRITE_ONCE(entry->rx, rx);

	/* attach under manager lock and initialize history to current aggregates */
	mutex_lock(&m->lock);
	list_add_tail(&entry->node, &m->leds);

	/* initialize history so future reads won't see a spurious delta */
	entry->last_tx_packets = m->agg_tx_packets;
	entry->last_rx_packets = m->agg_rx_packets;
	entry->last_tx_bytes = m->agg_tx_bytes;
	entry->last_rx_bytes = m->agg_rx_bytes;
	mutex_unlock(&m->lock);

	/* Publish trigger data after entry is fully linked into the manager list.
	 * net_deactivate() relies on entry being on the list before list_del().
	 * Any sysfs access before this point returns -ENODEV, which is safe.
	 */
	led_set_trigger_data(led_cdev, entry);

	pr_info("LED %s - trigger %s%s attached\n",
		name,
		type_names[m->type],
		labels[(READ_ONCE(entry->link) << 2) |
		       (READ_ONCE(entry->tx) << 1) |
		       (READ_ONCE(entry->rx) << 0)]);

	return 0;
}

static struct led_trigger network_trigger = {
	.name = "network",
	.activate = net_activate,
	.deactivate = net_deactivate,
	/* Use .groups + led_trigger_get_drvdata/set_drvdata; avoid device_create_file/remove_file. */
	.groups = (const struct attribute_group *[]) { &net_attr_group, NULL },
};

module_led_trigger(network_trigger);

MODULE_AUTHOR("Mieczyslaw Nalewaj <namiltd@yahoo.com>");
MODULE_DESCRIPTION("LED trigger for network interfaces - aggregated by family; supports link/tx/rx and -online");
MODULE_LICENSE("GPL");
