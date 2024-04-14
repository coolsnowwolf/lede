include $(TOPDIR)/rules.mk

PKG_NAME:=zstd
PKG_VERSION:=1.5.5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/facebook/zstd/releases/download/v$(PKG_VERSION)
PKG_HASH:=9c4396cc829cfae319a6e2615202e82aad41372073482fce286fac78646d3ee4

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:facebook:zstandard

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/meson.mk

MESON_HOST_BUILD_DIR:=$(HOST_BUILD_DIR)/build/meson/openwrt-build

HOSTCC:= $(HOSTCC_NOCACHE)

MESON_HOST_ARGS += \
	-Ddefault_library=static \
	-Dlegacy_level=7 \
	-Ddebug_level=0 \
	-Dbacktrace=disabled \
	-Dstatic_runtime=false \
	-Dbin_programs=true \
	-Dbin_tests=false \
	-Dbin_contrib=false \
	-Dmulti_thread=enabled \
	-Dzlib=disabled \
	-Dlzma=disabled \
	-Dlz4=disabled

$(eval $(call HostBuild))
