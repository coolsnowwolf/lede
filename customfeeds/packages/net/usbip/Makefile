#
# Copyright (C) 2010-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=usbip
PKG_RELEASE:=13
PKG_LICENSE:=GPL-2.0-only

# Since kernel 2.6.39.1 userspace tools are inside the kernel tree
# Package Automatic match version in kernel
# HASH is not useful kernel package already check it
PKG_VERSION:=$(shell  sed -n -e '/^AC_INIT/s/.*\[\([[:digit:]\.]*\)\].*/\1/gp' $(LINUX_DIR)/tools/usb/usbip/configure.ac)
PKG_SOURCE:=
PKG_SOURCE_URL:=
PKG_HASH:=unknown

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

PATCH_DIR:=./patches-$(PKG_VERSION)

define prepare_source_directory
	rm -rf $(PKG_BUILD_DIR)
	$(CP) $(LINUX_DIR)/tools/usb/usbip $(PKG_BUILD_DIR)
endef
Hooks/Prepare/Pre += prepare_source_directory

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_ASLR_PIE:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/usbip/Common
  TITLE:=USB-over-IP
  URL:=http://usbip.sourceforge.net/
  DEPENDS:=@USB_SUPPORT
endef

define Package/usbip/Default
  $(call Package/usbip/Common)
  SECTION:=net
  CATEGORY:=Network
endef

define Package/usbip
  $(call Package/usbip/Default)
  TITLE+= (common)
  DEPENDS+= +libwrap +kmod-usbip +libudev +USE_GLIBC:libbsd +usbids
endef

define Package/usbip-client
  $(call Package/usbip/Default)
  TITLE+= (client)
  DEPENDS+= usbip +kmod-usbip-client
endef

define Package/usbip-server
  $(call Package/usbip/Default)
  TITLE+= (server)
  DEPENDS+= usbip +kmod-usbip-server
endef

CONFIGURE_PATH:=.
MAKE_PATH:=.
LIBTOOL_PATHS:=.
CONFIGURE_ARGS+= --disable-silent-rules

define Build/Configure
	(cd $(PKG_BUILD_DIR); ./autogen.sh );
	$(call Build/Configure/Default)
endef

CONFIGURE_VARS+= $(if $(CONFIG_USE_GLIBC),LIBS='-lbsd -lpthread')
CFLAGS+="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include"

define Package/usbip/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusbip.so.* $(1)/usr/lib/
endef

define Package/usbip-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/usbip $(1)/usr/sbin/
endef

define Package/usbip-server/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/usbipd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,usbip))
$(eval $(call BuildPackage,usbip-client))
$(eval $(call BuildPackage,usbip-server))
