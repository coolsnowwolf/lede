define KernelPackage/phy-sf19a2890-usb
  TITLE:=Siflower SF19A2890 USB 2.0 PHY Driver
  KCONFIG:=CONFIG_PHY_SF19A2890_USB
  DEPENDS:=@TARGET_siflower_sf19a2890
  SUBMENU:=$(USB_MENU)
  FILES:=$(LINUX_DIR)/drivers/phy/siflower/phy-sf19a2890-usb.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-sf19a2890-usb,1)
endef

define KernelPackage/phy-sf19a2890-usb/description
  Support for Siflower SF19A2890 USB 2.0 PHY connected to the USB
  controller.
endef

$(eval $(call KernelPackage,phy-sf19a2890-usb))

define KernelPackage/phy-sf21-usb
  TITLE:=Siflower SF21 USB 2.0 PHY Driver
  KCONFIG:=CONFIG_PHY_SF21_USB
  DEPENDS:=@TARGET_siflower_sf21
  SUBMENU:=$(USB_MENU)
  FILES:=$(LINUX_DIR)/drivers/phy/siflower/phy-sf21-usb.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-sf21-usb,1)
endef

define KernelPackage/phy-sf21-usb/description
  Support for Siflower SF21 USB 2.0 PHY connected to the USB
  controller.
endef

$(eval $(call KernelPackage,phy-sf21-usb))
