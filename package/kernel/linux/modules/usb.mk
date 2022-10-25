#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

USB_MENU:=USB Support

USBNET_DIR:=net/usb
USBHID_DIR?=hid/usbhid
USBINPUT_DIR?=input/misc

define KernelPackage/usb-core
  SUBMENU:=$(USB_MENU)
  TITLE:=Support for USB
  DEPENDS:=@USB_SUPPORT
  KCONFIG:=CONFIG_USB CONFIG_XPS_USB_HCD_XILINX=n CONFIG_USB_FHCI_HCD=n
  FILES:= \
	$(LINUX_DIR)/drivers/usb/core/usbcore.ko \
	$(LINUX_DIR)/drivers/usb/common/usb-common.ko
  AUTOLOAD:=$(call AutoLoad,20,usb-common usbcore,1)
  $(call AddDepends/nls)
endef

define KernelPackage/usb-core/description
 Kernel support for USB
endef

$(eval $(call KernelPackage,usb-core))


define AddDepends/usb
  SUBMENU:=$(USB_MENU)
  DEPENDS+=+kmod-usb-core $(1)
endef


define KernelPackage/usb-ledtrig-usbport
  TITLE:=LED trigger for USB ports
  KCONFIG:=CONFIG_USB_LEDS_TRIGGER_USBPORT
  FILES:=$(LINUX_DIR)/drivers/usb/core/ledtrig-usbport.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-usbport)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-ledtrig-usbport/description
  This driver allows LEDs to be controlled by USB events. Enabling this
  trigger allows specifying list of USB ports that should turn on LED
  when some USB device gets connected.
  If possible it should be prefered over similar ledtrig-usbdev.
endef

$(eval $(call KernelPackage,usb-ledtrig-usbport))


