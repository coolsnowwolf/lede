#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mjpg-streamer
PKG_VERSION:=1.0.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/jacksonliam/mjpg-streamer/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=31850cd533b0290640cbdf4da44f7a774bfba050647cb0a0c84a435e90b08598

PKG_MAINTAINER:=Roger D <rogerdammit@gmail.com>, \
		Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip=2 -xf $(DL_DIR)/$(PKG_SOURCE)
PKG_BUILD_DEPENDS:=MJPG_STREAMER_V4L2:libv4l zmq protobuf-c/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mjpg-streamer
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=MJPG-streamer
  DEPENDS:=+libpthread +libjpeg +MJPG_STREAMER_V4L2:libv4l
  URL:=https://github.com/jacksonliam/mjpg-streamer
endef

define Package/mjpg-streamer/Default/description
 Streaming application for Linux-UVC compatible webcams
endef

define Package/mjpg-streamer/description
$(call Package/mjpg-streamer/Default/description)
endef

define Package/mjpg-streamer/conffiles
/etc/config/mjpg-streamer
endef

define Package/mjpg-streamer/config
if PACKAGE_mjpg-streamer-input-uvc

config MJPG_STREAMER_V4L2
	bool "Build input_uvc with libv4l2 (camera controls)"
	default n
	select PACKAGE_libv4l
endif
endef

define Package/mjpg-streamer/Default
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=MJPG-streamer
  URL:=https://github.com/jacksonliam/mjpg-streamer
  DEPENDS:=mjpg-streamer
endef

define Package/mjpg-streamer-input-file
$(call Package/mjpg-streamer/Default)
  TITLE+= (file input)
endef

define Package/mjpg-streamer-input-file/description
$(call Package/mjpg-streamer/Default/description)

This package provides the file input plugin.
endef

define Package/mjpg-streamer-input-uvc
$(call Package/mjpg-streamer/Default)
  TITLE+= (UVC input)
endef

define Package/mjpg-streamer-input-uvc/description
$(call Package/mjpg-streamer/Default/description)

This package provides the UVC input plugin.
endef

define Package/mjpg-streamer-input-http
$(call Package/mjpg-streamer/Default)
  TITLE+= (HTTP input)
endef

define Package/mjpg-streamer-input-http/description
$(call Package/mjpg-streamer/Default/description)

This package provides the http input plugin.
endef

define Package/mjpg-streamer-output-rtsp
$(call Package/mjpg-streamer/Default)
  TITLE+= (RTSP output)
endef

define Package/mjpg-streamer-output-rtsp/description
$(call Package/mjpg-streamer/Default/description)

This package provides the RTSP output plugin.
endef

define Package/mjpg-streamer-output-udp
$(call Package/mjpg-streamer/Default)
  TITLE+= (UDP output)
endef

define Package/mjpg-streamer-output-udp/description
$(call Package/mjpg-streamer/Default/description)

This package provides the UDP output plugin.
endef

define Package/mjpg-streamer-output-file
$(call Package/mjpg-streamer/Default)
  TITLE+= (file output)
endef

define Package/mjpg-streamer-output-file/description
$(call Package/mjpg-streamer/Default/description)

This package provides the file output plugin.
endef

define Package/mjpg-streamer-output-http
$(call Package/mjpg-streamer/Default)
  TITLE+= (HTTP output)
endef

define Package/mjpg-streamer-output-http/description
$(call Package/mjpg-streamer/Default/description)

This package provides the HTTP output plugin.
endef

define Package/mjpg-streamer-output-zmq
$(call Package/mjpg-streamer/Default)
  TITLE+= (zmq output)
  DEPENDS+= +libzmq +libprotobuf-c
endef

define Package/mjpg-streamer-output-zmq/description
$(call Package/mjpg-streamer/Default/description)

This package provides the ZeroMQ output plugin.
endef

define Package/mjpg-streamer-www
$(call Package/mjpg-streamer/Default)
  TITLE+= (full www)
  CONFLICTS:=mjpg-streamer-www-simple
