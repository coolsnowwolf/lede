include $(TOPDIR)/rules.mk

PKG_NAME:=zstd
PKG_VERSION:=1.5.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/facebook/zstd/releases/download/v$(PKG_VERSION)
PKG_HASH:=7c42d56fac126929a6a85dbc73ff1db2411d04f104fae9bdea51305663a83fd0

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:facebook:zstandard

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/meson.mk

MESON_HOST_BUILD_DIR:=$(HOST_BUILD_DIR)/build/meson/openwrt-build

HOSTCC:= $(HOSTCC_NOCACHE)
HOST_LDFLAGS += -Wl,-rpath,$(STAGING_DIR_HOST)/lib

MESON_HOST_ARGS += \
	-Dlegacy_level=7 \
	-Ddebug_level=0 \
	-Dbacktrace=false \
	-Dstatic_runtime=false \
	-Dbin_programs=true \
	-Dbin_tests=false \
	-Dbin_contrib=false \
	-Dmulti_thread=enabled \
	-Dzlib=disabled \
	-Dlzma=disabled \
	-Dlz4=disabled

$(eval $(call HostBuild))
