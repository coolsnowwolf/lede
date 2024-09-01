# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=cpufreq
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cpufreq
  TITLE:=CPU Frequency Scaling adjustment tool
  DEPENDS:=@(arm||aarch64)
  PKGARCH:=all
endef

define Build/Compile
endef

define Package/cpufreq/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(CURDIR)/files/cpufreq.config $(1)/etc/config/cpufreq
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(CURDIR)/files/cpufreq.init $(1)/etc/init.d/cpufreq
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(CURDIR)/files/cpufreq.uci $(1)/etc/uci-defaults/10-cpufreq
endef

$(eval $(call BuildPackage,cpufreq))
