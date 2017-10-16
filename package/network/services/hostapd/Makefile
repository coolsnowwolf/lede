# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hostapd
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://w1.fi/hostap.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2017-08-24
PKG_SOURCE_VERSION:=c2d4f2eb5dba0b5c5a8c5805823084da958a9b52
PKG_MIRROR_HASH:=c6ad9a73fc1ae0ba8bc48f71cf14394b274bc9c2c1d1b53c2775f08312597e74

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-3-Clause

PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_WPA_SUPPLICANT_NO_TIMESTAMP_CHECK \
	CONFIG_PACKAGE_kmod-ath9k \
	CONFIG_PACKAGE_kmod-cfg80211 \
	CONFIG_PACKAGE_hostapd \
	CONFIG_PACKAGE_hostapd-mini \
	CONFIG_WPA_RFKILL_SUPPORT \
	CONFIG_DRIVER_WEXT_SUPPORT \
	CONFIG_DRIVER_11N_SUPPORT \
	CONFIG_DRIVER_11AC_SUPPORT

LOCAL_TYPE=$(strip \
		$(if $(findstring wpad,$(BUILD_VARIANT)),wpad, \
		$(if $(findstring supplicant,$(BUILD_VARIANT)),supplicant, \
		hostapd \
	)))
LOCAL_VARIANT=$(patsubst wpad-%,%,$(patsubst supplicant-%,%,$(BUILD_VARIANT)))
CONFIG_VARIANT:=$(LOCAL_VARIANT)
ifeq ($(LOCAL_VARIANT),mesh)
  CONFIG_VARIANT:=full
endif

ifneq ($(LOCAL_TYPE),hostapd)
  ifeq ($(LOCAL_VARIANT),full)
    PKG_CONFIG_DEPENDS += \
		CONFIG_WPA_SUPPLICANT_INTERNAL \
		CONFIG_WPA_SUPPLICANT_OPENSSL
  endif
endif

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

STAMP_CONFIGURED:=$(STAMP_CONFIGURED)_$(CONFIG_WPA_MSG_MIN_PRIORITY)

ifneq ($(CONFIG_DRIVER_11N_SUPPORT),)
  HOSTAPD_IEEE80211N:=y
endif

ifneq ($(CONFIG_DRIVER_11AC_SUPPORT),)
  HOSTAPD_IEEE80211AC:=y
endif

DRIVER_MAKEOPTS= \
	CONFIG_ACS=$(CONFIG_PACKAGE_kmod-cfg80211) \
	CONFIG_DRIVER_NL80211=$(CONFIG_PACKAGE_kmod-cfg80211) \
	CONFIG_IEEE80211N=$(HOSTAPD_IEEE80211N) \
	CONFIG_IEEE80211AC=$(HOSTAPD_IEEE80211AC) \
	CONFIG_DRIVER_WEXT=$(CONFIG_DRIVER_WEXT_SUPPORT) \

ifeq ($(LOCAL_VARIANT),full)
  DRIVER_MAKEOPTS += CONFIG_IEEE80211W=$(CONFIG_DRIVER_11W_SUPPORT)
endif

ifneq ($(LOCAL_TYPE),hostapd)
  ifdef CONFIG_WPA_SUPPLICANT_OPENSSL
    ifeq ($(LOCAL_VARIANT),full)
      DRIVER_MAKEOPTS += CONFIG_TLS=openssl
      TARGET_LDFLAGS += -lcrypto -lssl
    endif
  endif
  ifeq ($(LOCAL_VARIANT),mesh)
    DRIVER_MAKEOPTS += CONFIG_TLS=openssl CONFIG_AP=y CONFIG_SAE=y CONFIG_MESH=y
    TARGET_LDFLAGS += -lcrypto -lssl
  endif
  ifdef CONFIG_WPA_SUPPLICANT_NO_TIMESTAMP_CHECK
    TARGET_CFLAGS += -DNO_TIMESTAMP_CHECK
  endif
  ifdef CONFIG_WPA_RFKILL_SUPPORT
    DRIVER_MAKEOPTS += NEED_RFKILL=y
  endif
  DRIVER_MAKEOPTS += \
	CONFIG_DRIVER_ROBOSWITCH=$(CONFIG_PACKAGE_kmod-switch)
