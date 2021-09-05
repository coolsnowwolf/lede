#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fdk-aac
PKG_VERSION:=2.0.1
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mstorsjo/fdk-aac/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=a4142815d8d52d0e798212a5adea54ecf42bcd4eec8092b37a8cb615ace91dc6

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=Fraunhofer-FDK-AAC-for-Android
PKG_LICENSE_FILES:=NOTICE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_BUILD_PATENTED \
	CONFIG_FDK-AAC_OPTIMIZE_SPEED

ifneq ($(CONFIG_BUILD_PATENTED),y)
PATCH_DIR:=./patches-free
endif

include $(INCLUDE_DIR)/package.mk

define Package/fdk-aac
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=Fraunhofer FDK AAC Codec Library
  URL:=https://sourceforge.net/projects/opencore-amr/
endef

define Package/fdk-aac/config
  source "$(SOURCE)/Config.in"
endef

define Package/fdk-aac/description
  Port of the Fraunhofer FDK AAC Codec Library for Android
endef

ifeq ($(CONFIG_FDK-AAC_OPTIMIZE_SPEED),y)
	TARGET_CFLAGS:= $(filter-out -O%,$(TARGET_CFLAGS)) -O2 -flto
	TARGET_CXXFLAGS:= $(filter-out -O%,$(TARGET_CXXFLAGS)) -O2 -flto
	TARGET_LDFLAGS += -flto
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/fdk-aac $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfdk-aac.{la,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/fdk-aac.pc $(1)/usr/lib/pkgconfig/
endef

define Package/fdk-aac/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfdk-aac.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,fdk-aac))
