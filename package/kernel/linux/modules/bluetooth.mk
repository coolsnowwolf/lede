#
# Copyright (C) 2006-2025 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

BLUETOOTH_MENU:=Bluetooth Support

define KernelPackage/bluetooth
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=Bluetooth support
  DEPENDS:=+kmod-crypto-hash +kmod-crypto-ecb +kmod-lib-crc16 +kmod-hid +kmod-crypto-cmac +kmod-regmap-core +kmod-crypto-ecdh
  KCONFIG:= \
	CONFIG_BT \
	CONFIG_BT_BREDR=y \
	CONFIG_BT_DEBUGFS=n \
	CONFIG_BT_LE=y \
	CONFIG_BT_RFCOMM \
	CONFIG_BT_BNEP \
	CONFIG_BT_HIDP
  $(call AddDepends/rfkill)
  FILES:= \
	$(LINUX_DIR)/net/bluetooth/bluetooth.ko \
	$(LINUX_DIR)/net/bluetooth/rfcomm/rfcomm.ko \
	$(LINUX_DIR)/net/bluetooth/bnep/bnep.ko \
	$(LINUX_DIR)/net/bluetooth/hidp/hidp.ko
  AUTOLOAD:=$(call AutoProbe,bluetooth rfcomm bnep hidp)
endef

define KernelPackage/bluetooth/description
 Kernel support for Bluetooth devices
endef

$(eval $(call KernelPackage,bluetooth))


define KernelPackage/hci-uart
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=Bluetooth HCI UART support
  DEPENDS:=+kmod-bluetooth
  KCONFIG:= \
	CONFIG_BT_HCIUART \
	CONFIG_BT_HCIUART_BCM=n \
	CONFIG_BT_HCIUART_INTEL=n \
	CONFIG_BT_HCIUART_H4 \
	CONFIG_BT_HCIUART_NOKIA=n
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/hci_uart.ko
  AUTOLOAD:=$(call AutoProbe,hci_uart)
endef

define KernelPackage/hci-uart/description
 Kernel support for Bluetooth HCI UART devices
endef

$(eval $(call KernelPackage,hci-uart))


define KernelPackage/btusb
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=Bluetooth HCI USB support
  DEPENDS:=@USB_SUPPORT +kmod-usb-core +kmod-bluetooth +kmod-btmtk
  KCONFIG:= \
	CONFIG_BT_HCIBTUSB \
	CONFIG_BT_HCIBTUSB_BCM=n \
	CONFIG_BT_HCIBTUSB_MTK=y \
	CONFIG_BT_HCIBTUSB_RTL=y
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/btusb.ko \
	$(LINUX_DIR)/drivers/bluetooth/btintel.ko \
	$(LINUX_DIR)/drivers/bluetooth/btrtl.ko
  AUTOLOAD:=$(call AutoProbe,btusb)
endef

define KernelPackage/btusb/description
 Kernel support for USB Bluetooth HCI USB devices
endef

$(eval $(call KernelPackage,btusb))


define KernelPackage/btmtk
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=MTK Bluetooth support
  HIDDEN:=1
  DEPENDS:=+kmod-bluetooth +USB_SUPPORT:kmod-usb-core
  KCONFIG:=CONFIG_BT_MTK
  FILES:=$(LINUX_DIR)/drivers/bluetooth/btmtk.ko
endef

$(eval $(call KernelPackage,btmtk))


define KernelPackage/ath3k
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=ATH3K Kernel Module support
  DEPENDS:=+kmod-hci-uart +kmod-btusb +ar3k-firmware
  KCONFIG:= \
	CONFIG_BT_ATH3K \
	CONFIG_BT_HCIUART_ATH3K=y
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/ath3k.ko
  AUTOLOAD:=$(call AutoProbe,ath3k)
endef

define KernelPackage/ath3k/description
 Kernel support for ATH3K Module
endef

$(eval $(call KernelPackage,ath3k))


define KernelPackage/bluetooth-6lowpan
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=Bluetooth 6LoWPAN support
  DEPENDS:=+kmod-6lowpan +kmod-bluetooth
  KCONFIG:=CONFIG_BT_6LOWPAN
  FILES:=$(LINUX_DIR)/net/bluetooth/bluetooth_6lowpan.ko
  AUTOLOAD:=$(call AutoProbe,bluetooth_6lowpan)
endef

define KernelPackage/bluetooth-6lowpan/description
 Kernel support for 6LoWPAN over Bluetooth Low Energy devices
endef

$(eval $(call KernelPackage,bluetooth-6lowpan))


define KernelPackage/btmrvl
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=Marvell Bluetooth Kernel Module support
  DEPENDS:=+kmod-mmc +kmod-bluetooth +mwifiex-sdio-firmware
  KCONFIG:= \
	CONFIG_BT_MRVL \
	CONFIG_BT_MRVL_SDIO
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/btmrvl.ko \
	$(LINUX_DIR)/drivers/bluetooth/btmrvl_sdio.ko
  AUTOLOAD:=$(call AutoProbe,btmrvl btmrvl_sdio)
endef

define KernelPackage/btmrvl/description
 Kernel support for Marvell SDIO Bluetooth Module
endef

$(eval $(call KernelPackage,btmrvl))


define KernelPackage/btsdio
  SUBMENU:=$(BLUETOOTH_MENU)
  TITLE:=Bluetooth HCI SDIO driver
  DEPENDS:=+kmod-bluetooth +kmod-mmc
  KCONFIG:= \
	CONFIG_BT_HCIBTSDIO
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/btsdio.ko
  AUTOLOAD:=$(call AutoProbe,btsdio)
endef

define KernelPackage/btsdio/description
 Kernel support for Bluetooth device with SDIO interface
endef

$(eval $(call KernelPackage,btsdio))
