#
# Copyright (c) 2021 Sergey Ryazanov <ryazanov.s.a@gmail.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=atheepmgr
PKG_VERSION:=2.1.1
PKG_RELEASE:=2

PKG_SOURCE_VERSION:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rsa9000/$(PKG_NAME).git
PKG_MIRROR_HASH:=dc77354538bd16343afb3e93c50cdd46792ced0f1632c7be0ddf56d2fcd75ba4

PKG_MAINTAINER:=Sergey Ryazanov <ryazanov.s.a@gmail.com>
PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS:=\
	CONFIG_ATHEEPMGR_DANGEROUS_FUNCTIONS \

include $(INCLUDE_DIR)/package.mk

define Package/atheepmgr
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=EEPROM/boarddata management utility for Atheros WLAN chips
  MENU:=1
endef

define Package/atheepmgr/description
  This utility is intended to investigate the content of Atheros WLAN
  chips EEPROM (OTP, boarddata, ART, etc.). It supports data extraction,
  unpacking, parsing, dumping in a text form, saving binary data for
  further analysis, as well as quick editing of some basic fields (e.g.
  MAC address, etc.).

  The utility supports both data access methods: direct EEPROM (OTP)
  access (via the chip) and offline binary dumps handling.

endef

define Package/atheepmgr/config
	if PACKAGE_atheepmgr
		config ATHEEPMGR_DANGEROUS_FUNCTIONS
			bool "Enable dangerous functions"
			default n
			help
			  This option enables some functions, the use of which could cause
			  permanent hardware damages and (or) make the operation of the
			  equipment illegal due to RF spectre usage violation.

			  These functions are intended for use by developers and radio
			  engineers.

			  You have been warned. If unsure, say no.
	endif
endef

MAKE_VARS += \
	OS=Linux \
	CONFIG_CON_PCI=n \
	CONFIG_I_KNOW_WHAT_I_AM_DOING=$(CONFIG_ATHEEPMGR_DANGEROUS_FUNCTIONS) \

define Package/atheepmgr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/atheepmgr $(1)/usr/bin/
endef

$(eval $(call BuildPackage,atheepmgr))
