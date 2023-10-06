#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "crypto/crypto.h"
#include "crypto/tls.h"

#include "ap/ap_config.h"
#include "eap_server/eap.h"
#include "radius/radius.h"
#include "radius/radius_server.h"
#include "eap_register.h"

#include <libubox/blobmsg_json.h>
#include <libubox/blobmsg.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/kvlist.h>

#include <sys/stat.h>
#include <fnmatch.h>

#define VENDOR_ID_WISPR 14122
#define VENDOR_ATTR_SIZE 6

struct radius_parse_attr_data {
	unsigned int vendor;
	u8 type;
	int size;
	char format;
	const char *data;
};

struct radius_parse_attr_state {
	struct hostapd_radius_attr *prev;
	struct hostapd_radius_attr *attr;
	struct wpabuf *buf;
	void *attrdata;
};

struct radius_user_state {
	struct avl_node node;
	struct eap_user data;
};

struct radius_user_data {
	struct kvlist users;
	struct avl_tree user_state;
	struct blob_attr *wildcard;
};

struct radius_state {
	struct radius_server_data *radius;
	struct eap_config eap;

	struct radius_user_data phase1, phase2;
	const char *user_file;
	time_t user_file_ts;

	int n_attrs;
	struct hostapd_radius_attr *attrs;
};

struct radius_config {
	struct tls_connection_params tls;
	struct radius_server_conf radius;
};

enum {
	USER_ATTR_PASSWORD,
	USER_ATTR_HASH,
	USER_ATTR_SALT,
	USER_ATTR_METHODS,
	USER_ATTR_RADIUS,
	USER_ATTR_VLAN,
	USER_ATTR_MAX_RATE_UP,
	USER_ATTR_MAX_RATE_DOWN,
	__USER_ATTR_MAX
};

static void radius_tls_event(void *ctx, enum tls_event ev,
			      union tls_event_data *data)
{
	switch (ev) {
	case TLS_CERT_CHAIN_SUCCESS:
		wpa_printf(MSG_DEBUG, "radius: remote certificate verification success");
		break;
	case TLS_CERT_CHAIN_FAILURE:
		wpa_printf(MSG_INFO, "radius: certificate chain failure: reason=%d depth=%d subject='%s' err='%s'",
			   data->cert_fail.reason,
			   data->cert_fail.depth,
			   data->cert_fail.subject,
			   data->cert_fail.reason_txt);
		break;
	case TLS_PEER_CERTIFICATE:
		wpa_printf(MSG_DEBUG, "radius: peer certificate: depth=%d serial_num=%s subject=%s",
			   data->peer_cert.depth,
			   data->peer_cert.serial_num ? data->peer_cert.serial_num : "N/A",
			   data->peer_cert.subject);
		break;
	case TLS_ALERT:
		if (data->alert.is_local)
			wpa_printf(MSG_DEBUG, "radius: local TLS alert: %s",
				   data->alert.description);
		else
			wpa_printf(MSG_DEBUG, "radius: remote TLS alert: %s",
				   data->alert.description);
		break;
	case TLS_UNSAFE_RENEGOTIATION_DISABLED:
		/* Not applicable to TLS server */
		break;
	}
}

static void radius_userdata_init(struct radius_user_data *u)
{
	kvlist_init(&u->users, kvlist_blob_len);
	avl_init(&u->user_state, avl_strcmp, false, NULL);
}

static void radius_userdata_free(struct radius_user_data *u)
{
	struct radius_user_state *s, *tmp;

	kvlist_free(&u->users);
	free(u->wildcard);
	u->wildcard = NULL;
	avl_remove_all_elements(&u->user_state, s, node, tmp)
		free(s);
}

static void
radius_userdata_load(struct radius_user_data *u, struct blob_attr *data)
{
	enum {
		USERSTATE_USERS,
		USERSTATE_WILDCARD,
		__USERSTATE_MAX,
	};
	static const struct blobmsg_policy policy[__USERSTATE_MAX] = {
		[USERSTATE_USERS] = { "users", BLOBMSG_TYPE_TABLE },
		[USERSTATE_WILDCARD] = { "wildcard", BLOBMSG_TYPE_ARRAY },
	};
	struct blob_attr *tb[__USERSTATE_MAX], *cur;
	int rem;

	if (!data)
		return;

	blobmsg_parse(policy, __USERSTATE_MAX, tb, blobmsg_data(data), blobmsg_len(data));

	blobmsg_for_each_attr(cur, tb[USERSTATE_USERS], rem)
		kvlist_set(&u->users, blobmsg_name(cur), cur);

	if (tb[USERSTATE_WILDCARD])
		u->wildcard = blob_memdup(tb[USERSTATE_WILDCARD]);
}

