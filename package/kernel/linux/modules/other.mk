#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

OTHER_MENU:=Other modules

WATCHDOG_DIR:=watchdog


define KernelPackage/6lowpan
  SUBMENU:=$(OTHER_MENU)
  TITLE:=6LoWPAN shared code
  KCONFIG:= \
	CONFIG_6LOWPAN \
	CONFIG_6LOWPAN_NHC=n
  FILES:=$(LINUX_DIR)/net/6lowpan/6lowpan.ko
  AUTOLOAD:=$(call AutoProbe,6lowpan)
endef

define KernelPackage/6lowpan/description
  Shared 6lowpan code for IEEE 802.15.4 and Bluetooth.
endef

$(eval $(call KernelPackage,6lowpan))


define KernelPackage/bluetooth
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Bluetooth support
  DEPENDS:=@USB_SUPPORT +kmod-usb-core +kmod-crypto-hash +kmod-crypto-ecb +kmod-lib-crc16 +kmod-hid +!LINUX_3_18:kmod-crypto-cmac +!LINUX_3_18:kmod-regmap +LINUX_4_14:kmod-crypto-ecdh
  KCONFIG:= \
	CONFIG_BT \
	CONFIG_BT_BREDR=y \
	CONFIG_BT_DEBUGFS=n \
	CONFIG_BT_LE=y \
	CONFIG_BT_RFCOMM \
	CONFIG_BT_BNEP \
	CONFIG_BT_HCIBTUSB \
	CONFIG_BT_HCIBTUSB_BCM=n \
	CONFIG_BT_HCIUART \
	CONFIG_BT_HCIUART_BCM=n \
	CONFIG_BT_HCIUART_INTEL=n \
	CONFIG_BT_HCIUART_H4 \
	CONFIG_BT_HCIUART_NOKIA=n \
	CONFIG_BT_HIDP
  $(call AddDepends/rfkill)
  FILES:= \
	$(LINUX_DIR)/net/bluetooth/bluetooth.ko \
	$(LINUX_DIR)/net/bluetooth/rfcomm/rfcomm.ko \
	$(LINUX_DIR)/net/bluetooth/bnep/bnep.ko \
	$(LINUX_DIR)/net/bluetooth/hidp/hidp.ko \
	$(LINUX_DIR)/drivers/bluetooth/hci_uart.ko \
	$(LINUX_DIR)/drivers/bluetooth/btusb.ko
ifeq ($(strip $(call CompareKernelPatchVer,$(KERNEL_PATCHVER),ge,4.1.0)),1)
  FILES+= \
	$(LINUX_DIR)/drivers/bluetooth/btintel.ko
endif
  AUTOLOAD:=$(call AutoProbe,bluetooth rfcomm bnep hidp hci_uart btusb)
endef

define KernelPackage/bluetooth/description
 Kernel support for Bluetooth devices
endef

$(eval $(call KernelPackage,bluetooth))

define KernelPackage/ath3k
  SUBMENU:=$(OTHER_MENU)
  TITLE:=ATH3K Kernel Module support
  DEPENDS:=+kmod-bluetooth +ar3k-firmware
  KCONFIG:= \
	CONFIG_BT_ATH3K \
	CONFIG_BT_HCIUART_ATH3K=y
  $(call AddDepends/bluetooth)
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/ath3k.ko
  AUTOLOAD:=$(call AutoProbe,ath3k)
endef

define KernelPackage/ath3k/description
 Kernel support for ATH3K Module
endef

$(eval $(call KernelPackage,ath3k))


define KernelPackage/bluetooth_6lowpan
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Bluetooth 6LoWPAN support
  DEPENDS:=+kmod-6lowpan +kmod-bluetooth
  KCONFIG:=CONFIG_BT_6LOWPAN
  FILES:=$(LINUX_DIR)/net/bluetooth/bluetooth_6lowpan.ko
  AUTOLOAD:=$(call AutoProbe,bluetooth_6lowpan)
endef

define KernelPackage/bluetooth_6lowpan/description
 Kernel support for 6LoWPAN over Bluetooth Low Energy devices
endef

$(eval $(call KernelPackage,bluetooth_6lowpan))


define KernelPackage/btmrvl
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Marvell Bluetooth Kernel Module support
  DEPENDS:=+kmod-mmc +kmod-bluetooth +mwifiex-sdio-firmware
  KCONFIG:= \
	CONFIG_BT_MRVL \
	CONFIG_BT_MRVL_SDIO
  $(call AddDepends/bluetooth)
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/btmrvl.ko \
	$(LINUX_DIR)/drivers/bluetooth/btmrvl_sdio.ko
  AUTOLOAD:=$(call AutoProbe,btmrvl btmrvl_sdio)
endef

define KernelPackage/btmrvl/description
 Kernel support for Marvell SDIO Bluetooth Module
endef

$(eval $(call KernelPackage,btmrvl))


define KernelPackage/dma-buf
  SUBMENU:=$(OTHER_MENU)
  TITLE:=DMA shared buffer support
  HIDDEN:=1
  KCONFIG:=CONFIG_DMA_SHARED_BUFFER
  FILES:=$(LINUX_DIR)/drivers/dma-buf/dma-shared-buffer.ko
  AUTOLOAD:=$(call AutoLoad,20,dma-shared-buffer)
