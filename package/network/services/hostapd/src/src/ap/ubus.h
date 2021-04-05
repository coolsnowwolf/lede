/*
 * hostapd / ubus support
 * Copyright (c) 2013, Felix Fietkau <nbd@nbd.name>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
#ifndef __HOSTAPD_UBUS_H
#define __HOSTAPD_UBUS_H

enum hostapd_ubus_event_type {
	HOSTAPD_UBUS_PROBE_REQ,
	HOSTAPD_UBUS_AUTH_REQ,
	HOSTAPD_UBUS_ASSOC_REQ,
	HOSTAPD_UBUS_TYPE_MAX
};

struct hostapd_ubus_request {
	enum hostapd_ubus_event_type type;
	const struct ieee80211_mgmt *mgmt_frame;
	const struct ieee802_11_elems *elems;
	int ssi_signal; /* dBm */
	const u8 *addr;
};

struct hostapd_iface;
struct hostapd_data;
struct hapd_interfaces;
struct rrm_measurement_beacon_report;

#ifdef UBUS_SUPPORT

#include <libubox/avl.h>
#include <libubus.h>

struct hostapd_ubus_bss {
	struct ubus_object obj;
	struct avl_tree banned;
	int notify_response;
};

void hostapd_ubus_add_iface(struct hostapd_iface *iface);
void hostapd_ubus_free_iface(struct hostapd_iface *iface);
void hostapd_ubus_add_bss(struct hostapd_data *hapd);
void hostapd_ubus_free_bss(struct hostapd_data *hapd);

int hostapd_ubus_handle_event(struct hostapd_data *hapd, struct hostapd_ubus_request *req);
void hostapd_ubus_notify(struct hostapd_data *hapd, const char *type, const u8 *mac);
void hostapd_ubus_notify_beacon_report(struct hostapd_data *hapd,
				       const u8 *addr, u8 token, u8 rep_mode,
				       struct rrm_measurement_beacon_report *rep,
				       size_t len);

void hostapd_ubus_add(struct hapd_interfaces *interfaces);
void hostapd_ubus_free(struct hapd_interfaces *interfaces);

#else

struct hostapd_ubus_bss {};

static inline void hostapd_ubus_add_iface(struct hostapd_iface *iface)
{
}

static inline void hostapd_ubus_free_iface(struct hostapd_iface *iface)
{
}

static inline void hostapd_ubus_add_bss(struct hostapd_data *hapd)
{
}

static inline void hostapd_ubus_free_bss(struct hostapd_data *hapd)
{
}

static inline int hostapd_ubus_handle_event(struct hostapd_data *hapd, struct hostapd_ubus_request *req)
{
	return 0;
}

static inline void hostapd_ubus_notify(struct hostapd_data *hapd, const char *type, const u8 *mac)
{
}

static inline void hostapd_ubus_notify_beacon_report(struct hostapd_data *hapd,
						     const u8 *addr, u8 token,
						     u8 rep_mode,
						     struct rrm_measurement_beacon_report *rep,
						     size_t len)
{
}

static inline void hostapd_ubus_add(struct hapd_interfaces *interfaces)
{
}

static inline void hostapd_ubus_free(struct hapd_interfaces *interfaces)
{
}
#endif

#endif