static void
load_userfile(struct radius_state *s)
{
	enum {
		USERDATA_PHASE1,
		USERDATA_PHASE2,
		__USERDATA_MAX
	};
	static const struct blobmsg_policy policy[__USERDATA_MAX] = {
		[USERDATA_PHASE1] = { "phase1", BLOBMSG_TYPE_TABLE },
		[USERDATA_PHASE2] = { "phase2", BLOBMSG_TYPE_TABLE },
	};
	struct blob_attr *tb[__USERDATA_MAX], *cur;
	static struct blob_buf b;
	struct stat st;
	int rem;

	if (stat(s->user_file, &st))
		return;

	if (s->user_file_ts == st.st_mtime)
		return;

	s->user_file_ts = st.st_mtime;
	radius_userdata_free(&s->phase1);
	radius_userdata_free(&s->phase2);

	blob_buf_init(&b, 0);
	blobmsg_add_json_from_file(&b, s->user_file);
	blobmsg_parse(policy, __USERDATA_MAX, tb, blob_data(b.head), blob_len(b.head));
	radius_userdata_load(&s->phase1, tb[USERDATA_PHASE1]);
	radius_userdata_load(&s->phase2, tb[USERDATA_PHASE2]);

	blob_buf_free(&b);
}

static struct blob_attr *
radius_user_get(struct radius_user_data *s, const char *name)
{
	struct blob_attr *cur;
	int rem;

	cur = kvlist_get(&s->users, name);
	if (cur)
		return cur;

	blobmsg_for_each_attr(cur, s->wildcard, rem) {
		static const struct blobmsg_policy policy = {
			"name", BLOBMSG_TYPE_STRING
		};
		struct blob_attr *pattern;

		if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE)
			continue;

		blobmsg_parse(&policy, 1, &pattern, blobmsg_data(cur), blobmsg_len(cur));
		if (!name)
			continue;

		if (!fnmatch(blobmsg_get_string(pattern), name, 0))
			return cur;
	}

	return NULL;
}

static struct radius_parse_attr_data *
radius_parse_attr(struct blob_attr *attr)
{
	static const struct blobmsg_policy policy[4] = {
		{ .type = BLOBMSG_TYPE_INT32 },
		{ .type = BLOBMSG_TYPE_INT32 },
		{ .type = BLOBMSG_TYPE_STRING },
		{ .type = BLOBMSG_TYPE_STRING },
	};
	static struct radius_parse_attr_data data;
	struct blob_attr *tb[4];
	const char *format;

	blobmsg_parse_array(policy, ARRAY_SIZE(policy), tb, blobmsg_data(attr), blobmsg_len(attr));

	if (!tb[0] || !tb[1] || !tb[2] || !tb[3])
		return NULL;

	format = blobmsg_get_string(tb[2]);
	if (strlen(format) != 1)
		return NULL;

	data.vendor = blobmsg_get_u32(tb[0]);
	data.type = blobmsg_get_u32(tb[1]);
	data.format = format[0];
	data.data = blobmsg_get_string(tb[3]);
	data.size = strlen(data.data);

	switch (data.format) {
	case 's':
		break;
	case 'x':
		if (data.size & 1)
			return NULL;
		data.size /= 2;
		break;
	case 'd':
		data.size = 4;
		break;
	default:
		return NULL;
	}

	return &data;
}

static void
radius_count_attrs(struct blob_attr **tb, int *n_attr, size_t *attr_size)
{
	struct blob_attr *data = tb[USER_ATTR_RADIUS];
	struct blob_attr *cur;
	int rem;

	blobmsg_for_each_attr(cur, data, rem) {
		struct radius_parse_attr_data *data;
		size_t prev = *attr_size;

		data = radius_parse_attr(cur);
		if (!data)
			continue;

		*attr_size += data->size;
		if (data->vendor)
			*attr_size += VENDOR_ATTR_SIZE;

		(*n_attr)++;
	}

	*n_attr += !!tb[USER_ATTR_VLAN] * 3 +
		   !!tb[USER_ATTR_MAX_RATE_UP] +
		   !!tb[USER_ATTR_MAX_RATE_DOWN];
	*attr_size += !!tb[USER_ATTR_VLAN] * (4 + 4 + 5) +
		      !!tb[USER_ATTR_MAX_RATE_UP] * (4 + VENDOR_ATTR_SIZE) +
		      !!tb[USER_ATTR_MAX_RATE_DOWN] * (4 + VENDOR_ATTR_SIZE);
}

