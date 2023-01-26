#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>

#include "ucode/module.h"

#define err_return_int(err, ...) do { set_error(err, __VA_ARGS__); return -1; } while(0)
#define err_return(err, ...) do { set_error(err, __VA_ARGS__); return NULL; } while(0)
#define TRUE ucv_boolean_new(true)

static uc_resource_type_t *module_type, *map_type, *map_iter_type, *program_type;
static uc_value_t *registry;
static uc_vm_t *debug_vm;

static struct {
	int code;
	char *msg;
} last_error;

struct uc_bpf_fd {
	int fd;
	bool close;
};

struct uc_bpf_map {
	struct uc_bpf_fd fd; /* must be first */
	unsigned int key_size, val_size;
};

struct uc_bpf_map_iter {
	int fd;
	unsigned int key_size;
	bool has_next;
	uint8_t key[];
};

__attribute__((format(printf, 2, 3))) static void
set_error(int errcode, const char *fmt, ...)
{
	va_list ap;

	free(last_error.msg);

	last_error.code = errcode;
	last_error.msg = NULL;

	if (fmt) {
		va_start(ap, fmt);
		xvasprintf(&last_error.msg, fmt, ap);
		va_end(ap);
	}
}

static void init_env(void)
{
	static bool init_done = false;
	struct rlimit limit = {
		.rlim_cur = RLIM_INFINITY,
		.rlim_max = RLIM_INFINITY,
	};

	if (init_done)
		return;

	setrlimit(RLIMIT_MEMLOCK, &limit);
	init_done = true;
}

static uc_value_t *
uc_bpf_error(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *numeric = uc_fn_arg(0);
	const char *msg = last_error.msg;
	int code = last_error.code;
	uc_stringbuf_t *buf;
	const char *s;

	if (last_error.code == 0)
		return NULL;

	set_error(0, NULL);

	if (ucv_is_truish(numeric))
		return ucv_int64_new(code);

	buf = ucv_stringbuf_new();
	if (code < 0 && msg) {
		ucv_stringbuf_addstr(buf, msg, strlen(msg));
	} else {
		s = strerror(code);
		ucv_stringbuf_addstr(buf, s, strlen(s));
		if (msg)
			ucv_stringbuf_printf(buf, ": %s", msg);
	}

	return ucv_stringbuf_finish(buf);
}

static int
uc_bpf_module_set_opts(struct bpf_object *obj, uc_value_t *opts)
{
	uc_value_t *val;

	if (!opts)
		return 0;

	if (ucv_type(opts) != UC_OBJECT)
		err_return_int(EINVAL, "options argument");

	if ((val = ucv_object_get(opts, "rodata", NULL)) != NULL) {
		struct bpf_map *map = NULL;

		if (ucv_type(val) != UC_STRING)
			err_return_int(EINVAL, "rodata type");

		while ((map = bpf_object__next_map(obj, map)) != NULL) {
			if (!strstr(bpf_map__name(map), ".rodata"))
				continue;

			break;
		}

		if (!map)
			err_return_int(errno, "rodata map");

		if (bpf_map__set_initial_value(map, ucv_string_get(val),
					       ucv_string_length(val)))
			err_return_int(errno, "rodata");
	}

	if ((val = ucv_object_get(opts, "program-type", NULL)) != NULL) {
		if (ucv_type(val) != UC_OBJECT)
			err_return_int(EINVAL, "prog_types argument");

		ucv_object_foreach(val, name, type) {
			struct bpf_program *prog;

			if (ucv_type(type) != UC_INTEGER)
				err_return_int(EINVAL, "program %s type", name);

			prog = bpf_object__find_program_by_name(obj, name);
			if (!prog)
				err_return_int(-1, "program %s not found", name);

			bpf_program__set_type(prog, ucv_int64_get(type));
		}
	}

	return 0;
}

