# SPDX-License-Identifier: GPL-2.0-only
include $(TOPDIR)/rules.mk

LUCI_TITLE:=ZTE Mesh controller for the wired LAN
LUCI_DEPENDS:=@TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010 +luci-base +zte-mesh
LUCI_PKGARCH:=all

PKG_VERSION:=0.1.0
PKG_RELEASE:=1
PKG_PO_VERSION:=$(PKG_VERSION)-r$(PKG_RELEASE)
PKG_LICENSE:=GPL-2.0-only

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
