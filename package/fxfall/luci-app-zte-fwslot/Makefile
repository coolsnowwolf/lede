# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI page to switch the firmware slot of the ZTE SR1010
LUCI_DEPENDS:=@TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010 +luci-base +zte-fwslot
LUCI_PKGARCH:=all

PKG_LICENSE:=GPL-2.0-only

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
