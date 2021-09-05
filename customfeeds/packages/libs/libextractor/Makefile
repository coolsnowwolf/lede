#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libextractor
PKG_VERSION:=1.11
PKG_RELEASE:=$(AUTORELEASE)

# ToDo:
# - package missing optional dependencies: libexiv2, gsf, librpm, smf, tidy

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=16f633ab8746a38547c4a1da3f4591192b0825ad83c4336f0575b85843d8bd8f

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_libextractor-plugin-thumbnailffmpeg \
	CONFIG_PACKAGE_libextractor-plugin-gstreamer

PLUGINS:= \
	archive:+libarchive-noopenssl \
	deb \
	dvi \
	flac:+libflac \
	gif:+giflib \
	gstreamer:+libgstreamer1:+gstreamer1-plugins-base:+libgst1app:+libgst1pbutils:+libgst1tag \
	it \
	jpeg:+libjpeg-turbo \
	man \
	mime:+libmagic \
	mpeg:+libmpeg2 \
	nsf \
	nsfe \
	odf \
	ogg:+libvorbis \
	png \
	ps \
	riff \
	s3m \
	sid \
	thumbnailffmpeg:+libffmpeg-full:+libmagic:@BUILD_PATENTED \
	tiff:+libtiff \
	wav \
	xm \
	zip


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

CONFIGURE_ARGS += \
	--$(if $(CONFIG_PACKAGE_libextractor-plugin-thumbnailffmpeg),en,dis)able-ffmpeg \
	--disable-glibtest \
	--disable-gsf \
	--disable-rpath \
	--with$(if $(CONFIG_PACKAGE_libextractor-plugin-gstreamer),,out)-gstreamer

CONFIGURE_VARS += \
	ac_cv_lib_jpeg_jpeg_mem_src=yes

TARGET_LDFLAGS += \
	-liconv

define Package/libextractor
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=GNU Libextractor
	URL:=https://www.gnu.org/software/libextractor/
	DEPENDS:=+libbz2 +libltdl +librt +zlib $(ICONV_DEPENDS) $(INTL_DEPENDS)
	MENU:=1
endef

define Package/libextractor/description
  GNU Libextractor is a library used to extract meta data from files.
  The goal is to provide developers of file-sharing networks, browsers or
  WWW-indexing bots with a universal library to obtain simple keywords and meta
  data to match against queries and to show to users instead of only relying on
  filenames.
endef

define PluginGen
define Package/libextractor-plugin-$(subst _,-,$(firstword $(subst :, ,$(1))))
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=GNU Libextractor ($(firstword $(subst :, ,$(1))) plugin)
	URL:=https://www.gnu.org/software/libextractor/
	DEPENDS:=libextractor $(wordlist 2,$(words $(subst :, ,$(1))),$(subst :, ,$(1)))
endef
endef

$(foreach file,$(PLUGINS),$(eval $(call PluginGen,$(file))))

define Package/extract
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=extract util from GNU Libextractor
	URL:=https://www.gnu.org/software/libextractor/
	DEPENDS:=+libextractor
endef

define Package/extract/description
  libextractor contains the shell command extract that, similar to the
  well-known file command, can extract meta data from a file an print the results
  to stdout.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
endef

define Package/libextractor/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define PluginInstall
define Package/libextractor-plugin-$(subst _,-,$(firstword $(subst :, ,$(1))))/install
	$(INSTALL_DIR) $$(1)/usr/lib/libextractor
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/libextractor/libextractor_$(firstword $(subst :, ,$(1))).so \
		$$(1)/usr/lib/libextractor
endef
endef

define Package/extract/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(foreach file,$(PLUGINS),$(eval $(call PluginInstall,$(file))))

$(eval $(call BuildPackage,libextractor))
$(foreach file,$(PLUGINS),$(eval $(call BuildPackage,libextractor-plugin-$(subst _,-,$(firstword $(subst :, ,$(file)))))))
$(eval $(call BuildPackage,extract))
