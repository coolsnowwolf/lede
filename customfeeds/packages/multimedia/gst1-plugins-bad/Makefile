#
# Copyright (C) 2011-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gst1-plugins-bad
PKG_VERSION:=1.18.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=gst-plugins-bad-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://gstreamer.freedesktop.org/src/gst-plugins-bad/
PKG_HASH:=74e806bc5595b18c70e9ca93571e27e79dfb808e5d2e7967afa952b52e99c85f
PKG_BUILD_DIR:=$(BUILD_DIR)/gst-plugins-bad-$(PKG_VERSION)

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org> \
		Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=LGPL-2.1-or-later GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING.LIB COPYING

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

GST_BAD_LIBS:=
GST_BAD_MODULES:=

define Package/gstreamer1-bad/Default
  CATEGORY:=Multimedia
  SECTION:=multimedia
  TITLE:=GStreamer
  URL:=http://gstreamer.freedesktop.org/
  DEPENDS:= +gst1-plugins-base +libgstreamer1 +libgst1pbutils $(ICONV_DEPENDS)
endef

define Package/gstreamer1-bad/description/Default
 GStreamer open source multimedia framework
endef


define Package/gst1-plugins-bad
$(call Package/gstreamer1-bad/Default)
  TITLE+= plugins collection (bad)
  DEPENDS+= $(GST_DEPENDS)
  HIDDEN:=1
endef

define Package/gts1-plugins-bad/description
$(call Package/gstreamer1-bad/description/Default)
 .
 This meta package contains only dependencies to the other plugins from
 the bad plugins collection.
endef

define Package/gstreamer1-plugins-bad
  $(call Package/gstreamer1-bad/Default)
  TITLE+= plugins collection (bad)
  DEPENDS:=+ALL:gst1-plugins-bad +gstreamer1-plugins-base
endef

define Package/gstreamer1-plugins-bad/config
    menu "Select GStreamer bad modules and libraries"
      depends on PACKAGE_gstreamer1-plugins-bad


    config gst1-plugins-bad-all
      bool "Include all GStreamer bad plugins"
      select PACKAGE_gst1-plugins-bad

    comment "Modules"

   $(foreach mod,$(GST_BAD_MODULES), \
    config PACKAGE_gst1-mod-$(mod)
      prompt "GStreamer $(mod) module"

   )

    comment "Libraries"

   $(foreach lib,$(GST_BAD_LIBS), \
    config PACKAGE_libgst1$(lib)
      prompt "GStreamer $(lib) library"

   )

  endmenu

endef

GST_COND_SELECT = -D$(1)=$(if $(CONFIG_PACKAGE_gst1-mod-$(1)),en,dis)abled

GST_VERSION:=1.0

