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
PKG_VERSION:=5.9.8
PKG_RELEASE:=1

PKG_SOURCE:=qt-everywhere-opensource-src-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://download.qt-project.org/archive/qt/5.9/$(PKG_VERSION)/single
PKG_HASH:=86aca7dc37f161fc730a9d4f6bddf684962ca560327682e282ff61bf8b859c36

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_BUILD_DIR=$(BUILD_DIR)/qt-everywhere-opensource-src-$(PKG_VERSION)

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
	DEPENDS+=+libatomic +zlib +libpcre2-16 +libdouble-conversion
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
		-sysroot $(STAGING_DIR) \
		-plugindir /usr/lib/Qt/plugins \
		-R /usr/lib \
		-I $(STAGING_DIR)/usr/include \
		-L $(STAGING_DIR)/usr/lib \
		-xplatform linux-openwrt-g++ \
		-release \
		-shared \
		-opensource \
		-confirm-license \
		-no-use-gold-linker \
		-strip \
		-ltcg \
		-optimized-qmake \
		-system-doubleconversion \
		-no-glib \
		-no-icu \
		-system-pcre \
		-system-zlib \
		-no-slog2 \
		-openssl-linked \
		-no-cups \
		-no-freetype \
		-no-harfbuzz \
		-no-gtk \
		-no-opengl \
		-no-opengles3 \
		-no-angle \
		-no-openvg \
		-no-directfb \
		-no-eglfs \
		-no-egl \
		-no-iconv \
		-no-gbm \
		-no-kms \
		-no-xcb \
		-no-pch \
		-no-libudev \
		-no-mtdev \
		-no-tslib \
		-no-xkbcommon-evdev \
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
		-no-gui \
		-no-widgets \
		-no-dbus \
		-no-qml-debug \
		-make libs \
		-nomake examples \
		-nomake tests \
		-nomake tools \
		-no-compile-examples \
		-skip qt3d \
		-skip qtactiveqt \
		-skip qtandroidextras \
		-skip qtcanvas3d \
		-skip qtcharts \
		-skip qtconnectivity \
		-skip qtdatavis3d \
		-skip qtdeclarative \
		-skip qtdoc \
		-skip qtgamepad \
		-skip qtgraphicaleffects \
		-skip qtimageformats \
		-skip qtlocation \
		-skip qtmacextras \
		-skip qtmultimedia \
		-skip qtnetworkauth \
		-skip qtpurchasing \
		-skip qtquickcontrols \
		-skip qtquickcontrols2 \
		-skip qtremoteobjects \
		-skip qtscript \
		-skip qtscxml \
		-skip qtsensors \
		-skip qtserialbus \
		-skip qtserialport \
		-skip qtspeech \
		-skip qtsvg \
		-skip qttranslations \
		-skip qtvirtualkeyboard \
		-skip qtwayland \
		-skip qtwebchannel \
		-skip qtwebengine \
		-skip qtwebsockets \
		-skip qtwebview \
		-skip qtwinextras \
		-skip qtx11extras \
		-skip qtxmlpatterns \
		-v

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