define KernelPackage/usb-phy-nop
  TITLE:=Support for USB NOP transceiver
  KCONFIG:=CONFIG_NOP_USB_XCEIV
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/usb/phy/phy-generic.ko
  AUTOLOAD:=$(call AutoLoad,21,phy-generic,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-phy-nop/description
  Support for USB NOP transceiver
endef

$(eval $(call KernelPackage,usb-phy-nop))


define KernelPackage/phy-ath79-usb
  TITLE:=Support for ATH79 USB PHY
  KCONFIG:=CONFIG_PHY_AR7100_USB \
	CONFIG_PHY_AR7200_USB
  DEPENDS:=@TARGET_ath79
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/phy/phy-ar7100-usb.ko \
	$(LINUX_DIR)/drivers/phy/phy-ar7200-usb.ko
  AUTOLOAD:=$(call AutoLoad,21,phy-ar7100-usb phy-ar7200-usb,1)
  $(call AddDepends/usb)
endef

define KernelPackage/phy-ath79-usb/description
  Support for ATH79 USB transceiver
endef

$(eval $(call KernelPackage,phy-ath79-usb))


define KernelPackage/usb-gadget
  TITLE:=USB Gadget support
  KCONFIG:=CONFIG_USB_GADGET
  HIDDEN:=1
  FILES:=\
	$(LINUX_DIR)/drivers/usb/gadget/udc/udc-core.ko
  AUTOLOAD:=$(call AutoLoad,21,udc-core,1)
  DEPENDS:=@USB_GADGET_SUPPORT
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget/description
 Kernel support for USB Gadget mode
endef

$(eval $(call KernelPackage,usb-gadget))

define KernelPackage/usb-lib-composite
  TITLE:=USB lib composite
  KCONFIG:=CONFIG_USB_LIBCOMPOSITE
  DEPENDS:=+kmod-usb-gadget +kmod-fs-configfs
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/libcomposite.ko
  AUTOLOAD:=$(call AutoLoad,50,libcomposite)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-lib-composite/description
 Lib Composite
endef

$(eval $(call KernelPackage,usb-lib-composite))

define KernelPackage/usb-gadget-hid
  TITLE:=USB HID Gadget Support
  KCONFIG:=CONFIG_USB_G_HID
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite
  FILES:= \
	  $(LINUX_DIR)/drivers/usb/gadget/legacy/g_hid.ko \
	  $(LINUX_DIR)/drivers/usb/gadget/function/usb_f_hid.ko
  AUTOLOAD:=$(call AutoLoad,52,usb_f_hid)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-hid/description
  Kernel support for USB HID Gadget.
endef

$(eval $(call KernelPackage,usb-gadget-hid))

define KernelPackage/usb-gadget-ehci-debug
  TITLE:=USB EHCI debug port Gadget support
  KCONFIG:=\
	CONFIG_USB_G_DBGP \
	CONFIG_USB_G_DBGP_SERIAL=y \
	CONFIG_USB_G_DBGP_PRINTK=n
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite +kmod-usb-gadget-serial
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/legacy/g_dbgp.ko
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-ehci-debug/description
  Kernel support for USB EHCI debug port Gadget.
endef

$(eval $(call KernelPackage,usb-gadget-ehci-debug))

define KernelPackage/usb-gadget-eth
  TITLE:=USB Ethernet Gadget support
  KCONFIG:= \
	CONFIG_USB_ETH \
	CONFIG_USB_ETH_RNDIS=y \
	CONFIG_USB_ETH_EEM=n
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite
  FILES:= \
	$(LINUX_DIR)/drivers/usb/gadget/function/u_ether.ko \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_ecm.ko \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_ecm_subset.ko \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_rndis.ko \
	$(LINUX_DIR)/drivers/usb/gadget/legacy/g_ether.ko
  AUTOLOAD:=$(call AutoLoad,52,usb_f_ecm)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-eth/description
 Kernel support for USB Ethernet Gadget
endef

$(eval $(call KernelPackage,usb-gadget-eth))

define KernelPackage/usb-gadget-ncm
  TITLE:=USB Network Control Model (NCM) Gadget support
  KCONFIG:=CONFIG_USB_G_NCM
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite \
	+kmod-usb-gadget-eth
  FILES:= \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_ncm.ko \
	$(LINUX_DIR)/drivers/usb/gadget/legacy/g_ncm.ko
  AUTOLOAD:=$(call AutoLoad,52,usb_f_ncm)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-ncm/description
  Kernel support for USB Network Control Model (NCM) Gadget
endef

$(eval $(call KernelPackage,usb-gadget-ncm))

define KernelPackage/usb-gadget-serial
  TITLE:=USB Serial Gadget support
  KCONFIG:=CONFIG_USB_G_SERIAL
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite
  FILES:= \
	$(LINUX_DIR)/drivers/usb/gadget/function/u_serial.ko \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_acm.ko \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_obex.ko \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_serial.ko \
	$(LINUX_DIR)/drivers/usb/gadget/legacy/g_serial.ko
  AUTOLOAD:=$(call AutoLoad,52,usb_f_acm)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-serial/description
  Kernel support for USB Serial Gadget.
endef

$(eval $(call KernelPackage,usb-gadget-serial))

define KernelPackage/usb-gadget-mass-storage
  TITLE:=USB Mass Storage support
  KCONFIG:=CONFIG_USB_MASS_STORAGE
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite
  FILES:= \
	$(LINUX_DIR)/drivers/usb/gadget/function/usb_f_mass_storage.ko \
	$(LINUX_DIR)/drivers/usb/gadget/legacy/g_mass_storage.ko
  AUTOLOAD:=$(call AutoLoad,52,usb_f_mass_storage)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-mass-storage/description
  Kernel support for USB Gadget Mass Storage
endef

$(eval $(call KernelPackage,usb-gadget-mass-storage))

define KernelPackage/usb-gadget-cdc-composite
  TITLE:= USB CDC Composite (Ethernet + ACM)
  KCONFIG:=CONFIG_USB_CDC_COMPOSITE
  DEPENDS:=+kmod-usb-gadget +kmod-usb-lib-composite \
	+kmod-usb-gadget-eth +kmod-usb-gadget-serial
  FILES:= $(LINUX_DIR)/drivers/usb/gadget/legacy/g_cdc.ko
  $(call AddDepends/usb)
endef

define KernelPackage/usb-gadget-cdc-composite/description
  Kernel support for the USB CDC Composite gadget.
  This appears as an ethernet + ACM serial gadget.
endef

$(eval $(call KernelPackage,usb-gadget-cdc-composite))


define KernelPackage/usb-uhci
  TITLE:=Support for UHCI controllers
  KCONFIG:= \
	CONFIG_USB_PCI=y \
	CONFIG_USB_UHCI_ALT \
	CONFIG_USB_UHCI_HCD
  FILES:=$(LINUX_DIR)/drivers/usb/host/uhci-hcd.ko
  AUTOLOAD:=$(call AutoLoad,50,uhci-hcd,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-uhci/description
 Kernel support for USB UHCI controllers
endef

$(eval $(call KernelPackage,usb-uhci,1))


define KernelPackage/usb-ohci
  TITLE:=Support for OHCI controllers
  DEPENDS:= \
	+TARGET_bcm53xx:kmod-usb-bcma \
	+TARGET_bcm47xx:kmod-usb-bcma \
	+TARGET_bcm47xx:kmod-usb-ssb
  KCONFIG:= \
	CONFIG_USB_OHCI \
	CONFIG_USB_OHCI_HCD \
	CONFIG_USB_OHCI_ATH79=y \
	CONFIG_USB_OHCI_HCD_AT91=y \
	CONFIG_USB_OHCI_BCM63XX=y \
	CONFIG_USB_OCTEON_OHCI=y \
	CONFIG_USB_OHCI_HCD_OMAP3=y \
	CONFIG_USB_OHCI_HCD_PLATFORM=y
  FILES:= \
	$(LINUX_DIR)/drivers/usb/host/ohci-hcd.ko \
	$(LINUX_DIR)/drivers/usb/host/ohci-platform.ko
  ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/host/ohci-at91.ko),)
    FILES+=$(LINUX_DIR)/drivers/usb/host/ohci-at91.ko
  endif
  AUTOLOAD:=$(call AutoLoad,50,ohci-hcd ohci-platform ohci-at91,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-ohci/description
 Kernel support for USB OHCI controllers
endef

$(eval $(call KernelPackage,usb-ohci,1))


define KernelPackage/usb-ohci-pci
  TITLE:=Support for PCI OHCI controllers
  DEPENDS:=@PCI_SUPPORT +kmod-usb-ohci
  KCONFIG:= \
	CONFIG_USB_PCI=y \
	CONFIG_USB_OHCI_HCD_PCI
  FILES:=$(LINUX_DIR)/drivers/usb/host/ohci-pci.ko
  AUTOLOAD:=$(call AutoLoad,51,ohci-pci,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-ohci-pci/description
 Kernel support for PCI OHCI controllers
endef

$(eval $(call KernelPackage,usb-ohci-pci))


define KernelPackage/usb-bcma
  TITLE:=Support for BCMA USB controllers
  DEPENDS:=@USB_SUPPORT @TARGET_bcm47xx||TARGET_bcm53xx
  HIDDEN:=1
  KCONFIG:=CONFIG_USB_HCD_BCMA
  FILES:= \
	$(if $(CONFIG_USB_HCD_BCMA),$(LINUX_DIR)/drivers/usb/host/bcma-hcd.ko)
  AUTOLOAD:=$(call AutoLoad,19,$(if $(CONFIG_USB_HCD_BCMA),bcma-hcd),1)
  $(call AddDepends/usb)
endef
$(eval $(call KernelPackage,usb-bcma))

define KernelPackage/usb-fotg210
  TITLE:=Support for FOTG210 USB host controllers
  DEPENDS:=@USB_SUPPORT @TARGET_gemini
  KCONFIG:=CONFIG_USB_FOTG210_HCD
  FILES:= \
	$(if $(CONFIG_USB_FOTG210_HCD),$(LINUX_DIR)/drivers/usb/host/fotg210-hcd.ko)
  AUTOLOAD:=$(call AutoLoad,50,fotg210-hcd,1)
  $(call AddDepends/usb)
endef
$(eval $(call KernelPackage,usb-fotg210))

define KernelPackage/usb-ssb
  TITLE:=Support for SSB USB controllers
  DEPENDS:=@USB_SUPPORT @TARGET_bcm47xx
  HIDDEN:=1
  KCONFIG:=CONFIG_USB_HCD_SSB
  FILES:= \
	$(if $(CONFIG_USB_HCD_SSB),$(LINUX_DIR)/drivers/usb/host/ssb-hcd.ko)
  AUTOLOAD:=$(call AutoLoad,19,$(if $(CONFIG_USB_HCD_SSB),ssb-hcd),1)
  $(call AddDepends/usb)
endef
$(eval $(call KernelPackage,usb-ssb))

define KernelPackage/usb-ehci
  TITLE:=EHCI controller support
  HIDDEN:=1
  KCONFIG:= \
	CONFIG_USB_EHCI_HCD
  FILES:= \
	$(LINUX_DIR)/drivers/usb/host/ehci-hcd.ko
  AUTOLOAD:=$(call AutoLoad,35,ehci-hcd,1)
  $(call AddDepends/usb)
endef
$(eval $(call KernelPackage,usb-ehci))

define KernelPackage/usb2
  TITLE:=Support for USB2 controllers
  DEPENDS:=\
	+TARGET_bcm47xx:kmod-usb-bcma \
	+TARGET_bcm47xx:kmod-usb-ssb \
	+TARGET_bcm53xx:kmod-usb-bcma \
	+TARGET_bcm53xx:kmod-phy-bcm-ns-usb2 \
	+TARGET_ath79:kmod-phy-ath79-usb \
	+kmod-usb-ehci
  KCONFIG:=\
	CONFIG_USB_EHCI_HCD_PLATFORM \
	CONFIG_USB_EHCI_BCM63XX=y \
	CONFIG_USB_IMX21_HCD=y \
	CONFIG_USB_EHCI_MXC=y \
	CONFIG_USB_OCTEON_EHCI=y \
	CONFIG_USB_EHCI_HCD_ORION=y \
	CONFIG_USB_EHCI_HCD_AT91=y \
	CONFIG_USB_EHCI_FSL
  FILES:= \
	$(LINUX_DIR)/drivers/usb/host/ehci-platform.ko
  ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/host/ehci-orion.ko),)
    FILES+=$(LINUX_DIR)/drivers/usb/host/ehci-orion.ko
  endif
  ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/host/ehci-atmel.ko),)
    FILES+=$(LINUX_DIR)/drivers/usb/host/ehci-atmel.ko
  endif
  ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/host/ehci-fsl.ko),)
    FILES+=$(LINUX_DIR)/drivers/usb/host/ehci-fsl.ko
  endif
  ifneq ($(wildcard $(LINUX_DIR)/drivers/usb/host/fsl-mph-dr-of.ko),)
    FILES+=$(LINUX_DIR)/drivers/usb/host/fsl-mph-dr-of.ko
  endif
  AUTOLOAD:=$(call AutoLoad,40,ehci-hcd ehci-platform ehci-orion ehci-atmel ehci-fsl fsl-mph-dr-of,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb2/description
 Kernel support for USB2 (EHCI) controllers
endef

$(eval $(call KernelPackage,usb2))


define KernelPackage/usb2-pci
  TITLE:=Support for PCI USB2 controllers
  DEPENDS:=@PCI_SUPPORT +kmod-usb2
  KCONFIG:= \
	CONFIG_USB_PCI=y \
	CONFIG_USB_EHCI_PCI
  FILES:=$(LINUX_DIR)/drivers/usb/host/ehci-pci.ko
  AUTOLOAD:=$(call AutoLoad,42,ehci-pci,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb2-pci/description
 Kernel support for PCI USB2 (EHCI) controllers
endef

$(eval $(call KernelPackage,usb2-pci))


define KernelPackage/usb-dwc2
  TITLE:=DWC2 USB controller driver
  DEPENDS:=+USB_GADGET_SUPPORT:kmod-usb-gadget +kmod-usb-roles
  KCONFIG:= \
	CONFIG_USB_PCI=y \
	CONFIG_USB_DWC2 \
	CONFIG_USB_DWC2_PCI \
	CONFIG_USB_DWC2_PLATFORM \
	CONFIG_USB_DWC2_DEBUG=n \
	CONFIG_USB_DWC2_VERBOSE=n \
	CONFIG_USB_DWC2_TRACK_MISSED_SOFS=n \
	CONFIG_USB_DWC2_DEBUG_PERIODIC=n
  FILES:= \
	$(LINUX_DIR)/drivers/usb/dwc2/dwc2.ko
  AUTOLOAD:=$(call AutoLoad,54,dwc2,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-dwc2/description
 This driver provides USB Device Controller support for the
 Synopsys DesignWare USB OTG Core
endef

$(eval $(call KernelPackage,usb-dwc2))


define KernelPackage/usb-dwc3
  TITLE:=DWC3 USB controller driver
  KCONFIG:= \
	CONFIG_USB_DWC3 \
	CONFIG_USB_DWC3_HOST=y \
	CONFIG_USB_DWC3_GADGET=n \
	CONFIG_USB_DWC3_DUAL_ROLE=n \
	CONFIG_USB_DWC3_DEBUG=n \
	CONFIG_USB_DWC3_VERBOSE=n
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3.ko
  AUTOLOAD:=$(call AutoLoad,54,dwc3,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-dwc3/description
 This driver provides support for the Dual Role SuperSpeed
 USB Controller based on the Synopsys DesignWare USB3 IP Core
endef

$(eval $(call KernelPackage,usb-dwc3))


define KernelPackage/usb-dwc3-qcom
  TITLE:=DWC3 Qualcomm USB driver
  DEPENDS:=@(TARGET_ipq40xx||TARGET_ipq806x||TARGET_ipq807x||TARGET_ipq60xx) +kmod-usb-dwc3
  KCONFIG:= CONFIG_USB_DWC3_QCOM
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3-qcom.ko
  AUTOLOAD:=$(call AutoLoad,53,dwc3-qcom,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-dwc3-qcom/description
 Some Qualcomm SoCs use DesignWare Core IP for USB2/3 functionality.
 This driver also handles Qscratch wrapper which is needed for
 peripheral mode support.
endef


$(eval $(call KernelPackage,usb-dwc3-qcom))


define KernelPackage/usb-acm
  TITLE:=Support for modems/isdn controllers
  KCONFIG:=CONFIG_USB_ACM
  FILES:=$(LINUX_DIR)/drivers/usb/class/cdc-acm.ko
  AUTOLOAD:=$(call AutoProbe,cdc-acm)
$(call AddDepends/usb)
endef

define KernelPackage/usb-acm/description
 Kernel support for USB ACM devices (modems/isdn controllers)
endef

$(eval $(call KernelPackage,usb-acm))


define KernelPackage/usb-wdm
  TITLE:=USB Wireless Device Management
  KCONFIG:=CONFIG_USB_WDM
  FILES:=$(LINUX_DIR)/drivers/usb/class/cdc-wdm.ko
  AUTOLOAD:=$(call AutoProbe,cdc-wdm)
$(call AddDepends/usb)
$(call AddDepends/usb-net)
endef

define KernelPackage/usb-wdm/description
 USB Wireless Device Management support
endef

$(eval $(call KernelPackage,usb-wdm))


define KernelPackage/usb-audio
  TITLE:=Support for USB audio devices
  KCONFIG:= \
	CONFIG_USB_AUDIO \
	CONFIG_SND_USB=y \
	CONFIG_SND_USB_AUDIO
  $(call AddDepends/usb)
  $(call AddDepends/sound)
  FILES:= \
	$(LINUX_DIR)/sound/usb/snd-usbmidi-lib.ko \
	$(LINUX_DIR)/sound/usb/snd-usb-audio.ko
  AUTOLOAD:=$(call AutoProbe,snd-usbmidi-lib snd-usb-audio)
endef

define KernelPackage/usb-audio/description
 Kernel support for USB audio devices
endef

$(eval $(call KernelPackage,usb-audio))


define KernelPackage/usb-printer
  TITLE:=Support for printers
  KCONFIG:=CONFIG_USB_PRINTER
  FILES:=$(LINUX_DIR)/drivers/usb/class/usblp.ko
  AUTOLOAD:=$(call AutoProbe,usblp)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-printer/description
 Kernel support for USB printers
endef

$(eval $(call KernelPackage,usb-printer))


define KernelPackage/usb-serial
  TITLE:=Support for USB-to-Serial converters
  KCONFIG:=CONFIG_USB_SERIAL
  FILES:=$(LINUX_DIR)/drivers/usb/serial/usbserial.ko
  AUTOLOAD:=$(call AutoProbe,usbserial)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-serial/description
 Kernel support for USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial))


define AddDepends/usb-serial
  SUBMENU:=$(USB_MENU)
  DEPENDS+=+kmod-usb-serial $(1)
endef


define KernelPackage/usb-serial-belkin
  TITLE:=Support for Belkin devices
  KCONFIG:=CONFIG_USB_SERIAL_BELKIN
  FILES:=$(LINUX_DIR)/drivers/usb/serial/belkin_sa.ko
  AUTOLOAD:=$(call AutoProbe,belkin_sa)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-belkin/description
 Kernel support for Belkin USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-belkin))


define KernelPackage/usb-serial-ch341
  TITLE:=Support for CH341 devices
  KCONFIG:=CONFIG_USB_SERIAL_CH341
  FILES:=$(LINUX_DIR)/drivers/usb/serial/ch341.ko
  AUTOLOAD:=$(call AutoProbe,ch341)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-ch341/description
 Kernel support for Winchiphead CH341 USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-ch341))


