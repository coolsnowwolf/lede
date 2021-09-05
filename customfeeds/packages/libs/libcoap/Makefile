#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcoap
PKG_VERSION:=4.2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/obgm/libcoap/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=29a0394a265d3febee41e5e2dc03d34292a0aede37f5f80334e529ac0dab2321

PKG_MAINTAINER:=Anton Glukhov <anton.a.glukhov@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later BSD-2-Clause
PKG_LICENSE_FILES:=COPYING LICENSE.GPL LICENSE.BSD

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libcoap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=CoAP (RFC 7252) library
  URL:=https://libcoap.net/
  ABI_VERSION:=2
endef

define Package/libcoap/description
  Constrained Application Protocol (RFC 7252) library
endef

define Package/coap-client
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libcoap
  TITLE:=CoAP (RFC 7252) client tool
endef

define Package/coap-client/description
  Constrained Application Protocol (RFC7252) client tool
endef

define Package/coap-server
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libcoap
  TITLE:=CoAP (RFC 7252) server programs
endef

define Package/coap-server/description
  Constrained Application Protocol (RFC 7252) server and resource directory server
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-examples \
	--disable-documentation \
	--disable-doxygen \
	--disable-dtls \
	--disable-gcov \
	--disable-tests

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/libcoap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcoap-$(ABI_VERSION).so* $(1)/usr/lib/
endef

define Package/coap-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/coap-client $(1)/usr/bin/
endef

define Package/coap-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/coap-server $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/coap-rd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libcoap))
$(eval $(call BuildPackage,coap-client))
$(eval $(call BuildPackage,coap-server))
