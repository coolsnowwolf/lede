#
# Copyright (C) 2006-2011 Openwrt.org
# Copyright (C) 2011 SMBPhone Inc.
# Copyright (C) 2019 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=slang
PKG_VERSION:=2.3.2
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.jedsoft.org/releases/slang \
	https://www.jedsoft.org/releases/slang/old
PKG_HASH:=fc9e3b0fc4f67c3c1f6d43c90c16a5c42d117b8e28457c5b46831b8b5d3ae31a

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

PKG_BUILD_PARALLEL:=0

SLANG_MODULES:= base64 chksum csv fcntl fork histogram iconv json onig pcre \
	png rand select slsmg socket stats sysconf termios varray zlib

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libslang2/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=S-Lang Programmer's Library
  URL:=https://www.jedsoft.org/slang/
endef

define Package/libslang2/Default/description
  Multi-platform programmer's library providing facilities for interactive
  applications.  Includes such things as display/screen management,
  keyboard input, keymaps, etc.  Includes the embeddable S-Lang interpreter.
endef

define Package/libslang2
$(call Package/libslang2/Default)
  DEPENDS:=+terminfo
endef

define Package/libslang2/description
$(call Package/libslang2/Default/description)
endef

define Package/libslang2-modules
$(call Package/libslang2/Default)
  TITLE+= (all modules)
  DEPENDS:=$(foreach mod,$(SLANG_MODULES),+libslang2-mod-$(mod))
endef

define Package/libslang2-modules/description
$(call Package/libslang2/Default/description)

  This installs all of S-Lang's bundled modules.
endef

define Package/libslang2-mod-iconv
$(call Package/libslang2/Default)
  TITLE+= (iconv module)
  DEPENDS:=$(ICONV_DEPENDS)
endef

define Package/libslang2-mod-onig
$(call Package/libslang2/Default)
  TITLE+= (onig module)
  DEPENDS:=+oniguruma
endef

define Package/libslang2-mod-pcre
$(call Package/libslang2/Default)
  TITLE+= (pcre module)
  DEPENDS:=+libpcre
endef

define Package/libslang2-mod-png
$(call Package/libslang2/Default)
  TITLE+= (png module)
  DEPENDS:=+libpng
endef

define Package/libslang2-mod-zlib
$(call Package/libslang2/Default)
  TITLE+= (zlib module)
  DEPENDS:=+zlib
endef

define Package/slsh
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=S-Lang shell
  URL:=https://www.jedsoft.org/slang/slsh.html
  DEPENDS:=+libslang2 +libslang2-modules
endef

define Package/slsh/description
  This is a shell which is mostly just a wrapper around the S-Lang Interpreter,
  which is part of the S-Lang Library.
endef

TARGET_CFLAGS+= $(FPIC)

CONFIGURE_ARGS+= \
	--enable-largefile \
	--enable-warnings \
	--with-terminfo=default \
	--with-readline=slang \
	--with-iconv="$(ICONV_PREFIX)" \
	--with-onig="$(STAGING_DIR)/usr" \
	--with-pcre="$(STAGING_DIR)/usr" \
	--with-png="$(STAGING_DIR)/usr" \
	--with-z="$(STAGING_DIR)/usr" \
	--without-x

define Build/Compile
	$(call Build/Compile/Default,)
	$(call Build/Compile/Default,static)
endef

# Default install last, so that the dynamic slsh is installed
define Build/Install
	$(call Build/Install/Default,install-static)
	$(call Build/Install/Default,)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libslang.{a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/slang.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libslang2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libslang.so* $(1)/usr/lib/
endef

Package/libslang2-modules/install:=:

define Package/slsh/install
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/slsh.rc $(1)/etc/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/slsh $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/slsh/ $(1)/usr/share/
endef

define BuildModule
  ifndef Package/libslang2-mod-$(1)
    define Package/libslang2-mod-$(1)
    $$(call Package/libslang2/Default)
      TITLE+= ($(1) module)
    endef
  endif

  define Package/libslang2-mod-$(1)/description
  $$(call Package/libslang2/Default/description)

    This contains the $(1) module.
  endef

  define Package/libslang2-mod-$(1)/install
	$$(INSTALL_DIR) $$(1)/usr/lib/slang/v2/modules
	$$(CP) $$(PKG_INSTALL_DIR)/usr/lib/slang/v2/modules/$(1)-module.so $$(1)/usr/lib/slang/v2/modules/
  endef

  $$(eval $$(call BuildPackage,libslang2-mod-$(1)))
endef

$(foreach mod,$(SLANG_MODULES),$(eval $(call BuildModule,$(mod))))

$(eval $(call BuildPackage,libslang2))
$(eval $(call BuildPackage,libslang2-modules))
$(eval $(call BuildPackage,slsh))