endef
$(eval $(call KernelPackage,dma-buf))


define KernelPackage/nvmem
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Non Volatile Memory support
  KCONFIG:=CONFIG_NVMEM
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/nvmem/nvmem_core.ko@ge4.9
endef

define KernelPackage/nvmem/description
  Support for NVMEM(Non Volatile Memory) devices like EEPROM, EFUSES, etc.
endef

$(eval $(call KernelPackage,nvmem))

define KernelPackage/eeprom-93cx6
  SUBMENU:=$(OTHER_MENU)
  TITLE:=EEPROM 93CX6 support
  KCONFIG:=CONFIG_EEPROM_93CX6
  FILES:=$(LINUX_DIR)/drivers/misc/eeprom/eeprom_93cx6.ko
  AUTOLOAD:=$(call AutoLoad,20,eeprom_93cx6)
endef

define KernelPackage/eeprom-93cx6/description
 Kernel module for EEPROM 93CX6 support
endef

$(eval $(call KernelPackage,eeprom-93cx6))


define KernelPackage/eeprom-at24
  SUBMENU:=$(OTHER_MENU)
  TITLE:=EEPROM AT24 support
  KCONFIG:=CONFIG_EEPROM_AT24
  DEPENDS:=+kmod-i2c-core +kmod-nvmem
  FILES:=$(LINUX_DIR)/drivers/misc/eeprom/at24.ko
  AUTOLOAD:=$(call AutoProbe,at24)
endef

define KernelPackage/eeprom-at24/description
 Kernel module for most I2C EEPROMs
endef

$(eval $(call KernelPackage,eeprom-at24))


define KernelPackage/eeprom-at25
  SUBMENU:=$(OTHER_MENU)
  TITLE:=EEPROM AT25 support
  KCONFIG:=CONFIG_EEPROM_AT25
  DEPENDS:=+kmod-nvmem
  FILES:=$(LINUX_DIR)/drivers/misc/eeprom/at25.ko
  AUTOLOAD:=$(call AutoProbe,at25)
endef

define KernelPackage/eeprom-at25/description
 Kernel module for most SPI EEPROMs
endef

$(eval $(call KernelPackage,eeprom-at25))


define KernelPackage/gpio-dev
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Generic GPIO char device support
  DEPENDS:=@GPIO_SUPPORT
  KCONFIG:=CONFIG_GPIO_DEVICE
  FILES:=$(LINUX_DIR)/drivers/char/gpio_dev.ko
  AUTOLOAD:=$(call AutoLoad,40,gpio_dev)
endef

define KernelPackage/gpio-dev/description
 Kernel module to allows control of GPIO pins using a character device.
endef

$(eval $(call KernelPackage,gpio-dev))


define KernelPackage/gpio-mcp23s08
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Microchip MCP23xxx I/O expander
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-core +LINUX_4_14:kmod-regmap
  KCONFIG:= \
	CONFIG_GPIO_MCP23S08 \
	CONFIG_PINCTRL_MCP23S08
  FILES:= \
	$(LINUX_DIR)/drivers/gpio/gpio-mcp23s08.ko@lt4.13 \
	$(LINUX_DIR)/drivers/pinctrl/pinctrl-mcp23s08.ko@ge4.13
  AUTOLOAD:=$(call AutoLoad,40,gpio-mcp23s08@lt4.13 pinctrl-mcp23s08@ge4.13)
endef

define KernelPackage/gpio-mcp23s08/description
 Kernel module for Microchip MCP23xxx SPI/I2C I/O expander
endef

$(eval $(call KernelPackage,gpio-mcp23s08))


define KernelPackage/gpio-nxp-74hc164
  SUBMENU:=$(OTHER_MENU)
  TITLE:=NXP 74HC164 GPIO expander support
  KCONFIG:=CONFIG_GPIO_74X164
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-74x164.ko
  AUTOLOAD:=$(call AutoProbe,gpio-74x164)
endef

define KernelPackage/gpio-nxp-74hc164/description
 Kernel module for NXP 74HC164 GPIO expander
endef

$(eval $(call KernelPackage,gpio-nxp-74hc164))

define KernelPackage/gpio-pca953x
  SUBMENU:=$(OTHER_MENU)
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-core
  TITLE:=PCA95xx, TCA64xx, and MAX7310 I/O ports
  KCONFIG:=CONFIG_GPIO_PCA953X
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-pca953x.ko
  AUTOLOAD:=$(call AutoLoad,55,gpio-pca953x)
endef

define KernelPackage/gpio-pca953x/description
 Kernel module for MAX731{0,2,3,5}, PCA6107, PCA953{4-9}, PCA955{4-7},
 PCA957{4,5} and TCA64{08,16} I2C GPIO expanders
endef

$(eval $(call KernelPackage,gpio-pca953x))

