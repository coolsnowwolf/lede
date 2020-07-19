/*
 * hostapd / ubus support
 * Copyright (c) 2013, Felix Fietkau <nbd@nbd.name>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/wpabuf.h"
#include "common/ieee802_11_defs.h"
#include "hostapd.h"
#include "neighbor_db.h"
#include "wps_hostapd.h"
#include "sta_info.h"
#include "ubus.h"
#include "ap_drv_ops.h"
#include "beacon.h"
#include "rrm.h"
#include "wnm_ap.h"
#include "taxonomy.h"

static struct ubus_context *ctx;
static struct blob_buf b;
static int ctx_ref;

static inline struct hapd_interfaces *get_hapd_interfaces_from_object(struct ubus_object *obj)
{
	return container_of(obj, struct hapd_interfaces, ubus);
}

static inline struct hostapd_data *get_hapd_from_object(struct ubus_object *obj)
{
	return container_of(obj, struct hostapd_data, ubus.obj);
}

struct ubus_banned_client {
	struct avl_node avl;
	u8 addr[ETH_ALEN];
};

static void ubus_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct ubus_context *ctx = eloop_ctx;
	ubus_handle_event(ctx);
}

static void ubus_reconnect_timeout(void *eloop_data, void *user_ctx)
{
	if (ubus_reconnect(ctx, NULL)) {
		eloop_register_timeout(1, 0, ubus_reconnect_timeout, ctx, NULL);
		return;
	}

	eloop_register_read_sock(ctx->sock.fd, ubus_receive, ctx, NULL);
}

static void hostapd_ubus_connection_lost(struct ubus_context *ctx)
{
	eloop_unregister_read_sock(ctx->sock.fd);
	eloop_register_timeout(1, 0, ubus_reconnect_timeout, ctx, NULL);
}

static bool hostapd_ubus_init(void)
{
	if (ctx)
		return true;

	ctx = ubus_connect(NULL);
	if (!ctx)
		return false;

	ctx->connection_lost = hostapd_ubus_connection_lost;
	eloop_register_read_sock(ctx->sock.fd, ubus_receive, ctx, NULL);
	return true;
}

static void hostapd_ubus_ref_inc(void)
{
	ctx_ref++;
}

static void hostapd_ubus_ref_dec(void)
{
	ctx_ref--;
	if (!ctx)
		return;

	if (ctx_ref)
		return;

	eloop_unregister_read_sock(ctx->sock.fd);
	ubus_free(ctx);
	ctx = NULL;
}

void hostapd_ubus_add_iface(struct hostapd_iface *iface)
{
	if (!hostapd_ubus_init())
		return;
}

void hostapd_ubus_free_iface(struct hostapd_iface *iface)
{
	if (!ctx)
		return;
}

static void
hostapd_bss_del_ban(void *eloop_data, void *user_ctx)
{
	struct ubus_banned_client *ban = eloop_data;
	struct hostapd_data *hapd = user_ctx;

	avl_delete(&hapd->ubus.banned, &ban->avl);
	free(ban);
}

static void
hostapd_bss_ban_client(struct hostapd_data *hapd, u8 *addr, int time)
{
	struct ubus_banned_client *ban;

	if (time < 0)
		time = 0;

	ban = avl_find_element(&hapd->ubus.banned, addr, ban, avl);
	if (!ban) {
		if (!time)
			return;

		ban = os_zalloc(sizeof(*ban));
		memcpy(ban->addr, addr, sizeof(ban->addr));
		ban->avl.key = ban->addr;
		avl_insert(&hapd->ubus.banned, &ban->avl);
	} else {
		eloop_cancel_timeout(hostapd_bss_del_ban, ban, hapd);
		if (!time) {
			hostapd_bss_del_ban(ban, hapd);
			return;
		}
	}

	eloop_register_timeout(0, time * 1000, hostapd_bss_del_ban, ban, hapd);
}

static int
hostapd_bss_reload(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);
	return hostapd_reload_config(hapd->iface, 1);
}

static int
hostapd_bss_get_clients(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);
	struct sta_info *sta;
	void *list, *c;
	char mac_buf[20];
	static const struct {
		const char *name;
		uint32_t flag;
	} sta_flags[] = {
		{ "auth", WLAN_STA_AUTH },
		{ "assoc", WLAN_STA_ASSOC },
		{ "authorized", WLAN_STA_AUTHORIZED },
		{ "preauth", WLAN_STA_PREAUTH },
		{ "wds", WLAN_STA_WDS },
		{ "wmm", WLAN_STA_WMM },
		{ "ht", WLAN_STA_HT },
		{ "vht", WLAN_STA_VHT },
		{ "wps", WLAN_STA_WPS },
		{ "mfp", WLAN_STA_MFP },
	};

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "freq", hapd->iface->freq);
	list = blobmsg_open_table(&b, "clients");
	for (sta = hapd->sta_list; sta; sta = sta->next) {
		void *r;
		int i;

		sprintf(mac_buf, MACSTR, MAC2STR(sta->addr));
		c = blobmsg_open_table(&b, mac_buf);
		for (i = 0; i < ARRAY_SIZE(sta_flags); i++)
			blobmsg_add_u8(&b, sta_flags[i].name,
				       !!(sta->flags & sta_flags[i].flag));

		r = blobmsg_open_array(&b, "rrm");
		for (i = 0; i < ARRAY_SIZE(sta->rrm_enabled_capa); i++)
			blobmsg_add_u32(&b, "", sta->rrm_enabled_capa[i]);
		blobmsg_close_array(&b, r);
		blobmsg_add_u32(&b, "aid", sta->aid);
#ifdef CONFIG_TAXONOMY
		r = blobmsg_alloc_string_buffer(&b, "signature", 1024);
		if (retrieve_sta_taxonomy(hapd, sta, r, 1024) > 0)
			blobmsg_add_string_buffer(&b);
#endif
		blobmsg_close_table(&b, c);
	}
	blobmsg_close_array(&b, list);
	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
hostapd_bss_get_features(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "ht_supported", ht_supported(hapd->iface->hw_features));
	blobmsg_add_u8(&b, "vht_supported", vht_supported(hapd->iface->hw_features));
	ubus_send_reply(ctx, req, b.head);

	return 0;
}

enum {
	NOTIFY_RESPONSE,
	__NOTIFY_MAX
};

static const struct blobmsg_policy notify_policy[__NOTIFY_MAX] = {
	[NOTIFY_RESPONSE] = { "notify_response", BLOBMSG_TYPE_INT32 },
};

static int
hostapd_notify_response(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__NOTIFY_MAX];
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct wpabuf *elems;
	const char *pos;
	size_t len;

	blobmsg_parse(notify_policy, __NOTIFY_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[NOTIFY_RESPONSE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	hapd->ubus.notify_response = blobmsg_get_u32(tb[NOTIFY_RESPONSE]);

	return UBUS_STATUS_OK;
}

enum {
	DEL_CLIENT_ADDR,
	DEL_CLIENT_REASON,
	DEL_CLIENT_DEAUTH,
	DEL_CLIENT_BAN_TIME,
	__DEL_CLIENT_MAX
};

static const struct blobmsg_policy del_policy[__DEL_CLIENT_MAX] = {
	[DEL_CLIENT_ADDR] = { "addr", BLOBMSG_TYPE_STRING },
	[DEL_CLIENT_REASON] = { "reason", BLOBMSG_TYPE_INT32 },
	[DEL_CLIENT_DEAUTH] = { "deauth", BLOBMSG_TYPE_INT8 },
	[DEL_CLIENT_BAN_TIME] = { "ban_time", BLOBMSG_TYPE_INT32 },
};

static int
hostapd_bss_del_client(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__DEL_CLIENT_MAX];
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);
	struct sta_info *sta;
	bool deauth = false;
	int reason;
	u8 addr[ETH_ALEN];

	blobmsg_parse(del_policy, __DEL_CLIENT_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[DEL_CLIENT_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (hwaddr_aton(blobmsg_data(tb[DEL_CLIENT_ADDR]), addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (tb[DEL_CLIENT_REASON])
		reason = blobmsg_get_u32(tb[DEL_CLIENT_REASON]);

	if (tb[DEL_CLIENT_DEAUTH])
		deauth = blobmsg_get_bool(tb[DEL_CLIENT_DEAUTH]);

	sta = ap_get_sta(hapd, addr);
	if (sta) {
		if (deauth) {
			hostapd_drv_sta_deauth(hapd, addr, reason);
			ap_sta_deauthenticate(hapd, sta, reason);
		} else {
			hostapd_drv_sta_disassoc(hapd, addr, reason);
			ap_sta_disassociate(hapd, sta, reason);
		}
	}

	if (tb[DEL_CLIENT_BAN_TIME])
		hostapd_bss_ban_client(hapd, addr, blobmsg_get_u32(tb[DEL_CLIENT_BAN_TIME]));

	return 0;
}

static void
blobmsg_add_macaddr(struct blob_buf *buf, const char *name, const u8 *addr)
{
	char *s;

	s = blobmsg_alloc_string_buffer(buf, name, 20);
	sprintf(s, MACSTR, MAC2STR(addr));
	blobmsg_add_string_buffer(buf);
}

static int
hostapd_bss_list_bans(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);
	struct ubus_banned_client *ban;
	void *c;

	blob_buf_init(&b, 0);
	c = blobmsg_open_array(&b, "clients");
	avl_for_each_element(&hapd->ubus.banned, ban, avl)
		blobmsg_add_macaddr(&b, NULL, ban->addr);
	blobmsg_close_array(&b, c);
	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
hostapd_bss_wps_start(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	int rc;
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);

	rc = hostapd_wps_button_pushed(hapd, NULL);

	if (rc != 0)
		return UBUS_STATUS_NOT_SUPPORTED;

	return 0;
}

static int
hostapd_bss_wps_cancel(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	int rc;
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);

	rc = hostapd_wps_cancel(hapd);

	if (rc != 0)
		return UBUS_STATUS_NOT_SUPPORTED;

	return 0;
}

static int
hostapd_bss_update_beacon(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	int rc;
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);

	rc = ieee802_11_set_beacon(hapd);

	if (rc != 0)
		return UBUS_STATUS_NOT_SUPPORTED;

	return 0;
}

enum {
	CONFIG_IFACE,
	CONFIG_FILE,
	__CONFIG_MAX
};

static const struct blobmsg_policy config_add_policy[__CONFIG_MAX] = {
	[CONFIG_IFACE] = { "iface", BLOBMSG_TYPE_STRING },
	[CONFIG_FILE] = { "config", BLOBMSG_TYPE_STRING },
};

static int
hostapd_config_add(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	struct blob_attr *tb[__CONFIG_MAX];
	struct hapd_interfaces *interfaces = get_hapd_interfaces_from_object(obj);
	char buf[128];

	blobmsg_parse(config_add_policy, __CONFIG_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[CONFIG_FILE] || !tb[CONFIG_IFACE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	snprintf(buf, sizeof(buf), "bss_config=%s:%s",
		blobmsg_get_string(tb[CONFIG_IFACE]),
		blobmsg_get_string(tb[CONFIG_FILE]));

	if (hostapd_add_iface(interfaces, buf))
		return UBUS_STATUS_INVALID_ARGUMENT;

	return UBUS_STATUS_OK;
}

enum {
	CONFIG_REM_IFACE,
	__CONFIG_REM_MAX
};

static const struct blobmsg_policy config_remove_policy[__CONFIG_REM_MAX] = {
	[CONFIG_REM_IFACE] = { "iface", BLOBMSG_TYPE_STRING },
};

static int
hostapd_config_remove(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__CONFIG_REM_MAX];
	struct hapd_interfaces *interfaces = get_hapd_interfaces_from_object(obj);
	char buf[128];

	blobmsg_parse(config_remove_policy, __CONFIG_REM_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[CONFIG_REM_IFACE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (hostapd_remove_iface(interfaces, blobmsg_get_string(tb[CONFIG_REM_IFACE])))
		return UBUS_STATUS_INVALID_ARGUMENT;

	return UBUS_STATUS_OK;
}

enum {
	CSA_FREQ,
	CSA_BCN_COUNT,
	CSA_CENTER_FREQ1,
	CSA_CENTER_FREQ2,
	CSA_BANDWIDTH,
	CSA_SEC_CHANNEL_OFFSET,
	CSA_HT,
	CSA_VHT,
	CSA_BLOCK_TX,
	__CSA_MAX
};

static const struct blobmsg_policy csa_policy[__CSA_MAX] = {
	[CSA_FREQ] = { "freq", BLOBMSG_TYPE_INT32 },
	[CSA_BCN_COUNT] = { "bcn_count", BLOBMSG_TYPE_INT32 },
	[CSA_CENTER_FREQ1] = { "center_freq1", BLOBMSG_TYPE_INT32 },
	[CSA_CENTER_FREQ2] = { "center_freq2", BLOBMSG_TYPE_INT32 },
	[CSA_BANDWIDTH] = { "bandwidth", BLOBMSG_TYPE_INT32 },
	[CSA_SEC_CHANNEL_OFFSET] = { "sec_channel_offset", BLOBMSG_TYPE_INT32 },
	[CSA_HT] = { "ht", BLOBMSG_TYPE_BOOL },
	[CSA_VHT] = { "vht", BLOBMSG_TYPE_BOOL },
	[CSA_BLOCK_TX] = { "block_tx", BLOBMSG_TYPE_BOOL },
};

#ifdef NEED_AP_MLME
static int
hostapd_switch_chan(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	struct blob_attr *tb[__CSA_MAX];
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct csa_settings css;

	blobmsg_parse(csa_policy, __CSA_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[CSA_FREQ])
		return UBUS_STATUS_INVALID_ARGUMENT;

	memset(&css, 0, sizeof(css));
	css.freq_params.freq = blobmsg_get_u32(tb[CSA_FREQ]);

#define SET_CSA_SETTING(name, field, type) \
	do { \
		if (tb[name]) \
			css.field = blobmsg_get_ ## type(tb[name]); \
	} while(0)

	SET_CSA_SETTING(CSA_BCN_COUNT, cs_count, u32);
	SET_CSA_SETTING(CSA_CENTER_FREQ1, freq_params.center_freq1, u32);
	SET_CSA_SETTING(CSA_CENTER_FREQ2, freq_params.center_freq2, u32);
	SET_CSA_SETTING(CSA_BANDWIDTH, freq_params.bandwidth, u32);
	SET_CSA_SETTING(CSA_SEC_CHANNEL_OFFSET, freq_params.sec_channel_offset, u32);
	SET_CSA_SETTING(CSA_HT, freq_params.ht_enabled, bool);
	SET_CSA_SETTING(CSA_VHT, freq_params.vht_enabled, bool);
	SET_CSA_SETTING(CSA_BLOCK_TX, block_tx, bool);


	if (hostapd_switch_channel(hapd, &css) != 0)
		return UBUS_STATUS_NOT_SUPPORTED;
	return UBUS_STATUS_OK;
#undef SET_CSA_SETTING
}
#endif

enum {
	VENDOR_ELEMENTS,
	__VENDOR_ELEMENTS_MAX
};

static const struct blobmsg_policy ve_policy[__VENDOR_ELEMENTS_MAX] = {
	/* vendor elements are provided as hex-string */
	[VENDOR_ELEMENTS] = { "vendor_elements", BLOBMSG_TYPE_STRING },
};

