#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-ppa
PKG_VERSION:=lsdk-1806
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/yangbolu1991/ppa-binary.git
PKG_SOURCE_VERSION:=73ce533bed868717b891e5dbf881428f56f11b4e
PKG_MIRROR_HASH:=d917a692197e8c3c3db6de1f8a4bc0e52dca5fc6fa84a02d68d79072cd6cbc3f

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-ppa/Config
  define Package/layerscape-ppa-$(1)
    SECTION:=firmware
    CATEGORY:=Firmware
    TITLE:=$(2)
    DEPENDS:=@TARGET_layerscape
    CONFIG:=$(3)
  endef
endef

define Package/layerscape-ppa/ls1012ardb
  TITLE:=NXP LS1012ARDB PPA binary
  CONFIG:=ppa-ls1012a.itb
endef

define Package/layerscape-ppa/ls1012afrwy
  TITLE:=NXP LS1012AFRWY PPA binary
  CONFIG:=ppa-ls1012a.itb
endef

define Package/layerscape-ppa/ls1043ardb
  TITLE:=NXP LS1043ARDB PPA binary
  CONFIG:=ppa-ls1043a.itb
endef

define Package/layerscape-ppa/ls1046ardb
  TITLE:=NXP LS1046ARDB PPA binary
  CONFIG:=ppa-ls1046a.itb
endef

define Package/layerscape-ppa/ls1088ardb
  TITLE:=NXP LS1088ARDB PPA binary
  CONFIG:=ppa-ls1088a.itb
endef

define Package/layerscape-ppa/ls2088ardb
  TITLE:=NXP LS2088ARDB PPA binary
  CONFIG:=ppa-ls2088a.itb
endef

define Build/Compile
endef

define Package/layerscape-ppa/Install
  define Package/layerscape-ppa-$(1)/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/$(1)-ppa.itb
  endef
endef

PPAS := \
  ls1012ardb \
  ls1012afrwy \
  ls1043ardb \
  ls1046ardb \
  ls1088ardb \
  ls2088ardb

$(foreach ppa,$(PPAS), \
	$(eval $(Package/layerscape-ppa/$(ppa))) \
	$(eval $(call Package/layerscape-ppa/Config,$(ppa),$(TITLE),$(CONFIG))) \
	$(eval $(call Package/layerscape-ppa/Install,$(ppa))) \
	$(eval $(call BuildPackage,layerscape-ppa-$(ppa))) \
)
