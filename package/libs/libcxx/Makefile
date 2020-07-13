
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcxx
PKG_VERSION:=9.0.1
PKG_RELEASE:=4

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.llvm.org/git/libcxx
PKG_SOURCE_VERSION:=2076f539f410805ef88692b9c0ce0a0b882a7680
PKG_MIRROR_HASH:=d527880a18dec9109575c76717cf5288fb91c11381b9d261cae2e5bebcbdab2e

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

PKG_BUILD_PARALLEL:=1
CMAKE_BINARY_SUBDIR:=build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libcxx
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=LLVM libstdc++
  URL:=https://libcxx.llvm.org/
  DEPENDS:=+libatomic +libpthread
endef

define Package/libcxx/description
  LLVM C++ library targeting C++11 and above
endef

CMAKE_OPTIONS += \
	-DLIBCXX_CXX_ABI="libsupc++" \
	-DLIBCXX_CXX_ABI_INCLUDE_PATHS="$(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)/include/c++/$(CONFIG_GCC_VERSION);$(TOOLCHAIN_DIR)/$(REAL_GNU_TARGET_NAME)/include/c++/$(CONFIG_GCC_VERSION)/$(REAL_GNU_TARGET_NAME)" \
	-DLIBCXX_ENABLE_ASSERTIONS=OFF \
	-DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF \
	-DLIBCXX_INCLUDE_BENCHMARKS=OFF \
	-DLIBCXX_INCLUDE_DOCS=OFF \
	-DLIBCXX_INCLUDE_TESTS=OFF \
	-DLIBCXX_LIBDIR_SUFFIX="" \
	-DLIBCXX_STANDALONE_BUILD=ON \
	-DLIBCXX_HAS_MUSL_LIBC=$(if $(CONFIG_USE_MUSL),ON,OFF)

TARGET_CXXFLAGS += -D_LIBCPP_DISABLE_DEPRECATION_WARNINGS -Wno-attributes -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(CP) files/g++-libcxx	$(TOOLCHAIN_DIR)/bin/
	$(SED) 's,CXX,$(TARGET_CXX),g' $(TOOLCHAIN_DIR)/bin/g++-libcxx
endef

define Package/libcxx/install
	$(INSTALL_DIR)					$(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libc++.so.*	$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcxx))