MESON_ARGS += \
	$(call GST_COND_SELECT,accurip) \
	$(call GST_COND_SELECT,adpcmdec) \
	$(call GST_COND_SELECT,adpcmenc) \
	$(call GST_COND_SELECT,aiff) \
	$(call GST_COND_SELECT,asfmux) \
	$(call GST_COND_SELECT,audiobuffersplit) \
	$(call GST_COND_SELECT,audiofxbad) \
	$(call GST_COND_SELECT,audiolatency) \
	$(call GST_COND_SELECT,audiomixmatrix) \
	$(call GST_COND_SELECT,audiovisualizers) \
	$(call GST_COND_SELECT,autoconvert) \
	$(call GST_COND_SELECT,bayer) \
	-Dcamerabin2=$(if $(CONFIG_PACKAGE_gst1-mod-camerabin),en,dis)abled \
	$(call GST_COND_SELECT,coloreffects) \
	-Ddebugutils=$(if $(CONFIG_PACKAGE_gst1-mod-debugutilsbad),en,dis)abled \
	$(call GST_COND_SELECT,dvbsubenc) \
	$(call GST_COND_SELECT,dvbsuboverlay) \
	$(call GST_COND_SELECT,dvdspu) \
	$(call GST_COND_SELECT,faceoverlay) \
	$(call GST_COND_SELECT,festival) \
	$(call GST_COND_SELECT,fieldanalysis) \
	$(call GST_COND_SELECT,freeverb) \
	$(call GST_COND_SELECT,frei0r) \
	$(call GST_COND_SELECT,gaudieffects) \
	$(call GST_COND_SELECT,gdp) \
	$(call GST_COND_SELECT,geometrictransform) \
	$(call GST_COND_SELECT,id3tag) \
	$(call GST_COND_SELECT,inter) \
	$(call GST_COND_SELECT,interlace) \
	$(call GST_COND_SELECT,ivfparse) \
	$(call GST_COND_SELECT,ivtc) \
	$(call GST_COND_SELECT,jp2kdecimator) \
	$(call GST_COND_SELECT,jpegformat) \
	-Dlibrfb=$(if $(CONFIG_PACKAGE_gst1-mod-rfbsrc),en,dis)abled \
	$(call GST_COND_SELECT,midi) \
	-Dmpegdemux=$(if $(CONFIG_PACKAGE_gst1-mod-mpegpsdemux),en,dis)abled \
	$(call GST_COND_SELECT,mpegpsmux) \
	$(call GST_COND_SELECT,mpegtsdemux) \
	$(call GST_COND_SELECT,mpegtsmux) \
	$(call GST_COND_SELECT,mxf) \
	$(call GST_COND_SELECT,netsim) \
	-Donvif=$(if $(CONFIG_PACKAGE_gst1-mod-rtponvif),en,dis)abled \
	$(call GST_COND_SELECT,pcapparse) \
	$(call GST_COND_SELECT,pnm) \
	$(call GST_COND_SELECT,proxy) \
	-Drawparse=$(if $(CONFIG_PACKAGE_gst1-mod-legacyrawparse),en,dis)abled \
	$(call GST_COND_SELECT,removesilence) \
	-Dsdp=$(if $(CONFIG_PACKAGE_gst1-mod-sdpelem),en,dis)abled \
	$(call GST_COND_SELECT,segmentclip) \
	$(call GST_COND_SELECT,siren) \
	$(call GST_COND_SELECT,smooth) \
	$(call GST_COND_SELECT,speed) \
	$(call GST_COND_SELECT,subenc) \
	$(call GST_COND_SELECT,switchbin) \
	$(call GST_COND_SELECT,timecode) \
	-Dvideofilters=$(if $(CONFIG_PACKAGE_gst1-mod-videofiltersbad),en,dis)abled \
	$(call GST_COND_SELECT,videoframe_audiolevel) \
	-Dvideoparsers=$(if $(CONFIG_PACKAGE_gst1-mod-videoparsersbad),en,dis)abled \
	$(call GST_COND_SELECT,videosignal) \
	$(call GST_COND_SELECT,vmnc) \
	-Dy4m=$(if $(CONFIG_PACKAGE_gst1-mod-y4mdec),en,dis)abled \
	\
	-Dopencv=disabled \
	-Dwayland=disabled \
	-Dx11=disabled \
	\
	-Daom=disabled \
	-Davtp=disabled \
	-Dandroidmedia=disabled \
	-Dapplemedia=disabled \
	-Dassrender=disabled \
	$(call GST_COND_SELECT,bluez) \
	-Dbs2b=disabled \
	$(call GST_COND_SELECT,bz2) \
	-Dchromaprint=disabled \
	-Dclosedcaption=disabled \
	-Dcolormanagement=disabled \
	-Dcurl=disabled \
	-Dcurl-ssh2=disabled \
	-Dd3dvideosink=disabled \
	-Dd3d11=disabled \
	-Ddash=disabled \
	-Ddc1394=disabled \
	-Ddecklink=disabled \
	-Ddirectfb=disabled \
	-Ddirectsound=disabled \
	$(call GST_COND_SELECT,dtls) \
	-Ddts=disabled \
	-Ddvb=disabled \
	-Dfaac=disabled \
	$(call GST_COND_SELECT,faad) \
	-Dfbdev=$(if $(CONFIG_PACKAGE_gst1-mod-fbdevsink),en,dis)abled \
	-Dfdkaac=disabled \
	-Dflite=disabled \
	-Dfluidsynth=disabled \
	-Dgl=disabled \
	-Dgme=disabled \
	-Dgsm=disabled \
	-Dipcpipeline=disabled \
	-Diqa=disabled \
	-Dkate=disabled \
	-Dkms=disabled \
	-Dladspa=disabled \
	-Dlibde265=disabled \
	-Dlibmms=disabled \
	-Dlv2=disabled \
	-Dmediafoundation=disabled \
	-Dmicrodns=disabled \
	-Dmodplug=disabled \
	-Dmpeg2enc=disabled \
	-Dmplex=disabled \
	-Dmsdk=disabled \
	-Dmusepack=disabled \
	-Dneon=disabled \
	-Dnvcodec=disabled \
	-Dofa=disabled \
	-Dopenal=disabled \
	-Dopenexr=disabled \
	-Dopenh264=disabled \
	-Dopenjpeg=disabled \
	-Dopenmpt=disabled \
	-Dopenni2=disabled \
	-Dopensles=disabled \
	-Dopus=$(if $(CONFIG_PACKAGE_gst1-mod-opusparse),en,dis)abled \
	-Dresindvd=disabled \
	-Drsvg=disabled \
	-Drtmp=disabled \
	$(call GST_COND_SELECT,sbc) \
	-Dsctp=disabled \
	$(call GST_COND_SELECT,shm) \
	-Dsmoothstreaming=disabled \
	$(call GST_COND_SELECT,sndfile) \
	-Dsoundtouch=disabled \
	-Dspandsp=disabled \
	-Dsrt=disabled \
	-Dsrtp=disabled \
	-Dteletext=disabled \
	-Dtinyalsa=disabled \
	-Dttml=disabled \
	-Duvch264=disabled \
	-Dvoaacenc=disabled \
	-Dvoamrwbenc=disabled \
	-Dvulkan=disabled \
	-Dwasapi=disabled \
	-Dwasapi2=disabled \
	$(call GST_COND_SELECT,webp) \
	-Dwebrtc=disabled \
	-Dwebrtcdsp=disabled \
	-Dwildmidi=disabled \
	-Dwinks=disabled \
	-Dwinscreencap=disabled \
	-Dx265=disabled \
	-Dzbar=disabled \
	-Dzxing=disabled \
	-Dwpe=disabled \
	-Dmagicleap=disabled \
	-Dv4l2codecs=disabled \
	\
	$(call GST_COND_SELECT,hls) \
	-Dhls-crypto=nettle \
	-Dexamples=disabled \
	-Dtests=disabled \
	-Dintrospection=disabled \
	-Dorc=disabled \
	-Dgobject-cast-checks=disabled \
	-Dglib-asserts=disabled \
	-Dglib-checks=disabled \
	-Dextra-checks=disabled \
	-Ddoc=disabled

