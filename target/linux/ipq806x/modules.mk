define KernelPackage/usb-phy-qcom-ipq4019
  TITLE:=IPQ4019 PHY wrappers support
  DEPENDS:=+kmod-usb-dwc3
  KCONFIG:= CONFIG_USB_IPQ4019_PHY
  FILES:= $(LINUX_DIR)/drivers/usb/phy/phy-qca-baldur.ko $(LINUX_DIR)/drivers/usb/phy/phy-qca-uniphy.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qca-uniphy phy-qca-baldur,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-phy-qcom-ipq4019/description
  This driver provides support for the USB PHY transceivers on QCA961x chips.
endef

$(eval $(call KernelPackage,usb-phy-qcom-ipq4019))