define KernelPackage/gpio-pcf857x
  SUBMENU:=$(OTHER_MENU)
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-core
  TITLE:=PCX857x, PCA967x and MAX732X I2C GPIO expanders
  KCONFIG:=CONFIG_GPIO_PCF857X
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-pcf857x.ko
  AUTOLOAD:=$(call AutoLoad,55,gpio-pcf857x)
endef

define KernelPackage/gpio-pcf857x/description
 Kernel module for PCF857x, PCA{85,96}7x, and MAX732[89] I2C GPIO expanders
endef

$(eval $(call KernelPackage,gpio-pcf857x))


define KernelPackage/ppdev
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Parallel port support
  KCONFIG:= \
	CONFIG_PARPORT \
	CONFIG_PPDEV
  FILES:= \
	$(LINUX_DIR)/drivers/parport/parport.ko \
	$(LINUX_DIR)/drivers/char/ppdev.ko
  AUTOLOAD:=$(call AutoLoad,50,parport ppdev)
endef

$(eval $(call KernelPackage,ppdev))


define KernelPackage/parport-pc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Parallel port interface (PC-style) support
  DEPENDS:=+kmod-ppdev
  KCONFIG:= \
	CONFIG_KS0108=n \
	CONFIG_PARPORT_PC \
	CONFIG_PARPORT_1284=y \
	CONFIG_PARPORT_PC_FIFO=y \
	CONFIG_PARPORT_PC_PCMCIA=n \
	CONFIG_PARPORT_PC_SUPERIO=y \
	CONFIG_PARPORT_SERIAL=n \
	CONFIG_PARIDE=n \
	CONFIG_SCSI_IMM=n \
	CONFIG_SCSI_PPA=n
  FILES:= \
	$(LINUX_DIR)/drivers/parport/parport_pc.ko
  AUTOLOAD:=$(call AutoLoad,51,parport_pc)
endef

$(eval $(call KernelPackage,parport-pc))


define KernelPackage/lp
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Parallel port line printer device support
  DEPENDS:=+kmod-ppdev
  KCONFIG:= \
	CONFIG_PRINTER
  FILES:= \
	$(LINUX_DIR)/drivers/char/lp.ko
  AUTOLOAD:=$(call AutoLoad,52,lp)
endef

$(eval $(call KernelPackage,lp))


define KernelPackage/mmc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=MMC/SD Card Support
  DEPENDS:=@!TARGET_uml
  KCONFIG:= \
	CONFIG_MMC \
	CONFIG_MMC_BLOCK \
	CONFIG_MMC_DEBUG=n \
	CONFIG_MMC_UNSAFE_RESUME=n \
	CONFIG_MMC_BLOCK_BOUNCE=y \
	CONFIG_MMC_TIFM_SD=n \
	CONFIG_MMC_WBSD=n \
	CONFIG_SDIO_UART=n
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/core/mmc_core.ko \
	$(LINUX_DIR)/drivers/mmc/card/mmc_block.ko@lt4.10 \
	$(LINUX_DIR)/drivers/mmc/core/mmc_block.ko@ge4.10
  AUTOLOAD:=$(call AutoProbe,mmc_core mmc_block,1)
endef

define KernelPackage/mmc/description
 Kernel support for MMC/SD cards
endef

$(eval $(call KernelPackage,mmc))


define KernelPackage/sdhci
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Secure Digital Host Controller Interface support
  DEPENDS:=+kmod-mmc
  KCONFIG:= \
	CONFIG_MMC_SDHCI \
	CONFIG_MMC_SDHCI_PLTFM \
	CONFIG_MMC_SDHCI_PCI=n
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/sdhci.ko \
	$(LINUX_DIR)/drivers/mmc/host/sdhci-pltfm.ko

  AUTOLOAD:=$(call AutoProbe,sdhci-pltfm,1)
endef

define KernelPackage/sdhci/description
 Kernel support for SDHCI Hosts
endef

$(eval $(call KernelPackage,sdhci))


define KernelPackage/rfkill
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RF switch subsystem support
  DEPENDS:=@USE_RFKILL +kmod-input-core
  KCONFIG:= \
    CONFIG_RFKILL_FULL \
    CONFIG_RFKILL_INPUT=y \
    CONFIG_RFKILL_LEDS=y
  FILES:= \
    $(LINUX_DIR)/net/rfkill/rfkill.ko
  AUTOLOAD:=$(call AutoLoad,20,rfkill)
endef

define KernelPackage/rfkill/description
 Say Y here if you want to have control over RF switches
 found on many WiFi and Bluetooth cards
endef

$(eval $(call KernelPackage,rfkill))


define KernelPackage/softdog
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Software watchdog driver
  KCONFIG:=CONFIG_SOFT_WATCHDOG \
  	CONFIG_SOFT_WATCHDOG_PRETIMEOUT=n
  FILES:=$(LINUX_DIR)/drivers/$(WATCHDOG_DIR)/softdog.ko
  AUTOLOAD:=$(call AutoLoad,50,softdog,1)
endef

define KernelPackage/softdog/description
 Software watchdog driver
endef

$(eval $(call KernelPackage,softdog))


