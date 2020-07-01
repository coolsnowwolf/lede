#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

HWMON_MENU:=Hardware Monitoring Support

define KernelPackage/hwmon-core
  SUBMENU:=$(HWMON_MENU)
  TITLE:=Hardware monitoring support
  KCONFIG:= \
	CONFIG_HWMON \
	CONFIG_HWMON_DEBUG_CHIP=n
  FILES:= \
	$(LINUX_DIR)/drivers/hwmon/hwmon.ko
endef

define KernelPackage/hwmon-core/description
 Kernel modules for hardware monitoring
endef

$(eval $(call KernelPackage,hwmon-core))


define AddDepends/hwmon
  SUBMENU:=$(HWMON_MENU)
  DEPENDS:=kmod-hwmon-core $(1)
endef

define KernelPackage/hwmon-ads1015
  TITLE:=Texas Instruments ADS1015
  KCONFIG:= CONFIG_SENSORS_ADS1015
  FILES:= $(LINUX_DIR)/drivers/hwmon/ads1015.ko
  AUTOLOAD:=$(call AutoLoad,60,ads1015)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-ads1015/description
 Kernel module for Texas Instruments ADS1015 Analog-to-Digital converter
endef

$(eval $(call KernelPackage,hwmon-ads1015))

define KernelPackage/hwmon-adt7410
  TITLE:=ADT7410 monitoring support
  KCONFIG:= \
	CONFIG_SENSORS_ADT7X10 \
	CONFIG_SENSORS_ADT7410
  FILES:= \
	$(LINUX_DIR)/drivers/hwmon/adt7x10.ko \
	$(LINUX_DIR)/drivers/hwmon/adt7410.ko
  AUTOLOAD:=$(call AutoLoad,60,adt7x10 adt7410)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-adt7410/description
 Kernel module for ADT7410/7420 I2C thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-adt7410))


define KernelPackage/hwmon-adt7475
  TITLE:=ADT7473/7475/7476/7490 monitoring support
  KCONFIG:=CONFIG_SENSORS_ADT7475
  FILES:=$(LINUX_DIR)/drivers/hwmon/adt7475.ko
  AUTOLOAD:=$(call AutoProbe,adt7475)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-adt7475/description
 Kernel module for ADT7473/7475/7476/7490 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-adt7475))


define KernelPackage/hwmon-gpiofan
  TITLE:=Generic GPIO FAN support
  KCONFIG:=CONFIG_SENSORS_GPIO_FAN
  FILES:=$(LINUX_DIR)/drivers/hwmon/gpio-fan.ko
  AUTOLOAD:=$(call AutoLoad,60,gpio-fan)
  $(call AddDepends/hwmon,+kmod-i2c-core +PACKAGE_kmod-thermal:kmod-thermal)
endef

define KernelPackage/hwmon-gpiofan/description
  Kernel module for GPIO controlled FANs
endef

$(eval $(call KernelPackage,hwmon-gpiofan))


define KernelPackage/hwmon-ina209
  TITLE:=INA209 monitoring support
  KCONFIG:=CONFIG_SENSORS_INA209
  FILES:=$(LINUX_DIR)/drivers/hwmon/ina209.ko
  AUTOLOAD:=$(call AutoProbe,ina209)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-ina209/description
 Kernel module for ina209 dc power monitor chips
endef

$(eval $(call KernelPackage,hwmon-ina209))


define KernelPackage/hwmon-ina2xx
  TITLE:=INA2XX monitoring support
  KCONFIG:=CONFIG_SENSORS_INA2XX
  FILES:=$(LINUX_DIR)/drivers/hwmon/ina2xx.ko
  AUTOLOAD:=$(call AutoProbe,ina2xx)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-regmap-i2c)
endef

define KernelPackage/hwmon-ina2xx/description
 Kernel module for ina2xx dc current monitor chips
endef

$(eval $(call KernelPackage,hwmon-ina2xx))


define KernelPackage/hwmon-it87
  TITLE:=IT87 monitoring support
  KCONFIG:=CONFIG_SENSORS_IT87
  FILES:=$(LINUX_DIR)/drivers/hwmon/it87.ko
  AUTOLOAD:=$(call AutoProbe,it87)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-hwmon-vid +PACKAGE_kmod-thermal:kmod-thermal)
