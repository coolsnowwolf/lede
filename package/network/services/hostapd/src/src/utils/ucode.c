#include <unistd.h>
#include "ucode.h"
#include "utils/eloop.h"
#include "crypto/crypto.h"
#include "crypto/sha1.h"
#include "common/ieee802_11_common.h"
#include <libubox/uloop.h>
#include <ucode/compiler.h>

static uc_value_t *registry;
static uc_vm_t vm;
static struct uloop_timeout gc_timer;

static void uc_gc_timer(struct uloop_timeout *timeout)
{
	ucv_gc(&vm);
}

uc_value_t *uc_wpa_printf(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *level = uc_fn_arg(0);
	uc_value_t *ret, **args;
	uc_cfn_ptr_t _sprintf;
	int l = MSG_INFO;
	int i, start = 0;

	_sprintf = uc_stdlib_function("sprintf");
	if (!sprintf)
		return NULL;

	if (ucv_type(level) == UC_INTEGER) {
		l = ucv_int64_get(level);
		start++;
	}

	if (nargs <= start)
		return NULL;

	ret = _sprintf(vm, nargs - start);
	if (ucv_type(ret) != UC_STRING)
		return NULL;

	wpa_printf(l, "%s", ucv_string_get(ret));
	ucv_put(ret);

	return NULL;
}

uc_value_t *uc_wpa_freq_info(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *freq = uc_fn_arg(0);
	uc_value_t *sec = uc_fn_arg(1);
	int width = ucv_uint64_get(uc_fn_arg(2));
	int freq_val, center_idx, center_ofs;
	enum oper_chan_width chanwidth;
	enum hostapd_hw_mode hw_mode;
	u8 op_class, channel, tmp_channel;
	const char *modestr;
	int sec_channel = 0;
	uc_value_t *ret;

	if (ucv_type(freq) != UC_INTEGER)
		return NULL;

	freq_val = ucv_int64_get(freq);
	if (ucv_type(sec) == UC_INTEGER)
		sec_channel = ucv_int64_get(sec);
	else if (sec)
		return NULL;
	else if (freq_val > 4000)
		sec_channel = (freq_val / 20) & 1 ? 1 : -1;
	else
		sec_channel = freq_val < 2442 ? 1 : -1;

	if (sec_channel != -1 && sec_channel != 1 && sec_channel != 0)
		return NULL;

	switch (width) {
	case 0:
		chanwidth = CONF_OPER_CHWIDTH_USE_HT;
		break;
	case 1:
		chanwidth = CONF_OPER_CHWIDTH_80MHZ;
		break;
	case 2:
		chanwidth = CONF_OPER_CHWIDTH_160MHZ;
		break;
	default:
		return NULL;
	}

	hw_mode = ieee80211_freq_to_channel_ext(freq_val, sec_channel,
						chanwidth, &op_class, &channel);
	switch (hw_mode) {
	case HOSTAPD_MODE_IEEE80211B:
		modestr = "b";
		break;
	case HOSTAPD_MODE_IEEE80211G:
		modestr = "g";
		break;
	case HOSTAPD_MODE_IEEE80211A:
		modestr = "a";
		break;
	case HOSTAPD_MODE_IEEE80211AD:
		modestr = "ad";
		break;
	default:
		return NULL;
	}

	ret = ucv_object_new(vm);
	ucv_object_add(ret, "op_class", ucv_int64_new(op_class));
	ucv_object_add(ret, "channel", ucv_int64_new(channel));
	ucv_object_add(ret, "hw_mode", ucv_int64_new(hw_mode));
	ucv_object_add(ret, "hw_mode_str", ucv_get(ucv_string_new(modestr)));
	ucv_object_add(ret, "sec_channel", ucv_int64_new(sec_channel));
	ucv_object_add(ret, "frequency", ucv_int64_new(freq_val));

	if (!sec_channel)
		return ret;

	if (freq_val >= 5900)
		center_ofs = 0;
	else if (freq_val >= 5745)
		center_ofs = 20;
	else
		center_ofs = 35;
	tmp_channel = channel - center_ofs;
	tmp_channel &= ~((8 << width) - 1);
	center_idx = tmp_channel + center_ofs + (4 << width) - 1;

	if (freq_val < 3000)
		ucv_object_add(ret, "center_seg0_idx", ucv_int64_new(0));
	else
		ucv_object_add(ret, "center_seg0_idx", ucv_int64_new(center_idx));
	center_idx = (center_idx - channel) * 5 + freq_val;
	ucv_object_add(ret, "center_freq1", ucv_int64_new(center_idx));

out:
	return ret;
}

uc_value_t *uc_wpa_getpid(uc_vm_t *vm, size_t nargs)
{
	return ucv_int64_new(getpid());
}

