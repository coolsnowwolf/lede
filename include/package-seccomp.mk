#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

PKG_CONFIG_DEPENDS+= CONFIG_KERNEL_SECCOMP

ifeq ($(CONFIG_KERNEL_SECCOMP),y)
  define InstallSeccomp
	$(INSTALL_DIR) $(1)/etc/seccomp
	$(INSTALL_DATA) $(2) $(1)/etc/seccomp/
  endef
endif
