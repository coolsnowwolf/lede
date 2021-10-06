#
# Copyright (C) 2020 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qtbase
PKG_BASE:=5.15
PKG_BUGFIX:=2
PKG_VERSION:=$(PKG_BASE).$(PKG_BUGFIX)
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-everywhere-src-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:= \
		http://download.qt.io/official_releases/qt/$(PKG_BASE)/$(PKG_VERSION)/submodules \
		http://master.qt.io/archive/qt/$(PKG_BASE)/$(PKG_VERSION)/submodules \
		http://mirrors.tuna.tsinghua.edu.cn/qt/archive/qt/$(PKG_BASE)/$(PKG_VERSION)/submodules \
		http://qt.mirror.constant.com/archive/qt/$(PKG_BASE)/$(PKG_VERSION)/submodules
PKG_HASH:=909fad2591ee367993a75d7e2ea50ad4db332f05e1c38dd7a5a274e156a4e0f8

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-everywhere-src-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0
PKG_CPE_ID:=cpe:/a:qt:qt

include $(INCLUDE_DIR)/package.mk

define Package/qtbase/Default
	SECTION:=libs
	CATEGORY:=Libraries
	SUBMENU:=Qt5
	TITLE:=qtbase
	URL:=http://qt-project.org
	DEPENDS:=+libgcc +libstdcpp @!LINUX_2_6_36
endef

define Package/qtbase/description
  This package provides the Qt5 libraries.

  Qt is a cross-platform application development framework for desktop, embedded and mobile.
  Supported Platforms include Linux, OS X, Windows, VxWorks, QNX, Android, iOS, BlackBerry,
  Sailfish OS and others.

  Qt is not a programming language on its own. It is a framework written in C++.

  This package provides the following run-time libraries:
  - core
  - network
  - xml
endef

define Package/qtbase
	$(call Package/qtbase/Default)
	TITLE+=all libs
	DEPENDS+=$(QTBASE_DEPENDS)
endef

define Package/qtbase/install
	true
endef

define Package/qtbase/config
	config QT5_INCLUDE_ATOMIC
		bool "Depends on libatomic"
		default y if !(arm_v7 || ARCH_64BIT || TARGET_x86)
		default n
		help
			Check this options to add the libatomic to the dependences.
			- You'd better to confirm whether your compiler need the external libatomic.
			Otherwise, it is not recommended to modify this option mannually.

	menu "Select Qtbase Libraries"
		comment "Qtbase Libraries"

		$(foreach lib,$(QTBASE_LIBS),
		config PACKAGE_qt5-$(lib)
			prompt "Qtbase $(lib) library."
			default m if ALL
		)
	endmenu
endef

PKG_CONFIG_DEPENDS = CONFIG_QT5_INCLUDE_ATOMIC

EXTRA_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections -flto
EXTRA_LDFLAGS += -Wl,--gc-sections,--as-needed

CONFIGURE_ARGS = \
		-sysroot $(STAGING_DIR) \
		-hostprefix $(STAGING_DIR_HOSTPKG) \
		-extprefix $(STAGING_DIR)/usr \
		-prefix /usr \
		-archdatadir /usr/share/Qt \
		-datadir /usr/share/Qt \
		-device linux-generic-g++ \
		-device-option CROSS_COMPILE="$(TARGET_CROSS)" \
		-device-option COMPILER_FLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
		-device-option LINKER_FLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \
		-confirm-license \
		-opensource \
		-release \
		-shared \
		-strip \
		-no-rpath \
		-no-use-gold-linker \
		-ltcg \
		-mimetype-database \
		-openssl-linked \
		-qt-doubleconversion \
		-system-pcre \
		-system-zlib \
		$(if $(findstring i386,$(ARCH)),-no-sse2 -no-sse4.1) \
		-no-angle \
		-no-cups \
		-no-dbus \
		-no-directfb \
		-no-dtls\
		-no-egl \
		-no-eglfs \
		-no-freetype \
		-no-gbm \
		-no-glib \
		-no-gtk \
		-no-gui \
		-no-harfbuzz \
		-no-iconv \
		-no-icu \
		-no-kms \
		-no-libjpeg \
		-no-libmd4c \
		-no-libpng \
		-no-libudev \
		-no-mtdev \
		-no-opengl \
		-no-opengles3 \
		-no-openvg \
		-no-pch \
		-no-slog2 \
		-sql-sqlite \
		-no-trace \
		-no-tslib \
		-no-vulkan \
		-no-widgets \
		-no-xcb \
		-no-xkbcommon \
		-no-zstd \
		-no-compile-examples \
		-no-feature-concurrent \
		-no-feature-gssapi \
		-no-feature-testlib \
		-make libs \
		-nomake examples \
		-nomake tests \
		-nomake tools \
		-v 


define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	$(MAKE) -C $(PKG_BUILD_DIR) install
endef

define Package/qtbase/Default/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libQt5$(shell echo $(2) | sed -e 's/\b[[:alpha:]]/\u&/g').so* $(1)/usr/lib
endef

define DefineQtbaseLibrary
  QTBASE_DEPENDS+=+qt5-$(1)

  QTBASE_LIBS+=$(1)

  define Package/qt5-$(1)
    $(call Package/qtbase/Default)
    TITLE+=$(1)
    DEPENDS+=$(foreach lib,$(2),+qt5-$(lib)) $(3)
    HIDDEN:=1
  endef

  define Package/qt5-$(1)/description
    This package contains the qt5 $(1) library.
  endef

  define Package/qt5-$(1)/install
    $(call Package/qtbase/Default/install,$$(1),$(1))
  endef
endef

$(eval $(call DefineQtbaseLibrary,core,,+QT5_INCLUDE_ATOMIC:libatomic +libpcre2-16 +zlib))
$(eval $(call DefineQtbaseLibrary,network,core,+libopenssl +zlib))
$(eval $(call DefineQtbaseLibrary,xml,core,))
$(eval $(call DefineQtbaseLibrary,sql,core))

$(foreach lib,$(QTBASE_LIBS),$(eval $(call BuildPackage,qt5-$(lib))))
$(eval $(call BuildPackage,qtbase))
