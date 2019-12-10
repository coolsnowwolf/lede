#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=librouteros
PKG_SOURCE_DATE:=2018-07-19
PKG_SOURCE_VERSION:=c485c777ffbbbd87c3d72d843af36ba016803cae
PKG_RELEASE:=1

PKG_MAINTAINER:=Sven Roederer <devel-sven@geroedel.de>
PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/octo/librouteros/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=427e071fe270ff6c08e32a10e5beff2add4205e6c864b142f950efdb8d2245a4
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/librouteros
  TITLE:=A library that talks to MikroTik devices.
  URL:=https://octo.github.io/librouteros/
  SECTION:=libs
  CATEGORY:=Libraries
endef

define Package/librouteros/description
  librouteros is a library to communicate with RouterOS, the operating system of MikroTik's RouterBoards.
  It uses the API port provided by those systems to connect and talk to the devices. librouteros is a
  low-level library in that it abstracts the network protocol used but has next to no knowledge about the
  commands and responses available
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/routeros_*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/librouteros.{a,la,so*} $(1)/usr/lib/
endef

define Package/librouteros/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/librouteros.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,librouteros))