static uc_value_t *
uc_bpf_open_module(uc_vm_t *vm, size_t nargs)
{
	DECLARE_LIBBPF_OPTS(bpf_object_open_opts, bpf_opts);
	uc_value_t *path = uc_fn_arg(0);
	uc_value_t *opts = uc_fn_arg(1);
	struct bpf_object *obj;

	if (ucv_type(path) != UC_STRING)
		err_return(EINVAL, "module path");

	init_env();
	obj = bpf_object__open_file(ucv_string_get(path), &bpf_opts);
	if (libbpf_get_error(obj))
		err_return(errno, NULL);

	if (uc_bpf_module_set_opts(obj, opts)) {
		bpf_object__close(obj);
		return NULL;
	}

	if (bpf_object__load(obj)) {
		bpf_object__close(obj);
		err_return(errno, NULL);
	}

	return uc_resource_new(module_type, obj);
}

static uc_value_t *
uc_bpf_map_create(int fd, unsigned int key_size, unsigned int val_size, bool close)
{
	struct uc_bpf_map *uc_map;

	uc_map = xalloc(sizeof(*uc_map));
	uc_map->fd.fd = fd;
	uc_map->key_size = key_size;
	uc_map->val_size = val_size;
	uc_map->fd.close = close;

	return uc_resource_new(map_type, uc_map);
}

static uc_value_t *
uc_bpf_open_map(uc_vm_t *vm, size_t nargs)
{
	struct bpf_map_info info;
	uc_value_t *path = uc_fn_arg(0);
	__u32 len = sizeof(info);
	int err;
	int fd;

	if (ucv_type(path) != UC_STRING)
		err_return(EINVAL, "module path");

	fd = bpf_obj_get(ucv_string_get(path));
	if (fd < 0)
		err_return(errno, NULL);

	err = bpf_obj_get_info_by_fd(fd, &info, &len);
	if (err) {
		close(fd);
		err_return(errno, NULL);
	}

	return uc_bpf_map_create(fd, info.key_size, info.value_size, true);
}

static uc_value_t *
uc_bpf_open_program(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *path = uc_fn_arg(0);
	struct uc_bpf_fd *f;
	int fd;

	if (ucv_type(path) != UC_STRING)
		err_return(EINVAL, "module path");

	fd = bpf_obj_get(ucv_string_get(path));
	if (fd < 0)
		err_return(errno, NULL);

	f = xalloc(sizeof(*f));
	f->fd = fd;
	f->close = true;

	return uc_resource_new(program_type, f);
}

static uc_value_t *
uc_bpf_module_get_maps(uc_vm_t *vm, size_t nargs)
{
	struct bpf_object *obj = uc_fn_thisval("bpf.module");
	struct bpf_map *map = NULL;
	uc_value_t *rv;
	int i = 0;

	if (!obj)
		err_return(EINVAL, NULL);

	rv = ucv_array_new(vm);
	bpf_object__for_each_map(map, obj)
		ucv_array_set(rv, i++, ucv_string_new(bpf_map__name(map)));

	return rv;
}

static uc_value_t *
uc_bpf_module_get_map(uc_vm_t *vm, size_t nargs)
{
	struct bpf_object *obj = uc_fn_thisval("bpf.module");
	struct bpf_map *map;
	uc_value_t *name = uc_fn_arg(0);
	int fd;

	if (!obj || ucv_type(name) != UC_STRING)
		err_return(EINVAL, NULL);

	map = bpf_object__find_map_by_name(obj, ucv_string_get(name));
	if (!map)
		err_return(errno, NULL);

	fd = bpf_map__fd(map);
	if (fd < 0)
		err_return(EINVAL, NULL);

	return uc_bpf_map_create(fd, bpf_map__key_size(map), bpf_map__value_size(map), false);
}

