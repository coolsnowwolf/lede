include $(TOPDIR)/rules.mk

PKG_NAME:=hs20
PKG_RELEASE:=4

PKG_SOURCE_URL:=http://w1.fi/hostap.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-06-08
PKG_SOURCE_VERSION:=5a8b366233f5585e68a4ffbb604fbb4a848eb325

PKG_SOURCE_NAME:=hostapd
PKG_MIRROR_HASH:=1b2a4947034142587a3f81eac2ea27dd64650e77f8943d973623b47533be2393
PKG_MIRROR_VERSION:=$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_MIRROR_VERSION).tar.xz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_SOURCE_NAME)-$(PKG_MIRROR_VERSION)

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause
PKG_CPE_ID:=cpe:/a:w1.fi:hostapd

define Download/dm-ddf-dtd
  URL:=http://www.openmobilealliance.org/tech/DTD
  FILE:=dm_ddf-v1_3.dtd
  HASH:=9b3e1dc66a2d38fc6e1300e9fb6f06f95d042206dabc94ae989739feaeb0733e
endef

define Download/spp-xsd
  URL:=https://raw.githubusercontent.com/greearb/hostap-ct/7a7b81d0e58c81796c7408a47082c9cc7587aa21/hs20/server/examples/Fedora20/hs20/spp
  URL_FILE:=spp.xsd
  FILE:=spp-v1_0.xsd
  HASH:=83be16db72e96cca8d654d7f0b446cdd017090d5512a4cd5f7172204331fd8f2
endef

PKG_BUILD_PARALLEL:=1
PKG_ASLR_PIE_REGULAR:=1

ifdef CONFIG_USE_GLIBC
  TARGET_LDFLAGS += -lrt
  TARGET_LDFLAGS_C += -lrt
endif

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/hs20-common
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WirelessAPD
  TITLE:=Hotspot 2.0 OSU common files
  URL:=http://hostap.epitest.fi/
endef

define Package/hs20-common/description
Hotspot 2.0 OSU common files

Hotspot 2.0 SPP and OMA DM XML schema/DTD files
endef

define Package/hs20-client
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WirelessAPD
  TITLE:=Hotspot 2.0 OSU client
  URL:=http://hostap.epitest.fi/
  DEPENDS:=+hs20-common +libopenssl +libcurl +libxml2
endef

define Package/hs20-client/description
Hotspot 2.0 OSU client

endef

define Package/hs20-server
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WirelessAPD
  TITLE:=Hotspot 2.0 OSU server
  URL:=http://hostap.epitest.fi/
  DEPENDS:=+hs20-common +libopenssl +libsqlite3 +libxml2 +php7 +php7-cgi +php7-mod-xml +php7-mod-pdo-sqlite +openssl-util +sqlite3-cli +uhttpd +xxd
endef

define Package/hs20-server/description
Hotspot 2.0 OSU server

This implementation and the example configuration is meant only for
testing purposes in a lab environment. This design is not secure to be
installed in a publicly available Internet server without considerable
amount of modification and review for security issues.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(eval $(call Download,dm-ddf-dtd))
	$(eval $(call Download,spp-xsd))
endef

define Build/Configure
	$(CP) ./files/hostapd.config $(PKG_BUILD_DIR)/hostapd/.config
endef

define Build/RunMake
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(1) \
		$(TARGET_CONFIGURE_OPTS) \
		LIBS="$(TARGET_LDFLAGS) $(2)" \
		LIBS_c="$(TARGET_LDFLAGS_C)" \
		AR="$(TARGET_CROSS)gcc-ar" \
		BCHECK= CONFIG_NO_BROWSER=y \
		$(3)
endef

define Build/Compile
	$(call Build/RunMake,hostapd,-lssl -lcrypto -lsqlite3,hostapd hlr_auc_gw)
	$(call Build/RunMake,hs20/server,-lsqlite3 -lxml2,all)
	$(call Build/RunMake,hs20/client,-lssl -lcrypto -lcurl -lxml2,all)
endef

define Package/hs20-common/install
	$(INSTALL_DIR) $(1)/etc/hs20/spp
	$(INSTALL_DATA) $(DL_DIR)/dm_ddf-v1_3.dtd $(1)/etc/hs20/spp
	$(INSTALL_DATA) $(DL_DIR)/spp-v1_0.xsd $(1)/etc/hs20/spp/spp.xsd
endef

define Package/hs20-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hs20/client/hs20-osu-client $(1)/usr/sbin/
endef

define Package/hs20-server/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/share/hs20/ca $(1)/www/hs20 $(1)/etc/uci-defaults $(1)/etc/hs20/spp $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd $(1)/usr/sbin/hostapd-hs20-radius-server
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hlr_auc_gw $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hs20/server/hs20_spp_server $(1)/usr/sbin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hs20/server/www/* $(1)/www/hs20
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hs20/server/sql.txt $(1)/usr/share/hs20
	$(CP) $(PKG_BUILD_DIR)/hs20/server/ca/* $(1)/usr/share/hs20/ca
	$(INSTALL_BIN) ./files/hs20-server.defaults $(1)/etc/uci-defaults/hs20-server
	$(INSTALL_CONF) ./files/hs20.config $(1)/etc/config/hs20
	$(INSTALL_BIN) ./files/hs20.init $(1)/etc/init.d/hs20
endef

$(eval $(call BuildPackage,hs20-common))
$(eval $(call BuildPackage,hs20-client))
$(eval $(call BuildPackage,hs20-server))
