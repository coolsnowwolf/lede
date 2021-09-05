include $(TOPDIR)/rules.mk

PKG_NAME:=netstinky
PKG_VERSION:=1.0.0
PKG_RELEASE:=2

PKG_SOURCE:=nsids-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/wanduow/netstinky-ids/releases/download/v$(PKG_VERSION)/
PKG_HASH:=143e8d7a9ee7f58893d92b065aed7eca35798f5807b07d9a78f404ea8614e216
PKG_BUILD_DIR:=$(BUILD_DIR)/nsids-$(PKG_VERSION)

PKG_MAINTAINER:=Andrew Mackintosh <amackint@waikato.ac.nz>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/netstinky
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcap +libuv +libopenssl +umdns
  TITLE:=NetStinky IDS for detecting IoCs on an active network
  URL:=https://netstinky.wand.net.nz/
endef

define Package/netstinky/description
  The NetStinky IDS is a component of the NetStinky suite of tools. It
  monitors the traffic on the LAN interfaces of your router for Indications
  of Compromise (IoCs), drawn from an auto-updating list of definitions.
  IoCs are subsequently reported to the NetStinky smartphone applications.
endef

CONFIGURE_ARGS += \
	--enable-mdns=no \
	--enable-updates

define Package/netstinky/conffiles
/etc/config/netstinky
endef

define Package/netstinky/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/nsids \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) \
		./files/nsids.init \
		$(1)/etc/init.d/nsids

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) \
		./files/nsids.conf \
		$(1)/etc/config/netstinky
endef

$(eval $(call BuildPackage,netstinky))

