# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015-2020 OpenWrt.org

PKG_CONFIG_DEPENDS+= CONFIG_KERNEL_SECCOMP

ifeq ($(CONFIG_KERNEL_SECCOMP),y)
  define InstallSeccomp
	$(INSTALL_DIR) $(1)/etc/seccomp
	$(INSTALL_DATA) $(2) $(1)/etc/seccomp/
  endef
endif