define KernelPackage/ssb
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Silicon Sonics Backplane glue code
  DEPENDS:=@PCI_SUPPORT @!TARGET_brcm47xx @!TARGET_brcm63xx
  KCONFIG:=\
	CONFIG_SSB \
	CONFIG_SSB_B43_PCI_BRIDGE=y \
	CONFIG_SSB_DRIVER_MIPS=n \
	CONFIG_SSB_DRIVER_PCICORE=y \
	CONFIG_SSB_DRIVER_PCICORE_POSSIBLE=y \
	CONFIG_SSB_PCIHOST=y \
	CONFIG_SSB_PCIHOST_POSSIBLE=y \
	CONFIG_SSB_POSSIBLE=y \
	CONFIG_SSB_SPROM=y \
	CONFIG_SSB_SILENT=y
  FILES:=$(LINUX_DIR)/drivers/ssb/ssb.ko
  AUTOLOAD:=$(call AutoLoad,18,ssb,1)
endef

define KernelPackage/ssb/description
 Silicon Sonics Backplane glue code.
endef

$(eval $(call KernelPackage,ssb))


define KernelPackage/bcma
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCMA support
  DEPENDS:=@PCI_SUPPORT @!TARGET_brcm47xx @!TARGET_bcm53xx
  KCONFIG:=\
	CONFIG_BCMA \
	CONFIG_BCMA_POSSIBLE=y \
	CONFIG_BCMA_BLOCKIO=y \
	CONFIG_BCMA_HOST_PCI_POSSIBLE=y \
	CONFIG_BCMA_HOST_PCI=y \
	CONFIG_BCMA_HOST_SOC=n \
	CONFIG_BCMA_DRIVER_MIPS=n \
	CONFIG_BCMA_DRIVER_PCI_HOSTMODE=n \
	CONFIG_BCMA_DRIVER_GMAC_CMN=n \
	CONFIG_BCMA_DEBUG=n
  FILES:=$(LINUX_DIR)/drivers/bcma/bcma.ko
  AUTOLOAD:=$(call AutoLoad,29,bcma)
endef

define KernelPackage/bcma/description
 Bus driver for Broadcom specific Advanced Microcontroller Bus Architecture
endef

$(eval $(call KernelPackage,bcma))


define KernelPackage/rtc-ds1307
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Dallas/Maxim DS1307 (and compatible) RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core +LINUX_4_14:kmod-regmap
  KCONFIG:=CONFIG_RTC_DRV_DS1307 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-ds1307.ko
  AUTOLOAD:=$(call AutoProbe,rtc-ds1307)
endef

define KernelPackage/rtc-ds1307/description
 Kernel module for Dallas/Maxim DS1307/DS1337/DS1338/DS1340/DS1388/DS3231,
 Epson RX-8025 and various other compatible RTC chips connected via I2C.
endef

$(eval $(call KernelPackage,rtc-ds1307))


define KernelPackage/rtc-ds1374
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Dallas/Maxim DS1374 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_DS1374 \
	CONFIG_RTC_DRV_DS1374_WDT=n \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-ds1374.ko
  AUTOLOAD:=$(call AutoProbe,rtc-ds1374)
endef

define KernelPackage/rtc-ds1374/description
 Kernel module for Dallas/Maxim DS1374.
endef

$(eval $(call KernelPackage,rtc-ds1374))


define KernelPackage/rtc-ds1672
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Dallas/Maxim DS1672 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_DS1672 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-ds1672.ko
  AUTOLOAD:=$(call AutoProbe,rtc-ds1672)
endef

define KernelPackage/rtc-ds1672/description
 Kernel module for Dallas/Maxim DS1672 RTC.
endef

$(eval $(call KernelPackage,rtc-ds1672))


define KernelPackage/rtc-isl1208
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Intersil ISL1208 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_ISL1208 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-isl1208.ko
  AUTOLOAD:=$(call AutoProbe,rtc-isl1208)
endef

define KernelPackage/rtc-isl1208/description
 Kernel module for Intersil ISL1208 RTC.
endef

$(eval $(call KernelPackage,rtc-isl1208))


define KernelPackage/rtc-pcf8563
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Philips PCF8563/Epson RTC8564 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_PCF8563 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-pcf8563.ko
  AUTOLOAD:=$(call AutoProbe,rtc-pcf8563)
endef

define KernelPackage/rtc-pcf8563/description
 Kernel module for Philips PCF8563 RTC chip.
 The Epson RTC8564 should work as well.
endef

$(eval $(call KernelPackage,rtc-pcf8563))


define KernelPackage/rtc-pcf2123
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Philips PCF2123 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  KCONFIG:=CONFIG_RTC_DRV_PCF2123 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-pcf2123.ko
  AUTOLOAD:=$(call AutoProbe,rtc-pcf2123)
endef

define KernelPackage/rtc-pcf2123/description
 Kernel module for Philips PCF2123 RTC chip
endef

$(eval $(call KernelPackage,rtc-pcf2123))

define KernelPackage/rtc-pt7c4338
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Pericom PT7C4338 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_PT7C4338 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-pt7c4338.ko
  AUTOLOAD:=$(call AutoProbe,rtc-pt7c4338)
