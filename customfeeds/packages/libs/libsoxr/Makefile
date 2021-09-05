#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# updated to work with latest source from abrasive
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsoxr
PKG_VERSION:=0.1.3
PKG_RELEASE:=2

PKG_SOURCE_URL:=@SF/project/soxr/
PKG_SOURCE:=soxr-$(PKG_VERSION)-Source.tar.xz
PKG_HASH:=b111c15fdc8c029989330ff559184198c161100a59312f5dc19ddeb9b5a15889
PKG_BUILD_DIR:=$(BUILD_DIR)/soxr-$(PKG_VERSION)-Source

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>, \
		Mike Brady <mikebrady@eircom.net>
PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=LICENCE
PKG_CPE_ID:=cpe:/a:sox:sox

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libsoxr
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=The SoX Resampler library
  URL:=https://sourceforge.net/projects/soxr/
  DEPENDS:= +libpthread
endef

define Package/libsoxr/description
  The SoX Resampler library
  High quality, one-dimensional sample-rate conversion library
endef

CMAKE_OPTIONS += \
	-DBUILD_TESTS=0 \
	-DBUILD_EXAMPLES=0 \
	-DWITH_OPENMP=0 \
	-DHAVE_WORDS_BIGENDIAN_EXITCODE=$(if $(CONFIG_BIG_ENDIAN),0,1)

define Package/libsoxr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsoxr.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsoxr))