uc_value_t *uc_wpa_sha1(uc_vm_t *vm, size_t nargs)
{
	u8 hash[SHA1_MAC_LEN];
	char hash_hex[2 * ARRAY_SIZE(hash) + 1];
	uc_value_t *val;
	size_t *lens;
	const u8 **args;
	int i;

	if (!nargs)
		return NULL;

	args = alloca(nargs * sizeof(*args));
	lens = alloca(nargs * sizeof(*lens));
	for (i = 0; i < nargs; i++) {
		val = uc_fn_arg(i);
		if (ucv_type(val) != UC_STRING)
			return NULL;

		args[i] = ucv_string_get(val);
		lens[i] = ucv_string_length(val);
	}

	if (sha1_vector(nargs, args, lens, hash))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(hash); i++)
		sprintf(hash_hex + 2 * i, "%02x", hash[i]);

	return ucv_string_new_length(hash_hex, 2 * ARRAY_SIZE(hash));
}

uc_vm_t *wpa_ucode_create_vm(void)
{
	static uc_parse_config_t config = {
		.strict_declarations = true,
		.lstrip_blocks = true,
		.trim_blocks = true,
		.raw_mode = true
	};

	uc_search_path_init(&config.module_search_path);
	uc_search_path_add(&config.module_search_path, HOSTAPD_UC_PATH "*.so");
	uc_search_path_add(&config.module_search_path, HOSTAPD_UC_PATH "*.uc");

	uc_vm_init(&vm, &config);

	uc_stdlib_load(uc_vm_scope_get(&vm));
	eloop_add_uloop();
	gc_timer.cb = uc_gc_timer;

	return &vm;
}

int wpa_ucode_run(const char *script)
{
	uc_source_t *source;
	uc_program_t *prog;
	uc_value_t *ops;
	char *err;
	int ret;

	source = uc_source_new_file(script);
	if (!source)
		return -1;

	prog = uc_compile(vm.config, source, &err);
	uc_source_put(source);
	if (!prog) {
		wpa_printf(MSG_ERROR, "Error loading ucode: %s\n", err);
		return -1;
	}

	ret = uc_vm_execute(&vm, prog, &ops);
	uc_program_put(prog);
	if (ret || !ops)
		return -1;

	registry = ucv_array_new(&vm);
	uc_vm_registry_set(&vm, "hostap.registry", registry);
	ucv_array_set(registry, 0, ucv_get(ops));

	return 0;
}

int wpa_ucode_call_prepare(const char *fname)
{
	uc_value_t *obj, *func;

	if (!registry)
		return -1;

	obj = ucv_array_get(registry, 0);
	if (!obj)
		return -1;

	func = ucv_object_get(obj, fname, NULL);
	if (!ucv_is_callable(func))
		return -1;

	uc_vm_stack_push(&vm, ucv_get(obj));
	uc_vm_stack_push(&vm, ucv_get(func));

	return 0;
}

uc_value_t *wpa_ucode_global_init(const char *name, uc_resource_type_t *global_type)
{
	uc_value_t *global = uc_resource_new(global_type, NULL);
	uc_value_t *proto;

	uc_vm_registry_set(&vm, "hostap.global", global);
	proto = ucv_prototype_get(global);
	ucv_object_add(proto, "data", ucv_get(ucv_object_new(&vm)));

#define ADD_CONST(x) ucv_object_add(proto, #x, ucv_int64_new(x))
	ADD_CONST(MSG_EXCESSIVE);
	ADD_CONST(MSG_MSGDUMP);
	ADD_CONST(MSG_DEBUG);
	ADD_CONST(MSG_INFO);
	ADD_CONST(MSG_WARNING);
	ADD_CONST(MSG_ERROR);
#undef ADD_CONST

	ucv_object_add(uc_vm_scope_get(&vm), name, ucv_get(global));

	return global;
}

int wpa_ucode_registry_add(uc_value_t *reg, uc_value_t *val)
{
	uc_value_t *data;
	int i = 0;

	while (ucv_array_get(reg, i))
		i++;

	ucv_array_set(reg, i, ucv_get(val));

	return i + 1;
}

uc_value_t *wpa_ucode_registry_get(uc_value_t *reg, int idx)
{
	if (!idx)
		return NULL;

	return ucv_array_get(reg, idx - 1);
}

uc_value_t *wpa_ucode_registry_remove(uc_value_t *reg, int idx)
{
	uc_value_t *val = wpa_ucode_registry_get(reg, idx);
	void **dataptr;

	if (!val)
		return NULL;

	ucv_array_set(reg, idx - 1, NULL);
	dataptr = ucv_resource_dataptr(val, NULL);
	if (dataptr)
		*dataptr = NULL;

	return val;
}


uc_value_t *wpa_ucode_call(size_t nargs)
{
	if (uc_vm_call(&vm, true, nargs) != EXCEPTION_NONE)
		return NULL;

	if (!gc_timer.pending)
		uloop_timeout_set(&gc_timer, 10);

	return uc_vm_stack_pop(&vm);
}

void wpa_ucode_free_vm(void)
{
	if (!vm.config)
		return;

	uc_search_path_free(&vm.config->module_search_path);
	uc_vm_free(&vm);
	registry = NULL;
	vm = (uc_vm_t){};
}
