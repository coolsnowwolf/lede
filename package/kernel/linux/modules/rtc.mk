#
# Copyright (C) 2006-2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

RTC_MENU:=RTC Real-Time Clock Support

define KernelPackage/rtc-bq32k
  SUBMENU:=$(RTC_MENU)
  TITLE:=Texas Instruments BQ32000 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_BQ32K \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-bq32k.ko
  AUTOLOAD:=$(call AutoProbe,rtc-bq32k)
endef

define KernelPackage/rtc-bq32k/description
 Kernel module for Texas Instruments BQ32000 I2C RTC.
endef

$(eval $(call KernelPackage,rtc-bq32k))


define KernelPackage/rtc-ds1307
  SUBMENU:=$(RTC_MENU)
  TITLE:=Dallas/Maxim DS1307 (and compatible) RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core +kmod-regmap-i2c +kmod-hwmon-core
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
  SUBMENU:=$(RTC_MENU)
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
  SUBMENU:=$(RTC_MENU)
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


define KernelPackage/rtc-em3027
  SUBMENU:=$(RTC_MENU)
  TITLE:=Microelectronic EM3027 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_EM3027 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-em3027.ko
  AUTOLOAD:=$(call AutoProbe,rtc-em3027)
endef

define KernelPackage/rtc-em3027/description
 Kernel module for Microelectronic EM3027 RTC.
endef

$(eval $(call KernelPackage,rtc-em3027))


define KernelPackage/rtc-isl1208
  SUBMENU:=$(RTC_MENU)
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


define KernelPackage/rtc-mv
  SUBMENU:=$(RTC_MENU)
  TITLE:=Marvell SoC RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  KCONFIG:=CONFIG_RTC_DRV_MV \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-mv.ko
  AUTOLOAD:=$(call AutoProbe,rtc-mv)
endef

define KernelPackage/rtc-mv/description
 Kernel module for Marvell SoC RTC.
endef

$(eval $(call KernelPackage,rtc-mv))


define KernelPackage/rtc-pcf8563
  SUBMENU:=$(RTC_MENU)
  TITLE:=Philips PCF8563/Epson RTC8564 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core +!LINUX_6_12:kmod-regmap-i2c
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
  SUBMENU:=$(RTC_MENU)
  TITLE:=Philips PCF2123 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-regmap-spi
  KCONFIG:=CONFIG_RTC_DRV_PCF2123 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-pcf2123.ko
  AUTOLOAD:=$(call AutoProbe,rtc-pcf2123)
endef

define KernelPackage/rtc-pcf2123/description
 Kernel module for Philips PCF2123 RTC chip
endef

$(eval $(call KernelPackage,rtc-pcf2123))

define KernelPackage/rtc-pcf2127
  SUBMENU:=$(RTC_MENU)
  TITLE:=NXP PCF2127 and PCF2129 RTC support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core +kmod-regmap-spi
  KCONFIG:=CONFIG_RTC_DRV_PCF2127 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-pcf2127.ko
  AUTOLOAD:=$(call AutoProbe,rtc-pcf2127)
endef

define KernelPackage/rtc-pcf2127/description
 Kernel module for NXP PCF2127 and PCF2129 RTC chip
endef

$(eval $(call KernelPackage,rtc-pcf2127))

define KernelPackage/rtc-r7301
  SUBMENU:=$(RTC_MENU)
  TITLE:=Epson RTC7301 support
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-regmap-mmio
  KCONFIG:=CONFIG_RTC_DRV_R7301 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-r7301.ko
  AUTOLOAD:=$(call AutoProbe,rtc-r7301)
endef

define KernelPackage/rtc-r7301/description
 Kernel module for Epson RTC7301 RTC chip
endef

$(eval $(call KernelPackage,rtc-r7301))

define KernelPackage/rtc-rs5c372a
  SUBMENU:=$(RTC_MENU)
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

define KernelPackage/rtc-rx8025
  SUBMENU:=$(RTC_MENU)
  TITLE:=Epson RX-8025 / RX-8035
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_RX8025 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-rx8025.ko
  AUTOLOAD:=$(call AutoLoad,50,rtc-rx8025,1)
endef

define KernelPackage/rtc-rx8025/description
 Kernel module for Epson RX-8025 and RX-8035 I2C RTC chip
endef

$(eval $(call KernelPackage,rtc-rx8025))

define KernelPackage/rtc-s35390a
  SUBMENU:=$(RTC_MENU)
  TITLE:=Seico S-35390A
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_S35390A \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-s35390a.ko
  AUTOLOAD:=$(call AutoLoad,50,rtc-s35390a,1)
endef

define KernelPackage/rtc-s35390a/description
 Kernel module for Seiko Instruments S-35390A I2C RTC chip
endef

$(eval $(call KernelPackage,rtc-s35390a))

define KernelPackage/rtc-x1205
  SUBMENU:=$(RTC_MENU)
  TITLE:=Xicor Intersil X1205
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_RTC_DRV_X1205 \
	CONFIG_RTC_CLASS=y
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-x1205.ko
  AUTOLOAD:=$(call AutoProbe,rtc-x1205)
endef

define KernelPackage/rtc-x1205/description
 Kernel module for Xicor Intersil X1205 I2C RTC chip
endef

$(eval $(call KernelPackage,rtc-x1205))