endef

define KernelPackage/hwmon-it87/description
 Kernel module for it87 thermal and voltage monitor chip
endef

$(eval $(call KernelPackage,hwmon-it87))


define KernelPackage/hwmon-lm63
  TITLE:=LM63/64 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM63
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm63.ko
  AUTOLOAD:=$(call AutoProbe,lm63)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-regmap-i2c)
endef

define KernelPackage/hwmon-lm63/description
 Kernel module for lm63 and lm64 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm63))


define KernelPackage/hwmon-lm75
  TITLE:=LM75 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM75
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm75.ko
  AUTOLOAD:=$(call AutoProbe,lm75)
  $(call AddDepends/hwmon,+kmod-i2c-core +PACKAGE_kmod-thermal:kmod-thermal +kmod-regmap-i2c)
endef

define KernelPackage/hwmon-lm75/description
 Kernel module for lm75 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm75))


define KernelPackage/hwmon-lm77
  TITLE:=LM77 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM77
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm77.ko
  AUTOLOAD:=$(call AutoProbe,lm77)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-lm77/description
 Kernel module for LM77 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm77))


define KernelPackage/hwmon-lm85
  TITLE:=LM85 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM85
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm85.ko
  AUTOLOAD:=$(call AutoProbe,lm85)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-lm85/description
 Kernel module for LM85 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm85))


define KernelPackage/hwmon-lm90
  TITLE:=LM90 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM90
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm90.ko
  AUTOLOAD:=$(call AutoProbe,lm90)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-lm90/description
 Kernel module for LM90 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm90))


define KernelPackage/hwmon-lm92
  TITLE:=LM92 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM92
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm92.ko
  AUTOLOAD:=$(call AutoProbe,lm92)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-lm92/description
 Kernel module for LM92 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm92))


define KernelPackage/hwmon-lm95241
  TITLE:=LM95241 monitoring support
  KCONFIG:=CONFIG_SENSORS_LM95241
  FILES:=$(LINUX_DIR)/drivers/hwmon/lm95241.ko
  AUTOLOAD:=$(call AutoProbe,lm95241)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-lm95241/description
 Kernel module for LM95241 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-lm95241))


define KernelPackage/hwmon-ltc4151
  TITLE:=LTC4151 monitoring support
  KCONFIG:=CONFIG_SENSORS_LTC4151
  FILES:=$(LINUX_DIR)/drivers/hwmon/ltc4151.ko
  AUTOLOAD:=$(call AutoProbe,ltc4151)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-ltc4151/description
 Kernel module for Linear Technology LTC4151 current and voltage monitor chip
endef

$(eval $(call KernelPackage,hwmon-ltc4151))


define KernelPackage/hwmon-mcp3021
  TITLE:=MCP3021/3221 monitoring support
  KCONFIG:=CONFIG_SENSORS_MCP3021
  FILES:=$(LINUX_DIR)/drivers/hwmon/mcp3021.ko
  AUTOLOAD:=$(call AutoProbe,mcp3021)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-mcp3021/description
 Kernel module for Linear Technology MCP3021/3221 current and voltage monitor chip
endef

$(eval $(call KernelPackage,hwmon-mcp3021))


define KernelPackage/hwmon-nct6775
  TITLE:=NCT6106D/6775F/6776F/6779D/6791D/6792D/6793D and compatibles monitoring support
  KCONFIG:=CONFIG_SENSORS_NCT6775
  FILES:=$(LINUX_DIR)/drivers/hwmon/nct6775.ko
  AUTOLOAD:=$(call AutoProbe,nct6775)
  $(call AddDepends/hwmon,@PCI_SUPPORT @TARGET_x86 +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-nct6775/description
 Kernel module for NCT6106D/6775F/6776F/6779D/6791D/6792D/6793D thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-nct6775))


