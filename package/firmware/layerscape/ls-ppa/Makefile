#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=ls-ppa
PKG_SOURCE_DATE:=2017-10-26
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/yangbolu1991/ppa-binary.git
PKG_SOURCE_VERSION:=76a80ca988affbee67bb17096aefb82ba6d91be7
PKG_MIRROR_HASH:=55e6cfa0f03f2db667ed4270e5aaf43ab830343af3b9724f92e830a1e3a3a25f

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define ppa/Default
	TITLE:=
	CONFIG:=
endef

define ppa/ls1043ardb
	TITLE=PPA firmware $(PKG_VERSION) for NXP ls1043ardb 64b/32b Dev Board
	CONFIG=ppa-ls1043a.itb
endef

define ppa/ls1046ardb
	TITLE=PPA firmware $(PKG_VERSION) for NXP ls1046ardb 64b/32b Dev Board
	CONFIG=ppa-ls1046a.itb
endef

define ppa/ls1012ardb
	TITLE=PPA firmware $(PKG_VERSION) for NXP ls1012ardb 64b/32b Dev Board
	CONFIG=ppa-ls1012a.itb
endef

define ppa/ls1088ardb
	TITLE=PPA firmware $(PKG_VERSION) for NXP ls1088ardb 64b/32b Dev Board
	CONFIG=ppa-ls1088a.itb
endef

define ppa/ls2088ardb
	TITLE=PPA firmware $(PKG_VERSION) for NXP ls2088ardb 64b/32b Dev Board
	CONFIG=ppa-ls2088a.itb
endef

define ppa/ls1012afrdm
	TITLE=PPA firmware $(PKG_VERSION) for NXP ls1012afrdm 64b/32b Dev Board
	CONFIG=ppa-ls1012a.itb
endef

PPAS := \
	ls1043ardb \
	ls1046ardb \
	ls1012ardb \
	ls1088ardb \
	ls2088ardb \
	ls1012afrdm

define Package/ppa/template
define Package/layerscape-ppa-$(1)
	SECTION:=firmware
	CATEGORY:=Firmware
	DEPENDS:= @TARGET_layerscape
	TITLE:=$(2)
	VARIANT:=$(1)
endef
endef

define BuildPPAPackage
	$(eval $(ppa/Default))
	$(eval $(ppa/$(1)))
	$(call Package/ppa/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call ppa/$(BUILD_VARIANT)))
PPA_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
endif

define Build/Compile
endef

define Package/ppa/install/default
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(PPA_CONFIG) $(STAGING_DIR_IMAGE)/$(1)-ppa.itb
endef

define Package/ppa/install/template
define Package/layerscape-ppa-$(1)/install
	$(call Package/ppa/install/default,$(2))
endef
endef

$(foreach r,$(PPAS), \
	$(eval $(call Package/ppa/install/template,$(r),$(r))) \
)

$(foreach r,$(PPAS), \
	$(eval $(call BuildPPAPackage,$(r))) \
	$(eval $(call BuildPackage,layerscape-ppa-$(r))) \
)
