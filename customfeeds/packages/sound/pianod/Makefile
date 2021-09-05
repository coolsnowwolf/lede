#
# Copyright (C) 2015-2017 Ted Hess <thess@kitschensync.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pianod
PKG_VERSION:=174.09
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/thess/pianod-sc/releases/download/$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=744c833ee17a7c95068c6925f4301f342bcad838ad8e48b40a19fd6739533eac

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_LICENSE:=MIT VARIOUS
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pianod
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libao +libfaad2 +libmpg123 +libmbedtls +libjson-c +libgcrypt +libpthread
  TITLE:=Pandora radio daemon
  USERID:=pianod=88:pianod=88
  URL:=http://deviousfish.com/pianod1/
endef

define Package/pianod/description
  pianod is a Pandora music player client with line-oriented and Websocket control interfaces.
  A simple protocol makes for easy clients, mix scripting, integration with a
  home automation system, use as multiple-user music system for home or office.
  Documentation and configuration information can be found at:
      http://deviousfish.com/pianod1/
endef

define Package/pianod-client
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+pianod
  TITLE:=Pandora radio daemon WebUI
  URL:=http://deviousfish.com/pianod1/
endef

define Package/pianod-client/description
  Web client UI and remote player viewer for pianod
  Examples --- (Note: The viewer.* files need not be on the pianod host)
  Main interface: http://pianod-host:4446/pianod
  Remote viewer: file:///<path>/viewer.html?server=pianod-host:4446  --or--
                 http://pianod-host:4446/pianod/viewer.html?server=pianod-host:4446
endef

CONFIGURE_ARGS+= --with-mbedtls

PIANOD_CLIENT:=pianod-client-compiled-51.tar.gz

define Download/pianod-client
  URL:=http://deviousfish.com/Downloads/pianod/
  FILE:=$(PIANOD_CLIENT)
  HASH:=1596ec2da30992e4008bbd6654333b025165399c3c7e22078f35bee6a438c5ad
endef

define Package/pianod/conffiles
/etc/pianod/startscript
/etc/pianod/users
endef

define Build/Prepare
    $(call Build/Prepare/Default)
    ifneq ($(CONFIG_PACKAGE_pianod-client),)
	$(eval $(call Download,pianod-client))
	mkdir -p $(PKG_BUILD_DIR)/pianod-client
	$(TAR) -xf $(DL_DIR)/$(PIANOD_CLIENT) --strip=1 -C $(PKG_BUILD_DIR)/pianod-client
    endif
endef

define Package/pianod-client/Configure
endef

define Package/pianod-client/Compile
endef

define Package/pianod/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pianod $(1)/usr/bin/
# These scripts need ksh (install mksh and link)
#	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/piano $(1)/usr/bin/
#	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/runmix $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/pianod
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/contrib/startscript-example $(1)/etc/pianod/startscript
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pianod.init $(1)/etc/init.d/pianod
endef

define Package/pianod-client/install
	$(INSTALL_DIR) $(1)/www/pianod
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/pianod-client/*.{html,js,gif,jpeg,css} $(1)/www/pianod/
endef

$(eval $(call BuildPackage,pianod))
$(eval $(call BuildPackage,pianod-client))