static uc_value_t *
uc_bpf_module_get_programs(uc_vm_t *vm, size_t nargs)
{
	struct bpf_object *obj = uc_fn_thisval("bpf.module");
	struct bpf_program *prog = NULL;
	uc_value_t *rv;
	int i = 0;

	if (!obj)
		err_return(EINVAL, NULL);

	rv = ucv_array_new(vm);
	bpf_object__for_each_program(prog, obj)
		ucv_array_set(rv, i++, ucv_string_new(bpf_program__name(prog)));

	return rv;
}

static uc_value_t *
uc_bpf_module_get_program(uc_vm_t *vm, size_t nargs)
{
	struct bpf_object *obj = uc_fn_thisval("bpf.module");
	struct bpf_program *prog;
	uc_value_t *name = uc_fn_arg(0);
	struct uc_bpf_fd *f;
	int fd;

	if (!obj || !name || ucv_type(name) != UC_STRING)
		err_return(EINVAL, NULL);

	prog = bpf_object__find_program_by_name(obj, ucv_string_get(name));
	if (!prog)
		err_return(errno, NULL);

	fd = bpf_program__fd(prog);
	if (fd < 0)
		err_return(EINVAL, NULL);

	f = xalloc(sizeof(*f));
	f->fd = fd;

	return uc_resource_new(program_type, f);
}

static void *
uc_bpf_map_arg(uc_value_t *val, const char *kind, unsigned int size)
{
	static union {
		uint32_t u32;
		uint64_t u64;
	} val_int;

	switch (ucv_type(val)) {
	case UC_INTEGER:
		if (size == 4)
			val_int.u32 = ucv_int64_get(val);
		else if (size == 8)
			val_int.u64 = ucv_int64_get(val);
		else
			break;

		return &val_int;
	case UC_STRING:
		if (size != ucv_string_length(val))
			break;

		return ucv_string_get(val);
	default:
		err_return(EINVAL, "%s type", kind);
	}

	err_return(EINVAL, "%s size mismatch (expected: %d)", kind, size);
}

static uc_value_t *
uc_bpf_map_get(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map *map = uc_fn_thisval("bpf.map");
	uc_value_t *a_key = uc_fn_arg(0);
	void *key, *val;

	if (!map)
		err_return(EINVAL, NULL);

	key = uc_bpf_map_arg(a_key, "key", map->key_size);
	if (!key)
		return NULL;

	val = alloca(map->val_size);
	if (bpf_map_lookup_elem(map->fd.fd, key, val))
		return NULL;

	return ucv_string_new_length(val, map->val_size);
}

static uc_value_t *
uc_bpf_map_set(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map *map = uc_fn_thisval("bpf.map");
	uc_value_t *a_key = uc_fn_arg(0);
	uc_value_t *a_val = uc_fn_arg(1);
	uc_value_t *a_flags = uc_fn_arg(2);
	uint64_t flags;
	void *key, *val;

	if (!map)
		err_return(EINVAL, NULL);

	key = uc_bpf_map_arg(a_key, "key", map->key_size);
	if (!key)
		return NULL;

	val = uc_bpf_map_arg(a_val, "value", map->val_size);
	if (!val)
		return NULL;

	if (!a_flags)
		flags = BPF_ANY;
	else if (ucv_type(a_flags) != UC_INTEGER)
		err_return(EINVAL, "flags");
	else
		flags = ucv_int64_get(a_flags);

	if (bpf_map_update_elem(map->fd.fd, key, val, flags))
		return NULL;

	return ucv_string_new_length(val, map->val_size);
}

static uc_value_t *
uc_bpf_map_delete(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map *map = uc_fn_thisval("bpf.map");
	uc_value_t *a_key = uc_fn_arg(0);
	uc_value_t *a_return = uc_fn_arg(1);
	void *key, *val = NULL;
	int ret;

	if (!map)
		err_return(EINVAL, NULL);

	key = uc_bpf_map_arg(a_key, "key", map->key_size);
	if (!key)
		return NULL;

	if (!ucv_is_truish(a_return)) {
		ret = bpf_map_delete_elem(map->fd.fd, key);

		return ucv_boolean_new(ret == 0);
	}

	val = alloca(map->val_size);
	if (bpf_map_lookup_and_delete_elem(map->fd.fd, key, val))
		return NULL;

	return ucv_string_new_length(val, map->val_size);
}