static void *
radius_add_attr(struct radius_parse_attr_state *state,
		u32 vendor, u8 type, u8 len)
{
	struct hostapd_radius_attr *attr;
	struct wpabuf *buf;
	void *val;

	val = state->attrdata;

	buf = state->buf++;
	buf->buf = val;

	attr = state->attr++;
	attr->val = buf;
	attr->type = type;

	if (state->prev)
		state->prev->next = attr;
	state->prev = attr;

	if (vendor) {
		u8 *vendor_hdr = val + 4;

		WPA_PUT_BE32(val, vendor);
		vendor_hdr[0] = type;
		vendor_hdr[1] = len + 2;

		len += VENDOR_ATTR_SIZE;
		val += VENDOR_ATTR_SIZE;
		attr->type = RADIUS_ATTR_VENDOR_SPECIFIC;
	}

	buf->size = buf->used = len;
	state->attrdata += len;

	return val;
}

static void
radius_parse_attrs(struct blob_attr **tb, struct radius_parse_attr_state *state)
{
	struct blob_attr *data = tb[USER_ATTR_RADIUS];
	struct hostapd_radius_attr *prev = NULL;
	struct blob_attr *cur;
	int len, rem;
	void *val;

	if ((cur = tb[USER_ATTR_VLAN]) != NULL && blobmsg_get_u32(cur) < 4096) {
		char buf[5];

		val = radius_add_attr(state, 0, RADIUS_ATTR_TUNNEL_TYPE, 4);
		WPA_PUT_BE32(val, RADIUS_TUNNEL_TYPE_VLAN);

		val = radius_add_attr(state, 0, RADIUS_ATTR_TUNNEL_MEDIUM_TYPE, 4);
		WPA_PUT_BE32(val, RADIUS_TUNNEL_MEDIUM_TYPE_802);

		len = snprintf(buf, sizeof(buf), "%d", blobmsg_get_u32(cur));
		val = radius_add_attr(state, 0, RADIUS_ATTR_TUNNEL_PRIVATE_GROUP_ID, len);
		memcpy(val, buf, len);
	}

	if ((cur = tb[USER_ATTR_MAX_RATE_UP]) != NULL) {
		val = radius_add_attr(state, VENDOR_ID_WISPR, 7, 4);
		WPA_PUT_BE32(val, blobmsg_get_u32(cur));
	}

	if ((cur = tb[USER_ATTR_MAX_RATE_DOWN]) != NULL) {
		val = radius_add_attr(state, VENDOR_ID_WISPR, 8, 4);
		WPA_PUT_BE32(val, blobmsg_get_u32(cur));
	}

	blobmsg_for_each_attr(cur, data, rem) {
		struct radius_parse_attr_data *data;
		void *val;
		int size;

		data = radius_parse_attr(cur);
		if (!data)
			continue;

		val = radius_add_attr(state, data->vendor, data->type, data->size);
		switch (data->format) {
		case 's':
			memcpy(val, data->data, data->size);
			break;
		case 'x':
			hexstr2bin(data->data, val, data->size);
			break;
		case 'd':
			WPA_PUT_BE32(val, atoi(data->data));
			break;
		}
	}
}

static void
radius_user_parse_methods(struct eap_user *eap, struct blob_attr *data)
{
	struct blob_attr *cur;
	int rem, n = 0;

	if (!data)
		return;

	blobmsg_for_each_attr(cur, data, rem) {
		const char *method;

		if (blobmsg_type(cur) != BLOBMSG_TYPE_STRING)
			continue;

		if (n == EAP_MAX_METHODS)
			break;

		method = blobmsg_get_string(cur);
		eap->methods[n].method = eap_server_get_type(method, &eap->methods[n].vendor);
		if (eap->methods[n].vendor == EAP_VENDOR_IETF &&
		    eap->methods[n].method == EAP_TYPE_NONE) {
			if (!strcmp(method, "TTLS-PAP")) {
				eap->ttls_auth |= EAP_TTLS_AUTH_PAP;
				continue;
			}
			if (!strcmp(method, "TTLS-CHAP")) {
				eap->ttls_auth |= EAP_TTLS_AUTH_CHAP;
				continue;
			}
			if (!strcmp(method, "TTLS-MSCHAP")) {
				eap->ttls_auth |= EAP_TTLS_AUTH_MSCHAP;
				continue;
			}
			if (!strcmp(method, "TTLS-MSCHAPV2")) {
				eap->ttls_auth |= EAP_TTLS_AUTH_MSCHAPV2;
				continue;
			}
		}
		n++;
	}
}

