
/*
*  ########## DO NOT EDIT ###########

Automatically generated on Tue, 19 Dec 2017 22:57:40 -0800

*
* Adapter from qcsapi.h functions
* to RPC client functions.
*/

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <qcsapi.h>
#include "qcsapi_rpc.h"
#include <qcsapi_rpc/client/qcsapi_rpc_client.h>

static int retries_limit = 3;

/* Default timeout can be changed using clnt_control() */
static struct timeval __timeout = { 25, 0 };

static CLIENT *__clnt = NULL;

static const int debug = 0;

static CLIENT *qcsapi_adapter_get_client(void)
{
	if (__clnt == NULL) {
		fprintf(stderr, "%s: client is null!\n", __FUNCTION__);
		exit (1);
	}

	return __clnt;
}

void client_qcsapi_set_rpcclient(CLIENT * clnt)
{
	__clnt = clnt;
}

static client_qcsapi_callback_pre_t __pre_callback = NULL;
static client_qcsapi_callback_post_t __post_callback = NULL;
static client_qcsapi_callback_reconnect_t __reconnect_callback = NULL;

void client_qcsapi_set_callbacks(client_qcsapi_callback_pre_t pre,
		client_qcsapi_callback_post_t post,
		client_qcsapi_callback_reconnect_t reconnect)
{
	__pre_callback = pre;
	__post_callback = post;
	__reconnect_callback = reconnect;
}

#define client_qcsapi_pre() __client_qcsapi_pre(__FUNCTION__)
static void __client_qcsapi_pre(const char *func)
{
	if (__pre_callback) {
		__pre_callback(func);
	}
}

#define client_qcsapi_post(x) __client_qcsapi_post(__FUNCTION__, (x))
static void __client_qcsapi_post(const char *func, int was_error)
{
	if (__post_callback) {
		__post_callback(func, was_error);
	}
}

#define client_qcsapi_reconnect() __client_qcsapi_reconnect(__FUNCTION__)
static void __client_qcsapi_reconnect(const char *func)
{
	if (__reconnect_callback) {
		__reconnect_callback(func);
	}
}