endif

ifdef CONFIG_USE_GLIBC
  TARGET_LDFLAGS += -lrt
  TARGET_LDFLAGS_C += -lrt
endif

DRV_DEPENDS:=+PACKAGE_kmod-cfg80211:libnl-tiny

define Package/hostapd/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=IEEE 802.1x Authenticator
  URL:=http://hostap.epitest.fi/
  DEPENDS:=$(DRV_DEPENDS) +hostapd-common +libubus
endef

define Package/hostapd
$(call Package/hostapd/Default)
  TITLE+= (full)
  VARIANT:=full
  CONFLICTS:=wpad wpad-mini wpad-mesh
endef

define Package/hostapd/description
 This package contains a full featured IEEE 802.1x/WPA/EAP/RADIUS
 Authenticator.
endef

define Package/hostapd-mini
$(call Package/hostapd/Default)
  TITLE+= (WPA-PSK only)
  VARIANT:=mini
  CONFLICTS:=wpad wpad-mini wpad-mesh
endef

define Package/hostapd-mini/description
 This package contains a minimal IEEE 802.1x/WPA Authenticator (WPA-PSK only).
endef

define Package/hostapd-utils
  $(call Package/hostapd/Default)
  TITLE+= (utils)
  DEPENDS:=@PACKAGE_hostapd||PACKAGE_hostapd-mini||PACKAGE_wpad||PACKAGE_wpad-mesh||PACKAGE_wpad-mini
endef

define Package/hostapd-utils/description
 This package contains a command line utility to control the
 IEEE 802.1x/WPA/EAP/RADIUS Authenticator.
endef

define Package/wpad/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=IEEE 802.1x Authenticator/Supplicant
  DEPENDS:=$(DRV_DEPENDS) +hostapd-common +libubus
  URL:=http://hostap.epitest.fi/
endef

define Package/wpad
$(call Package/wpad/Default)
  TITLE+= (full)
  DEPENDS+=+WPA_SUPPLICANT_OPENSSL:libopenssl
  VARIANT:=wpad-full
endef

define Package/wpad/description
 This package contains a full featured IEEE 802.1x/WPA/EAP/RADIUS
 Authenticator and Supplicant
endef

define Package/wpad-mini
$(call Package/wpad/Default)
  TITLE+= (WPA-PSK only)
  VARIANT:=wpad-mini
endef

define Package/wpad-mini/description
 This package contains a minimal IEEE 802.1x/WPA Authenticator and Supplicant (WPA-PSK only).
endef

define Package/wpad-mesh
$(call Package/wpad/Default)
  TITLE+= (with 802.11s mesh and SAE support)
  DEPENDS:=$(DRV_DEPENDS) +libubus +PACKAGE_wpad-mesh:libopenssl @PACKAGE_kmod-cfg80211 @(!TARGET_uml||BROKEN)
  VARIANT:=wpad-mesh
endef

define Package/wpad-mesh/description
 This package contains a minimal IEEE 802.1x/WPA Authenticator and Supplicant (with 802.11s mesh and SAE support).
endef

define Package/wpa-supplicant
  SECTION:=net
  CATEGORY:=Network
  TITLE:=WPA Supplicant
  URL:=http://hostap.epitest.fi/wpa_supplicant/
  DEPENDS:=$(DRV_DEPENDS) +WPA_SUPPLICANT_OPENSSL:libopenssl
  CONFLICTS:=wpad wpad-mini wpad-mesh
  VARIANT:=supplicant-full
endef

define Package/wpa-supplicant/config
	source "$(SOURCE)/Config.in"
endef

define Package/wpa-supplicant-p2p
  $(Package/wpa-supplicant)
  TITLE:=WPA Supplicant (with Wi-Fi P2P support)
  DEPENDS:=$(DRV_DEPENDS) @PACKAGE_kmod-cfg80211
  CONFLICTS:=wpad wpad-mini wpad-mesh
  VARIANT:=supplicant-p2p
