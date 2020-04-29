include $(TOPDIR)/rules.mk
PKG_NAME:=rclone
PKG_VERSION:=1.51.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/rclone-v$(PKG_VERSION)
PKG_SOURCE:=rclone-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/rclone/rclone/releases/download/v$(PKG_VERSION)
PKG_HASH:=3eb5b7ffce17e56fadb29bf854666723a14c93fedc02046c7f34c792dbd227ee
PKG_MAINTAINER:=xinxijishuwyq@gmail.com
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

GOARCH:=$(ARCH)

ifeq ($(GOARCH),i386)
  GOARCH:=386
  ifeq ($(CONFIG_CPU_TYPE),"pentium4")
    GOSUBARCH:=GO386=sse2
  else
    GOSUBARCH:=GO386=387
  endif
endif
ifeq ($(GOARCH),x86_64)
  GOARCH:=amd64
endif
ifeq ($(GOARCH),aarch64)
  GOARCH:=arm64
endif
ifeq ($(GOARCH),arm)
  ifeq ($(CONFIG_arm_v5),y)
    GOSUBARCH:=GOARM=5
  endif
  ifeq ($(CONFIG_arm_v6),y)
    GOSUBARCH:=GOARM=6
  endif
  ifeq ($(CONFIG_arm_v7),y)
    GOSUBARCH:=GOARM=7
  endif
endif



define Package/rclone
  TITLE:=rclone
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=github
  DEPENDS:=
endef

GOENV:=GOPATH=$(PKG_BUILD_DIR)/.. GOARCH=$(GOARCH) $(GOSUBARCH) CGO_ENABLED=0 CC=$(TARGET_CC)
GOFLAGS:=--ldflags='-s -w'

define Build/Compile
	cd $(PKG_BUILD_DIR);  $(GOENV) go build $(GOFLAGS)
endef

define Package/rclone/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rclone $(1)/usr/bin/rclone
endef

define Package/rclone/conffiles
  /root/.config/rclone/*
endef

$(eval $(call BuildPackage,rclone))

