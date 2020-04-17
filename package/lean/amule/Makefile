#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aMule
PKG_VERSION:=2.3.2
PKG_RELEASE:=3
PKG_REV=4b87b20


PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-r$(PKG_REV).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_URL:=https://github.com/amule-project/amule.git

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libgd libcryptopp

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/amule/config
config AMULE_CRYPTOPP_STATIC_LINKING
	bool "Link libcryptopp statically"
	default n
endef

define Package/amule
  SUBMENU:=P2P
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A multi-platform eMule-like ed2k client
  URL:=http://www.amule.org/
  DEPENDS:=+libpng +libpthread +libncurses +libreadline +libwxbase +libupnp +libbfd \
	$(ICONV_DEPENDS) $(INTL_DEPENDS) +!AMULE_CRYPTOPP_STATIC_LINKING:libcryptopp
endef

CONFIGURE_ARGS+= \
	--disable-static \
	--disable-rpath \
	--with-gnu-ld \
	--disable-ccache \
	--disable-optimize \
	--disable-profile \
	--disable-monolithic \
	--enable-amule-daemon \
	--enable-amulecmd \
	--enable-webserver \
	--disable-amule-gui \
	--disable-cas \
	--disable-wxcas \
	--disable-ed2k \
	--disable-alc \
	--disable-alcc \
	--disable-fileview \
	--disable-plasmamule \
	--without-wxdebug \
	--enable-dlp \
	--enable-upnp \
	--with-zlib="$(STAGING_DIR)/usr" \
	--with-gdlib-prefix="$(STAGING_DIR)/usr" \
	--with-libpng-prefix="$(STAGING_DIR)/usr" \
	--with-wx-prefix="$(STAGING_DIR)/usr" \
	--with-crypto-prefix="$(STAGING_DIR)/usr" \
	--with-libiconv-prefix="$(ICONV_PREFIX)" \
	--with-libintl-prefix="$(INTL_PREFIX)" \
	--with-libupnp-prefix="$(STAGING_DIR)/usr" \
	--without-x \
	--disable-debug 

TARGET_LDFLAGS += -liconv 


define Build/Configure
	cd $(PKG_BUILD_DIR) && sh ./autogen.sh
	$(call Build/Configure/Default)
endef

ifeq ($(CONFIG_AMULE_CRYPTOPP_STATIC_LINKING),y)
	SED_CMD:=sed
else
	SED_CMD:=true
endif

define Build/Compile
	$(SED_CMD) -i 's;^CRYPTOPP_LIBS.*;CRYPTOPP_LIBS \= "$(STAGING_DIR)/usr/lib/libcryptopp.a";g' \
		$(PKG_BUILD_DIR)/src/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		HOSTCC="$(HOSTCC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/amule/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share/amule/webserver
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/amule{cmd,d} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/webserver/src/amuleweb $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/src/webserver/default $(1)/usr/share/amule/webserver
	
endef

$(eval $(call BuildPackage,amule))
