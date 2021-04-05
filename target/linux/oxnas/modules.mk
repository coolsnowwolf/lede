define KernelPackage/ata-oxnas-sata
  TITLE:=oxnas Serial ATA support
  KCONFIG:=CONFIG_SATA_OXNAS
  FILES:=$(LINUX_DIR)/drivers/ata/sata_oxnas.ko
  AUTOLOAD:=$(call AutoLoad,41,sata_oxnas,1)
  $(call AddDepends/ata,@TARGET_oxnas)
endef

define KernelPackage/ata-oxnas-sata/description
 SATA support for OX934 core found in the OX8xx/PLX782x SoCs
endef

$(eval $(call KernelPackage,ata-oxnas-sata))


define KernelPackage/usb2-oxnas
  TITLE:=OX820 EHCI driver
  KCONFIG:=CONFIG_USB_EHCI_OXNAS
  FILES:=$(LINUX_DIR)/drivers/usb/host/ehci-oxnas.ko
  AUTOLOAD:=$(call AutoLoad,55,ehci-oxnas,1)
  $(call AddDepends/usb,@TARGET_oxnas_ox820 +kmod-usb2)
endef

define KernelPackage/usb2-oxnas/description
 This driver provides USB Device Controller support for the
 EHCI USB host built-in to the OX820 SoC.
endef

$(eval $(call KernelPackage,usb2-oxnas))