static int
hostapd_vendor_elements(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__VENDOR_ELEMENTS_MAX];
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct hostapd_bss_config *bss = hapd->conf;
	struct wpabuf *elems;
	const char *pos;
	size_t len;

	blobmsg_parse(ve_policy, __VENDOR_ELEMENTS_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[VENDOR_ELEMENTS])
		return UBUS_STATUS_INVALID_ARGUMENT;

	pos = blobmsg_data(tb[VENDOR_ELEMENTS]);
	len = os_strlen(pos);
	if (len & 0x01)
			return UBUS_STATUS_INVALID_ARGUMENT;

	len /= 2;
	if (len == 0) {
		wpabuf_free(bss->vendor_elements);
		bss->vendor_elements = NULL;
		return 0;
	}

	elems = wpabuf_alloc(len);
	if (elems == NULL)
		return 1;

	if (hexstr2bin(pos, wpabuf_put(elems, len), len)) {
		wpabuf_free(elems);
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	wpabuf_free(bss->vendor_elements);
	bss->vendor_elements = elems;

	/* update beacons if vendor elements were set successfully */
	if (ieee802_11_update_beacons(hapd->iface) != 0)
		return UBUS_STATUS_NOT_SUPPORTED;
	return UBUS_STATUS_OK;
}

