#
# Copyright (C) 2017-2020 Ian Leonard <antonlacon@gmail.com>
# Copyright (C) 2018 Ted Hess <thess@kitschensync.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ffmpeg
PKG_VERSION:=4.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://ffmpeg.org/releases/
PKG_HASH:=06b10a183ce5371f915c6bb15b7b1fffbe046e8275099c96affc29e17645d909
PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>, \
		Ian Leonard <antonlacon@gmail.com>

PKG_LICENSE:=LGPL-2.1-or-later GPL-2.0-or-later LGPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING.GPLv2 COPYING.GPLv3 COPYING.LGPLv2.1 COPYING.LGPLv3
PKG_CPE_ID:=cpe:/a:ffmpeg:ffmpeg

PKG_BUILD_PARALLEL:=1

FFMPEG_CUSTOM_ENCODERS:= \
	ac3 \
	jpegls \
	mpeg1video \
	mpeg2video \
	mpeg4 \
	pcm_s16be \
	pcm_s16le \
	png \
	vorbis \
	zlib \

FFMPEG_CUSTOM_DECODERS:= \
	aac \
	ac3 \
	alac \
	amrnb \
	amrwb \
	ape \
	atrac3 \
	flac \
	gif \
	h264 \
	hevc \
	jpegls \
	mp2 \
	mp3 \
	mpeg1video \
	mpeg2video \
	mpeg4 \
	mpegvideo \
	mpc7 \
	mpc8 \
	pcm_s16be \
	pcm_s16le \
	png \
	vc1 \
	vorbis \
	wavpack \
	wmav1 \
	wmav2 \
	zlib \

FFMPEG_CUSTOM_MUXERS:= \
	ac3 \
	ffm \
	h264 \
	hevc \
	mp3 \
	mp4 \
	mpeg1video \
	mpeg2video \
	mpegts \
	ogg \
	rtp \

FFMPEG_CUSTOM_DEMUXERS:= \
	aac \
	ac3 \
	amr \
	ape \
	avi \
	flac \
	ffm \
	h264 \
	hevc \
	matroska \
	mov \
	mp3 \
	mpegps \
	mpegts \
	mpegvideo \
	mpc \
	mpc8 \
	ogg \
	rm \
	rtsp \
	rtp \
	sdp \
	v4l2 \
	vc1 \
	wav \
	wv \

FFMPEG_CUSTOM_PARSERS:= \
	aac \
	flac \
	ac3 \
	h264 \
	hevc \
	mpegaudio \
	mpeg4video \
	mpegvideo \
	vc1 \

FFMPEG_CUSTOM_PROTOCOLS:= \
	file http icecast pipe rtp tcp udp

FFMPEG_MINI_DECODERS:= \
	ac3 \
	flac \
	jpegls \
	mp2 \
	mp3 \
	mpeg1video \
	mpeg2video \
	mpeg4 \
	mpegvideo \
	opus \
	png \
	vorbis \

FFMPEG_PATENTED_MINI_DECODERS:= \
	h264 \
	hevc \
	vc1 \

FFMPEG_MINI_DEMUXERS:= \
	avi \
	flac \
	matroska \
	mov \
	mp3 \
	mpegps \
	mpegts \
	mpegvideo \
	ogg \

FFMPEG_MINI_PROTOCOLS:= \
	file

FFMPEG_AUDIO_DECODERS:= \
	ac3 \
	adpcm_* \
	alac \
	amrnb \
	amrwb \
	ape \
	atrac3 \
	flac \
	mp2 \
	mp3* \
	mpc7 \
	mpc8 \
	opus \
	pcm_* \
	vorbis \
	wavpack \
	zlib \

FFMPEG_AUDIO_DEMUXERS:= \
	ac3 \
	aiff \
	amr \
	ape \
	avi \
	flac \
	ffm \
	matroska \
	mp3 \
	mov \
	mpc \
	mpc8 \
	mpegts \
	ogg \
	rm \
	rtsp \
	rtp \
	sdp \
	wav \
	wv \

FFMPEG_AUDIO_PROTOCOLS:= \
	file http icecast rtp tcp udp

