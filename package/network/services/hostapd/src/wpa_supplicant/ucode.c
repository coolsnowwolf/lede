#include "utils/includes.h"
#include "utils/common.h"
#include "utils/ucode.h"
#include "drivers/driver.h"
#include "ap/hostapd.h"
#include "wpa_supplicant_i.h"
#include "wps_supplicant.h"
#include "bss.h"
#include "ucode.h"

static struct wpa_global *wpa_global;
static uc_resource_type_t *global_type, *iface_type;
static uc_value_t *global, *iface_registry;
static uc_vm_t *vm;

static uc_value_t *
wpas_ucode_iface_get_uval(struct wpa_supplicant *wpa_s)
{
	uc_value_t *val;

	if (wpa_s->ucode.idx)
		return wpa_ucode_registry_get(iface_registry, wpa_s->ucode.idx);

	val = uc_resource_new(iface_type, wpa_s);
	wpa_s->ucode.idx = wpa_ucode_registry_add(iface_registry, val);

	return val;
}

static void
wpas_ucode_update_interfaces(void)
{
	uc_value_t *ifs = ucv_object_new(vm);
	struct wpa_supplicant *wpa_s;
	int i;

	for (wpa_s = wpa_global->ifaces; wpa_s; wpa_s = wpa_s->next)
		ucv_object_add(ifs, wpa_s->ifname, ucv_get(wpas_ucode_iface_get_uval(wpa_s)));

	ucv_object_add(ucv_prototype_get(global), "interfaces", ucv_get(ifs));
	ucv_gc(vm);
}

void wpas_ucode_add_bss(struct wpa_supplicant *wpa_s)
{
	uc_value_t *val;

	if (wpa_ucode_call_prepare("iface_add"))
		return;

	uc_value_push(ucv_get(ucv_string_new(wpa_s->ifname)));
	uc_value_push(ucv_get(wpas_ucode_iface_get_uval(wpa_s)));
	ucv_put(wpa_ucode_call(2));
	ucv_gc(vm);
}

void wpas_ucode_free_bss(struct wpa_supplicant *wpa_s)
{
	uc_value_t *val;

	val = wpa_ucode_registry_remove(iface_registry, wpa_s->ucode.idx);
	if (!val)
		return;

	wpa_s->ucode.idx = 0;
	if (wpa_ucode_call_prepare("iface_remove"))
		return;

	uc_value_push(ucv_get(ucv_string_new(wpa_s->ifname)));
	uc_value_push(ucv_get(val));
	ucv_put(wpa_ucode_call(2));
	ucv_gc(vm);
}

void wpas_ucode_update_state(struct wpa_supplicant *wpa_s)
{
	const char *state;
	uc_value_t *val;

	val = wpa_ucode_registry_get(iface_registry, wpa_s->ucode.idx);
	if (!val)
		return;

	if (wpa_ucode_call_prepare("state"))
		return;

	state = wpa_supplicant_state_txt(wpa_s->wpa_state);
	uc_value_push(ucv_get(ucv_string_new(wpa_s->ifname)));
	uc_value_push(ucv_get(val));
	uc_value_push(ucv_get(ucv_string_new(state)));
	ucv_put(wpa_ucode_call(3));
	ucv_gc(vm);
}

void wpas_ucode_event(struct wpa_supplicant *wpa_s, int event, union wpa_event_data *data)
{
	const char *state;
	uc_value_t *val;

	if (event != EVENT_CH_SWITCH_STARTED)
		return;

	val = wpa_ucode_registry_get(iface_registry, wpa_s->ucode.idx);
	if (!val)
		return;

	if (wpa_ucode_call_prepare("event"))
		return;

	uc_value_push(ucv_get(ucv_string_new(wpa_s->ifname)));
	uc_value_push(ucv_get(val));
	uc_value_push(ucv_get(ucv_string_new(event_to_string(event))));
	val = ucv_object_new(vm);
	uc_value_push(ucv_get(val));

	if (event == EVENT_CH_SWITCH_STARTED) {
		ucv_object_add(val, "csa_count", ucv_int64_new(data->ch_switch.count));
		ucv_object_add(val, "frequency", ucv_int64_new(data->ch_switch.freq));
		ucv_object_add(val, "sec_chan_offset", ucv_int64_new(data->ch_switch.ch_offset));
		ucv_object_add(val, "center_freq1", ucv_int64_new(data->ch_switch.cf1));
		ucv_object_add(val, "center_freq2", ucv_int64_new(data->ch_switch.cf2));
	}

	ucv_put(wpa_ucode_call(4));
	ucv_gc(vm);
}

static const char *obj_stringval(uc_value_t *obj, const char *name)
{
	uc_value_t *val = ucv_object_get(obj, name, NULL);

	return ucv_string_get(val);
}

