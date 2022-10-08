include $(TOPDIR)/rules.mk

PKG_NAME:=zstd
PKG_VERSION:=1.5.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/facebook/zstd/releases/download/v$(PKG_VERSION)
PKG_HASH:=7c42d56fac126929a6a85dbc73ff1db2411d04f104fae9bdea51305663a83fd0

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:facebook:zstandard

CMAKE_SOURCE_SUBDIR:=build/cmake

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_HOST_OPTIONS += \
	-DBUILD_TESTING=OFF \
	-DCMAKE_C_COMPILER_LAUNCHER="" \
	-DCMAKE_C_COMPILER=$(HOSTCC_NOCACHE) \
	-DZSTD_LEGACY_SUPPORT=OFF

$(eval $(call HostBuild))