FFMPEG_PATENTED_DECODERS:= \
	atrac3 \
	h264 \
	hevc \
	vc1 \

FFMPEG_PATENTED_MUXERS:= \
	h264 \
	hevc \
	vc1 \

FFMPEG_PATENTED_DEMUXERS:=$(FFMPEG_PATENTED_MUXERS)

FFMPEG_PATENTED_PARSERS:= \
	h264 \
	hevc \
	vc1 \

PKG_CONFIG_DEPENDS:= \
	$(patsubst %,CONFIG_FFMPEG_CUSTOM_ENCODER_%,$(FFMPEG_CUSTOM_ENCODERS)) \
	$(patsubst %,CONFIG_FFMPEG_CUSTOM_DECODER_%,$(FFMPEG_CUSTOM_DECODERS)) \
	$(patsubst %,CONFIG_FFMPEG_CUSTOM_MUXER_%,$(FFMPEG_CUSTOM_MUXERS)) \
	$(patsubst %,CONFIG_FFMPEG_CUSTOM_DEMUXER_%,$(FFMPEG_CUSTOM_DEMUXERS)) \
	$(patsubst %,CONFIG_FFMPEG_CUSTOM_PARSER_%,$(FFMPEG_CUSTOM_PARSERS)) \
	$(patsubst %,CONFIG_FFMPEG_CUSTOM_PROTOCOL_%,$(FFMPEG_CUSTOM_PROTOCOLS))

include $(INCLUDE_DIR)/package.mk

define Package/ffmpeg/Default
 TITLE:=FFmpeg
 URL:=https://ffmpeg.org/
 DEPENDS+= +libpthread
endef

define Package/ffmpeg/Default/description
 FFmpeg is a a software package that can record, convert and stream digital
 audio and video in numerous formats.

 FFmpeg licensing / patent issues are complex. It is the reponsibility of the
 user to understand any requirements in this regard with its usage. See:
 https://ffmpeg.org/legal.html for further information.
endef


define Package/ffmpeg
$(call Package/ffmpeg/Default)
 SECTION:=multimedia
 CATEGORY:=Multimedia
 TITLE+= program
 DEPENDS+= +libffmpeg-full
 VARIANT:=full
endef

define Package/ffmpeg-custom
$(call Package/ffmpeg/Default)
 SECTION:=multimedia
 CATEGORY:=Multimedia
 TITLE+= program (custom)
 DEPENDS+= @!ALL libffmpeg-custom
 VARIANT:=custom
endef

define Package/ffmpeg/description
$(call Package/ffmpeg/Default/description)
 .
 This package contains the FFmpeg command line tool.
endef

Package/ffmpeg-custom/description = $(Package/ffmpeg/description)

define Package/ffprobe
$(call Package/ffmpeg/Default)
 SECTION:=multimedia
 CATEGORY:=Multimedia
 TITLE+= CLI media identifier
 DEPENDS+= +libffmpeg-full
 VARIANT:=full
endef

define Package/ffprobe-custom
$(call Package/ffmpeg/Default)
 SECTION:=multimedia
 CATEGORY:=Multimedia
 TITLE+= CLI media identifier (custom)
 DEPENDS+= @!ALL libffmpeg-custom
 VARIANT:=custom
endef

define Package/ffprobe/description
$(call Package/ffmpeg/Default/description)
 .
 This package contains the FFprobe command line tool.
endef

Package/ffprobe-custom/description = $(Package/ffprobe/description)

define Package/libffmpeg/Default
$(call Package/ffmpeg/Default)
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE+= libraries
 DEPENDS+= +libpthread +zlib +libbz2
 PROVIDES:= libffmpeg
endef


# Package build suppressed when CONFIG_ALL=y
define Package/libffmpeg-custom
$(call Package/libffmpeg/Default)
 TITLE+= (custom)
 DEPENDS+= @!ALL \
           +FFMPEG_CUSTOM_SELECT_libopus:libopus \
           +PACKAGE_libx264:libx264 +PACKAGE_lame-lib:lame-lib \
           +FFMPEG_CUSTOM_SELECT_libshine:shine \
           +PACKAGE_fdk-aac:fdk-aac
 VARIANT:=custom
 MENU:=1
