# SPDX-License-Identifier: GPL-2.0

include $(TOPDIR)/rules.mk

PKG_NAME:=attendedsysupgrade-common
PKG_VERSION:=$(COMMITCOUNT)
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/attendedsysupgrade-common
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Common files neede by attendedsysupgrade packages
  MAINTAINER:=Paul Spooren <mail@aparcar.org>
  DEPENDS:=+rpcd +rpcd-mod-rpcsys
  PKGARCH:=all
endef

define Package/attendedsysupgrade-common/description
	Common needed files for attendedsysupgrade tool

	Manages dependencies and brings settings used by clients.

	UCI options:

	* attendedsysupgrade.server.url
	  URL of compatible upgrade server [1]

	* attendedsysupgrade.client.upgrade_packages
	  Client should request image also if no new release but new packages
	  upgrade are available.

	* attendedsysupgrade.client.advanced_mode
	  Offer advanced options like editing packages before request and show
	  additional information.

	* attendedsysupgrade.client.auto_search
	  Tells the client to automattically search for upgrades

	[1]: https://github.com/aparcar/asu
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/attendedsysupgrade-common/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults/
	$(INSTALL_BIN) ./files/attendedsysupgrade.defaults $(1)/etc/uci-defaults/attendedsysupgrade

	$(INSTALL_DIR) $(1)/etc/opkg/keys/
	$(INSTALL_BIN) ./files/86241a707a30cb7f $(1)/etc/opkg/keys/86241a707a30cb7f
endef

$(eval $(call BuildPackage,attendedsysupgrade-common))
