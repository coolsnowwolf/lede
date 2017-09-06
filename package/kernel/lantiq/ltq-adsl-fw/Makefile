#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ltq-adsl-fw
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/ltq-dsl-fw-$(PKG_VERSION)
PKG_SOURCE:=ltq-dsl-fw-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://mirror2.openwrt.org/sources/
PKG_HASH:=28676d41c4b76e5bf7a2c5eae106a61fb96b93eabc0cb71120575fff9997269f
PKG_MAINTAINER:=John Crispin <john@phrozen.org>

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/kmod-ltq-adsl-fw-template
  TITLE+=Firmware Annex-$(1) $(2)
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  VARIANT:= $(2)-fw-$(1)
  SOC:=$(2)
  ANNEX:=$(1)
  URL:=http://www.lantiq.com/
  DEPENDS:=@$(3) +kmod-ltq-adsl-$(2)
endef

Package/kmod-ltq-adsl-danube-fw-a=$(call Package/kmod-ltq-adsl-fw-template,a,danube,(TARGET_lantiq_xway||TARGET_lantiq_xway_legacy))
Package/kmod-ltq-adsl-danube-fw-b=$(call Package/kmod-ltq-adsl-fw-template,b,danube,(TARGET_lantiq_xway||TARGET_lantiq_xway_legacy))
Package/kmod-ltq-adsl-ar9-fw-a=$(call Package/kmod-ltq-adsl-fw-template,a,ar9,TARGET_lantiq_xway)
Package/kmod-ltq-adsl-ar9-fw-b=$(call Package/kmod-ltq-adsl-fw-template,b,ar9,TARGET_lantiq_xway)
Package/kmod-ltq-adsl-ase-fw-a=$(call Package/kmod-ltq-adsl-fw-template,a,ase,TARGET_lantiq_ase)
Package/kmod-ltq-adsl-ase-fw-b=$(call Package/kmod-ltq-adsl-fw-template,b,ase,TARGET_lantiq_ase)

define Build/Compile
endef

define Package/kmod-ltq-adsl-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(CP) $(PKG_BUILD_DIR)/$(FW_NAME)/ltq-dsl-fw-$(ANNEX)-$(SOC).bin $(1)/lib/firmware/
	ln -s /lib/firmware/$(FW_NAME)/ltq-dsl-fw-$(ANNEX)-$(SOC).bin $(1)/lib/firmware/adsl.bin
endef

$(eval $(call BuildPackage,kmod-ltq-adsl-danube-fw-a))
$(eval $(call BuildPackage,kmod-ltq-adsl-danube-fw-b))
$(eval $(call BuildPackage,kmod-ltq-adsl-ase-fw-a))
$(eval $(call BuildPackage,kmod-ltq-adsl-ase-fw-b))
$(eval $(call BuildPackage,kmod-ltq-adsl-ar9-fw-a))
$(eval $(call BuildPackage,kmod-ltq-adsl-ar9-fw-b))