define KernelPackage/hwmon-pc87360
  TITLE:=PC87360 monitoring support
  KCONFIG:=CONFIG_SENSORS_PC87360
  FILES:=$(LINUX_DIR)/drivers/hwmon/pc87360.ko
  AUTOLOAD:=$(call AutoProbe,pc87360)
  $(call AddDepends/hwmon,@TARGET_x86 +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-pc87360/description
 Kernel modules for PC87360 chips
endef

$(eval $(call KernelPackage,hwmon-pc87360))


define KernelPackage/pmbus-core
  TITLE:=PMBus support
  KCONFIG:= CONFIG_PMBUS
  FILES:=$(LINUX_DIR)/drivers/hwmon/pmbus/pmbus_core.ko
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/pmbus-core/description
 Kernel modules for Power Management Bus
endef

$(eval $(call KernelPackage,pmbus-core))


define KernelPackage/pmbus-zl6100
  TITLE:=Intersil / Zilker Labs ZL6100 hardware monitoring
  KCONFIG:=CONFIG_SENSORS_ZL6100
  FILES:=$(LINUX_DIR)/drivers/hwmon/pmbus/zl6100.ko
  AUTOLOAD:=$(call AutoProbe,zl6100)
  $(call AddDepends/hwmon, +kmod-pmbus-core)
endef

define KernelPackage/pmbus-zl6100/description
 Kernel module for Intersil / Zilker Labs ZL6100 and
compatible digital DC-DC controllers
endef

$(eval $(call KernelPackage,pmbus-zl6100))


define KernelPackage/hwmon-pwmfan
  TITLE:=Generic PWM FAN support
  KCONFIG:=CONFIG_SENSORS_PWM_FAN
  FILES:=$(LINUX_DIR)/drivers/hwmon/pwm-fan.ko
  AUTOLOAD:=$(call AutoLoad,60,pwm-fan)
  $(call AddDepends/hwmon, +PACKAGE_kmod-thermal:kmod-thermal)
endef

define KernelPackage/hwmon-pwmfan/description
  Kernel module for PWM controlled FANs
endef

$(eval $(call KernelPackage,hwmon-pwmfan))


define KernelPackage/hwmon-sch5627
  TITLE:=SMSC SCH5627 monitoring support
  KCONFIG:=CONFIG_SENSORS_SCH5627
  FILES:= \
	$(LINUX_DIR)/drivers/hwmon/sch5627.ko \
	$(LINUX_DIR)/drivers/hwmon/sch56xx-common.ko
  AUTOLOAD:=$(call AutoProbe,sch5627)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-sch5627/description
 SMSC SCH5627 Super I/O chips include complete hardware monitoring
endef

$(eval $(call KernelPackage,hwmon-sch5627))


define KernelPackage/hwmon-sht21
  TITLE:=Sensiron SHT21 and compat. monitoring support
  KCONFIG:=CONFIG_SENSORS_SHT21
  FILES:=$(LINUX_DIR)/drivers/hwmon/sht21.ko
  AUTOLOAD:=$(call AutoProbe,sht21)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-sht21/description
 Kernel module for Sensirion SHT21 and SHT25 temperature and humidity sensors chip
endef

$(eval $(call KernelPackage,hwmon-sht21))


define KernelPackage/hwmon-tmp102
  TITLE:=Texas Instruments TMP102 monitoring support
  KCONFIG:=CONFIG_SENSORS_TMP102
  FILES:=$(LINUX_DIR)/drivers/hwmon/tmp102.ko
  AUTOLOAD:=$(call AutoProbe,tmp102)
  $(call AddDepends/hwmon,+kmod-i2c-core +PACKAGE_kmod-thermal:kmod-thermal +kmod-regmap-i2c)
endef

define KernelPackage/hwmon-tmp102/description
 Kernel module for Texas Instruments TMP102 temperature sensors chip
endef

$(eval $(call KernelPackage,hwmon-tmp102))


define KernelPackage/hwmon-tmp103
  TITLE:=Texas Instruments TMP103 monitoring support
  KCONFIG:=CONFIG_SENSORS_TMP103
  FILES:=$(LINUX_DIR)/drivers/hwmon/tmp103.ko
  AUTOLOAD:=$(call AutoProbe,tmp103)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-regmap-i2c)
endef