define Package/gst1-plugins-bad/install
	/bin/true
endef

# 1: short name
# 2: description
# 3: dependencies on other gstreamer libraries (short name)
# 4: dependencies on other packages
define GstBuildLibrary

  GST_DEPENDS += +libgst1$(1)

  GST_BAD_LIBS+= $(1)
  define Package/libgst1$(1)
    $(call Package/gstreamer1-bad/Default)
    TITLE+= $(2) library (base)
    DEPENDS+= $$(foreach p,$(3),+libgst1$$(p)) $(4)
    HIDDEN:=1
  endef

  define Package/libgst1$(1)/description
   $(call Package/gstreamer1-bad/description/Default)
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

$(eval $(call GstBuildLibrary,adaptivedemux,adaptivedemux,app uridownloader,))
$(eval $(call GstBuildLibrary,photography,photography,,))
$(eval $(call GstBuildLibrary,basecamerabinsrc,basecamerabinsrc,app,))
$(eval $(call GstBuildLibrary,uridownloader,uridownloader,,))
$(eval $(call GstBuildLibrary,codecparsers,codecparsers,,))
$(eval $(call GstBuildLibrary,mpegts,mpegts,,))

# 1: short name
# 2: description
# 3: dependencies on other gstreamer libraries (short name)
# 4: dependencies on other gstreamer plugins (short name)
# 5: dependencies on other packages
define GstBuildPlugin

  GST_DEPENDS += +gst1-mod-$(1)

  GST_BAD_MODULES+= $(1)

  define Package/gst1-mod-$(1)
    $(call Package/gstreamer1-bad/Default)
    TITLE+= $(2) plugin (bad)
    DEPENDS+= $$(foreach p,$(3),+libgst1$$(p)) $$(foreach p,$(4),+gst1-mod-$$(p)) $(5)
    HIDDEN:=1
  endef

  define Package/gst1-mod-$(1)/description
   $(call Package/gstreamer1-bad/description/Default)
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