endef

define Package/wpa-supplicant-mesh
  $(Package/wpa-supplicant)
  TITLE:=WPA Supplicant (with 802.11s and SAE)
  DEPENDS:=$(DRV_DEPENDS) +PACKAGE_wpa-supplicant-mesh:libopenssl @PACKAGE_kmod-cfg80211 @(!TARGET_uml||BROKEN)
  CONFLICTS:=wpad wpad-mesh wpad-mesh
  VARIANT:=supplicant-mesh
endef

define Package/wpa-supplicant-mini
  $(Package/wpa-supplicant)
  TITLE:=WPA Supplicant (minimal version)
  DEPENDS:=$(DRV_DEPENDS)
  CONFLICTS:=wpad wpad-mini wpad-mesh
  VARIANT:=supplicant-mini
endef

define Package/wpa-cli
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@PACKAGE_wpa-supplicant||PACKAGE_wpa-supplicant-p2p||PACKAGE_wpad-mini||PACKAGE_wpad||PACKAGE_wpad-mesh
  TITLE:=WPA Supplicant command line control utility
endef

define Package/hostapd-common
  TITLE:=hostapd/wpa_supplicant common support files
  SECTION:=net
  CATEGORY:=Network
endef

define Package/eapol-test
  TITLE:=802.1x authentication test utility
  SECTION:=net
  CATEGORY:=Network
  VARIANT:=supplicant-full
  DEPENDS:=$(DRV_DEPENDS) +WPA_SUPPLICANT_OPENSSL:libopenssl
endef


ifneq ($(wildcard $(PKG_BUILD_DIR)/.config_*),$(subst .configured_,.config_,$(STAMP_CONFIGURED)))
  define Build/Configure/rebuild
	$(FIND) $(PKG_BUILD_DIR) -name \*.o -or -name \*.a | $(XARGS) rm -f
	rm -f $(PKG_BUILD_DIR)/hostapd/hostapd
	rm -f $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant
	rm -f $(PKG_BUILD_DIR)/.config_*
	touch $(subst .configured_,.config_,$(STAMP_CONFIGURED))
  endef
endif

define Build/Configure
	$(Build/Configure/rebuild)
	$(if $(wildcard ./files/hostapd-$(CONFIG_VARIANT).config), \
		$(CP) ./files/hostapd-$(CONFIG_VARIANT).config $(PKG_BUILD_DIR)/hostapd/.config \
	)
	$(CP) ./files/wpa_supplicant-$(CONFIG_VARIANT).config $(PKG_BUILD_DIR)/wpa_supplicant/.config
endef

TARGET_CPPFLAGS := \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR)/src/crypto \
	$(TARGET_CPPFLAGS) \
	-DCONFIG_LIBNL20 \
	-D_GNU_SOURCE \
	$(if $(CONFIG_WPA_MSG_MIN_PRIORITY),-DCONFIG_MSG_MIN_PRIORITY=$(CONFIG_WPA_MSG_MIN_PRIORITY))

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections
ifeq ($(findstring supplicant,$(BUILD_VARIANT)),)
  TARGET_LDFLAGS += -lubox -lubus
endif

ifdef CONFIG_PACKAGE_kmod-cfg80211
  TARGET_LDFLAGS += -lm -lnl-tiny
endif

define Build/RunMake
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(1) \
		$(TARGET_CONFIGURE_OPTS) \
		$(DRIVER_MAKEOPTS) \
		LIBS="$(TARGET_LDFLAGS)" \
		LIBS_c="$(TARGET_LDFLAGS_C)" \
		BCHECK= \
		$(2)
endef

