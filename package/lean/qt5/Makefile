#
# Copyright (C) 2017-2019 Entware
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# ported to Entware from
# https://github.com/KryptonLee/qBittorrent-openwrt-package and
# https://github.com/pawelkn/qt5-openwrt-package

include $(TOPDIR)/rules.mk

PKG_NAME:=qt5
PKG_VERSION:=5.13
PKG_RELEASE:=2

PKG_SOURCE:=qt-everywhere-src-$(PKG_VERSION).$(PKG_RELEASE).tar.xz
PKG_SOURCE_URL:=http://download.qt-project.org/archive/qt/$(PKG_VERSION)/$(PKG_VERSION).$(PKG_RELEASE)/single/
PKG_HASH:=55e8273536be41f4f63064a79e552a22133848bb419400b6fa8e9fc0dc05de08

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_BUILD_DIR=$(BUILD_DIR)/qt-everywhere-src-$(PKG_VERSION).$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

ifndef CONFIG_USE_GLIBC
# not using sstrip here as this fucks up the .so's somehow
STRIP:=/bin/true
RSTRIP:= \
	NM="$(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)nm" \
	STRIP="$(STRIP)" \
	STRIP_KMOD="$(STRIP)" \
	$(SCRIPT_DIR)/rstrip.sh
endif

define Package/qt5/Default
	SECTION:=libs
	CATEGORY:=Libraries
	SUBMENU:=Qt5
	TITLE:=qt5
	URL:=http://qt-project.org
	DEPENDS:=+librt +libstdcpp +libpthread @!LINUX_2_6_36
endef

define Package/qt5-core
	$(call Package/qt5/Default)
	TITLE+=core
	DEPENDS+=+libatomic +libzstd +zlib +libpcre2-16 +libdouble-conversion +icu
endef

define Package/qt5-network
	$(call Package/qt5/Default)
	TITLE+=network
	DEPENDS+=+qt5-core +libopenssl
endef

define Package/qt5-xml
	$(call Package/qt5/Default)
	TITLE+=xml
	DEPENDS+=+qt5-core
endef

CONFIGURE_ARGS = \
		-prefix /usr \
		-extprefix $(TOOLCHAIN_DIR) \
		-sysroot $(STAGING_DIR) \
		-plugindir /usr/lib/Qt/plugins \
		-R /usr/lib \
		-I $(STAGING_DIR)/usr/include \
		-L $(STAGING_DIR)/usr/lib \
		-xplatform linux-openwrt-g++ \
		-opensource \
		-confirm-license \
		-system-doubleconversion \
		-system-pcre \
		-system-zlib \
		-openssl-runtime \
		-no-cups \
		-no-freetype \
		-no-harfbuzz \
		-no-gtk \
		-no-opengl \
		-no-opengles3 \
		-no-angle \
		-no-openvg \
		-no-dbus \
		-no-directfb \
		-no-eglfs \
		-no-feature-getentropy \
		-no-iconv \
		-no-kms \
		-no-pch \
		-no-libudev \
		-no-mtdev \
		-no-tslib \
		-no-libpng \
		-no-libjpeg \
		-no-sql-db2 \
		-no-sql-ibase \
		-no-sql-mysql \
		-no-sql-oci \
		-no-sql-odbc \
		-no-sql-psql \
		-no-sql-sqlite \
		-no-sql-sqlite2 \
		-no-sqlite \
		-no-sql-tds \
		-no-strip \
		-no-xcb \
		-no-gui \
		-no-widgets \
		-no-glib \
		-make libs \
		-nomake examples \
		-nomake tests \
		-nomake tools \
		-skip 3d \
		-skip activeqt \
		-skip charts \
		-skip connectivity \
		-skip datavis3d \
		-skip declarative \
		-skip doc \
		-skip gamepad \
		-skip imageformats \
		-skip location \
		-skip networkauth \
		-skip multimedia \
		-skip purchasing \
		-skip remoteobjects \
		-skip script \
		-skip sensors \
		-skip serialbus \
		-skip serialport \
		-skip speech \
		-skip svg \
		-skip wayland \
		-skip webengine \
		-skip websockets \
		-skip webchannel \
		-skip xmlpatterns \
		-v \

define Build/Configure
	$(SED) 's@$$$$(TARGET_CROSS)@$(TARGET_CROSS)@g' $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++/qmake.conf
	$(SED) 's@$$$$(TARGET_CFLAGS)@$(TARGET_CFLAGS)@g' $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++/qmake.conf
	$(SED) 's@$$$$(TARGET_CXXFLAGS)@$(TARGET_CXXFLAGS)@g' $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++/qmake.conf
	$(SED) 's@$$$$(TARGET_LDFLAGS)@$(TARGET_LDFLAGS)@g' $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++/qmake.conf

	$(call Build/Configure/Default)
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	$(MAKE) -C $(PKG_BUILD_DIR) install
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/qtbase/bin/qt.conf $(STAGING_DIR)/usr/bin
	$(RM) $(STAGING_DIR)/usr/bin/qt{attributionsscanner,paths,plugininfo}
endef

define Package/qt5-core/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/qtbase/lib/libQt5Core.so* $(1)/usr/lib
endef

define Package/qt5-network/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/qtbase/lib/libQt5Network.so* $(1)/usr/lib
endef

define Package/qt5-xml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/qtbase/lib/libQt5Xml.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,qt5-core,+zlib,+libpcre2-16,+libdouble-conversion))
$(eval $(call BuildPackage,qt5-network))
$(eval $(call BuildPackage,qt5-xml))