endef

define Package/libffmpeg-custom/config
source "$(SOURCE)/Config.in"
endef

define Package/libffmpeg-custom/description
$(call Package/ffmpeg/Default/description)
 .
 This package contains customized FFmpeg shared libraries.
endef


define Package/libffmpeg-audio-dec
$(call Package/libffmpeg/Default)
 TITLE+= (audio)
 VARIANT:=audio-dec
endef

define Package/libffmpeg-audio-dec/description
$(call Package/ffmpeg/Default/description)
 .
 This package contains FFmpeg shared libraries for audio decoding
endef


define Package/libffmpeg-full
$(call Package/libffmpeg/Default)
 TITLE+= (full)
 DEPENDS+= +alsa-lib +libgnutls +PACKAGE_libopus:libopus \
    +SOFT_FLOAT:shine \
    +!SOFT_FLOAT:lame-lib \
    +PACKAGE_libx264:libx264 \
    +!PACKAGE_libx264:fdk-aac
 VARIANT:=full
endef


define Package/libffmpeg-full/description
$(call Package/ffmpeg/Default/description)
 .
 This package contains full-featured FFmpeg shared libraries.
endef


define Package/libffmpeg-mini
$(call Package/libffmpeg/Default)
 TITLE+= (mini)
 VARIANT:=mini
endef

define Package/libffmpeg-mini/description
$(call Package/ffmpeg/Default/description)
 .
 This package contains minimal-featured FFmpeg shared libraries.
endef


# Strip off FPU notation
REAL_CPU_TYPE:=$(firstword $(subst +, ,$(CONFIG_CPU_TYPE)))
# Fixup cpu types recogized by ffmpeg configure
REAL_CPU_TYPE:=$(subst octeonplus,octeon+,$(REAL_CPU_TYPE))

FFMPEG_CONFIGURE:= \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(FPIC)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	./configure \
	--enable-cross-compile \
	--cross-prefix="$(TARGET_CROSS)" \
	--arch="$(ARCH)" \
	$(if $(REAL_CPU_TYPE),--cpu=$(call qstrip,$(REAL_CPU_TYPE)),) \
	--target-os=linux \
	--prefix="/usr" \
	--pkg-config="pkg-config" \
	--enable-shared \
	--enable-static \
	--enable-pthreads \
	--enable-zlib \
	--disable-doc \
	--disable-debug \
	\
	--disable-lzma \
	--disable-vaapi \
	--disable-vdpau \
	--disable-outdevs

ifeq ($(CONFIG_SOFT_FLOAT),y)
FFMPEG_CONFIGURE+= \
	--disable-altivec \
	--disable-vsx \
	--disable-power8 \
	--disable-armv5te \
	--disable-armv6 \
	--disable-armv6t2 \
	--disable-fast-unaligned \
	--disable-runtime-cpudetect

else ifneq ($(findstring arm,$(CONFIG_ARCH))$(findstring aarch64,$(CONFIG_ARCH)),)
FFMPEG_CONFIGURE+= \
	--disable-runtime-cpudetect
# XXX: GitHub issue 3320 ppc cpu with fpu but no altivec (WNDR4700)
else ifneq ($(findstring powerpc,$(CONFIG_ARCH)),)
FFMPEG_CONFIGURE+= \
	--disable-altivec
endif

# selectively disable optimizations according to arch/cpu type
ifneq ($(findstring arm,$(CONFIG_ARCH)),)
	FFMPEG_CONFIGURE+= --enable-lto

	ifneq ($(findstring vfp,$(CONFIG_CPU_TYPE)),)
		FFMPEG_CONFIGURE+= --enable-vfp
	else
		FFMPEG_CONFIGURE+= --disable-vfp
	endif
	ifneq ($(findstring neon,$(CONFIG_CPU_TYPE)),)
		FFMPEG_CONFIGURE+= \
			--enable-neon \
			--enable-vfp
	else
		FFMPEG_CONFIGURE+= --disable-neon
	endif
endif

