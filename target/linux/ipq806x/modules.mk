define KernelPackage/phy-qcom-ipq806x-usb
  TITLE:=Qualcomm IPQ806x DWC3 USB PHY driver
  DEPENDS:=@TARGET_ipq806x
  KCONFIG:= CONFIG_PHY_QCOM_IPQ806X_USB
  FILES:= \
    $(LINUX_DIR)/drivers/phy/qualcomm/phy-qcom-ipq806x-usb.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qcom-ipq806x-usb,1)
  $(call AddDepends/usb)
endef

define KernelPackage/phy-qcom-ipq806x-usb/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the QCOM SoCs.
endef

$(eval $(call KernelPackage,phy-qcom-ipq806x-usb))
