#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=squeezelite
PKG_VERSION:=1.9.6-1210
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ralph-irving/squeezelite
PKG_SOURCE_VERSION:=1b7a17616cd2bbd9935c710dc33cda11cd0de45e
PKG_MIRROR_HASH:=5993fa5fcacd1640962bc60f88a93faa4fcac74a767fbf820b40a27c338a9062

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE.txt

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/squeezelite/default
    SECTION:=sound
    CATEGORY:=Sound
    TITLE:=Headless squeezebox emulator
    PROVIDES:=squeezelite
    URL:=https://github.com/ralph-irving/squeezelite
    DEPENDS:= +alsa-lib +SQUEEZELITE_RESAMPLE:libsoxr
    MENU:=1
endef

define Package/squeezelite-full
    $(call Package/squeezelite/default)
    TITLE+= (full)
    DEPENDS+= +libflac +libvorbis +libmpg123 +libfaad2 \
              +SQUEEZELITE_WMA:libffmpeg-audio-dec
    VARIANT:=full
endef

define Package/squeezelite-mini
    $(call Package/squeezelite/default)
    TITLE+= (minimal)
    VARIANT:=mini
endef

define Package/squeezelite/config/default

	config SQUEEZELITE_WMA
	    bool "WMA/ALAC decode support"
	    depends on BUILD_PATENTED
	    help
		Include WMA and ALAC decoding using ffmpeg
	    default n

	config SQUEEZELITE_RESAMPLE
	    bool "Resample support"
	    help
		Include support for resampling using libsoxr
	    default n

	config SQUEEZELITE_DSD
	    bool "DSD playback over PCM (DoP)"
	    help
		Include support for DSD over PCM for compatible DAC"
	    default n
endef

define Package/squeezelite-full/config
    if PACKAGE_squeezelite-full
	$(call Package/squeezelite/config/default)
    endif
endef

define Package/squeezelite-mini/config
    if PACKAGE_squeezelite-mini
	$(call Package/squeezelite/config/default)
    endif
endef

define Package/squeezelite/description/default
    Squeezelite is a small headless squeezebox emulator for linux using alsa audio output
    It is aimed at supporting high quality audio at multiple sample rates including
    44.1/48/88.2/96/176.4/192k/352.8/384kHz
    Supported codecs: mp3, flac, ogg, aac, (wma and alac via ffmpeg)
    Native support for PCM builtin
    Optional support of DSD playback via PCM for DoP capable DAC
    Optional resampling to match sound device
endef

define Package/squeezelite-full/description
    $(call Package/squeezelite/description/default)

    This package has all the audio codecs compiled in.
endef

define Package/squeezelite-mini/description
    $(call Package/squeezelite/description/default)

    This package will dynamically load installed codecs.
endef

ifeq ($(CONFIG_SQUEEZELITE_WMA),y)
    opts+= -DFFMPEG
endif

ifeq ($(CONFIG_SQUEEZELITE_DSD),y)
    opts+= -DDSD
endif

ifeq ($(CONFIG_SQUEEZELITE_RESAMPLE),y)
    opts+= -DRESAMPLE
endif

ifeq ($(BUILD_VARIANT),full)
    opts+= -DLINKALL
endif

MAKE_FLAGS+=OPTS="$(opts)"

define Package/squeezelite/conffiles
/etc/config/squeezelite
endef

Package/squeezelite-mini/conffiles = $(Package/squeezelite/conffiles)
Package/squeezelite-full/conffiles = $(Package/squeezelite/conffiles)

define Package/squeezelite/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/squeezelite $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/squeezelite.init $(1)/etc/init.d/squeezelite
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/squeezelite.conf $(1)/etc/config/squeezelite
endef

Package/squeezelite-mini/install=$(Package/squeezelite/install)
Package/squeezelite-full/install=$(Package/squeezelite/install)

$(eval $(call BuildPackage,squeezelite-mini))
$(eval $(call BuildPackage,squeezelite-full))