define KernelPackage/usb-serial-edgeport
  TITLE:=Support for Digi Edgeport devices
  KCONFIG:=CONFIG_USB_SERIAL_EDGEPORT
  FILES:=$(LINUX_DIR)/drivers/usb/serial/io_edgeport.ko
  AUTOLOAD:=$(call AutoProbe,io_edgeport)
  $(call AddDepends/usb-serial)
  DEPENDS+=+edgeport-firmware
endef

define KernelPackage/usb-serial-edgeport/description
 Kernel support for Inside Out Networks (Digi)
	Edgeport/4
	Rapidport/4
	Edgeport/4t
	Edgeport/2
	Edgeport/4i
	Edgeport/2i
	Edgeport/421
	Edgeport/21
	Edgeport/8
	Edgeport/8 Dual
	Edgeport/2D8
	Edgeport/4D8
	Edgeport/8i
	Edgeport/2 DIN
	Edgeport/4 DIN
	Edgeport/16 Dual
endef

$(eval $(call KernelPackage,usb-serial-edgeport))


define KernelPackage/usb-serial-ftdi
  TITLE:=Support for FTDI devices
  KCONFIG:=CONFIG_USB_SERIAL_FTDI_SIO
  FILES:=$(LINUX_DIR)/drivers/usb/serial/ftdi_sio.ko
  AUTOLOAD:=$(call AutoProbe,ftdi_sio)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-ftdi/description
 Kernel support for FTDI USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-ftdi))


define KernelPackage/usb-serial-garmin
  TITLE:=Support for Garmin GPS devices
  KCONFIG:=CONFIG_USB_SERIAL_GARMIN
  FILES:=$(LINUX_DIR)/drivers/usb/serial/garmin_gps.ko
  AUTOLOAD:=$(call AutoProbe,garmin_gps)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-garmin/description
 Should work with most Garmin GPS devices which have a native USB port.
endef

$(eval $(call KernelPackage,usb-serial-garmin))


define KernelPackage/usb-serial-simple
  TITLE:=USB Serial Simple (Motorola phone)
  KCONFIG:=CONFIG_USB_SERIAL_SIMPLE
  FILES:=$(LINUX_DIR)/drivers/usb/serial/usb-serial-simple.ko
  AUTOLOAD:=$(call AutoProbe,usb-serial-simple)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-simple/description
  Kernel support for "very simple devices".