ifneq ($(findstring aarch64,$(CONFIG_ARCH)),)
	FFMPEG_CONFIGURE+= \
		--enable-lto \
		--enable-neon \
		--enable-vfp
endif

ifeq ($(ARCH),x86_64)
	FFMPEG_CONFIGURE+= --enable-lto
endif

ifeq ($(CONFIG_FFMPEG_X86ASM),y)
  FFMPEG_CONFIGURE += --enable-x86asm
else
  FFMPEG_CONFIGURE += --disable-x86asm
endif

ifeq ($(BUILD_VARIANT),full)

  FFMPEG_DISABLE= \
	$(foreach c, $(2), \
		--disable-$(1)="$(c)" \
	)

	FFMPEG_CONFIGURE+= \
		--enable-gnutls \
		$(if $(CONFIG_BUILD_PATENTED),, \
			$(call FFMPEG_DISABLE,decoder,$(FFMPEG_PATENTED_DECODERS)) \
			$(call FFMPEG_DISABLE,muxer,$(FFMPEG_PATENTED_MUXERS)) \
			$(call FFMPEG_DISABLE,demuxer,$(FFMPEG_PATENTED_DEMUXERS)) \
			$(call FFMPEG_DISABLE,parser,$(FFMPEG_PATENTED_PARSERS))) \
		$(if $(CONFIG_PACKAGE_libopus),--enable-libopus)
  ifeq ($(CONFIG_SOFT_FLOAT),y)
	FFMPEG_CONFIGURE+= \
		--enable-small \
		\
		$(if $(CONFIG_PACKAGE_shine),--enable-libshine)
  else
	ifeq ($(ARCH),x86_64)
		FFMPEG_CONFIGURE+= --enable-hardcoded-tables
	else
		FFMPEG_CONFIGURE+= --enable-small
	endif
	FFMPEG_CONFIGURE+= $(if $(CONFIG_PACKAGE_lame-lib),--enable-libmp3lame)
  endif
  # x264 support and fdk-aac support can't coexist and be distributed.
  # Prioritize x264 over fdk-aac in default builds (maintain status-quo).
  ifneq ($(CONFIG_PACKAGE_libx264),)
	FFMPEG_CONFIGURE+= \
		--enable-gpl \
		--enable-libx264
  else
	FFMPEG_CONFIGURE+= $(if $(CONFIG_PACKAGE_fdk-aac),--enable-libfdk-aac)
  endif
endif

ifeq ($(BUILD_VARIANT),custom)

  FFMPEG_ENABLE= \
	$(foreach c, $(2), \
		$(if $($(3)_$(c)),--enable-$(1)="$(c)") \
	)

  ifeq ($(CONFIG_FFMPEG_CUSTOM_LARGE),y)
	FFMPEG_CONFIGURE+= \
		--enable-hardcoded-tables
  else
	FFMPEG_CONFIGURE+= \
		--enable-small
  endif

  ifeq ($(CONFIG_FFMPEG_CUSTOM_GPL),y)
	FFMPEG_CONFIGURE+= --enable-gpl
  endif

  ifeq ($(CONFIG_FFMPEG_CUSTOM_GPLV3),y)
	FFMPEG_CONFIGURE+= --enable-version3
  endif

  ifeq ($(CONFIG_FFMPEG_CUSTOM_NONFREE),y)
	FFMPEG_CONFIGURE+= --enable-nonfree
  endif

  ifneq ($(CONFIG_FFMPEG_CUSTOM_PROGRAMS),y)
      FFMPEG_CONFIGURE+= \
	--disable-programs \
	--disable-avfilter \
	--disable-swresample
  endif

  FFMPEG_CONFIGURE+= \
	--disable-swscale \
	--disable-everything \
	$(call FFMPEG_ENABLE,encoder,$(FFMPEG_CUSTOM_ENCODERS),CONFIG_FFMPEG_CUSTOM_ENCODER) \
	$(call FFMPEG_ENABLE,decoder,$(FFMPEG_CUSTOM_DECODERS),CONFIG_FFMPEG_CUSTOM_DECODER) \
	$(call FFMPEG_ENABLE,muxer,$(FFMPEG_CUSTOM_MUXERS),CONFIG_FFMPEG_CUSTOM_MUXER) \
	$(call FFMPEG_ENABLE,demuxer,$(FFMPEG_CUSTOM_DEMUXERS),CONFIG_FFMPEG_CUSTOM_DEMUXER) \
	$(call FFMPEG_ENABLE,parser,$(FFMPEG_CUSTOM_PARSERS),CONFIG_FFMPEG_CUSTOM_PARSER) \
	$(call FFMPEG_ENABLE,protocol,$(FFMPEG_CUSTOM_PROTOCOLS),CONFIG_FFMPEG_CUSTOM_PROTOCOL) \

ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_adpcm),y)
  FFMPEG_CONFIGURE+= \
	--enable-decoder=adpcm_ima_wav \
	--enable-decoder=adpcm_ima_qt \
	--enable-decoder=adpcm_ms