static struct eap_user *
radius_user_get_state(struct radius_user_data *u, struct blob_attr *data,
		      const char *id)
{
	static const struct blobmsg_policy policy[__USER_ATTR_MAX] = {
		[USER_ATTR_PASSWORD] = { "password", BLOBMSG_TYPE_STRING },
		[USER_ATTR_HASH] = { "hash", BLOBMSG_TYPE_STRING },
		[USER_ATTR_SALT] = { "salt", BLOBMSG_TYPE_STRING },
		[USER_ATTR_METHODS] = { "methods", BLOBMSG_TYPE_ARRAY },
		[USER_ATTR_RADIUS] = { "radius", BLOBMSG_TYPE_ARRAY },
		[USER_ATTR_VLAN] = { "vlan-id", BLOBMSG_TYPE_INT32 },
		[USER_ATTR_MAX_RATE_UP] = { "max-rate-up", BLOBMSG_TYPE_INT32 },
		[USER_ATTR_MAX_RATE_DOWN] = { "max-rate-down", BLOBMSG_TYPE_INT32 },
	};
	struct blob_attr *tb[__USER_ATTR_MAX], *cur;
	char *password_buf, *salt_buf, *name_buf;
	struct radius_parse_attr_state astate = {};
	struct hostapd_radius_attr *attr;
	struct radius_user_state *state;
	int pw_len = 0, salt_len = 0;
	struct eap_user *eap;
	struct wpabuf *val;
	size_t attrsize = 0;
	void *attrdata;
	int n_attr = 0;

	state = avl_find_element(&u->user_state, id, state, node);
	if (state)
		return &state->data;

	blobmsg_parse(policy, __USER_ATTR_MAX, tb, blobmsg_data(data), blobmsg_len(data));

	if ((cur = tb[USER_ATTR_SALT]) != NULL)
		salt_len = strlen(blobmsg_get_string(cur)) / 2;
	if ((cur = tb[USER_ATTR_HASH]) != NULL)
		pw_len = strlen(blobmsg_get_string(cur)) / 2;
	else if ((cur = tb[USER_ATTR_PASSWORD]) != NULL)
		pw_len = blobmsg_len(cur) - 1;
	radius_count_attrs(tb, &n_attr, &attrsize);

	state = calloc_a(sizeof(*state), &name_buf, strlen(id) + 1,
			 &password_buf, pw_len,
			 &salt_buf, salt_len,
			 &astate.attr, n_attr * sizeof(*astate.attr),
			 &astate.buf, n_attr * sizeof(*astate.buf),
			 &astate.attrdata, attrsize);
	eap = &state->data;
	eap->salt = salt_len ? salt_buf : NULL;
	eap->salt_len = salt_len;
	eap->password = pw_len ? password_buf : NULL;
	eap->password_len = pw_len;
	eap->force_version = -1;

	if ((cur = tb[USER_ATTR_SALT]) != NULL)
		hexstr2bin(blobmsg_get_string(cur), salt_buf, salt_len);
	if ((cur = tb[USER_ATTR_PASSWORD]) != NULL)
		memcpy(password_buf, blobmsg_get_string(cur), pw_len);
	else if ((cur = tb[USER_ATTR_HASH]) != NULL) {
		hexstr2bin(blobmsg_get_string(cur), password_buf, pw_len);
		eap->password_hash = 1;
	}
	radius_user_parse_methods(eap, tb[USER_ATTR_METHODS]);

	if (n_attr > 0) {
		cur = tb[USER_ATTR_RADIUS];
		eap->accept_attr = astate.attr;
		radius_parse_attrs(tb, &astate);
	}

	state->node.key = strcpy(name_buf, id);
	avl_insert(&u->user_state, &state->node);

	return &state->data;

free:
	free(state);
	return NULL;
}

static int radius_get_eap_user(void *ctx, const u8 *identity,
			       size_t identity_len, int phase2,
			       struct eap_user *user)
{
	struct radius_state *s = ctx;
	struct radius_user_data *u = phase2 ? &s->phase2 : &s->phase1;
	struct blob_attr *entry;
	struct eap_user *data;
	char *id;

	if (identity_len > 512)
		return -1;

	load_userfile(s);

	id = alloca(identity_len + 1);
	memcpy(id, identity, identity_len);
	id[identity_len] = 0;