Specifically, it supports:
	- Suunto ANT+ USB device.
	- Medtronic CareLink USB device (3.18)
	- Fundamental Software dongle.
	- Google USB serial devices (3.19)
	- HP4x calculators
	- a number of Motorola phones
	- Novatel Wireless GPS receivers (3.18)
	- Siemens USB/MPI adapter.
	- ViVOtech ViVOpay USB device.
	- Infineon Modem Flashloader USB interface
	- ZIO Motherboard USB serial interface
endef

$(eval $(call KernelPackage,usb-serial-simple))


define KernelPackage/usb-serial-ti-usb
  TITLE:=Support for TI USB 3410/5052
  KCONFIG:=CONFIG_USB_SERIAL_TI
  FILES:=$(LINUX_DIR)/drivers/usb/serial/ti_usb_3410_5052.ko
  AUTOLOAD:=$(call AutoProbe,ti_usb_3410_5052)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-ti-usb/description
 Kernel support for TI USB 3410/5052 devices
endef

$(eval $(call KernelPackage,usb-serial-ti-usb))


define KernelPackage/usb-serial-ipw
  TITLE:=Support for IPWireless 3G devices
  KCONFIG:=CONFIG_USB_SERIAL_IPW
  FILES:=$(LINUX_DIR)/drivers/usb/serial/ipw.ko
  AUTOLOAD:=$(call AutoProbe,ipw)
  $(call AddDepends/usb-serial,+kmod-usb-serial-wwan)
endef

$(eval $(call KernelPackage,usb-serial-ipw))


define KernelPackage/usb-serial-mct
  TITLE:=Support for Magic Control Tech. devices
  KCONFIG:=CONFIG_USB_SERIAL_MCT_U232
  FILES:=$(LINUX_DIR)/drivers/usb/serial/mct_u232.ko
  AUTOLOAD:=$(call AutoProbe,mct_u232)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-mct/description
 Kernel support for Magic Control Technology USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-mct))


define KernelPackage/usb-serial-mos7720
  TITLE:=Support for Moschip MOS7720 devices
  KCONFIG:=CONFIG_USB_SERIAL_MOS7720
  FILES:=$(LINUX_DIR)/drivers/usb/serial/mos7720.ko
  AUTOLOAD:=$(call AutoProbe,mos7720)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-mos7720/description
 Kernel support for Moschip MOS7720 USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-mos7720))


define KernelPackage/usb-serial-mos7840
  TITLE:=Support for Moschip MOS7840 devices
  KCONFIG:=CONFIG_USB_SERIAL_MOS7840
  FILES:=$(LINUX_DIR)/drivers/usb/serial/mos7840.ko
  AUTOLOAD:=$(call AutoProbe,mos7840)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-mos7840/description
 Kernel support for Moschip MOS7840 USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-mos7840))


define KernelPackage/usb-serial-pl2303
  TITLE:=Support for Prolific PL2303 devices
  KCONFIG:=CONFIG_USB_SERIAL_PL2303
  FILES:=$(LINUX_DIR)/drivers/usb/serial/pl2303.ko
  AUTOLOAD:=$(call AutoProbe,pl2303)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-pl2303/description
 Kernel support for Prolific PL2303 USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-pl2303))


define KernelPackage/usb-serial-cp210x
  TITLE:=Support for Silicon Labs cp210x devices
  KCONFIG:=CONFIG_USB_SERIAL_CP210X
  FILES:=$(LINUX_DIR)/drivers/usb/serial/cp210x.ko
  AUTOLOAD:=$(call AutoProbe,cp210x)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-cp210x/description
 Kernel support for Silicon Labs cp210x USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-cp210x))


define KernelPackage/usb-serial-ark3116
  TITLE:=Support for ArkMicroChips ARK3116 devices
  KCONFIG:=CONFIG_USB_SERIAL_ARK3116
  FILES:=$(LINUX_DIR)/drivers/usb/serial/ark3116.ko
  AUTOLOAD:=$(call AutoProbe,ark3116)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-ark3116/description
 Kernel support for ArkMicroChips ARK3116 USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-ark3116))


define KernelPackage/usb-serial-oti6858
  TITLE:=Support for Ours Technology OTI6858 devices
  KCONFIG:=CONFIG_USB_SERIAL_OTI6858
  FILES:=$(LINUX_DIR)/drivers/usb/serial/oti6858.ko
  AUTOLOAD:=$(call AutoProbe,oti6858)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-oti6858/description
 Kernel support for Ours Technology OTI6858 USB-to-Serial converters
endef

$(eval $(call KernelPackage,usb-serial-oti6858))


define KernelPackage/usb-serial-sierrawireless
  TITLE:=Support for Sierra Wireless devices
  KCONFIG:=CONFIG_USB_SERIAL_SIERRAWIRELESS
  FILES:=$(LINUX_DIR)/drivers/usb/serial/sierra.ko
  AUTOLOAD:=$(call AutoProbe,sierra)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-sierrawireless/description
 Kernel support for Sierra Wireless devices
endef

$(eval $(call KernelPackage,usb-serial-sierrawireless))


define KernelPackage/usb-serial-visor
  TITLE:=Support for Handspring Visor devices
  KCONFIG:=CONFIG_USB_SERIAL_VISOR
  FILES:=$(LINUX_DIR)/drivers/usb/serial/visor.ko
  AUTOLOAD:=$(call AutoProbe,visor)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-visor/description
 Kernel support for Handspring Visor PDAs
endef

$(eval $(call KernelPackage,usb-serial-visor))


define KernelPackage/usb-serial-cypress-m8
  TITLE:=Support for CypressM8 USB-Serial
  KCONFIG:=CONFIG_USB_SERIAL_CYPRESS_M8
  FILES:=$(LINUX_DIR)/drivers/usb/serial/cypress_m8.ko
  AUTOLOAD:=$(call AutoProbe,cypress_m8)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-cypress-m8/description
 Kernel support for devices with Cypress M8 USB to Serial chip
 (for example, the Delorme Earthmate LT-20 GPS)
 Supported microcontrollers in the CY4601 family are:
 CY7C63741 CY7C63742 CY7C63743 CY7C64013
endef

$(eval $(call KernelPackage,usb-serial-cypress-m8))


define KernelPackage/usb-serial-keyspan
  TITLE:=Support for Keyspan USB-to-Serial devices
  KCONFIG:= \
	CONFIG_USB_SERIAL_KEYSPAN \
	CONFIG_USB_SERIAL_KEYSPAN_USA28 \
	CONFIG_USB_SERIAL_KEYSPAN_USA28X \
	CONFIG_USB_SERIAL_KEYSPAN_USA28XA \
	CONFIG_USB_SERIAL_KEYSPAN_USA28XB \
	CONFIG_USB_SERIAL_KEYSPAN_USA19 \
	CONFIG_USB_SERIAL_KEYSPAN_USA18X \
	CONFIG_USB_SERIAL_KEYSPAN_USA19W \
	CONFIG_USB_SERIAL_KEYSPAN_USA19QW \
	CONFIG_USB_SERIAL_KEYSPAN_USA19QI \
	CONFIG_USB_SERIAL_KEYSPAN_MPR \
	CONFIG_USB_SERIAL_KEYSPAN_USA49W \
	CONFIG_USB_SERIAL_KEYSPAN_USA49WLC
  FILES:= \
	$(LINUX_DIR)/drivers/usb/serial/keyspan.ko \
	$(wildcard $(LINUX_DIR)/drivers/usb/misc/ezusb.ko)
  AUTOLOAD:=$(call AutoProbe,ezusb keyspan)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-keyspan/description
 Kernel support for Keyspan USB-to-Serial devices
endef

$(eval $(call KernelPackage,usb-serial-keyspan))


define KernelPackage/usb-serial-wwan
  TITLE:=Support for GSM and CDMA modems
  KCONFIG:=CONFIG_USB_SERIAL_WWAN
  FILES:=$(LINUX_DIR)/drivers/usb/serial/usb_wwan.ko
  HIDDEN:=1
  AUTOLOAD:=$(call AutoProbe,usb_wwan)
  $(call AddDepends/usb-serial)
endef

