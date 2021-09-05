#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gst1-plugins-base
PKG_VERSION:=1.18.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=gst-plugins-base-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://gstreamer.freedesktop.org/src/gst-plugins-base
PKG_HASH:=29e53229a84d01d722f6f6db13087231cdf6113dd85c25746b9b58c3d68e8323
PKG_BUILD_DIR:=$(BUILD_DIR)/gst-plugins-base-$(PKG_VERSION)

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org> \
		Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=LGPL-2.1-or-later GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING.LIB COPYING
PKG_CPE_ID:=cpe:/a:gstreamer:gst-plugins-base

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_gst1-mod-alsa \
	CONFIG_PACKAGE_gst1-mod-app \
	CONFIG_PACKAGE_gst1-mod-audioconvert \
	CONFIG_PACKAGE_gst1-mod-audiorate \
	CONFIG_PACKAGE_gst1-mod-audioresample \
	CONFIG_PACKAGE_gst1-mod-audiotestsrc \
	CONFIG_PACKAGE_gst1-mod-playback \
	CONFIG_PACKAGE_gst1-mod-gio \
	CONFIG_PACKAGE_gst1-mod-ogg \
	CONFIG_PACKAGE_gst1-mod-opus \
	CONFIG_PACKAGE_gst1-mod-tcp \
	CONFIG_PACKAGE_gst1-mod-theora \
	CONFIG_PACKAGE_gst1-mod-typefind \
	CONFIG_PACKAGE_gst1-mod-videoconvert \
	CONFIG_PACKAGE_gst1-mod-videorate \
	CONFIG_PACKAGE_gst1-mod-videoscale \
	CONFIG_PACKAGE_gst1-mod-videotestsrc \
	CONFIG_PACKAGE_gst1-mod-volume \
	CONFIG_PACKAGE_gst1-mod-vorbis \
	CONFIG_PACKAGE_libgst1app

GST_BASE_LIBS:=
GST_BASE_MODULES:=

define Package/gstreamer1-base/Default
  CATEGORY:=Multimedia
  SECTION:=multimedia
  TITLE:=GStreamer
  URL:=https://gstreamer.freedesktop.org/
  DEPENDS:= +libgstreamer1 $(ICONV_DEPENDS)
endef

define Package/gstreamer1-base/description/Default
 GStreamer open source multimedia framework
endef


define Package/gst1-plugins-base
$(call Package/gstreamer1-base/Default)
  TITLE+= plugins collection (base)
  DEPENDS+= $(GST_DEPENDS)
  HIDDEN:=1
endef

define Package/gts1-plugins-base/description
$(call Package/gstreamer1-base/description/Default)
 .
 This meta package contains only dependencies to the other libraries and
 plugins from the base plugins collection.
endef

define Package/gstreamer1-plugins-base
  $(call Package/gstreamer1-base/Default)
  TITLE+= plugins collection (base)
  DEPENDS:=+ALL:gst1-plugins-base +gstreamer1-libs
endef

define Package/gstreamer1-plugins-base/config
    menu "Select GStreamer base modules and libraries"
      depends on PACKAGE_gstreamer1-plugins-base


    config gst1-plugins-base-all
      bool "Include all GStreamer base plugins"
      select PACKAGE_gst1-plugins-base

    comment "Modules"

   $(foreach mod,$(GST_BASE_MODULES), \
    config PACKAGE_gst1-mod-$(mod)
      prompt "GStreamer $(mod) module"

   )

    comment "Libraries"

   $(foreach lib,$(GST_BASE_LIBS), \
    config PACKAGE_libgst1$(lib)
      prompt "GStreamer $(lib) library"

   )

  endmenu

endef

GST_COND_SELECT = -D$(1)=$(if $(CONFIG_PACKAGE_gst1-mod-$(1)),en,dis)abled

GST_VERSION:=1.0

