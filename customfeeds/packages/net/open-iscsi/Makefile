# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=open-iscsi
PKG_VERSION:=2.1.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/open-iscsi/open-iscsi/tar.gz/$(PKG_VERSION)?
PKG_HASH:=5410474b23552016220d04aa181903cb50ae988f29e99cb03f3e2de86a109be4

PKG_MAINTAINER:=Lucian CRISTIAN <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/open-iscsi
	SECTION:=net
	CATEGORY:=Network
	URL:=http://www.open-iscsi.com/
	TITLE:=iSCSI userland tools
	DEPENDS:=+libopenssl +open-isns +libmount +libkmod +kmod-iscsi-initiator
endef

define Package/open-iscsi/description
The Open-iSCSI project provides a high-performance, transport independent,
implementation of RFC 3720 iSCSI for Linux.
User space contains the entire control plane: configuration manager,
iSCSI Discovery, Login and Logout processing, connection-level error processing,
Nop-In and Nop-Out handling, etc.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(SED)'s,/run,/var/run,g' $(PKG_BUILD_DIR)/libopeniscsiusr/idbm.c
	$(SED)'s,/run,/var/run,g' $(PKG_BUILD_DIR)/usr/initiator.h
endef

define Build/Configure
	$(call Build/Configure/Default)
	(cd $(PKG_BUILD_DIR)/iscsiuio/; \
	autoreconf --install; \
	./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		LIB_DIR=/usr/lib \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
	)
endef

TARGET_CFLAGS += \
	$(FPIC) -flto \
	-D_GNU_SOURCE -DNO_SYSTEMD

TARGET_LDFLAGS += -lkmod

MAKE_FLAGS += \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) \
		-I$(PKG_BUILD_DIR)/include \
		-I$(PKG_BUILD_DIR)/usr \
		-I$(PKG_BUILD_DIR)/libopeniscsiusr" \
	LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS) \
		-L$(PKG_BUILD_DIR)/libopeniscsiusr" \
	PREFIX=/usr \
	LIB_DIR=/usr/lib

define Package/open-iscsi/conffiles
/etc/iscsi/
endef

define Package/open-iscsi/postinst
#!/bin/sh
[ -x /usr/sbin/iscsi-gen-initiatorname ] && /usr/sbin/iscsi-gen-initiatorname || exit 0
endef

define Package/open-iscsi/install
	$(INSTALL_DIR) \
	 $(1)/etc/init.d $(1)/etc/iscsi \
	 $(1)/usr/lib $(1)/sbin $(1)/usr/sbin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/iscsi/iscsid.conf $(1)/etc/iscsi/
	$(INSTALL_BIN) ./files/iscsi* $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/open-iscsi $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/iscsi{adm,d,-iname,uio} $(1)/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopeniscsiusr.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,open-iscsi))