$(eval $(call GstBuildPlugin,adpcmdec,adpcm decoding support,audio,,))
$(eval $(call GstBuildPlugin,adpcmenc,adpcm encoding support,audio,,))
$(eval $(call GstBuildPlugin,aiff,aiff support,audio tag,,))
$(eval $(call GstBuildPlugin,asfmux,asf muxing support,rtp,,))
$(eval $(call GstBuildPlugin,audiobuffersplit,audiobuffersplit support,audio,,))
$(eval $(call GstBuildPlugin,audiofxbad,audiofxbad support,audio,,))
$(eval $(call GstBuildPlugin,audiolatency,audiolatency support,audio,,))
$(eval $(call GstBuildPlugin,audiomixmatrix,audiomixmatrix support,audio,,))
$(eval $(call GstBuildPlugin,audiovisualizers,audiovisualizers support,audio,,))
$(eval $(call GstBuildPlugin,autoconvert,autoconvert support,,,))
$(eval $(call GstBuildPlugin,bayer,bayer support,video,,))
$(eval $(call GstBuildPlugin,camerabin,camerabin support,basecamerabinsrc photography pbutils app,,))
$(eval $(call GstBuildPlugin,coloreffects,coloreffects support,video,,))
$(eval $(call GstBuildPlugin,debugutilsbad,debugutils support,video,,))
$(eval $(call GstBuildPlugin,dvbsubenc,dvbsubenc support,video,,))
$(eval $(call GstBuildPlugin,dvbsuboverlay,dvbsuboverlay support,video,,))
$(eval $(call GstBuildPlugin,dvdspu,dvdspu support,video,,))
$(eval $(call GstBuildPlugin,faceoverlay,faceoverlay support,pbutils,,))
$(eval $(call GstBuildPlugin,festival,festival support,audio,,))
$(eval $(call GstBuildPlugin,fieldanalysis,fieldanalysis support,audio,,))
$(eval $(call GstBuildPlugin,freeverb,freeverb support,,,))
$(eval $(call GstBuildPlugin,frei0r,frei0r support,controller video,,))
$(eval $(call GstBuildPlugin,gaudieffects,gaudieffects support,audio,,))
$(eval $(call GstBuildPlugin,gdp,gdp support,,,))
$(eval $(call GstBuildPlugin,geometrictransform,geometrictransform support,,,))
$(eval $(call GstBuildPlugin,id3tag,id3tag support,tag,,))
$(eval $(call GstBuildPlugin,inter,inter support,,,))
$(eval $(call GstBuildPlugin,interlace,interlace support,,,))
$(eval $(call GstBuildPlugin,ivfparse,ivfparse support,,,))
$(eval $(call GstBuildPlugin,ivtc,ivtc support,,,))
$(eval $(call GstBuildPlugin,jp2kdecimator,jp2kdecimator support,,,))
$(eval $(call GstBuildPlugin,jpegformat,jpegformat support,tag,,))
$(eval $(call GstBuildPlugin,legacyrawparse,rawparse support,audio video,,))
$(eval $(call GstBuildPlugin,midi,midi support,audio,,))
$(eval $(call GstBuildPlugin,mpegpsdemux,mpegpsdemux support,pbutils,,))
$(eval $(call GstBuildPlugin,mpegpsmux,mpegpsmux support,,,))
$(eval $(call GstBuildPlugin,mpegtsdemux,mpegtsdemux support,codecparsers mpegts pbutils,,))
$(eval $(call GstBuildPlugin,mpegtsmux,mpegtsmux support,mpegts video,,))
$(eval $(call GstBuildPlugin,mxf,mxf support,audio video,,))
$(eval $(call GstBuildPlugin,netsim,netsim support,,,))
$(eval $(call GstBuildPlugin,pcapparse,pcapparse support,,,))
$(eval $(call GstBuildPlugin,pnm,pnm support,video,,))
$(eval $(call GstBuildPlugin,proxy,proxy support,,,))
$(eval $(call GstBuildPlugin,removesilence,removesilence support,audio,,))
$(eval $(call GstBuildPlugin,rfbsrc,librfb support,video,,))
$(eval $(call GstBuildPlugin,rtponvif,rtponvif support,,,))
$(eval $(call GstBuildPlugin,sdpelem,sdp support,rtp sdp,,))
$(eval $(call GstBuildPlugin,segmentclip,segmentclip support,audio,,))
$(eval $(call GstBuildPlugin,siren,siren support,audio rtp,,))
$(eval $(call GstBuildPlugin,smooth,smooth support,,,))
$(eval $(call GstBuildPlugin,speed,speed support,audio,,))
$(eval $(call GstBuildPlugin,subenc,subenc support,controller,,))
$(eval $(call GstBuildPlugin,switchbin,switchbin support,,,))
$(eval $(call GstBuildPlugin,timecode,timecode support,,,))
$(eval $(call GstBuildPlugin,videofiltersbad,videofiltersbad support,,,))
$(eval $(call GstBuildPlugin,videoframe_audiolevel,videoframe_audiolevel support,,,))
#$(eval $(call GstBuildPlugin,videoparsersbad,videoparsersbad support,,,))
$(eval $(call GstBuildPlugin,videosignal,videosignal support,,,))
$(eval $(call GstBuildPlugin,vmnc,vmnc support,,,))
$(eval $(call GstBuildPlugin,y4mdec,y4mdec support,,,))

