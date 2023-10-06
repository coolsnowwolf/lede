#ifndef __HOSTAPD_UTILS_UCODE_H
#define __HOSTAPD_UTILS_UCODE_H

#include "utils/includes.h"
#include "utils/common.h"
#include <ucode/lib.h>
#include <ucode/vm.h>

#define HOSTAPD_UC_PATH	"/usr/share/hostap/"

extern uc_value_t *uc_registry;
uc_vm_t *wpa_ucode_create_vm(void);
int wpa_ucode_run(const char *script);
int wpa_ucode_call_prepare(const char *fname);
uc_value_t *wpa_ucode_call(size_t nargs);
void wpa_ucode_free_vm(void);

uc_value_t *wpa_ucode_global_init(const char *name, uc_resource_type_t *global_type);

int wpa_ucode_registry_add(uc_value_t *reg, uc_value_t *val);
uc_value_t *wpa_ucode_registry_get(uc_value_t *reg, int idx);
uc_value_t *wpa_ucode_registry_remove(uc_value_t *reg, int idx);

uc_value_t *uc_wpa_printf(uc_vm_t *vm, size_t nargs);
uc_value_t *uc_wpa_getpid(uc_vm_t *vm, size_t nargs);
uc_value_t *uc_wpa_sha1(uc_vm_t *vm, size_t nargs);
uc_value_t *uc_wpa_freq_info(uc_vm_t *vm, size_t nargs);

#endif
