include $(TOPDIR)/rules.mk

PKG_NAME:=uqmi
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(LEDE_GIT)/project/uqmi.git
PKG_SOURCE_DATE:=2016-12-19
PKG_SOURCE_VERSION:=8ceeab690d8c6f1e3afbd4bcaee7bc2ba3fbe165
PKG_MIRROR_HASH:=b3637ff04e51769137af1c5e173e73311e11c3c2dcc49eeaca6aa3520f61d247
PKG_MAINTAINER:=Matti Laakso <malaakso@elisanet.fi>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

PKG_FLAGS:=nonshared

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
	-I$(STAGING_DIR)/usr/include -ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections

CMAKE_OPTIONS += \
	-DDEBUG=1

define Package/uqmi/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmi $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,uqmi))