static uc_value_t *
uc_wpas_add_iface(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *info = uc_fn_arg(0);
	uc_value_t *driver = ucv_object_get(info, "driver", NULL);
	uc_value_t *ifname = ucv_object_get(info, "iface", NULL);
	uc_value_t *bridge = ucv_object_get(info, "bridge", NULL);
	uc_value_t *config = ucv_object_get(info, "config", NULL);
	uc_value_t *ctrl = ucv_object_get(info, "ctrl", NULL);
	struct wpa_interface iface;
	int ret = -1;

	if (ucv_type(info) != UC_OBJECT)
		goto out;

	iface = (struct wpa_interface){
		.driver = "nl80211",
		.ifname = ucv_string_get(ifname),
		.bridge_ifname = ucv_string_get(bridge),
		.confname = ucv_string_get(config),
		.ctrl_interface = ucv_string_get(ctrl),
	};

	if (driver) {
		const char *drvname;
		if (ucv_type(driver) != UC_STRING)
			goto out;

		iface.driver = NULL;
		drvname = ucv_string_get(driver);
		for (int i = 0; wpa_drivers[i]; i++) {
			if (!strcmp(drvname, wpa_drivers[i]->name))
				iface.driver = wpa_drivers[i]->name;
		}

		if (!iface.driver)
			goto out;
	}

	if (!iface.ifname || !iface.confname)
		goto out;

	ret = wpa_supplicant_add_iface(wpa_global, &iface, 0) ? 0 : -1;
	wpas_ucode_update_interfaces();

out:
	return ucv_int64_new(ret);
}

static uc_value_t *
uc_wpas_remove_iface(uc_vm_t *vm, size_t nargs)
{
	struct wpa_supplicant *wpa_s = NULL;
	uc_value_t *ifname_arg = uc_fn_arg(0);
	const char *ifname = ucv_string_get(ifname_arg);
	int ret = -1;

	if (!ifname)
		goto out;

	for (wpa_s = wpa_global->ifaces; wpa_s; wpa_s = wpa_s->next)
		if (!strcmp(wpa_s->ifname, ifname))
			break;

	if (!wpa_s)
		goto out;

	ret = wpa_supplicant_remove_iface(wpa_global, wpa_s, 0);
	wpas_ucode_update_interfaces();

out:
	return ucv_int64_new(ret);
}

static uc_value_t *
uc_wpas_iface_status(uc_vm_t *vm, size_t nargs)
{
	struct wpa_supplicant *wpa_s = uc_fn_thisval("wpas.iface");
	struct wpa_bss *bss;
	uc_value_t *ret, *val;

	if (!wpa_s)
		return NULL;

	ret = ucv_object_new(vm);

	val = ucv_string_new(wpa_supplicant_state_txt(wpa_s->wpa_state));
	ucv_object_add(ret, "state", ucv_get(val));

	bss = wpa_s->current_bss;
	if (bss) {
		int sec_chan = 0;
		const u8 *ie;

		ie = wpa_bss_get_ie(bss, WLAN_EID_HT_OPERATION);
		if (ie && ie[1] >= 2) {
			const struct ieee80211_ht_operation *ht_oper;
			int sec;

			ht_oper = (const void *) (ie + 2);
			sec = ht_oper->ht_param & HT_INFO_HT_PARAM_SECONDARY_CHNL_OFF_MASK;
			if (sec == HT_INFO_HT_PARAM_SECONDARY_CHNL_ABOVE)
				sec_chan = 1;
			else if (sec == HT_INFO_HT_PARAM_SECONDARY_CHNL_BELOW)
				sec_chan = -1;
		}

		ucv_object_add(ret, "sec_chan_offset", ucv_int64_new(sec_chan));
		ucv_object_add(ret, "frequency", ucv_int64_new(bss->freq));
	}

#ifdef CONFIG_MESH
	if (wpa_s->ifmsh) {
		struct hostapd_iface *ifmsh = wpa_s->ifmsh;

		ucv_object_add(ret, "sec_chan_offset", ucv_int64_new(ifmsh->conf->secondary_channel));
		ucv_object_add(ret, "frequency", ucv_int64_new(ifmsh->freq));
	}
#endif

	return ret;
}

int wpas_ucode_init(struct wpa_global *gl)
{
	static const uc_function_list_t global_fns[] = {
		{ "printf",	uc_wpa_printf },
		{ "getpid", uc_wpa_getpid },
		{ "add_iface", uc_wpas_add_iface },
		{ "remove_iface", uc_wpas_remove_iface },
	};
	static const uc_function_list_t iface_fns[] = {
		{ "status", uc_wpas_iface_status },
	};
	uc_value_t *data, *proto;

	wpa_global = gl;
	vm = wpa_ucode_create_vm();

	global_type = uc_type_declare(vm, "wpas.global", global_fns, NULL);
	iface_type = uc_type_declare(vm, "wpas.iface", iface_fns, NULL);

	iface_registry = ucv_array_new(vm);
	uc_vm_registry_set(vm, "wpas.iface_registry", iface_registry);

	global = wpa_ucode_global_init("wpas", global_type);

	if (wpa_ucode_run(HOSTAPD_UC_PATH "wpa_supplicant.uc"))
		goto free_vm;

	ucv_gc(vm);
	return 0;

free_vm:
	wpa_ucode_free_vm();
	return -1;
}

void wpas_ucode_free(void)
{
	if (wpa_ucode_call_prepare("shutdown") == 0)
		ucv_put(wpa_ucode_call(0));
	wpa_ucode_free_vm();
}