define Build/Compile/wpad
	echo ` \
		$(call Build/RunMake,hostapd,-s MULTICALL=1 dump_cflags); \
		$(call Build/RunMake,wpa_supplicant,-s MULTICALL=1 dump_cflags) | \
		sed -e 's,-n ,,g' -e 's^$(TARGET_CFLAGS)^^' \
	` > $(PKG_BUILD_DIR)/.cflags
	sed -i 's/"/\\"/g' $(PKG_BUILD_DIR)/.cflags
	+$(call Build/RunMake,hostapd, \
		CFLAGS="$$$$(cat $(PKG_BUILD_DIR)/.cflags)" \
		MULTICALL=1 \
		hostapd_cli hostapd_multi.a \
	)
	+$(call Build/RunMake,wpa_supplicant, \
		CFLAGS="$$$$(cat $(PKG_BUILD_DIR)/.cflags)" \
		MULTICALL=1 \
		wpa_cli wpa_supplicant_multi.a \
	)
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/wpad \
		$(TARGET_CFLAGS) \
		./files/multicall.c \
		$(PKG_BUILD_DIR)/hostapd/hostapd_multi.a \
		$(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant_multi.a \
		$(TARGET_LDFLAGS)
endef

define Build/Compile/hostapd
	+$(call Build/RunMake,hostapd, \
		hostapd hostapd_cli \
	)
endef

define Build/Compile/supplicant
	+$(call Build/RunMake,wpa_supplicant, \
		wpa_cli wpa_supplicant \
	)
endef

define Build/Compile/supplicant-full
	+$(call Build/RunMake,wpa_supplicant, \
		eapol_test \
	)
endef

define Build/Compile
	$(Build/Compile/$(LOCAL_TYPE))
	$(Build/Compile/$(BUILD_VARIANT))
endef

define Install/hostapd
	$(INSTALL_DIR) $(1)/usr/sbin
endef

define Install/supplicant
	$(INSTALL_DIR) $(1)/usr/sbin
endef

define Package/hostapd-common/install
	$(INSTALL_DIR) $(1)/lib/netifd
	$(INSTALL_DATA) ./files/hostapd.sh $(1)/lib/netifd/hostapd.sh
endef

define Package/hostapd/install
	$(call Install/hostapd,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd $(1)/usr/sbin/
endef
Package/hostapd-mini/install = $(Package/hostapd/install)

ifneq ($(LOCAL_TYPE),supplicant)
  define Package/hostapd-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/rc.button
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd_cli $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/wps-hotplug.sh $(1)/etc/rc.button/wps
  endef
endif

define Package/wpad/install
	$(call Install/hostapd,$(1))
	$(call Install/supplicant,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpad $(1)/usr/sbin/
	$(LN) wpad $(1)/usr/sbin/hostapd
	$(LN) wpad $(1)/usr/sbin/wpa_supplicant
endef
Package/wpad-mini/install = $(Package/wpad/install)
Package/wpad-mesh/install = $(Package/wpad/install)

define Package/wpa-supplicant/install
	$(call Install/supplicant,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant $(1)/usr/sbin/
endef
Package/wpa-supplicant-mini/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-p2p/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-mesh/install = $(Package/wpa-supplicant/install)

ifneq ($(LOCAL_TYPE),hostapd)
  define Package/wpa-cli/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_cli $(1)/usr/sbin/
  endef
endif

ifeq ($(BUILD_VARIANT),supplicant-full)
  define Package/eapol-test/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/eapol_test $(1)/usr/sbin/
  endef
endif

$(eval $(call BuildPackage,hostapd))
$(eval $(call BuildPackage,hostapd-mini))
$(eval $(call BuildPackage,wpad))
$(eval $(call BuildPackage,wpad-mesh))
$(eval $(call BuildPackage,wpad-mini))
$(eval $(call BuildPackage,wpa-supplicant))
$(eval $(call BuildPackage,wpa-supplicant-mesh))
$(eval $(call BuildPackage,wpa-supplicant-mini))
$(eval $(call BuildPackage,wpa-supplicant-p2p))
$(eval $(call BuildPackage,wpa-cli))
$(eval $(call BuildPackage,hostapd-utils))
$(eval $(call BuildPackage,hostapd-common))
$(eval $(call BuildPackage,eapol-test))