int qcsapi_bootcfg_get_parameter(const char * param_name, char * param_value, const size_t max_param_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_bootcfg_get_parameter_rpcdata __req;
	struct qcsapi_bootcfg_get_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	__req.max_param_len = (uint32_t)max_param_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_BOOTCFG_GET_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_bootcfg_get_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_bootcfg_get_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_bootcfg_get_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_bootcfg_get_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (param_value && __resp.param_value)
			strcpy(param_value, __resp.param_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_bootcfg_get_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_bootcfg_update_parameter(const char * param_name, const char * param_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_bootcfg_update_parameter_rpcdata __req;
	struct qcsapi_bootcfg_update_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_BOOTCFG_UPDATE_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_bootcfg_update_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_bootcfg_update_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_bootcfg_update_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_bootcfg_update_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_bootcfg_update_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_bootcfg_commit()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_bootcfg_commit_rpcdata __req;
	struct qcsapi_bootcfg_commit_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_BOOTCFG_COMMIT_REMOTE,
				(xdrproc_t)xdr_qcsapi_bootcfg_commit_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_bootcfg_commit_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_bootcfg_commit call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_bootcfg_commit_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_bootcfg_commit_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_telnet_enable(const qcsapi_unsigned_int onoff)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_telnet_enable_rpcdata __req;
	struct qcsapi_telnet_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.onoff = (unsigned int)onoff;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_TELNET_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_telnet_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_telnet_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_telnet_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_telnet_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_telnet_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_service_name_enum(const char * lookup_service, qcsapi_service_name * serv_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_service_name_enum_rpcdata __req;
	struct qcsapi_get_service_name_enum_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpclookup_service = {(char *)lookup_service};
	__rpc_string *p__rpclookup_service = (lookup_service) ? &__rpclookup_service : NULL;
	__req.lookup_service = p__rpclookup_service;

	__req.serv_name = (int *)serv_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_SERVICE_NAME_ENUM_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_service_name_enum_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_service_name_enum_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_service_name_enum call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_service_name_enum_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (serv_name)
			*serv_name = *__resp.serv_name;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_service_name_enum_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_service_action_enum(const char * lookup_action, qcsapi_service_action * serv_action)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_service_action_enum_rpcdata __req;
	struct qcsapi_get_service_action_enum_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpclookup_action = {(char *)lookup_action};
	__rpc_string *p__rpclookup_action = (lookup_action) ? &__rpclookup_action : NULL;
	__req.lookup_action = p__rpclookup_action;

	__req.serv_action = (int *)serv_action;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_SERVICE_ACTION_ENUM_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_service_action_enum_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_service_action_enum_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_service_action_enum call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_service_action_enum_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (serv_action)
			*serv_action = *__resp.serv_action;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_service_action_enum_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_service_control(qcsapi_service_name service, qcsapi_service_action action)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_service_control_rpcdata __req;
	struct qcsapi_service_control_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.service = (int)service;

	__req.action = (int)action;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SERVICE_CONTROL_REMOTE,
				(xdrproc_t)xdr_qcsapi_service_control_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_service_control_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_service_control call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_service_control_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_service_control_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wfa_cert_mode_enable(uint16_t enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wfa_cert_mode_enable_rpcdata __req;
	struct qcsapi_wfa_cert_mode_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.enable = (uint16_t)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WFA_CERT_MODE_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wfa_cert_mode_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wfa_cert_mode_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wfa_cert_mode_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wfa_cert_mode_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wfa_cert_mode_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_cce_channels(const char * ifname, qcsapi_unsigned_int * p_prev_channel, qcsapi_unsigned_int * p_cur_channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_cce_channels_rpcdata __req;
	struct qcsapi_wifi_get_scs_cce_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_prev_channel = (unsigned int *)p_prev_channel;

	__req.p_cur_channel = (unsigned int *)p_cur_channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_CCE_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_cce_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_cce_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_cce_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_cce_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_prev_channel)
			*p_prev_channel = *__resp.p_prev_channel;
	}
	if (__resp.return_code >= 0) {
		if (p_cur_channel)
			*p_cur_channel = *__resp.p_cur_channel;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_cce_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_scs_enable(const char * ifname, uint16_t enable_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_scs_enable_rpcdata __req;
	struct qcsapi_wifi_scs_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable_val = (uint16_t)enable_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SCS_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_scs_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_scs_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_scs_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_scs_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_scs_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_scs_switch_channel(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_scs_switch_channel_rpcdata __req;
	struct qcsapi_wifi_scs_switch_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SCS_SWITCH_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_scs_switch_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_scs_switch_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_scs_switch_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_scs_switch_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_scs_switch_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_verbose(const char * ifname, uint16_t enable_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_verbose_rpcdata __req;
	struct qcsapi_wifi_set_scs_verbose_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable_val = (uint16_t)enable_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_VERBOSE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_verbose_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_verbose_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_verbose call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_verbose_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_verbose_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_status(const char * ifname, qcsapi_unsigned_int * p_scs_status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_status_rpcdata __req;
	struct qcsapi_wifi_get_scs_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_scs_status = (unsigned int *)p_scs_status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_scs_status)
			*p_scs_status = *__resp.p_scs_status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_smpl_enable(const char * ifname, uint16_t enable_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_smpl_enable_rpcdata __req;
	struct qcsapi_wifi_set_scs_smpl_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable_val = (uint16_t)enable_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_SMPL_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_smpl_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_smpl_dwell_time(const char * ifname, uint16_t scs_sample_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_smpl_dwell_time_rpcdata __req;
	struct qcsapi_wifi_set_scs_smpl_dwell_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_sample_time = (uint16_t)scs_sample_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_SMPL_DWELL_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_dwell_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_dwell_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_smpl_dwell_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_dwell_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_smpl_dwell_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_sample_intv(const char * ifname, uint16_t scs_sample_intv)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_sample_intv_rpcdata __req;
	struct qcsapi_wifi_set_scs_sample_intv_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_sample_intv = (uint16_t)scs_sample_intv;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_SAMPLE_INTV_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_sample_intv_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_sample_intv_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_sample_intv call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_sample_intv_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_sample_intv_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_intf_detect_intv(const char * ifname, uint16_t scs_intf_detect_intv)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_intf_detect_intv_rpcdata __req;
	struct qcsapi_wifi_set_scs_intf_detect_intv_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_intf_detect_intv = (uint16_t)scs_intf_detect_intv;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_INTF_DETECT_INTV_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_intf_detect_intv_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_intf_detect_intv_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_intf_detect_intv call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_intf_detect_intv_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_intf_detect_intv_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_thrshld(const char * ifname, const char * scs_param_name, uint16_t scs_threshold)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_thrshld_rpcdata __req;
	struct qcsapi_wifi_set_scs_thrshld_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcscs_param_name = {(char *)scs_param_name};
	__rpc_string *p__rpcscs_param_name = (scs_param_name) ? &__rpcscs_param_name : NULL;
	__req.scs_param_name = p__rpcscs_param_name;

	__req.scs_threshold = (uint16_t)scs_threshold;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_THRSHLD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_thrshld_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_thrshld_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_thrshld call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_thrshld_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_thrshld_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_report_only(const char * ifname, uint16_t scs_report_only)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_report_only_rpcdata __req;
	struct qcsapi_wifi_set_scs_report_only_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_report_only = (uint16_t)scs_report_only;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_REPORT_ONLY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_report_only_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_report_only_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_report_only call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_report_only_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_report_only_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_stat_report(const char * ifname, struct qcsapi_scs_ranking_rpt * scs_rpt)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_stat_report_rpcdata __req;
	struct qcsapi_wifi_get_scs_stat_report_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_rpt = (__rpc_qcsapi_scs_ranking_rpt*)scs_rpt;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_STAT_REPORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_stat_report_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_stat_report_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_stat_report call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_stat_report_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.scs_rpt && scs_rpt)
			memcpy(scs_rpt, __resp.scs_rpt, sizeof(*scs_rpt));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_stat_report_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_score_report(const char * ifname, struct qcsapi_scs_score_rpt * scs_rpt)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_score_report_rpcdata __req;
	struct qcsapi_wifi_get_scs_score_report_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_rpt = (__rpc_qcsapi_scs_score_rpt*)scs_rpt;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_SCORE_REPORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_score_report_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_score_report_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_score_report call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_score_report_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.scs_rpt && scs_rpt)
			memcpy(scs_rpt, __resp.scs_rpt, sizeof(*scs_rpt));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_score_report_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_currchan_report(const char * ifname, struct qcsapi_scs_currchan_rpt * scs_currchan_rpt)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_currchan_report_rpcdata __req;
	struct qcsapi_wifi_get_scs_currchan_report_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scs_currchan_rpt = (__rpc_qcsapi_scs_currchan_rpt*)scs_currchan_rpt;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_CURRCHAN_REPORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_currchan_report_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_currchan_report_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_currchan_report call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_currchan_report_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.scs_currchan_rpt && scs_currchan_rpt)
			memcpy(scs_currchan_rpt, __resp.scs_currchan_rpt, sizeof(*scs_currchan_rpt));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_currchan_report_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_stats(const char * ifname, uint16_t start)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_stats_rpcdata __req;
	struct qcsapi_wifi_set_scs_stats_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.start = (uint16_t)start;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_STATS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_stats_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_stats_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_stats call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_stats_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_stats_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_autochan_report(const char * ifname, struct qcsapi_autochan_rpt * autochan_rpt)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_autochan_report_rpcdata __req;
	struct qcsapi_wifi_get_autochan_report_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.autochan_rpt = (__rpc_qcsapi_autochan_rpt*)autochan_rpt;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AUTOCHAN_REPORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_autochan_report_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_autochan_report_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_autochan_report call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_autochan_report_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.autochan_rpt && autochan_rpt)
			memcpy(autochan_rpt, __resp.autochan_rpt, sizeof(*autochan_rpt));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_autochan_report_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_cca_intf_smth_fctr(const char * ifname, uint8_t smth_fctr_noxp, uint8_t smth_fctr_xped)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_cca_intf_smth_fctr_rpcdata __req;
	struct qcsapi_wifi_set_scs_cca_intf_smth_fctr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.smth_fctr_noxp = (uint8_t)smth_fctr_noxp;

	__req.smth_fctr_xped = (uint8_t)smth_fctr_xped;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_CCA_INTF_SMTH_FCTR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_cca_intf_smth_fctr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_cca_intf_smth_fctr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_cca_intf_smth_fctr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_cca_intf_smth_fctr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_cca_intf_smth_fctr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scs_chan_mtrc_mrgn(const char * ifname, uint8_t chan_mtrc_mrgn)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scs_chan_mtrc_mrgn_rpcdata __req;
	struct qcsapi_wifi_set_scs_chan_mtrc_mrgn_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.chan_mtrc_mrgn = (uint8_t)chan_mtrc_mrgn;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCS_CHAN_MTRC_MRGN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_chan_mtrc_mrgn_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scs_chan_mtrc_mrgn_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scs_chan_mtrc_mrgn call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_chan_mtrc_mrgn_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scs_chan_mtrc_mrgn_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_cca_intf(const char * ifname, const qcsapi_unsigned_int the_channel, int * p_cca_intf)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_cca_intf_rpcdata __req;
	struct qcsapi_wifi_get_scs_cca_intf_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.p_cca_intf = (int *)p_cca_intf;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_CCA_INTF_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_cca_intf_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_cca_intf_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_cca_intf call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_cca_intf_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_cca_intf)
			*p_cca_intf = *__resp.p_cca_intf;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_cca_intf_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_param_report(const char * ifname, struct qcsapi_scs_param_rpt * p_scs_param_rpt, uint32_t param_num)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_param_report_rpcdata __req;
	struct qcsapi_wifi_get_scs_param_report_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_scs_param_rpt = (__rpc_qcsapi_scs_param_rpt*)p_scs_param_rpt;

	__req.param_num = (uint32_t)param_num;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_PARAM_REPORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_param_report_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_param_report_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_param_report call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_param_report_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.p_scs_param_rpt && p_scs_param_rpt)
			memcpy(p_scs_param_rpt, __resp.p_scs_param_rpt, sizeof(*p_scs_param_rpt));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_param_report_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scs_dfs_reentry_request(const char * ifname, qcsapi_unsigned_int * p_scs_dfs_reentry_request)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scs_dfs_reentry_request_rpcdata __req;
	struct qcsapi_wifi_get_scs_dfs_reentry_request_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_scs_dfs_reentry_request = (unsigned int *)p_scs_dfs_reentry_request;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCS_DFS_REENTRY_REQUEST_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_dfs_reentry_request_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scs_dfs_reentry_request_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scs_dfs_reentry_request call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_dfs_reentry_request_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_scs_dfs_reentry_request)
			*p_scs_dfs_reentry_request = *__resp.p_scs_dfs_reentry_request;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scs_dfs_reentry_request_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_start_ocac(const char * ifname, uint16_t channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_start_ocac_rpcdata __req;
	struct qcsapi_wifi_start_ocac_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.channel = (uint16_t)channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_START_OCAC_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_start_ocac_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_start_ocac_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_start_ocac call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_ocac_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_ocac_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_stop_ocac(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_stop_ocac_rpcdata __req;
	struct qcsapi_wifi_stop_ocac_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_STOP_OCAC_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_stop_ocac_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_stop_ocac_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_stop_ocac call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_stop_ocac_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_stop_ocac_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_ocac_status(const char * ifname, qcsapi_unsigned_int * status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_ocac_status_rpcdata __req;
	struct qcsapi_wifi_get_ocac_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.status = (unsigned int *)status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_OCAC_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_ocac_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_ocac_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_ocac_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ocac_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (status)
			*status = *__resp.status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ocac_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ocac_dwell_time(const char * ifname, uint16_t dwell_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ocac_dwell_time_rpcdata __req;
	struct qcsapi_wifi_set_ocac_dwell_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.dwell_time = (uint16_t)dwell_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OCAC_DWELL_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_dwell_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_dwell_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ocac_dwell_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_dwell_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_dwell_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ocac_duration(const char * ifname, uint16_t duration)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ocac_duration_rpcdata __req;
	struct qcsapi_wifi_set_ocac_duration_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.duration = (uint16_t)duration;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OCAC_DURATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_duration_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_duration_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ocac_duration call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_duration_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_duration_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ocac_cac_time(const char * ifname, uint16_t cac_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ocac_cac_time_rpcdata __req;
	struct qcsapi_wifi_set_ocac_cac_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.cac_time = (uint16_t)cac_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OCAC_CAC_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_cac_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_cac_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ocac_cac_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_cac_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_cac_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ocac_report_only(const char * ifname, uint16_t enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ocac_report_only_rpcdata __req;
	struct qcsapi_wifi_set_ocac_report_only_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (uint16_t)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OCAC_REPORT_ONLY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_report_only_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_report_only_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ocac_report_only call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_report_only_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_report_only_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ocac_thrshld(const char * ifname, const char * param_name, uint16_t threshold)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ocac_thrshld_rpcdata __req;
	struct qcsapi_wifi_set_ocac_thrshld_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__req.threshold = (uint16_t)threshold;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OCAC_THRSHLD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_thrshld_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ocac_thrshld_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ocac_thrshld call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_thrshld_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ocac_thrshld_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_start_dfs_s_radio(const char * ifname, uint16_t channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_start_dfs_s_radio_rpcdata __req;
	struct qcsapi_wifi_start_dfs_s_radio_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.channel = (uint16_t)channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_START_DFS_S_RADIO_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_start_dfs_s_radio_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_start_dfs_s_radio_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_start_dfs_s_radio call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_dfs_s_radio_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_dfs_s_radio_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_stop_dfs_s_radio(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_stop_dfs_s_radio_rpcdata __req;
	struct qcsapi_wifi_stop_dfs_s_radio_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_STOP_DFS_S_RADIO_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_stop_dfs_s_radio_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_stop_dfs_s_radio_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_stop_dfs_s_radio call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_stop_dfs_s_radio_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_stop_dfs_s_radio_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dfs_s_radio_status(const char * ifname, qcsapi_unsigned_int * status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dfs_s_radio_status_rpcdata __req;
	struct qcsapi_wifi_get_dfs_s_radio_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.status = (unsigned int *)status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DFS_S_RADIO_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dfs_s_radio_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (status)
			*status = *__resp.status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dfs_s_radio_availability(const char * ifname, qcsapi_unsigned_int * available)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dfs_s_radio_availability_rpcdata __req;
	struct qcsapi_wifi_get_dfs_s_radio_availability_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.available = (unsigned int *)available;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DFS_S_RADIO_AVAILABILITY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_availability_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_availability_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dfs_s_radio_availability call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_availability_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (available)
			*available = *__resp.available;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dfs_s_radio_availability_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_dwell_time(const char * ifname, uint16_t dwell_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_dwell_time_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_dwell_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.dwell_time = (uint16_t)dwell_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_DWELL_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_dwell_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_dwell_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_dwell_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_dwell_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_dwell_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_duration(const char * ifname, uint16_t duration)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_duration_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_duration_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.duration = (uint16_t)duration;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_DURATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_duration_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_duration_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_duration call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_duration_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_duration_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_wea_duration(const char * ifname, uint32_t duration)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_wea_duration_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_wea_duration_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.duration = (uint32_t)duration;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_WEA_DURATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_duration_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_duration_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_wea_duration call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_duration_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_duration_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_cac_time(const char * ifname, uint16_t cac_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_cac_time_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_cac_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.cac_time = (uint16_t)cac_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_CAC_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_cac_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_cac_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_cac_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_cac_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_cac_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_wea_cac_time(const char * ifname, uint32_t cac_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_wea_cac_time_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_wea_cac_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.cac_time = (uint32_t)cac_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_WEA_CAC_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_cac_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_cac_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_wea_cac_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_cac_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_wea_cac_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_report_only(const char * ifname, uint16_t enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_report_only_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_report_only_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (uint16_t)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_REPORT_ONLY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_report_only_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_report_only_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_report_only call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_report_only_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_report_only_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dfs_s_radio_thrshld(const char * ifname, const char * param_name, uint16_t threshold)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dfs_s_radio_thrshld_rpcdata __req;
	struct qcsapi_wifi_set_dfs_s_radio_thrshld_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__req.threshold = (uint16_t)threshold;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_S_RADIO_THRSHLD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_thrshld_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_thrshld_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dfs_s_radio_thrshld call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_thrshld_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dfs_s_radio_thrshld_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_init()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_init_rpcdata __req;
	struct qcsapi_init_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INIT_REMOTE,
				(xdrproc_t)xdr_qcsapi_init_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_init_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_init call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_init_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_init_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_console_disconnect()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_console_disconnect_rpcdata __req;
	struct qcsapi_console_disconnect_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CONSOLE_DISCONNECT_REMOTE,
				(xdrproc_t)xdr_qcsapi_console_disconnect_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_console_disconnect_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_console_disconnect call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_console_disconnect_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_console_disconnect_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_startprod()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_startprod_rpcdata __req;
	struct qcsapi_wifi_startprod_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_STARTPROD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_startprod_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_startprod_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_startprod call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_startprod_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_startprod_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_is_startprod_done(int * p_status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_is_startprod_done_rpcdata __req;
	struct qcsapi_is_startprod_done_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_status = (int *)p_status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_IS_STARTPROD_DONE_REMOTE,
				(xdrproc_t)xdr_qcsapi_is_startprod_done_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_is_startprod_done_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_is_startprod_done call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_is_startprod_done_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_status)
			*p_status = *__resp.p_status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_is_startprod_done_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_system_get_time_since_start(qcsapi_unsigned_int * p_elapsed_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_system_get_time_since_start_rpcdata __req;
	struct qcsapi_system_get_time_since_start_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_elapsed_time = (unsigned int *)p_elapsed_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SYSTEM_GET_TIME_SINCE_START_REMOTE,
				(xdrproc_t)xdr_qcsapi_system_get_time_since_start_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_system_get_time_since_start_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_system_get_time_since_start call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_system_get_time_since_start_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_elapsed_time)
			*p_elapsed_time = *__resp.p_elapsed_time;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_system_get_time_since_start_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_system_status(qcsapi_unsigned_int * p_status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_system_status_rpcdata __req;
	struct qcsapi_get_system_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_status = (unsigned int *)p_status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_SYSTEM_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_system_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_system_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_system_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_system_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_status)
			*p_status = *__resp.p_status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_system_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_random_seed(struct qcsapi_data_512bytes * random_buf)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_random_seed_rpcdata __req;
	struct qcsapi_get_random_seed_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.random_buf = (__rpc_qcsapi_data_512bytes*)random_buf;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_RANDOM_SEED_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_random_seed_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_random_seed_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_random_seed call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_random_seed_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.random_buf && random_buf)
			memcpy(random_buf, __resp.random_buf, sizeof(*random_buf));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_random_seed_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_random_seed(const struct qcsapi_data_512bytes * random_buf, const qcsapi_unsigned_int entropy)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_random_seed_rpcdata __req;
	struct qcsapi_set_random_seed_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.random_buf = (__rpc_qcsapi_data_512bytes*)random_buf;

	__req.entropy = (unsigned int)entropy;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_RANDOM_SEED_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_random_seed_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_random_seed_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_random_seed call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_random_seed_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_random_seed_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_carrier_id(qcsapi_unsigned_int * p_carrier_id)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_carrier_id_rpcdata __req;
	struct qcsapi_get_carrier_id_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_carrier_id = (unsigned int *)p_carrier_id;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_CARRIER_ID_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_carrier_id_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_carrier_id_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_carrier_id call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_carrier_id_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_carrier_id)
			*p_carrier_id = *__resp.p_carrier_id;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_carrier_id_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_carrier_id(uint32_t carrier_id, uint32_t update_uboot)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_carrier_id_rpcdata __req;
	struct qcsapi_set_carrier_id_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.carrier_id = (uint32_t)carrier_id;

	__req.update_uboot = (uint32_t)update_uboot;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_CARRIER_ID_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_carrier_id_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_carrier_id_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_carrier_id call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_carrier_id_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_carrier_id_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_spinor_jedecid(const char * ifname, unsigned int * p_jedecid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_spinor_jedecid_rpcdata __req;
	struct qcsapi_wifi_get_spinor_jedecid_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_jedecid = (unsigned int *)p_jedecid;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SPINOR_JEDECID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_spinor_jedecid_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_spinor_jedecid_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_spinor_jedecid call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_spinor_jedecid_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_jedecid)
			*p_jedecid = *__resp.p_jedecid;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_spinor_jedecid_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bb_param(const char * ifname, unsigned int * p_jedecid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bb_param_rpcdata __req;
	struct qcsapi_wifi_get_bb_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_jedecid = (unsigned int *)p_jedecid;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BB_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bb_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bb_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bb_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bb_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_jedecid)
			*p_jedecid = *__resp.p_jedecid;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bb_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bb_param(const char * ifname, const qcsapi_unsigned_int p_jedecid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bb_param_rpcdata __req;
	struct qcsapi_wifi_set_bb_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_jedecid = (unsigned int)p_jedecid;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BB_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bb_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bb_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bb_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bb_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bb_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_optim_stats(const char * ifname, const qcsapi_unsigned_int p_jedecid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_optim_stats_rpcdata __req;
	struct qcsapi_wifi_set_optim_stats_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_jedecid = (unsigned int)p_jedecid;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OPTIM_STATS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_optim_stats_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_optim_stats_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_optim_stats call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_optim_stats_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_optim_stats_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_sys_time(const uint32_t timestamp)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_sys_time_rpcdata __req;
	struct qcsapi_wifi_set_sys_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.timestamp = (uint32_t)timestamp;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SYS_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_sys_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_sys_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_sys_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_sys_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_sys_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_sys_time(uint32_t * timestamp)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_sys_time_rpcdata __req;
	struct qcsapi_wifi_get_sys_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.timestamp = (uint32_t *)timestamp;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SYS_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_sys_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_sys_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_sys_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_sys_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (timestamp)
			*timestamp = *__resp.timestamp;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_sys_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_soc_mac_addr(const char * ifname, const qcsapi_mac_addr soc_mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_soc_mac_addr_rpcdata __req;
	struct qcsapi_set_soc_mac_addr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcsoc_mac_addr;
	if (soc_mac_addr) {
		memcpy(__rpcsoc_mac_addr.data, soc_mac_addr, sizeof(__rpcsoc_mac_addr));
		__req.soc_mac_addr = &__rpcsoc_mac_addr;
	} else {
		__req.soc_mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_SOC_MAC_ADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_soc_mac_addr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_soc_mac_addr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_soc_mac_addr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_soc_mac_addr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_soc_mac_addr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_custom_value(const char * custom_key, string_128 custom_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_custom_value_rpcdata __req;
	struct qcsapi_get_custom_value_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpccustom_key = {(char *)custom_key};
	__rpc_string *p__rpccustom_key = (custom_key) ? &__rpccustom_key : NULL;
	__req.custom_key = p__rpccustom_key;

	__rpc_string __rpccustom_value = {(char *)custom_value};
	__rpc_string *p__rpccustom_value = (custom_value) ? &__rpccustom_value : NULL;
	__req.custom_value = p__rpccustom_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_CUSTOM_VALUE_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_custom_value_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_custom_value_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_custom_value call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_custom_value_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (custom_value && __resp.custom_value)
			strcpy(custom_value, __resp.custom_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_custom_value_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_config_get_parameter(const char * ifname, const char * param_name, char * param_value, const size_t max_param_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_config_get_parameter_rpcdata __req;
	struct qcsapi_config_get_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	__req.max_param_len = (uint32_t)max_param_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CONFIG_GET_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_config_get_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_config_get_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_config_get_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_config_get_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (param_value && __resp.param_value)
			strcpy(param_value, __resp.param_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_config_get_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_config_update_parameter(const char * ifname, const char * param_name, const char * param_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_config_update_parameter_rpcdata __req;
	struct qcsapi_config_update_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CONFIG_UPDATE_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_config_update_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_config_update_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_config_update_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_config_update_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_config_update_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_config_get_ssid_parameter(const char * ifname, const char * param_name, char * param_value, const size_t max_param_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_config_get_ssid_parameter_rpcdata __req;
	struct qcsapi_config_get_ssid_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	__req.max_param_len = (uint32_t)max_param_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CONFIG_GET_SSID_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_config_get_ssid_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_config_get_ssid_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_config_get_ssid_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_config_get_ssid_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (param_value && __resp.param_value)
			strcpy(param_value, __resp.param_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_config_get_ssid_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_config_update_ssid_parameter(const char * ifname, const char * param_name, const char * param_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_config_update_ssid_parameter_rpcdata __req;
	struct qcsapi_config_update_ssid_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam_name = {(char *)param_name};
	__rpc_string *p__rpcparam_name = (param_name) ? &__rpcparam_name : NULL;
	__req.param_name = p__rpcparam_name;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CONFIG_UPDATE_SSID_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_config_update_ssid_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_config_update_ssid_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_config_update_ssid_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_config_update_ssid_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_config_update_ssid_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_file_path_get_config(const qcsapi_file_path_config e_file_path, char * file_path, qcsapi_unsigned_int path_size)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_file_path_get_config_rpcdata __req;
	struct qcsapi_file_path_get_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.e_file_path = (int)e_file_path;

	__rpc_string __rpcfile_path = {(char *)file_path};
	__rpc_string *p__rpcfile_path = (file_path) ? &__rpcfile_path : NULL;
	__req.file_path = p__rpcfile_path;

	__req.path_size = (unsigned int)path_size;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_FILE_PATH_GET_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_file_path_get_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_file_path_get_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_file_path_get_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_file_path_get_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (file_path && __resp.file_path)
			strcpy(file_path, __resp.file_path->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_file_path_get_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_file_path_set_config(const qcsapi_file_path_config e_file_path, const char * new_path)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_file_path_set_config_rpcdata __req;
	struct qcsapi_file_path_set_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.e_file_path = (int)e_file_path;

	__rpc_string __rpcnew_path = {(char *)new_path};
	__rpc_string *p__rpcnew_path = (new_path) ? &__rpcnew_path : NULL;
	__req.new_path = p__rpcnew_path;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_FILE_PATH_SET_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_file_path_set_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_file_path_set_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_file_path_set_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_file_path_set_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_file_path_set_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_restore_default_config(int flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_restore_default_config_rpcdata __req;
	struct qcsapi_restore_default_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.flag = (int)flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_RESTORE_DEFAULT_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_restore_default_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_restore_default_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_restore_default_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_restore_default_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_restore_default_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_store_ipaddr(qcsapi_unsigned_int ipaddr, qcsapi_unsigned_int netmask)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_store_ipaddr_rpcdata __req;
	struct qcsapi_store_ipaddr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.ipaddr = (unsigned int)ipaddr;

	__req.netmask = (unsigned int)netmask;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_STORE_IPADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_store_ipaddr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_store_ipaddr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_store_ipaddr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_store_ipaddr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_store_ipaddr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_enable(const char * ifname, const int enable_flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_enable_rpcdata __req;
	struct qcsapi_interface_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable_flag = (int)enable_flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_get_status(const char * ifname, char * interface_status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_get_status_rpcdata __req;
	struct qcsapi_interface_get_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcinterface_status = {(char *)interface_status};
	__rpc_string *p__rpcinterface_status = (interface_status) ? &__rpcinterface_status : NULL;
	__req.interface_status = p__rpcinterface_status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_GET_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_get_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_get_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_get_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_get_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (interface_status && __resp.interface_status)
			strcpy(interface_status, __resp.interface_status->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_get_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_set_ip4(const char * ifname, const char * if_param, uint32_t if_param_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_set_ip4_rpcdata __req;
	struct qcsapi_interface_set_ip4_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcif_param = {(char *)if_param};
	__rpc_string *p__rpcif_param = (if_param) ? &__rpcif_param : NULL;
	__req.if_param = p__rpcif_param;

	__req.if_param_val = (uint32_t)if_param_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_SET_IP4_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_set_ip4_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_set_ip4_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_set_ip4 call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_set_ip4_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_set_ip4_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_get_ip4(const char * ifname, const char * if_param, string_64 if_param_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_get_ip4_rpcdata __req;
	struct qcsapi_interface_get_ip4_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcif_param = {(char *)if_param};
	__rpc_string *p__rpcif_param = (if_param) ? &__rpcif_param : NULL;
	__req.if_param = p__rpcif_param;

	__rpc_string __rpcif_param_val = {(char *)if_param_val};
	__rpc_string *p__rpcif_param_val = (if_param_val) ? &__rpcif_param_val : NULL;
	__req.if_param_val = p__rpcif_param_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_GET_IP4_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_get_ip4_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_get_ip4_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_get_ip4 call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_get_ip4_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (if_param_val && __resp.if_param_val)
			strcpy(if_param_val, __resp.if_param_val->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_get_ip4_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_get_counter(const char * ifname, qcsapi_counter_type qcsapi_counter, qcsapi_unsigned_int * p_counter_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_get_counter_rpcdata __req;
	struct qcsapi_interface_get_counter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.qcsapi_counter = (int)qcsapi_counter;

	__req.p_counter_value = (unsigned int *)p_counter_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_GET_COUNTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_get_counter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_get_counter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_get_counter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_get_counter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_counter_value)
			*p_counter_value = *__resp.p_counter_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_get_counter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_get_counter64(const char * ifname, qcsapi_counter_type qcsapi_counter, uint64_t * p_counter_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_get_counter64_rpcdata __req;
	struct qcsapi_interface_get_counter64_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.qcsapi_counter = (int)qcsapi_counter;

	__req.p_counter_value = (uint64_t *)p_counter_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_GET_COUNTER64_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_get_counter64_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_get_counter64_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_get_counter64 call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_get_counter64_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_counter_value)
			*p_counter_value = *__resp.p_counter_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_get_counter64_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_get_mac_addr(const char * ifname, qcsapi_mac_addr current_mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_get_mac_addr_rpcdata __req;
	struct qcsapi_interface_get_mac_addr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpccurrent_mac_addr;
	if (current_mac_addr) {
		memcpy(__rpccurrent_mac_addr.data, current_mac_addr, sizeof(__rpccurrent_mac_addr));
		__req.current_mac_addr = &__rpccurrent_mac_addr;
	} else {
		__req.current_mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_GET_MAC_ADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_get_mac_addr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_get_mac_addr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_get_mac_addr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_get_mac_addr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_mac_addr && __resp.current_mac_addr)
			memcpy(current_mac_addr, __resp.current_mac_addr->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_get_mac_addr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_set_mac_addr(const char * ifname, const qcsapi_mac_addr interface_mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_set_mac_addr_rpcdata __req;
	struct qcsapi_interface_set_mac_addr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcinterface_mac_addr;
	if (interface_mac_addr) {
		memcpy(__rpcinterface_mac_addr.data, interface_mac_addr, sizeof(__rpcinterface_mac_addr));
		__req.interface_mac_addr = &__rpcinterface_mac_addr;
	} else {
		__req.interface_mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_SET_MAC_ADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_set_mac_addr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_set_mac_addr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_set_mac_addr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_set_mac_addr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_set_mac_addr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_pm_get_counter(const char * ifname, qcsapi_counter_type qcsapi_counter, const char * pm_interval, qcsapi_unsigned_int * p_counter_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_pm_get_counter_rpcdata __req;
	struct qcsapi_pm_get_counter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.qcsapi_counter = (int)qcsapi_counter;

	__rpc_string __rpcpm_interval = {(char *)pm_interval};
	__rpc_string *p__rpcpm_interval = (pm_interval) ? &__rpcpm_interval : NULL;
	__req.pm_interval = p__rpcpm_interval;

	__req.p_counter_value = (unsigned int *)p_counter_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_PM_GET_COUNTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_pm_get_counter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_pm_get_counter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_pm_get_counter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_pm_get_counter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_counter_value)
			*p_counter_value = *__resp.p_counter_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_pm_get_counter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_aspm_l1(int enable, int latency)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_aspm_l1_rpcdata __req;
	struct qcsapi_set_aspm_l1_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.enable = (int)enable;

	__req.latency = (int)latency;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_ASPM_L1_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_aspm_l1_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_aspm_l1_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_aspm_l1 call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_aspm_l1_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_aspm_l1_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_l1(int enter)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_l1_rpcdata __req;
	struct qcsapi_set_l1_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.enter = (int)enter;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_L1_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_l1_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_l1_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_l1 call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_l1_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_l1_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_pm_get_elapsed_time(const char * pm_interval, qcsapi_unsigned_int * p_elapsed_time)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_pm_get_elapsed_time_rpcdata __req;
	struct qcsapi_pm_get_elapsed_time_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcpm_interval = {(char *)pm_interval};
	__rpc_string *p__rpcpm_interval = (pm_interval) ? &__rpcpm_interval : NULL;
	__req.pm_interval = p__rpcpm_interval;

	__req.p_elapsed_time = (unsigned int *)p_elapsed_time;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_PM_GET_ELAPSED_TIME_REMOTE,
				(xdrproc_t)xdr_qcsapi_pm_get_elapsed_time_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_pm_get_elapsed_time_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_pm_get_elapsed_time call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_pm_get_elapsed_time_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_elapsed_time)
			*p_elapsed_time = *__resp.p_elapsed_time;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_pm_get_elapsed_time_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_eth_phy_power_control(int on_off, const char * interface)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_eth_phy_power_control_rpcdata __req;
	struct qcsapi_eth_phy_power_control_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.on_off = (int)on_off;

	__rpc_string __rpcinterface = {(char *)interface};
	__rpc_string *p__rpcinterface = (interface) ? &__rpcinterface : NULL;
	__req.interface = p__rpcinterface;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_ETH_PHY_POWER_CONTROL_REMOTE,
				(xdrproc_t)xdr_qcsapi_eth_phy_power_control_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_eth_phy_power_control_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_eth_phy_power_control call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_eth_phy_power_control_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_eth_phy_power_control_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_emac_switch(char * buf)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_emac_switch_rpcdata __req;
	struct qcsapi_get_emac_switch_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcbuf = {(char *)buf};
	__rpc_string *p__rpcbuf = (buf) ? &__rpcbuf : NULL;
	__req.buf = p__rpcbuf;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_EMAC_SWITCH_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_emac_switch_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_emac_switch_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_emac_switch call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_emac_switch_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (buf && __resp.buf)
			strcpy(buf, __resp.buf->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_emac_switch_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_emac_switch(qcsapi_emac_switch value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_emac_switch_rpcdata __req;
	struct qcsapi_set_emac_switch_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.value = (int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_EMAC_SWITCH_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_emac_switch_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_emac_switch_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_emac_switch call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_emac_switch_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_emac_switch_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_eth_dscp_map(qcsapi_eth_dscp_oper oper, const char * eth_type, const char * level, const char * value, char * buf, const unsigned int size)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_eth_dscp_map_rpcdata __req;
	struct qcsapi_eth_dscp_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.oper = (int)oper;

	__rpc_string __rpceth_type = {(char *)eth_type};
	__rpc_string *p__rpceth_type = (eth_type) ? &__rpceth_type : NULL;
	__req.eth_type = p__rpceth_type;

	__rpc_string __rpclevel = {(char *)level};
	__rpc_string *p__rpclevel = (level) ? &__rpclevel : NULL;
	__req.level = p__rpclevel;

	__rpc_string __rpcvalue = {(char *)value};
	__rpc_string *p__rpcvalue = (value) ? &__rpcvalue : NULL;
	__req.value = p__rpcvalue;

	__rpc_string __rpcbuf = {(char *)buf};
	__rpc_string *p__rpcbuf = (buf) ? &__rpcbuf : NULL;
	__req.buf = p__rpcbuf;

	__req.size = (unsigned int)size;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_ETH_DSCP_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_eth_dscp_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_eth_dscp_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_eth_dscp_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_eth_dscp_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (buf && __resp.buf)
			strcpy(buf, __resp.buf->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_eth_dscp_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_eth_info(const char * ifname, const qcsapi_eth_info_type eth_info_type)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_eth_info_rpcdata __req;
	struct qcsapi_get_eth_info_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.eth_info_type = (int)eth_info_type;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_ETH_INFO_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_eth_info_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_eth_info_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_eth_info call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_eth_info_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_eth_info_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mode(const char * ifname, qcsapi_wifi_mode * p_wifi_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mode_rpcdata __req;
	struct qcsapi_wifi_get_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_wifi_mode = (int *)p_wifi_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_wifi_mode)
			*p_wifi_mode = *__resp.p_wifi_mode;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_mode(const char * ifname, const qcsapi_wifi_mode new_wifi_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_mode_rpcdata __req;
	struct qcsapi_wifi_set_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_wifi_mode = (int)new_wifi_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_phy_mode(const char * ifname, char * p_wifi_phy_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_phy_mode_rpcdata __req;
	struct qcsapi_wifi_get_phy_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_wifi_phy_mode = {(char *)p_wifi_phy_mode};
	__rpc_string *p__rpcp_wifi_phy_mode = (p_wifi_phy_mode) ? &__rpcp_wifi_phy_mode : NULL;
	__req.p_wifi_phy_mode = p__rpcp_wifi_phy_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PHY_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_phy_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_phy_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_phy_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_phy_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_wifi_phy_mode && __resp.p_wifi_phy_mode)
			strcpy(p_wifi_phy_mode, __resp.p_wifi_phy_mode->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_phy_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_phy_mode(const char * ifname, const char * new_phy_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_phy_mode_rpcdata __req;
	struct qcsapi_wifi_set_phy_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcnew_phy_mode = {(char *)new_phy_mode};
	__rpc_string *p__rpcnew_phy_mode = (new_phy_mode) ? &__rpcnew_phy_mode : NULL;
	__req.new_phy_mode = p__rpcnew_phy_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PHY_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_phy_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_phy_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_phy_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_phy_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_phy_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_reload_in_mode(const char * ifname, const qcsapi_wifi_mode new_wifi_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_reload_in_mode_rpcdata __req;
	struct qcsapi_wifi_reload_in_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_wifi_mode = (int)new_wifi_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_RELOAD_IN_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_reload_in_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_reload_in_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_reload_in_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_reload_in_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_reload_in_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_rfenable(const qcsapi_unsigned_int onoff)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_rfenable_rpcdata __req;
	struct qcsapi_wifi_rfenable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.onoff = (unsigned int)onoff;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_RFENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_rfenable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_rfenable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_rfenable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_rfenable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_rfenable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_rfstatus(qcsapi_unsigned_int * rfstatus)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_rfstatus_rpcdata __req;
	struct qcsapi_wifi_rfstatus_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.rfstatus = (unsigned int *)rfstatus;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_RFSTATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_rfstatus_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_rfstatus_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_rfstatus call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_rfstatus_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (rfstatus)
			*rfstatus = *__resp.rfstatus;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_rfstatus_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bw(const char * ifname, qcsapi_unsigned_int * p_bw)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bw_rpcdata __req;
	struct qcsapi_wifi_get_bw_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_bw = (unsigned int *)p_bw;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BW_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bw_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bw_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bw call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bw_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_bw)
			*p_bw = *__resp.p_bw;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bw_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bw(const char * ifname, const qcsapi_unsigned_int bw)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bw_rpcdata __req;
	struct qcsapi_wifi_set_bw_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.bw = (unsigned int)bw;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BW_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bw_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bw_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bw call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bw_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bw_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_vht(const char * ifname, const qcsapi_unsigned_int the_vht)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_vht_rpcdata __req;
	struct qcsapi_wifi_set_vht_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_vht = (unsigned int)the_vht;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_VHT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_vht_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_vht_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_vht call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_vht_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_vht_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_vht(const char * ifname, qcsapi_unsigned_int * vht)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_vht_rpcdata __req;
	struct qcsapi_wifi_get_vht_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.vht = (unsigned int *)vht;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_VHT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_vht_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_vht_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_vht call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_vht_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (vht)
			*vht = *__resp.vht;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_vht_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_channel(const char * ifname, qcsapi_unsigned_int * p_current_channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_channel_rpcdata __req;
	struct qcsapi_wifi_get_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_current_channel = (unsigned int *)p_current_channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_current_channel)
			*p_current_channel = *__resp.p_current_channel;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_channel(const char * ifname, const qcsapi_unsigned_int new_channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_channel_rpcdata __req;
	struct qcsapi_wifi_set_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_channel = (unsigned int)new_channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_chan_pri_inactive(const char * ifname, const qcsapi_unsigned_int channel, const qcsapi_unsigned_int inactive)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_chan_pri_inactive_rpcdata __req;
	struct qcsapi_wifi_set_chan_pri_inactive_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.channel = (unsigned int)channel;

	__req.inactive = (unsigned int)inactive;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_CHAN_PRI_INACTIVE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_chan_pri_inactive_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_chan_pri_inactive_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_chan_pri_inactive call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_chan_pri_inactive_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_chan_pri_inactive_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_chan_control(const char * ifname, const struct qcsapi_data_256bytes * chans, const uint32_t cnt, const uint8_t flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_chan_control_rpcdata __req;
	struct qcsapi_wifi_chan_control_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.chans = (__rpc_qcsapi_data_256bytes*)chans;

	__req.cnt = (uint32_t)cnt;

	__req.flag = (uint8_t)flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_CHAN_CONTROL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_chan_control_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_chan_control_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_chan_control call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_chan_control_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_chan_control_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_chan_disabled(const char * ifname, struct qcsapi_data_256bytes * p_chans, uint8_t * p_cnt)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_chan_disabled_rpcdata __req;
	struct qcsapi_wifi_get_chan_disabled_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_chans = (__rpc_qcsapi_data_256bytes*)p_chans;

	__req.p_cnt = (uint8_t *)p_cnt;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CHAN_DISABLED_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_chan_disabled_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_chan_disabled_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_chan_disabled call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_chan_disabled_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.p_chans && p_chans)
			memcpy(p_chans, __resp.p_chans, sizeof(*p_chans));
	}
	if (__resp.return_code >= 0) {
		if (p_cnt)
			*p_cnt = *__resp.p_cnt;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_chan_disabled_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_beacon_interval(const char * ifname, qcsapi_unsigned_int * p_current_intval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_beacon_interval_rpcdata __req;
	struct qcsapi_wifi_get_beacon_interval_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_current_intval = (unsigned int *)p_current_intval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BEACON_INTERVAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_beacon_interval_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_beacon_interval_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_beacon_interval call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_beacon_interval_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_current_intval)
			*p_current_intval = *__resp.p_current_intval;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_beacon_interval_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_beacon_interval(const char * ifname, const qcsapi_unsigned_int new_intval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_beacon_interval_rpcdata __req;
	struct qcsapi_wifi_set_beacon_interval_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_intval = (unsigned int)new_intval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BEACON_INTERVAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_beacon_interval_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_beacon_interval_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_beacon_interval call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_beacon_interval_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_beacon_interval_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dtim(const char * ifname, qcsapi_unsigned_int * p_dtim)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dtim_rpcdata __req;
	struct qcsapi_wifi_get_dtim_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_dtim = (unsigned int *)p_dtim;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DTIM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dtim_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dtim_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dtim call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dtim_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_dtim)
			*p_dtim = *__resp.p_dtim;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dtim_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dtim(const char * ifname, const qcsapi_unsigned_int new_dtim)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dtim_rpcdata __req;
	struct qcsapi_wifi_set_dtim_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_dtim = (unsigned int)new_dtim;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DTIM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dtim_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dtim_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dtim call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dtim_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dtim_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_assoc_limit(const char * ifname, qcsapi_unsigned_int * p_assoc_limit)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_assoc_limit_rpcdata __req;
	struct qcsapi_wifi_get_assoc_limit_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_assoc_limit = (unsigned int *)p_assoc_limit;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ASSOC_LIMIT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_assoc_limit_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_assoc_limit_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_assoc_limit call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_assoc_limit_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_assoc_limit)
			*p_assoc_limit = *__resp.p_assoc_limit;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_assoc_limit_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bss_assoc_limit(const char * ifname, qcsapi_unsigned_int * p_bss_lim_pri)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bss_assoc_limit_rpcdata __req;
	struct qcsapi_wifi_get_bss_assoc_limit_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_bss_lim_pri = (unsigned int *)p_bss_lim_pri;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BSS_ASSOC_LIMIT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bss_assoc_limit_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bss_assoc_limit_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bss_assoc_limit call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bss_assoc_limit_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_bss_lim_pri)
			*p_bss_lim_pri = *__resp.p_bss_lim_pri;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bss_assoc_limit_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_assoc_limit(const char * ifname, const qcsapi_unsigned_int new_assoc_limit)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_assoc_limit_rpcdata __req;
	struct qcsapi_wifi_set_assoc_limit_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_assoc_limit = (unsigned int)new_assoc_limit;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_ASSOC_LIMIT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_assoc_limit_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_assoc_limit_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_assoc_limit call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_assoc_limit_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_assoc_limit_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bss_assoc_limit(const char * ifname, const qcsapi_unsigned_int bss_lim_pri)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bss_assoc_limit_rpcdata __req;
	struct qcsapi_wifi_set_bss_assoc_limit_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.bss_lim_pri = (unsigned int)bss_lim_pri;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BSS_ASSOC_LIMIT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bss_assoc_limit_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bss_assoc_limit_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bss_assoc_limit call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bss_assoc_limit_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bss_assoc_limit_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_BSSID(const char * ifname, qcsapi_mac_addr current_BSSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_BSSID_rpcdata __req;
	struct qcsapi_wifi_get_BSSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpccurrent_BSSID;
	if (current_BSSID) {
		memcpy(__rpccurrent_BSSID.data, current_BSSID, sizeof(__rpccurrent_BSSID));
		__req.current_BSSID = &__rpccurrent_BSSID;
	} else {
		__req.current_BSSID = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BSSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_BSSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_BSSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_BSSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_BSSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_BSSID && __resp.current_BSSID)
			memcpy(current_BSSID, __resp.current_BSSID->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_BSSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_config_BSSID(const char * ifname, qcsapi_mac_addr config_BSSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_config_BSSID_rpcdata __req;
	struct qcsapi_wifi_get_config_BSSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcconfig_BSSID;
	if (config_BSSID) {
		memcpy(__rpcconfig_BSSID.data, config_BSSID, sizeof(__rpcconfig_BSSID));
		__req.config_BSSID = &__rpcconfig_BSSID;
	} else {
		__req.config_BSSID = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CONFIG_BSSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_config_BSSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_config_BSSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_config_BSSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_config_BSSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (config_BSSID && __resp.config_BSSID)
			memcpy(config_BSSID, __resp.config_BSSID->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_config_BSSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_ssid_get_bssid(const char * ifname, const qcsapi_SSID ssid_str, qcsapi_mac_addr bssid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_ssid_get_bssid_rpcdata __req;
	struct qcsapi_wifi_ssid_get_bssid_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcssid_str = {(char *)ssid_str};
	__rpc_string *p__rpcssid_str = (ssid_str) ? &__rpcssid_str : NULL;
	__req.ssid_str = p__rpcssid_str;

	struct __rpc_qcsapi_mac_addr __rpcbssid;
	if (bssid) {
		memcpy(__rpcbssid.data, bssid, sizeof(__rpcbssid));
		__req.bssid = &__rpcbssid;
	} else {
		__req.bssid = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SSID_GET_BSSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_ssid_get_bssid_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_ssid_get_bssid_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_ssid_get_bssid call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_ssid_get_bssid_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (bssid && __resp.bssid)
			memcpy(bssid, __resp.bssid->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_ssid_get_bssid_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_ssid_set_bssid(const char * ifname, const qcsapi_SSID ssid_str, const qcsapi_mac_addr bssid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_ssid_set_bssid_rpcdata __req;
	struct qcsapi_wifi_ssid_set_bssid_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcssid_str = {(char *)ssid_str};
	__rpc_string *p__rpcssid_str = (ssid_str) ? &__rpcssid_str : NULL;
	__req.ssid_str = p__rpcssid_str;

	struct __rpc_qcsapi_mac_addr __rpcbssid;
	if (bssid) {
		memcpy(__rpcbssid.data, bssid, sizeof(__rpcbssid));
		__req.bssid = &__rpcbssid;
	} else {
		__req.bssid = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SSID_SET_BSSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_ssid_set_bssid_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_ssid_set_bssid_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_ssid_set_bssid call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_ssid_set_bssid_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_ssid_set_bssid_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_SSID(const char * ifname, qcsapi_SSID SSID_str)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_SSID_rpcdata __req;
	struct qcsapi_wifi_get_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcSSID_str = {(char *)SSID_str};
	__rpc_string *p__rpcSSID_str = (SSID_str) ? &__rpcSSID_str : NULL;
	__req.SSID_str = p__rpcSSID_str;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (SSID_str && __resp.SSID_str)
			strcpy(SSID_str, __resp.SSID_str->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_SSID(const char * ifname, const qcsapi_SSID SSID_str)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_SSID_rpcdata __req;
	struct qcsapi_wifi_set_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcSSID_str = {(char *)SSID_str};
	__rpc_string *p__rpcSSID_str = (SSID_str) ? &__rpcSSID_str : NULL;
	__req.SSID_str = p__rpcSSID_str;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_IEEE_802_11_standard(const char * ifname, char * IEEE_802_11_standard)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_IEEE_802_11_standard_rpcdata __req;
	struct qcsapi_wifi_get_IEEE_802_11_standard_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcIEEE_802_11_standard = {(char *)IEEE_802_11_standard};
	__rpc_string *p__rpcIEEE_802_11_standard = (IEEE_802_11_standard) ? &__rpcIEEE_802_11_standard : NULL;
	__req.IEEE_802_11_standard = p__rpcIEEE_802_11_standard;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_IEEE_802_11_STANDARD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_IEEE_802_11_standard_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_IEEE_802_11_standard_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_IEEE_802_11_standard call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_IEEE_802_11_standard_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (IEEE_802_11_standard && __resp.IEEE_802_11_standard)
			strcpy(IEEE_802_11_standard, __resp.IEEE_802_11_standard->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_IEEE_802_11_standard_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_list_channels(const char * ifname, string_1024 list_of_channels)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_list_channels_rpcdata __req;
	struct qcsapi_wifi_get_list_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclist_of_channels = {(char *)list_of_channels};
	__rpc_string *p__rpclist_of_channels = (list_of_channels) ? &__rpclist_of_channels : NULL;
	__req.list_of_channels = p__rpclist_of_channels;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_LIST_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_list_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_of_channels && __resp.list_of_channels)
			strcpy(list_of_channels, __resp.list_of_channels->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mode_switch(uint8_t * p_wifi_mode_switch_setting)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mode_switch_rpcdata __req;
	struct qcsapi_wifi_get_mode_switch_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_wifi_mode_switch_setting = (uint8_t *)p_wifi_mode_switch_setting;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MODE_SWITCH_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mode_switch_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mode_switch_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mode_switch call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mode_switch_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_wifi_mode_switch_setting)
			*p_wifi_mode_switch_setting = *__resp.p_wifi_mode_switch_setting;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mode_switch_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_disassociate(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_disassociate_rpcdata __req;
	struct qcsapi_wifi_disassociate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DISASSOCIATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_disassociate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_disassociate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_disassociate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_disassociate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_disassociate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_disassociate_sta(const char * ifname, qcsapi_mac_addr mac)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_disassociate_sta_rpcdata __req;
	struct qcsapi_wifi_disassociate_sta_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcmac;
	if (mac) {
		memcpy(__rpcmac.data, mac, sizeof(__rpcmac));
		__req.mac = &__rpcmac;
	} else {
		__req.mac = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DISASSOCIATE_STA_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_disassociate_sta_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_disassociate_sta_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_disassociate_sta call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_disassociate_sta_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (mac && __resp.mac)
			memcpy(mac, __resp.mac->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_disassociate_sta_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_reassociate(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_reassociate_rpcdata __req;
	struct qcsapi_wifi_reassociate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_REASSOCIATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_reassociate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_reassociate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_reassociate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_reassociate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_reassociate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_disconn_info(const char * ifname, qcsapi_disconn_info * disconn_info)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_disconn_info_rpcdata __req;
	struct qcsapi_wifi_get_disconn_info_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.disconn_info = (__rpc_qcsapi_disconn_info*)disconn_info;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DISCONN_INFO_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_disconn_info_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_disconn_info_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_disconn_info call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_disconn_info_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.disconn_info && disconn_info)
			memcpy(disconn_info, __resp.disconn_info, sizeof(*disconn_info));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_disconn_info_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_disable_wps(const char * ifname, int disable_wps)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_disable_wps_rpcdata __req;
	struct qcsapi_wifi_disable_wps_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.disable_wps = (int)disable_wps;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DISABLE_WPS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_disable_wps_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_disable_wps_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_disable_wps call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_disable_wps_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_disable_wps_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_associate(const char * ifname, const qcsapi_SSID join_ssid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_associate_rpcdata __req;
	struct qcsapi_wifi_associate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcjoin_ssid = {(char *)join_ssid};
	__rpc_string *p__rpcjoin_ssid = (join_ssid) ? &__rpcjoin_ssid : NULL;
	__req.join_ssid = p__rpcjoin_ssid;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_ASSOCIATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_associate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_associate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_associate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_associate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_associate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_start_cca(const char * ifname, int channel, int duration)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_start_cca_rpcdata __req;
	struct qcsapi_wifi_start_cca_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.channel = (int)channel;

	__req.duration = (int)duration;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_START_CCA_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_start_cca_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_start_cca_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_start_cca call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_cca_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_cca_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_noise(const char * ifname, int * p_noise)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_noise_rpcdata __req;
	struct qcsapi_wifi_get_noise_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_noise = (int *)p_noise;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_NOISE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_noise_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_noise_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_noise call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_noise_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_noise)
			*p_noise = *__resp.p_noise;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_noise_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rssi_by_chain(const char * ifname, int rf_chain, int * p_rssi)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rssi_by_chain_rpcdata __req;
	struct qcsapi_wifi_get_rssi_by_chain_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.rf_chain = (int)rf_chain;

	__req.p_rssi = (int *)p_rssi;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RSSI_BY_CHAIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rssi_by_chain_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rssi_by_chain_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rssi_by_chain call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rssi_by_chain_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_rssi)
			*p_rssi = *__resp.p_rssi;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rssi_by_chain_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_avg_snr(const char * ifname, int * p_snr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_avg_snr_rpcdata __req;
	struct qcsapi_wifi_get_avg_snr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_snr = (int *)p_snr;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AVG_SNR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_avg_snr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_avg_snr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_avg_snr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_avg_snr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_snr)
			*p_snr = *__resp.p_snr;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_avg_snr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_primary_interface(char * ifname, size_t maxlen)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_primary_interface_rpcdata __req;
	struct qcsapi_get_primary_interface_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.maxlen = (uint32_t)maxlen;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_PRIMARY_INTERFACE_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_primary_interface_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_primary_interface_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_primary_interface call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_primary_interface_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ifname && __resp.ifname)
			strcpy(ifname, __resp.ifname->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_primary_interface_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_interface_by_index(unsigned int if_index, char * ifname, size_t maxlen)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_interface_by_index_rpcdata __req;
	struct qcsapi_get_interface_by_index_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.if_index = (unsigned int)if_index;

	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.maxlen = (uint32_t)maxlen;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_INTERFACE_BY_INDEX_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_interface_by_index_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_interface_by_index_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_interface_by_index call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_interface_by_index_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ifname && __resp.ifname)
			strcpy(ifname, __resp.ifname->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_interface_by_index_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_wifi_macaddr(const qcsapi_mac_addr new_mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_wifi_macaddr_rpcdata __req;
	struct qcsapi_wifi_set_wifi_macaddr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	struct __rpc_qcsapi_mac_addr __rpcnew_mac_addr;
	if (new_mac_addr) {
		memcpy(__rpcnew_mac_addr.data, new_mac_addr, sizeof(__rpcnew_mac_addr));
		__req.new_mac_addr = &__rpcnew_mac_addr;
	} else {
		__req.new_mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WIFI_MACADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_wifi_macaddr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_wifi_macaddr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_wifi_macaddr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_wifi_macaddr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_wifi_macaddr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_interface_get_BSSID(const char * ifname, qcsapi_mac_addr current_BSSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_interface_get_BSSID_rpcdata __req;
	struct qcsapi_interface_get_BSSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpccurrent_BSSID;
	if (current_BSSID) {
		memcpy(__rpccurrent_BSSID.data, current_BSSID, sizeof(__rpccurrent_BSSID));
		__req.current_BSSID = &__rpccurrent_BSSID;
	} else {
		__req.current_BSSID = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_INTERFACE_GET_BSSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_interface_get_BSSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_interface_get_BSSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_interface_get_BSSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_interface_get_BSSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_BSSID && __resp.current_BSSID)
			memcpy(current_BSSID, __resp.current_BSSID->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_interface_get_BSSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rates(const char * ifname, qcsapi_rate_type rate_type, string_1024 supported_rates)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rates_rpcdata __req;
	struct qcsapi_wifi_get_rates_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.rate_type = (int)rate_type;

	__rpc_string __rpcsupported_rates = {(char *)supported_rates};
	__rpc_string *p__rpcsupported_rates = (supported_rates) ? &__rpcsupported_rates : NULL;
	__req.supported_rates = p__rpcsupported_rates;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RATES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rates_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rates_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rates call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rates_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (supported_rates && __resp.supported_rates)
			strcpy(supported_rates, __resp.supported_rates->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rates_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_rates(const char * ifname, qcsapi_rate_type rate_type, const string_256 current_rates, int num_rates)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_rates_rpcdata __req;
	struct qcsapi_wifi_set_rates_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.rate_type = (int)rate_type;

	__rpc_string __rpccurrent_rates = {(char *)current_rates};
	__rpc_string *p__rpccurrent_rates = (current_rates) ? &__rpccurrent_rates : NULL;
	__req.current_rates = p__rpccurrent_rates;

	__req.num_rates = (int)num_rates;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_RATES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_rates_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_rates_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_rates call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_rates_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_rates_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_max_bitrate(const char * ifname, char * max_bitrate, const int max_str_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_max_bitrate_rpcdata __req;
	struct qcsapi_get_max_bitrate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcmax_bitrate = {(char *)max_bitrate};
	__rpc_string *p__rpcmax_bitrate = (max_bitrate) ? &__rpcmax_bitrate : NULL;
	__req.max_bitrate = p__rpcmax_bitrate;

	__req.max_str_len = (int)max_str_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_MAX_BITRATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_max_bitrate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_max_bitrate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_max_bitrate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_max_bitrate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (max_bitrate && __resp.max_bitrate)
			strcpy(max_bitrate, __resp.max_bitrate->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_max_bitrate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_max_bitrate(const char * ifname, const char * max_bitrate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_max_bitrate_rpcdata __req;
	struct qcsapi_set_max_bitrate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcmax_bitrate = {(char *)max_bitrate};
	__rpc_string *p__rpcmax_bitrate = (max_bitrate) ? &__rpcmax_bitrate : NULL;
	__req.max_bitrate = p__rpcmax_bitrate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_MAX_BITRATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_max_bitrate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_max_bitrate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_max_bitrate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_max_bitrate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_max_bitrate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_qos_get_param(const char * ifname, int the_queue, int the_param, int ap_bss_flag, int * p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_qos_get_param_rpcdata __req;
	struct qcsapi_wifi_qos_get_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_queue = (int)the_queue;

	__req.the_param = (int)the_param;

	__req.ap_bss_flag = (int)ap_bss_flag;

	__req.p_value = (int *)p_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_QOS_GET_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_qos_get_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_qos_get_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_qos_get_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_qos_get_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_qos_get_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_qos_set_param(const char * ifname, int the_queue, int the_param, int ap_bss_flag, int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_qos_set_param_rpcdata __req;
	struct qcsapi_wifi_qos_set_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_queue = (int)the_queue;

	__req.the_param = (int)the_param;

	__req.ap_bss_flag = (int)ap_bss_flag;

	__req.value = (int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_QOS_SET_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_qos_set_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_qos_set_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_qos_set_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_qos_set_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_qos_set_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_wmm_ac_map(const char * ifname, string_64 mapping_table)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_wmm_ac_map_rpcdata __req;
	struct qcsapi_wifi_get_wmm_ac_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcmapping_table = {(char *)mapping_table};
	__rpc_string *p__rpcmapping_table = (mapping_table) ? &__rpcmapping_table : NULL;
	__req.mapping_table = p__rpcmapping_table;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WMM_AC_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_wmm_ac_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_wmm_ac_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_wmm_ac_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_wmm_ac_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (mapping_table && __resp.mapping_table)
			strcpy(mapping_table, __resp.mapping_table->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_wmm_ac_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_wmm_ac_map(const char * ifname, int user_prio, int ac_index)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_wmm_ac_map_rpcdata __req;
	struct qcsapi_wifi_set_wmm_ac_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.user_prio = (int)user_prio;

	__req.ac_index = (int)ac_index;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WMM_AC_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_wmm_ac_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_wmm_ac_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_wmm_ac_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_wmm_ac_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_wmm_ac_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dscp_8021p_map(const char * ifname, string_64 mapping_table)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dscp_8021p_map_rpcdata __req;
	struct qcsapi_wifi_get_dscp_8021p_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcmapping_table = {(char *)mapping_table};
	__rpc_string *p__rpcmapping_table = (mapping_table) ? &__rpcmapping_table : NULL;
	__req.mapping_table = p__rpcmapping_table;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DSCP_8021P_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dscp_8021p_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dscp_8021p_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dscp_8021p_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dscp_8021p_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (mapping_table && __resp.mapping_table)
			strcpy(mapping_table, __resp.mapping_table->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dscp_8021p_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dscp_ac_map(const char * ifname, struct qcsapi_data_64bytes * mapping_table)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dscp_ac_map_rpcdata __req;
	struct qcsapi_wifi_get_dscp_ac_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.mapping_table = (__rpc_qcsapi_data_64bytes*)mapping_table;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DSCP_AC_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dscp_ac_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dscp_ac_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dscp_ac_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dscp_ac_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.mapping_table && mapping_table)
			memcpy(mapping_table, __resp.mapping_table, sizeof(*mapping_table));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dscp_ac_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dscp_8021p_map(const char * ifname, const char * ip_dscp_list, uint8_t dot1p_up)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dscp_8021p_map_rpcdata __req;
	struct qcsapi_wifi_set_dscp_8021p_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcip_dscp_list = {(char *)ip_dscp_list};
	__rpc_string *p__rpcip_dscp_list = (ip_dscp_list) ? &__rpcip_dscp_list : NULL;
	__req.ip_dscp_list = p__rpcip_dscp_list;

	__req.dot1p_up = (uint8_t)dot1p_up;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DSCP_8021P_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dscp_8021p_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dscp_8021p_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dscp_8021p_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dscp_8021p_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dscp_8021p_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dscp_ac_map(const char * ifname, const struct qcsapi_data_64bytes * dscp_list, uint8_t dscp_list_len, uint8_t ac)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dscp_ac_map_rpcdata __req;
	struct qcsapi_wifi_set_dscp_ac_map_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.dscp_list = (__rpc_qcsapi_data_64bytes*)dscp_list;

	__req.dscp_list_len = (uint8_t)dscp_list_len;

	__req.ac = (uint8_t)ac;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DSCP_AC_MAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dscp_ac_map_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dscp_ac_map_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dscp_ac_map call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dscp_ac_map_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dscp_ac_map_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_priority(const char * ifname, uint8_t * p_priority)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_priority_rpcdata __req;
	struct qcsapi_wifi_get_priority_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_priority = (uint8_t *)p_priority;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PRIORITY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_priority_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_priority_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_priority call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_priority_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_priority)
			*p_priority = *__resp.p_priority;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_priority_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_priority(const char * ifname, uint8_t priority)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_priority_rpcdata __req;
	struct qcsapi_wifi_set_priority_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.priority = (uint8_t)priority;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PRIORITY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_priority_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_priority_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_priority call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_priority_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_priority_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_airfair(const char * ifname, uint8_t * p_airfair)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_airfair_rpcdata __req;
	struct qcsapi_wifi_get_airfair_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_airfair = (uint8_t *)p_airfair;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AIRFAIR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_airfair_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_airfair_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_airfair call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_airfair_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_airfair)
			*p_airfair = *__resp.p_airfair;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_airfair_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_airfair(const char * ifname, uint8_t airfair)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_airfair_rpcdata __req;
	struct qcsapi_wifi_set_airfair_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.airfair = (uint8_t)airfair;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_AIRFAIR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_airfair_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_airfair_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_airfair call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_airfair_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_airfair_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_power(const char * ifname, const qcsapi_unsigned_int the_channel, int * p_tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_power_rpcdata __req;
	struct qcsapi_wifi_get_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.p_tx_power = (int *)p_tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_power)
			*p_tx_power = *__resp.p_tx_power;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_tx_power(const char * ifname, const qcsapi_unsigned_int the_channel, const int tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_tx_power_rpcdata __req;
	struct qcsapi_wifi_set_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.tx_power = (int)tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bw_power(const char * ifname, const qcsapi_unsigned_int the_channel, int * p_power_20M, int * p_power_40M, int * p_power_80M)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bw_power_rpcdata __req;
	struct qcsapi_wifi_get_bw_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.p_power_20M = (int *)p_power_20M;

	__req.p_power_40M = (int *)p_power_40M;

	__req.p_power_80M = (int *)p_power_80M;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BW_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bw_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bw_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bw_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bw_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_power_20M)
			*p_power_20M = *__resp.p_power_20M;
	}
	if (__resp.return_code >= 0) {
		if (p_power_40M)
			*p_power_40M = *__resp.p_power_40M;
	}
	if (__resp.return_code >= 0) {
		if (p_power_80M)
			*p_power_80M = *__resp.p_power_80M;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bw_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bw_power(const char * ifname, const qcsapi_unsigned_int the_channel, const int power_20M, const int power_40M, const int power_80M)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bw_power_rpcdata __req;
	struct qcsapi_wifi_set_bw_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.power_20M = (int)power_20M;

	__req.power_40M = (int)power_40M;

	__req.power_80M = (int)power_80M;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BW_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bw_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bw_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bw_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bw_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bw_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bf_power(const char * ifname, const qcsapi_unsigned_int the_channel, const qcsapi_unsigned_int number_ss, int * p_power_20M, int * p_power_40M, int * p_power_80M)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bf_power_rpcdata __req;
	struct qcsapi_wifi_get_bf_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.number_ss = (unsigned int)number_ss;

	__req.p_power_20M = (int *)p_power_20M;

	__req.p_power_40M = (int *)p_power_40M;

	__req.p_power_80M = (int *)p_power_80M;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BF_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bf_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bf_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bf_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bf_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_power_20M)
			*p_power_20M = *__resp.p_power_20M;
	}
	if (__resp.return_code >= 0) {
		if (p_power_40M)
			*p_power_40M = *__resp.p_power_40M;
	}
	if (__resp.return_code >= 0) {
		if (p_power_80M)
			*p_power_80M = *__resp.p_power_80M;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bf_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bf_power(const char * ifname, const qcsapi_unsigned_int the_channel, const qcsapi_unsigned_int number_ss, const int power_20M, const int power_40M, const int power_80M)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bf_power_rpcdata __req;
	struct qcsapi_wifi_set_bf_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.number_ss = (unsigned int)number_ss;

	__req.power_20M = (int)power_20M;

	__req.power_40M = (int)power_40M;

	__req.power_80M = (int)power_80M;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BF_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bf_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bf_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bf_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bf_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bf_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_power_ext(const char * ifname, const qcsapi_unsigned_int the_channel, const qcsapi_unsigned_int bf_on, const qcsapi_unsigned_int number_ss, int * p_power_20M, int * p_power_40M, int * p_power_80M)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_power_ext_rpcdata __req;
	struct qcsapi_wifi_get_tx_power_ext_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.bf_on = (unsigned int)bf_on;

	__req.number_ss = (unsigned int)number_ss;

	__req.p_power_20M = (int *)p_power_20M;

	__req.p_power_40M = (int *)p_power_40M;

	__req.p_power_80M = (int *)p_power_80M;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_POWER_EXT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_power_ext_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_power_ext_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_power_ext call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_power_ext_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_power_20M)
			*p_power_20M = *__resp.p_power_20M;
	}
	if (__resp.return_code >= 0) {
		if (p_power_40M)
			*p_power_40M = *__resp.p_power_40M;
	}
	if (__resp.return_code >= 0) {
		if (p_power_80M)
			*p_power_80M = *__resp.p_power_80M;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_power_ext_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_tx_power_ext(const char * ifname, const qcsapi_unsigned_int the_channel, const qcsapi_unsigned_int bf_on, const qcsapi_unsigned_int number_ss, const int power_20M, const int power_40M, const int power_80M)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_tx_power_ext_rpcdata __req;
	struct qcsapi_wifi_set_tx_power_ext_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__req.bf_on = (unsigned int)bf_on;

	__req.number_ss = (unsigned int)number_ss;

	__req.power_20M = (int)power_20M;

	__req.power_40M = (int)power_40M;

	__req.power_80M = (int)power_80M;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_TX_POWER_EXT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_tx_power_ext_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_tx_power_ext_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_tx_power_ext call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tx_power_ext_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tx_power_ext_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_chan_power_table(const char * ifname, qcsapi_channel_power_table * chan_power_table)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_chan_power_table_rpcdata __req;
	struct qcsapi_wifi_get_chan_power_table_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.chan_power_table = (__rpc_qcsapi_channel_power_table*)chan_power_table;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CHAN_POWER_TABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_chan_power_table_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_chan_power_table_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_chan_power_table call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_chan_power_table_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.chan_power_table && chan_power_table)
			memcpy(chan_power_table, __resp.chan_power_table, sizeof(*chan_power_table));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_chan_power_table_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_chan_power_table(const char * ifname, qcsapi_channel_power_table * chan_power_table)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_chan_power_table_rpcdata __req;
	struct qcsapi_wifi_set_chan_power_table_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.chan_power_table = (__rpc_qcsapi_channel_power_table*)chan_power_table;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_CHAN_POWER_TABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_chan_power_table_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_chan_power_table_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_chan_power_table call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_chan_power_table_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.chan_power_table && chan_power_table)
			memcpy(chan_power_table, __resp.chan_power_table, sizeof(*chan_power_table));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_chan_power_table_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_power_selection(qcsapi_unsigned_int * p_power_selection)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_power_selection_rpcdata __req;
	struct qcsapi_wifi_get_power_selection_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_power_selection = (unsigned int *)p_power_selection;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_POWER_SELECTION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_power_selection_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_power_selection_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_power_selection call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_power_selection_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_power_selection)
			*p_power_selection = *__resp.p_power_selection;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_power_selection_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_power_selection(const qcsapi_unsigned_int power_selection)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_power_selection_rpcdata __req;
	struct qcsapi_wifi_set_power_selection_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.power_selection = (unsigned int)power_selection;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_POWER_SELECTION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_power_selection_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_power_selection_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_power_selection call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_power_selection_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_power_selection_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_carrier_interference(const char * ifname, int * ci)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_carrier_interference_rpcdata __req;
	struct qcsapi_wifi_get_carrier_interference_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ci = (int *)ci;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CARRIER_INTERFERENCE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_carrier_interference_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_carrier_interference_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_carrier_interference call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_carrier_interference_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ci)
			*ci = *__resp.ci;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_carrier_interference_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_congestion_index(const char * ifname, int * ci)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_congestion_index_rpcdata __req;
	struct qcsapi_wifi_get_congestion_index_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ci = (int *)ci;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CONGESTION_INDEX_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_congestion_index_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_congestion_index_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_congestion_index call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_congestion_index_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ci)
			*ci = *__resp.ci;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_congestion_index_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_supported_tx_power_levels(const char * ifname, string_128 available_percentages)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_supported_tx_power_levels_rpcdata __req;
	struct qcsapi_wifi_get_supported_tx_power_levels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcavailable_percentages = {(char *)available_percentages};
	__rpc_string *p__rpcavailable_percentages = (available_percentages) ? &__rpcavailable_percentages : NULL;
	__req.available_percentages = p__rpcavailable_percentages;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SUPPORTED_TX_POWER_LEVELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_supported_tx_power_levels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_supported_tx_power_levels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_supported_tx_power_levels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_supported_tx_power_levels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (available_percentages && __resp.available_percentages)
			strcpy(available_percentages, __resp.available_percentages->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_supported_tx_power_levels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_current_tx_power_level(const char * ifname, uint32_t * p_current_percentage)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_current_tx_power_level_rpcdata __req;
	struct qcsapi_wifi_get_current_tx_power_level_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_current_percentage = (uint32_t *)p_current_percentage;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CURRENT_TX_POWER_LEVEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_current_tx_power_level_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_current_tx_power_level_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_current_tx_power_level call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_current_tx_power_level_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_current_percentage)
			*p_current_percentage = *__resp.p_current_percentage;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_current_tx_power_level_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_power_constraint(const char * ifname, uint32_t pwr_constraint)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_power_constraint_rpcdata __req;
	struct qcsapi_wifi_set_power_constraint_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.pwr_constraint = (uint32_t)pwr_constraint;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_POWER_CONSTRAINT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_power_constraint_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_power_constraint_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_power_constraint call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_power_constraint_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_power_constraint_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_power_constraint(const char * ifname, uint32_t * p_pwr_constraint)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_power_constraint_rpcdata __req;
	struct qcsapi_wifi_get_power_constraint_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_pwr_constraint = (uint32_t *)p_pwr_constraint;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_POWER_CONSTRAINT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_power_constraint_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_power_constraint_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_power_constraint call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_power_constraint_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_pwr_constraint)
			*p_pwr_constraint = *__resp.p_pwr_constraint;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_power_constraint_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_tpc_interval(const char * ifname, int32_t tpc_interval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_tpc_interval_rpcdata __req;
	struct qcsapi_wifi_set_tpc_interval_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.tpc_interval = (int32_t)tpc_interval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_TPC_INTERVAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_tpc_interval_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_tpc_interval_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_tpc_interval call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tpc_interval_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tpc_interval_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tpc_interval(const char * ifname, uint32_t * p_tpc_interval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tpc_interval_rpcdata __req;
	struct qcsapi_wifi_get_tpc_interval_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_tpc_interval = (uint32_t *)p_tpc_interval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TPC_INTERVAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tpc_interval_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tpc_interval_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tpc_interval call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tpc_interval_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tpc_interval)
			*p_tpc_interval = *__resp.p_tpc_interval;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tpc_interval_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_assoc_records(const char * ifname, int reset, qcsapi_assoc_records * records)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_assoc_records_rpcdata __req;
	struct qcsapi_wifi_get_assoc_records_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.reset = (int)reset;

	__req.records = (__rpc_qcsapi_assoc_records*)records;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ASSOC_RECORDS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_assoc_records_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_assoc_records_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_assoc_records call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_assoc_records_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.records && records)
			memcpy(records, __resp.records, sizeof(*records));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_assoc_records_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_ap_isolate(const char * ifname, int * p_ap_isolate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_ap_isolate_rpcdata __req;
	struct qcsapi_wifi_get_ap_isolate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_ap_isolate = (int *)p_ap_isolate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AP_ISOLATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_ap_isolate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_ap_isolate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_ap_isolate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ap_isolate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_ap_isolate)
			*p_ap_isolate = *__resp.p_ap_isolate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ap_isolate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ap_isolate(const char * ifname, const int new_ap_isolate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ap_isolate_rpcdata __req;
	struct qcsapi_wifi_set_ap_isolate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_ap_isolate = (int)new_ap_isolate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_AP_ISOLATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ap_isolate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ap_isolate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ap_isolate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ap_isolate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ap_isolate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_intra_bss_isolate(const char * ifname, qcsapi_unsigned_int * p_ap_isolate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_intra_bss_isolate_rpcdata __req;
	struct qcsapi_wifi_get_intra_bss_isolate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_ap_isolate = (unsigned int *)p_ap_isolate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_INTRA_BSS_ISOLATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_intra_bss_isolate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_intra_bss_isolate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_intra_bss_isolate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_intra_bss_isolate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_ap_isolate)
			*p_ap_isolate = *__resp.p_ap_isolate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_intra_bss_isolate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_intra_bss_isolate(const char * ifname, const qcsapi_unsigned_int new_ap_isolate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_intra_bss_isolate_rpcdata __req;
	struct qcsapi_wifi_set_intra_bss_isolate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_ap_isolate = (unsigned int)new_ap_isolate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_INTRA_BSS_ISOLATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_intra_bss_isolate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_intra_bss_isolate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_intra_bss_isolate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_intra_bss_isolate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_intra_bss_isolate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bss_isolate(const char * ifname, qcsapi_unsigned_int * p_ap_isolate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bss_isolate_rpcdata __req;
	struct qcsapi_wifi_get_bss_isolate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_ap_isolate = (unsigned int *)p_ap_isolate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BSS_ISOLATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bss_isolate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bss_isolate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bss_isolate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bss_isolate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_ap_isolate)
			*p_ap_isolate = *__resp.p_ap_isolate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bss_isolate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bss_isolate(const char * ifname, const qcsapi_unsigned_int new_ap_isolate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bss_isolate_rpcdata __req;
	struct qcsapi_wifi_set_bss_isolate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_ap_isolate = (unsigned int)new_ap_isolate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BSS_ISOLATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bss_isolate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bss_isolate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bss_isolate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bss_isolate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bss_isolate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_disable_dfs_channels(const char * ifname, int disable_dfs, int channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_disable_dfs_channels_rpcdata __req;
	struct qcsapi_wifi_disable_dfs_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.disable_dfs = (int)disable_dfs;

	__req.channel = (int)channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DISABLE_DFS_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_disable_dfs_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_disable_dfs_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_disable_dfs_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_disable_dfs_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_disable_dfs_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_create_restricted_bss(const char * ifname, const qcsapi_mac_addr mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_create_restricted_bss_rpcdata __req;
	struct qcsapi_wifi_create_restricted_bss_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcmac_addr;
	if (mac_addr) {
		memcpy(__rpcmac_addr.data, mac_addr, sizeof(__rpcmac_addr));
		__req.mac_addr = &__rpcmac_addr;
	} else {
		__req.mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_CREATE_RESTRICTED_BSS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_create_restricted_bss_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_create_restricted_bss_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_create_restricted_bss call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_create_restricted_bss_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_create_restricted_bss_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_create_bss(const char * ifname, const qcsapi_mac_addr mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_create_bss_rpcdata __req;
	struct qcsapi_wifi_create_bss_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcmac_addr;
	if (mac_addr) {
		memcpy(__rpcmac_addr.data, mac_addr, sizeof(__rpcmac_addr));
		__req.mac_addr = &__rpcmac_addr;
	} else {
		__req.mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_CREATE_BSS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_create_bss_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_create_bss_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_create_bss call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_create_bss_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_create_bss_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_remove_bss(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_remove_bss_rpcdata __req;
	struct qcsapi_wifi_remove_bss_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_REMOVE_BSS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_remove_bss_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_remove_bss_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_remove_bss call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_remove_bss_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_remove_bss_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_add_peer(const char * ifname, const qcsapi_mac_addr peer_address)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_add_peer_rpcdata __req;
	struct qcsapi_wds_add_peer_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcpeer_address;
	if (peer_address) {
		memcpy(__rpcpeer_address.data, peer_address, sizeof(__rpcpeer_address));
		__req.peer_address = &__rpcpeer_address;
	} else {
		__req.peer_address = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_ADD_PEER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_add_peer_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_add_peer_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_add_peer call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_add_peer_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_add_peer_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_add_peer_encrypt(const char * ifname, const qcsapi_mac_addr peer_address, const qcsapi_unsigned_int encryption)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_add_peer_encrypt_rpcdata __req;
	struct qcsapi_wds_add_peer_encrypt_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcpeer_address;
	if (peer_address) {
		memcpy(__rpcpeer_address.data, peer_address, sizeof(__rpcpeer_address));
		__req.peer_address = &__rpcpeer_address;
	} else {
		__req.peer_address = NULL;
	}
	__req.encryption = (unsigned int)encryption;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_ADD_PEER_ENCRYPT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_add_peer_encrypt_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_add_peer_encrypt_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_add_peer_encrypt call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_add_peer_encrypt_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_add_peer_encrypt_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_remove_peer(const char * ifname, const qcsapi_mac_addr peer_address)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_remove_peer_rpcdata __req;
	struct qcsapi_wds_remove_peer_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcpeer_address;
	if (peer_address) {
		memcpy(__rpcpeer_address.data, peer_address, sizeof(__rpcpeer_address));
		__req.peer_address = &__rpcpeer_address;
	} else {
		__req.peer_address = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_REMOVE_PEER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_remove_peer_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_remove_peer_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_remove_peer call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_remove_peer_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_remove_peer_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_get_peer_address(const char * ifname, const int index, qcsapi_mac_addr peer_address)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_get_peer_address_rpcdata __req;
	struct qcsapi_wds_get_peer_address_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.index = (int)index;

	struct __rpc_qcsapi_mac_addr __rpcpeer_address;
	if (peer_address) {
		memcpy(__rpcpeer_address.data, peer_address, sizeof(__rpcpeer_address));
		__req.peer_address = &__rpcpeer_address;
	} else {
		__req.peer_address = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_GET_PEER_ADDRESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_get_peer_address_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_get_peer_address_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_get_peer_address call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_get_peer_address_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (peer_address && __resp.peer_address)
			memcpy(peer_address, __resp.peer_address->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_get_peer_address_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_set_psk(const char * ifname, const qcsapi_mac_addr peer_address, const string_64 pre_shared_key)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_set_psk_rpcdata __req;
	struct qcsapi_wds_set_psk_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcpeer_address;
	if (peer_address) {
		memcpy(__rpcpeer_address.data, peer_address, sizeof(__rpcpeer_address));
		__req.peer_address = &__rpcpeer_address;
	} else {
		__req.peer_address = NULL;
	}
	__rpc_string __rpcpre_shared_key = {(char *)pre_shared_key};
	__rpc_string *p__rpcpre_shared_key = (pre_shared_key) ? &__rpcpre_shared_key : NULL;
	__req.pre_shared_key = p__rpcpre_shared_key;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_SET_PSK_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_set_psk_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_set_psk_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_set_psk call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_set_psk_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_set_psk_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_set_mode(const char * ifname, const qcsapi_mac_addr peer_address, const int mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_set_mode_rpcdata __req;
	struct qcsapi_wds_set_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcpeer_address;
	if (peer_address) {
		memcpy(__rpcpeer_address.data, peer_address, sizeof(__rpcpeer_address));
		__req.peer_address = &__rpcpeer_address;
	} else {
		__req.peer_address = NULL;
	}
	__req.mode = (int)mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_SET_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_set_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_set_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_set_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_set_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_set_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wds_get_mode(const char * ifname, const int index, int * mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wds_get_mode_rpcdata __req;
	struct qcsapi_wds_get_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.index = (int)index;

	__req.mode = (int *)mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WDS_GET_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wds_get_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wds_get_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wds_get_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wds_get_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (mode)
			*mode = *__resp.mode;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wds_get_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_extender_params(const char * ifname, const qcsapi_extender_type type, const int param_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_extender_params_rpcdata __req;
	struct qcsapi_wifi_set_extender_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.type = (int)type;

	__req.param_value = (int)param_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_EXTENDER_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_extender_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_extender_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_extender_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_extender_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_extender_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_extender_params(const char * ifname, const qcsapi_extender_type type, int * p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_extender_params_rpcdata __req;
	struct qcsapi_wifi_get_extender_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.type = (int)type;

	__req.p_value = (int *)p_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_EXTENDER_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_extender_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_extender_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_extender_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_extender_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_extender_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_beacon_type(const char * ifname, char * p_current_beacon)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_beacon_type_rpcdata __req;
	struct qcsapi_wifi_get_beacon_type_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_current_beacon = {(char *)p_current_beacon};
	__rpc_string *p__rpcp_current_beacon = (p_current_beacon) ? &__rpcp_current_beacon : NULL;
	__req.p_current_beacon = p__rpcp_current_beacon;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BEACON_TYPE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_beacon_type_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_beacon_type_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_beacon_type call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_beacon_type_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_current_beacon && __resp.p_current_beacon)
			strcpy(p_current_beacon, __resp.p_current_beacon->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_beacon_type_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_beacon_type(const char * ifname, const char * p_new_beacon)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_beacon_type_rpcdata __req;
	struct qcsapi_wifi_set_beacon_type_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_new_beacon = {(char *)p_new_beacon};
	__rpc_string *p__rpcp_new_beacon = (p_new_beacon) ? &__rpcp_new_beacon : NULL;
	__req.p_new_beacon = p__rpcp_new_beacon;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BEACON_TYPE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_beacon_type_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_beacon_type_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_beacon_type call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_beacon_type_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_beacon_type_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_WEP_key_index(const char * ifname, qcsapi_unsigned_int * p_key_index)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_WEP_key_index_rpcdata __req;
	struct qcsapi_wifi_get_WEP_key_index_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_key_index = (unsigned int *)p_key_index;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WEP_KEY_INDEX_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_index_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_index_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_WEP_key_index call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_index_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_key_index)
			*p_key_index = *__resp.p_key_index;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_index_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_WEP_key_index(const char * ifname, const qcsapi_unsigned_int key_index)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_WEP_key_index_rpcdata __req;
	struct qcsapi_wifi_set_WEP_key_index_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WEP_KEY_INDEX_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_index_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_index_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_WEP_key_index call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_index_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_index_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_WEP_key_passphrase(const char * ifname, string_64 current_passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_WEP_key_passphrase_rpcdata __req;
	struct qcsapi_wifi_get_WEP_key_passphrase_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_passphrase = {(char *)current_passphrase};
	__rpc_string *p__rpccurrent_passphrase = (current_passphrase) ? &__rpccurrent_passphrase : NULL;
	__req.current_passphrase = p__rpccurrent_passphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WEP_KEY_PASSPHRASE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_passphrase_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_passphrase_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_WEP_key_passphrase call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_passphrase_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_passphrase && __resp.current_passphrase)
			strcpy(current_passphrase, __resp.current_passphrase->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_passphrase_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_WEP_key_passphrase(const char * ifname, const string_64 new_passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_WEP_key_passphrase_rpcdata __req;
	struct qcsapi_wifi_set_WEP_key_passphrase_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcnew_passphrase = {(char *)new_passphrase};
	__rpc_string *p__rpcnew_passphrase = (new_passphrase) ? &__rpcnew_passphrase : NULL;
	__req.new_passphrase = p__rpcnew_passphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WEP_KEY_PASSPHRASE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_passphrase_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_passphrase_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_WEP_key_passphrase call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_passphrase_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_passphrase_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_WEP_encryption_level(const char * ifname, string_64 current_encryption_level)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_WEP_encryption_level_rpcdata __req;
	struct qcsapi_wifi_get_WEP_encryption_level_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_encryption_level = {(char *)current_encryption_level};
	__rpc_string *p__rpccurrent_encryption_level = (current_encryption_level) ? &__rpccurrent_encryption_level : NULL;
	__req.current_encryption_level = p__rpccurrent_encryption_level;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WEP_ENCRYPTION_LEVEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_encryption_level_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_encryption_level_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_WEP_encryption_level call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_encryption_level_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_encryption_level && __resp.current_encryption_level)
			strcpy(current_encryption_level, __resp.current_encryption_level->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_encryption_level_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_basic_encryption_modes(const char * ifname, string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_basic_encryption_modes_rpcdata __req;
	struct qcsapi_wifi_get_basic_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BASIC_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_basic_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_basic_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_basic_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_basic_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (encryption_modes && __resp.encryption_modes)
			strcpy(encryption_modes, __resp.encryption_modes->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_basic_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_basic_encryption_modes(const char * ifname, const string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_basic_encryption_modes_rpcdata __req;
	struct qcsapi_wifi_set_basic_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BASIC_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_basic_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_basic_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_basic_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_basic_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_basic_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_basic_authentication_mode(const char * ifname, string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_basic_authentication_mode_rpcdata __req;
	struct qcsapi_wifi_get_basic_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BASIC_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_basic_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_basic_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_basic_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_basic_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (authentication_mode && __resp.authentication_mode)
			strcpy(authentication_mode, __resp.authentication_mode->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_basic_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_basic_authentication_mode(const char * ifname, const string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_basic_authentication_mode_rpcdata __req;
	struct qcsapi_wifi_set_basic_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BASIC_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_basic_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_basic_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_basic_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_basic_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_basic_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_WEP_key(const char * ifname, qcsapi_unsigned_int key_index, string_64 current_passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_WEP_key_rpcdata __req;
	struct qcsapi_wifi_get_WEP_key_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpccurrent_passphrase = {(char *)current_passphrase};
	__rpc_string *p__rpccurrent_passphrase = (current_passphrase) ? &__rpccurrent_passphrase : NULL;
	__req.current_passphrase = p__rpccurrent_passphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WEP_KEY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_WEP_key call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_passphrase && __resp.current_passphrase)
			strcpy(current_passphrase, __resp.current_passphrase->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WEP_key_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_WEP_key(const char * ifname, qcsapi_unsigned_int key_index, const string_64 new_passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_WEP_key_rpcdata __req;
	struct qcsapi_wifi_set_WEP_key_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcnew_passphrase = {(char *)new_passphrase};
	__rpc_string *p__rpcnew_passphrase = (new_passphrase) ? &__rpcnew_passphrase : NULL;
	__req.new_passphrase = p__rpcnew_passphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WEP_KEY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_WEP_key call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WEP_key_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_WPA_encryption_modes(const char * ifname, string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_WPA_encryption_modes_rpcdata __req;
	struct qcsapi_wifi_get_WPA_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WPA_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_WPA_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_WPA_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_WPA_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WPA_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (encryption_modes && __resp.encryption_modes)
			strcpy(encryption_modes, __resp.encryption_modes->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WPA_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_WPA_encryption_modes(const char * ifname, const string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_WPA_encryption_modes_rpcdata __req;
	struct qcsapi_wifi_set_WPA_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WPA_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_WPA_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_WPA_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_WPA_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WPA_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WPA_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_WPA_authentication_mode(const char * ifname, string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_WPA_authentication_mode_rpcdata __req;
	struct qcsapi_wifi_get_WPA_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WPA_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_WPA_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_WPA_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_WPA_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WPA_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (authentication_mode && __resp.authentication_mode)
			strcpy(authentication_mode, __resp.authentication_mode->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_WPA_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_WPA_authentication_mode(const char * ifname, const string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_WPA_authentication_mode_rpcdata __req;
	struct qcsapi_wifi_set_WPA_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_WPA_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_WPA_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_WPA_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_WPA_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WPA_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_WPA_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_interworking(const char * ifname, string_32 interworking)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_interworking_rpcdata __req;
	struct qcsapi_wifi_get_interworking_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcinterworking = {(char *)interworking};
	__rpc_string *p__rpcinterworking = (interworking) ? &__rpcinterworking : NULL;
	__req.interworking = p__rpcinterworking;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_INTERWORKING_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_interworking_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_interworking_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_interworking call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_interworking_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (interworking && __resp.interworking)
			strcpy(interworking, __resp.interworking->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_interworking_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_interworking(const char * ifname, const string_32 interworking)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_interworking_rpcdata __req;
	struct qcsapi_wifi_set_interworking_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcinterworking = {(char *)interworking};
	__rpc_string *p__rpcinterworking = (interworking) ? &__rpcinterworking : NULL;
	__req.interworking = p__rpcinterworking;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_INTERWORKING_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_interworking_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_interworking_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_interworking call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_interworking_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_interworking_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_80211u_params(const char * ifname, const string_32 u_param, string_256 p_buffer)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_80211u_params_rpcdata __req;
	struct qcsapi_wifi_get_80211u_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcu_param = {(char *)u_param};
	__rpc_string *p__rpcu_param = (u_param) ? &__rpcu_param : NULL;
	__req.u_param = p__rpcu_param;

	__rpc_string __rpcp_buffer = {(char *)p_buffer};
	__rpc_string *p__rpcp_buffer = (p_buffer) ? &__rpcp_buffer : NULL;
	__req.p_buffer = p__rpcp_buffer;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_80211U_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_80211u_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_80211u_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_80211u_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_80211u_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_buffer && __resp.p_buffer)
			strcpy(p_buffer, __resp.p_buffer->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_80211u_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_80211u_params(const char * ifname, const string_32 param, const string_256 value1, const string_32 value2)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_80211u_params_rpcdata __req;
	struct qcsapi_wifi_set_80211u_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam = {(char *)param};
	__rpc_string *p__rpcparam = (param) ? &__rpcparam : NULL;
	__req.param = p__rpcparam;

	__rpc_string __rpcvalue1 = {(char *)value1};
	__rpc_string *p__rpcvalue1 = (value1) ? &__rpcvalue1 : NULL;
	__req.value1 = p__rpcvalue1;

	__rpc_string __rpcvalue2 = {(char *)value2};
	__rpc_string *p__rpcvalue2 = (value2) ? &__rpcvalue2 : NULL;
	__req.value2 = p__rpcvalue2;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_80211U_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_80211u_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_80211u_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_80211u_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_80211u_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_80211u_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_get_nai_realms(const char * ifname, string_4096 p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_get_nai_realms_rpcdata __req;
	struct qcsapi_security_get_nai_realms_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_GET_NAI_REALMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_get_nai_realms_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_get_nai_realms_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_get_nai_realms call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_get_nai_realms_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value && __resp.p_value)
			strcpy(p_value, __resp.p_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_get_nai_realms_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_add_nai_realm(const char * ifname, const int encoding, const char * nai_realm, const char * eap_method)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_add_nai_realm_rpcdata __req;
	struct qcsapi_security_add_nai_realm_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.encoding = (int)encoding;

	__rpc_string __rpcnai_realm = {(char *)nai_realm};
	__rpc_string *p__rpcnai_realm = (nai_realm) ? &__rpcnai_realm : NULL;
	__req.nai_realm = p__rpcnai_realm;

	__rpc_string __rpceap_method = {(char *)eap_method};
	__rpc_string *p__rpceap_method = (eap_method) ? &__rpceap_method : NULL;
	__req.eap_method = p__rpceap_method;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_ADD_NAI_REALM_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_add_nai_realm_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_add_nai_realm_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_add_nai_realm call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_add_nai_realm_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_add_nai_realm_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_del_nai_realm(const char * ifname, const char * nai_realm)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_del_nai_realm_rpcdata __req;
	struct qcsapi_security_del_nai_realm_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcnai_realm = {(char *)nai_realm};
	__rpc_string *p__rpcnai_realm = (nai_realm) ? &__rpcnai_realm : NULL;
	__req.nai_realm = p__rpcnai_realm;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_DEL_NAI_REALM_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_del_nai_realm_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_del_nai_realm_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_del_nai_realm call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_del_nai_realm_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_del_nai_realm_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_get_roaming_consortium(const char * ifname, string_1024 p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_get_roaming_consortium_rpcdata __req;
	struct qcsapi_security_get_roaming_consortium_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_GET_ROAMING_CONSORTIUM_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_get_roaming_consortium_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_get_roaming_consortium_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_get_roaming_consortium call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_get_roaming_consortium_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value && __resp.p_value)
			strcpy(p_value, __resp.p_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_get_roaming_consortium_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_add_roaming_consortium(const char * ifname, const char * p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_add_roaming_consortium_rpcdata __req;
	struct qcsapi_security_add_roaming_consortium_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_ADD_ROAMING_CONSORTIUM_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_add_roaming_consortium_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_add_roaming_consortium_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_add_roaming_consortium call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_add_roaming_consortium_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_add_roaming_consortium_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_del_roaming_consortium(const char * ifname, const char * p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_del_roaming_consortium_rpcdata __req;
	struct qcsapi_security_del_roaming_consortium_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_DEL_ROAMING_CONSORTIUM_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_del_roaming_consortium_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_del_roaming_consortium_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_del_roaming_consortium call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_del_roaming_consortium_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_del_roaming_consortium_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_get_venue_name(const char * ifname, string_4096 p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_get_venue_name_rpcdata __req;
	struct qcsapi_security_get_venue_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_GET_VENUE_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_get_venue_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_get_venue_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_get_venue_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_get_venue_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value && __resp.p_value)
			strcpy(p_value, __resp.p_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_get_venue_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_add_venue_name(const char * ifname, const char * lang_code, const char * venue_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_add_venue_name_rpcdata __req;
	struct qcsapi_security_add_venue_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclang_code = {(char *)lang_code};
	__rpc_string *p__rpclang_code = (lang_code) ? &__rpclang_code : NULL;
	__req.lang_code = p__rpclang_code;

	__rpc_string __rpcvenue_name = {(char *)venue_name};
	__rpc_string *p__rpcvenue_name = (venue_name) ? &__rpcvenue_name : NULL;
	__req.venue_name = p__rpcvenue_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_ADD_VENUE_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_add_venue_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_add_venue_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_add_venue_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_add_venue_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_add_venue_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_del_venue_name(const char * ifname, const char * lang_code, const char * venue_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_del_venue_name_rpcdata __req;
	struct qcsapi_security_del_venue_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclang_code = {(char *)lang_code};
	__rpc_string *p__rpclang_code = (lang_code) ? &__rpclang_code : NULL;
	__req.lang_code = p__rpclang_code;

	__rpc_string __rpcvenue_name = {(char *)venue_name};
	__rpc_string *p__rpcvenue_name = (venue_name) ? &__rpcvenue_name : NULL;
	__req.venue_name = p__rpcvenue_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_DEL_VENUE_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_del_venue_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_del_venue_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_del_venue_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_del_venue_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_del_venue_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_get_oper_friendly_name(const char * ifname, string_4096 p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_get_oper_friendly_name_rpcdata __req;
	struct qcsapi_security_get_oper_friendly_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_GET_OPER_FRIENDLY_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_get_oper_friendly_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_get_oper_friendly_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_get_oper_friendly_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_get_oper_friendly_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value && __resp.p_value)
			strcpy(p_value, __resp.p_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_get_oper_friendly_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_add_oper_friendly_name(const char * ifname, const char * lang_code, const char * oper_friendly_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_add_oper_friendly_name_rpcdata __req;
	struct qcsapi_security_add_oper_friendly_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclang_code = {(char *)lang_code};
	__rpc_string *p__rpclang_code = (lang_code) ? &__rpclang_code : NULL;
	__req.lang_code = p__rpclang_code;

	__rpc_string __rpcoper_friendly_name = {(char *)oper_friendly_name};
	__rpc_string *p__rpcoper_friendly_name = (oper_friendly_name) ? &__rpcoper_friendly_name : NULL;
	__req.oper_friendly_name = p__rpcoper_friendly_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_ADD_OPER_FRIENDLY_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_add_oper_friendly_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_add_oper_friendly_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_add_oper_friendly_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_add_oper_friendly_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_add_oper_friendly_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_del_oper_friendly_name(const char * ifname, const char * lang_code, const char * oper_friendly_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_del_oper_friendly_name_rpcdata __req;
	struct qcsapi_security_del_oper_friendly_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclang_code = {(char *)lang_code};
	__rpc_string *p__rpclang_code = (lang_code) ? &__rpclang_code : NULL;
	__req.lang_code = p__rpclang_code;

	__rpc_string __rpcoper_friendly_name = {(char *)oper_friendly_name};
	__rpc_string *p__rpcoper_friendly_name = (oper_friendly_name) ? &__rpcoper_friendly_name : NULL;
	__req.oper_friendly_name = p__rpcoper_friendly_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_DEL_OPER_FRIENDLY_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_del_oper_friendly_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_del_oper_friendly_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_del_oper_friendly_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_del_oper_friendly_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_del_oper_friendly_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_get_hs20_conn_capab(const char * ifname, string_4096 p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_get_hs20_conn_capab_rpcdata __req;
	struct qcsapi_security_get_hs20_conn_capab_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_value = {(char *)p_value};
	__rpc_string *p__rpcp_value = (p_value) ? &__rpcp_value : NULL;
	__req.p_value = p__rpcp_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_GET_HS20_CONN_CAPAB_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_get_hs20_conn_capab_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_get_hs20_conn_capab_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_get_hs20_conn_capab call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_get_hs20_conn_capab_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value && __resp.p_value)
			strcpy(p_value, __resp.p_value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_get_hs20_conn_capab_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_add_hs20_conn_capab(const char * ifname, const char * ip_proto, const char * port_num, const char * status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_add_hs20_conn_capab_rpcdata __req;
	struct qcsapi_security_add_hs20_conn_capab_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcip_proto = {(char *)ip_proto};
	__rpc_string *p__rpcip_proto = (ip_proto) ? &__rpcip_proto : NULL;
	__req.ip_proto = p__rpcip_proto;

	__rpc_string __rpcport_num = {(char *)port_num};
	__rpc_string *p__rpcport_num = (port_num) ? &__rpcport_num : NULL;
	__req.port_num = p__rpcport_num;

	__rpc_string __rpcstatus = {(char *)status};
	__rpc_string *p__rpcstatus = (status) ? &__rpcstatus : NULL;
	__req.status = p__rpcstatus;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_ADD_HS20_CONN_CAPAB_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_add_hs20_conn_capab_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_add_hs20_conn_capab_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_add_hs20_conn_capab call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_add_hs20_conn_capab_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_add_hs20_conn_capab_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_security_del_hs20_conn_capab(const char * ifname, const char * ip_proto, const char * port_num, const char * status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_security_del_hs20_conn_capab_rpcdata __req;
	struct qcsapi_security_del_hs20_conn_capab_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcip_proto = {(char *)ip_proto};
	__rpc_string *p__rpcip_proto = (ip_proto) ? &__rpcip_proto : NULL;
	__req.ip_proto = p__rpcip_proto;

	__rpc_string __rpcport_num = {(char *)port_num};
	__rpc_string *p__rpcport_num = (port_num) ? &__rpcport_num : NULL;
	__req.port_num = p__rpcport_num;

	__rpc_string __rpcstatus = {(char *)status};
	__rpc_string *p__rpcstatus = (status) ? &__rpcstatus : NULL;
	__req.status = p__rpcstatus;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SECURITY_DEL_HS20_CONN_CAPAB_REMOTE,
				(xdrproc_t)xdr_qcsapi_security_del_hs20_conn_capab_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_security_del_hs20_conn_capab_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_security_del_hs20_conn_capab call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_security_del_hs20_conn_capab_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_security_del_hs20_conn_capab_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_hs20_status(const char * ifname, string_32 p_hs20)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_hs20_status_rpcdata __req;
	struct qcsapi_wifi_get_hs20_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_hs20 = {(char *)p_hs20};
	__rpc_string *p__rpcp_hs20 = (p_hs20) ? &__rpcp_hs20 : NULL;
	__req.p_hs20 = p__rpcp_hs20;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_HS20_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_hs20_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_hs20_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_hs20_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_hs20_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_hs20 && __resp.p_hs20)
			strcpy(p_hs20, __resp.p_hs20->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_hs20_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_hs20_status(const char * ifname, const string_32 hs20_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_hs20_status_rpcdata __req;
	struct qcsapi_wifi_set_hs20_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpchs20_val = {(char *)hs20_val};
	__rpc_string *p__rpchs20_val = (hs20_val) ? &__rpchs20_val : NULL;
	__req.hs20_val = p__rpchs20_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_HS20_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_hs20_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_hs20_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_hs20_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_hs20_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_hs20_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_proxy_arp(const char * ifname, string_32 p_proxy_arp)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_proxy_arp_rpcdata __req;
	struct qcsapi_wifi_get_proxy_arp_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcp_proxy_arp = {(char *)p_proxy_arp};
	__rpc_string *p__rpcp_proxy_arp = (p_proxy_arp) ? &__rpcp_proxy_arp : NULL;
	__req.p_proxy_arp = p__rpcp_proxy_arp;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PROXY_ARP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_proxy_arp_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_proxy_arp_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_proxy_arp call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_proxy_arp_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_proxy_arp && __resp.p_proxy_arp)
			strcpy(p_proxy_arp, __resp.p_proxy_arp->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_proxy_arp_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_proxy_arp(const char * ifname, const string_32 proxy_arp_val)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_proxy_arp_rpcdata __req;
	struct qcsapi_wifi_set_proxy_arp_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcproxy_arp_val = {(char *)proxy_arp_val};
	__rpc_string *p__rpcproxy_arp_val = (proxy_arp_val) ? &__rpcproxy_arp_val : NULL;
	__req.proxy_arp_val = p__rpcproxy_arp_val;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PROXY_ARP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_proxy_arp_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_proxy_arp_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_proxy_arp call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_proxy_arp_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_proxy_arp_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_l2_ext_filter(const char * ifname, const string_32 param, string_32 value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_l2_ext_filter_rpcdata __req;
	struct qcsapi_wifi_get_l2_ext_filter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam = {(char *)param};
	__rpc_string *p__rpcparam = (param) ? &__rpcparam : NULL;
	__req.param = p__rpcparam;

	__rpc_string __rpcvalue = {(char *)value};
	__rpc_string *p__rpcvalue = (value) ? &__rpcvalue : NULL;
	__req.value = p__rpcvalue;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_L2_EXT_FILTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_l2_ext_filter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_l2_ext_filter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_l2_ext_filter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_l2_ext_filter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (value && __resp.value)
			strcpy(value, __resp.value->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_l2_ext_filter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_l2_ext_filter(const char * ifname, const string_32 param, const string_32 value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_l2_ext_filter_rpcdata __req;
	struct qcsapi_wifi_set_l2_ext_filter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam = {(char *)param};
	__rpc_string *p__rpcparam = (param) ? &__rpcparam : NULL;
	__req.param = p__rpcparam;

	__rpc_string __rpcvalue = {(char *)value};
	__rpc_string *p__rpcvalue = (value) ? &__rpcvalue : NULL;
	__req.value = p__rpcvalue;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_L2_EXT_FILTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_l2_ext_filter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_l2_ext_filter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_l2_ext_filter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_l2_ext_filter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_l2_ext_filter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_hs20_params(const char * ifname, const string_32 hs_param, string_32 p_buffer)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_hs20_params_rpcdata __req;
	struct qcsapi_wifi_get_hs20_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpchs_param = {(char *)hs_param};
	__rpc_string *p__rpchs_param = (hs_param) ? &__rpchs_param : NULL;
	__req.hs_param = p__rpchs_param;

	__rpc_string __rpcp_buffer = {(char *)p_buffer};
	__rpc_string *p__rpcp_buffer = (p_buffer) ? &__rpcp_buffer : NULL;
	__req.p_buffer = p__rpcp_buffer;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_HS20_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_hs20_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_hs20_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_hs20_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_hs20_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_buffer && __resp.p_buffer)
			strcpy(p_buffer, __resp.p_buffer->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_hs20_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_hs20_params(const char * ifname, const string_32 hs_param, const string_64 value1, const string_64 value2, const string_64 value3, const string_64 value4, const string_64 value5, const string_64 value6)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_hs20_params_rpcdata __req;
	struct qcsapi_wifi_set_hs20_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpchs_param = {(char *)hs_param};
	__rpc_string *p__rpchs_param = (hs_param) ? &__rpchs_param : NULL;
	__req.hs_param = p__rpchs_param;

	__rpc_string __rpcvalue1 = {(char *)value1};
	__rpc_string *p__rpcvalue1 = (value1) ? &__rpcvalue1 : NULL;
	__req.value1 = p__rpcvalue1;

	__rpc_string __rpcvalue2 = {(char *)value2};
	__rpc_string *p__rpcvalue2 = (value2) ? &__rpcvalue2 : NULL;
	__req.value2 = p__rpcvalue2;

	__rpc_string __rpcvalue3 = {(char *)value3};
	__rpc_string *p__rpcvalue3 = (value3) ? &__rpcvalue3 : NULL;
	__req.value3 = p__rpcvalue3;

	__rpc_string __rpcvalue4 = {(char *)value4};
	__rpc_string *p__rpcvalue4 = (value4) ? &__rpcvalue4 : NULL;
	__req.value4 = p__rpcvalue4;

	__rpc_string __rpcvalue5 = {(char *)value5};
	__rpc_string *p__rpcvalue5 = (value5) ? &__rpcvalue5 : NULL;
	__req.value5 = p__rpcvalue5;

	__rpc_string __rpcvalue6 = {(char *)value6};
	__rpc_string *p__rpcvalue6 = (value6) ? &__rpcvalue6 : NULL;
	__req.value6 = p__rpcvalue6;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_HS20_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_hs20_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_hs20_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_hs20_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_hs20_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_hs20_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_remove_11u_param(const char * ifname, const string_64 param)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_remove_11u_param_rpcdata __req;
	struct qcsapi_remove_11u_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcparam = {(char *)param};
	__rpc_string *p__rpcparam = (param) ? &__rpcparam : NULL;
	__req.param = p__rpcparam;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REMOVE_11U_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_remove_11u_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_remove_11u_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_remove_11u_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_remove_11u_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_remove_11u_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_remove_hs20_param(const char * ifname, const string_64 hs_param)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_remove_hs20_param_rpcdata __req;
	struct qcsapi_remove_hs20_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpchs_param = {(char *)hs_param};
	__rpc_string *p__rpchs_param = (hs_param) ? &__rpchs_param : NULL;
	__req.hs_param = p__rpchs_param;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REMOVE_HS20_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_remove_hs20_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_remove_hs20_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_remove_hs20_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_remove_hs20_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_remove_hs20_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_IEEE11i_encryption_modes(const char * ifname, string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_IEEE11i_encryption_modes_rpcdata __req;
	struct qcsapi_wifi_get_IEEE11i_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_IEEE11I_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_IEEE11i_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (encryption_modes && __resp.encryption_modes)
			strcpy(encryption_modes, __resp.encryption_modes->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_IEEE11i_encryption_modes(const char * ifname, const string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_IEEE11i_encryption_modes_rpcdata __req;
	struct qcsapi_wifi_set_IEEE11i_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_IEEE11I_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_IEEE11i_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_IEEE11i_authentication_mode(const char * ifname, string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_IEEE11i_authentication_mode_rpcdata __req;
	struct qcsapi_wifi_get_IEEE11i_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_IEEE11I_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_IEEE11i_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (authentication_mode && __resp.authentication_mode)
			strcpy(authentication_mode, __resp.authentication_mode->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_IEEE11i_authentication_mode(const char * ifname, const string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_IEEE11i_authentication_mode_rpcdata __req;
	struct qcsapi_wifi_set_IEEE11i_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_IEEE11I_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_IEEE11i_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_IEEE11i_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_michael_errcnt(const char * ifname, uint32_t * errcount)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_michael_errcnt_rpcdata __req;
	struct qcsapi_wifi_get_michael_errcnt_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.errcount = (uint32_t *)errcount;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MICHAEL_ERRCNT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_michael_errcnt_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_michael_errcnt_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_michael_errcnt call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_michael_errcnt_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (errcount)
			*errcount = *__resp.errcount;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_michael_errcnt_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_pre_shared_key(const char * ifname, const qcsapi_unsigned_int key_index, string_64 pre_shared_key)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_pre_shared_key_rpcdata __req;
	struct qcsapi_wifi_get_pre_shared_key_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpre_shared_key = {(char *)pre_shared_key};
	__rpc_string *p__rpcpre_shared_key = (pre_shared_key) ? &__rpcpre_shared_key : NULL;
	__req.pre_shared_key = p__rpcpre_shared_key;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PRE_SHARED_KEY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_pre_shared_key_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_pre_shared_key_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_pre_shared_key call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pre_shared_key_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (pre_shared_key && __resp.pre_shared_key)
			strcpy(pre_shared_key, __resp.pre_shared_key->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pre_shared_key_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_pre_shared_key(const char * ifname, const qcsapi_unsigned_int key_index, const string_64 pre_shared_key)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_pre_shared_key_rpcdata __req;
	struct qcsapi_wifi_set_pre_shared_key_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpre_shared_key = {(char *)pre_shared_key};
	__rpc_string *p__rpcpre_shared_key = (pre_shared_key) ? &__rpcpre_shared_key : NULL;
	__req.pre_shared_key = p__rpcpre_shared_key;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PRE_SHARED_KEY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_pre_shared_key_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_pre_shared_key_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_pre_shared_key call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pre_shared_key_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pre_shared_key_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_add_radius_auth_server_cfg(const char * ifname, const char * radius_auth_server_ipaddr, const char * radius_auth_server_port, const char * radius_auth_server_sh_key)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_add_radius_auth_server_cfg_rpcdata __req;
	struct qcsapi_wifi_add_radius_auth_server_cfg_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcradius_auth_server_ipaddr = {(char *)radius_auth_server_ipaddr};
	__rpc_string *p__rpcradius_auth_server_ipaddr = (radius_auth_server_ipaddr) ? &__rpcradius_auth_server_ipaddr : NULL;
	__req.radius_auth_server_ipaddr = p__rpcradius_auth_server_ipaddr;

	__rpc_string __rpcradius_auth_server_port = {(char *)radius_auth_server_port};
	__rpc_string *p__rpcradius_auth_server_port = (radius_auth_server_port) ? &__rpcradius_auth_server_port : NULL;
	__req.radius_auth_server_port = p__rpcradius_auth_server_port;

	__rpc_string __rpcradius_auth_server_sh_key = {(char *)radius_auth_server_sh_key};
	__rpc_string *p__rpcradius_auth_server_sh_key = (radius_auth_server_sh_key) ? &__rpcradius_auth_server_sh_key : NULL;
	__req.radius_auth_server_sh_key = p__rpcradius_auth_server_sh_key;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_ADD_RADIUS_AUTH_SERVER_CFG_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_add_radius_auth_server_cfg_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_add_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_add_radius_auth_server_cfg call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_add_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_add_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_del_radius_auth_server_cfg(const char * ifname, const char * radius_auth_server_ipaddr, const char * constp_radius_port)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_del_radius_auth_server_cfg_rpcdata __req;
	struct qcsapi_wifi_del_radius_auth_server_cfg_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcradius_auth_server_ipaddr = {(char *)radius_auth_server_ipaddr};
	__rpc_string *p__rpcradius_auth_server_ipaddr = (radius_auth_server_ipaddr) ? &__rpcradius_auth_server_ipaddr : NULL;
	__req.radius_auth_server_ipaddr = p__rpcradius_auth_server_ipaddr;

	__rpc_string __rpcconstp_radius_port = {(char *)constp_radius_port};
	__rpc_string *p__rpcconstp_radius_port = (constp_radius_port) ? &__rpcconstp_radius_port : NULL;
	__req.constp_radius_port = p__rpcconstp_radius_port;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DEL_RADIUS_AUTH_SERVER_CFG_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_del_radius_auth_server_cfg_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_del_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_del_radius_auth_server_cfg call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_del_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_del_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_radius_auth_server_cfg(const char * ifname, string_1024 radius_auth_server_cfg)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_radius_auth_server_cfg_rpcdata __req;
	struct qcsapi_wifi_get_radius_auth_server_cfg_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcradius_auth_server_cfg = {(char *)radius_auth_server_cfg};
	__rpc_string *p__rpcradius_auth_server_cfg = (radius_auth_server_cfg) ? &__rpcradius_auth_server_cfg : NULL;
	__req.radius_auth_server_cfg = p__rpcradius_auth_server_cfg;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RADIUS_AUTH_SERVER_CFG_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_radius_auth_server_cfg_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_radius_auth_server_cfg call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (radius_auth_server_cfg && __resp.radius_auth_server_cfg)
			strcpy(radius_auth_server_cfg, __resp.radius_auth_server_cfg->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_radius_auth_server_cfg_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_own_ip_addr(const char * ifname, const string_16 own_ip_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_own_ip_addr_rpcdata __req;
	struct qcsapi_wifi_set_own_ip_addr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcown_ip_addr = {(char *)own_ip_addr};
	__rpc_string *p__rpcown_ip_addr = (own_ip_addr) ? &__rpcown_ip_addr : NULL;
	__req.own_ip_addr = p__rpcown_ip_addr;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OWN_IP_ADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_own_ip_addr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_own_ip_addr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_own_ip_addr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_own_ip_addr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_own_ip_addr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_key_passphrase(const char * ifname, const qcsapi_unsigned_int key_index, string_64 passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_key_passphrase_rpcdata __req;
	struct qcsapi_wifi_get_key_passphrase_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpassphrase = {(char *)passphrase};
	__rpc_string *p__rpcpassphrase = (passphrase) ? &__rpcpassphrase : NULL;
	__req.passphrase = p__rpcpassphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_KEY_PASSPHRASE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_key_passphrase_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_key_passphrase_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_key_passphrase call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_key_passphrase_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (passphrase && __resp.passphrase)
			strcpy(passphrase, __resp.passphrase->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_key_passphrase_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_key_passphrase(const char * ifname, const qcsapi_unsigned_int key_index, const string_64 passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_key_passphrase_rpcdata __req;
	struct qcsapi_wifi_set_key_passphrase_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpassphrase = {(char *)passphrase};
	__rpc_string *p__rpcpassphrase = (passphrase) ? &__rpcpassphrase : NULL;
	__req.passphrase = p__rpcpassphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_KEY_PASSPHRASE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_key_passphrase_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_key_passphrase_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_key_passphrase call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_key_passphrase_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_key_passphrase_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_group_key_interval(const char * ifname, string_16 group_key_interval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_group_key_interval_rpcdata __req;
	struct qcsapi_wifi_get_group_key_interval_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcgroup_key_interval = {(char *)group_key_interval};
	__rpc_string *p__rpcgroup_key_interval = (group_key_interval) ? &__rpcgroup_key_interval : NULL;
	__req.group_key_interval = p__rpcgroup_key_interval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_GROUP_KEY_INTERVAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_group_key_interval_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_group_key_interval_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_group_key_interval call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_group_key_interval_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (group_key_interval && __resp.group_key_interval)
			strcpy(group_key_interval, __resp.group_key_interval->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_group_key_interval_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_group_key_interval(const char * ifname, const string_16 group_key_interval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_group_key_interval_rpcdata __req;
	struct qcsapi_wifi_set_group_key_interval_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcgroup_key_interval = {(char *)group_key_interval};
	__rpc_string *p__rpcgroup_key_interval = (group_key_interval) ? &__rpcgroup_key_interval : NULL;
	__req.group_key_interval = p__rpcgroup_key_interval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_GROUP_KEY_INTERVAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_group_key_interval_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_group_key_interval_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_group_key_interval call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_group_key_interval_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_group_key_interval_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_pmf(const char * ifname, int * p_pmf_cap)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_pmf_rpcdata __req;
	struct qcsapi_wifi_get_pmf_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_pmf_cap = (int *)p_pmf_cap;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PMF_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_pmf_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_pmf_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_pmf call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pmf_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_pmf_cap)
			*p_pmf_cap = *__resp.p_pmf_cap;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pmf_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_pmf(const char * ifname, int pmf_cap)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_pmf_rpcdata __req;
	struct qcsapi_wifi_set_pmf_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.pmf_cap = (int)pmf_cap;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PMF_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_pmf_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_pmf_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_pmf call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pmf_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pmf_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_wpa_status(const char * ifname, char * wpa_status, const char * mac_addr, const qcsapi_unsigned_int max_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_wpa_status_rpcdata __req;
	struct qcsapi_wifi_get_wpa_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwpa_status = {(char *)wpa_status};
	__rpc_string *p__rpcwpa_status = (wpa_status) ? &__rpcwpa_status : NULL;
	__req.wpa_status = p__rpcwpa_status;

	__rpc_string __rpcmac_addr = {(char *)mac_addr};
	__rpc_string *p__rpcmac_addr = (mac_addr) ? &__rpcmac_addr : NULL;
	__req.mac_addr = p__rpcmac_addr;

	__req.max_len = (unsigned int)max_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_WPA_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_wpa_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_wpa_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_wpa_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_wpa_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wpa_status && __resp.wpa_status)
			strcpy(wpa_status, __resp.wpa_status->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_wpa_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_psk_auth_failures(const char * ifname, qcsapi_unsigned_int * count)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_psk_auth_failures_rpcdata __req;
	struct qcsapi_wifi_get_psk_auth_failures_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.count = (unsigned int *)count;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PSK_AUTH_FAILURES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_psk_auth_failures_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_psk_auth_failures_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_psk_auth_failures call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_psk_auth_failures_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (count)
			*count = *__resp.count;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_psk_auth_failures_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_auth_state(const char * ifname, const char * mac_addr, int * auth_state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_auth_state_rpcdata __req;
	struct qcsapi_wifi_get_auth_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcmac_addr = {(char *)mac_addr};
	__rpc_string *p__rpcmac_addr = (mac_addr) ? &__rpcmac_addr : NULL;
	__req.mac_addr = p__rpcmac_addr;

	__req.auth_state = (int *)auth_state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AUTH_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_auth_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_auth_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_auth_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_auth_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (auth_state)
			*auth_state = *__resp.auth_state;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_auth_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_security_defer_mode(const char * ifname, int defer)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_security_defer_mode_rpcdata __req;
	struct qcsapi_wifi_set_security_defer_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.defer = (int)defer;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SECURITY_DEFER_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_security_defer_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_security_defer_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_security_defer_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_security_defer_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_security_defer_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_security_defer_mode(const char * ifname, int * defer)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_security_defer_mode_rpcdata __req;
	struct qcsapi_wifi_get_security_defer_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.defer = (int *)defer;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SECURITY_DEFER_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_security_defer_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_security_defer_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_security_defer_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_security_defer_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (defer)
			*defer = *__resp.defer;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_security_defer_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_apply_security_config(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_apply_security_config_rpcdata __req;
	struct qcsapi_wifi_apply_security_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_APPLY_SECURITY_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_apply_security_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_apply_security_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_apply_security_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_apply_security_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_apply_security_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_mac_address_filtering(const char * ifname, const qcsapi_mac_address_filtering new_mac_address_filtering)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_mac_address_filtering_rpcdata __req;
	struct qcsapi_wifi_set_mac_address_filtering_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.new_mac_address_filtering = (int)new_mac_address_filtering;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_MAC_ADDRESS_FILTERING_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_mac_address_filtering_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_mac_address_filtering_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_mac_address_filtering call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mac_address_filtering_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mac_address_filtering_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mac_address_filtering(const char * ifname, qcsapi_mac_address_filtering * current_mac_address_filtering)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mac_address_filtering_rpcdata __req;
	struct qcsapi_wifi_get_mac_address_filtering_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.current_mac_address_filtering = (int *)current_mac_address_filtering;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MAC_ADDRESS_FILTERING_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mac_address_filtering_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mac_address_filtering_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mac_address_filtering call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mac_address_filtering_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_mac_address_filtering)
			*current_mac_address_filtering = *__resp.current_mac_address_filtering;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mac_address_filtering_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_authorize_mac_address(const char * ifname, const qcsapi_mac_addr address_to_authorize)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_authorize_mac_address_rpcdata __req;
	struct qcsapi_wifi_authorize_mac_address_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcaddress_to_authorize;
	if (address_to_authorize) {
		memcpy(__rpcaddress_to_authorize.data, address_to_authorize, sizeof(__rpcaddress_to_authorize));
		__req.address_to_authorize = &__rpcaddress_to_authorize;
	} else {
		__req.address_to_authorize = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_AUTHORIZE_MAC_ADDRESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_authorize_mac_address_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_authorize_mac_address_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_authorize_mac_address call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_authorize_mac_address_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_authorize_mac_address_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_deny_mac_address(const char * ifname, const qcsapi_mac_addr address_to_deny)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_deny_mac_address_rpcdata __req;
	struct qcsapi_wifi_deny_mac_address_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcaddress_to_deny;
	if (address_to_deny) {
		memcpy(__rpcaddress_to_deny.data, address_to_deny, sizeof(__rpcaddress_to_deny));
		__req.address_to_deny = &__rpcaddress_to_deny;
	} else {
		__req.address_to_deny = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DENY_MAC_ADDRESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_deny_mac_address_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_deny_mac_address_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_deny_mac_address call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_deny_mac_address_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_deny_mac_address_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_remove_mac_address(const char * ifname, const qcsapi_mac_addr address_to_remove)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_remove_mac_address_rpcdata __req;
	struct qcsapi_wifi_remove_mac_address_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcaddress_to_remove;
	if (address_to_remove) {
		memcpy(__rpcaddress_to_remove.data, address_to_remove, sizeof(__rpcaddress_to_remove));
		__req.address_to_remove = &__rpcaddress_to_remove;
	} else {
		__req.address_to_remove = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_REMOVE_MAC_ADDRESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_remove_mac_address_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_remove_mac_address_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_remove_mac_address call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_remove_mac_address_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_remove_mac_address_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_is_mac_address_authorized(const char * ifname, const qcsapi_mac_addr address_to_verify, int * p_mac_address_authorized)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_is_mac_address_authorized_rpcdata __req;
	struct qcsapi_wifi_is_mac_address_authorized_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcaddress_to_verify;
	if (address_to_verify) {
		memcpy(__rpcaddress_to_verify.data, address_to_verify, sizeof(__rpcaddress_to_verify));
		__req.address_to_verify = &__rpcaddress_to_verify;
	} else {
		__req.address_to_verify = NULL;
	}
	__req.p_mac_address_authorized = (int *)p_mac_address_authorized;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_IS_MAC_ADDRESS_AUTHORIZED_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_is_mac_address_authorized_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_is_mac_address_authorized_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_is_mac_address_authorized call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_is_mac_address_authorized_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_mac_address_authorized)
			*p_mac_address_authorized = *__resp.p_mac_address_authorized;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_is_mac_address_authorized_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_authorized_mac_addresses(const char * ifname, char * list_mac_addresses, const unsigned int sizeof_list)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_authorized_mac_addresses_rpcdata __req;
	struct qcsapi_wifi_get_authorized_mac_addresses_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclist_mac_addresses = {(char *)list_mac_addresses};
	__rpc_string *p__rpclist_mac_addresses = (list_mac_addresses) ? &__rpclist_mac_addresses : NULL;
	__req.list_mac_addresses = p__rpclist_mac_addresses;

	__req.sizeof_list = (unsigned int)sizeof_list;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AUTHORIZED_MAC_ADDRESSES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_authorized_mac_addresses_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_authorized_mac_addresses_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_authorized_mac_addresses call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_authorized_mac_addresses_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_mac_addresses && __resp.list_mac_addresses)
			strcpy(list_mac_addresses, __resp.list_mac_addresses->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_authorized_mac_addresses_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_denied_mac_addresses(const char * ifname, char * list_mac_addresses, const unsigned int sizeof_list)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_denied_mac_addresses_rpcdata __req;
	struct qcsapi_wifi_get_denied_mac_addresses_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpclist_mac_addresses = {(char *)list_mac_addresses};
	__rpc_string *p__rpclist_mac_addresses = (list_mac_addresses) ? &__rpclist_mac_addresses : NULL;
	__req.list_mac_addresses = p__rpclist_mac_addresses;

	__req.sizeof_list = (unsigned int)sizeof_list;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DENIED_MAC_ADDRESSES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_denied_mac_addresses_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_denied_mac_addresses_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_denied_mac_addresses call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_denied_mac_addresses_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_mac_addresses && __resp.list_mac_addresses)
			strcpy(list_mac_addresses, __resp.list_mac_addresses->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_denied_mac_addresses_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_accept_oui_filter(const char * ifname, const qcsapi_mac_addr oui, int flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_accept_oui_filter_rpcdata __req;
	struct qcsapi_wifi_set_accept_oui_filter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcoui;
	if (oui) {
		memcpy(__rpcoui.data, oui, sizeof(__rpcoui));
		__req.oui = &__rpcoui;
	} else {
		__req.oui = NULL;
	}
	__req.flag = (int)flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_ACCEPT_OUI_FILTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_accept_oui_filter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_accept_oui_filter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_accept_oui_filter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_accept_oui_filter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_accept_oui_filter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_accept_oui_filter(const char * ifname, char * oui_list, const unsigned int sizeof_list)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_accept_oui_filter_rpcdata __req;
	struct qcsapi_wifi_get_accept_oui_filter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcoui_list = {(char *)oui_list};
	__rpc_string *p__rpcoui_list = (oui_list) ? &__rpcoui_list : NULL;
	__req.oui_list = p__rpcoui_list;

	__req.sizeof_list = (unsigned int)sizeof_list;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ACCEPT_OUI_FILTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_accept_oui_filter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_accept_oui_filter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_accept_oui_filter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_accept_oui_filter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (oui_list && __resp.oui_list)
			strcpy(oui_list, __resp.oui_list->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_accept_oui_filter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_clear_mac_address_filters(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_clear_mac_address_filters_rpcdata __req;
	struct qcsapi_wifi_clear_mac_address_filters_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_CLEAR_MAC_ADDRESS_FILTERS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_filters_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_filters_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_clear_mac_address_filters call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_filters_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_filters_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_mac_address_reserve(const char * ifname, const char * addr, const char * mask)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_mac_address_reserve_rpcdata __req;
	struct qcsapi_wifi_set_mac_address_reserve_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcaddr = {(char *)addr};
	__rpc_string *p__rpcaddr = (addr) ? &__rpcaddr : NULL;
	__req.addr = p__rpcaddr;

	__rpc_string __rpcmask = {(char *)mask};
	__rpc_string *p__rpcmask = (mask) ? &__rpcmask : NULL;
	__req.mask = p__rpcmask;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_MAC_ADDRESS_RESERVE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_mac_address_reserve_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_mac_address_reserve_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_mac_address_reserve call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mac_address_reserve_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mac_address_reserve_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mac_address_reserve(const char * ifname, string_256 buf)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mac_address_reserve_rpcdata __req;
	struct qcsapi_wifi_get_mac_address_reserve_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcbuf = {(char *)buf};
	__rpc_string *p__rpcbuf = (buf) ? &__rpcbuf : NULL;
	__req.buf = p__rpcbuf;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MAC_ADDRESS_RESERVE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mac_address_reserve_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mac_address_reserve_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mac_address_reserve call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mac_address_reserve_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (buf && __resp.buf)
			strcpy(buf, __resp.buf->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mac_address_reserve_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_clear_mac_address_reserve(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_clear_mac_address_reserve_rpcdata __req;
	struct qcsapi_wifi_clear_mac_address_reserve_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_CLEAR_MAC_ADDRESS_RESERVE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_reserve_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_reserve_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_clear_mac_address_reserve call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_reserve_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_clear_mac_address_reserve_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_option(const char * ifname, qcsapi_option_type qcsapi_option, int * p_current_option)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_option_rpcdata __req;
	struct qcsapi_wifi_get_option_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.qcsapi_option = (int)qcsapi_option;

	__req.p_current_option = (int *)p_current_option;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_OPTION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_option_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_option_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_option call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_option_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_current_option)
			*p_current_option = *__resp.p_current_option;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_option_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_option(const char * ifname, qcsapi_option_type qcsapi_option, int new_option)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_option_rpcdata __req;
	struct qcsapi_wifi_set_option_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.qcsapi_option = (int)qcsapi_option;

	__req.new_option = (int)new_option;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_OPTION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_option_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_option_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_option call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_option_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_option_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_board_parameter(qcsapi_board_parameter_type board_param, string_64 p_buffer)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_board_parameter_rpcdata __req;
	struct qcsapi_get_board_parameter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.board_param = (int)board_param;

	__rpc_string __rpcp_buffer = {(char *)p_buffer};
	__rpc_string *p__rpcp_buffer = (p_buffer) ? &__rpcp_buffer : NULL;
	__req.p_buffer = p__rpcp_buffer;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_BOARD_PARAMETER_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_board_parameter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_board_parameter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_board_parameter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_board_parameter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_buffer && __resp.p_buffer)
			strcpy(p_buffer, __resp.p_buffer->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_board_parameter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_swfeat_list(string_4096 p_buffer)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_swfeat_list_rpcdata __req;
	struct qcsapi_get_swfeat_list_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcp_buffer = {(char *)p_buffer};
	__rpc_string *p__rpcp_buffer = (p_buffer) ? &__rpcp_buffer : NULL;
	__req.p_buffer = p__rpcp_buffer;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_SWFEAT_LIST_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_swfeat_list_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_swfeat_list_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_swfeat_list call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_swfeat_list_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_buffer && __resp.p_buffer)
			strcpy(p_buffer, __resp.p_buffer->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_swfeat_list_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_create_SSID(const char * ifname, const qcsapi_SSID new_SSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_create_SSID_rpcdata __req;
	struct qcsapi_SSID_create_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcnew_SSID = {(char *)new_SSID};
	__rpc_string *p__rpcnew_SSID = (new_SSID) ? &__rpcnew_SSID : NULL;
	__req.new_SSID = p__rpcnew_SSID;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_CREATE_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_create_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_create_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_create_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_create_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_create_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_remove_SSID(const char * ifname, const qcsapi_SSID del_SSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_remove_SSID_rpcdata __req;
	struct qcsapi_SSID_remove_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcdel_SSID = {(char *)del_SSID};
	__rpc_string *p__rpcdel_SSID = (del_SSID) ? &__rpcdel_SSID : NULL;
	__req.del_SSID = p__rpcdel_SSID;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_REMOVE_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_remove_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_remove_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_remove_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_remove_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_remove_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_verify_SSID(const char * ifname, const qcsapi_SSID current_SSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_verify_SSID_rpcdata __req;
	struct qcsapi_SSID_verify_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_VERIFY_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_verify_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_verify_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_verify_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_verify_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_verify_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_rename_SSID(const char * ifname, const qcsapi_SSID current_SSID, const qcsapi_SSID new_SSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_rename_SSID_rpcdata __req;
	struct qcsapi_SSID_rename_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcnew_SSID = {(char *)new_SSID};
	__rpc_string *p__rpcnew_SSID = (new_SSID) ? &__rpcnew_SSID : NULL;
	__req.new_SSID = p__rpcnew_SSID;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_RENAME_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_rename_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_rename_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_rename_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_rename_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_rename_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_SSID_list(const char * ifname, const unsigned int arrayc, char ** list_SSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_SSID_list_rpcdata __req;
	struct qcsapi_SSID_get_SSID_list_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (list_SSID == NULL) {
		return -EFAULT;
	}
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.arrayc = (unsigned int)arrayc;

	/* TODO: string array member - list_SSID */
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_SSID_LIST_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_SSID_list_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_SSID_list_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_SSID_list call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_SSID_list_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		unsigned int i;
		for (i = 0; i < __resp.list_SSID.list_SSID_len; i++) {
			strcpy(list_SSID[i], __resp.list_SSID.list_SSID_val[i]);
		}
		list_SSID[i] = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_SSID_list_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_protocol(const char * ifname, const qcsapi_SSID current_SSID, const char * new_protocol)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_protocol_rpcdata __req;
	struct qcsapi_SSID_set_protocol_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcnew_protocol = {(char *)new_protocol};
	__rpc_string *p__rpcnew_protocol = (new_protocol) ? &__rpcnew_protocol : NULL;
	__req.new_protocol = p__rpcnew_protocol;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_PROTOCOL_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_protocol_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_protocol_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_protocol call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_protocol_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_protocol_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_protocol(const char * ifname, const qcsapi_SSID current_SSID, string_16 current_protocol)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_protocol_rpcdata __req;
	struct qcsapi_SSID_get_protocol_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpccurrent_protocol = {(char *)current_protocol};
	__rpc_string *p__rpccurrent_protocol = (current_protocol) ? &__rpccurrent_protocol : NULL;
	__req.current_protocol = p__rpccurrent_protocol;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_PROTOCOL_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_protocol_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_protocol_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_protocol call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_protocol_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_protocol && __resp.current_protocol)
			strcpy(current_protocol, __resp.current_protocol->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_protocol_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_encryption_modes(const char * ifname, const qcsapi_SSID current_SSID, string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_encryption_modes_rpcdata __req;
	struct qcsapi_SSID_get_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (encryption_modes && __resp.encryption_modes)
			strcpy(encryption_modes, __resp.encryption_modes->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_encryption_modes(const char * ifname, const qcsapi_SSID current_SSID, const string_32 encryption_modes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_encryption_modes_rpcdata __req;
	struct qcsapi_SSID_set_encryption_modes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcencryption_modes = {(char *)encryption_modes};
	__rpc_string *p__rpcencryption_modes = (encryption_modes) ? &__rpcencryption_modes : NULL;
	__req.encryption_modes = p__rpcencryption_modes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_ENCRYPTION_MODES_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_encryption_modes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_encryption_modes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_encryption_modes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_encryption_modes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_encryption_modes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_group_encryption(const char * ifname, const qcsapi_SSID current_SSID, string_32 encryption_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_group_encryption_rpcdata __req;
	struct qcsapi_SSID_get_group_encryption_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcencryption_mode = {(char *)encryption_mode};
	__rpc_string *p__rpcencryption_mode = (encryption_mode) ? &__rpcencryption_mode : NULL;
	__req.encryption_mode = p__rpcencryption_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_GROUP_ENCRYPTION_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_group_encryption_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_group_encryption_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_group_encryption call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_group_encryption_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (encryption_mode && __resp.encryption_mode)
			strcpy(encryption_mode, __resp.encryption_mode->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_group_encryption_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_group_encryption(const char * ifname, const qcsapi_SSID current_SSID, const string_32 encryption_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_group_encryption_rpcdata __req;
	struct qcsapi_SSID_set_group_encryption_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcencryption_mode = {(char *)encryption_mode};
	__rpc_string *p__rpcencryption_mode = (encryption_mode) ? &__rpcencryption_mode : NULL;
	__req.encryption_mode = p__rpcencryption_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_GROUP_ENCRYPTION_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_group_encryption_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_group_encryption_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_group_encryption call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_group_encryption_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_group_encryption_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_authentication_mode(const char * ifname, const qcsapi_SSID current_SSID, string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_authentication_mode_rpcdata __req;
	struct qcsapi_SSID_get_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (authentication_mode && __resp.authentication_mode)
			strcpy(authentication_mode, __resp.authentication_mode->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_authentication_mode(const char * ifname, const qcsapi_SSID current_SSID, const string_32 authentication_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_authentication_mode_rpcdata __req;
	struct qcsapi_SSID_set_authentication_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__rpc_string __rpcauthentication_mode = {(char *)authentication_mode};
	__rpc_string *p__rpcauthentication_mode = (authentication_mode) ? &__rpcauthentication_mode : NULL;
	__req.authentication_mode = p__rpcauthentication_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_AUTHENTICATION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_authentication_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_authentication_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_authentication_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_authentication_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_authentication_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_pre_shared_key(const char * ifname, const qcsapi_SSID current_SSID, const qcsapi_unsigned_int key_index, string_64 pre_shared_key)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_pre_shared_key_rpcdata __req;
	struct qcsapi_SSID_get_pre_shared_key_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpre_shared_key = {(char *)pre_shared_key};
	__rpc_string *p__rpcpre_shared_key = (pre_shared_key) ? &__rpcpre_shared_key : NULL;
	__req.pre_shared_key = p__rpcpre_shared_key;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_PRE_SHARED_KEY_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_pre_shared_key_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_pre_shared_key_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_pre_shared_key call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_pre_shared_key_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (pre_shared_key && __resp.pre_shared_key)
			strcpy(pre_shared_key, __resp.pre_shared_key->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_pre_shared_key_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_pre_shared_key(const char * ifname, const qcsapi_SSID current_SSID, const qcsapi_unsigned_int key_index, const string_64 pre_shared_key)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_pre_shared_key_rpcdata __req;
	struct qcsapi_SSID_set_pre_shared_key_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpre_shared_key = {(char *)pre_shared_key};
	__rpc_string *p__rpcpre_shared_key = (pre_shared_key) ? &__rpcpre_shared_key : NULL;
	__req.pre_shared_key = p__rpcpre_shared_key;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_PRE_SHARED_KEY_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_pre_shared_key_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_pre_shared_key_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_pre_shared_key call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_pre_shared_key_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_pre_shared_key_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_key_passphrase(const char * ifname, const qcsapi_SSID current_SSID, const qcsapi_unsigned_int key_index, string_64 passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_key_passphrase_rpcdata __req;
	struct qcsapi_SSID_get_key_passphrase_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpassphrase = {(char *)passphrase};
	__rpc_string *p__rpcpassphrase = (passphrase) ? &__rpcpassphrase : NULL;
	__req.passphrase = p__rpcpassphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_KEY_PASSPHRASE_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_key_passphrase_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_key_passphrase_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_key_passphrase call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_key_passphrase_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (passphrase && __resp.passphrase)
			strcpy(passphrase, __resp.passphrase->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_key_passphrase_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_key_passphrase(const char * ifname, const qcsapi_SSID current_SSID, const qcsapi_unsigned_int key_index, const string_64 passphrase)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_key_passphrase_rpcdata __req;
	struct qcsapi_SSID_set_key_passphrase_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__req.key_index = (unsigned int)key_index;

	__rpc_string __rpcpassphrase = {(char *)passphrase};
	__rpc_string *p__rpcpassphrase = (passphrase) ? &__rpcpassphrase : NULL;
	__req.passphrase = p__rpcpassphrase;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_KEY_PASSPHRASE_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_key_passphrase_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_key_passphrase_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_key_passphrase call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_key_passphrase_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_key_passphrase_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_pmf(const char * ifname, const qcsapi_SSID current_SSID, int * p_pmf_cap)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_pmf_rpcdata __req;
	struct qcsapi_SSID_get_pmf_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_SSID = {(char *)current_SSID};
	__rpc_string *p__rpccurrent_SSID = (current_SSID) ? &__rpccurrent_SSID : NULL;
	__req.current_SSID = p__rpccurrent_SSID;

	__req.p_pmf_cap = (int *)p_pmf_cap;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_PMF_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_pmf_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_pmf_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_pmf call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_pmf_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_pmf_cap)
			*p_pmf_cap = *__resp.p_pmf_cap;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_pmf_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_set_pmf(const char * ifname, const qcsapi_SSID SSID_str, int pmf_cap)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_set_pmf_rpcdata __req;
	struct qcsapi_SSID_set_pmf_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcSSID_str = {(char *)SSID_str};
	__rpc_string *p__rpcSSID_str = (SSID_str) ? &__rpcSSID_str : NULL;
	__req.SSID_str = p__rpcSSID_str;

	__req.pmf_cap = (int)pmf_cap;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_SET_PMF_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_set_pmf_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_set_pmf_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_set_pmf call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_pmf_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_set_pmf_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_SSID_get_wps_SSID(const char * ifname, qcsapi_SSID wps_SSID)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_SSID_get_wps_SSID_rpcdata __req;
	struct qcsapi_SSID_get_wps_SSID_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_SSID = {(char *)wps_SSID};
	__rpc_string *p__rpcwps_SSID = (wps_SSID) ? &__rpcwps_SSID : NULL;
	__req.wps_SSID = p__rpcwps_SSID;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SSID_GET_WPS_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_SSID_get_wps_SSID_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_SSID_get_wps_SSID_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_SSID_get_wps_SSID call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_wps_SSID_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_SSID && __resp.wps_SSID)
			strcpy(wps_SSID, __resp.wps_SSID->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_SSID_get_wps_SSID_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_vlan_config(const char * ifname, qcsapi_vlan_cmd cmd, uint32_t vlanid, uint32_t flags)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_vlan_config_rpcdata __req;
	struct qcsapi_wifi_vlan_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.cmd = (int)cmd;

	__req.vlanid = (uint32_t)vlanid;

	__req.flags = (uint32_t)flags;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_VLAN_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_vlan_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_vlan_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_vlan_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_vlan_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_vlan_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_show_vlan_config(const char * ifname, string_1024 vcfg)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_show_vlan_config_rpcdata __req;
	struct qcsapi_wifi_show_vlan_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcvcfg = {(char *)vcfg};
	__rpc_string *p__rpcvcfg = (vcfg) ? &__rpcvcfg : NULL;
	__req.vcfg = p__rpcvcfg;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SHOW_VLAN_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_show_vlan_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_show_vlan_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_show_vlan_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_show_vlan_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (vcfg && __resp.vcfg)
			strcpy(vcfg, __resp.vcfg->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_show_vlan_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_enable_vlan_pass_through(const char * ifname, int enabled)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_enable_vlan_pass_through_rpcdata __req;
	struct qcsapi_enable_vlan_pass_through_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enabled = (int)enabled;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_ENABLE_VLAN_PASS_THROUGH_REMOTE,
				(xdrproc_t)xdr_qcsapi_enable_vlan_pass_through_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_enable_vlan_pass_through_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_enable_vlan_pass_through call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_enable_vlan_pass_through_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_enable_vlan_pass_through_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_vlan_promisc(int enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_vlan_promisc_rpcdata __req;
	struct qcsapi_wifi_set_vlan_promisc_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.enable = (int)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_VLAN_PROMISC_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_vlan_promisc_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_vlan_promisc_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_vlan_promisc call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_vlan_promisc_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_vlan_promisc_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_registrar_report_button_press(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_registrar_report_button_press_rpcdata __req;
	struct qcsapi_wps_registrar_report_button_press_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_REGISTRAR_REPORT_BUTTON_PRESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_registrar_report_button_press_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_registrar_report_button_press_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_registrar_report_button_press call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_report_button_press_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_report_button_press_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_registrar_report_pin(const char * ifname, const char * wps_pin)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_registrar_report_pin_rpcdata __req;
	struct qcsapi_wps_registrar_report_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_pin = {(char *)wps_pin};
	__rpc_string *p__rpcwps_pin = (wps_pin) ? &__rpcwps_pin : NULL;
	__req.wps_pin = p__rpcwps_pin;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_REGISTRAR_REPORT_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_registrar_report_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_registrar_report_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_registrar_report_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_report_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_report_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_registrar_get_pp_devname(const char * ifname, int blacklist, string_128 pp_devname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_registrar_get_pp_devname_rpcdata __req;
	struct qcsapi_wps_registrar_get_pp_devname_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.blacklist = (int)blacklist;

	__rpc_string __rpcpp_devname = {(char *)pp_devname};
	__rpc_string *p__rpcpp_devname = (pp_devname) ? &__rpcpp_devname : NULL;
	__req.pp_devname = p__rpcpp_devname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_REGISTRAR_GET_PP_DEVNAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_registrar_get_pp_devname_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_registrar_get_pp_devname_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_registrar_get_pp_devname call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_get_pp_devname_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (pp_devname && __resp.pp_devname)
			strcpy(pp_devname, __resp.pp_devname->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_get_pp_devname_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_registrar_set_pp_devname(const char * ifname, int update_blacklist, const string_256 pp_devname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_registrar_set_pp_devname_rpcdata __req;
	struct qcsapi_wps_registrar_set_pp_devname_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.update_blacklist = (int)update_blacklist;

	__rpc_string __rpcpp_devname = {(char *)pp_devname};
	__rpc_string *p__rpcpp_devname = (pp_devname) ? &__rpcpp_devname : NULL;
	__req.pp_devname = p__rpcpp_devname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_REGISTRAR_SET_PP_DEVNAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_registrar_set_pp_devname_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_registrar_set_pp_devname_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_registrar_set_pp_devname call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_set_pp_devname_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_registrar_set_pp_devname_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_enrollee_report_button_press(const char * ifname, const qcsapi_mac_addr bssid)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_enrollee_report_button_press_rpcdata __req;
	struct qcsapi_wps_enrollee_report_button_press_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcbssid;
	if (bssid) {
		memcpy(__rpcbssid.data, bssid, sizeof(__rpcbssid));
		__req.bssid = &__rpcbssid;
	} else {
		__req.bssid = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ENROLLEE_REPORT_BUTTON_PRESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_enrollee_report_button_press_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_enrollee_report_button_press_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_enrollee_report_button_press call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_enrollee_report_button_press_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_enrollee_report_button_press_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_enrollee_report_pin(const char * ifname, const qcsapi_mac_addr bssid, const char * wps_pin)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_enrollee_report_pin_rpcdata __req;
	struct qcsapi_wps_enrollee_report_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcbssid;
	if (bssid) {
		memcpy(__rpcbssid.data, bssid, sizeof(__rpcbssid));
		__req.bssid = &__rpcbssid;
	} else {
		__req.bssid = NULL;
	}
	__rpc_string __rpcwps_pin = {(char *)wps_pin};
	__rpc_string *p__rpcwps_pin = (wps_pin) ? &__rpcwps_pin : NULL;
	__req.wps_pin = p__rpcwps_pin;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ENROLLEE_REPORT_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_enrollee_report_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_enrollee_report_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_enrollee_report_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_enrollee_report_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_enrollee_report_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_enrollee_generate_pin(const char * ifname, const qcsapi_mac_addr bssid, char * wps_pin)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_enrollee_generate_pin_rpcdata __req;
	struct qcsapi_wps_enrollee_generate_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	struct __rpc_qcsapi_mac_addr __rpcbssid;
	if (bssid) {
		memcpy(__rpcbssid.data, bssid, sizeof(__rpcbssid));
		__req.bssid = &__rpcbssid;
	} else {
		__req.bssid = NULL;
	}
	__rpc_string __rpcwps_pin = {(char *)wps_pin};
	__rpc_string *p__rpcwps_pin = (wps_pin) ? &__rpcwps_pin : NULL;
	__req.wps_pin = p__rpcwps_pin;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ENROLLEE_GENERATE_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_enrollee_generate_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_enrollee_generate_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_enrollee_generate_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_enrollee_generate_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_pin && __resp.wps_pin)
			strcpy(wps_pin, __resp.wps_pin->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_enrollee_generate_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_ap_pin(const char * ifname, char * wps_pin, int force_regenerate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_ap_pin_rpcdata __req;
	struct qcsapi_wps_get_ap_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_pin = {(char *)wps_pin};
	__rpc_string *p__rpcwps_pin = (wps_pin) ? &__rpcwps_pin : NULL;
	__req.wps_pin = p__rpcwps_pin;

	__req.force_regenerate = (int)force_regenerate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_AP_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_ap_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_ap_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_ap_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_ap_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_pin && __resp.wps_pin)
			strcpy(wps_pin, __resp.wps_pin->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_ap_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_set_ap_pin(const char * ifname, const char * wps_pin)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_set_ap_pin_rpcdata __req;
	struct qcsapi_wps_set_ap_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_pin = {(char *)wps_pin};
	__rpc_string *p__rpcwps_pin = (wps_pin) ? &__rpcwps_pin : NULL;
	__req.wps_pin = p__rpcwps_pin;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SET_AP_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_set_ap_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_set_ap_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_set_ap_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_set_ap_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_set_ap_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_save_ap_pin(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_save_ap_pin_rpcdata __req;
	struct qcsapi_wps_save_ap_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SAVE_AP_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_save_ap_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_save_ap_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_save_ap_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_save_ap_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_save_ap_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_enable_ap_pin(const char * ifname, int enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_enable_ap_pin_rpcdata __req;
	struct qcsapi_wps_enable_ap_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (int)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ENABLE_AP_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_enable_ap_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_enable_ap_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_enable_ap_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_enable_ap_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_enable_ap_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_sta_pin(const char * ifname, char * wps_pin)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_sta_pin_rpcdata __req;
	struct qcsapi_wps_get_sta_pin_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_pin = {(char *)wps_pin};
	__rpc_string *p__rpcwps_pin = (wps_pin) ? &__rpcwps_pin : NULL;
	__req.wps_pin = p__rpcwps_pin;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_STA_PIN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_sta_pin_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_sta_pin_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_sta_pin call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_sta_pin_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_pin && __resp.wps_pin)
			strcpy(wps_pin, __resp.wps_pin->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_sta_pin_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_state(const char * ifname, char * wps_state, const qcsapi_unsigned_int max_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_state_rpcdata __req;
	struct qcsapi_wps_get_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_state = {(char *)wps_state};
	__rpc_string *p__rpcwps_state = (wps_state) ? &__rpcwps_state : NULL;
	__req.wps_state = p__rpcwps_state;

	__req.max_len = (unsigned int)max_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_state && __resp.wps_state)
			strcpy(wps_state, __resp.wps_state->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_configured_state(const char * ifname, char * wps_state, const qcsapi_unsigned_int max_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_configured_state_rpcdata __req;
	struct qcsapi_wps_get_configured_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcwps_state = {(char *)wps_state};
	__rpc_string *p__rpcwps_state = (wps_state) ? &__rpcwps_state : NULL;
	__req.wps_state = p__rpcwps_state;

	__req.max_len = (unsigned int)max_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_CONFIGURED_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_configured_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_configured_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_configured_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_configured_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_state && __resp.wps_state)
			strcpy(wps_state, __resp.wps_state->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_configured_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_runtime_state(const char * ifname, char * state, int max_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_runtime_state_rpcdata __req;
	struct qcsapi_wps_get_runtime_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcstate = {(char *)state};
	__rpc_string *p__rpcstate = (state) ? &__rpcstate : NULL;
	__req.state = p__rpcstate;

	__req.max_len = (int)max_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_RUNTIME_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_runtime_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_runtime_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_runtime_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_runtime_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (state && __resp.state)
			strcpy(state, __resp.state->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_runtime_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_set_configured_state(const char * ifname, const qcsapi_unsigned_int state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_set_configured_state_rpcdata __req;
	struct qcsapi_wps_set_configured_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.state = (unsigned int)state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SET_CONFIGURED_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_set_configured_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_set_configured_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_set_configured_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_set_configured_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_set_configured_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_param(const char * ifname, qcsapi_wps_param_type wps_type, char * wps_str, const qcsapi_unsigned_int max_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_param_rpcdata __req;
	struct qcsapi_wps_get_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.wps_type = (int)wps_type;

	__rpc_string __rpcwps_str = {(char *)wps_str};
	__rpc_string *p__rpcwps_str = (wps_str) ? &__rpcwps_str : NULL;
	__req.wps_str = p__rpcwps_str;

	__req.max_len = (unsigned int)max_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (wps_str && __resp.wps_str)
			strcpy(wps_str, __resp.wps_str->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_set_timeout(const char * ifname, const int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_set_timeout_rpcdata __req;
	struct qcsapi_wps_set_timeout_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.value = (int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SET_TIMEOUT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_set_timeout_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_set_timeout_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_set_timeout call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_set_timeout_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_set_timeout_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_on_hidden_ssid(const char * ifname, const int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_on_hidden_ssid_rpcdata __req;
	struct qcsapi_wps_on_hidden_ssid_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.value = (int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ON_HIDDEN_SSID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_on_hidden_ssid call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_on_hidden_ssid_status(const char * ifname, char * state, int max_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_on_hidden_ssid_status_rpcdata __req;
	struct qcsapi_wps_on_hidden_ssid_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcstate = {(char *)state};
	__rpc_string *p__rpcstate = (state) ? &__rpcstate : NULL;
	__req.state = p__rpcstate;

	__req.max_len = (int)max_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ON_HIDDEN_SSID_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_on_hidden_ssid_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (state && __resp.state)
			strcpy(state, __resp.state->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_on_hidden_ssid_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_upnp_enable(const char * ifname, const int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_upnp_enable_rpcdata __req;
	struct qcsapi_wps_upnp_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.value = (int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_UPNP_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_upnp_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_upnp_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_upnp_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_upnp_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_upnp_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_upnp_status(const char * ifname, char * reply, int reply_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_upnp_status_rpcdata __req;
	struct qcsapi_wps_upnp_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcreply = {(char *)reply};
	__rpc_string *p__rpcreply = (reply) ? &__rpcreply : NULL;
	__req.reply = p__rpcreply;

	__req.reply_len = (int)reply_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_UPNP_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_upnp_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_upnp_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_upnp_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_upnp_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (reply && __resp.reply)
			strcpy(reply, __resp.reply->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_upnp_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_allow_pbc_overlap(const char * ifname, const qcsapi_unsigned_int allow)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_allow_pbc_overlap_rpcdata __req;
	struct qcsapi_wps_allow_pbc_overlap_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.allow = (unsigned int)allow;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_ALLOW_PBC_OVERLAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_allow_pbc_overlap_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_allow_pbc_overlap_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_allow_pbc_overlap call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_allow_pbc_overlap_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_allow_pbc_overlap_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_allow_pbc_overlap_status(const char * ifname, int * status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_allow_pbc_overlap_status_rpcdata __req;
	struct qcsapi_wps_get_allow_pbc_overlap_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.status = (int *)status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_ALLOW_PBC_OVERLAP_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_allow_pbc_overlap_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_allow_pbc_overlap_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_allow_pbc_overlap_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_allow_pbc_overlap_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (status)
			*status = *__resp.status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_allow_pbc_overlap_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_set_access_control(const char * ifname, uint32_t ctrl_state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_set_access_control_rpcdata __req;
	struct qcsapi_wps_set_access_control_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ctrl_state = (uint32_t)ctrl_state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SET_ACCESS_CONTROL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_set_access_control_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_set_access_control_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_set_access_control call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_set_access_control_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_set_access_control_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_access_control(const char * ifname, uint32_t * ctrl_state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_access_control_rpcdata __req;
	struct qcsapi_wps_get_access_control_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ctrl_state = (uint32_t *)ctrl_state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_ACCESS_CONTROL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_access_control_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_access_control_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_access_control call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_access_control_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ctrl_state)
			*ctrl_state = *__resp.ctrl_state;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_access_control_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_set_param(const char * ifname, const qcsapi_wps_param_type param_type, const char * param_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_set_param_rpcdata __req;
	struct qcsapi_wps_set_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.param_type = (int)param_type;

	__rpc_string __rpcparam_value = {(char *)param_value};
	__rpc_string *p__rpcparam_value = (param_value) ? &__rpcparam_value : NULL;
	__req.param_value = p__rpcparam_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SET_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_set_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_set_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_set_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_set_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_set_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_cancel(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_cancel_rpcdata __req;
	struct qcsapi_wps_cancel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_CANCEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_cancel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_cancel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_cancel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_cancel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_cancel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_set_pbc_in_srcm(const char * ifname, const qcsapi_unsigned_int enabled)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_set_pbc_in_srcm_rpcdata __req;
	struct qcsapi_wps_set_pbc_in_srcm_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enabled = (unsigned int)enabled;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_SET_PBC_IN_SRCM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_set_pbc_in_srcm_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_set_pbc_in_srcm_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_set_pbc_in_srcm call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_set_pbc_in_srcm_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_set_pbc_in_srcm_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wps_get_pbc_in_srcm(const char * ifname, qcsapi_unsigned_int * p_enabled)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wps_get_pbc_in_srcm_rpcdata __req;
	struct qcsapi_wps_get_pbc_in_srcm_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_enabled = (unsigned int *)p_enabled;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WPS_GET_PBC_IN_SRCM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wps_get_pbc_in_srcm_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wps_get_pbc_in_srcm_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wps_get_pbc_in_srcm call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wps_get_pbc_in_srcm_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_enabled)
			*p_enabled = *__resp.p_enabled;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wps_get_pbc_in_srcm_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_registrar_set_default_pbc_bss(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_registrar_set_default_pbc_bss_rpcdata __req;
	struct qcsapi_registrar_set_default_pbc_bss_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGISTRAR_SET_DEFAULT_PBC_BSS_REMOTE,
				(xdrproc_t)xdr_qcsapi_registrar_set_default_pbc_bss_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_registrar_set_default_pbc_bss_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_registrar_set_default_pbc_bss call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_registrar_set_default_pbc_bss_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_registrar_set_default_pbc_bss_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_registrar_get_default_pbc_bss(char * default_bss, int len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_registrar_get_default_pbc_bss_rpcdata __req;
	struct qcsapi_registrar_get_default_pbc_bss_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcdefault_bss = {(char *)default_bss};
	__rpc_string *p__rpcdefault_bss = (default_bss) ? &__rpcdefault_bss : NULL;
	__req.default_bss = p__rpcdefault_bss;

	__req.len = (int)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGISTRAR_GET_DEFAULT_PBC_BSS_REMOTE,
				(xdrproc_t)xdr_qcsapi_registrar_get_default_pbc_bss_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_registrar_get_default_pbc_bss_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_registrar_get_default_pbc_bss call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_registrar_get_default_pbc_bss_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (default_bss && __resp.default_bss)
			strcpy(default_bss, __resp.default_bss->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_registrar_get_default_pbc_bss_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_gpio_set_config(const uint8_t gpio_pin, const qcsapi_gpio_config new_gpio_config)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_gpio_set_config_rpcdata __req;
	struct qcsapi_gpio_set_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.gpio_pin = (uint8_t)gpio_pin;

	__req.new_gpio_config = (int)new_gpio_config;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GPIO_SET_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_gpio_set_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_gpio_set_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_gpio_set_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_gpio_set_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_gpio_set_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_gpio_get_config(const uint8_t gpio_pin, qcsapi_gpio_config * p_gpio_config)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_gpio_get_config_rpcdata __req;
	struct qcsapi_gpio_get_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.gpio_pin = (uint8_t)gpio_pin;

	__req.p_gpio_config = (int *)p_gpio_config;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GPIO_GET_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_gpio_get_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_gpio_get_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_gpio_get_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_gpio_get_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_gpio_config)
			*p_gpio_config = *__resp.p_gpio_config;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_gpio_get_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_led_get(const uint8_t led_ident, uint8_t * p_led_setting)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_led_get_rpcdata __req;
	struct qcsapi_led_get_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.led_ident = (uint8_t)led_ident;

	__req.p_led_setting = (uint8_t *)p_led_setting;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_LED_GET_REMOTE,
				(xdrproc_t)xdr_qcsapi_led_get_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_led_get_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_led_get call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_led_get_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_led_setting)
			*p_led_setting = *__resp.p_led_setting;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_led_get_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_led_set(const uint8_t led_ident, const uint8_t new_led_setting)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_led_set_rpcdata __req;
	struct qcsapi_led_set_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.led_ident = (uint8_t)led_ident;

	__req.new_led_setting = (uint8_t)new_led_setting;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_LED_SET_REMOTE,
				(xdrproc_t)xdr_qcsapi_led_set_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_led_set_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_led_set call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_led_set_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_led_set_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_led_pwm_enable(const uint8_t led_ident, const uint8_t onoff, const qcsapi_unsigned_int high_count, const qcsapi_unsigned_int low_count)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_led_pwm_enable_rpcdata __req;
	struct qcsapi_led_pwm_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.led_ident = (uint8_t)led_ident;

	__req.onoff = (uint8_t)onoff;

	__req.high_count = (unsigned int)high_count;

	__req.low_count = (unsigned int)low_count;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_LED_PWM_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_led_pwm_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_led_pwm_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_led_pwm_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_led_pwm_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_led_pwm_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_led_brightness(const uint8_t led_ident, const qcsapi_unsigned_int level)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_led_brightness_rpcdata __req;
	struct qcsapi_led_brightness_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.led_ident = (uint8_t)led_ident;

	__req.level = (unsigned int)level;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_LED_BRIGHTNESS_REMOTE,
				(xdrproc_t)xdr_qcsapi_led_brightness_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_led_brightness_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_led_brightness call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_led_brightness_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_led_brightness_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_gpio_monitor_reset_device(const uint8_t reset_device_pin, const uint8_t active_logic, const int blocking_flag, reset_device_callback respond_reset_device)
{
	/* stubbed, not implemented */
	fprintf(stderr, "%s not implemented\n", "qcsapi_gpio_monitor_reset_device");
	return -qcsapi_programming_error;
}
int qcsapi_gpio_enable_wps_push_button(const uint8_t wps_push_button, const uint8_t active_logic, const uint8_t use_interrupt_flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_gpio_enable_wps_push_button_rpcdata __req;
	struct qcsapi_gpio_enable_wps_push_button_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.wps_push_button = (uint8_t)wps_push_button;

	__req.active_logic = (uint8_t)active_logic;

	__req.use_interrupt_flag = (uint8_t)use_interrupt_flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GPIO_ENABLE_WPS_PUSH_BUTTON_REMOTE,
				(xdrproc_t)xdr_qcsapi_gpio_enable_wps_push_button_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_gpio_enable_wps_push_button_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_gpio_enable_wps_push_button call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_gpio_enable_wps_push_button_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_gpio_enable_wps_push_button_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_count_associations(const char * ifname, qcsapi_unsigned_int * p_association_count)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_count_associations_rpcdata __req;
	struct qcsapi_wifi_get_count_associations_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_association_count = (unsigned int *)p_association_count;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_COUNT_ASSOCIATIONS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_count_associations_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_count_associations_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_count_associations call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_count_associations_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_association_count)
			*p_association_count = *__resp.p_association_count;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_count_associations_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_associated_device_mac_addr(const char * ifname, const qcsapi_unsigned_int device_index, qcsapi_mac_addr device_mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_associated_device_mac_addr_rpcdata __req;
	struct qcsapi_wifi_get_associated_device_mac_addr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.device_index = (unsigned int)device_index;

	struct __rpc_qcsapi_mac_addr __rpcdevice_mac_addr;
	if (device_mac_addr) {
		memcpy(__rpcdevice_mac_addr.data, device_mac_addr, sizeof(__rpcdevice_mac_addr));
		__req.device_mac_addr = &__rpcdevice_mac_addr;
	} else {
		__req.device_mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ASSOCIATED_DEVICE_MAC_ADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_associated_device_mac_addr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_associated_device_mac_addr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_associated_device_mac_addr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_associated_device_mac_addr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (device_mac_addr && __resp.device_mac_addr)
			memcpy(device_mac_addr, __resp.device_mac_addr->data,
				sizeof(qcsapi_mac_addr));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_associated_device_mac_addr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_associated_device_ip_addr(const char * ifname, const qcsapi_unsigned_int device_index, unsigned int * ip_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_associated_device_ip_addr_rpcdata __req;
	struct qcsapi_wifi_get_associated_device_ip_addr_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.device_index = (unsigned int)device_index;

	__req.ip_addr = (unsigned int *)ip_addr;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ASSOCIATED_DEVICE_IP_ADDR_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_associated_device_ip_addr_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_associated_device_ip_addr_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_associated_device_ip_addr call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_associated_device_ip_addr_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ip_addr)
			*ip_addr = *__resp.ip_addr;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_associated_device_ip_addr_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_link_quality(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_link_quality)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_link_quality_rpcdata __req;
	struct qcsapi_wifi_get_link_quality_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_link_quality = (unsigned int *)p_link_quality;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_LINK_QUALITY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_link_quality_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_link_quality_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_link_quality call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_link_quality_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_link_quality)
			*p_link_quality = *__resp.p_link_quality;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_link_quality_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_link_quality_max(const char * ifname, qcsapi_unsigned_int * p_max_quality)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_link_quality_max_rpcdata __req;
	struct qcsapi_wifi_get_link_quality_max_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_max_quality = (unsigned int *)p_max_quality;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_LINK_QUALITY_MAX_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_link_quality_max_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_link_quality_max_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_link_quality_max call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_link_quality_max_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_max_quality)
			*p_max_quality = *__resp.p_max_quality;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_link_quality_max_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rx_bytes_per_association(const char * ifname, const qcsapi_unsigned_int association_index, u_int64_t * p_rx_bytes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rx_bytes_per_association_rpcdata __req;
	struct qcsapi_wifi_get_rx_bytes_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_rx_bytes = (uint64_t *)p_rx_bytes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RX_BYTES_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_bytes_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_bytes_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rx_bytes_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_bytes_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_rx_bytes)
			*p_rx_bytes = *__resp.p_rx_bytes;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_bytes_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_bytes_per_association(const char * ifname, const qcsapi_unsigned_int association_index, u_int64_t * p_tx_bytes)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_bytes_per_association_rpcdata __req;
	struct qcsapi_wifi_get_tx_bytes_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_tx_bytes = (uint64_t *)p_tx_bytes;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_BYTES_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_bytes_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_bytes_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_bytes_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_bytes_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_bytes)
			*p_tx_bytes = *__resp.p_tx_bytes;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_bytes_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rx_packets_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_rx_packets)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rx_packets_per_association_rpcdata __req;
	struct qcsapi_wifi_get_rx_packets_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_rx_packets = (unsigned int *)p_rx_packets;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RX_PACKETS_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_packets_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_packets_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rx_packets_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_packets_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_rx_packets)
			*p_rx_packets = *__resp.p_rx_packets;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_packets_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_packets_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_tx_packets)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_packets_per_association_rpcdata __req;
	struct qcsapi_wifi_get_tx_packets_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_tx_packets = (unsigned int *)p_tx_packets;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_PACKETS_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_packets_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_packets_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_packets_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_packets_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_packets)
			*p_tx_packets = *__resp.p_tx_packets;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_packets_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_err_packets_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_tx_err_packets)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_err_packets_per_association_rpcdata __req;
	struct qcsapi_wifi_get_tx_err_packets_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_tx_err_packets = (unsigned int *)p_tx_err_packets;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_ERR_PACKETS_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_err_packets_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_err_packets_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_err_packets_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_err_packets_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_err_packets)
			*p_tx_err_packets = *__resp.p_tx_err_packets;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_err_packets_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rssi_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_rssi)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rssi_per_association_rpcdata __req;
	struct qcsapi_wifi_get_rssi_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_rssi = (unsigned int *)p_rssi;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RSSI_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rssi_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rssi_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rssi_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rssi_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_rssi)
			*p_rssi = *__resp.p_rssi;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rssi_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rssi_in_dbm_per_association(const char * ifname, const qcsapi_unsigned_int association_index, int * p_rssi)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rssi_in_dbm_per_association_rpcdata __req;
	struct qcsapi_wifi_get_rssi_in_dbm_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_rssi = (int *)p_rssi;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RSSI_IN_DBM_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rssi_in_dbm_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rssi_in_dbm_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rssi_in_dbm_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rssi_in_dbm_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_rssi)
			*p_rssi = *__resp.p_rssi;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rssi_in_dbm_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bw_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_bw)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bw_per_association_rpcdata __req;
	struct qcsapi_wifi_get_bw_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_bw = (unsigned int *)p_bw;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BW_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bw_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bw_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bw_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bw_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_bw)
			*p_bw = *__resp.p_bw;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bw_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_phy_rate_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_tx_phy_rate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_phy_rate_per_association_rpcdata __req;
	struct qcsapi_wifi_get_tx_phy_rate_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_tx_phy_rate = (unsigned int *)p_tx_phy_rate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_PHY_RATE_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_phy_rate_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_phy_rate_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_phy_rate_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_phy_rate)
			*p_tx_phy_rate = *__resp.p_tx_phy_rate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rx_phy_rate_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_rx_phy_rate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rx_phy_rate_per_association_rpcdata __req;
	struct qcsapi_wifi_get_rx_phy_rate_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_rx_phy_rate = (unsigned int *)p_rx_phy_rate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RX_PHY_RATE_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_phy_rate_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_phy_rate_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rx_phy_rate_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_rx_phy_rate)
			*p_rx_phy_rate = *__resp.p_rx_phy_rate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_mcs_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_mcs)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_mcs_per_association_rpcdata __req;
	struct qcsapi_wifi_get_tx_mcs_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_mcs = (unsigned int *)p_mcs;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_MCS_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_mcs_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_mcs_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_mcs_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_mcs_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_mcs)
			*p_mcs = *__resp.p_mcs;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_mcs_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rx_mcs_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_mcs)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rx_mcs_per_association_rpcdata __req;
	struct qcsapi_wifi_get_rx_mcs_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_mcs = (unsigned int *)p_mcs;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RX_MCS_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_mcs_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rx_mcs_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rx_mcs_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_mcs_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_mcs)
			*p_mcs = *__resp.p_mcs;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rx_mcs_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_achievable_tx_phy_rate_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_achievable_tx_phy_rate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_achievable_tx_phy_rate_per_association_rpcdata __req;
	struct qcsapi_wifi_get_achievable_tx_phy_rate_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_achievable_tx_phy_rate = (unsigned int *)p_achievable_tx_phy_rate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ACHIEVABLE_TX_PHY_RATE_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_achievable_tx_phy_rate_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_achievable_tx_phy_rate_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_achievable_tx_phy_rate_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_achievable_tx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_achievable_tx_phy_rate)
			*p_achievable_tx_phy_rate = *__resp.p_achievable_tx_phy_rate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_achievable_tx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_achievable_rx_phy_rate_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_achievable_rx_phy_rate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_achievable_rx_phy_rate_per_association_rpcdata __req;
	struct qcsapi_wifi_get_achievable_rx_phy_rate_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_achievable_rx_phy_rate = (unsigned int *)p_achievable_rx_phy_rate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ACHIEVABLE_RX_PHY_RATE_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_achievable_rx_phy_rate_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_achievable_rx_phy_rate_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_achievable_rx_phy_rate_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_achievable_rx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_achievable_rx_phy_rate)
			*p_achievable_rx_phy_rate = *__resp.p_achievable_rx_phy_rate;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_achievable_rx_phy_rate_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_auth_enc_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_auth_enc)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_auth_enc_per_association_rpcdata __req;
	struct qcsapi_wifi_get_auth_enc_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_auth_enc = (unsigned int *)p_auth_enc;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AUTH_ENC_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_auth_enc_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_auth_enc_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_auth_enc_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_auth_enc_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_auth_enc)
			*p_auth_enc = *__resp.p_auth_enc;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_auth_enc_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tput_caps(const char * ifname, const qcsapi_unsigned_int association_index, struct ieee8011req_sta_tput_caps * tput_caps)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tput_caps_rpcdata __req;
	struct qcsapi_wifi_get_tput_caps_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.tput_caps = (__rpc_ieee8011req_sta_tput_caps*)tput_caps;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TPUT_CAPS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tput_caps_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tput_caps_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tput_caps call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tput_caps_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.tput_caps && tput_caps)
			memcpy(tput_caps, __resp.tput_caps, sizeof(*tput_caps));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tput_caps_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_connection_mode(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * connection_mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_connection_mode_rpcdata __req;
	struct qcsapi_wifi_get_connection_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.connection_mode = (unsigned int *)connection_mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CONNECTION_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_connection_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_connection_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_connection_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_connection_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (connection_mode)
			*connection_mode = *__resp.connection_mode;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_connection_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_vendor_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * p_vendor)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_vendor_per_association_rpcdata __req;
	struct qcsapi_wifi_get_vendor_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_vendor = (unsigned int *)p_vendor;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_VENDOR_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_vendor_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_vendor_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_vendor_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_vendor_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_vendor)
			*p_vendor = *__resp.p_vendor;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_vendor_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_max_mimo(const char * ifname, const qcsapi_unsigned_int association_index, string_16 p_max_mimo)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_max_mimo_rpcdata __req;
	struct qcsapi_wifi_get_max_mimo_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__rpc_string __rpcp_max_mimo = {(char *)p_max_mimo};
	__rpc_string *p__rpcp_max_mimo = (p_max_mimo) ? &__rpcp_max_mimo : NULL;
	__req.p_max_mimo = p__rpcp_max_mimo;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MAX_MIMO_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_max_mimo_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_max_mimo_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_max_mimo call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_max_mimo_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_max_mimo && __resp.p_max_mimo)
			strcpy(p_max_mimo, __resp.p_max_mimo->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_max_mimo_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_snr_per_association(const char * ifname, const qcsapi_unsigned_int association_index, int * p_snr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_snr_per_association_rpcdata __req;
	struct qcsapi_wifi_get_snr_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_snr = (int *)p_snr;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SNR_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_snr_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_snr_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_snr_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_snr_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_snr)
			*p_snr = *__resp.p_snr;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_snr_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_time_associated_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_unsigned_int * time_associated)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_time_associated_per_association_rpcdata __req;
	struct qcsapi_wifi_get_time_associated_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.time_associated = (unsigned int *)time_associated;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TIME_ASSOCIATED_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_time_associated_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_time_associated_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_time_associated_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_time_associated_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (time_associated)
			*time_associated = *__resp.time_associated;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_time_associated_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_node_param(const char * ifname, const uint32_t node_index, qcsapi_per_assoc_param param_type, int local_remote_flag, string_128 input_param_str, qcsapi_measure_report_result * report_result)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_node_param_rpcdata __req;
	struct qcsapi_wifi_get_node_param_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.node_index = (uint32_t)node_index;

	__req.param_type = (int)param_type;

	__req.local_remote_flag = (int)local_remote_flag;

	__rpc_string __rpcinput_param_str = {(char *)input_param_str};
	__rpc_string *p__rpcinput_param_str = (input_param_str) ? &__rpcinput_param_str : NULL;
	__req.input_param_str = p__rpcinput_param_str;

	__req.report_result = (__rpc_qcsapi_measure_report_result*)report_result;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_NODE_PARAM_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_node_param_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_node_param_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_node_param call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_node_param_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (input_param_str && __resp.input_param_str)
			strcpy(input_param_str, __resp.input_param_str->data);
	}
	if (__resp.return_code >= 0) {
		if (__resp.report_result && report_result)
			memcpy(report_result, __resp.report_result, sizeof(*report_result));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_node_param_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_node_counter(const char * ifname, const uint32_t node_index, qcsapi_counter_type counter_type, int local_remote_flag, uint64_t * p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_node_counter_rpcdata __req;
	struct qcsapi_wifi_get_node_counter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.node_index = (uint32_t)node_index;

	__req.counter_type = (int)counter_type;

	__req.local_remote_flag = (int)local_remote_flag;

	__req.p_value = (uint64_t *)p_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_NODE_COUNTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_node_counter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_node_counter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_node_counter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_node_counter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_node_counter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_node_stats(const char * ifname, const uint32_t node_index, int local_remote_flag, struct qcsapi_node_stats * p_stats)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_node_stats_rpcdata __req;
	struct qcsapi_wifi_get_node_stats_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.node_index = (uint32_t)node_index;

	__req.local_remote_flag = (int)local_remote_flag;

	__req.p_stats = (__rpc_qcsapi_node_stats*)p_stats;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_NODE_STATS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_node_stats_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_node_stats_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_node_stats call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_node_stats_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.p_stats && p_stats)
			memcpy(p_stats, __resp.p_stats, sizeof(*p_stats));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_node_stats_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_max_queued(const char * ifname, const uint32_t node_index, int local_remote_flag, int reset_flag, uint32_t * max_queued)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_max_queued_rpcdata __req;
	struct qcsapi_wifi_get_max_queued_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.node_index = (uint32_t)node_index;

	__req.local_remote_flag = (int)local_remote_flag;

	__req.reset_flag = (int)reset_flag;

	__req.max_queued = (uint32_t *)max_queued;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MAX_QUEUED_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_max_queued_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_max_queued_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_max_queued call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_max_queued_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (max_queued)
			*max_queued = *__resp.max_queued;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_max_queued_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_hw_noise_per_association(const char * ifname, const qcsapi_unsigned_int association_index, int * p_hw_noise)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_hw_noise_per_association_rpcdata __req;
	struct qcsapi_wifi_get_hw_noise_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.p_hw_noise = (int *)p_hw_noise;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_HW_NOISE_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_hw_noise_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_hw_noise_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_hw_noise_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_hw_noise_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_hw_noise)
			*p_hw_noise = *__resp.p_hw_noise;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_hw_noise_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mlme_stats_per_mac(const qcsapi_mac_addr client_mac_addr, qcsapi_mlme_stats * stats)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mlme_stats_per_mac_rpcdata __req;
	struct qcsapi_wifi_get_mlme_stats_per_mac_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	struct __rpc_qcsapi_mac_addr __rpcclient_mac_addr;
	if (client_mac_addr) {
		memcpy(__rpcclient_mac_addr.data, client_mac_addr, sizeof(__rpcclient_mac_addr));
		__req.client_mac_addr = &__rpcclient_mac_addr;
	} else {
		__req.client_mac_addr = NULL;
	}
	__req.stats = (__rpc_qcsapi_mlme_stats*)stats;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MLME_STATS_PER_MAC_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_mac_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_mac_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mlme_stats_per_mac call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_mac_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.stats && stats)
			memcpy(stats, __resp.stats, sizeof(*stats));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_mac_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mlme_stats_per_association(const char * ifname, const qcsapi_unsigned_int association_index, qcsapi_mlme_stats * stats)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mlme_stats_per_association_rpcdata __req;
	struct qcsapi_wifi_get_mlme_stats_per_association_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.association_index = (unsigned int)association_index;

	__req.stats = (__rpc_qcsapi_mlme_stats*)stats;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MLME_STATS_PER_ASSOCIATION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_association_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_association_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mlme_stats_per_association call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_association_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.stats && stats)
			memcpy(stats, __resp.stats, sizeof(*stats));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_per_association_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mlme_stats_macs_list(qcsapi_mlme_stats_macs * macs_list)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mlme_stats_macs_list_rpcdata __req;
	struct qcsapi_wifi_get_mlme_stats_macs_list_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.macs_list = (__rpc_qcsapi_mlme_stats_macs*)macs_list;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MLME_STATS_MACS_LIST_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_macs_list_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_macs_list_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mlme_stats_macs_list call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_macs_list_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.macs_list && macs_list)
			memcpy(macs_list, __resp.macs_list, sizeof(*macs_list));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mlme_stats_macs_list_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_list_regulatory_regions(string_256 list_regulatory_regions)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_list_regulatory_regions_rpcdata __req;
	struct qcsapi_wifi_get_list_regulatory_regions_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpclist_regulatory_regions = {(char *)list_regulatory_regions};
	__rpc_string *p__rpclist_regulatory_regions = (list_regulatory_regions) ? &__rpclist_regulatory_regions : NULL;
	__req.list_regulatory_regions = p__rpclist_regulatory_regions;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_LIST_REGULATORY_REGIONS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_regions_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_regions_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_list_regulatory_regions call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_regions_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_regulatory_regions && __resp.list_regulatory_regions)
			strcpy(list_regulatory_regions, __resp.list_regulatory_regions->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_regions_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_list_regulatory_regions(string_256 list_regulatory_regions)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_list_regulatory_regions_rpcdata __req;
	struct qcsapi_regulatory_get_list_regulatory_regions_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpclist_regulatory_regions = {(char *)list_regulatory_regions};
	__rpc_string *p__rpclist_regulatory_regions = (list_regulatory_regions) ? &__rpclist_regulatory_regions : NULL;
	__req.list_regulatory_regions = p__rpclist_regulatory_regions;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_LIST_REGULATORY_REGIONS_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_regions_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_regions_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_list_regulatory_regions call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_regions_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_regulatory_regions && __resp.list_regulatory_regions)
			strcpy(list_regulatory_regions, __resp.list_regulatory_regions->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_regions_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_list_regulatory_channels(const char * region_by_name, const qcsapi_unsigned_int bw, string_1024 list_of_channels)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_list_regulatory_channels_rpcdata __req;
	struct qcsapi_wifi_get_list_regulatory_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.bw = (unsigned int)bw;

	__rpc_string __rpclist_of_channels = {(char *)list_of_channels};
	__rpc_string *p__rpclist_of_channels = (list_of_channels) ? &__rpclist_of_channels : NULL;
	__req.list_of_channels = p__rpclist_of_channels;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_LIST_REGULATORY_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_list_regulatory_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_of_channels && __resp.list_of_channels)
			strcpy(list_of_channels, __resp.list_of_channels->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_regulatory_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_list_regulatory_channels(const char * region_by_name, const qcsapi_unsigned_int bw, string_1024 list_of_channels)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_list_regulatory_channels_rpcdata __req;
	struct qcsapi_regulatory_get_list_regulatory_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.bw = (unsigned int)bw;

	__rpc_string __rpclist_of_channels = {(char *)list_of_channels};
	__rpc_string *p__rpclist_of_channels = (list_of_channels) ? &__rpclist_of_channels : NULL;
	__req.list_of_channels = p__rpclist_of_channels;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_LIST_REGULATORY_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_list_regulatory_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_of_channels && __resp.list_of_channels)
			strcpy(list_of_channels, __resp.list_of_channels->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_list_regulatory_bands(const char * region_by_name, string_128 list_of_bands)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_list_regulatory_bands_rpcdata __req;
	struct qcsapi_regulatory_get_list_regulatory_bands_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__rpc_string __rpclist_of_bands = {(char *)list_of_bands};
	__rpc_string *p__rpclist_of_bands = (list_of_bands) ? &__rpclist_of_bands : NULL;
	__req.list_of_bands = p__rpclist_of_bands;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_LIST_REGULATORY_BANDS_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_bands_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_bands_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_list_regulatory_bands call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_bands_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_of_bands && __resp.list_of_bands)
			strcpy(list_of_bands, __resp.list_of_bands->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_regulatory_bands_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_regulatory_tx_power(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, int * p_tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_regulatory_tx_power_rpcdata __req;
	struct qcsapi_wifi_get_regulatory_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.p_tx_power = (int *)p_tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_REGULATORY_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_regulatory_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_regulatory_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_regulatory_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_regulatory_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_power)
			*p_tx_power = *__resp.p_tx_power;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_regulatory_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_regulatory_tx_power(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, int * p_tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_regulatory_tx_power_rpcdata __req;
	struct qcsapi_regulatory_get_regulatory_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.p_tx_power = (int *)p_tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_REGULATORY_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_regulatory_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_regulatory_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_regulatory_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_regulatory_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_power)
			*p_tx_power = *__resp.p_tx_power;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_regulatory_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_configured_tx_power(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, const qcsapi_unsigned_int bw, int * p_tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_configured_tx_power_rpcdata __req;
	struct qcsapi_wifi_get_configured_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.bw = (unsigned int)bw;

	__req.p_tx_power = (int *)p_tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CONFIGURED_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_configured_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_configured_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_configured_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_configured_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_power)
			*p_tx_power = *__resp.p_tx_power;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_configured_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_configured_tx_power(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, const qcsapi_unsigned_int bw, int * p_tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_configured_tx_power_rpcdata __req;
	struct qcsapi_regulatory_get_configured_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.bw = (unsigned int)bw;

	__req.p_tx_power = (int *)p_tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_CONFIGURED_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_configured_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_power)
			*p_tx_power = *__resp.p_tx_power;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_configured_tx_power_ext(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, const qcsapi_bw the_bw, const qcsapi_unsigned_int bf_on, const qcsapi_unsigned_int number_ss, int * p_tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_configured_tx_power_ext_rpcdata __req;
	struct qcsapi_regulatory_get_configured_tx_power_ext_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.the_bw = (int)the_bw;

	__req.bf_on = (unsigned int)bf_on;

	__req.number_ss = (unsigned int)number_ss;

	__req.p_tx_power = (int *)p_tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_CONFIGURED_TX_POWER_EXT_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_ext_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_ext_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_configured_tx_power_ext call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_ext_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tx_power)
			*p_tx_power = *__resp.p_tx_power;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_configured_tx_power_ext_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_regulatory_region(const char * ifname, const char * region_by_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_regulatory_region_rpcdata __req;
	struct qcsapi_wifi_set_regulatory_region_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_REGULATORY_REGION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_regulatory_region_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_regulatory_region_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_regulatory_region call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_regulatory_region_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_regulatory_region_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_set_regulatory_region(const char * ifname, const char * region_by_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_set_regulatory_region_rpcdata __req;
	struct qcsapi_regulatory_set_regulatory_region_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_SET_REGULATORY_REGION_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_region_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_region_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_set_regulatory_region call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_region_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_region_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_restore_regulatory_tx_power(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_restore_regulatory_tx_power_rpcdata __req;
	struct qcsapi_regulatory_restore_regulatory_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_RESTORE_REGULATORY_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_restore_regulatory_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_restore_regulatory_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_restore_regulatory_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_restore_regulatory_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_restore_regulatory_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_regulatory_region(const char * ifname, char * region_by_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_regulatory_region_rpcdata __req;
	struct qcsapi_wifi_get_regulatory_region_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_REGULATORY_REGION_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_regulatory_region_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_regulatory_region_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_regulatory_region call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_regulatory_region_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (region_by_name && __resp.region_by_name)
			strcpy(region_by_name, __resp.region_by_name->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_regulatory_region_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_overwrite_country_code(const char * ifname, const char * curr_country_name, const char * new_country_name)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_overwrite_country_code_rpcdata __req;
	struct qcsapi_regulatory_overwrite_country_code_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurr_country_name = {(char *)curr_country_name};
	__rpc_string *p__rpccurr_country_name = (curr_country_name) ? &__rpccurr_country_name : NULL;
	__req.curr_country_name = p__rpccurr_country_name;

	__rpc_string __rpcnew_country_name = {(char *)new_country_name};
	__rpc_string *p__rpcnew_country_name = (new_country_name) ? &__rpcnew_country_name : NULL;
	__req.new_country_name = p__rpcnew_country_name;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_OVERWRITE_COUNTRY_CODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_overwrite_country_code_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_overwrite_country_code_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_overwrite_country_code call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_overwrite_country_code_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_overwrite_country_code_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_regulatory_channel(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, const qcsapi_unsigned_int tx_power_offset)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_regulatory_channel_rpcdata __req;
	struct qcsapi_wifi_set_regulatory_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.tx_power_offset = (unsigned int)tx_power_offset;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_REGULATORY_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_regulatory_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_regulatory_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_regulatory_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_regulatory_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_regulatory_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_set_regulatory_channel(const char * ifname, const qcsapi_unsigned_int the_channel, const char * region_by_name, const qcsapi_unsigned_int tx_power_offset)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_set_regulatory_channel_rpcdata __req;
	struct qcsapi_regulatory_set_regulatory_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.the_channel = (unsigned int)the_channel;

	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.tx_power_offset = (unsigned int)tx_power_offset;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_SET_REGULATORY_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_set_regulatory_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_set_regulatory_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_db_version(int * p_version, const int index)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_db_version_rpcdata __req;
	struct qcsapi_regulatory_get_db_version_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.p_version = (int *)p_version;

	__req.index = (int)index;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_DB_VERSION_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_db_version_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_db_version_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_db_version call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_db_version_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_version)
			*p_version = *__resp.p_version;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_db_version_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_apply_tx_power_cap(int capped)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_apply_tx_power_cap_rpcdata __req;
	struct qcsapi_regulatory_apply_tx_power_cap_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.capped = (int)capped;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_APPLY_TX_POWER_CAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_apply_tx_power_cap_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_apply_tx_power_cap_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_apply_tx_power_cap call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_apply_tx_power_cap_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_apply_tx_power_cap_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_list_DFS_channels(const char * region_by_name, const int DFS_flag, const qcsapi_unsigned_int bw, string_1024 list_of_channels)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_list_DFS_channels_rpcdata __req;
	struct qcsapi_wifi_get_list_DFS_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.DFS_flag = (int)DFS_flag;

	__req.bw = (unsigned int)bw;

	__rpc_string __rpclist_of_channels = {(char *)list_of_channels};
	__rpc_string *p__rpclist_of_channels = (list_of_channels) ? &__rpclist_of_channels : NULL;
	__req.list_of_channels = p__rpclist_of_channels;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_LIST_DFS_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_DFS_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_list_DFS_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_list_DFS_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_DFS_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_of_channels && __resp.list_of_channels)
			strcpy(list_of_channels, __resp.list_of_channels->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_list_DFS_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_get_list_DFS_channels(const char * region_by_name, const int DFS_flag, const qcsapi_unsigned_int bw, string_1024 list_of_channels)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_get_list_DFS_channels_rpcdata __req;
	struct qcsapi_regulatory_get_list_DFS_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.DFS_flag = (int)DFS_flag;

	__req.bw = (unsigned int)bw;

	__rpc_string __rpclist_of_channels = {(char *)list_of_channels};
	__rpc_string *p__rpclist_of_channels = (list_of_channels) ? &__rpclist_of_channels : NULL;
	__req.list_of_channels = p__rpclist_of_channels;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_GET_LIST_DFS_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_DFS_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_get_list_DFS_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_get_list_DFS_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_DFS_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (list_of_channels && __resp.list_of_channels)
			strcpy(list_of_channels, __resp.list_of_channels->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_get_list_DFS_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_is_channel_DFS(const char * region_by_name, const qcsapi_unsigned_int the_channel, int * p_channel_is_DFS)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_is_channel_DFS_rpcdata __req;
	struct qcsapi_wifi_is_channel_DFS_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.the_channel = (unsigned int)the_channel;

	__req.p_channel_is_DFS = (int *)p_channel_is_DFS;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_IS_CHANNEL_DFS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_is_channel_DFS_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_is_channel_DFS_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_is_channel_DFS call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_is_channel_DFS_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_channel_is_DFS)
			*p_channel_is_DFS = *__resp.p_channel_is_DFS;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_is_channel_DFS_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_regulatory_is_channel_DFS(const char * region_by_name, const qcsapi_unsigned_int the_channel, int * p_channel_is_DFS)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_regulatory_is_channel_DFS_rpcdata __req;
	struct qcsapi_regulatory_is_channel_DFS_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcregion_by_name = {(char *)region_by_name};
	__rpc_string *p__rpcregion_by_name = (region_by_name) ? &__rpcregion_by_name : NULL;
	__req.region_by_name = p__rpcregion_by_name;

	__req.the_channel = (unsigned int)the_channel;

	__req.p_channel_is_DFS = (int *)p_channel_is_DFS;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_REGULATORY_IS_CHANNEL_DFS_REMOTE,
				(xdrproc_t)xdr_qcsapi_regulatory_is_channel_DFS_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_regulatory_is_channel_DFS_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_regulatory_is_channel_DFS call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_regulatory_is_channel_DFS_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_channel_is_DFS)
			*p_channel_is_DFS = *__resp.p_channel_is_DFS;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_regulatory_is_channel_DFS_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dfs_cce_channels(const char * ifname, qcsapi_unsigned_int * p_prev_channel, qcsapi_unsigned_int * p_cur_channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dfs_cce_channels_rpcdata __req;
	struct qcsapi_wifi_get_dfs_cce_channels_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_prev_channel = (unsigned int *)p_prev_channel;

	__req.p_cur_channel = (unsigned int *)p_cur_channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DFS_CCE_CHANNELS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dfs_cce_channels_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dfs_cce_channels_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dfs_cce_channels call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dfs_cce_channels_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_prev_channel)
			*p_prev_channel = *__resp.p_prev_channel;
	}
	if (__resp.return_code >= 0) {
		if (p_cur_channel)
			*p_cur_channel = *__resp.p_cur_channel;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dfs_cce_channels_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_DFS_alt_channel(const char * ifname, qcsapi_unsigned_int * p_dfs_alt_chan)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_DFS_alt_channel_rpcdata __req;
	struct qcsapi_wifi_get_DFS_alt_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_dfs_alt_chan = (unsigned int *)p_dfs_alt_chan;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DFS_ALT_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_DFS_alt_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_DFS_alt_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_DFS_alt_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_DFS_alt_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_dfs_alt_chan)
			*p_dfs_alt_chan = *__resp.p_dfs_alt_chan;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_DFS_alt_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_DFS_alt_channel(const char * ifname, const qcsapi_unsigned_int dfs_alt_chan)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_DFS_alt_channel_rpcdata __req;
	struct qcsapi_wifi_set_DFS_alt_channel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.dfs_alt_chan = (unsigned int)dfs_alt_chan;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DFS_ALT_CHANNEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_DFS_alt_channel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_DFS_alt_channel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_DFS_alt_channel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_DFS_alt_channel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_DFS_alt_channel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_start_dfs_reentry(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_start_dfs_reentry_rpcdata __req;
	struct qcsapi_wifi_start_dfs_reentry_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_START_DFS_REENTRY_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_start_dfs_reentry_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_start_dfs_reentry_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_start_dfs_reentry call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_dfs_reentry_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_dfs_reentry_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_start_scan_ext(const char * ifname, const int scan_flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_start_scan_ext_rpcdata __req;
	struct qcsapi_wifi_start_scan_ext_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scan_flag = (int)scan_flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_START_SCAN_EXT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_start_scan_ext_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_start_scan_ext_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_start_scan_ext call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_scan_ext_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_scan_ext_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_csw_records(const char * ifname, int reset, qcsapi_csw_record * record)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_csw_records_rpcdata __req;
	struct qcsapi_wifi_get_csw_records_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.reset = (int)reset;

	__req.record = (__rpc_qcsapi_csw_record*)record;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CSW_RECORDS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_csw_records_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_csw_records_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_csw_records call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_csw_records_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.record && record)
			memcpy(record, __resp.record, sizeof(*record));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_csw_records_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_radar_status(const char * ifname, qcsapi_radar_status * rdstatus)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_radar_status_rpcdata __req;
	struct qcsapi_wifi_get_radar_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.rdstatus = (__rpc_qcsapi_radar_status*)rdstatus;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RADAR_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_radar_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_radar_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_radar_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_radar_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.rdstatus && rdstatus)
			memcpy(rdstatus, __resp.rdstatus, sizeof(*rdstatus));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_radar_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_cac_status(const char * ifname, int * cacstatus)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_cac_status_rpcdata __req;
	struct qcsapi_wifi_get_cac_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.cacstatus = (int *)cacstatus;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_CAC_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_cac_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_cac_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_cac_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_cac_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (cacstatus)
			*cacstatus = *__resp.cacstatus;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_cac_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_results_AP_scan(const char * ifname, qcsapi_unsigned_int * p_count_APs)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_results_AP_scan_rpcdata __req;
	struct qcsapi_wifi_get_results_AP_scan_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_count_APs = (unsigned int *)p_count_APs;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RESULTS_AP_SCAN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_results_AP_scan_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_results_AP_scan_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_results_AP_scan call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_results_AP_scan_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_count_APs)
			*p_count_APs = *__resp.p_count_APs;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_results_AP_scan_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_count_APs_scanned(const char * ifname, qcsapi_unsigned_int * p_count_APs)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_count_APs_scanned_rpcdata __req;
	struct qcsapi_wifi_get_count_APs_scanned_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_count_APs = (unsigned int *)p_count_APs;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_COUNT_APS_SCANNED_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_count_APs_scanned_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_count_APs_scanned_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_count_APs_scanned call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_count_APs_scanned_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_count_APs)
			*p_count_APs = *__resp.p_count_APs;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_count_APs_scanned_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_properties_AP(const char * ifname, const qcsapi_unsigned_int index_AP, qcsapi_ap_properties * p_ap_properties)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_properties_AP_rpcdata __req;
	struct qcsapi_wifi_get_properties_AP_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.index_AP = (unsigned int)index_AP;

	__req.p_ap_properties = (__rpc_qcsapi_ap_properties*)p_ap_properties;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PROPERTIES_AP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_properties_AP_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_properties_AP_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_properties_AP call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_properties_AP_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.p_ap_properties && p_ap_properties)
			memcpy(p_ap_properties, __resp.p_ap_properties, sizeof(*p_ap_properties));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_properties_AP_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scan_chk_inv(const char * ifname, int scan_chk_inv)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scan_chk_inv_rpcdata __req;
	struct qcsapi_wifi_set_scan_chk_inv_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scan_chk_inv = (int)scan_chk_inv;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCAN_CHK_INV_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scan_chk_inv_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scan_chk_inv_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scan_chk_inv call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scan_chk_inv_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scan_chk_inv_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scan_chk_inv(const char * ifname, int * p)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scan_chk_inv_rpcdata __req;
	struct qcsapi_wifi_get_scan_chk_inv_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p = (int *)p;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCAN_CHK_INV_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_chk_inv_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_chk_inv_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scan_chk_inv call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_chk_inv_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p)
			*p = *__resp.p;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_chk_inv_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scan_buf_max_size(const char * ifname, const unsigned int max_buf_size)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scan_buf_max_size_rpcdata __req;
	struct qcsapi_wifi_set_scan_buf_max_size_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.max_buf_size = (unsigned int)max_buf_size;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCAN_BUF_MAX_SIZE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scan_buf_max_size_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scan_buf_max_size_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scan_buf_max_size call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scan_buf_max_size_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scan_buf_max_size_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scan_buf_max_size(const char * ifname, unsigned int * max_buf_size)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scan_buf_max_size_rpcdata __req;
	struct qcsapi_wifi_get_scan_buf_max_size_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.max_buf_size = (unsigned int *)max_buf_size;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCAN_BUF_MAX_SIZE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_buf_max_size_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_buf_max_size_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scan_buf_max_size call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_buf_max_size_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (max_buf_size)
			*max_buf_size = *__resp.max_buf_size;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_buf_max_size_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_scan_table_max_len(const char * ifname, const unsigned int max_table_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_scan_table_max_len_rpcdata __req;
	struct qcsapi_wifi_set_scan_table_max_len_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.max_table_len = (unsigned int)max_table_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_SCAN_TABLE_MAX_LEN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_scan_table_max_len_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_scan_table_max_len_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_scan_table_max_len call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scan_table_max_len_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_scan_table_max_len_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scan_table_max_len(const char * ifname, unsigned int * max_table_len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scan_table_max_len_rpcdata __req;
	struct qcsapi_wifi_get_scan_table_max_len_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.max_table_len = (unsigned int *)max_table_len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCAN_TABLE_MAX_LEN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_table_max_len_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_table_max_len_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scan_table_max_len call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_table_max_len_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (max_table_len)
			*max_table_len = *__resp.max_table_len;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_table_max_len_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_dwell_times(const char * ifname, const unsigned int max_dwell_time_active_chan, const unsigned int min_dwell_time_active_chan, const unsigned int max_dwell_time_passive_chan, const unsigned int min_dwell_time_passive_chan)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_dwell_times_rpcdata __req;
	struct qcsapi_wifi_set_dwell_times_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.max_dwell_time_active_chan = (unsigned int)max_dwell_time_active_chan;

	__req.min_dwell_time_active_chan = (unsigned int)min_dwell_time_active_chan;

	__req.max_dwell_time_passive_chan = (unsigned int)max_dwell_time_passive_chan;

	__req.min_dwell_time_passive_chan = (unsigned int)min_dwell_time_passive_chan;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_DWELL_TIMES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_dwell_times_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_dwell_times_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_dwell_times call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dwell_times_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_dwell_times_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_dwell_times(const char * ifname, unsigned int * p_max_dwell_time_active_chan, unsigned int * p_min_dwell_time_active_chan, unsigned int * p_max_dwell_time_passive_chan, unsigned int * p_min_dwell_time_passive_chan)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_dwell_times_rpcdata __req;
	struct qcsapi_wifi_get_dwell_times_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_max_dwell_time_active_chan = (unsigned int *)p_max_dwell_time_active_chan;

	__req.p_min_dwell_time_active_chan = (unsigned int *)p_min_dwell_time_active_chan;

	__req.p_max_dwell_time_passive_chan = (unsigned int *)p_max_dwell_time_passive_chan;

	__req.p_min_dwell_time_passive_chan = (unsigned int *)p_min_dwell_time_passive_chan;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DWELL_TIMES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_dwell_times_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_dwell_times_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_dwell_times call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dwell_times_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_max_dwell_time_active_chan)
			*p_max_dwell_time_active_chan = *__resp.p_max_dwell_time_active_chan;
	}
	if (__resp.return_code >= 0) {
		if (p_min_dwell_time_active_chan)
			*p_min_dwell_time_active_chan = *__resp.p_min_dwell_time_active_chan;
	}
	if (__resp.return_code >= 0) {
		if (p_max_dwell_time_passive_chan)
			*p_max_dwell_time_passive_chan = *__resp.p_max_dwell_time_passive_chan;
	}
	if (__resp.return_code >= 0) {
		if (p_min_dwell_time_passive_chan)
			*p_min_dwell_time_passive_chan = *__resp.p_min_dwell_time_passive_chan;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_dwell_times_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_bgscan_dwell_times(const char * ifname, const unsigned int dwell_time_active_chan, const unsigned int dwell_time_passive_chan)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_bgscan_dwell_times_rpcdata __req;
	struct qcsapi_wifi_set_bgscan_dwell_times_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.dwell_time_active_chan = (unsigned int)dwell_time_active_chan;

	__req.dwell_time_passive_chan = (unsigned int)dwell_time_passive_chan;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_BGSCAN_DWELL_TIMES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_bgscan_dwell_times_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_bgscan_dwell_times_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_bgscan_dwell_times call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bgscan_dwell_times_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_bgscan_dwell_times_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bgscan_dwell_times(const char * ifname, unsigned int * p_dwell_time_active_chan, unsigned int * p_dwell_time_passive_chan)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bgscan_dwell_times_rpcdata __req;
	struct qcsapi_wifi_get_bgscan_dwell_times_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_dwell_time_active_chan = (unsigned int *)p_dwell_time_active_chan;

	__req.p_dwell_time_passive_chan = (unsigned int *)p_dwell_time_passive_chan;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BGSCAN_DWELL_TIMES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bgscan_dwell_times_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bgscan_dwell_times_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bgscan_dwell_times call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bgscan_dwell_times_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_dwell_time_active_chan)
			*p_dwell_time_active_chan = *__resp.p_dwell_time_active_chan;
	}
	if (__resp.return_code >= 0) {
		if (p_dwell_time_passive_chan)
			*p_dwell_time_passive_chan = *__resp.p_dwell_time_passive_chan;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bgscan_dwell_times_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_start_scan(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_start_scan_rpcdata __req;
	struct qcsapi_wifi_start_scan_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_START_SCAN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_start_scan_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_start_scan_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_start_scan call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_scan_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_start_scan_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_cancel_scan(const char * ifname, int force)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_cancel_scan_rpcdata __req;
	struct qcsapi_wifi_cancel_scan_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.force = (int)force;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_CANCEL_SCAN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_cancel_scan_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_cancel_scan_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_cancel_scan call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_cancel_scan_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_cancel_scan_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_scan_status(const char * ifname, int * scanstatus)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_scan_status_rpcdata __req;
	struct qcsapi_wifi_get_scan_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.scanstatus = (int *)scanstatus;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_SCAN_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_scan_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_scan_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (scanstatus)
			*scanstatus = *__resp.scanstatus;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_scan_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_enable_bgscan(const char * ifname, const int enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_enable_bgscan_rpcdata __req;
	struct qcsapi_wifi_enable_bgscan_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (int)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_ENABLE_BGSCAN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_enable_bgscan_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_enable_bgscan_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_enable_bgscan call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_enable_bgscan_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_enable_bgscan_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_bgscan_status(const char * ifname, int * enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_bgscan_status_rpcdata __req;
	struct qcsapi_wifi_get_bgscan_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (int *)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_BGSCAN_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_bgscan_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_bgscan_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_bgscan_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bgscan_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (enable)
			*enable = *__resp.enable;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_bgscan_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_wait_scan_completes(const char * ifname, time_t timeout)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_wait_scan_completes_rpcdata __req;
	struct qcsapi_wifi_wait_scan_completes_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.timeout = (uint32_t)timeout;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_WAIT_SCAN_COMPLETES_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_wait_scan_completes_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_wait_scan_completes_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_wait_scan_completes call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_wait_scan_completes_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_wait_scan_completes_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_backoff_fail_max(const char * ifname, const int fail_max)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_backoff_fail_max_rpcdata __req;
	struct qcsapi_wifi_backoff_fail_max_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.fail_max = (int)fail_max;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_BACKOFF_FAIL_MAX_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_backoff_fail_max_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_backoff_fail_max_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_backoff_fail_max call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_backoff_fail_max_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_backoff_fail_max_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_backoff_timeout(const char * ifname, const int timeout)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_backoff_timeout_rpcdata __req;
	struct qcsapi_wifi_backoff_timeout_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.timeout = (int)timeout;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_BACKOFF_TIMEOUT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_backoff_timeout_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_backoff_timeout_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_backoff_timeout call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_backoff_timeout_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_backoff_timeout_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mcs_rate(const char * ifname, qcsapi_mcs_rate current_mcs_rate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mcs_rate_rpcdata __req;
	struct qcsapi_wifi_get_mcs_rate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpccurrent_mcs_rate = {(char *)current_mcs_rate};
	__rpc_string *p__rpccurrent_mcs_rate = (current_mcs_rate) ? &__rpccurrent_mcs_rate : NULL;
	__req.current_mcs_rate = p__rpccurrent_mcs_rate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MCS_RATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mcs_rate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mcs_rate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mcs_rate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mcs_rate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (current_mcs_rate && __resp.current_mcs_rate)
			strcpy(current_mcs_rate, __resp.current_mcs_rate->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mcs_rate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_mcs_rate(const char * ifname, const qcsapi_mcs_rate new_mcs_rate)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_mcs_rate_rpcdata __req;
	struct qcsapi_wifi_set_mcs_rate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcnew_mcs_rate = {(char *)new_mcs_rate};
	__rpc_string *p__rpcnew_mcs_rate = (new_mcs_rate) ? &__rpcnew_mcs_rate : NULL;
	__req.new_mcs_rate = p__rpcnew_mcs_rate;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_MCS_RATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_mcs_rate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_mcs_rate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_mcs_rate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mcs_rate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mcs_rate_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_pairing_id(const char * ifname, const char * pairing_id)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_pairing_id_rpcdata __req;
	struct qcsapi_wifi_set_pairing_id_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcpairing_id = {(char *)pairing_id};
	__rpc_string *p__rpcpairing_id = (pairing_id) ? &__rpcpairing_id : NULL;
	__req.pairing_id = p__rpcpairing_id;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PAIRING_ID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_pairing_id_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_pairing_id_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_pairing_id call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pairing_id_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pairing_id_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_pairing_id(const char * ifname, char * pairing_id)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_pairing_id_rpcdata __req;
	struct qcsapi_wifi_get_pairing_id_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcpairing_id = {(char *)pairing_id};
	__rpc_string *p__rpcpairing_id = (pairing_id) ? &__rpcpairing_id : NULL;
	__req.pairing_id = p__rpcpairing_id;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PAIRING_ID_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_pairing_id_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_pairing_id_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_pairing_id call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pairing_id_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (pairing_id && __resp.pairing_id)
			strcpy(pairing_id, __resp.pairing_id->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pairing_id_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_pairing_enable(const char * ifname, const char * enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_pairing_enable_rpcdata __req;
	struct qcsapi_wifi_set_pairing_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcenable = {(char *)enable};
	__rpc_string *p__rpcenable = (enable) ? &__rpcenable : NULL;
	__req.enable = p__rpcenable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_PAIRING_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_pairing_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_pairing_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_pairing_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pairing_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_pairing_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_pairing_enable(const char * ifname, char * enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_pairing_enable_rpcdata __req;
	struct qcsapi_wifi_get_pairing_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcenable = {(char *)enable};
	__rpc_string *p__rpcenable = (enable) ? &__rpcenable : NULL;
	__req.enable = p__rpcenable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_PAIRING_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_pairing_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_pairing_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_pairing_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pairing_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (enable && __resp.enable)
			strcpy(enable, __resp.enable->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_pairing_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_non_wps_set_pp_enable(const char * ifname, uint32_t ctrl_state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_non_wps_set_pp_enable_rpcdata __req;
	struct qcsapi_non_wps_set_pp_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ctrl_state = (uint32_t)ctrl_state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_NON_WPS_SET_PP_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_non_wps_set_pp_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_non_wps_set_pp_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_non_wps_set_pp_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_non_wps_set_pp_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_non_wps_set_pp_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_non_wps_get_pp_enable(const char * ifname, uint32_t * ctrl_state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_non_wps_get_pp_enable_rpcdata __req;
	struct qcsapi_non_wps_get_pp_enable_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ctrl_state = (uint32_t *)ctrl_state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_NON_WPS_GET_PP_ENABLE_REMOTE,
				(xdrproc_t)xdr_qcsapi_non_wps_get_pp_enable_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_non_wps_get_pp_enable_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_non_wps_get_pp_enable call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_non_wps_get_pp_enable_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ctrl_state)
			*ctrl_state = *__resp.ctrl_state;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_non_wps_get_pp_enable_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_vendor_fix(const char * ifname, int fix_param, int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_vendor_fix_rpcdata __req;
	struct qcsapi_wifi_set_vendor_fix_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.fix_param = (int)fix_param;

	__req.value = (int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_VENDOR_FIX_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_vendor_fix_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_vendor_fix_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_vendor_fix call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_vendor_fix_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_vendor_fix_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_errno_get_message(const int qcsapi_retval, char * error_msg, unsigned int msglen)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_errno_get_message_rpcdata __req;
	struct qcsapi_errno_get_message_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.qcsapi_retval = (int)qcsapi_retval;

	__rpc_string __rpcerror_msg = {(char *)error_msg};
	__rpc_string *p__rpcerror_msg = (error_msg) ? &__rpcerror_msg : NULL;
	__req.error_msg = p__rpcerror_msg;

	__req.msglen = (unsigned int)msglen;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_ERRNO_GET_MESSAGE_REMOTE,
				(xdrproc_t)xdr_qcsapi_errno_get_message_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_errno_get_message_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_errno_get_message call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_errno_get_message_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (error_msg && __resp.error_msg)
			strcpy(error_msg, __resp.error_msg->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_errno_get_message_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_interface_stats(const char * ifname, qcsapi_interface_stats * stats)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_interface_stats_rpcdata __req;
	struct qcsapi_get_interface_stats_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.stats = (__rpc_qcsapi_interface_stats*)stats;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_INTERFACE_STATS_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_interface_stats_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_interface_stats_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_interface_stats call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_interface_stats_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.stats && stats)
			memcpy(stats, __resp.stats, sizeof(*stats));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_interface_stats_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_phy_stats(const char * ifname, qcsapi_phy_stats * stats)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_phy_stats_rpcdata __req;
	struct qcsapi_get_phy_stats_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.stats = (__rpc_qcsapi_phy_stats*)stats;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_PHY_STATS_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_phy_stats_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_phy_stats_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_phy_stats call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_phy_stats_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.stats && stats)
			memcpy(stats, __resp.stats, sizeof(*stats));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_phy_stats_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_reset_all_counters(const char * ifname, const uint32_t node_index, int local_remote_flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_reset_all_counters_rpcdata __req;
	struct qcsapi_reset_all_counters_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.node_index = (uint32_t)node_index;

	__req.local_remote_flag = (int)local_remote_flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_RESET_ALL_COUNTERS_REMOTE,
				(xdrproc_t)xdr_qcsapi_reset_all_counters_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_reset_all_counters_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_reset_all_counters call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_reset_all_counters_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_reset_all_counters_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_uboot_info(string_32 uboot_version, struct early_flash_config * ef_config)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_uboot_info_rpcdata __req;
	struct qcsapi_get_uboot_info_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcuboot_version = {(char *)uboot_version};
	__rpc_string *p__rpcuboot_version = (uboot_version) ? &__rpcuboot_version : NULL;
	__req.uboot_version = p__rpcuboot_version;

	__req.ef_config = (__rpc_early_flash_config*)ef_config;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_UBOOT_INFO_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_uboot_info_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_uboot_info_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_uboot_info call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_uboot_info_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (uboot_version && __resp.uboot_version)
			strcpy(uboot_version, __resp.uboot_version->data);
	}
	if (__resp.return_code >= 0) {
		if (__resp.ef_config && ef_config)
			memcpy(ef_config, __resp.ef_config, sizeof(*ef_config));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_uboot_info_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_firmware_get_version(char * firmware_version, const qcsapi_unsigned_int version_size)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_firmware_get_version_rpcdata __req;
	struct qcsapi_firmware_get_version_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcfirmware_version = {(char *)firmware_version};
	__rpc_string *p__rpcfirmware_version = (firmware_version) ? &__rpcfirmware_version : NULL;
	__req.firmware_version = p__rpcfirmware_version;

	__req.version_size = (unsigned int)version_size;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_FIRMWARE_GET_VERSION_REMOTE,
				(xdrproc_t)xdr_qcsapi_firmware_get_version_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_firmware_get_version_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_firmware_get_version call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_firmware_get_version_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (firmware_version && __resp.firmware_version)
			strcpy(firmware_version, __resp.firmware_version->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_firmware_get_version_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_flash_image_update(const char * image_file, qcsapi_flash_partiton_type partition_to_upgrade)
{
	int retries = retries_limit;
	static struct timeval timeout = { 60, 0 };
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_flash_image_update_rpcdata __req;
	struct qcsapi_flash_image_update_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcimage_file = {(char *)image_file};
	__rpc_string *p__rpcimage_file = (image_file) ? &__rpcimage_file : NULL;
	__req.image_file = p__rpcimage_file;

	__req.partition_to_upgrade = (int)partition_to_upgrade;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_FLASH_IMAGE_UPDATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_flash_image_update_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_flash_image_update_rpcdata, (caddr_t)&__resp,
				timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_flash_image_update call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_flash_image_update_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_flash_image_update_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_send_file(const char * image_file_path, const int image_flags)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_send_file_rpcdata __req;
	struct qcsapi_send_file_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcimage_file_path = {(char *)image_file_path};
	__rpc_string *p__rpcimage_file_path = (image_file_path) ? &__rpcimage_file_path : NULL;
	__req.image_file_path = p__rpcimage_file_path;

	__req.image_flags = (int)image_flags;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SEND_FILE_REMOTE,
				(xdrproc_t)xdr_qcsapi_send_file_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_send_file_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_send_file call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_send_file_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_send_file_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_pm_set_mode(int mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_pm_set_mode_rpcdata __req;
	struct qcsapi_pm_set_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.mode = (int)mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_PM_SET_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_pm_set_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_pm_set_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_pm_set_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_pm_set_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_pm_set_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_pm_get_mode(int * mode)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_pm_get_mode_rpcdata __req;
	struct qcsapi_pm_get_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.mode = (int *)mode;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_PM_GET_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_pm_get_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_pm_get_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_pm_get_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_pm_get_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (mode)
			*mode = *__resp.mode;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_pm_get_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_qpm_level(int * qpm_level)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_qpm_level_rpcdata __req;
	struct qcsapi_get_qpm_level_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.qpm_level = (int *)qpm_level;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_QPM_LEVEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_qpm_level_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_qpm_level_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_qpm_level call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_qpm_level_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (qpm_level)
			*qpm_level = *__resp.qpm_level;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_qpm_level_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_set_host_state(const char * ifname, const uint32_t host_state)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_set_host_state_rpcdata __req;
	struct qcsapi_set_host_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.host_state = (uint32_t)host_state;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_SET_HOST_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_set_host_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_set_host_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_set_host_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_set_host_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_set_host_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_state(const char * ifname, unsigned int param, unsigned int * value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_state_rpcdata __req;
	struct qcsapi_qtm_get_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.param = (unsigned int)param;

	__req.value = (unsigned int *)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (value)
			*value = *__resp.value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_state_all(const char * ifname, struct qcsapi_data_128bytes * value, unsigned int max)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_state_all_rpcdata __req;
	struct qcsapi_qtm_get_state_all_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.value = (__rpc_qcsapi_data_128bytes*)value;

	__req.max = (unsigned int)max;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_STATE_ALL_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_state_all_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_state_all_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_state_all call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_state_all_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.value && value)
			memcpy(value, __resp.value, sizeof(*value));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_state_all_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_set_state(const char * ifname, unsigned int param, unsigned int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_set_state_rpcdata __req;
	struct qcsapi_qtm_set_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.param = (unsigned int)param;

	__req.value = (unsigned int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_SET_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_set_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_set_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_set_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_set_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_set_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_config(const char * ifname, unsigned int param, unsigned int * value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_config_rpcdata __req;
	struct qcsapi_qtm_get_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.param = (unsigned int)param;

	__req.value = (unsigned int *)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (value)
			*value = *__resp.value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_config_all(const char * ifname, struct qcsapi_data_1Kbytes * value, unsigned int max)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_config_all_rpcdata __req;
	struct qcsapi_qtm_get_config_all_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.value = (__rpc_qcsapi_data_1Kbytes*)value;

	__req.max = (unsigned int)max;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_CONFIG_ALL_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_config_all_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_config_all_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_config_all call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_config_all_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.value && value)
			memcpy(value, __resp.value, sizeof(*value));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_config_all_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_set_config(const char * ifname, unsigned int param, unsigned int value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_set_config_rpcdata __req;
	struct qcsapi_qtm_set_config_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.param = (unsigned int)param;

	__req.value = (unsigned int)value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_SET_CONFIG_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_set_config_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_set_config_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_set_config call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_set_config_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_set_config_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_add_rule(const char * ifname, const struct qcsapi_data_128bytes * entry)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_add_rule_rpcdata __req;
	struct qcsapi_qtm_add_rule_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.entry = (__rpc_qcsapi_data_128bytes*)entry;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_ADD_RULE_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_add_rule_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_add_rule_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_add_rule call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_add_rule_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_add_rule_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_del_rule(const char * ifname, const struct qcsapi_data_128bytes * entry)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_del_rule_rpcdata __req;
	struct qcsapi_qtm_del_rule_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.entry = (__rpc_qcsapi_data_128bytes*)entry;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_DEL_RULE_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_del_rule_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_del_rule_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_del_rule call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_del_rule_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_del_rule_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_del_rule_index(const char * ifname, unsigned int index)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_del_rule_index_rpcdata __req;
	struct qcsapi_qtm_del_rule_index_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.index = (unsigned int)index;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_DEL_RULE_INDEX_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_del_rule_index_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_del_rule_index_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_del_rule_index call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_del_rule_index_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_del_rule_index_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_rule(const char * ifname, struct qcsapi_data_3Kbytes * entries, unsigned int max_entries)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_rule_rpcdata __req;
	struct qcsapi_qtm_get_rule_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.entries = (__rpc_qcsapi_data_3Kbytes*)entries;

	__req.max_entries = (unsigned int)max_entries;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_RULE_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_rule_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_rule_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_rule call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_rule_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.entries && entries)
			memcpy(entries, __resp.entries, sizeof(*entries));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_rule_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_strm(const char * ifname, struct qcsapi_data_4Kbytes * strms, unsigned int max_entries, int show_all)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_strm_rpcdata __req;
	struct qcsapi_qtm_get_strm_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.strms = (__rpc_qcsapi_data_4Kbytes*)strms;

	__req.max_entries = (unsigned int)max_entries;

	__req.show_all = (int)show_all;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_STRM_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_strm_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_strm_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_strm call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_strm_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.strms && strms)
			memcpy(strms, __resp.strms, sizeof(*strms));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_strm_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_stats(const char * ifname, struct qcsapi_data_512bytes * stats)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_stats_rpcdata __req;
	struct qcsapi_qtm_get_stats_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.stats = (__rpc_qcsapi_data_512bytes*)stats;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_STATS_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_stats_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_stats_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_stats call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_stats_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.stats && stats)
			memcpy(stats, __resp.stats, sizeof(*stats));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_stats_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_qtm_get_inactive_flags(const char * ifname, unsigned long * flags)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_qtm_get_inactive_flags_rpcdata __req;
	struct qcsapi_qtm_get_inactive_flags_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.flags = (unsigned long *)flags;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_QTM_GET_INACTIVE_FLAGS_REMOTE,
				(xdrproc_t)xdr_qcsapi_qtm_get_inactive_flags_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_qtm_get_inactive_flags_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_qtm_get_inactive_flags call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_inactive_flags_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (flags)
			*flags = *__resp.flags;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_qtm_get_inactive_flags_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_run_script(const char * scriptname, const char * param)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_run_script_rpcdata __req;
	struct qcsapi_wifi_run_script_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcscriptname = {(char *)scriptname};
	__rpc_string *p__rpcscriptname = (scriptname) ? &__rpcscriptname : NULL;
	__req.scriptname = p__rpcscriptname;

	__rpc_string __rpcparam = {(char *)param};
	__rpc_string *p__rpcparam = (param) ? &__rpcparam : NULL;
	__req.param = p__rpcparam;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_RUN_SCRIPT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_run_script_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_run_script_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_run_script call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_run_script_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_run_script_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_test_traffic(const char * ifname, uint32_t period)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_test_traffic_rpcdata __req;
	struct qcsapi_wifi_test_traffic_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.period = (uint32_t)period;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_TEST_TRAFFIC_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_test_traffic_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_test_traffic_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_test_traffic call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_test_traffic_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_test_traffic_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_add_ipff(qcsapi_unsigned_int ipaddr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_add_ipff_rpcdata __req;
	struct qcsapi_wifi_add_ipff_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.ipaddr = (unsigned int)ipaddr;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_ADD_IPFF_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_add_ipff_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_add_ipff_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_add_ipff call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_add_ipff_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_add_ipff_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_del_ipff(qcsapi_unsigned_int ipaddr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_del_ipff_rpcdata __req;
	struct qcsapi_wifi_del_ipff_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.ipaddr = (unsigned int)ipaddr;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_DEL_IPFF_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_del_ipff_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_del_ipff_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_del_ipff call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_del_ipff_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_del_ipff_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_ipff(char * buf, int buflen)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_ipff_rpcdata __req;
	struct qcsapi_wifi_get_ipff_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcbuf = {(char *)buf};
	__rpc_string *p__rpcbuf = (buf) ? &__rpcbuf : NULL;
	__req.buf = p__rpcbuf;

	__req.buflen = (int)buflen;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_IPFF_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_ipff_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_ipff_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_ipff call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ipff_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (buf && __resp.buf)
			strcpy(buf, __resp.buf->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ipff_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_rts_threshold(const char * ifname, qcsapi_unsigned_int * rts_threshold)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_rts_threshold_rpcdata __req;
	struct qcsapi_wifi_get_rts_threshold_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.rts_threshold = (unsigned int *)rts_threshold;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_RTS_THRESHOLD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_rts_threshold_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_rts_threshold_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_rts_threshold call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rts_threshold_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (rts_threshold)
			*rts_threshold = *__resp.rts_threshold;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_rts_threshold_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_rts_threshold(const char * ifname, qcsapi_unsigned_int rts_threshold)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_rts_threshold_rpcdata __req;
	struct qcsapi_wifi_set_rts_threshold_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.rts_threshold = (unsigned int)rts_threshold;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_RTS_THRESHOLD_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_rts_threshold_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_rts_threshold_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_rts_threshold call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_rts_threshold_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_rts_threshold_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_nss_cap(const char * ifname, const qcsapi_mimo_type modulation, const qcsapi_unsigned_int nss)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_nss_cap_rpcdata __req;
	struct qcsapi_wifi_set_nss_cap_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.modulation = (int)modulation;

	__req.nss = (unsigned int)nss;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_NSS_CAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_nss_cap_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_nss_cap_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_nss_cap call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_nss_cap_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_nss_cap_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_nss_cap(const char * ifname, const qcsapi_mimo_type modulation, qcsapi_unsigned_int * nss)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_nss_cap_rpcdata __req;
	struct qcsapi_wifi_get_nss_cap_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.modulation = (int)modulation;

	__req.nss = (unsigned int *)nss;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_NSS_CAP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_nss_cap_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_nss_cap_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_nss_cap call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_nss_cap_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (nss)
			*nss = *__resp.nss;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_nss_cap_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tx_amsdu(const char * ifname, int * enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tx_amsdu_rpcdata __req;
	struct qcsapi_wifi_get_tx_amsdu_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (int *)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TX_AMSDU_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_amsdu_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tx_amsdu_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tx_amsdu call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_amsdu_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (enable)
			*enable = *__resp.enable;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tx_amsdu_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_tx_amsdu(const char * ifname, int enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_tx_amsdu_rpcdata __req;
	struct qcsapi_wifi_set_tx_amsdu_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable = (int)enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_TX_AMSDU_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_tx_amsdu_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_tx_amsdu_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_tx_amsdu call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tx_amsdu_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tx_amsdu_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_disassoc_reason(const char * ifname, qcsapi_unsigned_int * reason)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_disassoc_reason_rpcdata __req;
	struct qcsapi_wifi_get_disassoc_reason_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.reason = (unsigned int *)reason;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_DISASSOC_REASON_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_disassoc_reason_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_disassoc_reason_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_disassoc_reason call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_disassoc_reason_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (reason)
			*reason = *__resp.reason;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_disassoc_reason_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_block_bss(const char * ifname, const qcsapi_unsigned_int flag)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_block_bss_rpcdata __req;
	struct qcsapi_wifi_block_bss_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.flag = (unsigned int)flag;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_BLOCK_BSS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_block_bss_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_block_bss_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_block_bss call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_block_bss_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_block_bss_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_verify_repeater_mode()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_verify_repeater_mode_rpcdata __req;
	struct qcsapi_wifi_verify_repeater_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_VERIFY_REPEATER_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_verify_repeater_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_verify_repeater_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_verify_repeater_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_verify_repeater_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_verify_repeater_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_ap_interface_name(const char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_ap_interface_name_rpcdata __req;
	struct qcsapi_wifi_set_ap_interface_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_AP_INTERFACE_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_ap_interface_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_ap_interface_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_ap_interface_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ap_interface_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_ap_interface_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_ap_interface_name(char * ifname)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_ap_interface_name_rpcdata __req;
	struct qcsapi_wifi_get_ap_interface_name_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_AP_INTERFACE_NAME_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_ap_interface_name_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_ap_interface_name_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_ap_interface_name call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ap_interface_name_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (ifname && __resp.ifname)
			strcpy(ifname, __resp.ifname->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_ap_interface_name_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_get_temperature_info(int * temp_exter, int * temp_inter, int * temp_bbic)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_get_temperature_info_rpcdata __req;
	struct qcsapi_get_temperature_info_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.temp_exter = (int *)temp_exter;

	__req.temp_inter = (int *)temp_inter;

	__req.temp_bbic = (int *)temp_bbic;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_GET_TEMPERATURE_INFO_REMOTE,
				(xdrproc_t)xdr_qcsapi_get_temperature_info_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_get_temperature_info_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_get_temperature_info call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_get_temperature_info_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (temp_exter)
			*temp_exter = *__resp.temp_exter;
	}
	if (__resp.return_code >= 0) {
		if (temp_inter)
			*temp_inter = *__resp.temp_inter;
	}
	if (__resp.return_code >= 0) {
		if (temp_bbic)
			*temp_bbic = *__resp.temp_bbic;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_get_temperature_info_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_set_test_mode(qcsapi_unsigned_int channel, qcsapi_unsigned_int antenna, qcsapi_unsigned_int mcs, qcsapi_unsigned_int bw, qcsapi_unsigned_int pkt_size, qcsapi_unsigned_int eleven_n, qcsapi_unsigned_int bf)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_set_test_mode_rpcdata __req;
	struct qcsapi_calcmd_set_test_mode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.channel = (unsigned int)channel;

	__req.antenna = (unsigned int)antenna;

	__req.mcs = (unsigned int)mcs;

	__req.bw = (unsigned int)bw;

	__req.pkt_size = (unsigned int)pkt_size;

	__req.eleven_n = (unsigned int)eleven_n;

	__req.bf = (unsigned int)bf;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_SET_TEST_MODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_set_test_mode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_set_test_mode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_set_test_mode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_set_test_mode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_set_test_mode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_show_test_packet(qcsapi_unsigned_int * tx_packet_num, qcsapi_unsigned_int * rx_packet_num, qcsapi_unsigned_int * crc_packet_num)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_show_test_packet_rpcdata __req;
	struct qcsapi_calcmd_show_test_packet_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.tx_packet_num = (unsigned int *)tx_packet_num;

	__req.rx_packet_num = (unsigned int *)rx_packet_num;

	__req.crc_packet_num = (unsigned int *)crc_packet_num;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_SHOW_TEST_PACKET_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_show_test_packet_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_show_test_packet_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_show_test_packet call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_show_test_packet_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (tx_packet_num)
			*tx_packet_num = *__resp.tx_packet_num;
	}
	if (__resp.return_code >= 0) {
		if (rx_packet_num)
			*rx_packet_num = *__resp.rx_packet_num;
	}
	if (__resp.return_code >= 0) {
		if (crc_packet_num)
			*crc_packet_num = *__resp.crc_packet_num;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_show_test_packet_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_send_test_packet(qcsapi_unsigned_int to_transmit_packet_num)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_send_test_packet_rpcdata __req;
	struct qcsapi_calcmd_send_test_packet_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.to_transmit_packet_num = (unsigned int)to_transmit_packet_num;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_SEND_TEST_PACKET_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_send_test_packet_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_send_test_packet_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_send_test_packet call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_send_test_packet_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_send_test_packet_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_stop_test_packet()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_stop_test_packet_rpcdata __req;
	struct qcsapi_calcmd_stop_test_packet_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_STOP_TEST_PACKET_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_stop_test_packet_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_stop_test_packet_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_stop_test_packet call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_stop_test_packet_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_stop_test_packet_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_send_dc_cw_signal(qcsapi_unsigned_int channel)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_send_dc_cw_signal_rpcdata __req;
	struct qcsapi_calcmd_send_dc_cw_signal_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.channel = (unsigned int)channel;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_SEND_DC_CW_SIGNAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_send_dc_cw_signal_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_send_dc_cw_signal_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_send_dc_cw_signal call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_send_dc_cw_signal_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_send_dc_cw_signal_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_stop_dc_cw_signal()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_stop_dc_cw_signal_rpcdata __req;
	struct qcsapi_calcmd_stop_dc_cw_signal_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_STOP_DC_CW_SIGNAL_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_stop_dc_cw_signal_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_stop_dc_cw_signal_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_stop_dc_cw_signal call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_stop_dc_cw_signal_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_stop_dc_cw_signal_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_test_mode_antenna_sel(qcsapi_unsigned_int * antenna_bit_mask)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_test_mode_antenna_sel_rpcdata __req;
	struct qcsapi_calcmd_get_test_mode_antenna_sel_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.antenna_bit_mask = (unsigned int *)antenna_bit_mask;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_TEST_MODE_ANTENNA_SEL_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_antenna_sel_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_antenna_sel_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_test_mode_antenna_sel call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_antenna_sel_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (antenna_bit_mask)
			*antenna_bit_mask = *__resp.antenna_bit_mask;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_antenna_sel_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_test_mode_mcs(qcsapi_unsigned_int * test_mode_mcs)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_test_mode_mcs_rpcdata __req;
	struct qcsapi_calcmd_get_test_mode_mcs_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.test_mode_mcs = (unsigned int *)test_mode_mcs;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_TEST_MODE_MCS_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_mcs_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_mcs_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_test_mode_mcs call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_mcs_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (test_mode_mcs)
			*test_mode_mcs = *__resp.test_mode_mcs;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_mcs_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_test_mode_bw(qcsapi_unsigned_int * test_mode_bw)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_test_mode_bw_rpcdata __req;
	struct qcsapi_calcmd_get_test_mode_bw_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.test_mode_bw = (unsigned int *)test_mode_bw;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_TEST_MODE_BW_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_bw_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_bw_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_test_mode_bw call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_bw_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (test_mode_bw)
			*test_mode_bw = *__resp.test_mode_bw;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_bw_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_tx_power(qcsapi_calcmd_tx_power_rsp * tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_tx_power_rpcdata __req;
	struct qcsapi_calcmd_get_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.tx_power = (__rpc_qcsapi_calcmd_tx_power_rsp*)tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.tx_power && tx_power)
			memcpy(tx_power, __resp.tx_power, sizeof(*tx_power));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_set_tx_power(qcsapi_unsigned_int tx_power)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_set_tx_power_rpcdata __req;
	struct qcsapi_calcmd_set_tx_power_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.tx_power = (unsigned int)tx_power;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_SET_TX_POWER_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_set_tx_power_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_set_tx_power_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_set_tx_power call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_set_tx_power_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_set_tx_power_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_test_mode_rssi(qcsapi_calcmd_rssi_rsp * test_mode_rssi)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_test_mode_rssi_rpcdata __req;
	struct qcsapi_calcmd_get_test_mode_rssi_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.test_mode_rssi = (__rpc_qcsapi_calcmd_rssi_rsp*)test_mode_rssi;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_TEST_MODE_RSSI_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_rssi_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_rssi_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_test_mode_rssi call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_rssi_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.test_mode_rssi && test_mode_rssi)
			memcpy(test_mode_rssi, __resp.test_mode_rssi, sizeof(*test_mode_rssi));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_test_mode_rssi_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_set_mac_filter(int q_num, int sec_enable, const qcsapi_mac_addr mac_addr)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_set_mac_filter_rpcdata __req;
	struct qcsapi_calcmd_set_mac_filter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.q_num = (int)q_num;

	__req.sec_enable = (int)sec_enable;

	struct __rpc_qcsapi_mac_addr __rpcmac_addr;
	if (mac_addr) {
		memcpy(__rpcmac_addr.data, mac_addr, sizeof(__rpcmac_addr));
		__req.mac_addr = &__rpcmac_addr;
	} else {
		__req.mac_addr = NULL;
	}
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_SET_MAC_FILTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_set_mac_filter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_set_mac_filter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_set_mac_filter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_set_mac_filter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_set_mac_filter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_antenna_count(qcsapi_unsigned_int * antenna_count)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_antenna_count_rpcdata __req;
	struct qcsapi_calcmd_get_antenna_count_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__req.antenna_count = (unsigned int *)antenna_count;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_ANTENNA_COUNT_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_antenna_count_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_antenna_count_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_antenna_count call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_antenna_count_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (antenna_count)
			*antenna_count = *__resp.antenna_count;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_antenna_count_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_clear_counter()
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_clear_counter_rpcdata __req;
	struct qcsapi_calcmd_clear_counter_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_CLEAR_COUNTER_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_clear_counter_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_clear_counter_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_clear_counter call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_clear_counter_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_clear_counter_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_calcmd_get_info(string_1024 output_info)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_calcmd_get_info_rpcdata __req;
	struct qcsapi_calcmd_get_info_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcoutput_info = {(char *)output_info};
	__rpc_string *p__rpcoutput_info = (output_info) ? &__rpcoutput_info : NULL;
	__req.output_info = p__rpcoutput_info;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_CALCMD_GET_INFO_REMOTE,
				(xdrproc_t)xdr_qcsapi_calcmd_get_info_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_calcmd_get_info_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_calcmd_get_info call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_info_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (output_info && __resp.output_info)
			strcpy(output_info, __resp.output_info->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_calcmd_get_info_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wowlan_set_match_type(const char * ifname, const uint32_t wowlan_match)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wowlan_set_match_type_rpcdata __req;
	struct qcsapi_wowlan_set_match_type_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.wowlan_match = (uint32_t)wowlan_match;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WOWLAN_SET_MATCH_TYPE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wowlan_set_match_type_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wowlan_set_match_type_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wowlan_set_match_type call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_match_type_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_match_type_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wowlan_set_L2_type(const char * ifname, const uint32_t ether_type)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wowlan_set_L2_type_rpcdata __req;
	struct qcsapi_wowlan_set_L2_type_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.ether_type = (uint32_t)ether_type;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WOWLAN_SET_L2_TYPE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wowlan_set_L2_type_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wowlan_set_L2_type_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wowlan_set_L2_type call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_L2_type_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_L2_type_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wowlan_set_udp_port(const char * ifname, const uint32_t udp_port)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wowlan_set_udp_port_rpcdata __req;
	struct qcsapi_wowlan_set_udp_port_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.udp_port = (uint32_t)udp_port;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WOWLAN_SET_UDP_PORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wowlan_set_udp_port_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wowlan_set_udp_port_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wowlan_set_udp_port call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_udp_port_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_udp_port_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wowlan_set_magic_pattern(const char * ifname, struct qcsapi_data_256bytes * pattern, uint32_t len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wowlan_set_magic_pattern_rpcdata __req;
	struct qcsapi_wowlan_set_magic_pattern_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.pattern = (__rpc_qcsapi_data_256bytes*)pattern;

	__req.len = (uint32_t)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WOWLAN_SET_MAGIC_PATTERN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wowlan_set_magic_pattern_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wowlan_set_magic_pattern_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wowlan_set_magic_pattern call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_magic_pattern_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.pattern && pattern)
			memcpy(pattern, __resp.pattern, sizeof(*pattern));
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wowlan_set_magic_pattern_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_wowlan_get_host_state(const char * ifname, uint16_t * p_value, uint32_t * len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_wowlan_get_host_state_rpcdata __req;
	struct qcsapi_wifi_wowlan_get_host_state_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_value = (uint16_t *)p_value;

	__req.len = (uint32_t *)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_WOWLAN_GET_HOST_STATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_host_state_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_host_state_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_wowlan_get_host_state call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_host_state_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (__resp.return_code >= 0) {
		if (len)
			*len = *__resp.len;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_host_state_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_wowlan_get_match_type(const char * ifname, uint16_t * p_value, uint32_t * len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_wowlan_get_match_type_rpcdata __req;
	struct qcsapi_wifi_wowlan_get_match_type_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_value = (uint16_t *)p_value;

	__req.len = (uint32_t *)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_WOWLAN_GET_MATCH_TYPE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_match_type_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_match_type_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_wowlan_get_match_type call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_match_type_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (__resp.return_code >= 0) {
		if (len)
			*len = *__resp.len;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_match_type_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_wowlan_get_l2_type(const char * ifname, uint16_t * p_value, uint32_t * len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_wowlan_get_l2_type_rpcdata __req;
	struct qcsapi_wifi_wowlan_get_l2_type_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_value = (uint16_t *)p_value;

	__req.len = (uint32_t *)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_WOWLAN_GET_L2_TYPE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_l2_type_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_l2_type_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_wowlan_get_l2_type call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_l2_type_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (__resp.return_code >= 0) {
		if (len)
			*len = *__resp.len;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_l2_type_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_wowlan_get_udp_port(const char * ifname, uint16_t * p_value, uint32_t * len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_wowlan_get_udp_port_rpcdata __req;
	struct qcsapi_wifi_wowlan_get_udp_port_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_value = (uint16_t *)p_value;

	__req.len = (uint32_t *)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_WOWLAN_GET_UDP_PORT_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_udp_port_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_udp_port_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_wowlan_get_udp_port call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_udp_port_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (__resp.return_code >= 0) {
		if (len)
			*len = *__resp.len;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_udp_port_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_wowlan_get_magic_pattern(const char * ifname, struct qcsapi_data_256bytes * p_value, uint32_t * len)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_wowlan_get_magic_pattern_rpcdata __req;
	struct qcsapi_wifi_wowlan_get_magic_pattern_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_value = (__rpc_qcsapi_data_256bytes*)p_value;

	__req.len = (uint32_t *)len;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_WOWLAN_GET_MAGIC_PATTERN_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_magic_pattern_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_wowlan_get_magic_pattern_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_wowlan_get_magic_pattern call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_magic_pattern_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (__resp.p_value && p_value)
			memcpy(p_value, __resp.p_value, sizeof(*p_value));
	}
	if (__resp.return_code >= 0) {
		if (len)
			*len = *__resp.len;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_wowlan_get_magic_pattern_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_enable_mu(const char * ifname, const unsigned int mu_enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_enable_mu_rpcdata __req;
	struct qcsapi_wifi_set_enable_mu_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.mu_enable = (unsigned int)mu_enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_ENABLE_MU_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_enable_mu_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_enable_mu_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_enable_mu call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_enable_mu_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_enable_mu_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_enable_mu(const char * ifname, unsigned int * mu_enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_enable_mu_rpcdata __req;
	struct qcsapi_wifi_get_enable_mu_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.mu_enable = (unsigned int *)mu_enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_ENABLE_MU_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_enable_mu_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_enable_mu_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_enable_mu call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_enable_mu_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (mu_enable)
			*mu_enable = *__resp.mu_enable;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_enable_mu_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_mu_use_precode(const char * ifname, const unsigned int grp, const unsigned int prec_enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_mu_use_precode_rpcdata __req;
	struct qcsapi_wifi_set_mu_use_precode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.grp = (unsigned int)grp;

	__req.prec_enable = (unsigned int)prec_enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_MU_USE_PRECODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_mu_use_precode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_mu_use_precode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_mu_use_precode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mu_use_precode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mu_use_precode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mu_use_precode(const char * ifname, const unsigned int grp, unsigned int * prec_enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mu_use_precode_rpcdata __req;
	struct qcsapi_wifi_get_mu_use_precode_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.grp = (unsigned int)grp;

	__req.prec_enable = (unsigned int *)prec_enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MU_USE_PRECODE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mu_use_precode_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mu_use_precode_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mu_use_precode call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mu_use_precode_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (prec_enable)
			*prec_enable = *__resp.prec_enable;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mu_use_precode_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_mu_use_eq(const char * ifname, const unsigned int eq_enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_mu_use_eq_rpcdata __req;
	struct qcsapi_wifi_set_mu_use_eq_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.eq_enable = (unsigned int)eq_enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_MU_USE_EQ_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_mu_use_eq_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_mu_use_eq_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_mu_use_eq call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mu_use_eq_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_mu_use_eq_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mu_use_eq(const char * ifname, unsigned int * meq_enable)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mu_use_eq_rpcdata __req;
	struct qcsapi_wifi_get_mu_use_eq_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.meq_enable = (unsigned int *)meq_enable;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MU_USE_EQ_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mu_use_eq_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mu_use_eq_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mu_use_eq call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mu_use_eq_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (meq_enable)
			*meq_enable = *__resp.meq_enable;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mu_use_eq_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_mu_groups(const char * ifname, char * buf, const unsigned int size)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_mu_groups_rpcdata __req;
	struct qcsapi_wifi_get_mu_groups_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__rpc_string __rpcbuf = {(char *)buf};
	__rpc_string *p__rpcbuf = (buf) ? &__rpcbuf : NULL;
	__req.buf = p__rpcbuf;

	__req.size = (unsigned int)size;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_MU_GROUPS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_mu_groups_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_mu_groups_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_mu_groups call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mu_groups_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (buf && __resp.buf)
			strcpy(buf, __resp.buf->data);
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_mu_groups_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_enable_tdls(const char * ifname, uint32_t enable_tdls)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_enable_tdls_rpcdata __req;
	struct qcsapi_wifi_enable_tdls_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.enable_tdls = (uint32_t)enable_tdls;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_ENABLE_TDLS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_enable_tdls_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_enable_tdls_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_enable_tdls call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_enable_tdls_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_enable_tdls_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_enable_tdls_over_qhop(const char * ifname, uint32_t tdls_over_qhop_en)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_enable_tdls_over_qhop_rpcdata __req;
	struct qcsapi_wifi_enable_tdls_over_qhop_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.tdls_over_qhop_en = (uint32_t)tdls_over_qhop_en;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_ENABLE_TDLS_OVER_QHOP_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_enable_tdls_over_qhop_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_enable_tdls_over_qhop_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_enable_tdls_over_qhop call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_enable_tdls_over_qhop_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_enable_tdls_over_qhop_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tdls_status(const char * ifname, uint32_t * p_tdls_status)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tdls_status_rpcdata __req;
	struct qcsapi_wifi_get_tdls_status_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.p_tdls_status = (uint32_t *)p_tdls_status;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TDLS_STATUS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tdls_status_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tdls_status_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tdls_status call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tdls_status_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_tdls_status)
			*p_tdls_status = *__resp.p_tdls_status;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tdls_status_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_set_tdls_params(const char * ifname, qcsapi_tdls_type type, int param_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_set_tdls_params_rpcdata __req;
	struct qcsapi_wifi_set_tdls_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.type = (int)type;

	__req.param_value = (int)param_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_SET_TDLS_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_set_tdls_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_set_tdls_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_set_tdls_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tdls_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_set_tdls_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_get_tdls_params(const char * ifname, qcsapi_tdls_type type, int * p_value)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_get_tdls_params_rpcdata __req;
	struct qcsapi_wifi_get_tdls_params_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.type = (int)type;

	__req.p_value = (int *)p_value;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_GET_TDLS_PARAMS_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_get_tdls_params_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_get_tdls_params_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_get_tdls_params call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tdls_params_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (__resp.return_code >= 0) {
		if (p_value)
			*p_value = *__resp.p_value;
	}
	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_get_tdls_params_rpcdata, (caddr_t)&__resp);

	return ret;
}

int qcsapi_wifi_tdls_operate(const char * ifname, qcsapi_tdls_oper operate, const char * mac_addr_str, int cs_interval)
{
	int retries = 0;
	int ret;
	CLIENT *clnt = qcsapi_adapter_get_client();
	enum clnt_stat __rpcret;
	struct qcsapi_wifi_tdls_operate_rpcdata __req;
	struct qcsapi_wifi_tdls_operate_rpcdata __resp;
	memset(&__req, 0, sizeof(__req));
	memset(&__resp, 0, sizeof(__resp));
	__rpc_string __rpcifname = {(char *)ifname};
	__rpc_string *p__rpcifname = (ifname) ? &__rpcifname : NULL;
	__req.ifname = p__rpcifname;

	__req.operate = (int)operate;

	__rpc_string __rpcmac_addr_str = {(char *)mac_addr_str};
	__rpc_string *p__rpcmac_addr_str = (mac_addr_str) ? &__rpcmac_addr_str : NULL;
	__req.mac_addr_str = p__rpcmac_addr_str;

	__req.cs_interval = (int)cs_interval;

	if (debug) { fprintf(stderr, "%s:%d %s pre\n", __FILE__, __LINE__, __FUNCTION__); }
	client_qcsapi_pre();
	while (1) {
		__rpcret = clnt_call(clnt, QCSAPI_WIFI_TDLS_OPERATE_REMOTE,
				(xdrproc_t)xdr_qcsapi_wifi_tdls_operate_rpcdata, (caddr_t)&__req,
				(xdrproc_t)xdr_qcsapi_wifi_tdls_operate_rpcdata, (caddr_t)&__resp,
				__timeout);
		if (__rpcret == RPC_SUCCESS) {
			client_qcsapi_post(0);
			break;
		} else {
			clnt_perror (clnt, "qcsapi_wifi_tdls_operate call failed");
			clnt_perrno (__rpcret);
			if (retries >= retries_limit) {
				client_qcsapi_post(1);
				xdr_free((xdrproc_t)xdr_qcsapi_wifi_tdls_operate_rpcdata, (caddr_t)&__resp);
				return -ENOLINK;
			}
			retries++;
			client_qcsapi_reconnect();
		}

	}

	if (debug) { fprintf(stderr, "%s:%d %s post\n", __FILE__, __LINE__, __FUNCTION__); }

	ret = __resp.return_code;
	xdr_free((xdrproc_t)xdr_qcsapi_wifi_tdls_operate_rpcdata, (caddr_t)&__resp);

	return ret;
}

