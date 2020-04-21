# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hostapd
PKG_RELEASE:=9

PKG_SOURCE_URL:=http://w1.fi/hostap.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2019-08-08
PKG_SOURCE_VERSION:=ca8c2bd28ad53f431d6ee60ef754e98cfdb4c17b
PKG_MIRROR_HASH:=9d9f1c60afa5324ee17219bd3ec61c1a6fa4043b4187da9bb44e59025d3ed31d

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-3-Clause
PKG_CPE_ID:=cpe:/a:w1.fi:hostapd

PKG_BUILD_PARALLEL:=1
PKG_ASLR_PIE_REGULAR:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_kmod-ath9k \
	CONFIG_PACKAGE_kmod-cfg80211 \
	CONFIG_PACKAGE_hostapd \
	CONFIG_PACKAGE_hostapd-basic \
	CONFIG_PACKAGE_hostapd-mini \
	CONFIG_WPA_RFKILL_SUPPORT \
	CONFIG_DRIVER_WEXT_SUPPORT \
	CONFIG_DRIVER_11N_SUPPORT \
	CONFIG_DRIVER_11AC_SUPPORT \

EAPOL_TEST_PROVIDERS:=eapol-test eapol-test-openssl eapol-test-wolfssl

SUPPLICANT_PROVIDERS:=
HOSTAPD_PROVIDERS:=

LOCAL_TYPE=$(strip \
		$(if $(findstring wpad,$(BUILD_VARIANT)),wpad, \
		$(if $(findstring supplicant,$(BUILD_VARIANT)),supplicant, \
		hostapd \
		)))

LOCAL_AND_LIB_VARIANT=$(patsubst hostapd-%,%,\
		      $(patsubst wpad-%,%,\
		      $(patsubst supplicant-%,%,\
		      $(BUILD_VARIANT)\
		      )))

LOCAL_VARIANT=$(patsubst %-internal,%,\
	      $(patsubst %-openssl,%,\
	      $(patsubst %-wolfssl,%,\
	      $(LOCAL_AND_LIB_VARIANT)\
	      )))

SSL_VARIANT=$(strip \
		$(if $(findstring openssl,$(LOCAL_AND_LIB_VARIANT)),openssl,\
		$(if $(findstring wolfssl,$(LOCAL_AND_LIB_VARIANT)),wolfssl,\
		internal\
		)))

CONFIG_VARIANT:=$(LOCAL_VARIANT)
ifeq ($(LOCAL_VARIANT),mesh)
  CONFIG_VARIANT:=full
endif

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

ifneq ($(LOCAL_VARIANT),mini)
  DRIVER_MAKEOPTS += CONFIG_IEEE80211W=$(CONFIG_DRIVER_11W_SUPPORT)
endif

ifeq ($(LOCAL_VARIANT),full)
  ifeq ($(SSL_VARIANT),openssl)
    DRIVER_MAKEOPTS += CONFIG_TLS=openssl CONFIG_SAE=y CONFIG_OWE=y CONFIG_SUITEB192=y CONFIG_AP=y CONFIG_MESH=y
    TARGET_LDFLAGS += -lcrypto -lssl
  endif
  ifeq ($(SSL_VARIANT),wolfssl)
    DRIVER_MAKEOPTS += CONFIG_TLS=wolfssl CONFIG_WPS_NFC=1 CONFIG_SAE=y CONFIG_OWE=y CONFIG_SUITEB192=y CONFIG_AP=y CONFIG_MESH=y
    TARGET_LDFLAGS += -lwolfssl
  endif
endif

ifneq ($(LOCAL_TYPE),hostapd)
  ifeq ($(LOCAL_VARIANT),mesh)
    ifeq ($(SSL_VARIANT),openssl)
      DRIVER_MAKEOPTS += CONFIG_TLS=openssl CONFIG_AP=y CONFIG_SAE=y CONFIG_MESH=y
      TARGET_LDFLAGS += -lcrypto -lssl
    endif
    ifeq ($(SSL_VARIANT),wolfssl)
      DRIVER_MAKEOPTS += CONFIG_TLS=wolfssl CONFIG_WPS_NFC=1 CONFIG_AP=y CONFIG_SAE=y CONFIG_MESH=y
      TARGET_LDFLAGS += -lwolfssl
    endif
  endif

  ifdef CONFIG_WPA_RFKILL_SUPPORT
    DRIVER_MAKEOPTS += NEED_RFKILL=y
  endif
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
  PROVIDES:=hostapd
  CONFLICTS:=$(HOSTAPD_PROVIDERS)
  HOSTAPD_PROVIDERS+=$(1)
endef

define Package/hostapd
$(call Package/hostapd/Default,$(1))
  TITLE+= (full)
  VARIANT:=full-internal
