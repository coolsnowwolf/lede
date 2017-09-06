define KernelPackage/mediatek_hnat
  SUBMENU:=Network Devices
  TITLE:=MT7623 HNAT
  DEPENDS:=@TARGET_mediatek +kmod-nf-conntrack
  KCONFIG:= CONFIG_NET_MEDIATEK_HNAT=y
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/mediatek/mtk_hnat/mtkhnat.ko
endef

define KernelPackage/mediatek_hnat/description
  Kernel modules for MediaTek HW NAT offloading
endef

$(eval $(call KernelPackage,mediatek_hnat))