endef

define Package/mjpg-streamer-www/description
$(call Package/mjpg-streamer/Default/description)

This package provides full version of the web content.
Includes cambozola applet.
endef

define Package/mjpg-streamer-www-simple
$(call Package/mjpg-streamer/Default)
  TITLE+= (simple www)
endef

define Package/mjpg-streamer-www-simple/description
$(call Package/mjpg-streamer/Default/description)

This package provides simple version of the web content.
endef

CMAKE_OPTIONS += -DCMAKE_SKIP_RPATH=FALSE

CAMBOZOLA:=cambozola-0.936.tar.gz

# Distribution URL doesn't always have the correct version
# Using the OpenWrt mirror provides a stable version
#      orig-URL:=http://www.andywilcock.com/code/cambozola
define Download/cambozola
  URL:=https://sources.openwrt.org/
  FILE:=$(CAMBOZOLA)
  HASH:=4e4e92d95673edd051e6b4d36bed8c5328873f5fc374073760b457d4be33415b
endef

# redefine prepare to extract to our build dir
# apply patches
define Build/Prepare
    $(Build/Prepare/Default)
    # Fetch latest cambozola that works with latest Java(s)
    # Yes, I know this is ugly
    ifneq ($(CONFIG_PACKAGE_mjpg-streamer-www),)
	$(eval $(call Download,cambozola))
	$(TAR) -xf $(DL_DIR)/$(CAMBOZOLA) --strip=2 --wildcards \
		-C $(PKG_BUILD_DIR)/www */dist/cambozola.jar
    endif
endef

define Build/Configure
    $(RM) $(PKG_BUILD_DIR)/plugins/input_uvc/uvcvideo.h
$(call Build/Configure/Default)
endef

TARGET_LDFLAGS+= -ljpeg

ifeq ($(CONFIG_MJPG_STREAMER_V4L2),y)
    TARGET_CFLAGS+= -DUSE_LIBV4L2
    TARGET_LDFLAGS+= -lv4l2
endif

define Package/mjpg-streamer/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mjpg_streamer $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/mjpg-streamer.config $(1)/etc/config/mjpg-streamer
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/mjpg-streamer.init $(1)/etc/init.d/mjpg-streamer
	$(INSTALL_DIR) $(1)/etc/hotplug.d/usb
	$(INSTALL_DATA) ./files/mjpg-streamer.hotplug $(1)/etc/hotplug.d/usb/20-mjpg-streamer
endef

define Package/mjpg-streamer-input-file/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/input_file.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-input-uvc/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/input_uvc.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-input-http/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/input_http.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-output-rtsp/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/output_rtsp.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-output-udp/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/output_udp.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-output-file/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/output_file.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-output-http/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/output_http.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-output-zmq/install
	$(INSTALL_DIR) $(1)/usr/lib/mjpg-streamer
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mjpg-streamer/output_zmqserver.so $(1)/usr/lib/mjpg-streamer
endef

define Package/mjpg-streamer-www/install
	$(INSTALL_DIR) $(1)/www/webcam
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/mjpg-streamer/www/* $(1)/www/webcam
endef

define Package/mjpg-streamer-www-simple/install
	$(INSTALL_DIR) $(1)/www/webcam
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/mjpg-streamer/www/stream_simple.html $(1)/www/webcam/index.html
endef

$(eval $(call BuildPackage,mjpg-streamer))
$(eval $(call BuildPackage,mjpg-streamer-input-file))
$(eval $(call BuildPackage,mjpg-streamer-input-uvc))
$(eval $(call BuildPackage,mjpg-streamer-input-http))
$(eval $(call BuildPackage,mjpg-streamer-output-rtsp))
$(eval $(call BuildPackage,mjpg-streamer-output-file))
$(eval $(call BuildPackage,mjpg-streamer-output-http))
$(eval $(call BuildPackage,mjpg-streamer-output-zmq))
$(eval $(call BuildPackage,mjpg-streamer-www))
$(eval $(call BuildPackage,mjpg-streamer-www-simple))
