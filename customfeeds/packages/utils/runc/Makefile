include $(TOPDIR)/rules.mk

PKG_NAME:=runc
PKG_VERSION:=1.0.1
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/opencontainers/runc/tar.gz/v${PKG_VERSION}?
PKG_HASH:=b25e4273a895af3239bc5e495a007266356038adfb34c4b94b4fc39627a89ad9
PKG_SOURCE_VERSION:=4144b63817ebcc5b358fc2c8ef95f7cddd709aa7

PKG_MAINTAINER:=Gerard Ryan <G.M0N3Y.2503@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/opencontainers/runc

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/runc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=runc container runtime
  URL:=https://www.opencontainers.org/
  DEPENDS:=$(GO_ARCH_DEPENDS) +KERNEL_SECCOMP_FILTER:libseccomp
endef

define Package/runc/description
runc is a CLI tool for spawning and running containers according to the OCI specification.
endef

GO_PKG_INSTALL_ALL:=1
MAKE_PATH:=$(GO_PKG_WORK_DIR_NAME)/build/src/$(GO_PKG)
MAKE_VARS += $(GO_PKG_VARS)
MAKE_FLAGS += COMMIT=$(PKG_SOURCE_VERSION)

ifeq ($(ARCH),mips)
MAKE_FLAGS += EXTRA_FLAGS='-buildmode=default'
endif

BUILDTAGS:=
ifeq ($(CONFIG_KERNEL_SECCOMP_FILTER),y)
BUILDTAGS += seccomp
endif
ifeq ($(CONFIG_SELINUX),y)
BUILDTAGS += selinux
endif
MAKE_FLAGS += BUILDTAGS='$(BUILDTAGS)'

# Reset golang-package.mk overrides so we can use the Makefile
Build/Compile=$(call Build/Compile/Default)

define Package/runc/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_DIR)/src/$(GO_PKG)/runc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,runc))
