#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autocore
PKG_VERSION:=1
PKG_RELEASE:=35

include $(INCLUDE_DIR)/package.mk

define Package/autocore-arm
  TITLE:=Arm auto core loadbalance script.
  MAINTAINER:=CN_SZTL
  DEPENDS:=@(TARGET_bcm27xx||TARGET_bcm53xx||TARGET_ipq40xx||TARGET_ipq806x||TARGET_rockchip) \
    +TARGET_bcm53xx:nvram \
    +TARGET_ipq40xx:lm-sensors
  VARIANT:=arm
endef

define Package/autocore-x86
  TITLE:=x86/x64 auto core loadbalance script.
  MAINTAINER:=Lean
  DEPENDS:=@TARGET_x86 +bc +lm-sensors +ethtool
  VARIANT:=x86
endef

define Package/autocore-arm/description
  A luci autoconfig hotplug script.
endef

define Package/autocore-x86/description
  A usb autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/autocore-arm/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/arm/index.htm $(1)/etc/index.htm
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/arm/090-cover-index_htm $(1)/etc/uci-defaults/090-cover-index_htm
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/arm/sbin/cpuinfo $(1)/sbin/cpuinfo
endef

define Package/autocore-x86/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/x86/autocore $(1)/etc/init.d/autocore
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/x86/index.htm $(1)/etc/index.htm
	$(INSTALL_DIR) $(1)/sbin
	$(CP) ./files/x86/sbin/* $(1)/sbin
endef

$(eval $(call BuildPackage,autocore-arm))
$(eval $(call BuildPackage,autocore-x86))
