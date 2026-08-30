include $(TOPDIR)/rules.mk

PKG_NAME:=urngd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/urngd.git
PKG_SOURCE_DATE:=2025-10-03
PKG_SOURCE_VERSION:=f17e33d9942750edbd73124e6f03118a98abb402
PKG_MIRROR_HASH:=7ca9e2e796b28df27b9b1ecd35987fe6e0ae2425e4ec92dff420386572dbbdc9

PKG_LICENSE:=GPL-2.0 BSD-3-Clause
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/urngd
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=OpenWrt non-physical true random number generator based on timing jitter
  DEPENDS:=+libubox
endef

define Package/urngd/description
  urngd is OpenWrt's micro non-physical true random number generator based on
  timing jitter.

  Using the Jitter RNG core, the rngd provides an entropy source that feeds into
  the Linux /dev/random device if its entropy runs low. It updates the
  /dev/random entropy estimator such that the newly provided entropy unblocks
  /dev/random.

  The seeding of /dev/random also ensures that /dev/urandom benefits from
  entropy. Especially during boot time, when the entropy of Linux is low, the
  Jitter RNGd provides a source of sufficient entropy.
endef

define Package/urngd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/sbin/$(PKG_NAME)
endef

$(eval $(call BuildPackage,urngd))
