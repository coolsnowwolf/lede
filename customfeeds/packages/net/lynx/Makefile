#
# This is free software, licensed under the GNU General Public License v2.
#
# Based on https://github.com/kyak/openwrt-packages/tree/master/lynx
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lynx
PKG_VERSION:=2.8.9rel.1
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Leonid Esman <leonid.esman@gmail.com>
PKG_CPE_ID:=cpe:/a:lynx:lynx
PKG_SOURCE:=$(PKG_NAME)$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://invisible-mirror.net/archives/lynx/tarballs/ \
                https://ftp.icm.edu.pl/packages/lynx/tarballs/ \
                http://ftp.oregonstate.edu/pub/blfs/conglomeration/lynx/ \
                https://ftp.osuosl.org/pub/blfs/conglomeration/lynx/ \
                https://fossies.org/linux/www/
PKG_HASH:=387f193d7792f9cfada14c60b0e5c0bff18f227d9257a39483e14fa1aaf79595
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)$(PKG_VERSION)
# try current development version
# URL for current development version - http://invisible-island.net/datafiles/release/lynx-cur.tar.bz2


include $(INCLUDE_DIR)/package.mk

define Package/lynx
	TITLE:=Lynx is the text web browser 
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libncursesw +zlib +libopenssl +ca-bundle
	URL:=http://lynx.invisible-island.net/
endef

define Package/lynx/description
	Classic non-graphical (text-mode) web browser
endef

# customize as you need
CONFIGURE_ARGS += --with-zlib --with-ssl \
		  --enable-ipv6 \
		  --with-screen=ncursesw \
		  --enable-widec \
		  --enable-locale-charset \
		  --enable-charset-choice \
		  --enable-cjk \
		  --enable-japanese-utf8 \
		  --enable-nsl-fork \
		--disable-config-info --disable-echo \
		--disable-dired --disable-trace \
		--disable-finger --disable-gopher --disable-news --disable-wais \
		--disable-prettysrc --disable-scrollbar --disable-gzip-help \
		--disable-idna --disable-syslog --disable-font-switch

CONFIGURE_VARS += cf_cv_ncurses_header="ncursesw/curses.h" \
		  ac_cv_path_NCURSES_CONFIG=""

define Package/lynx/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lynx $(1)/usr/bin/
	$(INSTALL_DATA) ./files/lynx.cfg $(1)/etc/
	$(INSTALL_DATA) ./files/lynx.lss $(1)/etc/
endef

$(eval $(call BuildPackage,lynx))