static void
hostapd_rrm_print_nr(struct hostapd_neighbor_entry *nr)
{
	const u8 *data;
	char *str;
	int len;

	blobmsg_printf(&b, "", MACSTR, MAC2STR(nr->bssid));

	str = blobmsg_alloc_string_buffer(&b, "", nr->ssid.ssid_len + 1);
	memcpy(str, nr->ssid.ssid, nr->ssid.ssid_len);
	str[nr->ssid.ssid_len] = 0;
	blobmsg_add_string_buffer(&b);

	len = wpabuf_len(nr->nr);
	str = blobmsg_alloc_string_buffer(&b, "", 2 * len + 1);
	wpa_snprintf_hex(str, 2 * len + 1, wpabuf_head_u8(nr->nr), len);
	blobmsg_add_string_buffer(&b);
}

enum {
	BSS_MGMT_EN_NEIGHBOR,
	BSS_MGMT_EN_BEACON,
#ifdef CONFIG_WNM_AP
	BSS_MGMT_EN_BSS_TRANSITION,
#endif
	__BSS_MGMT_EN_MAX
};

static bool
__hostapd_bss_mgmt_enable_f(struct hostapd_data *hapd, int flag)
{
	struct hostapd_bss_config *bss = hapd->conf;
	uint32_t flags;

	switch (flag) {
	case BSS_MGMT_EN_NEIGHBOR:
		if (bss->radio_measurements[0] &
		    WLAN_RRM_CAPS_NEIGHBOR_REPORT)
			return false;

		bss->radio_measurements[0] |=
			WLAN_RRM_CAPS_NEIGHBOR_REPORT;
		hostapd_neighbor_set_own_report(hapd);
		return true;
	case BSS_MGMT_EN_BEACON:
		flags = WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE |
			WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE |
			WLAN_RRM_CAPS_BEACON_REPORT_TABLE;

		if (bss->radio_measurements[0] & flags == flags)
			return false;

		bss->radio_measurements[0] |= (u8) flags;
		return true;
#ifdef CONFIG_WNM_AP
	case BSS_MGMT_EN_BSS_TRANSITION:
		if (bss->bss_transition)
			return false;

		bss->bss_transition = 1;
		return true;
#endif
	}
}