define KernelPackage/usb-serial-wwan/description
 Kernel support for USB GSM and CDMA modems
endef

$(eval $(call KernelPackage,usb-serial-wwan))


define KernelPackage/usb-serial-option
  TITLE:=Support for Option HSDPA modems
  KCONFIG:=CONFIG_USB_SERIAL_OPTION
  FILES:=$(LINUX_DIR)/drivers/usb/serial/option.ko
  AUTOLOAD:=$(call AutoProbe,option)
  $(call AddDepends/usb-serial,+kmod-usb-serial-wwan)
endef

define KernelPackage/usb-serial-option/description
 Kernel support for Option HSDPA modems
endef

$(eval $(call KernelPackage,usb-serial-option))


define KernelPackage/usb-serial-qualcomm
  TITLE:=Support for Qualcomm USB serial
  KCONFIG:=CONFIG_USB_SERIAL_QUALCOMM
  FILES:=$(LINUX_DIR)/drivers/usb/serial/qcserial.ko
  AUTOLOAD:=$(call AutoProbe,qcserial)
  $(call AddDepends/usb-serial,+kmod-usb-serial-wwan)
endef

define KernelPackage/usb-serial-qualcomm/description
 Kernel support for Qualcomm USB Serial devices (Gobi)
endef

$(eval $(call KernelPackage,usb-serial-qualcomm))