static uc_value_t *
uc_bpf_map_delete_all(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map *map = uc_fn_thisval("bpf.map");
	uc_value_t *filter = uc_fn_arg(0);
	bool has_next;
	void *key, *next;

	if (!map)
		err_return(EINVAL, NULL);

	key = alloca(map->key_size);
	next = alloca(map->key_size);
	has_next = !bpf_map_get_next_key(map->fd.fd, NULL, next);
	while (has_next) {
		bool skip = false;

		memcpy(key, next, map->key_size);
		has_next = !bpf_map_get_next_key(map->fd.fd, next, next);

		if (ucv_is_callable(filter)) {
			uc_value_t *rv;

			uc_value_push(ucv_get(filter));
			uc_value_push(ucv_string_new_length((const char *)key, map->key_size));
			if (uc_call(1) != EXCEPTION_NONE)
				break;

			rv = uc_vm_stack_pop(vm);
			if (!rv)
				break;

			skip = !ucv_is_truish(rv);
			ucv_put(rv);
		}

		if (!skip)
			bpf_map_delete_elem(map->fd.fd, key);
	}

	return TRUE;
}

static uc_value_t *
uc_bpf_map_iterator(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map *map = uc_fn_thisval("bpf.map");
	struct uc_bpf_map_iter *iter;

	if (!map)
		err_return(EINVAL, NULL);

	iter = xalloc(sizeof(*iter) + map->key_size);
	iter->fd = map->fd.fd;
	iter->key_size = map->key_size;
	iter->has_next = !bpf_map_get_next_key(iter->fd, NULL, &iter->key);

	return uc_resource_new(map_iter_type, iter);
}

static uc_value_t *
uc_bpf_map_iter_next(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map_iter *iter = uc_fn_thisval("bpf.map_iter");
	uc_value_t *rv;

	if (!iter->has_next)
		return NULL;

	rv = ucv_string_new_length((const char *)iter->key, iter->key_size);
	iter->has_next = !bpf_map_get_next_key(iter->fd, &iter->key, &iter->key);

	return rv;
}

static uc_value_t *
uc_bpf_map_iter_next_int(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map_iter *iter = uc_fn_thisval("bpf.map_iter");
	uint64_t intval;
	uc_value_t *rv;

	if (!iter->has_next)
		return NULL;

	if (iter->key_size == 4)
		intval = *(uint32_t *)iter->key;
	else if (iter->key_size == 8)
		intval = *(uint64_t *)iter->key;
	else
		return NULL;

	rv = ucv_int64_new(intval);
	iter->has_next = !bpf_map_get_next_key(iter->fd, &iter->key, &iter->key);

	return rv;
}

static uc_value_t *
uc_bpf_map_foreach(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_map *map = uc_fn_thisval("bpf.map");
	uc_value_t *func = uc_fn_arg(0);
	bool has_next;
	void *key, *next;
	bool ret = false;

	key = alloca(map->key_size);
	next = alloca(map->key_size);
	has_next = !bpf_map_get_next_key(map->fd.fd, NULL, next);

	while (has_next) {
		uc_value_t *rv;
		bool stop;

		memcpy(key, next, map->key_size);
		has_next = !bpf_map_get_next_key(map->fd.fd, next, next);

		uc_value_push(ucv_get(func));
		uc_value_push(ucv_string_new_length((const char *)key, map->key_size));

		if (uc_call(1) != EXCEPTION_NONE)
			break;

		rv = uc_vm_stack_pop(vm);
		stop = (ucv_type(rv) == UC_BOOLEAN && !ucv_boolean_get(rv));
		ucv_put(rv);

		if (stop)
			break;

		ret = true;
	}

	return ucv_boolean_new(ret);
}

