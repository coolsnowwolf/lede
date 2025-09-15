include $(TOPDIR)/rules.mk

PKG_NAME:=uqmi
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/uqmi.git
PKG_SOURCE_DATE:=2024-08-25
PKG_SOURCE_VERSION:=28b48a10dbcd1177095b73c6d8086d10114f49b8
PKG_MIRROR_HASH:=bf1f8112c94ce49a7c03a4b9593141129d63ce679f98594257c1f6ab9bc49766
PKG_MAINTAINER:=Matti Laakso <malaakso@elisanet.fi>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_FLAGS:=nonshared
PKG_BUILD_FLAGS:=gc-sections

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/uqmi
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +libblobmsg-json +kmod-usb-net +kmod-usb-net-qmi-wwan +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/uqmi/description
  uqmi is a command line tool for controlling mobile broadband modems using
  the QMI-protocol.
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-Wno-error=maybe-uninitialized

CMAKE_OPTIONS += \
	-DDEBUG=1

define Package/uqmi/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmi/uqmi $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,uqmi))