endef

define Package/hostapd/description
 This package contains a full featured IEEE 802.1x/WPA/EAP/RADIUS
 Authenticator.
endef

define Package/hostapd-openssl
$(call Package/hostapd/Default,$(1))
  TITLE+= (full)
  VARIANT:=full-openssl
  DEPENDS+=+libopenssl
endef

Package/hostapd-openssl/description = $(Package/hostapd/description)

define Package/hostapd-wolfssl
$(call Package/hostapd/Default,$(1))
  TITLE+= (full)
  VARIANT:=full-wolfssl
  DEPENDS+=+libwolfssl
endef

Package/hostapd-wolfssl/description = $(Package/hostapd/description)

define Package/hostapd-basic
$(call Package/hostapd/Default,$(1))
  TITLE+= (WPA-PSK, 11r and 11w)
  VARIANT:=basic
endef

define Package/hostapd-basic/description
 This package contains a basic IEEE 802.1x/WPA Authenticator with WPA-PSK, 802.11r and 802.11w support.
endef

define Package/hostapd-mini
$(call Package/hostapd/Default,$(1))
  TITLE+= (WPA-PSK only)
  VARIANT:=mini
endef

define Package/hostapd-mini/description
 This package contains a minimal IEEE 802.1x/WPA Authenticator (WPA-PSK only).
endef


define Package/wpad/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=IEEE 802.1x Authenticator/Supplicant
  DEPENDS:=$(DRV_DEPENDS) +hostapd-common +libubus
  URL:=http://hostap.epitest.fi/
  PROVIDES:=hostapd wpa-supplicant
  CONFLICTS:=$(HOSTAPD_PROVIDERS) $(SUPPLICANT_PROVIDERS)
  HOSTAPD_PROVIDERS+=$(1)
  SUPPLICANT_PROVIDERS+=$(1)
endef

define Package/wpad
$(call Package/wpad/Default,$(1))
  TITLE+= (full)
  VARIANT:=wpad-full-internal
endef

define Package/wpad/description
 This package contains a full featured IEEE 802.1x/WPA/EAP/RADIUS
 Authenticator and Supplicant
endef

define Package/wpad-openssl
$(call Package/wpad/Default,$(1))
  TITLE+= (full)
  VARIANT:=wpad-full-openssl
  DEPENDS+=+libopenssl
endef

Package/wpad-openssl/description = $(Package/wpad/description)

define Package/wpad-wolfssl
$(call Package/wpad/Default,$(1))
  TITLE+= (full)
  VARIANT:=wpad-full-wolfssl
  DEPENDS+=+libwolfssl
endef

Package/wpad-wolfssl/description = $(Package/wpad/description)

define Package/wpad-basic
$(call Package/wpad/Default,$(1))
  TITLE+= (WPA-PSK, 11r and 11w)
  VARIANT:=wpad-basic
endef

define Package/wpad-basic/description
 This package contains a basic IEEE 802.1x/WPA Authenticator and Supplicant with WPA-PSK, 802.11r and 802.11w support.
endef

define Package/wpad-mini
$(call Package/wpad/Default,$(1))
  TITLE+= (WPA-PSK only)
  VARIANT:=wpad-mini
endef

define Package/wpad-mini/description
 This package contains a minimal IEEE 802.1x/WPA Authenticator and Supplicant (WPA-PSK only).
endef

define Package/wpad-mesh
$(call Package/wpad/Default,$(1))
  TITLE+= (with 802.11s mesh and SAE support)
  DEPENDS+=@PACKAGE_kmod-cfg80211 @(!TARGET_uml||BROKEN)
  PROVIDES+=wpa-supplicant-mesh wpad-mesh
endef

define Package/wpad-mesh/description
 This package contains a minimal IEEE 802.1x/WPA Authenticator and Supplicant (with 802.11s mesh and SAE support).
endef

define Package/wpad-mesh-openssl
$(call Package/wpad-mesh,$(1))
  DEPENDS+=+libopenssl
  VARIANT:=wpad-mesh-openssl
endef

Package/wpad-mesh-openssl/description = $(Package/wpad-mesh/description)

define Package/wpad-mesh-wolfssl
$(call Package/wpad-mesh,$(1))
  DEPENDS+=+libwolfssl
  VARIANT:=wpad-mesh-wolfssl
endef

Package/wpad-mesh-wolfssl/description = $(Package/wpad-mesh/description)


define Package/wpa-supplicant/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=WPA Supplicant
  URL:=http://hostap.epitest.fi/wpa_supplicant/
  DEPENDS:=$(DRV_DEPENDS) +hostapd-common +libubus
  PROVIDES:=wpa-supplicant
  CONFLICTS:=$(SUPPLICANT_PROVIDERS)
  SUPPLICANT_PROVIDERS+=$(1)
