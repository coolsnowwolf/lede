include $(TOPDIR)/rules.mk

PKG_NAME:=restic
PKG_VERSION:=0.9.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/restic/restic/tar.gz/v${PKG_VERSION}?
PKG_HASH:=1cc8655fa99f06e787871a9f8b5ceec283c856fa341a5b38824a0ca89420b0fe

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Markus Weippert <markus@gekmihesg.de>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/restic/restic/
GO_PKG_BUILD_PKG:=github.com/restic/restic/cmd/restic/
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.version=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/restic
  TITLE:=restic backup program
  URL:=http://github.com/restic/restic
  DEPENDS:=$(GO_ARCH_DEPENDS)
  SECTION:=utils
  CATEGORY:=Utilities
endef

define Package/restic/description
restic is a backup program that is fast, efficient and secure. It supports the
three major operating systems (Linux, macOS, Windows) and a few smaller ones
(FreeBSD, OpenBSD).
endef

$(eval $(call GoBinPackage,restic))
$(eval $(call BuildPackage,restic))