define KernelPackage/hwmon-tmp103/description
 Kernel module for Texas Instruments TMP103 temperature sensors chip
endef

$(eval $(call KernelPackage,hwmon-tmp103))


define KernelPackage/hwmon-tmp421
  TITLE:=TI TMP421 and compatible monitoring support
  KCONFIG:=CONFIG_SENSORS_TMP421
  FILES:=$(LINUX_DIR)/drivers/hwmon/tmp421.ko
  AUTOLOAD:=$(call AutoLoad,60,tmp421)
  $(call AddDepends/hwmon,+kmod-i2c-core)
endef

define KernelPackage/hwmon-tmp421/description
  Kernel module for the Texas Instruments TMP421 and compatible chips.
endef

$(eval $(call KernelPackage,hwmon-tmp421))


define KernelPackage/hwmon-vid
  TITLE:=VID/VRM/VRD voltage conversion module.
  KCONFIG:=CONFIG_HWMON_VID
  FILES:=$(LINUX_DIR)/drivers/hwmon/hwmon-vid.ko
  AUTOLOAD:=$(call AutoLoad,41,hwmon-vid)
  $(call AddDepends/hwmon,)
endef

define KernelPackage/hwmon-vid/description
 VID/VRM/VRD voltage conversion module for hardware monitoring
endef

$(eval $(call KernelPackage,hwmon-vid))


define KernelPackage/hwmon-w83627ehf
  TITLE:=Winbond W83627EHF/EHG/DHG/UHG, W83667HG monitoring support
  KCONFIG:=CONFIG_SENSORS_W83627EHF
  FILES:=$(LINUX_DIR)/drivers/hwmon/w83627ehf.ko
  AUTOLOAD:=$(call AutoProbe,w83627ehf)
  $(call AddDepends/hwmon,@TARGET_x86 +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-w83627ehf/description
 Kernel module for Winbond W83627EHF/EHG/DHG/UHG and W83667HG thermal monitor chip
 Support for NCT6775F and NCT6776F has been removed from this driver in favour of
 using the nct6775 driver to handle those chips.
endef

$(eval $(call KernelPackage,hwmon-w83627ehf))


define KernelPackage/hwmon-w83627hf
  TITLE:=Winbond W83627HF monitoring support
  KCONFIG:=CONFIG_SENSORS_W83627HF
  FILES:=$(LINUX_DIR)/drivers/hwmon/w83627hf.ko
  AUTOLOAD:=$(call AutoLoad,50,w83627hf)
  $(call AddDepends/hwmon,@TARGET_x86 +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-w83627hf/description
  Kernel module for the Winbond W83627HF chips.
endef

$(eval $(call KernelPackage,hwmon-w83627hf))


define KernelPackage/hwmon-w83793
  TITLE:=Winbond W83793G/R monitoring support
  KCONFIG:=CONFIG_SENSORS_W83793
  FILES:=$(LINUX_DIR)/drivers/hwmon/w83793.ko
  AUTOLOAD:=$(call AutoProbe,w83793)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-hwmon-vid)
endef

define KernelPackage/hwmon-w83793/description
  Kernel module for the Winbond W83793G and W83793R chips.
endef

$(eval $(call KernelPackage,hwmon-w83793))


define KernelPackage/hwmon-adcxx
  TITLE:=ADCxx monitoring support
  KCONFIG:=CONFIG_SENSORS_ADCXX
  FILES:=$(LINUX_DIR)/drivers/hwmon/adcxx.ko
  AUTOLOAD:=$(call AutoLoad,60,adcxx)
  $(call AddDepends/hwmon,)
endef

define KernelPackage/hwmon-adcxx/description
  Kernel module for the National Semiconductor
  ADC<bb><c>S<sss> chip family, where
  * bb  is the resolution in number of bits (8, 10, 12)
  * c   is the number of channels (1, 2, 4, 8)
  * sss is the maximum conversion speed (021 for 200 kSPS, 051 for 500
    kSPS and 101 for 1 MSPS)

  Examples : ADC081S101, ADC124S501, ...
endef

$(eval $(call KernelPackage,hwmon-adcxx))


