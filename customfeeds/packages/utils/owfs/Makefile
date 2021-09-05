#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=owfs
PKG_VERSION:=3.2p4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/owfs/owfs/releases/download/v$(PKG_VERSION)
PKG_HASH:=af0a5035f3f3df876ca15aea13486bfed6b3ef5409dee016db0be67755c35fcc

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:owfs:owfs

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_CONFIG_DEPENDS:= \
  CONFIG_LIBOW_MASTER_USB \
  CONFIG_LIBOW_MASTER_I2C \
  CONFIG_LIBOW_MASTER_W1 \
  CONFIG_LIBOW_ZEROCONF \
  CONFIG_LIBOW_DEBUG \
  CONFIG_LIBOW_OWTRAFFIC

include $(INCLUDE_DIR)/package.mk

#
# templates
#

define Package/owfs/Default
  TITLE:=OWFS (1-Wire File System)
  URL:=https://github.com/owfs/owfs
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
endef

define Package/owfs/Default/description
  OWFS is a suite of programs that designed to make the 1-wire bus and its
  devices easily accessible. The underlying principle is to create a virtual
  filesystem, with the unique ID being the directory, and the individual
  properties of the device are represented as simple files that can be read
  and written.

  Details of the individual slave or master design are hidden behind a
  consistent interface. The goal is to provide an easy set of tools for a
  software designer to create monitoring or control applications. There are
  some performance enhancements in the implementation, including data caching,
  parallel access to bus masters, and aggregation of device communication.
  Still the fundemental goal has been ease of use, flexibility and correctness
  rather than speed.
endef

define Package/owfs/Server
  $(call Package/owfs/Default)
  DEPENDS:=+libow +libpthread
endef

define Package/owfs/Library
  $(call Package/owfs/Default)
  SECTION:=libs
  CATEGORY:=Libraries
endef

define Package/owfs/Utility
  $(call Package/owfs/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libow
endef

#
# shared libraries
#

define Package/libow
  $(call Package/owfs/Library)
  DEPENDS:= \
    +libpthread \
    +LIBOW_MASTER_USB:libusb-compat \
    +LIBOW_MASTER_W1:kmod-w1
  TITLE:=OWFS - common shared library
endef

define Package/libow/config
  source "$(SOURCE)/Config.in"
endef

define Package/libow/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS library.
endef

define Package/libow-capi
  $(call Package/owfs/Library)
  DEPENDS:=+libow
  TITLE:=OWFS - C-API library
endef

define Package/libow-capi/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS C-API library.
endef

#
# utilities
#

define Package/owshell
  $(call Package/owfs/Utility)
  TITLE:=OWFS - shell utilities
endef

define Package/owshell/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS shell utilities.
endef

define Package/owfs
  $(call Package/owfs/Utility)
  # libfuse depends on kmod-fuse, no need to declare dependency
  DEPENDS+= +libfuse +fuse-utils
  TITLE:=OWFS - fuse file system
endef

define Package/owfs/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS fuse filesystem.
endef

#
# network daemons
#

define Package/owhttpd
  $(call Package/owfs/Server)
  TITLE:=OWFS - http server
endef

define Package/owhttpd/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS http server.
endef

define Package/owftpd
  $(call Package/owfs/Server)
  TITLE:=OWFS - ftp server
endef

define Package/owftpd/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS ftp server.
endef

define Package/owserver
  $(call Package/owfs/Server)
  TITLE:=OWFS - network server
endef

define Package/owserver/description
  $(call Package/owfs/Default/description)

  This package contains the OWFS network server.
endef

CONFIGURE_ARGS += \
	--enable-owftpd \
	--enable-owserver \
	--enable-owhttpd \
	--enable-owfs \
	--with-fuseinclude="$(STAGING_DIR)/usr/include" \
	--with-fuselib="$(STAGING_DIR)/usr/lib" \
	--enable-shared \
	--disable-parport \
	--disable-ownet \
	--disable-owpython \
	--disable-owphp \
	--disable-owtcl \
	--disable-swig \
	--disable-avahi \
	$(if $(CONFIG_LIBOW_MASTER_USB),--enable-usb,--disable-usb) \
	$(if $(CONFIG_LIBOW_MASTER_W1),--enable-w1,--disable-w1) \
	$(if $(CONFIG_LIBOW_MASTER_I2C),--enable-i2c,--disable-i2c) \
	$(if $(CONFIG_LIBOW_ZEROCONF),--enable-zero,--disable-zero) \
	$(if $(CONFIG_LIBOW_DEBUG),--enable-debug,--disable-debug) \
	$(if $(CONFIG_LIBOW_OWTRAFFIC),--enable-owtraffic,--disable-owtraffic)

CONFIGURE_VARS += \
	LDFLAGS="$(TARGET_LDFLAGS) -Wl,-rpath-link=$(STAGING_DIR)/usr/lib -Wl,-rpath-link=$(TOOLCHAIN_DIR)/usr/lib" \
	lt_cv_sys_lib_dlsearch_path_spec="$(STAGING_DIR)/lib $(STAGING_DIR)/usr/lib" \
	lt_cv_sys_lib_search_path_spec="$(STAGING_DIR)/lib $(STAGING_DIR)/usr/lib" \
	shrext_cmds=".so" \

MAKE_FLAGS += \
	CC="$(TARGET_CC)" \
	HOST_CPU="$(PKGARCH)"

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include
	$(CP)	$(PKG_INSTALL_DIR)/usr/include/ow{capi,fs_config}.h $(STAGING_DIR)/usr/include/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib
	$(CP)	$(PKG_INSTALL_DIR)/usr/lib/libow{,capi}*.so* $(STAGING_DIR)/usr/lib/
endef

define Package/owfs/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owfs $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/owfs.conf $(1)/etc/config/owfs
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/owfs.init $(1)/etc/init.d/owfs
	mkdir -p $(1)/mnt/owfs
endef

define Package/owfs/conffiles
/etc/config/owfs
endef

define Package/owshell/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owget $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owread $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owwrite $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owdir $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owpresent $(1)/usr/bin/
endef


define Package/owserver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owserver $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/owserver.conf $(1)/etc/config/owserver
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/owserver.init $(1)/etc/init.d/owserver
endef

define Package/owserver/conffiles
/etc/config/owserver
endef

define Package/owhttpd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owhttpd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/owhttpd.conf $(1)/etc/config/owhttpd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/owhttpd.init $(1)/etc/init.d/owhttpd
endef

define Package/owhttpd/conffiles
/etc/config/owhttpd
endef

define Package/owftpd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/owftpd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/owftpd.conf $(1)/etc/config/owftpd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/owftpd.init $(1)/etc/init.d/owftpd
endef

define Package/owftpd/conffiles
/etc/config/owftpd
endef

define Package/libow/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libow-*.so.* $(1)/usr/lib/
endef

define Package/libow-capi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libowcapi-*.so.* $(1)/usr/lib/
endef


$(eval $(call BuildPackage,owfs))
$(eval $(call BuildPackage,owshell))

$(eval $(call BuildPackage,owserver))
$(eval $(call BuildPackage,owhttpd))
$(eval $(call BuildPackage,owftpd))

$(eval $(call BuildPackage,libow))
$(eval $(call BuildPackage,libow-capi))