endef

define KernelPackage/rtc-pt7c4338/description
 Kernel module for Pericom PT7C4338 i2c RTC chip
endef

$(eval $(call KernelPackage,rtc-pt7c4338))

define KernelPackage/rtc-rs5c372a
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Ricoh R2025S/D, RS5C372A/B, RV5C386, RV5C387A
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_RS5C372 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-rs5c372.ko
  AUTOLOAD:=$(call AutoLoad,50,rtc-rs5c372,1)
endef

define KernelPackage/rtc-rs5c372a/description
 Kernel module for Ricoh R2025S/D, RS5C372A/B, RV5C386, RV5C387A RTC on chip module
endef

$(eval $(call KernelPackage,rtc-rs5c372a))


define KernelPackage/mtdtests
  SUBMENU:=$(OTHER_MENU)
  TITLE:=MTD subsystem tests
  KCONFIG:=CONFIG_MTD_TESTS
  FILES:=\
	$(LINUX_DIR)/drivers/mtd/tests/mtd_nandecctest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_oobtest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_pagetest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_readtest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_speedtest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_stresstest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_subpagetest.ko \
	$(LINUX_DIR)/drivers/mtd/tests/mtd_torturetest.ko
endef

define KernelPackage/mtdtests/description
 Kernel modules for MTD subsystem/driver testing
endef

$(eval $(call KernelPackage,mtdtests))


define KernelPackage/mtdoops
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Log panic/oops to an MTD buffer
  KCONFIG:=CONFIG_MTD_OOPS
  FILES:=$(LINUX_DIR)/drivers/mtd/mtdoops.ko
endef

define KernelPackage/mtdoops/description
 Kernel modules for Log panic/oops to an MTD buffer
endef

$(eval $(call KernelPackage,mtdoops))


define KernelPackage/mtdram
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Test MTD driver using RAM
  KCONFIG:=CONFIG_MTD_MTDRAM \
    CONFIG_MTDRAM_TOTAL_SIZE=4096 \
    CONFIG_MTDRAM_ERASE_SIZE=128
  FILES:=$(LINUX_DIR)/drivers/mtd/devices/mtdram.ko
endef

define KernelPackage/mtdram/description
  Test MTD driver using RAM
endef

$(eval $(call KernelPackage,mtdram))


define KernelPackage/serial-8250
  SUBMENU:=$(OTHER_MENU)
  TITLE:=8250 UARTs
  KCONFIG:= CONFIG_SERIAL_8250 \
	CONFIG_SERIAL_8250_PCI \
	CONFIG_SERIAL_8250_NR_UARTS=16 \
	CONFIG_SERIAL_8250_RUNTIME_UARTS=16 \
	CONFIG_SERIAL_8250_EXTENDED=y \
	CONFIG_SERIAL_8250_MANY_PORTS=y \
	CONFIG_SERIAL_8250_SHARE_IRQ=y \
	CONFIG_SERIAL_8250_DETECT_IRQ=n \
	CONFIG_SERIAL_8250_RSA=n
  FILES:= \
	$(LINUX_DIR)/drivers/tty/serial/8250/8250.ko \
	$(LINUX_DIR)/drivers/tty/serial/8250/8250_base.ko@ge4.4 \
	$(if $(CONFIG_PCI),$(LINUX_DIR)/drivers/tty/serial/8250/8250_pci.ko@ge4.4)
  AUTOLOAD:=$(call AutoProbe,8250 8250_base 8250_pci)
endef

define KernelPackage/serial-8250/description
 Kernel module for 8250 UART based serial ports
endef

$(eval $(call KernelPackage,serial-8250))


define KernelPackage/serial-8250-exar
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Exar 8250 UARTs
  KCONFIG:= CONFIG_SERIAL_8250_EXAR
  FILES:=$(LINUX_DIR)/drivers/tty/serial/8250/8250_exar.ko
  AUTOLOAD:=$(call AutoProbe,8250 8250_base 8250_exar)
  DEPENDS:=+kmod-serial-8250
endef

define KernelPackage/serial-8250-exar/description
 Kernel module for Exar serial ports
endef

$(eval $(call KernelPackage,serial-8250-exar))


define KernelPackage/regmap
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Generic register map support
  DEPENDS:=+kmod-lib-lzo +kmod-i2c-core
  KCONFIG:=CONFIG_REGMAP \
	   CONFIG_REGMAP_MMIO \
	   CONFIG_REGMAP_SPI \
	   CONFIG_REGMAP_I2C \
	   CONFIG_SPI=y
  FILES:= \
	$(LINUX_DIR)/drivers/base/regmap/regmap-i2c.ko \
	$(LINUX_DIR)/drivers/base/regmap/regmap-mmio.ko \
	$(if $(CONFIG_SPI),$(LINUX_DIR)/drivers/base/regmap/regmap-spi.ko)
  AUTOLOAD:=$(call AutoLoad,21,regmap-core regmap-i2c regmap-mmio regmap-spi)
  ifeq ($(strip $(CONFIG_EXTERNAL_KERNEL_TREE)),"")
   ifeq ($(strip $(CONFIG_KERNEL_GIT_CLONE_URI)),"")
    FILES += $(LINUX_DIR)/drivers/base/regmap/regmap-core.ko
   endif
  endif
