include $(TOPDIR)/rules.mk

LUCI_TITLE:=ZTE NPU SQM for SR1010
LUCI_DESCRIPTION:=Pure hardware WAN shaping and queue strategies for SR1010
LUCI_DEPENDS:=@TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010 +luci-base +sqm-npu-policy
PKG_VERSION:=1.0.0
PKG_RELEASE:=8

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