endif

ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libfdk-aac),y)
  FFMPEG_CONFIGURE+= \
	--enable-libfdk-aac --enable-encoder=libfdk_aac
endif

ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libmp3lame),y)
  FFMPEG_CONFIGURE+= \
	--enable-libmp3lame --enable-encoder=libmp3lame
endif

ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libopus),y)
  FFMPEG_CONFIGURE+= \
	--enable-libopus --enable-decoder=libopus --enable-encoder=libopus
endif

ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libshine),y)
  FFMPEG_CONFIGURE+= \
	--enable-libshine --enable-encoder=libshine
endif

ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libx264),y)
  FFMPEG_CONFIGURE+= \
	--enable-libx264 --enable-encoder=libx264
else
  FFMPEG_CONFIGURE+= --disable-postproc
endif

endif

ifeq ($(BUILD_VARIANT),audio-dec)

  FFMPEG_ENABLE= \
	$(foreach c, $(2), \
		--enable-$(1)="$(c)" \
	)

  FFMPEG_CONFIGURE+= \
	--enable-small \
	--enable-gpl \
	\
	--disable-programs \
	--disable-avfilter \
	--disable-postproc \
	--disable-swresample \
	--disable-swscale \
	--disable-everything \
	$(call FFMPEG_ENABLE,decoder,$(FFMPEG_AUDIO_DECODERS)) \
	$(call FFMPEG_ENABLE,demuxer,$(FFMPEG_AUDIO_DEMUXERS)) \
	$(call FFMPEG_ENABLE,protocol,$(FFMPEG_AUDIO_PROTOCOLS)) \
	--disable-decoder=pcm_bluray,pcm_dvd
endif

ifeq ($(BUILD_VARIANT),mini)

  FFMPEG_ENABLE= \
	$(foreach c, $(2), \
		--enable-$(1)="$(c)" \
	)

  FFMPEG_CONFIGURE+= \
	--enable-small \
	\
	--disable-programs \
	--disable-avdevice \
	--disable-avfilter \
	--disable-postproc \
	--disable-swresample \
	--disable-swscale \
	--disable-everything \
	$(call FFMPEG_ENABLE,decoder,$(FFMPEG_MINI_DECODERS)) \
	$(call FFMPEG_ENABLE,demuxer,$(FFMPEG_MINI_DEMUXERS)) \
	$(call FFMPEG_ENABLE,protocol,$(FFMPEG_MINI_PROTOCOLS)) \
	$(if $(CONFIG_BUILD_PATENTED), \
		$(call FFMPEG_ENABLE,decoder,$(FFMPEG_PATENTED_MINI_DECODERS)))
endif

ifneq ($(CONFIG_TARGET_x86),)
  TARGET_CFLAGS+= -fomit-frame-pointer
endif

define Build/Configure
	( cd $(PKG_BUILD_DIR); $(FFMPEG_CONFIGURE) )
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Build/InstallDev/custom
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lib{avcodec,avdevice,avformat,avutil} $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avdevice,avformat,avutil}.{a,so*} $(1)/usr/lib/
ifeq ($(CONFIG_FFMPEG_CUSTOM_PROGRAMS),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avfilter,swresample}.{a,so*} $(1)/usr/lib/
endif
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lib{avcodec,avdevice,avformat,avutil}.pc $(1)/usr/lib/pkgconfig/
ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libx264),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libpostproc $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpostproc.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libpostproc.pc $(1)/usr/lib/pkgconfig/
endif
endef

