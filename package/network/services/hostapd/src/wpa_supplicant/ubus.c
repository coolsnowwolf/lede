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

#ifdef CONFIG_WPS
static int
wpas_bss_wps_start(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	int rc;
	struct wpa_supplicant *wpa_s = get_wpas_from_object(obj);

	rc = wpas_wps_start_pbc(wpa_s, NULL, 0);

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
#ifdef CONFIG_WPS
	UBUS_METHOD_NOARG("wps_start", wpas_bss_wps_start),
	UBUS_METHOD_NOARG("wps_cancel", wpas_bss_wps_cancel),
#endif
	UBUS_METHOD_NOARG("get_features", wpas_bss_get_features),
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