static uc_value_t *
uc_bpf_obj_pin(uc_vm_t *vm, size_t nargs, const char *type)
{
	struct uc_bpf_fd *f = uc_fn_thisval(type);
	uc_value_t *path = uc_fn_arg(0);

	if (ucv_type(path) != UC_STRING)
		err_return(EINVAL, NULL);

	if (bpf_obj_pin(f->fd, ucv_string_get(path)))
		err_return(errno, NULL);

	return TRUE;
}

static uc_value_t *
uc_bpf_program_pin(uc_vm_t *vm, size_t nargs)
{
	return uc_bpf_obj_pin(vm, nargs, "bpf.program");
}

static uc_value_t *
uc_bpf_map_pin(uc_vm_t *vm, size_t nargs)
{
	return uc_bpf_obj_pin(vm, nargs, "bpf.map");
}

static uc_value_t *
uc_bpf_set_tc_hook(uc_value_t *ifname, uc_value_t *type, uc_value_t *prio,
		   int fd)
{
	DECLARE_LIBBPF_OPTS(bpf_tc_hook, hook);
	DECLARE_LIBBPF_OPTS(bpf_tc_opts, attach_tc,
			    .handle = 1);
	const char *type_str;
	uint64_t prio_val;

	if (ucv_type(ifname) != UC_STRING || ucv_type(type) != UC_STRING ||
	    ucv_type(prio) != UC_INTEGER)
		err_return(EINVAL, NULL);

	prio_val = ucv_int64_get(prio);
	if (prio_val > 0xffff)
		err_return(EINVAL, NULL);

	type_str = ucv_string_get(type);
	if (!strcmp(type_str, "ingress"))
		hook.attach_point = BPF_TC_INGRESS;
	else if (!strcmp(type_str, "egress"))
		hook.attach_point = BPF_TC_EGRESS;
	else
		err_return(EINVAL, NULL);

	hook.ifindex = if_nametoindex(ucv_string_get(ifname));
	if (!hook.ifindex)
		goto error;

	bpf_tc_hook_create(&hook);
	attach_tc.priority = prio_val;
	if (bpf_tc_detach(&hook, &attach_tc) < 0 && fd < 0)
		goto error;

	if (fd < 0)
		goto out;

	attach_tc.prog_fd = fd;
	if (bpf_tc_attach(&hook, &attach_tc) < 0)
		goto error;

out:
	return TRUE;

error:
	if (fd >= 0)
		err_return(ENOENT, NULL);
	return NULL;
}

static uc_value_t *
uc_bpf_program_tc_attach(uc_vm_t *vm, size_t nargs)
{
	struct uc_bpf_fd *f = uc_fn_thisval("bpf.program");
	uc_value_t *ifname = uc_fn_arg(0);
	uc_value_t *type = uc_fn_arg(1);
	uc_value_t *prio = uc_fn_arg(2);

	if (!f)
		err_return(EINVAL, NULL);

	return uc_bpf_set_tc_hook(ifname, type, prio, f->fd);
}

static uc_value_t *
uc_bpf_tc_detach(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *ifname = uc_fn_arg(0);
	uc_value_t *type = uc_fn_arg(1);
	uc_value_t *prio = uc_fn_arg(2);

	return uc_bpf_set_tc_hook(ifname, type, prio, -1);
}

