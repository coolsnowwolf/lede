#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gst1-libav
PKG_VERSION:=1.18.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=gst-libav-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://gstreamer.freedesktop.org/src/gst-libav
PKG_HASH:=344a463badca216c2cef6ee36f9510c190862bdee48dc4591c0a430df7e8c396
PKG_BUILD_DIR:=$(BUILD_DIR)/gst-libav-$(PKG_VERSION)

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org> \
		Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-or-later LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING COPYING.LIB

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

GST_VERSION:=1.0

LIBAV_ENCODERS:= \
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

LIBAV_DECODERS:= \
	aac \
	ac3 \
	atrac3 \
	gif \
	h264 \
	jpegls \
	mp2 \
	mp3 \
	mpeg1video \
	mpeg2video \
	mpeg4 \
	mpegvideo \
	pcm_s16be \
	pcm_s16le \
	png \
	vorbis \
	wmav1 \
	wmav2 \
	zlib \

LIBAV_MUXERS:= \
	ac3 \
	ffm \
	h264 \
	mp3 \
	mp4 \
	mpeg1video \
	mpeg2video \
	mpegts \
	ogg \
	oss \
	rtp \

LIBAV_DEMUXERS:= \
	ac3 \
	ffm \
	h264 \
	mp3 \
	mpegps \
	mpegts \
	mpegvideo \
	ogg \
	rm \
	rtsp \
	sdp \
	v4l2 \

LIBAV_PARSERS:= \
	aac \
	ac3 \
	h264 \
	mpegaudio \
	mpegvideo \
	mpeg4video \

LIBAV_PROTOCOLS:= \
	file http pipe rtp tcp udp

PKG_CONFIG_DEPENDS:= \
	$(patsubst %,CONFIG_GST1_LIBAV_ENCODER_%,$(LIBAV_ENCODERS)) \
	$(patsubst %,CONFIG_GST1_LIBAV_DECODER_%,$(LIBAV_DECODERS)) \
	$(patsubst %,CONFIG_GST1_LIBAV_MUXER_%,$(LIBAV_DEMUXERS)) \
	$(patsubst %,CONFIG_GST1_LIBAV_DEMUXER_%,$(LIBAV_DEMUXERS)) \
	$(patsubst %,CONFIG_GST1_LIBAV_PARSER_%,$(LIBAV_PARSERS)) \
	$(patsubst %,CONFIG_GST1_LIBAV_PROTOCOL_%,$(LIBAV_PROTOCOLS))

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/gst1-libav
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=GStreamer Libav plugin
  URL:=https://www.gstreamer.net/modules/gst-libav.html
  DEPENDS:= +libgstreamer1 +gstreamer1-plugins-base \
	    +gst1-mod-alsa +libgst1audio +libgst1pbutils +libgst1video \
	    +libbz2 +liblzma +libffmpeg-full
endef

define Package/gst1-libav/description
  GStreamer with libav bindings using internal Libav
endef

define Package/gst1-libav/config
  source "$(SOURCE)/Config.in"
endef

FILTER_CONFIG= \
	$(foreach c, $(3), \
		$(if $(CONFIG_GST1_LIBAV_$(1)_$(c)),--enable-$(2)='$(c)') \
	)

LIBAV_CONFIGURE_ENCODERS:=$(call FILTER_CONFIG,ENCODER,encoder,$(LIBAV_ENCODERS))
LIBAV_CONFIGURE_DECODERS:=$(call FILTER_CONFIG,DECODER,decoder,$(LIBAV_DECODERS))
LIBAV_CONFIGURE_MUXERS:=$(call FILTER_CONFIG,MUXER,muxer,$(LIBAV_MUXERS))
LIBAV_CONFIGURE_DEMUXERS:=$(call FILTER_CONFIG,DEMUXER,demuxer,$(LIBAV_DEMUXERS))
LIBAV_CONFIGURE_PARSERS:=$(call FILTER_CONFIG,PARSER,parser,$(LIBAV_PARSERS))
LIBAV_CONFIGURE_PROTOCOLS:=$(call FILTER_CONFIG,PROTOCOL,protocol,$(LIBAV_PROTOCOLS))

# Hack to build on mips64.
ifneq ($(findstring octeonplus,$(CONFIG_CPU_TYPE)),)
  REAL_CPU_TYPE:=octeon+
else
  # Strip off FPU notation.
  REAL_CPU_TYPE:=$(firstword $(subst +, ,$(CONFIG_CPU_TYPE)))
endif

CONFIGURE_ARGS += \
	--disable-Bsymbolic \
	--disable-fatal-warnings \
	--disable-glib-asserts \
	--disable-gobject-cast-checks \
	--disable-gtk-doc-html \
	--disable-valgrind \
	--without-system-libav \
	--with-pic \
	--with-libav-extra-configure=" \
	$(if $(REAL_CPU_TYPE),--cpu=$(call qstrip,$(REAL_CPU_TYPE)),) \
	--disable-bsfs \
	--disable-programs \
	--disable-devices \
	--disable-debug \
	--disable-doc \
	--disable-extra-warnings \
	--disable-htmlpages \
	--disable-manpages \
	--disable-podpages \
	--disable-txtpages \
	--disable-runtime-cpudetect \
	--disable-encoders \
	$(LIBAV_CONFIGURE_ENCODERS) \
	--disable-decoders \
	$(LIBAV_CONFIGURE_DECODERS) \
	--disable-muxers \
	$(LIBAV_CONFIGURE_MUXERS) \
	--disable-demuxers \
	$(LIBAV_CONFIGURE_DEMUXERS) \
	--disable-parsers \
	$(LIBAV_CONFIGURE_PARSERS) \
	--disable-protocols \
	$(LIBAV_CONFIGURE_PROTOCOLS) \
	--disable-asm \
	--disable-altivec \
	--disable-amd3dnow \
	--disable-amd3dnowext \
	--disable-mmx \
	--disable-mmxext \
	--disable-sse \
	--disable-sse2 \
	--disable-sse3 \
	--disable-ssse3 \
	--disable-sse4 \
	--disable-sse42 \
	--disable-avx \
	--disable-xop \
	--disable-fma3 \
	--disable-fma4 \
	--disable-avx2 \
	--disable-inline-asm \
	--disable-x86asm"

define Package/gst1-libav/install
	$(INSTALL_DIR) $(1)/usr/lib/gstreamer-$(GST_VERSION)
	( cd $(PKG_INSTALL_DIR); $(CP) \
                ./usr/lib/gstreamer-$(GST_VERSION)/libgstlibav.so* \
                $(1)/usr/lib/gstreamer-$(GST_VERSION)/ \
        )
endef

$(eval $(call BuildPackage,gst1-libav))
