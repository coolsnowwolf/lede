#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dbus
PKG_VERSION:=1.13.18
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://dbus.freedesktop.org/releases/dbus
PKG_HASH:=8078f5c25e34ab907ce06905d969dc8ef0ccbec367e1e1707c7ecf8460f4254e

PKG_MAINTAINER:=
PKG_LICENSE:=AFL-2.1
PKG_CPE_ID:=cpe:/a:freedesktop:dbus

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/dbus/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Simple interprocess messaging system
  URL:=https://dbus.freedesktop.org/
endef

define Package/dbus/Default/description
 D-Bus is a message bus system, a simple way for applications to talk to one
 another. In addition to interprocess communication, D-Bus helps coordinate
 process lifecycle; it makes it simple and reliable to code a "single instance"
 application or daemon, and to launch applications and daemons on demand when
 their services are needed.
endef

define Package/libdbus
$(call Package/dbus/Default)
  CATEGORY:=Libraries
  TITLE+= (library)
  DEPENDS:= +libpthread
endef

define Package/libdbus/Description
$(call Package/dbus/Default/description)
 This package contains the D-Bus shared library.
endef

define Package/dbus
$(call Package/dbus/Default)
 TITLE+= (daemon)
 DEPENDS:= +libexpat +libdbus
endef

define Package/dbus/Description
$(call Package/dbus/Default/description)
 This package contains the D-Bus daemon.
endef

define Package/dbus-utils
$(call Package/dbus/Default)
 TITLE+= (utilities)
 DEPENDS:= +dbus
endef

define Package/dbus-utils/Description
$(call Package/dbus/Default/description)
 This package contains D-Bus utilities.
endef

CMAKE_OPTIONS += \
	-DDBUS_SYSTEM_SOCKET=/var/run/dbus/system_bus_socket \
	-DDBUS_SESSION_SOCKET_DIR=/tmp \
	-DDBUS_SYSTEM_PID_FILE=/var/run/dbus.pid \
	-DDBUS_BUILD_TESTS=OFF \
	-DDBUS_LINUX=ON \
	-DDBUS_DISABLE_ASSERT=ON \
	-DBUS_ENABLE_STATS=OFF \
	-DDBUS_ENABLE_CONTAINERS=OFF \
	-DENABLE_SYSTEMD=OFF \
	-DDBUS_BUS_ENABLE_SYSTEMD=OFF \
	-DHAVE_SYSTEMD=OFF \
	-DDBUS_WITH_GLIB=OFF \
	-DDBUS_ENABLE_VERBOSE_MODE=OFF \
	-DDBUS_DISABLE_CHECKS=ON \
	-DDBUS_BUILD_X11=OFF \
	-DDBUS_ENABLE_DOXYGEN_DOCS=OFF \
	-DENABLE_QT_HELP=OFF

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/dbus-1.0 $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/dbus-1.0/include/dbus/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/dbus-1.0/include/dbus/*.h \
		$(1)/usr/lib/dbus-1.0/include/dbus/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libdbus-1.so* \
		$(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/dbus-1.0 $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/dbus-1.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libdbus/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdbus-1.so.* $(1)/usr/lib/
endef

define Package/dbus/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dbus-cleanup-sockets $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dbus-daemon $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dbus-launch $(1)/usr/bin/dbus-launch.real
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dbus-uuidgen $(1)/usr/bin/
	$(INSTALL_BIN) ./files/dbus-launch $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/dbus-daemon-launch-helper $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dbus.init $(1)/etc/init.d/dbus
	$(INSTALL_DIR) $(1)/usr/share/dbus-1
	$(CP) $(PKG_INSTALL_DIR)/usr/share/dbus-1 $(1)/usr/share/
endef

define Package/dbus-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) 	$(PKG_INSTALL_DIR)/usr/bin/dbus-{monitor,send,update-activation-environment} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libdbus))
$(eval $(call BuildPackage,dbus))
$(eval $(call BuildPackage,dbus-utils))
