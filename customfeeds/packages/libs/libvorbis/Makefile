#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libvorbis
PKG_VERSION:=1.3.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://downloads.xiph.org/releases/vorbis/
PKG_HASH:=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:xiph.org:libvorbis

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libvorbis
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libvorbis
  URL:=http://xiph.org/vorbis/
  DEPENDS:=+libogg
endef

define Package/libvorbis/description
Vorbis is a general purpose audio and music encoding format
contemporary to MPEG-4's AAC and TwinVQ, the next generation beyond
MPEG audio layer 3. Unlike the MPEG sponsored formats (and other
proprietary formats such as RealAudio G2 and Windows' flavor of the
month), the Vorbis CODEC specification belongs to the public domain.
All the technical details are published and documented, and any
software entity may make full use of the format without license
fee, royalty or patent concerns.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON

ifneq ($(findstring arm,$(CONFIG_ARCH)),)
	TARGET_CFLAGS += -flto
	TARGET_CXX_FLAGS += -flto
	TARGET_LDFLAGS += -flto
endif

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/vorbisenc.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/vorbisenc.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/vorbisfile.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/vorbisfile.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/vorbis.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/vorbis.pc
endef

define Package/libvorbis/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libvorbis))
