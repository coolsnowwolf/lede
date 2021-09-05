#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmms
PKG_VERSION:=0.6.4
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/libmms
PKG_HASH:=3c05e05aebcbfcc044d9e8c2d4646cd8359be39a3f0ba8ce4e72a9094bee704f
PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_LICENSE:=LGPLv2.1
PKG_LICENSE_FILES:=COPYING.LIB
PKG_CPE_ID:=cpe:/a:libmms_project:libmms

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmms
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=MMS stream protocol library
  URL:=http://libmms.sourceforge.net
endef

define Package/libmms/description
	LibMMS is a common library for parsing mms:// and mmsh:// type network streams.
	These are commonly used to stream Windows Media Video content over the web.
	LibMMS itself is only for receiving MMS stream,
	it doesn't handle sending at all.
endef

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libmms $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmms.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/libmms/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmms.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmms))
