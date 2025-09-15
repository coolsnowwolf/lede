# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=genext2fs
PKG_VERSION:=1.5.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/bestouff/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d3861e4fe89131bd21fbd25cf0b683b727b5c030c4c336fadcd738ada830aab0

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS = \
	--target=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--build=$(GNU_HOST_NAME) \
	--program-prefix="" \
	--program-suffix="" \
	--prefix=/usr \
	--exec-prefix=/usr \
	--bindir=/usr/bin \
	--sbindir=/usr/sbin \
	--libexecdir=/usr/lib \
	--sysconfdir=/etc \
	--datadir=/usr/share \
	--localstatedir=/var \
	--mandir=/usr/man \
	--infodir=/usr/info \

define Host/Configure
	(cd $(HOST_BUILD_DIR); \
		./autogen.sh \
	);
	$(call Host/Configure/Default)
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		all
endef

define Host/Install
	install -m0755 $(HOST_BUILD_DIR)/genext2fs $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/genext2fs
endef

$(eval $(call HostBuild))