static void
__hostapd_bss_mgmt_enable(struct hostapd_data *hapd, uint32_t flags)
{
	bool update = false;
	int i;

	for (i = 0; i < __BSS_MGMT_EN_MAX; i++) {
		if (!(flags & (1 << i)))
			continue;

		update |= __hostapd_bss_mgmt_enable_f(hapd, i);
	}

	if (update)
		ieee802_11_update_beacons(hapd->iface);
}


static const struct blobmsg_policy bss_mgmt_enable_policy[__BSS_MGMT_EN_MAX] = {
	[BSS_MGMT_EN_NEIGHBOR] = { "neighbor_report", BLOBMSG_TYPE_BOOL },
	[BSS_MGMT_EN_BEACON] = { "beacon_report", BLOBMSG_TYPE_BOOL },
#ifdef CONFIG_WNM_AP
	[BSS_MGMT_EN_BSS_TRANSITION] = { "bss_transition", BLOBMSG_TYPE_BOOL },
#endif
};

static int
hostapd_bss_mgmt_enable(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)

{
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct blob_attr *tb[__BSS_MGMT_EN_MAX];
	struct blob_attr *cur;
	uint32_t flags = 0;
	int i;
	bool neigh = false, beacon = false;

	blobmsg_parse(bss_mgmt_enable_policy, __BSS_MGMT_EN_MAX, tb, blob_data(msg), blob_len(msg));

	for (i = 0; i < ARRAY_SIZE(tb); i++) {
		if (!tb[i] || !blobmsg_get_bool(tb[i]))
			continue;

		flags |= (1 << i);
	}

	__hostapd_bss_mgmt_enable(hapd, flags);
}


static void
hostapd_rrm_nr_enable(struct hostapd_data *hapd)
{
	__hostapd_bss_mgmt_enable(hapd, 1 << BSS_MGMT_EN_NEIGHBOR);
}

static int
hostapd_rrm_nr_get_own(struct ubus_context *ctx, struct ubus_object *obj,
		       struct ubus_request_data *req, const char *method,
		       struct blob_attr *msg)
{
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct hostapd_neighbor_entry *nr;
	void *c;

	hostapd_rrm_nr_enable(hapd);

	nr = hostapd_neighbor_get(hapd, hapd->own_addr, NULL);
	if (!nr)
		return UBUS_STATUS_NOT_FOUND;

	blob_buf_init(&b, 0);

	c = blobmsg_open_array(&b, "value");
	hostapd_rrm_print_nr(nr);
	blobmsg_close_array(&b, c);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
hostapd_rrm_nr_list(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method,
		    struct blob_attr *msg)
{
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct hostapd_neighbor_entry *nr;
	void *c;

	hostapd_rrm_nr_enable(hapd);
	blob_buf_init(&b, 0);

