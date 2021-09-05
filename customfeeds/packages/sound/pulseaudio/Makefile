#
# Copyright (C) 2011-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pulseaudio
PKG_VERSION:=14.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://freedesktop.org/software/pulseaudio/releases
PKG_HASH:=75d3f7742c1ae449049a4c88900e454b8b350ecaa8c544f3488a2562a9ff66f1

PKG_MAINTAINER:=Peter Wagner <tripolar@gmx.at>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=GPL LICENSE
PKG_CPE_ID:=cpe:/a:pulseaudio:pulseaudio

PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/pulseaudio/Default
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libsndfile +libltdl +libpthread +librt +alsa-lib \
	+libopenssl +libcap $(ICONV_DEPENDS) $(INTL_DEPENDS)
  TITLE:=Network sound server
  URL:=https://www.pulseaudio.org
  PROVIDES:=pulseaudio
  USERID:=pulse=51:pulse=51
endef

define Package/pulseaudio-daemon
  $(call Package/pulseaudio/Default)
  VARIANT:=noavahi
endef

define Package/pulseaudio-daemon-avahi
  $(call Package/pulseaudio/Default)
  DEPENDS+=+dbus +libavahi-client +sbc
  TITLE+= (avahi/bluez)
  VARIANT:=avahi
endef

define Package/pulseaudio/Default/description
  PulseAudio (formerly Polypaudio) is a cross-platform, networked sound server.
endef

define Package/pulseaudio-daemon/description
  $(call Package/pulseaudio/Default/description)
endef

define Package/pulseaudio-daemon-avahi/description
  $(call Package/pulseaudio/Default/description)
  This package enables avahi,bluez and is compiled against dbus, sbc, and avahi.
endef

define Package/pulseaudio-daemon/conffiles
/etc/pulse/client.conf
/etc/pulse/daemon.conf
/etc/pulse/default.pa
/etc/pulse/system.pa
endef

define Package/pulseaudio-daemon-avahi/conffiles
/etc/pulse/client.conf
/etc/pulse/daemon.conf
/etc/pulse/default.pa
/etc/pulse/system.pa
endef

define Package/pulseaudio-tools
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libsndfile pulseaudio
  TITLE:=Tools for Pulseaudio
  URL:=http://www.pulseaudio.org
  VARIANT:=noavahi
endef

define Package/pulseaudio-profiles
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=pulseaudio
  TITLE:=Profiles for Pulseaudio
  URL:=http://www.pulseaudio.org
endef

MESON_ARGS += \
	-Db_lto=true \
	-Dgcov=false \
	-Dman=false \
	-Dtests=false \
	-Dsystem_user=pulse \
	-Dsystem_group=pulse \
	-Daccess_group=audio \
	-Ddatabase=simple \
	-Dlegacy-database-entry-format=false \
	-Dstream-restore-clear-old-devices=true \
	-Drunning-from-build-tree=false \
	-Datomic-arm-linux-helpers=false \
	-Datomic-arm-memory-barrier=false \
	-Dalsa=enabled \
	-Dasyncns=disabled \
	-Dbluez5-native-headset=false \
	-Dbluez5-ofono-headset=false \
	-Dfftw=disabled \
	-Dglib=disabled \
	-Dgsettings=disabled \
	-Dgstreamer=disabled \
	-Dgtk=disabled \
	-Dhal-compat=false \
	-Dipv6=true \
	-Djack=disabled \
	-Dlirc=disabled \
	-Dopenssl=enabled \
	-Dorc=disabled \
	-Dsamplerate=disabled \
	-Dsoxr=disabled \
	-Dspeex=disabled \
	-Dsystemd=disabled \
	-Dudev=disabled \
	-Dx11=disabled \
	-Dadrian-aec=true \
	-Dwebrtc-aec=disabled

ifeq ($(BUILD_VARIANT),avahi)
MESON_ARGS += \
	-Davahi=enabled \
	-Dbluez5=true \
	-Ddbus=enabled
endif

ifeq ($(BUILD_VARIANT),noavahi)
MESON_ARGS += \
	-Davahi=disabled \
	-Dbluez5=false \
	-Ddbus=disabled
endif

TARGET_LDFLAGS += -Wl,--gc-sections $(if $(INTL_FULL),-lintl)