# Only ffmpeg with libx264 is GPL (yes libpostproc); all other builds are lgpl (no libpostproc)
define Build/InstallDev/full
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lib{avcodec,avdevice,avfilter,avformat,avutil,swresample,swscale} $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avdevice,avfilter,avformat,avutil,swresample,swscale}.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lib{avcodec,avdevice,avfilter,avformat,avutil,swresample,swscale}.pc $(1)/usr/lib/pkgconfig/
ifneq ($(CONFIG_PACKAGE_libx264),)
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libpostproc $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpostproc.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libpostproc.pc $(1)/usr/lib/pkgconfig/
endif
endef

define Build/InstallDev/mini
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lib{avcodec,avformat,avutil} $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avformat,avutil}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lib{avcodec,avformat,avutil}.pc $(1)/usr/lib/pkgconfig/
endef

Build/InstallDev/audio-dec = $(Build/InstallDev/custom)

# XXX: attempt at installing "best" dev files available
ifeq ($(BUILD_VARIANT),custom)
  # XXX: only install "custom" dev files if -full & -mini are not selected
  ifeq ($(CONFIG_PACKAGE_libffmpeg-full)$(CONFIG_PACKAGE_libffmpeg-mini),)
    Build/InstallDev = $(Build/InstallDev/custom)
  endif
endif
ifeq ($(BUILD_VARIANT),audio-dec)
  # XXX: only install "audio-dec" dev files if -full & -mini are not selected
  ifeq ($(CONFIG_PACKAGE_libffmpeg-full)$(CONFIG_PACKAGE_libffmpeg-mini),)
    Build/InstallDev = $(Build/InstallDev/audio-dec)
  endif
endif
ifeq ($(BUILD_VARIANT),full)
  # XXX: always install "full" dev files if -full is selected
  Build/InstallDev = $(Build/InstallDev/full)
endif
ifeq ($(BUILD_VARIANT),mini)
  # XXX: only install "mini" dev files if -full is not selected
  ifeq ($(CONFIG_PACKAGE_libffmpeg-full),)
    Build/InstallDev = $(Build/InstallDev/mini)
  endif
endif

define Package/ffmpeg/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ffmpeg $(1)/usr/bin/
endef

Package/ffmpeg-custom/install = $(Package/ffmpeg/install)

define Package/ffprobe/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ffprobe $(1)/usr/bin/
endef

Package/ffprobe-custom/install = $(Package/ffprobe/install)

define Package/libffmpeg-custom/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avdevice,avformat,avutil}.so.* $(1)/usr/lib/
ifeq ($(CONFIG_FFMPEG_CUSTOM_SELECT_libx264),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpostproc.so.* $(1)/usr/lib/
endif
ifeq ($(CONFIG_FFMPEG_CUSTOM_PROGRAMS),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avfilter,swresample}.so.* $(1)/usr/lib/
endif
endef

# Only ffmpeg with libx264 is GPL (yes libpostproc); all other builds are lgpl (no libpostproc)
define Package/libffmpeg-full/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avdevice,avfilter,avformat,avutil,swresample,swscale}.so.* $(1)/usr/lib/
ifneq ($(CONFIG_PACKAGE_libx264),)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpostproc.so.* $(1)/usr/lib/
endif
endef

define Package/libffmpeg-mini/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avformat,avutil}.so.* $(1)/usr/lib/
endef

Package/libffmpeg-audio-dec/install = $(Package/libffmpeg-custom/install)

$(eval $(call BuildPackage,ffmpeg))
$(eval $(call BuildPackage,ffprobe))
$(eval $(call BuildPackage,ffmpeg-custom))
$(eval $(call BuildPackage,ffprobe-custom))
$(eval $(call BuildPackage,libffmpeg-audio-dec))
$(eval $(call BuildPackage,libffmpeg-full))
$(eval $(call BuildPackage,libffmpeg-mini))
$(eval $(call BuildPackage,libffmpeg-custom))