endef

define KernelPackage/regmap/description
 Generic register map support
endef

$(eval $(call KernelPackage,regmap))

define KernelPackage/ikconfig
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Kernel configuration via /proc/config.gz
  KCONFIG:=CONFIG_IKCONFIG \
	   CONFIG_IKCONFIG_PROC=y
  FILES:=$(LINUX_DIR)/kernel/configs.ko
  AUTOLOAD:=$(call AutoLoad,70,configs)
endef

define KernelPackage/ikconfig/description
 Kernel configuration via /proc/config.gz
endef

$(eval $(call KernelPackage,ikconfig))


define KernelPackage/zram
  SUBMENU:=$(OTHER_MENU)
  TITLE:=ZRAM
  DEPENDS:=+kmod-lib-lzo +kmod-lib-lz4
  KCONFIG:= \
	CONFIG_ZSMALLOC \
	CONFIG_ZRAM \
	CONFIG_ZRAM_DEBUG=n \
	CONFIG_PGTABLE_MAPPING=n \
	CONFIG_ZRAM_WRITEBACK=n \
	CONFIG_ZSMALLOC_STAT=n \
	CONFIG_ZRAM_LZ4_COMPRESS=y
  FILES:= \
	$(LINUX_DIR)/mm/zsmalloc.ko \
	$(LINUX_DIR)/drivers/block/zram/zram.ko
  AUTOLOAD:=$(call AutoLoad,20,zsmalloc zram)
endef

define KernelPackage/zram/description
 Compressed RAM block device support
endef

$(eval $(call KernelPackage,zram))


define KernelPackage/pps
  SUBMENU:=$(OTHER_MENU)
  TITLE:=PPS support
  KCONFIG:=CONFIG_PPS
  FILES:=$(LINUX_DIR)/drivers/pps/pps_core.ko
  AUTOLOAD:=$(call AutoLoad,17,pps_core,1)
endef

define KernelPackage/pps/description
 PPS (Pulse Per Second) is a special pulse provided by some GPS
 antennae. Userland can use it to get a high-precision time
 reference.
endef

$(eval $(call KernelPackage,pps))


define KernelPackage/pps-gpio
  SUBMENU:=$(OTHER_MENU)
  TITLE:=PPS client using GPIO
  DEPENDS:=+kmod-pps
  KCONFIG:=CONFIG_PPS_CLIENT_GPIO
  FILES:=$(LINUX_DIR)/drivers/pps/clients/pps-gpio.ko
  AUTOLOAD:=$(call AutoLoad,18,pps-gpio,1)
endef

define KernelPackage/pps-gpio/description
 Support for a PPS source using GPIO. To be useful you must
 also register a platform device specifying the GPIO pin and
 other options, usually in your board setup.
endef

$(eval $(call KernelPackage,pps-gpio))


define KernelPackage/pps-ldisc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=PPS line discipline
  DEPENDS:=+kmod-pps
  KCONFIG:=CONFIG_PPS_CLIENT_LDISC
  FILES:=$(LINUX_DIR)/drivers/pps/clients/pps-ldisc.ko
  AUTOLOAD:=$(call AutoLoad,18,pps-ldisc,1)
endef

define KernelPackage/pps-ldisc/description
 Support for a PPS source connected with the CD (Carrier
 Detect) pin of your serial port.
endef

$(eval $(call KernelPackage,pps-ldisc))


define KernelPackage/ptp
  SUBMENU:=$(OTHER_MENU)
  TITLE:=PTP clock support
  DEPENDS:=+kmod-pps
  KCONFIG:= \
	CONFIG_PTP_1588_CLOCK \
	CONFIG_NET_PTP_CLASSIFY=y
  FILES:=$(LINUX_DIR)/drivers/ptp/ptp.ko
  AUTOLOAD:=$(call AutoLoad,18,ptp,1)
endef

define KernelPackage/ptp/description
 The IEEE 1588 standard defines a method to precisely
 synchronize distributed clocks over Ethernet networks.
endef

$(eval $(call KernelPackage,ptp))


define KernelPackage/ptp-gianfar
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Freescale Gianfar PTP support
  DEPENDS:=@TARGET_mpc85xx +kmod-ptp
  KCONFIG:=CONFIG_PTP_1588_CLOCK_GIANFAR
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/freescale/gianfar_ptp.ko
  AUTOLOAD:=$(call AutoProbe,gianfar_ptp)
endef

define KernelPackage/ptp-gianfar/description
 Kernel module for IEEE 1588 support for Freescale
 Gianfar Ethernet drivers
endef

$(eval $(call KernelPackage,ptp-gianfar))


define KernelPackage/random-core
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Hardware Random Number Generator Core support
  KCONFIG:=CONFIG_HW_RANDOM
  FILES:=$(LINUX_DIR)/drivers/char/hw_random/rng-core.ko
endef

define KernelPackage/random-core/description
 Kernel module for the HW random number generator core infrastructure