	c = blobmsg_open_array(&b, "list");
	dl_list_for_each(nr, &hapd->nr_db, struct hostapd_neighbor_entry, list) {
		void *cur;

		if (!memcmp(nr->bssid, hapd->own_addr, ETH_ALEN))
			continue;

		cur = blobmsg_open_array(&b, NULL);
		hostapd_rrm_print_nr(nr);
		blobmsg_close_array(&b, cur);
	}
	blobmsg_close_array(&b, c);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

enum {
	NR_SET_LIST,
	__NR_SET_LIST_MAX
};

static const struct blobmsg_policy nr_set_policy[__NR_SET_LIST_MAX] = {
	[NR_SET_LIST] = { "list", BLOBMSG_TYPE_ARRAY },
};


static void
hostapd_rrm_nr_clear(struct hostapd_data *hapd)
{
	struct hostapd_neighbor_entry *nr;

restart:
	dl_list_for_each(nr, &hapd->nr_db, struct hostapd_neighbor_entry, list) {
		if (!memcmp(nr->bssid, hapd->own_addr, ETH_ALEN))
			continue;

		hostapd_neighbor_remove(hapd, nr->bssid, &nr->ssid);
		goto restart;
	}
}

static int
hostapd_rrm_nr_set(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	static const struct blobmsg_policy nr_e_policy[] = {
		{ .type = BLOBMSG_TYPE_STRING },
		{ .type = BLOBMSG_TYPE_STRING },
		{ .type = BLOBMSG_TYPE_STRING },
	};
	struct hostapd_data *hapd = get_hapd_from_object(obj);
	struct blob_attr *tb_l[__NR_SET_LIST_MAX];
	struct blob_attr *tb[ARRAY_SIZE(nr_e_policy)];
	struct blob_attr *cur;
	int ret = 0;
	int rem;

	hostapd_rrm_nr_enable(hapd);

	blobmsg_parse(nr_set_policy, __NR_SET_LIST_MAX, tb_l, blob_data(msg), blob_len(msg));
	if (!tb_l[NR_SET_LIST])
		return UBUS_STATUS_INVALID_ARGUMENT;

	hostapd_rrm_nr_clear(hapd);
	blobmsg_for_each_attr(cur, tb_l[NR_SET_LIST], rem) {
		struct wpa_ssid_value ssid;
		struct wpabuf *data;
		u8 bssid[ETH_ALEN];
		char *s;

		blobmsg_parse_array(nr_e_policy, ARRAY_SIZE(nr_e_policy), tb, blobmsg_data(cur), blobmsg_data_len(cur));
		if (!tb[0] || !tb[1] || !tb[2])
			goto invalid;

		s = blobmsg_get_string(tb[0]);
		if (hwaddr_aton(s, bssid))
			goto invalid;

		s = blobmsg_get_string(tb[1]);
		ssid.ssid_len = strlen(s);
		if (ssid.ssid_len > sizeof(ssid.ssid))
			goto invalid;

		memcpy(&ssid, s, ssid.ssid_len);
		data = wpabuf_parse_bin(blobmsg_get_string(tb[2]));
		if (!data)
			goto invalid;

		hostapd_neighbor_set(hapd, bssid, &ssid, data, NULL, NULL, 0);
		wpabuf_free(data);
		continue;

invalid:
		ret = UBUS_STATUS_INVALID_ARGUMENT;
	}

	return 0;
}

enum {
	BEACON_REQ_ADDR,
	BEACON_REQ_MODE,
	BEACON_REQ_OP_CLASS,
	BEACON_REQ_CHANNEL,
	BEACON_REQ_DURATION,
	BEACON_REQ_BSSID,
	BEACON_REQ_SSID,
	__BEACON_REQ_MAX,
};

static const struct blobmsg_policy beacon_req_policy[__BEACON_REQ_MAX] = {
	[BEACON_REQ_ADDR] = { "addr", BLOBMSG_TYPE_STRING },
	[BEACON_REQ_OP_CLASS] { "op_class", BLOBMSG_TYPE_INT32 },
	[BEACON_REQ_CHANNEL] { "channel", BLOBMSG_TYPE_INT32 },
	[BEACON_REQ_DURATION] { "duration", BLOBMSG_TYPE_INT32 },
	[BEACON_REQ_MODE] { "mode", BLOBMSG_TYPE_INT32 },
	[BEACON_REQ_BSSID] { "bssid", BLOBMSG_TYPE_STRING },
	[BEACON_REQ_SSID] { "ssid", BLOBMSG_TYPE_STRING },
};

static int
hostapd_rrm_beacon_req(struct ubus_context *ctx, struct ubus_object *obj,
		       struct ubus_request_data *ureq, const char *method,
		       struct blob_attr *msg)
{
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);
	struct blob_attr *tb[__BEACON_REQ_MAX];
	struct blob_attr *cur;
	struct wpabuf *req;
	u8 bssid[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	u8 addr[ETH_ALEN];
	int mode, rem, ret;
	int buf_len = 13;

	blobmsg_parse(beacon_req_policy, __BEACON_REQ_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[BEACON_REQ_ADDR] || !tb[BEACON_REQ_MODE] || !tb[BEACON_REQ_DURATION] ||
	    !tb[BEACON_REQ_OP_CLASS] || !tb[BEACON_REQ_CHANNEL])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (tb[BEACON_REQ_SSID])
		buf_len += blobmsg_data_len(tb[BEACON_REQ_SSID]) + 2 - 1;

	mode = blobmsg_get_u32(tb[BEACON_REQ_MODE]);
	if (hwaddr_aton(blobmsg_data(tb[BEACON_REQ_ADDR]), addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (tb[BEACON_REQ_BSSID] &&
	    hwaddr_aton(blobmsg_data(tb[BEACON_REQ_BSSID]), bssid))
		return UBUS_STATUS_INVALID_ARGUMENT;

	req = wpabuf_alloc(buf_len);
	if (!req)
		return UBUS_STATUS_UNKNOWN_ERROR;

	/* 1: regulatory class */
	wpabuf_put_u8(req, blobmsg_get_u32(tb[BEACON_REQ_OP_CLASS]));

	/* 2: channel number */
	wpabuf_put_u8(req, blobmsg_get_u32(tb[BEACON_REQ_CHANNEL]));

	/* 3-4: randomization interval */
	wpabuf_put_le16(req, 0);

	/* 5-6: duration */
	wpabuf_put_le16(req, blobmsg_get_u32(tb[BEACON_REQ_DURATION]));

	/* 7: mode */
	wpabuf_put_u8(req, blobmsg_get_u32(tb[BEACON_REQ_MODE]));

	/* 8-13: BSSID */
	wpabuf_put_data(req, bssid, ETH_ALEN);

	if ((cur = tb[BEACON_REQ_SSID]) != NULL) {
		wpabuf_put_u8(req, WLAN_EID_SSID);
		wpabuf_put_u8(req, blobmsg_data_len(cur) - 1);
		wpabuf_put_data(req, blobmsg_data(cur), blobmsg_data_len(cur) - 1);
	}

	ret = hostapd_send_beacon_req(hapd, addr, 0, req);
	if (ret < 0)
		return -ret;

	return 0;
}


#ifdef CONFIG_WNM_AP
enum {
	WNM_DISASSOC_ADDR,
	WNM_DISASSOC_DURATION,
	WNM_DISASSOC_NEIGHBORS,
	WNM_DISASSOC_ABRIDGED,
	__WNM_DISASSOC_MAX,
};

static const struct blobmsg_policy wnm_disassoc_policy[__WNM_DISASSOC_MAX] = {
	[WNM_DISASSOC_ADDR] = { "addr", BLOBMSG_TYPE_STRING },
	[WNM_DISASSOC_DURATION] { "duration", BLOBMSG_TYPE_INT32 },
	[WNM_DISASSOC_NEIGHBORS] { "neighbors", BLOBMSG_TYPE_ARRAY },
	[WNM_DISASSOC_ABRIDGED] { "abridged", BLOBMSG_TYPE_BOOL },
};

static int
hostapd_wnm_disassoc_imminent(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *ureq, const char *method,
			      struct blob_attr *msg)
{
	struct hostapd_data *hapd = container_of(obj, struct hostapd_data, ubus.obj);
	struct blob_attr *tb[__WNM_DISASSOC_MAX];
	struct blob_attr *cur;
	struct sta_info *sta;
	int duration = 10;
	int rem;
	int nr_len = 0;
	u8 *nr = NULL;
	u8 req_mode = WNM_BSS_TM_REQ_DISASSOC_IMMINENT;
	u8 addr[ETH_ALEN];

	blobmsg_parse(wnm_disassoc_policy, __WNM_DISASSOC_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[WNM_DISASSOC_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (hwaddr_aton(blobmsg_data(tb[WNM_DISASSOC_ADDR]), addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	if ((cur = tb[WNM_DISASSOC_DURATION]) != NULL)
		duration = blobmsg_get_u32(cur);

	sta = ap_get_sta(hapd, addr);
	if (!sta)
		return UBUS_STATUS_NOT_FOUND;

	if (tb[WNM_DISASSOC_NEIGHBORS]) {
		u8 *nr_cur;

		if (blobmsg_check_array(tb[WNM_DISASSOC_NEIGHBORS],
					BLOBMSG_TYPE_STRING) < 0)
			return UBUS_STATUS_INVALID_ARGUMENT;

		blobmsg_for_each_attr(cur, tb[WNM_DISASSOC_NEIGHBORS], rem) {
			int len = strlen(blobmsg_get_string(cur));

			if (len % 2)
				return UBUS_STATUS_INVALID_ARGUMENT;

			nr_len += (len / 2) + 2;
		}

		if (nr_len) {
			nr = os_zalloc(nr_len);
			if (!nr)
				return UBUS_STATUS_UNKNOWN_ERROR;
		}

		nr_cur = nr;
		blobmsg_for_each_attr(cur, tb[WNM_DISASSOC_NEIGHBORS], rem) {
			int len = strlen(blobmsg_get_string(cur)) / 2;

			*nr_cur++ = WLAN_EID_NEIGHBOR_REPORT;
			*nr_cur++ = (u8) len;
			if (hexstr2bin(blobmsg_data(cur), nr_cur, len)) {
				free(nr);
				return UBUS_STATUS_INVALID_ARGUMENT;
			}

			nr_cur += len;
		}
	}

	if (nr)
		req_mode |= WNM_BSS_TM_REQ_PREF_CAND_LIST_INCLUDED;

	if (tb[WNM_DISASSOC_ABRIDGED] && blobmsg_get_bool(tb[WNM_DISASSOC_ABRIDGED]))
		req_mode |= WNM_BSS_TM_REQ_ABRIDGED;

	if (wnm_send_bss_tm_req(hapd, sta, req_mode, duration, 0, NULL,
				NULL, nr, nr_len, NULL, 0))
		return UBUS_STATUS_UNKNOWN_ERROR;

	return 0;
}
#endif

static const struct ubus_method bss_methods[] = {
	UBUS_METHOD_NOARG("reload", hostapd_bss_reload),
	UBUS_METHOD_NOARG("get_clients", hostapd_bss_get_clients),
	UBUS_METHOD("del_client", hostapd_bss_del_client, del_policy),
	UBUS_METHOD_NOARG("list_bans", hostapd_bss_list_bans),
	UBUS_METHOD_NOARG("wps_start", hostapd_bss_wps_start),
	UBUS_METHOD_NOARG("wps_cancel", hostapd_bss_wps_cancel),
	UBUS_METHOD_NOARG("update_beacon", hostapd_bss_update_beacon),
	UBUS_METHOD_NOARG("get_features", hostapd_bss_get_features),
#ifdef NEED_AP_MLME
	UBUS_METHOD("switch_chan", hostapd_switch_chan, csa_policy),
#endif
	UBUS_METHOD("set_vendor_elements", hostapd_vendor_elements, ve_policy),
	UBUS_METHOD("notify_response", hostapd_notify_response, notify_policy),
	UBUS_METHOD("bss_mgmt_enable", hostapd_bss_mgmt_enable, bss_mgmt_enable_policy),
	UBUS_METHOD_NOARG("rrm_nr_get_own", hostapd_rrm_nr_get_own),
	UBUS_METHOD_NOARG("rrm_nr_list", hostapd_rrm_nr_list),
	UBUS_METHOD("rrm_nr_set", hostapd_rrm_nr_set, nr_set_policy),
	UBUS_METHOD("rrm_beacon_req", hostapd_rrm_beacon_req, beacon_req_policy),
#ifdef CONFIG_WNM_AP
	UBUS_METHOD("wnm_disassoc_imminent", hostapd_wnm_disassoc_imminent, wnm_disassoc_policy),
#endif
};

static struct ubus_object_type bss_object_type =
	UBUS_OBJECT_TYPE("hostapd_bss", bss_methods);

static int avl_compare_macaddr(const void *k1, const void *k2, void *ptr)
{
	return memcmp(k1, k2, ETH_ALEN);
}

void hostapd_ubus_add_bss(struct hostapd_data *hapd)
{
	struct ubus_object *obj = &hapd->ubus.obj;
	char *name;
	int ret;

#ifdef CONFIG_MESH
	if (hapd->conf->mesh & MESH_ENABLED)
		return;
#endif

	if (!hostapd_ubus_init())
		return;

	if (asprintf(&name, "hostapd.%s", hapd->conf->iface) < 0)
		return;

	avl_init(&hapd->ubus.banned, avl_compare_macaddr, false, NULL);
	obj->name = name;
	obj->type = &bss_object_type;
	obj->methods = bss_object_type.methods;
	obj->n_methods = bss_object_type.n_methods;
	ret = ubus_add_object(ctx, obj);
	hostapd_ubus_ref_inc();
}

void hostapd_ubus_free_bss(struct hostapd_data *hapd)
{
	struct ubus_object *obj = &hapd->ubus.obj;
	char *name = (char *) obj->name;

	if (!ctx)
		return;

	if (obj->id) {
		ubus_remove_object(ctx, obj);
		hostapd_ubus_ref_dec();
	}

	free(name);
}

static const struct ubus_method daemon_methods[] = {
	UBUS_METHOD("config_add", hostapd_config_add, config_add_policy),
	UBUS_METHOD("config_remove", hostapd_config_remove, config_remove_policy),
};

static struct ubus_object_type daemon_object_type =
	UBUS_OBJECT_TYPE("hostapd", daemon_methods);

void hostapd_ubus_add(struct hapd_interfaces *interfaces)
{
	struct ubus_object *obj = &interfaces->ubus;
	int ret;

	if (!hostapd_ubus_init())
		return;

	obj->name = strdup("hostapd");

	obj->type = &daemon_object_type;
	obj->methods = daemon_object_type.methods;
	obj->n_methods = daemon_object_type.n_methods;
	ret = ubus_add_object(ctx, obj);
	hostapd_ubus_ref_inc();
}

void hostapd_ubus_free(struct hapd_interfaces *interfaces)
{
	struct ubus_object *obj = &interfaces->ubus;
	char *name = (char *) obj->name;

	if (!ctx)
		return;

	if (obj->id) {
		ubus_remove_object(ctx, obj);
		hostapd_ubus_ref_dec();
	}

	free(name);
}

struct ubus_event_req {
	struct ubus_notify_request nreq;
	int resp;
};

static void
ubus_event_cb(struct ubus_notify_request *req, int idx, int ret)
{
	struct ubus_event_req *ureq = container_of(req, struct ubus_event_req, nreq);

	ureq->resp = ret;
}

int hostapd_ubus_handle_event(struct hostapd_data *hapd, struct hostapd_ubus_request *req)
{
	struct ubus_banned_client *ban;
	const char *types[HOSTAPD_UBUS_TYPE_MAX] = {
		[HOSTAPD_UBUS_PROBE_REQ] = "probe",
		[HOSTAPD_UBUS_AUTH_REQ] = "auth",
		[HOSTAPD_UBUS_ASSOC_REQ] = "assoc",
	};
	const char *type = "mgmt";
	struct ubus_event_req ureq = {};
	const u8 *addr;

	if (req->mgmt_frame)
		addr = req->mgmt_frame->sa;
	else
		addr = req->addr;

	ban = avl_find_element(&hapd->ubus.banned, addr, ban, avl);
	if (ban)
		return WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;

	if (!hapd->ubus.obj.has_subscribers)
		return WLAN_STATUS_SUCCESS;

	if (req->type < ARRAY_SIZE(types))
		type = types[req->type];

	blob_buf_init(&b, 0);
	blobmsg_add_macaddr(&b, "address", addr);
	if (req->mgmt_frame)
		blobmsg_add_macaddr(&b, "target", req->mgmt_frame->da);
	if (req->ssi_signal)
		blobmsg_add_u32(&b, "signal", req->ssi_signal);
	blobmsg_add_u32(&b, "freq", hapd->iface->freq);

	if (req->elems) {
		if(req->elems->ht_capabilities)
		{
			struct ieee80211_ht_capabilities *ht_capabilities;
			void *ht_cap, *ht_cap_mcs_set, *mcs_set;


			ht_capabilities = (struct ieee80211_ht_capabilities*) req->elems->ht_capabilities;
			ht_cap = blobmsg_open_table(&b, "ht_capabilities");
			blobmsg_add_u16(&b, "ht_capabilities_info", ht_capabilities->ht_capabilities_info);
			ht_cap_mcs_set = blobmsg_open_table(&b, "supported_mcs_set");
			blobmsg_add_u16(&b, "a_mpdu_params", ht_capabilities->a_mpdu_params);
			blobmsg_add_u16(&b, "ht_extended_capabilities", ht_capabilities->ht_extended_capabilities);
			blobmsg_add_u32(&b, "tx_bf_capability_info", ht_capabilities->tx_bf_capability_info);
			blobmsg_add_u16(&b, "asel_capabilities", ht_capabilities->asel_capabilities);
			mcs_set = blobmsg_open_array(&b, "supported_mcs_set");
			for (int i = 0; i < 16; i++) {
				blobmsg_add_u16(&b, NULL, (u16) ht_capabilities->supported_mcs_set[i]);
			}
			blobmsg_close_array(&b, mcs_set);
			blobmsg_close_table(&b, ht_cap_mcs_set);
			blobmsg_close_table(&b, ht_cap);
		}
		if(req->elems->vht_capabilities)
		{
			struct ieee80211_vht_capabilities *vht_capabilities;
			void *vht_cap, *vht_cap_mcs_set;

			vht_capabilities = (struct ieee80211_vht_capabilities*) req->elems->vht_capabilities;
			vht_cap = blobmsg_open_table(&b, "vht_capabilities");
			blobmsg_add_u32(&b, "vht_capabilities_info", vht_capabilities->vht_capabilities_info);
			vht_cap_mcs_set = blobmsg_open_table(&b, "vht_supported_mcs_set");
			blobmsg_add_u16(&b, "rx_map", vht_capabilities->vht_supported_mcs_set.rx_map);
			blobmsg_add_u16(&b, "rx_highest", vht_capabilities->vht_supported_mcs_set.rx_highest);
			blobmsg_add_u16(&b, "tx_map", vht_capabilities->vht_supported_mcs_set.tx_map);
			blobmsg_add_u16(&b, "tx_highest", vht_capabilities->vht_supported_mcs_set.tx_highest);
			blobmsg_close_table(&b, vht_cap_mcs_set);
			blobmsg_close_table(&b, vht_cap);
		}
	}

	if (!hapd->ubus.notify_response) {
		ubus_notify(ctx, &hapd->ubus.obj, type, b.head, -1);
		return WLAN_STATUS_SUCCESS;
	}

	if (ubus_notify_async(ctx, &hapd->ubus.obj, type, b.head, &ureq.nreq))
		return WLAN_STATUS_SUCCESS;

	ureq.nreq.status_cb = ubus_event_cb;
	ubus_complete_request(ctx, &ureq.nreq.req, 100);

	if (ureq.resp)
		return ureq.resp;

	return WLAN_STATUS_SUCCESS;
}

void hostapd_ubus_notify(struct hostapd_data *hapd, const char *type, const u8 *addr)
{
	if (!hapd->ubus.obj.has_subscribers)
		return;

	if (!addr)
		return;

	blob_buf_init(&b, 0);
	blobmsg_add_macaddr(&b, "address", addr);

	ubus_notify(ctx, &hapd->ubus.obj, type, b.head, -1);
}

void hostapd_ubus_notify_beacon_report(
	struct hostapd_data *hapd, const u8 *addr, u8 token, u8 rep_mode,
	struct rrm_measurement_beacon_report *rep, size_t len)
{
	if (!hapd->ubus.obj.has_subscribers)
		return;

	if (!addr || !rep)
		return;

	blob_buf_init(&b, 0);
	blobmsg_add_macaddr(&b, "address", addr);
	blobmsg_add_u16(&b, "op-class", rep->op_class);
	blobmsg_add_u16(&b, "channel", rep->channel);
	blobmsg_add_u64(&b, "start-time", rep->start_time);
	blobmsg_add_u16(&b, "duration", rep->duration);
	blobmsg_add_u16(&b, "report-info", rep->report_info);
	blobmsg_add_u16(&b, "rcpi", rep->rcpi);
	blobmsg_add_u16(&b, "rsni", rep->rsni);
	blobmsg_add_macaddr(&b, "bssid", rep->bssid);
	blobmsg_add_u16(&b, "antenna-id", rep->antenna_id);
	blobmsg_add_u16(&b, "parent-tsf", rep->parent_tsf);

	ubus_notify(ctx, &hapd->ubus.obj, "beacon-report", b.head, -1);
}
