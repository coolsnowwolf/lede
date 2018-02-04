include $(TOPDIR)/rules.mk

PKG_NAME:=umbim
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/umbim.git
PKG_SOURCE_DATE:=2015-07-04
PKG_SOURCE_VERSION:=29aaf43b097ee57f7aa1bb24341db6cc4148cbf3
PKG_MIRROR_HASH:=56e612d9da1ac26de96c4929fbabd047862d725a13ceb904807a9d3d7db43a30
PKG_MAINTAINER:=John Crispin <john@phrozen.org>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/umbim
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libubox +kmod-usb-net +kmod-usb-net-cdc-mbim +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/umbim/description
  umbim is a command line tool for controlling mobile broadband modems using
  the MBIM-protocol.
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include -ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections

define Package/umbim/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/umbim $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,umbim))