endef

define Package/wpa-supplicant
$(call Package/wpa-supplicant/Default,$(1))
  VARIANT:=supplicant-full-internal
endef

define Package/wpa-supplicant-openssl
$(call Package/wpa-supplicant/Default,$(1))
  VARIANT:=supplicant-full-openssl
  DEPENDS+=+libopenssl
endef

define Package/wpa-supplicant-wolfssl
$(call Package/wpa-supplicant/Default,$(1))
  VARIANT:=supplicant-full-wolfssl
  DEPENDS+=+libwolfssl
endef

define Package/wpa-supplicant/config
	source "$(SOURCE)/Config.in"
endef

define Package/wpa-supplicant-p2p
$(call Package/wpa-supplicant/Default,$(1))
  TITLE+= (with Wi-Fi P2P support)
  DEPENDS+=@PACKAGE_kmod-cfg80211
  VARIANT:=supplicant-p2p-internal
endef

define Package/wpa-supplicant-mesh/Default
$(call Package/wpa-supplicant/Default,$(1))
  TITLE+= (with 802.11s and SAE)
  DEPENDS+=@PACKAGE_kmod-cfg80211 @(!TARGET_uml||BROKEN)
  PROVIDES+=wpa-supplicant-mesh
endef

define Package/wpa-supplicant-mesh-openssl
$(call Package/wpa-supplicant-mesh/Default,$(1))
  VARIANT:=supplicant-mesh-openssl
  DEPENDS+=+libopenssl
endef

define Package/wpa-supplicant-mesh-wolfssl
$(call Package/wpa-supplicant-mesh/Default,$(1))
  VARIANT:=supplicant-mesh-wolfssl
  DEPENDS+=+libwolfssl
endef

define Package/wpa-supplicant-basic
$(call Package/wpa-supplicant/Default,$(1))
  TITLE+= (with 11r and 11w)
  VARIANT:=supplicant-basic
endef

define Package/wpa-supplicant-mini
$(call Package/wpa-supplicant/Default,$(1))
  TITLE+= (minimal version)
  VARIANT:=supplicant-mini
endef


define Package/hostapd-common
  TITLE:=hostapd/wpa_supplicant common support files
  SECTION:=net
  CATEGORY:=Network
endef

define Package/hostapd-utils
  SECTION:=net
  CATEGORY:=Network
  TITLE:=IEEE 802.1x Authenticator (utils)
  URL:=http://hostap.epitest.fi/
  DEPENDS:=@$(subst $(space),||,$(foreach pkg,$(HOSTAPD_PROVIDERS),PACKAGE_$(pkg)))
endef

define Package/hostapd-utils/description
 This package contains a command line utility to control the
 IEEE 802.1x/WPA/EAP/RADIUS Authenticator.
endef

define Package/wpa-cli
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@$(subst $(space),||,$(foreach pkg,$(SUPPLICANT_PROVIDERS),PACKAGE_$(pkg)))
  TITLE:=WPA Supplicant command line control utility
endef

define Package/eapol-test
  TITLE:=802.1x authentication test utility
  SECTION:=net
  CATEGORY:=Network
  VARIANT:=supplicant-full-internal
  DEPENDS:=$(DRV_DEPENDS) +libubus
endef

define Package/eapol-test-openssl
  TITLE:=802.1x authentication test utility
  SECTION:=net
  CATEGORY:=Network
  VARIANT:=supplicant-full-openssl
  CONFLICTS:=$(filter-out eapol-test-openssl ,$(EAPOL_TEST_PROVIDERS))
  DEPENDS:=$(DRV_DEPENDS) +libubus +libopenssl
  PROVIDES:=eapol-test
endef

define Package/eapol-test-wolfssl
  TITLE:=802.1x authentication test utility
  SECTION:=net
  CATEGORY:=Network
  VARIANT:=supplicant-full-wolfssl
  CONFLICTS:=$(filter-out eapol-test-openssl ,$(filter-out eapol-test-wolfssl ,$(EAPOL_TEST_PROVIDERS)))
  DEPENDS:=$(DRV_DEPENDS) +libubus +libwolfssl
  PROVIDES:=eapol-test
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

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -flto=jobserver -fuse-linker-plugin -lubox -lubus

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
		AR="$(TARGET_CROSS)gcc-ar" \
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
	+export MAKEFLAGS="$(MAKE_JOBSERVER)"; $(TARGET_CC) -o $(PKG_BUILD_DIR)/wpad \
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

define Build/Compile/supplicant-full-internal
	+$(call Build/RunMake,wpa_supplicant, \
		eapol_test \
	)
