# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl826x-firmware
PKG_SOURCE_DATE:=2026-01-24
PKG_RELEASE:=4

PKG_SOURCE_URL:=https://github.com/balika011/realtek_phy_firmware
PKG_SOURCE_VERSION:=0cd4abe2b0bf197f75c27088f86a74c7ddb103b4
PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=f665e9f119cbfddcccae09a17382daa8306772d1e37db7eb69a8771157d06d5f

PKG_MAINTAINER:=Balázs Triszka <info@balika011.hu>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Build/Compile
	(set -e; cd $(PKG_BUILD_DIR); \
		$(HOSTCC) rtl8261n_rtl8264b.c phy_patch.c -o phy_patch; \
		./phy_patch \
	)
endef

define Package/rtl826x-firmware/Default
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=+kmod-phy-realtek
  PKGARCH:=all
endef

define Package/rtl826x-firmware
  $(call Package/rtl826x-firmware/Default)
  TITLE:=Realtek RTL8251L/RTL8261BE/RTL8261N/RTL8254B/RTL8264/RTL8264B firmware
endef

define Package/rtl826x-lp-firmware
  $(call Package/rtl826x-firmware/Default)
  TITLE:=Realtek RTL8251L/RTL8261BE/RTL8261N/RTL8254B/RTL8264/RTL8264B low-power firmware
  PROVIDES:=rtl826x-firmware
  CONFLICTS:=rtl826x-firmware
endef

define Package/rtl826x-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/rtl8261n.bin \
		$(PKG_BUILD_DIR)/rtl8264b.bin \
		$(1)/lib/firmware/
endef

define Package/rtl826x-lp-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/rtl8261n_lp.bin \
		$(1)/lib/firmware/rtl8261n.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/rtl8264b.bin \
		$(1)/lib/firmware/
endef

$(eval $(call BuildPackage,rtl826x-firmware))
$(eval $(call BuildPackage,rtl826x-lp-firmware))