endef

$(eval $(call KernelPackage,random-core))

define KernelPackage/random-omap
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Hardware Random Number Generator OMAP support
  KCONFIG:=CONFIG_HW_RANDOM_OMAP
  FILES:=$(LINUX_DIR)/drivers/char/hw_random/omap-rng.ko
  DEPENDS:=@TARGET_omap24xx +kmod-random-core
  AUTOLOAD:=$(call AutoProbe,random-omap)
endef

define KernelPackage/random-omap/description
 Kernel module for the OMAP Random Number Generator
 found on OMAP16xx, OMAP2/3/4/5 and AM33xx/AM43xx multimedia processors.
endef

$(eval $(call KernelPackage,random-omap))

define KernelPackage/random-tpm
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Hardware Random Number Generator TPM support
  KCONFIG:=CONFIG_HW_RANDOM_TPM
  FILES:=$(LINUX_DIR)/drivers/char/hw_random/tpm-rng.ko
  DEPENDS:= +kmod-random-core +kmod-tpm
  AUTOLOAD:=$(call AutoProbe,tpm-rng)
endef

define KernelPackage/random-tpm/description
 Kernel module for the Random Number Generator
 in the Trusted Platform Module.
endef

$(eval $(call KernelPackage,random-tpm))

define KernelPackage/thermal
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Generic Thermal sysfs driver
  DEPENDS:=+kmod-hwmon-core
  HIDDEN:=1
  KCONFIG:= \
	CONFIG_THERMAL \
	CONFIG_THERMAL_OF=y \
	CONFIG_CPU_THERMAL=y \
	CONFIG_THERMAL_DEFAULT_GOV_STEP_WISE=y \
	CONFIG_THERMAL_DEFAULT_GOV_FAIR_SHARE=n \
	CONFIG_THERMAL_DEFAULT_GOV_USER_SPACE=n \
	CONFIG_THERMAL_EMERGENCY_POWEROFF_DELAY_MS=0 \
	CONFIG_THERMAL_GOV_FAIR_SHARE=n \
	CONFIG_THERMAL_GOV_STEP_WISE=y \
	CONFIG_THERMAL_GOV_USER_SPACE=n \
	CONFIG_THERMAL_HWMON=y \
	CONFIG_THERMAL_EMULATION=n
  FILES:=$(LINUX_DIR)/drivers/thermal/thermal_sys.ko
  AUTOLOAD:=$(call AutoProbe,thermal_sys)
endef

define KernelPackage/thermal/description
 Generic Thermal Sysfs driver offers a generic mechanism for thermal
 management. Usually it's made up of one or more thermal zone and cooling
 device.
endef

$(eval $(call KernelPackage,thermal))


define KernelPackage/gpio-beeper
  SUBMENU:=$(OTHER_MENU)
  TITLE:=GPIO beeper support
  DEPENDS:=+kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_MISC=y \
	CONFIG_INPUT_GPIO_BEEPER
  FILES:= \
	$(LINUX_DIR)/drivers/input/misc/gpio-beeper.ko
  AUTOLOAD:=$(call AutoLoad,50,gpio-beeper)
endef

define KernelPackage/gpio-beeper/description
 This enables playing beeps through an GPIO-connected buzzer
endef

$(eval $(call KernelPackage,gpio-beeper))


define KernelPackage/echo
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Line Echo Canceller
  KCONFIG:=CONFIG_ECHO
  FILES:=$(LINUX_DIR)/drivers/misc/echo/echo.ko
  AUTOLOAD:=$(call AutoLoad,50,echo)
endef

define KernelPackage/echo/description
 This driver provides line echo cancelling support for mISDN and
 DAHDI drivers
endef

$(eval $(call KernelPackage,echo))


define KernelPackage/bmp085
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BMP085/BMP18x pressure sensor
  DEPENDS:= +kmod-regmap @!LINUX_3_18
  KCONFIG:= CONFIG_BMP085
  FILES:= $(LINUX_DIR)/drivers/misc/bmp085.ko
endef

define KernelPackage/bmp085/description
 This driver adds support for Bosch Sensortec's digital pressure
 sensors BMP085 and BMP18x.
endef

$(eval $(call KernelPackage,bmp085))


define KernelPackage/bmp085-i2c
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BMP085/BMP18x pressure sensor I2C
  DEPENDS:= +kmod-bmp085
  KCONFIG:= CONFIG_BMP085_I2C
  FILES:= $(LINUX_DIR)/drivers/misc/bmp085-i2c.ko
  AUTOLOAD:=$(call AutoProbe,bmp085-i2c)
endef
define KernelPackage/bmp085-i2c/description
 This driver adds support for Bosch Sensortec's digital pressure
 sensor connected via I2C.
endef

$(eval $(call KernelPackage,bmp085-i2c))


define KernelPackage/bmp085-spi
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BMP085/BMP18x pressure sensor SPI
  DEPENDS:= +kmod-bmp085
  KCONFIG:= CONFIG_BMP085_SPI
  FILES:= $(LINUX_DIR)/drivers/misc/bmp085-spi.ko
  AUTOLOAD:=$(call AutoProbe,bmp085-spi)