endef

define Build/Compile/supplicant-full-openssl
	+$(call Build/RunMake,wpa_supplicant, \
		eapol_test \
	)
endef

define Build/Compile/supplicant-full-wolfssl
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
	$(INSTALL_DIR) $(1)/lib/netifd $(1)/etc/rc.button $(1)/etc/hotplug.d/ieee80211
	$(INSTALL_DATA) ./files/hostapd.sh $(1)/lib/netifd/hostapd.sh
	$(INSTALL_BIN) ./files/hostapd.hotplug $(1)/etc/hotplug.d/ieee80211/20-hostapd
	$(INSTALL_BIN) ./files/wps-hotplug.sh $(1)/etc/rc.button/wps
endef

define Package/hostapd/install
	$(call Install/hostapd,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd $(1)/usr/sbin/
endef
Package/hostapd-basic/install = $(Package/hostapd/install)
Package/hostapd-mini/install = $(Package/hostapd/install)
Package/hostapd-openssl/install = $(Package/hostapd/install)
Package/hostapd-wolfssl/install = $(Package/hostapd/install)

ifneq ($(LOCAL_TYPE),supplicant)
  define Package/hostapd-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd_cli $(1)/usr/sbin/
  endef
endif

define Package/wpad/install
	$(call Install/hostapd,$(1))
	$(call Install/supplicant,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpad $(1)/usr/sbin/
	$(LN) wpad $(1)/usr/sbin/hostapd
	$(LN) wpad $(1)/usr/sbin/wpa_supplicant
endef
Package/wpad-basic/install = $(Package/wpad/install)
Package/wpad-mini/install = $(Package/wpad/install)
Package/wpad-openssl/install = $(Package/wpad/install)
Package/wpad-wolfssl/install = $(Package/wpad/install)
Package/wpad-mesh-openssl/install = $(Package/wpad/install)
Package/wpad-mesh-wolfssl/install = $(Package/wpad/install)

define Package/wpa-supplicant/install
	$(call Install/supplicant,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant $(1)/usr/sbin/
endef
Package/wpa-supplicant-basic/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-mini/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-p2p/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-openssl/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-wolfssl/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-mesh-openssl/install = $(Package/wpa-supplicant/install)
Package/wpa-supplicant-mesh-wolfssl/install = $(Package/wpa-supplicant/install)

ifneq ($(LOCAL_TYPE),hostapd)
  define Package/wpa-cli/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_cli $(1)/usr/sbin/
  endef
endif

ifeq ($(BUILD_VARIANT),supplicant-full-internal)
  define Package/eapol-test/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/eapol_test $(1)/usr/sbin/
  endef
endif

ifeq ($(BUILD_VARIANT),supplicant-full-openssl)
  define Package/eapol-test-openssl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/eapol_test $(1)/usr/sbin/
  endef
endif

ifeq ($(BUILD_VARIANT),supplicant-full-wolfssl)
  define Package/eapol-test-wolfssl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/eapol_test $(1)/usr/sbin/
  endef
endif

$(eval $(call BuildPackage,hostapd))
$(eval $(call BuildPackage,hostapd-basic))
$(eval $(call BuildPackage,hostapd-mini))
$(eval $(call BuildPackage,hostapd-openssl))
$(eval $(call BuildPackage,hostapd-wolfssl))
$(eval $(call BuildPackage,wpad))
$(eval $(call BuildPackage,wpad-mesh-openssl))
$(eval $(call BuildPackage,wpad-mesh-wolfssl))
$(eval $(call BuildPackage,wpad-basic))
$(eval $(call BuildPackage,wpad-mini))
$(eval $(call BuildPackage,wpad-openssl))
$(eval $(call BuildPackage,wpad-wolfssl))
$(eval $(call BuildPackage,wpa-supplicant))
$(eval $(call BuildPackage,wpa-supplicant-mesh-openssl))
$(eval $(call BuildPackage,wpa-supplicant-mesh-wolfssl))
$(eval $(call BuildPackage,wpa-supplicant-basic))
$(eval $(call BuildPackage,wpa-supplicant-mini))
$(eval $(call BuildPackage,wpa-supplicant-p2p))
$(eval $(call BuildPackage,wpa-supplicant-openssl))
$(eval $(call BuildPackage,wpa-supplicant-wolfssl))
$(eval $(call BuildPackage,wpa-cli))
$(eval $(call BuildPackage,hostapd-utils))
$(eval $(call BuildPackage,hostapd-common))
$(eval $(call BuildPackage,eapol-test))
$(eval $(call BuildPackage,eapol-test-openssl))
$(eval $(call BuildPackage,eapol-test-wolfssl))
