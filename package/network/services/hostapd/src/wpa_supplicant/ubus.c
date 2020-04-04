/*
 * wpa_supplicant / ubus support
 * Copyright (c) 2018, Daniel Golle <daniel@makrotopia.org>
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
#include "wpa_supplicant_i.h"
#include "wps_supplicant.h"
#include "ubus.h"

static struct ubus_context *ctx;
static struct blob_buf b;
static int ctx_ref;

static inline struct wpa_global *get_wpa_global_from_object(struct ubus_object *obj)
{
	return container_of(obj, struct wpa_global, ubus_global);
}

static inline struct wpa_supplicant *get_wpas_from_object(struct ubus_object *obj)
{
	return container_of(obj, struct wpa_supplicant, ubus.obj);
}

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

static void wpas_ubus_connection_lost(struct ubus_context *ctx)
{
	eloop_unregister_read_sock(ctx->sock.fd);
	eloop_register_timeout(1, 0, ubus_reconnect_timeout, ctx, NULL);
}

static bool wpas_ubus_init(void)
{
	if (ctx)
		return true;

	ctx = ubus_connect(NULL);
	if (!ctx)
		return false;

	ctx->connection_lost = wpas_ubus_connection_lost;
	eloop_register_read_sock(ctx->sock.fd, ubus_receive, ctx, NULL);
	return true;
}

static void wpas_ubus_ref_inc(void)
{
	ctx_ref++;
}

static void wpas_ubus_ref_dec(void)
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

static int
wpas_bss_get_features(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct wpa_supplicant *wpa_s = get_wpas_from_object(obj);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "ht_supported", ht_supported(wpa_s->hw.modes));
	blobmsg_add_u8(&b, "vht_supported", vht_supported(wpa_s->hw.modes));
	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
wpas_bss_reload(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct wpa_supplicant *wpa_s = get_wpas_from_object(obj);

	if (wpa_supplicant_reload_configuration(wpa_s))
		return UBUS_STATUS_UNKNOWN_ERROR;
	else
		return 0;
}

#ifdef CONFIG_WPS
enum {
	WPS_START_MULTI_AP,
	__WPS_START_MAX
};

static const struct blobmsg_policy wps_start_policy[] = {
	[WPS_START_MULTI_AP] = { "multi_ap", BLOBMSG_TYPE_BOOL },
};

static int
wpas_bss_wps_start(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	int rc;
	struct wpa_supplicant *wpa_s = get_wpas_from_object(obj);
	struct blob_attr *tb[__WPS_START_MAX], *cur;
	int multi_ap = 0;

	blobmsg_parse(wps_start_policy, __WPS_START_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));

	if (tb[WPS_START_MULTI_AP])
		multi_ap = blobmsg_get_bool(tb[WPS_START_MULTI_AP]);

	rc = wpas_wps_start_pbc(wpa_s, NULL, 0, multi_ap);

	if (rc != 0)
		return UBUS_STATUS_NOT_SUPPORTED;

	return 0;
}

static int
wpas_bss_wps_cancel(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	int rc;
	struct wpa_supplicant *wpa_s = get_wpas_from_object(obj);

	rc = wpas_wps_cancel(wpa_s);

	if (rc != 0)
		return UBUS_STATUS_NOT_SUPPORTED;

	return 0;
}
#endif

static const struct ubus_method bss_methods[] = {
	UBUS_METHOD_NOARG("reload", wpas_bss_reload),
	UBUS_METHOD_NOARG("get_features", wpas_bss_get_features),
#ifdef CONFIG_WPS
	UBUS_METHOD_NOARG("wps_start", wpas_bss_wps_start),
	UBUS_METHOD_NOARG("wps_cancel", wpas_bss_wps_cancel),
#endif
};

static struct ubus_object_type bss_object_type =
	UBUS_OBJECT_TYPE("wpas_bss", bss_methods);

void wpas_ubus_add_bss(struct wpa_supplicant *wpa_s)
{
	struct ubus_object *obj = &wpa_s->ubus.obj;
	char *name;
	int ret;

	if (!wpas_ubus_init())
		return;

	if (asprintf(&name, "wpa_supplicant.%s", wpa_s->ifname) < 0)
		return;

	obj->name = name;
	obj->type = &bss_object_type;
	obj->methods = bss_object_type.methods;
	obj->n_methods = bss_object_type.n_methods;
	ret = ubus_add_object(ctx, obj);
	wpas_ubus_ref_inc();
}

void wpas_ubus_free_bss(struct wpa_supplicant *wpa_s)
{
	struct ubus_object *obj = &wpa_s->ubus.obj;
	char *name = (char *) obj->name;

	if (!ctx)
		return;

	if (obj->id) {
		ubus_remove_object(ctx, obj);
		wpas_ubus_ref_dec();
	}

	free(name);
}

enum {
	WPAS_CONFIG_DRIVER,
	WPAS_CONFIG_IFACE,
	WPAS_CONFIG_BRIDGE,
	WPAS_CONFIG_HOSTAPD_CTRL,
	WPAS_CONFIG_CTRL,
	WPAS_CONFIG_FILE,
	__WPAS_CONFIG_MAX
};

static const struct blobmsg_policy wpas_config_add_policy[__WPAS_CONFIG_MAX] = {
	[WPAS_CONFIG_DRIVER] = { "driver", BLOBMSG_TYPE_STRING },
	[WPAS_CONFIG_IFACE] = { "iface", BLOBMSG_TYPE_STRING },
	[WPAS_CONFIG_BRIDGE] = { "bridge", BLOBMSG_TYPE_STRING },
	[WPAS_CONFIG_HOSTAPD_CTRL] = { "hostapd_ctrl", BLOBMSG_TYPE_STRING },
	[WPAS_CONFIG_CTRL] = { "ctrl", BLOBMSG_TYPE_STRING },
	[WPAS_CONFIG_FILE] = { "config", BLOBMSG_TYPE_STRING },
};

static int
wpas_config_add(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__WPAS_CONFIG_MAX];
	struct wpa_global *global = get_wpa_global_from_object(obj);
	struct wpa_interface *iface;

	blobmsg_parse(wpas_config_add_policy, __WPAS_CONFIG_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[WPAS_CONFIG_FILE] || !tb[WPAS_CONFIG_IFACE] || !tb[WPAS_CONFIG_DRIVER])
		return UBUS_STATUS_INVALID_ARGUMENT;

	iface = os_zalloc(sizeof(struct wpa_interface));
	if (iface == NULL)
		return UBUS_STATUS_UNKNOWN_ERROR;

	iface->driver = blobmsg_get_string(tb[WPAS_CONFIG_DRIVER]);
	iface->ifname = blobmsg_get_string(tb[WPAS_CONFIG_IFACE]);
	iface->confname = blobmsg_get_string(tb[WPAS_CONFIG_FILE]);

	if (tb[WPAS_CONFIG_BRIDGE])
		iface->bridge_ifname = blobmsg_get_string(tb[WPAS_CONFIG_BRIDGE]);

	if (tb[WPAS_CONFIG_CTRL])
		iface->ctrl_interface = blobmsg_get_string(tb[WPAS_CONFIG_CTRL]);

	if (tb[WPAS_CONFIG_HOSTAPD_CTRL])
		iface->hostapd_ctrl = blobmsg_get_string(tb[WPAS_CONFIG_HOSTAPD_CTRL]);

	if (!wpa_supplicant_add_iface(global, iface, NULL))
		return UBUS_STATUS_INVALID_ARGUMENT;

	return UBUS_STATUS_OK;
}

enum {
	WPAS_CONFIG_REM_IFACE,
	__WPAS_CONFIG_REM_MAX
};

static const struct blobmsg_policy wpas_config_remove_policy[__WPAS_CONFIG_REM_MAX] = {
	[WPAS_CONFIG_REM_IFACE] = { "iface", BLOBMSG_TYPE_STRING },
};

static int
wpas_config_remove(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	struct blob_attr *tb[__WPAS_CONFIG_REM_MAX];
	struct wpa_global *global = get_wpa_global_from_object(obj);
	struct wpa_supplicant *wpa_s = NULL;
	unsigned int found = 0;

	blobmsg_parse(wpas_config_remove_policy, __WPAS_CONFIG_REM_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[WPAS_CONFIG_REM_IFACE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	/* find wpa_s object for to-be-removed interface */
	for (wpa_s = global->ifaces; wpa_s; wpa_s = wpa_s->next) {
		if (!strncmp(wpa_s->ifname,
			     blobmsg_get_string(tb[WPAS_CONFIG_REM_IFACE]),
			     sizeof(wpa_s->ifname)))
		{
			found = 1;
			break;
		}
	}

	if (!found)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (wpa_supplicant_remove_iface(global, wpa_s, 0))
		return UBUS_STATUS_INVALID_ARGUMENT;

	return UBUS_STATUS_OK;
}

