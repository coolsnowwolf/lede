define KernelPackage/usb-fotg210
  TITLE:=Support for FOTG210 USB host and device controllers
  DEPENDS:=@TARGET_gemini
  KCONFIG:= \
  CONFIG_USB_FOTG210 \
  CONFIG_USB_FOTG210_HCD=y \
  CONFIG_USB_FOTG210_UDC=y
  FILES:=$(LINUX_DIR)/drivers/usb/fotg210/fotg210.ko
  AUTOLOAD:=$(call AutoLoad,50,fotg210,1)
  $(call AddDepends/usb)
endef

$(eval $(call KernelPackage,usb-fotg210))