endef
define KernelPackage/bmp085-spi/description
 This driver adds support for Bosch Sensortec's digital pressure
 sensor connected via SPI.
endef

$(eval $(call KernelPackage,bmp085-spi))

define KernelPackage/tpm
  SUBMENU:=$(OTHER_MENU)
  TITLE:=TPM Hardware Support
  KCONFIG:= CONFIG_TCG_TPM
  FILES:= $(LINUX_DIR)/drivers/char/tpm/tpm.ko
  AUTOLOAD:=$(call AutoLoad,10,tpm,1)
endef

define KernelPackage/tpm/description
	This enables TPM Hardware Support.
endef

$(eval $(call KernelPackage,tpm))

define KernelPackage/tpm-tis
  SUBMENU:=$(OTHER_MENU)
  TITLE:=TPM TIS 1.2 Interface / TPM 2.0 FIFO Interface
	DEPENDS:= @TARGET_x86 +kmod-tpm
  KCONFIG:= CONFIG_TCG_TIS
  FILES:= \
	$(LINUX_DIR)/drivers/char/tpm/tpm_tis.ko \
	$(LINUX_DIR)/drivers/char/tpm/tpm_tis_core.ko
  AUTOLOAD:=$(call AutoLoad,20,tpm_tis,1)
endef

define KernelPackage/tpm-tis/description
	If you have a TPM security chip that is compliant with the
	TCG TIS 1.2 TPM specification (TPM1.2) or the TCG PTP FIFO
	specification (TPM2.0) say Yes and it will be accessible from
	within Linux.
endef

$(eval $(call KernelPackage,tpm-tis))

define KernelPackage/tpm-i2c-atmel
  SUBMENU:=$(OTHER_MENU)
  TITLE:=TPM I2C Atmel Support
  DEPENDS:= +kmod-tpm +kmod-i2c-core
  KCONFIG:= CONFIG_TCG_TIS_I2C_ATMEL
  FILES:= $(LINUX_DIR)/drivers/char/tpm/tpm_i2c_atmel.ko
  AUTOLOAD:=$(call AutoLoad,40,tpm_i2c_atmel,1)
endef

define KernelPackage/tpm-i2c-atmel/description
	This enables the TPM Interface Specification 1.2 Interface (I2C - Atmel)
endef

$(eval $(call KernelPackage,tpm-i2c-atmel))

define KernelPackage/tpm-i2c-infineon
  SUBMENU:=$(OTHER_MENU)
  TITLE:= TPM I2C Infineon driver
  DEPENDS:= +kmod-tpm +kmod-i2c-core
  KCONFIG:= CONFIG_TCG_TIS_I2C_INFINEON
  FILES:= $(LINUX_DIR)/drivers/char/tpm/tpm_i2c_infineon.ko
  AUTOLOAD:= $(call AutoLoad,40,tpm_i2c_infineon,1)
endef

define KernelPackage/tpm-i2c-infineon/description
	This enables the TPM Interface Specification 1.2 Interface (I2C - Infineon)
endef

$(eval $(call KernelPackage,tpm-i2c-infineon))


define KernelPackage/w83627hf-wdt
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Winbond 83627HF Watchdog Timer
  KCONFIG:=CONFIG_W83627HF_WDT
  FILES:=$(LINUX_DIR)/drivers/$(WATCHDOG_DIR)/w83627hf_wdt.ko
  AUTOLOAD:=$(call AutoLoad,50,w83627hf-wdt,1)
endef

define KernelPackage/w83627hf-wdt/description
  Kernel module for Winbond 83627HF Watchdog Timer
endef

$(eval $(call KernelPackage,w83627hf-wdt))


define KernelPackage/itco-wdt
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Intel iTCO Watchdog Timer
  KCONFIG:=CONFIG_ITCO_WDT \
           CONFIG_ITCO_VENDOR_SUPPORT=y
  FILES:=$(LINUX_DIR)/drivers/$(WATCHDOG_DIR)/iTCO_wdt.ko \
         $(LINUX_DIR)/drivers/$(WATCHDOG_DIR)/iTCO_vendor_support.ko
  AUTOLOAD:=$(call AutoLoad,50,iTCO_vendor_support iTCO_wdt,1)
endef

define KernelPackage/itco-wdt/description
  Kernel module for Intel iTCO Watchdog Timer
endef

$(eval $(call KernelPackage,itco-wdt))


define KernelPackage/it87-wdt
  SUBMENU:=$(OTHER_MENU)
  TITLE:=ITE IT87 Watchdog Timer
  KCONFIG:=CONFIG_IT87_WDT
  FILES:=$(LINUX_DIR)/drivers/$(WATCHDOG_DIR)/it87_wdt.ko
  AUTOLOAD:=$(call AutoLoad,50,it87-wdt,1)
  MODPARAMS.it87-wdt:= \
	nogameport=1 \
	nocir=1
endef

define KernelPackage/it87-wdt/description
  Kernel module for ITE IT87 Watchdog Timer
endef

$(eval $(call KernelPackage,it87-wdt))