static const struct ubus_method wpas_daemon_methods[] = {
	UBUS_METHOD("config_add", wpas_config_add, wpas_config_add_policy),
	UBUS_METHOD("config_remove", wpas_config_remove, wpas_config_remove_policy),
};

static struct ubus_object_type wpas_daemon_object_type =
	UBUS_OBJECT_TYPE("wpa_supplicant", wpas_daemon_methods);

void wpas_ubus_add(struct wpa_global *global)
{
	struct ubus_object *obj = &global->ubus_global;
	char *name;
	int name_len;
	int ret;

	if (!wpas_ubus_init())
		return;

	name_len = strlen("wpa_supplicant") + 1;
	if (global->params.name)
		name_len += strlen(global->params.name) + 1;

	name = malloc(name_len);
	strcpy(name, "wpa_supplicant");

	if (global->params.name)
	{
		strcat(name, ".");
		strcat(name, global->params.name);
	}

	obj->name = name;

	obj->type = &wpas_daemon_object_type;
	obj->methods = wpas_daemon_object_type.methods;
	obj->n_methods = wpas_daemon_object_type.n_methods;
	ret = ubus_add_object(ctx, obj);
	wpas_ubus_ref_inc();
}

void wpas_ubus_free(struct wpa_global *global)
{
	struct ubus_object *obj = &global->ubus_global;
	char *name = (char *) obj->name;

	if (!ctx)
		return;

	if (obj->id) {
		ubus_remove_object(ctx, obj);
		wpas_ubus_ref_dec();
	}

	free(name);
}