static int
uc_bpf_debug_print(enum libbpf_print_level level, const char *format,
		   va_list args)
{
	char buf[256], *str = NULL;
	uc_value_t *val;
	va_list ap;
	int size;

	va_copy(ap, args);
	size = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (size > 0 && (unsigned long)size < ARRAY_SIZE(buf) - 1) {
		val = ucv_string_new(buf);
		goto out;
	}

	if (vasprintf(&str, format, args) < 0)
		return 0;

	val = ucv_string_new(str);
	free(str);

out:
	uc_vm_stack_push(debug_vm, ucv_get(ucv_array_get(registry, 0)));
	uc_vm_stack_push(debug_vm, ucv_int64_new(level));
	uc_vm_stack_push(debug_vm, val);
	if (uc_vm_call(debug_vm, false, 2) == EXCEPTION_NONE)
		ucv_put(uc_vm_stack_pop(debug_vm));

	return 0;
}

static uc_value_t *
uc_bpf_set_debug_handler(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *handler = uc_fn_arg(0);

	if (handler && !ucv_is_callable(handler))
		err_return(EINVAL, NULL);

	debug_vm = vm;
	libbpf_set_print(handler ? uc_bpf_debug_print : NULL);

	ucv_array_set(registry, 0, ucv_get(handler));

	return NULL;
}

static void
register_constants(uc_vm_t *vm, uc_value_t *scope)
{
#define ADD_CONST(x) ucv_object_add(scope, #x, ucv_int64_new(x))
	ADD_CONST(BPF_PROG_TYPE_SCHED_CLS);
	ADD_CONST(BPF_PROG_TYPE_SCHED_ACT);

	ADD_CONST(BPF_ANY);
	ADD_CONST(BPF_NOEXIST);
	ADD_CONST(BPF_EXIST);
	ADD_CONST(BPF_F_LOCK);
}

static const uc_function_list_t module_fns[] = {
	{ "get_map",			uc_bpf_module_get_map },
	{ "get_maps",			uc_bpf_module_get_maps },
	{ "get_programs",		uc_bpf_module_get_programs },
	{ "get_program",		uc_bpf_module_get_program },
};

static void module_free(void *ptr)
{
	struct bpf_object *obj = ptr;

	bpf_object__close(obj);
}

static const uc_function_list_t map_fns[] = {
	{ "pin",			uc_bpf_map_pin },
	{ "get",			uc_bpf_map_get },
	{ "set",			uc_bpf_map_set },
	{ "delete",			uc_bpf_map_delete },
	{ "delete_all",			uc_bpf_map_delete_all },
	{ "foreach",			uc_bpf_map_foreach },
	{ "iterator",			uc_bpf_map_iterator },
};

static void uc_bpf_fd_free(void *ptr)
{
	struct uc_bpf_fd *f = ptr;

	if (f->close)
		close(f->fd);
	free(f);
}

static const uc_function_list_t map_iter_fns[] = {
	{ "next",			uc_bpf_map_iter_next },
	{ "next_int",			uc_bpf_map_iter_next_int },
};

static const uc_function_list_t prog_fns[] = {
	{ "pin",			uc_bpf_program_pin },
	{ "tc_attach",			uc_bpf_program_tc_attach },
};

static const uc_function_list_t global_fns[] = {
	{ "error",			uc_bpf_error },
	{ "set_debug_handler",		uc_bpf_set_debug_handler },
	{ "open_module",		uc_bpf_open_module },
	{ "open_map",			uc_bpf_open_map },
	{ "open_program",		uc_bpf_open_program },
	{ "tc_detach",			uc_bpf_tc_detach },
};

void uc_module_init(uc_vm_t *vm, uc_value_t *scope)
{
	uc_function_list_register(scope, global_fns);
	register_constants(vm, scope);

	registry = ucv_array_new(vm);
	uc_vm_registry_set(vm, "bpf.registry", registry);

	module_type = uc_type_declare(vm, "bpf.module", module_fns, module_free);
	map_type = uc_type_declare(vm, "bpf.map", map_fns, uc_bpf_fd_free);
	map_iter_type = uc_type_declare(vm, "bpf.map_iter", map_iter_fns, free);
	program_type = uc_type_declare(vm, "bpf.program", prog_fns, uc_bpf_fd_free);
}