$(eval $(call GstBuildPlugin,bluez,bluez support,,,+bluez-libs))
#$(eval $(call GstBuildPlugin,bz2,bzip2 support,,,+libbz2))
#$(eval $(call GstBuildPlugin,dataurisrc,dataurisrc support,,,))
$(eval $(call GstBuildPlugin,dtls,DTLS support,,,+libopenssl))
$(eval $(call GstBuildPlugin,faad,faad support,audio,,+libfaad2))
$(eval $(call GstBuildPlugin,fbdevsink,fbdev support,video,,))
$(eval $(call GstBuildPlugin,hls,HLS support,pbutils adaptivedemux,multifile,+libnettle))
$(eval $(call GstBuildPlugin,opusparse,OPUS streams library,pbutils,,+libopus))
$(eval $(call GstBuildPlugin,sbc,sbc support,audio,,+sbc))
$(eval $(call GstBuildPlugin,shm,POSIX shared memory source and sink,,,+librt))
$(eval $(call GstBuildPlugin,sndfile,sndfile support,audio,,+libsndfile))
#$(eval $(call GstBuildPlugin,srtp,srtp support,rtp,,+libsrtp))
$(eval $(call GstBuildPlugin,webp,webp support,,,+libwebp))
#$(eval $(call GstBuildPlugin,yadif,yadif support,,,))

$(eval $(call BuildPackage,gstreamer1-plugins-bad))
$(eval $(call BuildPackage,gst1-plugins-bad))