define Build/Prepare
	$(call Build/Prepare/Default)
ifneq ($(findstring arm,$(CONFIG_ARCH)),)
ifeq ($(findstring neon,$(CONFIG_CPU_TYPE)),)
	$(SED) '/remap_neon\.c/d' $(PKG_BUILD_DIR)/src/pulsecore/meson.build
endif
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib/pkgconfig \
		$(1)/usr/include/pulse \
		$(1)/usr/lib \
		$(1)/usr/lib/pulseaudio
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/pulse/* \
		$(1)/usr/include/pulse
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig
	$(SED) \
		's,/usr/include,$$$${prefix}/include,g' \
		$(1)/usr/lib/pkgconfig/libpulse.pc
	$(SED) \
		's,/usr/lib,$$$${exec_prefix}/lib,g' \
		$(1)/usr/lib/pkgconfig/libpulse.pc
	$(SED) \
		's,/usr/include,$$$${prefix}/include,g' \
		$(1)/usr/lib/pkgconfig/libpulse-simple.pc
	$(SED) \
		's,/usr/lib,$$$${exec_prefix}/lib,g' \
		$(1)/usr/lib/pkgconfig/libpulse-simple.pc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulseaudio/* \
		$(1)/usr/lib/pulseaudio/
endef

define Package/pulseaudio-daemon/install
	$(INSTALL_DIR) \
		$(1)/etc/pulse \
		$(1)/etc/init.d \
		$(1)/usr/bin \
		$(1)/usr/lib \
		$(1)/usr/lib/pulseaudio \
		$(1)/usr/lib/pulse-$(PKG_VERSION)/modules

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/pulseaudio \
		$(1)/usr/bin/pulseaudio

	$(INSTALL_BIN) \
		./files/pulseaudio.init \
		$(1)/etc/init.d/pulseaudio

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/pulse/* \
		$(1)/etc/pulse

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulseaudio/* \
		$(1)/usr/lib/pulseaudio/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulse-$(PKG_VERSION)/modules/lib*.so \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulse-$(PKG_VERSION)/modules/module*.so \
		$(1)/usr/lib/pulse-$(PKG_VERSION)/modules/

endef

define Package/pulseaudio-daemon-avahi/install
	$(INSTALL_DIR) \
		$(1)/etc/pulse \
		$(1)/etc/init.d \
		$(1)/usr/bin \
		$(1)/usr/lib \
		$(1)/usr/lib/pulseaudio \
		$(1)/usr/lib/pulse-$(PKG_VERSION)/modules \
		$(1)/etc/dbus-1/system.d

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/pulseaudio \
		$(1)/usr/bin/pulseaudio

	$(INSTALL_BIN) \
		./files/pulseaudio.init \
		$(1)/etc/init.d/pulseaudio

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/pulse/* \
		$(1)/etc/pulse

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulseaudio/* \
		$(1)/usr/lib/pulseaudio/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulse-$(PKG_VERSION)/modules/lib*.so \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pulse-$(PKG_VERSION)/modules/module*.so \
		$(1)/usr/lib/pulse-$(PKG_VERSION)/modules/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/dbus-1/system.d/pulseaudio-system.conf \
		$(1)/etc/dbus-1/system.d/pulseaudio-system.conf
endef

define Package/pulseaudio-tools/install
	$(INSTALL_DIR) \
		$(1)/usr/bin

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/pa* \
		$(1)/usr/bin/
endef

define Package/pulseaudio-profiles/install
	$(INSTALL_DIR) \
		$(1)/usr/share/pulseaudio/alsa-mixer/paths \
		$(1)/usr/share/pulseaudio/alsa-mixer/profile-sets

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/pulseaudio/alsa-mixer/paths/* \
		$(1)/usr/share/pulseaudio/alsa-mixer/paths

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/pulseaudio/alsa-mixer/profile-sets/* \
		$(1)/usr/share/pulseaudio/alsa-mixer/profile-sets
endef

$(eval $(call BuildPackage,pulseaudio-daemon))
$(eval $(call BuildPackage,pulseaudio-daemon-avahi))
$(eval $(call BuildPackage,pulseaudio-tools))
$(eval $(call BuildPackage,pulseaudio-profiles))