define KernelPackage/usb-storage
  TITLE:=USB Storage support
  DEPENDS:= +kmod-scsi-core
  KCONFIG:=CONFIG_USB_STORAGE
  FILES:=$(LINUX_DIR)/drivers/usb/storage/usb-storage.ko
  AUTOLOAD:=$(call AutoProbe,usb-storage,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-storage/description
 Kernel support for USB Mass Storage devices
endef

$(eval $(call KernelPackage,usb-storage))


define KernelPackage/usb-storage-extras
  SUBMENU:=$(USB_MENU)
  TITLE:=Extra drivers for usb-storage
  DEPENDS:=+kmod-usb-storage
  KCONFIG:= \
	CONFIG_USB_STORAGE_ALAUDA \
	CONFIG_USB_STORAGE_CYPRESS_ATACB \
	CONFIG_USB_STORAGE_DATAFAB \
	CONFIG_USB_STORAGE_FREECOM \
	CONFIG_USB_STORAGE_ISD200 \
	CONFIG_USB_STORAGE_JUMPSHOT \
	CONFIG_USB_STORAGE_KARMA \
	CONFIG_USB_STORAGE_SDDR09 \
	CONFIG_USB_STORAGE_SDDR55 \
	CONFIG_USB_STORAGE_USBAT
  FILES:= \
	$(LINUX_DIR)/drivers/usb/storage/ums-alauda.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-cypress.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-datafab.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-freecom.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-isd200.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-jumpshot.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-karma.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-sddr09.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-sddr55.ko \
	$(LINUX_DIR)/drivers/usb/storage/ums-usbat.ko
  AUTOLOAD:=$(call AutoProbe,ums-alauda ums-cypress ums-datafab \
				ums-freecom ums-isd200 ums-jumpshot \
				ums-karma ums-sddr09 ums-sddr55 ums-usbat)
endef

define KernelPackage/usb-storage-extras/description
 Say Y here if you want to have some more drivers,
 such as for SmartMedia card readers
endef

$(eval $(call KernelPackage,usb-storage-extras))


define KernelPackage/usb-storage-uas
  SUBMENU:=$(USB_MENU)
  TITLE:=USB Attached SCSI (UASP) support
  DEPENDS:=+kmod-usb-storage
  KCONFIG:=CONFIG_USB_UAS
  FILES:=$(LINUX_DIR)/drivers/usb/storage/uas.ko
  AUTOLOAD:=$(call AutoProbe,uas,1)
endef

define KernelPackage/usb-storage-uas/description
 Say Y here if you want to include support for
 USB Attached SCSI (UAS/UASP), a higher
 performance protocol available on many
 newer USB 3.0 storage devices
endef

$(eval $(call KernelPackage,usb-storage-uas))


define KernelPackage/usb-atm
  TITLE:=Support for ATM on USB bus
  DEPENDS:=+kmod-atm
  KCONFIG:=CONFIG_USB_ATM
  FILES:=$(LINUX_DIR)/drivers/usb/atm/usbatm.ko
  AUTOLOAD:=$(call AutoProbe,usbatm)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-atm/description
 Kernel support for USB DSL modems
endef

$(eval $(call KernelPackage,usb-atm))


define AddDepends/usb-atm
  SUBMENU:=$(USB_MENU)
  DEPENDS+=kmod-usb-atm $(1)
endef


define KernelPackage/usb-atm-speedtouch
  TITLE:=SpeedTouch USB ADSL modems support
  KCONFIG:=CONFIG_USB_SPEEDTOUCH
  FILES:=$(LINUX_DIR)/drivers/usb/atm/speedtch.ko
  AUTOLOAD:=$(call AutoProbe,speedtch)
  $(call AddDepends/usb-atm)
endef

define KernelPackage/usb-atm-speedtouch/description
 Kernel support for SpeedTouch USB ADSL modems
endef

$(eval $(call KernelPackage,usb-atm-speedtouch))


define KernelPackage/usb-atm-ueagle
  TITLE:=Eagle 8051 based USB ADSL modems support
  FILES:=$(LINUX_DIR)/drivers/usb/atm/ueagle-atm.ko
  KCONFIG:=CONFIG_USB_UEAGLEATM
  AUTOLOAD:=$(call AutoProbe,ueagle-atm)
  $(call AddDepends/usb-atm)
endef

define KernelPackage/usb-atm-ueagle/description
 Kernel support for Eagle 8051 based USB ADSL modems
endef

$(eval $(call KernelPackage,usb-atm-ueagle))


define KernelPackage/usb-atm-cxacru
  TITLE:=cxacru
  FILES:=$(LINUX_DIR)/drivers/usb/atm/cxacru.ko
  KCONFIG:=CONFIG_USB_CXACRU
  AUTOLOAD:=$(call AutoProbe,cxacru)
  $(call AddDepends/usb-atm)
endef

define KernelPackage/usb-atm-cxacru/description
 Kernel support for cxacru based USB ADSL modems
endef

$(eval $(call KernelPackage,usb-atm-cxacru))


define KernelPackage/usb-net
  TITLE:=Kernel modules for USB-to-Ethernet convertors
  DEPENDS:=+kmod-mii
  KCONFIG:=CONFIG_USB_USBNET \
	CONFIG_USB_NET_DRIVERS
  AUTOLOAD:=$(call AutoProbe,usbnet)
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/usbnet.ko
  $(call AddDepends/usb)
endef

define KernelPackage/usb-net/description
 Kernel modules for USB-to-Ethernet convertors
endef

$(eval $(call KernelPackage,usb-net))


define AddDepends/usb-net
  SUBMENU:=$(USB_MENU)
  DEPENDS+=+kmod-usb-net $(1)
endef


define KernelPackage/usb-net-aqc111
  TITLE:=Support for USB-to-Ethernet Aquantia AQtion 5/2.5GbE
  KCONFIG:=CONFIG_USB_NET_AQC111
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/aqc111.ko
  AUTOLOAD:=$(call AutoProbe,aqc111)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-aqc111/description
 Support for USB-to-Ethernet Aquantia AQtion 5/2.5GbE
endef

$(eval $(call KernelPackage,usb-net-aqc111))


define KernelPackage/usb-net-asix
  TITLE:=Kernel module for USB-to-Ethernet Asix convertors
  DEPENDS:=+kmod-libphy +(LINUX_5_15||LINUX_6_0):kmod-mdio-devres
  KCONFIG:=CONFIG_USB_NET_AX8817X
  FILES:= \
	$(LINUX_DIR)/drivers/$(USBNET_DIR)/asix.ko \
	$(LINUX_DIR)/net/core/selftests.ko@ge5.13
  AUTOLOAD:=$(call AutoProbe,asix)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-asix/description
 Kernel module for USB-to-Ethernet Asix convertors
endef

$(eval $(call KernelPackage,usb-net-asix))


define KernelPackage/usb-net-asix-ax88179
  TITLE:=Kernel module for USB-to-Gigabit-Ethernet Asix convertors
  DEPENDS:=+kmod-libphy
  KCONFIG:=CONFIG_USB_NET_AX88179_178A
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/ax88179_178a.ko
  AUTOLOAD:=$(call AutoProbe,ax88179_178a)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-asix-ax88179/description
 Kernel module for USB-to-Ethernet ASIX AX88179 based USB 3.0/2.0
 to Gigabit Ethernet adapters.
endef

$(eval $(call KernelPackage,usb-net-asix-ax88179))


define KernelPackage/usb-net-hso
  TITLE:=Kernel module for Option USB High Speed Mobile Devices
  KCONFIG:=CONFIG_USB_HSO
  FILES:= \
	$(LINUX_DIR)/drivers/$(USBNET_DIR)/hso.ko
  AUTOLOAD:=$(call AutoProbe,hso)
  $(call AddDepends/usb-net)
  $(call AddDepends/rfkill)
endef

define KernelPackage/usb-net-hso/description
 Kernel module for Option USB High Speed Mobile Devices
endef

$(eval $(call KernelPackage,usb-net-hso))


define KernelPackage/usb-net-kaweth
  TITLE:=Kernel module for USB-to-Ethernet Kaweth convertors
  KCONFIG:=CONFIG_USB_KAWETH
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/kaweth.ko
  AUTOLOAD:=$(call AutoProbe,kaweth)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-kaweth/description
 Kernel module for USB-to-Ethernet Kaweth convertors
endef

$(eval $(call KernelPackage,usb-net-kaweth))


define KernelPackage/usb-net-pegasus
  TITLE:=Kernel module for USB-to-Ethernet Pegasus convertors
  KCONFIG:=CONFIG_USB_PEGASUS
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/pegasus.ko
  AUTOLOAD:=$(call AutoProbe,pegasus)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-pegasus/description
 Kernel module for USB-to-Ethernet Pegasus convertors
endef

$(eval $(call KernelPackage,usb-net-pegasus))


define KernelPackage/usb-net-mcs7830
  TITLE:=Kernel module for USB-to-Ethernet MCS7830 convertors
  KCONFIG:=CONFIG_USB_NET_MCS7830
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/mcs7830.ko
  AUTOLOAD:=$(call AutoProbe,mcs7830)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-mcs7830/description
 Kernel module for USB-to-Ethernet MCS7830 convertors
endef

$(eval $(call KernelPackage,usb-net-mcs7830))


define KernelPackage/usb-net-smsc75xx
  TITLE:=SMSC LAN75XX based USB 2.0 Gigabit ethernet devices
  DEPENDS:=+!LINUX_5_4:kmod-libphy
  KCONFIG:=CONFIG_USB_NET_SMSC75XX
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/smsc75xx.ko
  AUTOLOAD:=$(call AutoProbe,smsc75xx)
  $(call AddDepends/usb-net, +kmod-lib-crc16)
endef

define KernelPackage/usb-net-smsc75xx/description
 Kernel module for SMSC LAN75XX based devices
endef

$(eval $(call KernelPackage,usb-net-smsc75xx))


define KernelPackage/usb-net-smsc95xx
  TITLE:=SMSC LAN95XX based USB 2.0 10/100 ethernet devices
  DEPENDS:=+!LINUX_5_4:kmod-libphy
  KCONFIG:=CONFIG_USB_NET_SMSC95XX
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/smsc95xx.ko
  AUTOLOAD:=$(call AutoProbe,smsc95xx)
  $(call AddDepends/usb-net, +kmod-lib-crc16)
endef

define KernelPackage/usb-net-smsc95xx/description
 Kernel module for SMSC LAN95XX based devices
endef

$(eval $(call KernelPackage,usb-net-smsc95xx))


define KernelPackage/usb-net-dm9601-ether
  TITLE:=Support for DM9601 ethernet connections
  KCONFIG:=CONFIG_USB_NET_DM9601
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/dm9601.ko
  AUTOLOAD:=$(call AutoProbe,dm9601)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-dm9601-ether/description
 Kernel support for USB DM9601 devices
endef

$(eval $(call KernelPackage,usb-net-dm9601-ether))

define KernelPackage/usb-net-cdc-ether
  TITLE:=Support for cdc ethernet connections
  KCONFIG:=CONFIG_USB_NET_CDCETHER
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/cdc_ether.ko
  AUTOLOAD:=$(call AutoProbe,cdc_ether)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-cdc-ether/description
 Kernel support for USB CDC Ethernet devices
endef

$(eval $(call KernelPackage,usb-net-cdc-ether))


define KernelPackage/usb-net-cdc-eem
  TITLE:=Support for CDC EEM connections
  KCONFIG:=CONFIG_USB_NET_CDC_EEM
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/cdc_eem.ko
  AUTOLOAD:=$(call AutoProbe,cdc_eem)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-cdc-eem/description
 Kernel support for USB CDC EEM
endef

$(eval $(call KernelPackage,usb-net-cdc-eem))


define KernelPackage/usb-net-cdc-subset
  TITLE:=Support for CDC Ethernet subset connections
  KCONFIG:= \
	CONFIG_USB_NET_CDC_SUBSET \
	CONFIG_USB_ARMLINUX
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/cdc_subset.ko
  AUTOLOAD:=$(call AutoProbe,cdc_subset)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-cdc-subset/description
 Kernel support for Simple USB Network Links (CDC Ethernet subset)
endef

$(eval $(call KernelPackage,usb-net-cdc-subset))


define KernelPackage/usb-net-qmi-wwan
  TITLE:=QMI WWAN driver
  KCONFIG:=CONFIG_USB_NET_QMI_WWAN
  FILES:= $(LINUX_DIR)/drivers/$(USBNET_DIR)/qmi_wwan.ko
  AUTOLOAD:=$(call AutoProbe,qmi_wwan)
  $(call AddDepends/usb-net,+kmod-usb-wdm)
endef

define KernelPackage/usb-net-qmi-wwan/description
 QMI WWAN driver for Qualcomm MSM based 3G and LTE modems
endef

$(eval $(call KernelPackage,usb-net-qmi-wwan))


define KernelPackage/usb-net-rtl8150
  TITLE:=Kernel module for USB-to-Ethernet Realtek convertors
  KCONFIG:=CONFIG_USB_RTL8150
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/rtl8150.ko
  AUTOLOAD:=$(call AutoProbe,rtl8150)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-rtl8150/description
 Kernel module for USB-to-Ethernet Realtek 8150 convertors
endef

$(eval $(call KernelPackage,usb-net-rtl8150))


define KernelPackage/usb-net-rtl8152
  TITLE:=Kernel module for USB-to-Ethernet Realtek convertors
  DEPENDS:=+r8152-firmware +kmod-crypto-sha256 +kmod-usb-net-cdc-ncm
  KCONFIG:=CONFIG_USB_RTL8152
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/r8152.ko
  AUTOLOAD:=$(call AutoProbe,r8152)
  $(call AddDepends/usb-net, +LINUX_5_10:kmod-crypto-hash)
endef

define KernelPackage/usb-net-rtl8152/description
 Kernel module for USB-to-Ethernet Realtek 8152 USB2.0/3.0 convertors
endef

$(eval $(call KernelPackage,usb-net-rtl8152))


define KernelPackage/usb-net-sr9700
  TITLE:=Support for CoreChip SR9700 ethernet devices
  KCONFIG:=CONFIG_USB_NET_SR9700
  FILES:=$(LINUX_DIR)/drivers/$(USBNET_DIR)/sr9700.ko
  AUTOLOAD:=$(call AutoProbe,sr9700)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-sr9700/description
 Kernel module for CoreChip-sz SR9700 based USB 1.1 10/100 ethernet devices
endef

$(eval $(call KernelPackage,usb-net-sr9700))


define KernelPackage/usb-net-rndis
  TITLE:=Support for RNDIS connections
  KCONFIG:=CONFIG_USB_NET_RNDIS_HOST
  FILES:= $(LINUX_DIR)/drivers/$(USBNET_DIR)/rndis_host.ko
  AUTOLOAD:=$(call AutoProbe,rndis_host)
  $(call AddDepends/usb-net,+kmod-usb-net-cdc-ether)
endef

define KernelPackage/usb-net-rndis/description
 Kernel support for RNDIS connections
endef

$(eval $(call KernelPackage,usb-net-rndis))


define KernelPackage/usb-net-cdc-mbim
  SUBMENU:=$(USB_MENU)
  TITLE:=Kernel module for MBIM Devices
  KCONFIG:=CONFIG_USB_NET_CDC_MBIM
  FILES:= \
   $(LINUX_DIR)/drivers/$(USBNET_DIR)/cdc_mbim.ko
  AUTOLOAD:=$(call AutoProbe,cdc_mbim)
  $(call AddDepends/usb-net,+kmod-usb-wdm +kmod-usb-net-cdc-ncm)
endef

define KernelPackage/usb-net-cdc-mbim/description
 Kernel module for CDC MBIM (Mobile Broadband Interface Model) devices
endef

$(eval $(call KernelPackage,usb-net-cdc-mbim))


define KernelPackage/usb-net-cdc-ncm
  TITLE:=Support for CDC NCM connections
  KCONFIG:=CONFIG_USB_NET_CDC_NCM
  FILES:= $(LINUX_DIR)/drivers/$(USBNET_DIR)/cdc_ncm.ko
  AUTOLOAD:=$(call AutoProbe,cdc_ncm)
  $(call AddDepends/usb-net,+!LINUX_5_4:kmod-usb-net-cdc-ether)
endef

define KernelPackage/usb-net-cdc-ncm/description
 Kernel support for CDC NCM connections
endef

$(eval $(call KernelPackage,usb-net-cdc-ncm))


define KernelPackage/usb-net-huawei-cdc-ncm
  TITLE:=Support for Huawei CDC NCM connections
  KCONFIG:=CONFIG_USB_NET_HUAWEI_CDC_NCM
  FILES:= $(LINUX_DIR)/drivers/$(USBNET_DIR)/huawei_cdc_ncm.ko
  AUTOLOAD:=$(call AutoProbe,huawei_cdc_ncm)
  $(call AddDepends/usb-net,+kmod-usb-net-cdc-ncm +kmod-usb-wdm)
endef

define KernelPackage/usb-net-huawei-cdc-ncm/description
 Kernel support for Huawei CDC NCM connections
endef

$(eval $(call KernelPackage,usb-net-huawei-cdc-ncm))


define KernelPackage/usb-net-sierrawireless
  TITLE:=Support for Sierra Wireless devices
  KCONFIG:=CONFIG_USB_SIERRA_NET
  FILES:=$(LINUX_DIR)/drivers/net/usb/sierra_net.ko
  AUTOLOAD:=$(call AutoProbe,sierra_net)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-sierrawireless/description
 Kernel support for Sierra Wireless devices
endef

$(eval $(call KernelPackage,usb-net-sierrawireless))


define KernelPackage/usb-net-ipheth
  TITLE:=Apple iPhone USB Ethernet driver
  KCONFIG:=CONFIG_USB_IPHETH
  FILES:=$(LINUX_DIR)/drivers/net/usb/ipheth.ko
  AUTOLOAD:=$(call AutoProbe,ipheth)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-ipheth/description
 Kernel support for Apple iPhone USB Ethernet driver
endef

$(eval $(call KernelPackage,usb-net-ipheth))


define KernelPackage/usb-net-kalmia
  TITLE:=Samsung Kalmia based LTE USB modem
  KCONFIG:=CONFIG_USB_NET_KALMIA
  FILES:=$(LINUX_DIR)/drivers/net/usb/kalmia.ko
  AUTOLOAD:=$(call AutoProbe,kalmia)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-kalmia/description
 Kernel support for Samsung Kalmia based LTE USB modem
endef

$(eval $(call KernelPackage,usb-net-kalmia))

define KernelPackage/usb-net-pl
  TITLE:=Prolific PL-2301/2302/25A1 based cables
  KCONFIG:=CONFIG_USB_NET_PLUSB
  FILES:=$(LINUX_DIR)/drivers/net/usb/plusb.ko
  AUTOLOAD:=$(call AutoProbe,plusb)
  $(call AddDepends/usb-net)
endef

define KernelPackage/usb-net-pl/description
 Kernel support for Prolific PL-2301/2302/25A1 based cables
endef

$(eval $(call KernelPackage,usb-net-pl))

define KernelPackage/usb-hid
  TITLE:=Support for USB Human Input Devices
  KCONFIG:=CONFIG_HID_SUPPORT=y CONFIG_USB_HID CONFIG_USB_HIDDEV=y
  DEPENDS:=+kmod-hid +kmod-hid-generic +kmod-input-evdev
  FILES:=$(LINUX_DIR)/drivers/$(USBHID_DIR)/usbhid.ko
  AUTOLOAD:=$(call AutoProbe,usbhid)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-hid/description
 Kernel support for USB HID devices such as keyboards and mice
endef

$(eval $(call KernelPackage,usb-hid))


define KernelPackage/usb-hid-cp2112
  SUBMENU:=$(USB_MENU)
  TITLE:=Silicon Labs CP2112 HID USB to SMBus Master Bridge
  KCONFIG:=CONFIG_GPIOLIB=y CONFIG_HID_CP2112
  DEPENDS:=+kmod-usb-hid +kmod-i2c-core
  FILES:=$(LINUX_DIR)/drivers/hid/hid-cp2112.ko
  AUTOLOAD:=$(call AutoProbe,hid-cp2112)
endef

define KernelPackage/usb-hid-cp2112/description
 HID device driver which registers as an i2c adapter and gpiochip to expose
 these functions of the CP2112.
endef

$(eval $(call KernelPackage,usb-hid-cp2112))


define KernelPackage/usb-yealink
  TITLE:=USB Yealink VOIP phone
  DEPENDS:=+kmod-input-evdev
  KCONFIG:=CONFIG_USB_YEALINK CONFIG_INPUT_YEALINK CONFIG_INPUT=m CONFIG_INPUT_MISC=y
  FILES:=$(LINUX_DIR)/drivers/$(USBINPUT_DIR)/yealink.ko
  AUTOLOAD:=$(call AutoProbe,yealink)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-yealink/description
 Kernel support for Yealink VOIP phone
endef

$(eval $(call KernelPackage,usb-yealink))


define KernelPackage/usb-cm109
  TITLE:=Support for CM109 device
  DEPENDS:=+kmod-input-evdev
  KCONFIG:=CONFIG_USB_CM109 CONFIG_INPUT_CM109 CONFIG_INPUT=m CONFIG_INPUT_MISC=y
  FILES:=$(LINUX_DIR)/drivers/$(USBINPUT_DIR)/cm109.ko
  AUTOLOAD:=$(call AutoProbe,cm109)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-cm109/description
 Kernel support for CM109 VOIP phone
endef

$(eval $(call KernelPackage,usb-cm109))


define KernelPackage/usb-test
  TITLE:=USB Testing Driver
  DEPENDS:=@DEVEL
  KCONFIG:=CONFIG_USB_TEST
  FILES:=$(LINUX_DIR)/drivers/usb/misc/usbtest.ko
  $(call AddDepends/usb)
endef

define KernelPackage/usb-test/description
 Kernel support for testing USB Host Controller software
endef

$(eval $(call KernelPackage,usb-test))


define KernelPackage/usbip
  TITLE := USB-over-IP kernel support
  KCONFIG:= \
	CONFIG_USBIP_CORE \
	CONFIG_USBIP_DEBUG=n
  FILES:=$(LINUX_DIR)/drivers/usb/usbip/usbip-core.ko
  AUTOLOAD:=$(call AutoProbe,usbip-core)
  $(call AddDepends/usb)
endef

$(eval $(call KernelPackage,usbip))


define KernelPackage/usbip-client
  TITLE := USB-over-IP client driver
  DEPENDS := +kmod-usbip
  KCONFIG := CONFIG_USBIP_VHCI_HCD
  FILES :=$(LINUX_DIR)/drivers/usb/usbip/vhci-hcd.ko
  AUTOLOAD := $(call AutoProbe,vhci-hcd)
  $(call AddDepends/usb)
endef

$(eval $(call KernelPackage,usbip-client))


define KernelPackage/usbip-server
$(call KernelPackage/usbip/Default)
  TITLE := USB-over-IP host driver
  DEPENDS := +kmod-usbip
  KCONFIG := CONFIG_USBIP_HOST
  FILES :=$(LINUX_DIR)/drivers/usb/usbip/usbip-host.ko
  AUTOLOAD := $(call AutoProbe,usbip-host)
  $(call AddDepends/usb)
endef

$(eval $(call KernelPackage,usbip-server))

define KernelPackage/usb-chipidea
  TITLE:=Host and device support for Chipidea controllers
  DEPENDS:=+USB_GADGET_SUPPORT:kmod-usb-gadget @TARGET_ath79 +kmod-usb-ehci +kmod-usb-phy-nop +kmod-usb-roles
  KCONFIG:= \
	CONFIG_EXTCON \
	CONFIG_USB_CHIPIDEA \
	CONFIG_USB_CHIPIDEA_GENERIC \
	CONFIG_USB_CHIPIDEA_HOST=y \
	CONFIG_USB_CHIPIDEA_UDC=y \
	CONFIG_USB_CHIPIDEA_DEBUG=y
  FILES:= \
	$(LINUX_DIR)/drivers/extcon/extcon-core.ko \
	$(LINUX_DIR)/drivers/usb/chipidea/ci_hdrc.ko \
	$(LINUX_DIR)/drivers/usb/common/ulpi.ko
  AUTOLOAD:=$(call AutoLoad,39,ci_hdrc,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-chipidea/description
 Kernel support for USB Chipidea controllers
endef

$(eval $(call KernelPackage,usb-chipidea))


define KernelPackage/usb-chipidea2
  TITLE:=Host and device support for Chipidea2 controllers
  DEPENDS:=+kmod-usb-chipidea
  KCONFIG:= \
	CONFIG_EXTCON \
	CONFIG_USB_CHIPIDEA \
	CONFIG_USB_CHIPIDEA_HOST=y \
	CONFIG_USB_CHIPIDEA_UDC=y \
	CONFIG_USB_CHIPIDEA_DEBUG=y
  FILES:= \
	$(LINUX_DIR)/drivers/extcon/extcon-core.ko \
	$(LINUX_DIR)/drivers/usb/chipidea/ci_hdrc_usb2.ko
  AUTOLOAD:=$(call AutoLoad,39,ci_hdrc_usb2,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-chipidea2/description
 Kernel support for USB Chipidea controllers
endef

$(eval $(call KernelPackage,usb-chipidea2))


define KernelPackage/usbmon
  TITLE:=USB traffic monitor
  KCONFIG:=CONFIG_USB_MON
  $(call AddDepends/usb)
  FILES:=$(LINUX_DIR)/drivers/usb/mon/usbmon.ko
  AUTOLOAD:=$(call AutoProbe,usbmon)
endef

define KernelPackage/usbmon/description
 Kernel support for USB traffic monitoring
endef

$(eval $(call KernelPackage,usbmon))

XHCI_MODULES := xhci-pci xhci-plat-hcd
XHCI_FILES := $(wildcard $(patsubst %,$(LINUX_DIR)/drivers/usb/host/%.ko,$(XHCI_MODULES)))
XHCI_AUTOLOAD := $(patsubst $(LINUX_DIR)/drivers/usb/host/%.ko,%,$(XHCI_FILES))

define KernelPackage/usb3
  TITLE:=Support for USB3 controllers
  DEPENDS:= \
	+kmod-usb-xhci-hcd \
	+TARGET_bcm53xx:kmod-usb-bcma \
	+TARGET_bcm53xx:kmod-phy-bcm-ns-usb3 \
	+TARGET_ramips_mt7621:kmod-usb-xhci-mtk \
	+TARGET_mediatek:kmod-usb-xhci-mtk \
	+(TARGET_apm821xx_nand&&LINUX_5_10):kmod-usb-xhci-pci-renesas
  KCONFIG:= \
	CONFIG_USB_PCI=y \
	CONFIG_USB_XHCI_PCI \
	CONFIG_USB_XHCI_PLATFORM
  FILES:= \
	$(XHCI_FILES)
  AUTOLOAD:=$(call AutoLoad,54,$(XHCI_AUTOLOAD),1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb3/description
 Kernel support for USB3 (XHCI) controllers
endef

$(eval $(call KernelPackage,usb3))


define KernelPackage/usb-net2280
  TITLE:=Support for NetChip 228x PCI USB peripheral controller
  KCONFIG:= \
	CONFIG_USB_PCI=y \
	CONFIG_USB_NET2280
  DEPENDS:=@PCI_SUPPORT +kmod-usb-gadget
  FILES:=$(LINUX_DIR)/drivers/usb/gadget/udc/net2280.ko
  AUTOLOAD:=$(call AutoLoad,46,net2280)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-net2280/description
  Kernel support for NetChip 228x / PLX USB338x PCI USB peripheral controller.
endef

$(eval $(call KernelPackage,usb-net2280))

define KernelPackage/usb-roles
  TITLE:=USB Role Switch Library Module
  KCONFIG:=CONFIG_USB_ROLE_SWITCH
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/usb/roles/roles.ko
  $(call AddDepends/usb)
endef

define KernelPackage/usb-roles/description
  Support for USB Role Switch
endef

$(eval $(call KernelPackage,usb-roles))


define KernelPackage/usb-xhci-hcd
  TITLE:=xHCI HCD (USB 3.0) support
  KCONFIG:= \
	  CONFIG_USB_XHCI_HCD \
	  CONFIG_USB_XHCI_HCD_DEBUGGING=n
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/usb/host/xhci-hcd.ko
  AUTOLOAD:=$(call AutoLoad,54,xhci-hcd,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-xhci-hcd/description
  The eXtensible Host Controller Interface (xHCI) is standard for USB 3.0
  "SuperSpeed" host controller hardware.
endef

$(eval $(call KernelPackage,usb-xhci-hcd))


define KernelPackage/usb-xhci-mtk
  TITLE:=xHCI support for MediaTek SoCs
  DEPENDS:=+kmod-usb-xhci-hcd
  KCONFIG:=CONFIG_USB_XHCI_MTK
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/usb/host/xhci-mtk.ko
  AUTOLOAD:=$(call AutoLoad,54,xhci-mtk,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-xhci-mtk/description
  Kernel support for the xHCI host controller found in MediaTek SoCs.
endef

$(eval $(call KernelPackage,usb-xhci-mtk))


define KernelPackage/usb-xhci-pci-renesas
  TITLE:=Support for additional Renesas xHCI controller with firmware
  DEPENDS:=@LINUX_5_10
  KCONFIG:=CONFIG_USB_XHCI_PCI_RENESAS
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/usb/host/xhci-pci-renesas.ko
  AUTOLOAD:=$(call AutoLoad,54,xhci-pci-renesas,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-xhci-pci-renesas/description
  Kernel support for the Renesas xHCI controller with firmware. Make sure you have
  the firwmare for the device and installed on your system for this device to work.
endef

$(eval $(call KernelPackage,usb-xhci-pci-renesas))


define KernelPackage/chaoskey
  SUBMENU:=$(USB_MENU)
  TITLE:=Chaoskey hardware RNG support
  DEPENDS:=+kmod-random-core
  KCONFIG:=CONFIG_USB_CHAOSKEY
  FILES:=$(LINUX_DIR)/drivers/usb/misc/chaoskey.ko
  AUTOLOAD:=$(call AutoProbe,chaoskey)
  $(call AddDepends/usb)
endef

define KernelPackage/chaoskey/description
  Kernel module for chaoskey, USB attached true random number generator
endef

$(eval $(call KernelPackage,chaoskey))