MESON_ARGS += \
	-Daudioresample_format=int \
	-Dgl=disabled \
	\
	$(call GST_COND_SELECT,adder) \
	$(call GST_COND_SELECT,app) \
	$(call GST_COND_SELECT,audioconvert) \
	$(call GST_COND_SELECT,audiomixer) \
	$(call GST_COND_SELECT,audiorate) \
	$(call GST_COND_SELECT,audioresample) \
	$(call GST_COND_SELECT,audiotestsrc) \
	$(call GST_COND_SELECT,compositor) \
	$(call GST_COND_SELECT,encoding) \
	$(call GST_COND_SELECT,gio) \
	$(call GST_COND_SELECT,overlaycomposition) \
	$(call GST_COND_SELECT,pbtypes) \
	$(call GST_COND_SELECT,playback) \
	$(call GST_COND_SELECT,rawparse) \
	$(call GST_COND_SELECT,subparse) \
	$(call GST_COND_SELECT,tcp) \
	-Dtypefind=$(if $(CONFIG_PACKAGE_gst1-mod-typefindfunctions),en,dis)abled \
	$(call GST_COND_SELECT,videoconvert) \
	$(call GST_COND_SELECT,videorate) \
	$(call GST_COND_SELECT,videoscale) \
	$(call GST_COND_SELECT,videotestsrc) \
	$(call GST_COND_SELECT,volume) \
	\
	$(call GST_COND_SELECT,alsa) \
	$(call GST_COND_SELECT,cdparanoia) \
	$(call GST_COND_SELECT,libvisual) \
	$(call GST_COND_SELECT,ogg) \
	$(call GST_COND_SELECT,opus) \
	$(call GST_COND_SELECT,pango) \
	$(call GST_COND_SELECT,theora) \
	$(call GST_COND_SELECT,tremor) \
	-Dtremor=$(if $(CONFIG_PACKAGE_gst1-mod-ivorbisdec),en,dis)abled \
	$(call GST_COND_SELECT,vorbis) \
	-Dx11=disabled \
	-Dxshm=disabled \
	-Dxvideo=disabled \
	\
	-Dexamples=disabled \
	-Dtests=disabled \
	-Dtools=disabled \
	-Dintrospection=disabled \
	-Dnls=enabled \
	-Dorc=disabled \
	-Dgobject-cast-checks=disabled \
	-Dglib-asserts=disabled \
	-Dglib-checks=disabled \
	-Ddoc=disabled

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/gstreamer-$(GST_VERSION)
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/include/gstreamer-$(GST_VERSION)/* \
		$(1)/usr/include/gstreamer-$(GST_VERSION)/ \
	)
	$(INSTALL_DIR) $(1)/usr/lib
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/libgst*-$(GST_VERSION).so* \
		$(1)/usr/lib/ \
	)
	if [ -d $(PKG_INSTALL_DIR)/usr/lib/gstreamer-$(GST_VERSION)/ ]; then \
		$(INSTALL_DIR) $(1)/usr/lib/gstreamer-$(GST_VERSION); \
		( cd $(PKG_INSTALL_DIR); $(FIND) \
			./usr/lib/gstreamer-$(GST_VERSION)/ -name libgst*.so -print0 | \
			xargs --null --no-run-if-empty $(CP) \
			--target-directory=$(1)/usr/lib/gstreamer-$(GST_VERSION)/ \
		) \
	fi
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/pkgconfig/gstreamer*-$(GST_VERSION).pc \
		$(1)/usr/lib/pkgconfig/ \
	)
endef


define Package/gst1-plugins-base/install
  true
endef

define Package/gstreamer1-plugins-base/install
  true
endef

# 1: short name
# 2: description
# 3: dependencies on other gstreamer libraries (short name)
# 4: dependencies on other packages
define GstBuildLibrary

  GST_DEPENDS += +libgst1$(1)

  GST_BASE_LIBS+= $(1)

  define Package/libgst1$(1)
    $(call Package/gstreamer1-base/Default)
    TITLE+= $(2) library (base)
    DEPENDS+= $$(foreach p,$(3),+libgst1$$(p)) $(4)
    HIDDEN:=1
  endef

  define Package/libgst1$(1)/description
   $(call Package/gstreamer1-base/description/Default)
   .
   This package contains the GStreamer $(2) library.
  endef

  define Package/libgst1$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/libgst$(1)-$(GST_VERSION).so.* \
		$$(1)/usr/lib/ \
	)
  endef

  $$(eval $$(call BuildPackage,libgst1$(1)))
endef

$(eval $(call GstBuildLibrary,allocators,allocators,,))
$(eval $(call GstBuildLibrary,app,app,,))
$(eval $(call GstBuildLibrary,audio,audio,tag,))
$(eval $(call GstBuildLibrary,fft,FFT,,))
$(eval $(call GstBuildLibrary,pbutils,utils,audio tag video,))
$(eval $(call GstBuildLibrary,riff,RIFF media,audio tag,,))
$(eval $(call GstBuildLibrary,rtp,RTP,,))
$(eval $(call GstBuildLibrary,rtsp,RTSP,,))
$(eval $(call GstBuildLibrary,sdp,SDP,rtp,))
$(eval $(call GstBuildLibrary,tag,tag support,,))
$(eval $(call GstBuildLibrary,video,video,,))

# 1: short name
# 2: description
# 3: dependencies on other gstreamer libraries (short name)
# 4: dependencies on other gstreamer plugins (short name)
# 5: dependencies on other packages
define GstBuildPlugin

  GST_DEPENDS += +gst1-mod-$(1)

  GST_BASE_MODULES+= $(1)

  define Package/gst1-mod-$(1)
    $(call Package/gstreamer1-base/Default)
    TITLE+= $(2) plugin (base)
    DEPENDS+= $$(foreach p,$(3),+libgst1$$(p)) $$(foreach p,$(4),+gst1-mod-$$(p)) $(5)
    HIDDEN:=1
  endef

  define Package/gst1-mod-$(1)/description
   $(call Package/gstreamer1-base/description/Default)
   .
   This package contains the GStreamer $(2) plugin.
  endef

  define Package/gst1-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/gstreamer-$(GST_VERSION)
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/gstreamer-$(GST_VERSION)/libgst$(1).so* \
		$$(1)/usr/lib/gstreamer-$(GST_VERSION)/ \
	)
  endef

  $$(eval $$(call BuildPackage,gst1-mod-$(1)))
endef

$(eval $(call GstBuildPlugin,adder,adder,audio,,))
$(eval $(call GstBuildPlugin,app,app,app,,))
$(eval $(call GstBuildPlugin,audioconvert,audio format conversion,audio,,))
$(eval $(call GstBuildPlugin,audiomixer,audio mixing,audio,,))
$(eval $(call GstBuildPlugin,audiorate,audio rate adjusting,audio tag,,))
$(eval $(call GstBuildPlugin,audioresample,audio resampling,audio tag,,))
$(eval $(call GstBuildPlugin,audiotestsrc,audio test,audio tag controller,,))
$(eval $(call GstBuildPlugin,compositor,video compositor,video,,))
$(eval $(call GstBuildPlugin,encoding,encoding plugin,pbutils video,,))
$(eval $(call GstBuildPlugin,gio,GIO,,,))
$(eval $(call GstBuildPlugin,overlaycomposition,overlay composition,video,,))
$(eval $(call GstBuildPlugin,pbtypes,pbtypes,video,,))
$(eval $(call GstBuildPlugin,playback,playback,pbutils,,))
$(eval $(call GstBuildPlugin,subparse,subparse,video,,))
$(eval $(call GstBuildPlugin,rawparse,parse raw media,audio video,,))
$(eval $(call GstBuildPlugin,tcp,TCP,net,,))
$(eval $(call GstBuildPlugin,typefindfunctions,'typefind' functions,audio pbutils tag video,,))
$(eval $(call GstBuildPlugin,videoconvert,video format conversion,video,,))
$(eval $(call GstBuildPlugin,videorate,Adjusts video frames,video,,))
$(eval $(call GstBuildPlugin,videoscale,Resizes video,video,,))
$(eval $(call GstBuildPlugin,videotestsrc,video test,video,,+liboil))
$(eval $(call GstBuildPlugin,volume,volume,audio controller,,+liboil))

$(eval $(call GstBuildPlugin,alsa,ALSA audio source/sink,audio tag,,+alsa-lib))
$(eval $(call GstBuildPlugin,ivorbisdec,Integer Vorbis decoder plugin for devices without floating point,audio tag,,+libvorbisidec))
#$(eval $(call GstBuildPlugin,cdparanoia,cdparanoia,,+libcdparanoia))
#$(eval $(call GstBuildPlugin,libvisual,libvisual audio visualization,,+libvisual))
$(eval $(call GstBuildPlugin,ogg,ogg parser muxer demuxer,riff tag pbutils video,,+libogg))
$(eval $(call GstBuildPlugin,opus,OPUS audio codec,pbutils video,,+libopus))
#$(eval $(call GstBuildPlugin,pango,Pango text rendering and overlay plugin,,+pango))
$(eval $(call GstBuildPlugin,theora,Theora,tag video,,+libogg +libtheora))
$(eval $(call GstBuildPlugin,vorbis,Vorbis,audio tag,ogg,+libvorbis))

$(eval $(call BuildPackage,gstreamer1-plugins-base))
$(eval $(call BuildPackage,gst1-plugins-base))