#ifdef CONFIG_WPS
void wpas_ubus_notify(struct wpa_supplicant *wpa_s, const struct wps_credential *cred)
{
	u16 auth_type;
	char *ifname, *encryption, *ssid, *key;
	size_t ifname_len;

	if (!cred)
		return;

	auth_type = cred->auth_type;

	if (auth_type == (WPS_AUTH_WPAPSK | WPS_AUTH_WPA2PSK))
		auth_type = WPS_AUTH_WPA2PSK;

	if (auth_type != WPS_AUTH_OPEN &&
	    auth_type != WPS_AUTH_WPAPSK &&
	    auth_type != WPS_AUTH_WPA2PSK) {
		wpa_printf(MSG_DEBUG, "WPS: Ignored credentials for "
			   "unsupported authentication type 0x%x",
			   auth_type);
		return;
	}

	if (auth_type == WPS_AUTH_WPAPSK || auth_type == WPS_AUTH_WPA2PSK) {
		if (cred->key_len < 8 || cred->key_len > 2 * PMK_LEN) {
			wpa_printf(MSG_ERROR, "WPS: Reject PSK credential with "
				   "invalid Network Key length %lu",
				   (unsigned long) cred->key_len);
			return;
		}
	}

	blob_buf_init(&b, 0);

	ifname_len = strlen(wpa_s->ifname);
	ifname = blobmsg_alloc_string_buffer(&b, "ifname", ifname_len + 1);
	memcpy(ifname, wpa_s->ifname, ifname_len + 1);
	ifname[ifname_len] = '\0';
	blobmsg_add_string_buffer(&b);

	switch (auth_type) {
		case WPS_AUTH_WPA2PSK:
			encryption = "psk2";
			break;
		case WPS_AUTH_WPAPSK:
			encryption = "psk";
			break;
		default:
			encryption = "none";
			break;
	}

	blobmsg_add_string(&b, "encryption", encryption);

	ssid = blobmsg_alloc_string_buffer(&b, "ssid", cred->ssid_len + 1);
	memcpy(ssid, cred->ssid, cred->ssid_len);
	ssid[cred->ssid_len] = '\0';
	blobmsg_add_string_buffer(&b);

	if (cred->key_len > 0) {
		key = blobmsg_alloc_string_buffer(&b, "key", cred->key_len + 1);
		memcpy(key, cred->key, cred->key_len);
		key[cred->key_len] = '\0';
		blobmsg_add_string_buffer(&b);
	}

//	ubus_notify(ctx, &wpa_s->ubus.obj, "wps_credentials", b.head, -1);
	ubus_send_event(ctx, "wps_credentials", b.head);
}
#endif /* CONFIG_WPS */
