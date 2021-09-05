#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mpg123
PKG_VERSION:=1.27.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/mpg123
PKG_HASH:=52f6ceb962c05db0c043bb27acf5a721381f5f356ac4610e5221f50293891b04

PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:mpg123:mpg123

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mpg123/Default
  URL:=http://www.mpg123.de
endef

define Package/libmpg123
  $(call Package/mpg123/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=fast console mpeg audio decoder library
  LICENSE:=LGPL-2.1-or-later
  DEPENDS:=+libltdl
endef

define Package/libout123
  $(call Package/mpg123/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for continuous playback of audio streams via various platform-specific output methods
  LICENSE:=LGPL-2.1-or-later
  DEPENDS:=+libltdl
endef

define Package/libsyn123
  $(call Package/mpg123/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for signal generation and format conversion
  LICENSE:=LGPL-2.1-or-later
  DEPENDS:=+libltdl
endef

define Package/mpg123
  $(call Package/mpg123/Default)
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=fast console mpeg audio player
  LICENSE:=GPL-2.0-or-later
  DEPENDS+=+libmpg123 +alsa-lib +libout123 +libsyn123
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--with-audio=alsa \
	--with-default-audio=alsa

ifeq ($(CONFIG_SOFT_FLOAT),y)
	CONFIGURE_ARGS += \
		--with-cpu=generic_nofpu \
		--enable-int-quality=yes
else ifneq ($(findstring 464fp,$(CONFIG_CPU_TYPE)),)
	CONFIGURE_ARGS += \
		--with-cpu=altivec
else ifneq ($(findstring neon,$(CONFIG_CPU_TYPE)),)
	CONFIGURE_ARGS += \
		--with-cpu=arm_fpu
else ifneq ($(findstring aarch64,$(CONFIG_ARCH)),)
	CONFIGURE_ARGS += \
		--with-cpu=aarch64
else
	CONFIGURE_ARGS += \
		--with-cpu=generic_fpu
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/fmt123.h \
		$(PKG_INSTALL_DIR)/usr/include/mpg123.h \
		$(PKG_INSTALL_DIR)/usr/include/out123.h \
		$(PKG_INSTALL_DIR)/usr/include/syn123.h \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmpg123.{la,a,so*} \
		$(PKG_INSTALL_DIR)/usr/lib/libout123.{la,a,so*} \
		$(PKG_INSTALL_DIR)/usr/lib/libsyn123.{la,a,so*} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/mpg123
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/mpg123/output* \
		$(1)/usr/lib/mpg123

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libmpg123.pc \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libout123.pc \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libsyn123.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/libmpg123/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmpg123.so.* \
		$(1)/usr/lib/
endef

define Package/libout123/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libout123.so.* \
		$(1)/usr/lib/
endef

define Package/libsyn123/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsyn123.so.* \
		$(1)/usr/lib/
endef

define Package/mpg123/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/mpg123{,-id3dump,-strip} \
		$(PKG_INSTALL_DIR)/usr/bin/out123 \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/mpg123
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/mpg123/output_*.so \
		$(1)/usr/lib/mpg123
endef

$(eval $(call BuildPackage,libmpg123))
$(eval $(call BuildPackage,libout123))
$(eval $(call BuildPackage,libsyn123))
$(eval $(call BuildPackage,mpg123))