	entry = radius_user_get(u, id);
	if (!entry)
		return -1;

	if (!user)
		return 0;

	data = radius_user_get_state(u, entry, id);
	if (!data)
		return -1;

	*user = *data;
	if (user->password_len > 0)
		user->password = os_memdup(user->password, user->password_len);
	if (user->salt_len > 0)
		user->salt = os_memdup(user->salt, user->salt_len);
	user->phase2 = phase2;

	return 0;
}

static int radius_setup(struct radius_state *s, struct radius_config *c)
{
	struct eap_config *eap = &s->eap;
	struct tls_config conf = {
		.event_cb = radius_tls_event,
		.tls_flags = TLS_CONN_DISABLE_TLSv1_3,
		.cb_ctx = s,
	};

	eap->eap_server = 1;
	eap->max_auth_rounds = 100;
	eap->max_auth_rounds_short = 50;
	eap->ssl_ctx = tls_init(&conf);
	if (!eap->ssl_ctx) {
		wpa_printf(MSG_INFO, "TLS init failed\n");
		return 1;
	}

	if (tls_global_set_params(eap->ssl_ctx, &c->tls)) {
		wpa_printf(MSG_INFO, "failed to set TLS parameters\n");
		return 1;
	}

	c->radius.eap_cfg = eap;
	c->radius.conf_ctx = s;
	c->radius.get_eap_user = radius_get_eap_user;
	s->radius = radius_server_init(&c->radius);
	if (!s->radius) {
		wpa_printf(MSG_INFO, "failed to initialize radius server\n");
		return 1;
	}

	return 0;
}

static int radius_init(struct radius_state *s)
{
	memset(s, 0, sizeof(*s));
	radius_userdata_init(&s->phase1);
	radius_userdata_init(&s->phase2);
}

static void radius_deinit(struct radius_state *s)
{
	if (s->radius)
		radius_server_deinit(s->radius);

	if (s->eap.ssl_ctx)
		tls_deinit(s->eap.ssl_ctx);

	radius_userdata_free(&s->phase1);
	radius_userdata_free(&s->phase2);
}

static int usage(const char *progname)
{
	fprintf(stderr, "Usage: %s <options>\n",
		progname);
}

int radius_main(int argc, char **argv)
{
	static struct radius_state state = {};
	static struct radius_config config = {};
	const char *progname = argv[0];
	int ret = 0;
	int ch;

	wpa_debug_setup_stdout();
	wpa_debug_level = 0;

	if (eloop_init()) {
		wpa_printf(MSG_ERROR, "Failed to initialize event loop");
		return 1;
	}

	eap_server_register_methods();
	radius_init(&state);

	while ((ch = getopt(argc, argv, "6C:c:d:i:k:K:p:P:s:u:")) != -1) {
		switch (ch) {
		case '6':
			config.radius.ipv6 = 1;
			break;
		case 'C':
			config.tls.ca_cert = optarg;
			break;
		case 'c':
			if (config.tls.client_cert2)
				return usage(progname);

			if (config.tls.client_cert)
				config.tls.client_cert2 = optarg;
			else
				config.tls.client_cert = optarg;
			break;
		case 'd':
			config.tls.dh_file = optarg;
			break;
		case 'i':
			state.eap.server_id = optarg;
			state.eap.server_id_len = strlen(optarg);
			break;
		case 'k':
			if (config.tls.private_key2)
				return usage(progname);

			if (config.tls.private_key)
				config.tls.private_key2 = optarg;
			else
				config.tls.private_key = optarg;
			break;
		case 'K':
			if (config.tls.private_key_passwd2)
				return usage(progname);

			if (config.tls.private_key_passwd)
				config.tls.private_key_passwd2 = optarg;
			else
				config.tls.private_key_passwd = optarg;
			break;
		case 'p':
			config.radius.auth_port = atoi(optarg);
			break;
		case 'P':
			config.radius.acct_port = atoi(optarg);
			break;
		case 's':
			config.radius.client_file = optarg;
			break;
		case 'u':
			state.user_file = optarg;
			break;
		default:
			return usage(progname);
		}
	}

	if (!config.tls.client_cert || !config.tls.private_key ||
	    !config.radius.client_file || !state.eap.server_id ||
	    !state.user_file) {
		wpa_printf(MSG_INFO, "missing options\n");
		goto out;
	}

	ret = radius_setup(&state, &config);
	if (ret)
		goto out;

	load_userfile(&state);
	eloop_run();

out:
	radius_deinit(&state);
	os_program_deinit();

	return ret;
}
